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

#ifndef PARSETREE_H_
#define PARSETREE_H_

#include <string>
#include <vector>
#include "EntitySpan.h" // TODO(atm): create a Span.h.

const char kParseTreeLabelSeparator = '|';

class ParseTreeNode {
 public:
  ParseTreeNode(class ParseTreeNode *parent) {
    span_ = NULL;
    parent_ = parent;
  }
  virtual ~ParseTreeNode() {
    // Delete all nodes underneath.
    delete span_;
    for (int i = 0; i < children_.size(); ++i) {
      delete children_[i];
    }
  }

  // True if node is a leaf.
  bool IsLeaf() const { return children_.size() == 0; }

  // True if node is a pre-terminal.
  bool IsPreTerminal() const {
    return children_.size() == 1 && children_[0]->IsLeaf();
  }

  // Get start/end positions.
  int start() const { return span_->start(); }
  int end() const { return span_->end(); }

  // Get/set node label.
  const std::string &label() const { return label_; }
  void set_label(const std::string &label) { label_ = label; }

  // Get/set parent node (NULL if this is the root).
  ParseTreeNode* parent() const { return parent_; }
  void set_parent(ParseTreeNode* parent) { parent_ = parent; }

  // Get children nodes.
  const std::vector<class ParseTreeNode*> &children() const {
    return children_;
  }

  // Append a child node.
  void AddChild(class ParseTreeNode *node) {
    children_.push_back(node);
  }

  // Remove all children nodes. Note: this does not destroy the children.
  void RemoveAllChildren() {
    children_.clear();
  }

  // Return all the descendent nodes in a post order traversal.
  void GetPostOrderTraversal(std::vector<ParseTreeNode*> *nodes) {
    nodes->clear();
    for (int i = 0; i < children_.size(); ++i) {
      std::vector<ParseTreeNode*> children_nodes;
      children_[i]->GetPostOrderTraversal(&children_nodes);
      nodes->insert(nodes->end(), children_nodes.begin(), children_nodes.end());
    }
    nodes->push_back(this);
  }

  // Extract words and tags spanned by this node.
  void ExtractWordsAndTags(std::vector<std::string> *words,
                           std::vector<std::string> *tags) const;

  // Compute the span positions underneath this node, recursively.
  // "start" is the start point of the span.
  void ComputeSpans(int start);

  // Print subtree rooted in this node.
  std::string ToString() const;

  // Merge singleton spines with the same label into a single node.
  // E.g. "(NP (NP ... ))" becomes (NP ...).
  // If same_label_only is false, merge all singleton spines and keep the
  // last label; and if append_labels is true, create a new label which
  // concatenates all the labels (e.g. S|NP|PRP).
  void CollapseSingletonSpines(bool same_label_only, bool append_labels);

 protected:
  std::string label_;
  Span *span_;
  class ParseTreeNode *parent_;
  std::vector<class ParseTreeNode*> children_;
};

class ParseTree {
 public:
  ParseTree() { root_ = NULL; }
  virtual ~ParseTree() { DeleteAllNodes(); }

 public:
  void DeleteAllNodes() {
    delete root_;
    terminals_.clear();
    non_terminals_.clear();
  }

  ParseTreeNode *GetRoot() const {
    return root_;
  }

  const std::vector<ParseTreeNode*> &terminals() const {
    return terminals_;
  }
  const std::vector<ParseTreeNode*> &non_terminals() const {
    return non_terminals_;
  }

  void FillNodes() {
    terminals_.clear();
    non_terminals_.clear();
    std::vector<ParseTreeNode*> nodes;
    root_->GetPostOrderTraversal(&nodes);
    for (int i = 0; i < nodes.size(); ++i) {
      if (nodes[i]->IsLeaf()) {
        terminals_.push_back(nodes[i]);
      } else {
        non_terminals_.push_back(nodes[i]);
      }
    }
  }

  void LoadFromString(const std::string &info);
  void SaveToString(std::string *info) const { *info = GetRoot()->ToString(); }
  void ExtractWordsAndTags(std::vector<std::string> *words,
                           std::vector<std::string> *tags) const {
    GetRoot()->ExtractWordsAndTags(words, tags);
  }

  // Merge singleton spines with the same label into a single node.
  // E.g. "(NP (NP ... ))" becomes (NP ...).
  // If same_label_only is false, merge all singleton spines and keep the
  // last label; and if append_labels is true, create a new label which
  // concatenates all the labels (e.g. S|NP|PRP).
  void CollapseSingletonSpines(bool same_label_only, bool append_labels) {
    root_->CollapseSingletonSpines(same_label_only, append_labels);
    //ComputeSpans();
    FillNodes();
  }

 protected:
  ParseTreeNode* root_;
  std::vector<ParseTreeNode*> terminals_; // Words.
  std::vector<ParseTreeNode*> non_terminals_; // Tags and internal nodes.
};

#endif /* PARSETREE_H_ */

