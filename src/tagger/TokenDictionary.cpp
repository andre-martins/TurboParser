// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.0.
//
// TurboParser 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.0.  If not, see <http://www.gnu.org/licenses/>.

#include "TokenDictionary.h"
#include "Pipe.h"
#include "SerializationUtils.h"
#include <algorithm>

using namespace std;

// Special symbols.
const string kTokenUnknown = "_UNKNOWN_"; // Unknown word/lemma.
const string kTokenStart = "_START_"; // Start symbol.
const string kTokenStop = "_STOP_"; // Stop symbol.

// Maximum alphabet sizes.
const unsigned int kMaxFormAlphabetSize = 0xffff;
const unsigned int kMaxLemmaAlphabetSize = 0xffff;
const unsigned int kMaxPosAlphabetSize = 0xff;
const unsigned int kMaxCoarsePosAlphabetSize = 0xff;
const unsigned int kMaxFeatsAlphabetSize = 0xfff; // 0xffff;

DEFINE_int32(form_cutoff, 0,
             "Ignore word forms whose frequency is less than this.");
DEFINE_int32(lemma_cutoff, 0,
             "Ignore word lemmas whose frequency is less than this.");
DEFINE_int32(feats_cutoff, 0,
             "Ignore morphological features whose frequency is less than this.");
DEFINE_int32(pos_cutoff, 0,
             "Ignore POS tags whose frequency is less than this.");
DEFINE_int32(cpos_cutoff, 0,
             "Ignore CPOS tags whose frequency is less than this.");
DEFINE_int32(affix_length, 4,
             "Length of affixes/suffixes.");
DEFINE_bool(form_case_sensitive, false,
            "Distinguish upper/lower case of word forms.");

void TokenDictionary::Load(FILE* fs) {
  bool success;
  success = ReadInteger(fs, &FLAGS_affix_length);
  CHECK(success);
  LOG(INFO) << "Setting --affix_length=" << FLAGS_affix_length;
  success = ReadBool(fs, &FLAGS_form_case_sensitive);
  CHECK(success);
  LOG(INFO) << "Setting --form_case_sensitive=" << FLAGS_form_case_sensitive;

  if (0 > form_alphabet_.Load(fs)) CHECK(false);
  if (0 > lemma_alphabet_.Load(fs)) CHECK(false);
  if (0 > prefix_alphabet_.Load(fs)) CHECK(false);
  if (0 > suffix_alphabet_.Load(fs)) CHECK(false);
  if (0 > feats_alphabet_.Load(fs)) CHECK(false);
  if (0 > pos_alphabet_.Load(fs)) CHECK(false);
  if (0 > cpos_alphabet_.Load(fs)) CHECK(false);
  if (0 > shape_alphabet_.Load(fs)) CHECK(false);

  // TODO: Remove this (only for debugging purposes)
  //BuildNames();
}

void TokenDictionary::Save(FILE* fs) {
  bool success;
  success = WriteInteger(fs, FLAGS_affix_length);
  CHECK(success);
  success = WriteBool(fs, FLAGS_form_case_sensitive);
  CHECK(success);

  if (0 > form_alphabet_.Save(fs)) CHECK(false);
  if (0 > lemma_alphabet_.Save(fs)) CHECK(false);
  if (0 > prefix_alphabet_.Save(fs)) CHECK(false);
  if (0 > suffix_alphabet_.Save(fs)) CHECK(false);
  if (0 > feats_alphabet_.Save(fs)) CHECK(false);
  if (0 > pos_alphabet_.Save(fs)) CHECK(false);
  if (0 > cpos_alphabet_.Save(fs)) CHECK(false);
  if (0 > shape_alphabet_.Save(fs)) CHECK(false);
}

