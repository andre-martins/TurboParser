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

#include "ConstituencyLabelerReader.h"
#include "ConstituencyLabelerInstance.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Instance *ConstituencyLabelerReader::GetNext() {
  // Fill all fields for the entire sentence.
  ConstituencyLabelerInstance *instance = NULL;
  std::string line;
  if (is_.is_open() && !is_.eof()) {
    getline(is_, line);
    if (line != "") {
      ParseTree tree;
      std::vector<std::string> words;
      std::vector<std::string> tags;
      tree.LoadFromString(line);
      tree.ExtractWordsAndTags(&words, &tags);
      tree.CollapseSingletonSpines(false, true);

      std::cout << line << std::endl;

      const std::vector<ParseTreeNode*> &terminals = tree.terminals();
      const std::vector<ParseTreeNode*> &non_terminals = tree.non_terminals();
      std::vector<std::string> constituent_labels;
      for (int i = 0; i < non_terminals.size(); ++i) {
        std::string full_label = non_terminals[i]->label();
        std::vector<std::string> labels;
        std::string delim = "";
        delim += kParseTreeLabelSeparator;
        StringSplit(full_label, delim, &labels);
        if (labels.size() > 1) {
          non_terminals[i]->set_label(labels.back());
          labels.pop_back();
          std::string label;
          StringJoin(labels, kParseTreeLabelSeparator, &label);
          constituent_labels.push_back(label);
        } else {
          constituent_labels.push_back(""); // NULL label.
        }
      }
      instance = new ConstituencyLabelerInstance;
      instance->Initialize(words, tags, tree, constituent_labels);

      std::string info;
      instance->GetParseTree().SaveToString(&info);
      std::cout << info << std::endl;
      for (int i = 0; i < instance->size(); ++i) {
        std::cout << instance->GetForm(i) << "\t"
                  << instance->GetTag(i) << std::endl;
      }
      CHECK_EQ(terminals.size(), instance->size());
      for (int i = 0; i < terminals.size(); ++i) {
        std::cout << terminals[i]->label() << "("
                  << terminals[i]->start() << ","
                  << terminals[i]->end() << ")"
                  << " fathered by "
                  << terminals[i]->parent()->label() << "("
                  << terminals[i]->parent()->start() << ","
                  << terminals[i]->parent()->end() << ")"
                  << endl;
      }
      for (int i = 0; i < non_terminals.size(); ++i) {
        CHECK(non_terminals[i]);
        std::cout << non_terminals[i]->label() << "("
                  << non_terminals[i]->start() << ","
                  << non_terminals[i]->end() << ")";
        if (non_terminals[i]->parent() != NULL) {
          std::cout << " fathered by "
                    << non_terminals[i]->parent()->label() << "("
                    << non_terminals[i]->parent()->start() << ","
                    << non_terminals[i]->parent()->end() << ")";
        }
        std::cout << " -> " << instance->GetConstituentLabels()[i] << endl;
      }
    }
  }

  return static_cast<Instance*>(instance);
}
