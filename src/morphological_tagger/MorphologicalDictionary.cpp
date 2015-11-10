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

#include "MorphologicalDictionary.h"
#include "MorphologicalReader.h"
#include "MorphologicalOptions.h"
#include "MorphologicalPipe.h"
#include <algorithm>





void MorphDictionary::CreateTagDictionary(MorphReader *reader) {
  SequenceDictionary::CreateTagDictionary(static_cast<SequenceReader*> (reader));

  LOG(INFO) << "Creating cpostag-morphtag dictionary...";
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  // Go through the corpus and build the existing tags for each word.
  cpostag_morphologicaltags_.clear();
  cpostag_morphologicaltags_.resize(token_dictionary_->GetNumCPosTags());

  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  MorphInstance *instance = static_cast<MorphInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;
      std::string cpostag = instance->GetCoarsePosTag(i);
      int cpostag_id = token_dictionary_->GetCoarsePosTagId(cpostag);
      //CHECK_GE(cpostag_id, 0);

      id = tag_alphabet_.Lookup(instance->GetTag(i));
      CHECK_GE(id, 0);

      // Insert new tag in the set of word tags, if it is not there
      // already. NOTE: this is inefficient, maybe we should be using a
      // different data structure.
      if (cpostag_id >= 0) {
        vector<int> &tags = cpostag_morphologicaltags_[cpostag_id];
        int j;
        for (j = 0; j < tags.size(); ++j) {
          if (tags[j] == id) break;
        }
        if (j == tags.size()) tags.push_back(id);
      }
    }
    delete instance;
    instance = static_cast<MorphInstance*>(reader->GetNext());
  }
  reader->Close();

  for (int i = 0; i < tag_alphabet_.size(); ++i) {
    unknown_cpostag_morphologicaltags_.push_back(i);
  }
  LOG(INFO) << "Number of unknown morphological tags: "
    << unknown_cpostag_morphologicaltags_.size();
}

