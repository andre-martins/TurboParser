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

#ifndef CONSTITUENCYINSTANCE_H_
#define CONSTITUENCYINSTANCE_H_

#include <string>
#include <vector>
#include "SequenceInstance.h"
#include "ParseTree.h"

class ConstituencyInstance : public SequenceInstance {
 public:
  ConstituencyInstance() {}
  virtual ~ConstituencyInstance() {}

  virtual Instance* Copy() {
    ConstituencyInstance* instance = new ConstituencyInstance();
    instance->Initialize(forms_, lemmas_, tags_, morph_, parse_tree_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const std::vector<std::string> &forms,
                  const std::vector<std::string> &lemmas,
                  const std::vector<std::string> &tags,
                  const std::vector<std::vector<std::string> > &morph,
                  const ParseTree &parse_tree);

  const string &GetLemma(int i) { return lemmas_[i]; };
  int GetNumMorphFeatures(int i) { return morph_[i].size(); };
  const string &GetMorphFeature(int i, int j) { return morph_[i][j]; };

  const ParseTree &GetParseTree() const { return parse_tree_; }
  ParseTree *GetMutableParseTree() { return &parse_tree_; }

  void SetParseTree(const ParseTree &parse_tree) { parse_tree_ = parse_tree; }

 protected:
  std::vector<std::string> lemmas_;
  std::vector<std::vector<std::string> > morph_;
  ParseTree parse_tree_;
};

#endif /* CONSTITUENCYINSTANCE_H_*/
