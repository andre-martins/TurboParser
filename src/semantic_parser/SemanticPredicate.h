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

#ifndef SEMANTICPREDICATE_H_
#define SEMANTICPREDICATE_H_

#include "SerializationUtils.h"
#include <set>

class SemanticPredicate {
public:
  SemanticPredicate() {}
  SemanticPredicate(int id) { id_ = id; }
  virtual ~SemanticPredicate() { roles_.clear(); }

public:
  int id() const { return id_; }
  const std::set<int> &GetRoles() const {
    return roles_;
  }

  bool HasRole(int role) const {
    std::set<int>::iterator it = roles_.find(role);
    return (it != roles_.end());
  }

  void InsertRole(int role) {
    CHECK(!HasRole(role)) << "Role existed already.";
    roles_.insert(role);
  }

  void Save(FILE *fs) {
    bool success;
    int length = roles_.size();
    success = WriteInteger(fs, id_);
    CHECK(success);
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = roles_.begin();
    it != roles_.end(); ++it) {
      int label = *it;
      success = WriteInteger(fs, label);
      CHECK(success);
    }
  }

  void Load(FILE *fs) {
    bool success;
    int length;
    success = ReadInteger(fs, &id_);
    CHECK(success);
    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int label;
      success = ReadInteger(fs, &label);
      CHECK(success);
      InsertRole(label);
    }
  }

protected:
  int id_;
  std::set<int> roles_;
};

#endif /* SEMANTICPREDICATE_H_ */
