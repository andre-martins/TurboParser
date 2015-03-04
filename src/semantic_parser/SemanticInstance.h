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

#ifndef SEMANTICINSTANCE_H_
#define SEMANTICINSTANCE_H_

#include <string>
#include <vector>
#include "DependencyInstance.h"
#include <iostream>

class SemanticInstance : public DependencyInstance {
 public:
  SemanticInstance() {};
  virtual ~SemanticInstance() {};

  Instance* Copy() {
    SemanticInstance* instance = new SemanticInstance();
    instance->Initialize(name_, forms_, lemmas_, cpostags_, postags_,
                         feats_, deprels_, heads_,
                         predicate_names_, predicate_indices_,
                         argument_roles_, argument_indices_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const string &name,
                  const vector<string> &forms,
                  const vector<string> &lemmas,
                  const vector<string> &cpos,
                  const vector<string> &pos,
                  const vector<vector<string> > &feats,
                  const vector<string> &deprels,
                  const vector<int> &heads,
                  const vector<string> &predicate_names,
                  const vector<int> &predicate_indices,
                  const vector<vector<string> > &argument_roles,
                  const vector<vector<int> > &argument_indices);

  const string &GetName() { return name_; }
  int GetNumPredicates() { return predicate_names_.size(); }
  const string &GetPredicateName(int k) { return predicate_names_[k]; }
  int GetPredicateIndex(int k) { return predicate_indices_[k]; }
  int GetNumArgumentsPredicate(int k) { return argument_roles_[k].size(); }
  const string &GetArgumentRole(int k, int l) { return argument_roles_[k][l]; }
  int GetArgumentIndex(int k, int l) { return argument_indices_[k][l]; }

  void ClearPredicates() {
    predicate_names_.clear();
    predicate_indices_.clear();
    for (int p = 0; p < argument_roles_.size(); ++p) {
      argument_indices_[p].clear();
      argument_roles_[p].clear();
    }
    argument_indices_.clear();
    argument_roles_.clear();
  }

  void AddPredicate(const string& predicate_name,
                    int predicate_index,
                    const vector<string> &argument_roles,
                    const vector<int> &argument_indices) {
    predicate_names_.push_back(predicate_name);
    predicate_indices_.push_back(predicate_index);
    argument_roles_.push_back(argument_roles);
    argument_indices_.push_back(argument_indices);
  }

 protected:
  // Name of the sentence (e.g. "#2000001").
  string name_;
  // Names of the predicates (e.g. "take.01").
  vector<string> predicate_names_;
  // Positions of each predicate in the sentence.
  vector<int> predicate_indices_;
  // Labels of each predicate's arguments (semantic roles).
  vector<vector<string> > argument_roles_;
  // Positions of each predicate's arguments.
  vector<vector<int> > argument_indices_;
};

#endif /* SEMANTICINSTANCE_H_*/