void TokenDictionary::InitializeFromReader(SequenceReader *reader) {
  LOG(INFO) << "Creating token dictionary...";

  int form_cutoff = FLAGS_form_cutoff;
  int affix_length = FLAGS_affix_length;
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  vector<int> form_freqs;

  Alphabet form_alphabet;

  string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kTokenUnknown;
  special_symbols[TOKEN_START] = kTokenStart;
  special_symbols[TOKEN_STOP] = kTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    prefix_alphabet_.Insert(special_symbols[i]);
    suffix_alphabet_.Insert(special_symbols[i]);
    form_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    form_freqs.push_back(-1);
  }

  // Go through the corpus and build the dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  SequenceInstance *instance = reader->GetNext();
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;

      // Add form to alphabet.
      string form = instance->GetForm(i);
      if (!form_case_sensitive) {
        transform(form.begin(), form.end(), form.begin(), ::tolower);
      }
      id = form_alphabet.Insert(form);
      if (id >= form_freqs.size()) {
        CHECK_EQ(id, form_freqs.size());
        form_freqs.push_back(0);
      }
      ++form_freqs[id];

      // Add prefix/suffix to alphabet.
      string prefix = form.substr(0, affix_length);
      id = prefix_alphabet_.Insert(prefix);
      int start = form.length() - affix_length;
      if (start < 0) start = 0;
      string suffix = form.substr(start, affix_length);
      id = suffix_alphabet_.Insert(suffix);
    }
    delete instance;
    instance = reader->GetNext();
  }
  reader->Close();

  // Now adjust the cutoffs if necessary.
  while (true) {
    form_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      form_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = form_alphabet.begin();
         iter != form_alphabet.end();
         ++iter) {
      if (form_freqs[iter->second] > form_cutoff) {
        form_alphabet_.Insert(iter->first);
      }
    }
    if (form_alphabet_.size() < kMaxFormAlphabetSize) break;
    ++form_cutoff;
    LOG(INFO) << "Incrementing form cutoff to " << form_cutoff << "...";
  }

  form_alphabet_.StopGrowth();
  lemma_alphabet_.StopGrowth();
  prefix_alphabet_.StopGrowth();
  suffix_alphabet_.StopGrowth();
  feats_alphabet_.StopGrowth();
  pos_alphabet_.StopGrowth();
  cpos_alphabet_.StopGrowth();

  LOG(INFO) << "Number of forms: " << form_alphabet_.size() << endl
            << "Number of prefixes: " 
            << prefix_alphabet_.size() << endl
            << "Number of suffixes: " 
            << suffix_alphabet_.size();

  CHECK_LT(form_alphabet_.size(), 0xffff);
  CHECK_LT(lemma_alphabet_.size(), 0xffff);
  CHECK_LT(prefix_alphabet_.size(), 0xffff);
  CHECK_LT(suffix_alphabet_.size(), 0xffff);
  CHECK_LT(feats_alphabet_.size(), 0xffff);
  CHECK_LT(pos_alphabet_.size(), 0xff);
  CHECK_LT(cpos_alphabet_.size(), 0xff);
}

