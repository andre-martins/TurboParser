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

#include "ConstituencyDictionary.h"
#include "ConstituencyLabelerPipe.h"
#include "ConstituencyInstance.h"
#include <algorithm>

// Special symbols.
const string kConstituencyTokenUnknown = "_UNKNOWN_"; // Unknown word/lemma.
const string kConstituencyTokenStart = "_START_"; // Start symbol.
const string kConstituencyTokenStop = "_STOP_"; // Stop symbol.

// Maximum alphabet sizes.
const unsigned int kConstituencyMaxLemmaAlphabetSize = 0xffff;
const unsigned int kConstituencyMaxMorphAlphabetSize = 0xfff; //0xffff;

DEFINE_int32(constituency_lemma_cutoff, 0,
             "Ignore word lemmas whose frequency is less than this.");
DEFINE_int32(constituency_morph_cutoff, 0,
             "Ignore morphological features whose frequency is less than this.");

void ConstituencyDictionary::CreateConstituentDictionary(
  ConstituencyReader *reader) {
  // Create tag dictionary.
  CreateTagDictionary(reader);

  // Create constituent dictionary.
  LOG(INFO) << "Creating constituent and rule dictionary...";
  std::vector<int> label_freqs(constituent_alphabet_.size(), -1);

  int lemma_cutoff = FLAGS_constituency_lemma_cutoff;
  int morph_cutoff = FLAGS_constituency_morph_cutoff;
  std::vector<int> lemma_freqs;
  std::vector<int> morph_freqs;
  Alphabet lemma_alphabet;
  Alphabet morph_alphabet;

  string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kConstituencyTokenUnknown;
  special_symbols[TOKEN_START] = kConstituencyTokenStart;
  special_symbols[TOKEN_STOP] = kConstituencyTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    lemma_alphabet.Insert(special_symbols[i]);
    morph_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    lemma_freqs.push_back(-1);
    morph_freqs.push_back(-1);
  }

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  ConstituencyInstance *instance =
    static_cast<ConstituencyInstance*>(reader->GetNext());
  while (instance != NULL) {
    // Word-level elements.
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;
      // Add lemma to alphabet.
      id = lemma_alphabet.Insert(instance->GetLemma(i));
      if (id >= lemma_freqs.size()) {
        CHECK_EQ(id, lemma_freqs.size());
        lemma_freqs.push_back(0);
      }
      ++lemma_freqs[id];

      // Add FEATS to alphabet.
      for (int j = 0; j < instance->GetNumMorphFeatures(i); ++j) {
        id = morph_alphabet.Insert(instance->GetMorphFeature(i, j));
        if (id >= morph_freqs.size()) {
          CHECK_EQ(id, morph_freqs.size());
          morph_freqs.push_back(0);
        }
        ++morph_freqs[id];
      }
    }

    // Tree-level elements.
    const ParseTree &tree = instance->GetParseTree();
    const std::vector<ParseTreeNode*> &non_terminals = tree.non_terminals();
    int num_nodes = non_terminals.size();
    for (int i = 0; i < num_nodes; ++i) {
      ParseTreeNode *node = non_terminals[i];
      const std::string &label = node->label();

      // Add constituent to alphabet.
      int id = constituent_alphabet_.Insert(label);
      if (id >= label_freqs.size()) {
        CHECK_EQ(id, label_freqs.size());
        label_freqs.push_back(0);
      }
      ++label_freqs[id];

      // Add rule to alphabet.
      if (!node->IsPreTerminal()) {
        std::string rule = label + ":";
        for (int j = 0; j < node->GetNumChildren(); ++j) {
          rule += " " + node->GetChild(j)->label();
        }
        int rule_id = rule_alphabet_.Insert(rule);
      }
    }
    delete instance;
    instance = static_cast<ConstituencyInstance*>(reader->GetNext());
  }
  reader->Close();
  constituent_alphabet_.StopGrowth();

  // Now adjust the cutoffs if necessary.
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
    if (lemma_alphabet_.size() < kConstituencyMaxLemmaAlphabetSize) break;
    ++lemma_cutoff;
    LOG(INFO) << "Incrementing lemma cutoff to " << lemma_cutoff << "...";
  }

  while (true) {
    morph_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      morph_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = morph_alphabet.begin();
         iter != morph_alphabet.end();
         ++iter) {
      if (morph_freqs[iter->second] > morph_cutoff) {
        morph_alphabet_.Insert(iter->first);
      }
    }
    if (morph_alphabet_.size() < kConstituencyMaxMorphAlphabetSize) break;
    ++morph_cutoff;
    LOG(INFO) << "Incrementing FEATS cutoff to " << morph_cutoff << "...";
  }

  lemma_alphabet_.StopGrowth();
  morph_alphabet_.StopGrowth();

  CHECK_LT(lemma_alphabet_.size(), 0xffff);
  CHECK_LT(morph_alphabet_.size(), 0xffff);

  LOG(INFO) << "Number of lemmas: " << lemma_alphabet_.size();
  LOG(INFO) << "Number of feats: " << morph_alphabet_.size();
  LOG(INFO) << "Number of constituent tags: " << constituent_alphabet_.size();
  LOG(INFO) << "Number of rules: " << rule_alphabet_.size();
  LOG(INFO) << "Constituent tags and their frequencies:";
  for (Alphabet::iterator it = constituent_alphabet_.begin();
       it != constituent_alphabet_.end();
       ++it) {
    std::string label = it->first;
    int label_id = it->second;
    LOG(INFO) << label << "\t" << label_freqs[label_id];
  }
}
