// Copyright (c) 2012-2015 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.3.
//
// TurboParser 2.3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.3.  If not, see <http://www.gnu.org/licenses/>.

#include "TokenDictionary.h"
#include "Pipe.h"
#include "SerializationUtils.h"
#include <algorithm>

using namespace std;


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
DEFINE_int32(prefix_length, 4,
             "Length of prefixes.");
DEFINE_int32(suffix_length, 4,
             "Length of suffixes.");
DEFINE_bool(form_case_sensitive, false,
            "Distinguish upper/lower case of word forms.");

void TokenDictionary::Load(FILE* fs) {
  bool success;
  success = ReadBool(fs, &FLAGS_form_case_sensitive);
  CHECK(success);
  LOG(INFO) << "Setting --form_case_sensitive=" << FLAGS_form_case_sensitive;
  success = ReadInteger(fs, &FLAGS_prefix_length);
  CHECK(success);
  LOG(INFO) << "Setting --prefix_length=" << FLAGS_prefix_length;
  success = ReadInteger(fs, &FLAGS_suffix_length);
  CHECK(success);
  LOG(INFO) << "Setting --suffix_length=" << FLAGS_suffix_length;

  if (0 > form_alphabet_.Load(fs)) CHECK(false);
  if (0 > form_lower_alphabet_.Load(fs)) CHECK(false);
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
  success = WriteBool(fs, FLAGS_form_case_sensitive);
  CHECK(success);
  success = WriteInteger(fs, FLAGS_prefix_length);
  CHECK(success);
  success = WriteInteger(fs, FLAGS_suffix_length);
  CHECK(success);

  if (0 > form_alphabet_.Save(fs)) CHECK(false);
  if (0 > form_lower_alphabet_.Save(fs)) CHECK(false);
  if (0 > lemma_alphabet_.Save(fs)) CHECK(false);
  if (0 > prefix_alphabet_.Save(fs)) CHECK(false);
  if (0 > suffix_alphabet_.Save(fs)) CHECK(false);
  if (0 > feats_alphabet_.Save(fs)) CHECK(false);
  if (0 > pos_alphabet_.Save(fs)) CHECK(false);
  if (0 > cpos_alphabet_.Save(fs)) CHECK(false);
  if (0 > shape_alphabet_.Save(fs)) CHECK(false);
}


void TokenDictionary::InitializeStarter() {
  form_cutoff = FLAGS_form_cutoff;
  form_lower_cutoff = FLAGS_form_cutoff;
  lemma_cutoff = FLAGS_lemma_cutoff;
  feats_cutoff = FLAGS_feats_cutoff;
  pos_cutoff = FLAGS_pos_cutoff;
  cpos_cutoff = FLAGS_cpos_cutoff;
  shape_cutoff = 0;
  prefix_length = FLAGS_prefix_length;
  suffix_length = FLAGS_suffix_length;
  form_case_sensitive = FLAGS_form_case_sensitive;
}



void TokenDictionary::InitializeFromSequenceReader(SequenceReader *reader) {
  TokenDictionary::InitializeStarter();
  LOG(INFO) << "Creating token dictionary...";

  std::vector<int> form_freqs;
  std::vector<int> form_lower_freqs;
  std::vector<int> shape_freqs;
  Alphabet form_alphabet;
  Alphabet form_lower_alphabet;
  Alphabet shape_alphabet;

  std::string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kTokenUnknown;
  special_symbols[TOKEN_START] = kTokenStart;
  special_symbols[TOKEN_STOP] = kTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    prefix_alphabet_.Insert(special_symbols[i]);
    suffix_alphabet_.Insert(special_symbols[i]);
    form_alphabet.Insert(special_symbols[i]);
    form_lower_alphabet.Insert(special_symbols[i]);
    shape_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    form_freqs.push_back(-1);
    form_lower_freqs.push_back(-1);
    shape_freqs.push_back(-1);
  }

  // Go through the corpus and build the dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  SequenceInstance *instance = static_cast<SequenceInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;

      // Add form to alphabet.
      std::string form = instance->GetForm(i);
      std::string form_lower(form);
      transform(form_lower.begin(), form_lower.end(), form_lower.begin(), ::tolower);
      if (!form_case_sensitive) form = form_lower;
      id = form_alphabet.Insert(form);
      if (id >= form_freqs.size()) {
        CHECK_EQ(id, form_freqs.size());
        form_freqs.push_back(0);
      }
      ++form_freqs[id];

      // Add lower-case form to the alphabet.
      id = form_lower_alphabet.Insert(form_lower);
      if (id >= form_lower_freqs.size()) {
        CHECK_EQ(id, form_lower_freqs.size());
        form_lower_freqs.push_back(0);
      }
      ++form_lower_freqs[id];

      // Add prefix/suffix to alphabet.
      std::string prefix = form.substr(0, prefix_length);
      id = prefix_alphabet_.Insert(prefix);
      int start = form.length() - suffix_length;
      if (start < 0) start = 0;
      std::string suffix = form.substr(start, suffix_length);
      id = suffix_alphabet_.Insert(suffix);

      // Add shape to alphabet.
      std::string shape;
      GetWordShape(instance->GetForm(i), &shape);
      id = shape_alphabet.Insert(shape);
      if (id >= shape_freqs.size()) {
        CHECK_EQ(id, shape_freqs.size());
        shape_freqs.push_back(0);
      }
      ++shape_freqs[id];
    }
    delete instance;
    instance = static_cast<SequenceInstance*>(reader->GetNext());
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
    form_lower_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      form_lower_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = form_lower_alphabet.begin();
    iter != form_lower_alphabet.end();
      ++iter) {
      if (form_lower_freqs[iter->second] > form_lower_cutoff) {
        form_lower_alphabet_.Insert(iter->first);
      }
    }
    if (form_lower_alphabet_.size() < kMaxFormAlphabetSize) break;
    ++form_lower_cutoff;
    LOG(INFO) << "Incrementing lower-case form cutoff to "
      << form_lower_cutoff << "...";
  }

  while (true) {
    shape_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      shape_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = shape_alphabet.begin();
    iter != shape_alphabet.end();
      ++iter) {
      if (shape_freqs[iter->second] > shape_cutoff) {
        shape_alphabet_.Insert(iter->first);
      }
    }
    if (shape_alphabet_.size() < kMaxShapeAlphabetSize) break;
    ++shape_cutoff;
    LOG(INFO) << "Incrementing shape cutoff to " << shape_cutoff << "...";
  }

  form_alphabet_.StopGrowth();
  form_lower_alphabet_.StopGrowth();
  shape_alphabet_.StopGrowth();
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
    << suffix_alphabet_.size() << endl
    << "Number of word shapes: "
    << shape_alphabet_.size();

  CHECK_LT(form_alphabet_.size(), 0xffff);
  CHECK_LT(form_lower_alphabet_.size(), 0xffff);
  CHECK_LT(shape_alphabet_.size(), 0xffff);
  CHECK_LT(lemma_alphabet_.size(), 0xffff);
  CHECK_LT(prefix_alphabet_.size(), 0xffff);
  CHECK_LT(suffix_alphabet_.size(), 0xffff);
  CHECK_LT(feats_alphabet_.size(), 0xffff);
  CHECK_LT(pos_alphabet_.size(), 0xff);
  CHECK_LT(cpos_alphabet_.size(), 0xff);
}
