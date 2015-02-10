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

#ifndef CONSTITUENCYINSTANCENUMERIC_H_
#define CONSTITUENCYINSTANCENUMERIC_H_

#include "SequenceInstanceNumeric.h"
#include "ConstituencyInstance.h"
#include "ConstituencyDictionary.h"
#include "ParseTree.h"

class ParseTreeNumericNode : public TreeNode<int> {
 public:
  ParseTreeNumericNode() {}
  ParseTreeNumericNode(ParseTreeNumericNode *parent) {
    parent_ = parent;
  }
  virtual ~ParseTreeNumericNode() {}

  // True if node is a pre-terminal.
  bool IsPreTerminal() const {
    return children_.size() == 1 && children_[0]->IsLeaf();
  }

  // Get parent node.
  ParseTreeNumericNode *parent() {
    return static_cast<ParseTreeNumericNode*>(parent_);
  }

  // Get child nodes.
  ParseTreeNumericNode *GetChild(int i) const {
    return static_cast<ParseTreeNumericNode*>(children_[i]);
  }

  // Get start/end positions.
  int start() const { return span_.start(); }
  int end() const { return span_.end(); }

  // Get/set node label.
  int label() const { return label_; }
  void set_label(int label) { label_ = label; }

  // Get/set node span.
  const Span &span() const { return span_; }
  void set_span(const Span &span) { span_ = span; }

  // Get/set production rule.
  int rule() const { return rule_; }
  void set_rule(int rule) { rule_ = rule; }

 protected:
  Span span_;
  int rule_; // Production rule whose left side is this node.
};

class ParseTreeNumeric {
 public:
  ParseTreeNumeric() { root_ = NULL; }
  virtual ~ParseTreeNumeric() { DeleteAllNodes(); }

 public:
  void DeleteAllNodes() {
    delete root_;
    terminals_.clear();
    non_terminals_.clear();
  }

  ParseTreeNumericNode *GetRoot() const {
    return root_;
  }

  const std::vector<ParseTreeNumericNode*> &terminals() const {
    return terminals_;
  }
  const std::vector<ParseTreeNumericNode*> &non_terminals() const {
    return non_terminals_;
  }

  void Initialize(const ConstituencyDictionary &dictionary,
                  const ParseTree &parse_tree);

 protected:
  ParseTreeNumericNode* root_;
  std::vector<ParseTreeNumericNode*> terminals_; // Words.
  std::vector<ParseTreeNumericNode*> non_terminals_; // Tags and internal nodes.
};

class ConstituencyInstanceNumeric : public SequenceInstanceNumeric {
 public:
  ConstituencyInstanceNumeric() {};
  virtual ~ConstituencyInstanceNumeric() { Clear(); };

  virtual void Clear() {
    SequenceInstanceNumeric::Clear();
    lemma_ids_.clear();
    for (int i = 0; i < morph_ids_.size(); i++) {
      morph_ids_[i].clear();
    }
  }

  void Initialize(const ConstituencyDictionary &dictionary,
                  ConstituencyInstance *instance);

  int GetLemmaId(int i) { return lemma_ids_[i]; };
  int GetNumMorphFeatures(int i) { return morph_ids_[i].size(); };
  int GetMorphFeature(int i, int j) { return morph_ids_[i][j]; };

  const ParseTreeNumeric &GetParseTree() const { return parse_tree_; }

 protected:
  std::vector<int> lemma_ids_;
  std::vector<std::vector<int> > morph_ids_;
  ParseTreeNumeric parse_tree_;
};

#endif /* CONSTITUENCYINSTANCENUMERIC_H_ */
