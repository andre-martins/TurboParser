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

#include "ConstituencyInstanceNumeric.h"
#include <iostream>
#include <algorithm>
#include <map>

void ParseTreeNumeric::Initialize(const ConstituencyDictionary &dictionary,
                                  const ParseTree &parse_tree) {
  terminals_.clear();
  non_terminals_.clear();

  // Build a map from parse tree nodes to their indices in
  // terminal/non-terminals.
  std::map<ParseTreeNode*, int> map_terminals;
  std::map<ParseTreeNode*, int> map_non_terminals;
  for (int i = 0; i < parse_tree.terminals().size(); ++i) {
    map_terminals[parse_tree.terminals()[i]] = i;
  }
  for (int i = 0; i < parse_tree.non_terminals().size(); ++i) {
    map_non_terminals[parse_tree.non_terminals()[i]] = i;
  }

  terminals_.resize(parse_tree.terminals().size());
  non_terminals_.resize(parse_tree.non_terminals().size());
  for (int i = 0; i < parse_tree.terminals().size(); ++i) {
    terminals_[i] = new ParseTreeNumericNode;
  }
  for (int i = 0; i < parse_tree.non_terminals().size(); ++i) {
    non_terminals_[i] = new ParseTreeNumericNode;
  }
  for (int i = 0; i < parse_tree.terminals().size(); ++i) {
    ParseTreeNode *original_node = parse_tree.terminals()[i];
    ParseTreeNumericNode *node = terminals_[i];
    node->set_label(-1); // Terminal nodes receive no label.
    node->set_span(original_node->span());
    if (!original_node->parent()) {
      node->set_parent(NULL);
      root_ = node;
    } else {
      int parent_index = map_non_terminals[original_node->parent()];
      node->set_parent(non_terminals_[parent_index]);
    }
    CHECK_EQ(original_node->GetNumChildren(), 0);
  }

  for (int i = 0; i < parse_tree.non_terminals().size(); ++i) {
    ParseTreeNode *original_node = parse_tree.non_terminals()[i];
    ParseTreeNumericNode *node = non_terminals_[i];
    int id = dictionary.GetConstituentId(original_node->label());
    CHECK_LT(id, 0xff);
    if (id < 0) id = TOKEN_UNKNOWN;
    node->set_label(id);
    node->set_span(original_node->span());
    if (!original_node->parent()) {
      node->set_parent(NULL);
      root_ = node;
    } else {
      int parent_index = map_non_terminals[original_node->parent()];
      node->set_parent(non_terminals_[parent_index]);
    }
    for (int j = 0; j < original_node->GetNumChildren(); ++j) {
      ParseTreeNode *child = original_node->GetChild(j);
      if (child->IsLeaf()) {
        int child_index = map_terminals[child];
        node->AddChild(terminals_[child_index]);
      } else {
        int child_index = map_non_terminals[child];
        node->AddChild(non_terminals_[child_index]);
      }
    }
  }
}

void ConstituencyInstanceNumeric::Initialize(
    const ConstituencyDictionary &dictionary,
    ConstituencyInstance* instance) {
  SequenceInstanceNumeric::Initialize(dictionary, instance);

  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  int length = instance->size();

  parse_tree_.Initialize(dictionary, instance->GetParseTree());
}