void TokenDictionary::InitializeFromReader(DependencyReader *reader) {
  LOG(INFO) << "Creating token dictionary...";

  int form_cutoff = FLAGS_form_cutoff;
  int lemma_cutoff = FLAGS_lemma_cutoff;
  int feats_cutoff = FLAGS_feats_cutoff;
  int pos_cutoff = FLAGS_pos_cutoff;
  int cpos_cutoff = FLAGS_cpos_cutoff;
  int affix_length = FLAGS_affix_length;
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  vector<int> form_freqs;
  vector<int> lemma_freqs;
  vector<int> feats_freqs;
  vector<int> pos_freqs;
  vector<int> cpos_freqs;
  
  Alphabet form_alphabet;
  Alphabet lemma_alphabet;
  Alphabet feats_alphabet;
  Alphabet pos_alphabet;
  Alphabet cpos_alphabet;

  string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kTokenUnknown;
  special_symbols[TOKEN_START] = kTokenStart;
  special_symbols[TOKEN_STOP] = kTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    prefix_alphabet_.Insert(special_symbols[i]);
    suffix_alphabet_.Insert(special_symbols[i]);
    form_alphabet.Insert(special_symbols[i]);
    lemma_alphabet.Insert(special_symbols[i]);
    feats_alphabet.Insert(special_symbols[i]);
    pos_alphabet.Insert(special_symbols[i]);
    cpos_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    form_freqs.push_back(-1);
    lemma_freqs.push_back(-1);
    feats_freqs.push_back(-1);
    pos_freqs.push_back(-1);
    cpos_freqs.push_back(-1);
  }

  // Go through the corpus and build the dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  DependencyInstance *instance = reader->GetNext();
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;

      // Add form to alphabet.
      string form = instance->GetForm(i);
      if (!form_case_sensitive) {
        transform(form.begin(), form.end(), form.begin(), ::tolower);
      }
      id = form_alphabet.Insert(form);
      if (id >= form_freqs.size()) {
        CHECK_EQ(id, form_freqs.size());
        form_freqs.push_back(0);
      }
      ++form_freqs[id];

      // Add lemma to alphabet.
      id = lemma_alphabet.Insert(instance->GetLemma(i));
      if (id >= lemma_freqs.size()) {
        CHECK_EQ(id, lemma_freqs.size());
        lemma_freqs.push_back(0);
      }
      ++lemma_freqs[id];

      // Add prefix/suffix to alphabet.
      // TODO: add varying lengths.
      string prefix = form.substr(0, affix_length);
      id = prefix_alphabet_.Insert(prefix);
      int start = form.length() - affix_length;
      if (start < 0) start = 0;
      string suffix = form.substr(start, affix_length);
      id = suffix_alphabet_.Insert(suffix);

      // Add POS to alphabet.
      id = pos_alphabet.Insert(instance->GetPosTag(i));
      if (id >= pos_freqs.size()) {
        CHECK_EQ(id, pos_freqs.size());
        pos_freqs.push_back(0);
      }
      ++pos_freqs[id];

      // Add CPOS to alphabet.
      id = cpos_alphabet.Insert(instance->GetCoarsePosTag(i));
      if (id >= cpos_freqs.size()) {
        CHECK_EQ(id, cpos_freqs.size());
        cpos_freqs.push_back(0);
      }
      ++cpos_freqs[id];

      // Add FEATS to alphabet.
      for (int j = 0; j < instance->GetNumMorphFeatures(i); ++j) {
        id = feats_alphabet.Insert(instance->GetMorphFeature(i,j));
        if (id >= feats_freqs.size()) {
          CHECK_EQ(id, feats_freqs.size());
          feats_freqs.push_back(0);
        }
        ++feats_freqs[id];
      }
    }
    delete instance;
    instance = reader->GetNext();
  }
  reader->Close();

  // Now adjust the cutoffs if necessary.
  while (true) {
    form_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      form_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = form_alphabet.begin();
         iter != form_alphabet.end(); 
         ++iter) {
      if (form_freqs[iter->second] > form_cutoff) {
        form_alphabet_.Insert(iter->first);
      }
    }
    if (form_alphabet_.size() < kMaxFormAlphabetSize) break;
    ++form_cutoff;
    LOG(INFO) << "Incrementing form cutoff to " << form_cutoff << "...";
  }

  while (true) {
    lemma_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      lemma_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = lemma_alphabet.begin();
         iter != lemma_alphabet.end(); 
         ++iter) {
      if (lemma_freqs[iter->second] > lemma_cutoff) {
        lemma_alphabet_.Insert(iter->first);
      }
    }
    if (lemma_alphabet_.size() < kMaxLemmaAlphabetSize) break;
    ++lemma_cutoff;
    LOG(INFO) << "Incrementing lemma cutoff to " << lemma_cutoff << "...";
  }

  while (true) {
    pos_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      pos_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = pos_alphabet.begin();
         iter != pos_alphabet.end(); 
         ++iter) {
      if (pos_freqs[iter->second] > pos_cutoff) {
        pos_alphabet_.Insert(iter->first);
      }
    }
    if (pos_alphabet_.size() < kMaxPosAlphabetSize) break;
    ++pos_cutoff;
    LOG(INFO) << "Incrementing POS cutoff to " << pos_cutoff << "...";
  }

  while (true) {
    cpos_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      cpos_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = cpos_alphabet.begin();
         iter != cpos_alphabet.end(); 
         ++iter) {
      if (cpos_freqs[iter->second] > cpos_cutoff) {
        cpos_alphabet_.Insert(iter->first);
      }
    }
    if (cpos_alphabet_.size() < kMaxCoarsePosAlphabetSize) break;
    ++cpos_cutoff;
    LOG(INFO) << "Incrementing CPOS cutoff to " << cpos_cutoff << "...";
  }

  while (true) {
    feats_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      feats_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = feats_alphabet.begin();
         iter != feats_alphabet.end(); 
         ++iter) {
      if (feats_freqs[iter->second] > feats_cutoff) {
        feats_alphabet_.Insert(iter->first);
      }
    }
    if (feats_alphabet_.size() < kMaxFeatsAlphabetSize) break;
    ++feats_cutoff;
    LOG(INFO) << "Incrementing FEATS cutoff to " << feats_cutoff << "...";
  }

  form_alphabet_.StopGrowth();
  lemma_alphabet_.StopGrowth();
  prefix_alphabet_.StopGrowth();
  suffix_alphabet_.StopGrowth();
  feats_alphabet_.StopGrowth();
  pos_alphabet_.StopGrowth();
  cpos_alphabet_.StopGrowth();

  LOG(INFO) << "Number of forms: " << form_alphabet_.size() << endl 
            << "Number of lemmas: " << lemma_alphabet_.size() << endl  
            << "Number of prefixes: " << prefix_alphabet_.size() << endl  
            << "Number of suffixes: " << suffix_alphabet_.size() << endl  
            << "Number of feats: " << feats_alphabet_.size() << endl  
            << "Number of pos: " << pos_alphabet_.size() << endl 
            << "Number of cpos: " << cpos_alphabet_.size();

  CHECK_LT(form_alphabet_.size(), 0xffff);
  CHECK_LT(lemma_alphabet_.size(), 0xffff);
  CHECK_LT(prefix_alphabet_.size(), 0xffff);
  CHECK_LT(suffix_alphabet_.size(), 0xffff);
  CHECK_LT(feats_alphabet_.size(), 0xffff);
  CHECK_LT(pos_alphabet_.size(), 0xff);
  CHECK_LT(cpos_alphabet_.size(), 0xff);

  // TODO: Remove this (only for debugging purposes).
  BuildNames();
}
