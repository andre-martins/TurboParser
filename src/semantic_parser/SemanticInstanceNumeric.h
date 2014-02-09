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

#ifndef SEMANTICINSTANCENUMERIC_H_
#define SEMANTICINSTANCENUMERIC_H_

#include <vector>
#include <string>
#include "Dictionary.h"
#include "DependencyInstanceNumeric.h"
#include "SemanticInstance.h"
#include "SemanticDictionary.h"

using namespace std;

class SemanticInstanceNumeric : public DependencyInstanceNumeric {
public:
  SemanticInstanceNumeric() {};
  virtual ~SemanticInstanceNumeric() { Clear(); };

  int size() { return form_ids_.size(); };

  void Clear() {
    DependencyInstanceNumeric::Clear();
    predicate_ids_.clear();
    predicate_indices_.clear();
    for (int j = 0; j < argument_role_ids_.size(); ++j) {
      argument_role_ids_[j].clear();
    }
    argument_indices_.clear();
    for (int j = 0; j < argument_indices_.size(); ++j) {
      argument_indices_[j].clear();
    }
    argument_indices_.clear();
    DeleteIndices();
  }

  void Initialize(const SemanticDictionary &dictionary,
                  SemanticInstance *instance);

  void DeleteIndices() {
    index_predicates_.clear();
    for (int p = 0; p < index_arcs_.size(); ++p) {
      index_arcs_[p].clear();
    }
    index_arcs_.clear();
  }

  void BuildIndices() {
    DeleteIndices();
    int length = size();
    index_predicates_.resize(length, -1);
    index_arcs_.resize(length);
    for (int p = 0; p < index_arcs_.size(); ++p) {
      index_arcs_[p].resize(length, -1);
    }
    for (int k = 0; k < GetNumPredicates(); ++k) {
      int p = GetPredicateIndex(k);
      index_predicates_[p] = k;
      for (int l = 0; l < GetNumArgumentsPredicate(k); ++l) {
        int a = GetArgumentIndex(k, l);
        index_arcs_[p][a] = l;
      }
    }
  }

  const vector<int> &GetPredicateIds() const { return predicate_ids_; }
  const vector<int> &GetPredicateIndices() const { return predicate_indices_; }
  const vector<vector<int> > &GetArgumentRoleIds() const {
    return argument_role_ids_;
  }
  const vector<vector<int> > &GetArgumentIndices() const {
    return argument_indices_;
  }

  int GetNumPredicates() { return predicate_ids_.size(); }
  int GetPredicateId(int k) { return predicate_ids_[k]; }
  int GetPredicateIndex(int k) { return predicate_indices_[k]; }
  int GetNumArgumentsPredicate(int k) { return argument_role_ids_[k].size(); }
  int GetArgumentRoleId(int k, int l) { return argument_role_ids_[k][l]; }
  int GetArgumentIndex(int k, int l) { return argument_indices_[k][l]; }

  int FindPredicate(int p) { return index_predicates_[p]; }
  int FindArc(int p, int a) { return index_arcs_[p][a]; }

 private:
  vector<int> predicate_ids_;
  vector<int> predicate_indices_;
  vector<vector<int> > argument_role_ids_;
  vector<vector<int> > argument_indices_;

  vector<int> index_predicates_;
  vector<vector<int> > index_arcs_;
};

#endif /* SEMANTICINSTANCENUMERIC_H_ */
