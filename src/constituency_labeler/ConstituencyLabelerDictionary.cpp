// Copyright (c) 2012-2013 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.1.
//
// TurboParser 2.1 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.1.  If not, see <http://www.gnu.org/licenses/>.

#include "ConstituencyLabelerDictionary.h"
#include "ConstituencyLabelerPipe.h"
#include "ConstituencyLabelerInstance.h"
#include <algorithm>

// TODO(atm): must create a NULL label and add it as an allowed label to every
// constituent.

// Special symbols.
const string kConstituentUnknown = "_UNKNOWN_"; // Unknown constituent.
const string kConstituentStart = "_START_"; // Start symbol.
const string kConstituentStop = "_STOP_"; // Stop symbol.

void ConstituencyLabelerDictionary::CreateConstituentDictionary(
    ConstituencyReader *reader) {
  std::string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kConstituentUnknown;
  special_symbols[TOKEN_START] = kConstituentStart;
  special_symbols[TOKEN_STOP] = kConstituentStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    constituent_alphabet_.Insert(special_symbols[i]);
  }

  ConstituencyDictionary::CreateConstituentDictionary(reader);
}

void ConstituencyLabelerDictionary::CreateLabelDictionary(
    ConstituencyLabelerReader *reader) {
  LOG(INFO) << "Creating label dictionary...";
  std::vector<int> label_freqs;

  // Go through the corpus and build the existing labels for each constituent.
  constituent_labels_.clear();
  constituent_labels_.resize(constituent_alphabet_.size());
  constituent_label_frequencies_.clear();
  constituent_label_frequencies_.resize(constituent_alphabet_.size());

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  ConstituencyLabelerInstance *instance =
    static_cast<ConstituencyLabelerInstance*>(reader->GetNext());
  while (instance != NULL) {
    const ParseTree &tree = instance->GetParseTree();
    const std::vector<ParseTreeNode*> &non_terminals = tree.non_terminals();
    int num_nodes = instance->GetNumConstituents();
    CHECK_EQ(num_nodes, non_terminals.size());
    for (int i = 0; i < num_nodes; ++i) {
      ParseTreeNode *node = non_terminals[i];
      const std::string &constituent_name = node->label();

      // Add label to alphabet.
      int label_id = label_alphabet_.Insert(instance->GetConstituentLabel(i));
      if (label_id >= label_freqs.size()) {
        CHECK_EQ(label_id, label_freqs.size());
        label_freqs.push_back(0);
      }
      ++label_freqs[label_id];

      // Insert new label in the set of constituent labels, if it is not there
      // already. NOTE: this is inefficient, maybe we should be using a
      // different data structure.
      int constituent_id = constituent_alphabet_.Lookup(constituent_name);
      if (constituent_id >= 0) {
        std::vector<int> &constituent_labels =
          constituent_labels_[constituent_id];
        std::vector<int> &constituent_label_freqs =
          constituent_label_frequencies_[constituent_id];
        int j;
        for (j = 0; j < constituent_labels.size(); ++j) {
          if (constituent_labels[j] == label_id) {
            ++constituent_label_freqs[j];
            break;
          }
        }
        if (j == constituent_labels.size()) {
          constituent_labels.push_back(label_id);
          constituent_label_freqs.push_back(1);
        }
      }
    }
    delete instance;
    instance = static_cast<ConstituencyLabelerInstance*>(reader->GetNext());
  }
  reader->Close();
  label_alphabet_.StopGrowth();

  LOG(INFO) << "Number of labels: " << label_alphabet_.size();
  LOG(INFO) << "Labels and their frequencies:";
  for (Alphabet::iterator it = label_alphabet_.begin();
       it != label_alphabet_.end(); ++it) {
    std::string label = it->first;
    int label_id = it->second;
    LOG(INFO) << label << "\t" << label_freqs[label_id];
  }

  label_alphabet_.BuildNames();

  LOG(INFO) << "Labels and their frequencies per constituent:";
  for (Alphabet::iterator it = constituent_alphabet_.begin();
       it != constituent_alphabet_.end(); ++it) {
    std::string constituent = it->first;
    int constituent_id = it->second;
    std::vector<int> &constituent_labels =
      constituent_labels_[constituent_id];
    std::vector<int> &constituent_label_freqs =
      constituent_label_frequencies_[constituent_id];
    for (int i = 0; i < constituent_labels.size(); ++i) {
      const std::string &label = GetLabelName(constituent_labels[i]);
      int frequency = constituent_label_freqs[i];
      LOG(INFO) << constituent << " -> " << label << "\t" << frequency;
    }
  }
}