void MorphTokenDictionary::InitializeFromMorphReader(MorphReader *reader) {
  TokenDictionary::InitializeStarter();
  LOG(INFO) << "Creating token dictionary...";

  std::vector<int> form_freqs;
  std::vector<int> form_lower_freqs;
  std::vector<int> lemma_freqs;
  std::vector<int> feats_freqs;
  std::vector<int> cpos_freqs;
  std::vector<int> shape_freqs;
  Alphabet form_alphabet;
  Alphabet form_lower_alphabet;
  Alphabet lemma_alphabet;
  Alphabet feats_alphabet;
  Alphabet cpos_alphabet;
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
    lemma_alphabet.Insert(special_symbols[i]);
    feats_alphabet.Insert(special_symbols[i]);
    cpos_alphabet.Insert(special_symbols[i]);
    shape_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    form_freqs.push_back(-1);
    form_lower_freqs.push_back(-1);
    lemma_freqs.push_back(-1);
    feats_freqs.push_back(-1);
    cpos_freqs.push_back(-1);
    shape_freqs.push_back(-1);
  }

  // Go through the corpus and build the dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  MorphInstance *instance = static_cast<MorphInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;

      // Add form to alphabet.
      std::string form = instance->GetForm(i);
      int form_length = static_cast<int>(form.length());
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

      // Add lemma to alphabet.
      id = lemma_alphabet.Insert(instance->GetLemma(i));
      if (id >= lemma_freqs.size()) {
        CHECK_EQ(id, lemma_freqs.size());
        lemma_freqs.push_back(0);
      }
      ++lemma_freqs[id];

      // Add prefix/suffix to alphabet.
      //using varying lengths : [1; desired_size].
      for (int numbchar = 1; numbchar <= std::min(prefix_length, form_length); numbchar++) {
        std::string prefix = form.substr(0, numbchar);
        id = prefix_alphabet_.Insert(prefix);
      }
      for (int numbchar = 1; numbchar <= std::min(suffix_length, form_length); numbchar++) {
        int start = form_length - numbchar;
        if (start < 0) start = 0;
        std::string suffix = form.substr(start, numbchar);
        id = suffix_alphabet_.Insert(suffix);
      }

      // Add CPOS to alphabet.
      id = cpos_alphabet.Insert(instance->GetCoarsePosTag(i));
      if (id >= cpos_freqs.size()) {
        CHECK_EQ(id, cpos_freqs.size());
        cpos_freqs.push_back(0);
      }
      ++cpos_freqs[id];
      // Add FEATS to alphabet.
      id = feats_alphabet.Insert(instance->GetTag(i));
      if (id >= feats_freqs.size()) {
        CHECK_EQ(id, feats_freqs.size());
        feats_freqs.push_back(0);
      }
      ++feats_freqs[id];

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
    instance = static_cast<MorphInstance*>(reader->GetNext());
  }
  reader->Close();

  // Now adjust the cutoffs if necessary.
  while (true) {
    form_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      form_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = form_alphabet.begin(); iter != form_alphabet.end(); ++iter) {
      if (form_freqs[iter->second] > form_cutoff) {
        form_alphabet_.Insert(iter->first);
      }
    }
    if (form_alphabet_.size() < kMaxFormAlphabetSize)
      break;
    ++form_cutoff;
    LOG(INFO) << "Incrementing form cutoff to " << form_cutoff << "...";
  }

  while (true) {
    form_lower_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      form_lower_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = form_lower_alphabet.begin(); iter != form_lower_alphabet.end(); ++iter) {
      if (form_lower_freqs[iter->second] > form_lower_cutoff) {
        form_lower_alphabet_.Insert(iter->first);
      }
    }
    if (form_lower_alphabet_.size() < kMaxFormAlphabetSize)
      break;
    ++form_lower_cutoff;
    LOG(INFO) << "Incrementing lower-case form cutoff to " << form_lower_cutoff << "...";
  }

  while (true) {
    lemma_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      lemma_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = lemma_alphabet.begin(); iter != lemma_alphabet.end(); ++iter) {
      if (lemma_freqs[iter->second] > lemma_cutoff) {
        lemma_alphabet_.Insert(iter->first);
      }
    }
    if (lemma_alphabet_.size() < kMaxLemmaAlphabetSize)
      break;
    ++lemma_cutoff;
    LOG(INFO) << "Incrementing lemma cutoff to " << lemma_cutoff << "...";
  }

  while (true) {
    cpos_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      cpos_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = cpos_alphabet.begin(); iter != cpos_alphabet.end(); ++iter) {
      if (cpos_freqs[iter->second] > cpos_cutoff) {
        cpos_alphabet_.Insert(iter->first);
      }
    }
    if (cpos_alphabet_.size() < kMaxCoarsePosAlphabetSize)
      break;
    ++cpos_cutoff;
    LOG(INFO) << "Incrementing CPOS cutoff to " << cpos_cutoff << "...";
  }

  while (true) {
    feats_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      feats_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = feats_alphabet.begin(); iter != feats_alphabet.end(); ++iter) {
      if (feats_freqs[iter->second] > feats_cutoff) {
        feats_alphabet_.Insert(iter->first);
      }
    }
    if (feats_alphabet_.size() < kMaxFeatsAlphabetSize)
      break;
    ++feats_cutoff;
    LOG(INFO) << "Incrementing FEATS cutoff to " << feats_cutoff << "...";
  }


  while (true) {
    shape_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      shape_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = shape_alphabet.begin(); iter != shape_alphabet.end(); ++iter) {
      if (shape_freqs[iter->second] > shape_cutoff) {
        shape_alphabet_.Insert(iter->first);
      }
    }
    if (shape_alphabet_.size() < kMaxShapeAlphabetSize)
      break;
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
    << "Number of lower-case forms: " << form_lower_alphabet_.size() << endl
    << "Number of lemmas: " << lemma_alphabet_.size() << endl
    << "Number of prefixes: " << prefix_alphabet_.size() << endl
    << "Number of suffixes: " << suffix_alphabet_.size() << endl
    << "Number of feats: " << feats_alphabet_.size() << endl
    << "Number of cpos: " << cpos_alphabet_.size() << endl
    << "Number of word shapes: " << shape_alphabet_.size();

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