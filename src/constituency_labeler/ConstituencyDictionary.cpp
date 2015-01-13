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

#include "ConstituencyDictionary.h"
#include "ConstituencyLabelerPipe.h"
#include "ConstituencyInstance.h"
#include <algorithm>

void ConstituencyDictionary::CreateConstituentDictionary(
    ConstituencyReader *reader) {
  // Create tag dictionary.
  CreateTagDictionary(reader);

  // Create constituent dictionary.
  LOG(INFO) << "Creating constituent and rule dictionary...";
  std::vector<int> label_freqs(constituent_alphabet_.size(), -1);

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  ConstituencyInstance *instance =
    static_cast<ConstituencyInstance*>(reader->GetNext());
  while (instance != NULL) {
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

  LOG(INFO) << "Number of constituent tags: " << constituent_alphabet_.size();
  LOG(INFO) << "Number of rules: " << rule_alphabet_.size();
  LOG(INFO) << "Constituent tags and their frequencies:";
  for (Alphabet::iterator it = constituent_alphabet_.begin();
       it != constituent_alphabet_.end(); ++it) {
    std::string label = it->first;
    int label_id = it->second;
    LOG(INFO) << label << "\t" << label_freqs[label_id];
  }
}
