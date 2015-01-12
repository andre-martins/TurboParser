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

#include "ParseTree.h"
#include <stack>
#include <glog/logging.h>

const char kLeftBracket = '(';
const char kRightBracket = ')';
const char kWhitespace = ' ';

void ParseTreeNode::ExtractWordsAndTags(std::vector<std::string> *words,
                                        std::vector<std::string> *tags) const {
  words->clear();
  tags->clear();
  if (IsPreTerminal()) {
    tags->push_back(label_);
    words->push_back(GetChild(0)->label());
  } else {
    for (int i = 0; i < children_.size(); ++i) {
      std::vector<std::string> child_words;
      std::vector<std::string> child_tags;
      GetChild(i)->ExtractWordsAndTags(&child_words, &child_tags);
      words->insert(words->end(), child_words.begin(), child_words.end());
      tags->insert(tags->end(), child_tags.begin(), child_tags.end());
    }
  }
}

void ParseTreeNode::ComputeSpans(int start) {
  if (IsLeaf()) {
    span_ = Span(start, start);
  } else {
    int end = start-1;
    for (int i = 0; i < children_.size(); ++i) {
      GetChild(i)->ComputeSpans(end+1);
      end = GetChild(i)->end();
    }
    span_ = Span(start, end);
  }
}

void ParseTreeNode::CollapseSingletonSpines(bool same_label_only,
                                            bool append_labels) {
  for (int i = 0; i < children_.size(); ++i) {
    GetChild(i)->CollapseSingletonSpines(same_label_only, append_labels);
  }
  if (children_.size() == 1 && !IsPreTerminal()) {
    if (same_label_only) {
      if (label_ == GetChild(0)->label()) {
        std::vector<TreeNode<std::string> *> children =
          children_[0]->children();
        children_[0]->RemoveAllChildren();
        delete children_[0];
        children_ = children;
        for (int i = 0; i < children_.size(); ++i) {
          children_[i]->set_parent(this);
        }
      }
    } else {
      if (label_ != "" && append_labels) {
        label_ += kParseTreeLabelSeparator;
        label_ += GetChild(0)->label();
      } else {
        label_ = GetChild(0)->label();
      }
      std::vector<TreeNode<std::string> *> children = children_[0]->children();
      GetChild(0)->RemoveAllChildren();
      delete children_[0];
      children_ = children;
      for (int i = 0; i < children_.size(); ++i) {
        children_[i]->set_parent(this);
      }
    }
  }
}

std::string ParseTreeNode::ToString() const {
  std::string info = "";
  if (!IsLeaf()) info += kLeftBracket;
  info += label_;
  if (!IsLeaf()) {
    for (int i = 0; i < children_.size(); ++i) {
      if (i > 0 || label_ != "") info += kWhitespace;
      info += GetChild(i)->ToString();
    }
    info += kRightBracket;
  }
  return info;
}

void ParseTree::LoadFromString(const std::string &info) {
  DeleteAllNodes();
  root_ = new ParseTreeNode(NULL);
  std::string name = "";
  std::stack<ParseTreeNode*> node_stack;
  ParseTreeNode *node = NULL;
  for (int j = 0; j < info.length(); ++j) {
    const char &ch = info[j];
    if (ch == kLeftBracket) {
      name = "";
      if (node_stack.size() > 0) {
        node = new ParseTreeNode(node_stack.top());
        node_stack.top()->AddChild(node);
      } else {
        node = root_;
      }
      node_stack.push(node);
    } else if (ch == kRightBracket) {
      node = node_stack.top();
      node_stack.pop();
    } else if (ch == kWhitespace) {
      continue;
    } else {
      name += ch;
      CHECK_LT(j+1, info.length());
      CHECK_NE(info[j+1], kLeftBracket);
      if (info[j+1] == kRightBracket) {
        // Finished a terminal node.
        node = new ParseTreeNode(node_stack.top());
        node->set_label(name);
        node_stack.top()->AddChild(node);
      } else if (info[j+1] == kWhitespace) {
        // Just started a non-terminal or a pre-terminal node.
        node_stack.top()->set_label(name);
        name = "";
      }
    }
  }

  CHECK_EQ(node_stack.size(), 0);

  root_->ComputeSpans(0);
  FillNodes();
}
