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

#ifndef SEMANTICPREDICATE_H_
#define SEMANTICPREDICATE_H_

#include "SerializationUtils.h"

class SemanticPredicate {
 public:
  SemanticPredicate() {}
  SemanticPredicate(int id) { id_ = id; }
  virtual ~SemanticPredicate() { roles_.clear(); }

 public:
  int id() const { return id_; }
  const std::vector<int> &GetRoles() const {
    return roles_;
  }

  bool HasRole(int role) const {
    for (int i = 0; i < roles_.size(); ++i) {
      if (roles_[i] == role) return true;
    }
    return false;
  }

  void InsertRole(int role) {
    CHECK(!HasRole(role)) << "Role existed already.";
    roles_.push_back(role);
  }

  void Save(FILE *fs) {
    bool success;
    int length = roles_.size();
    success = WriteInteger(fs, id_);
    CHECK(success);
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < roles_.size(); ++i) {
      int label = roles_[i];
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
    roles_.resize(length);
    for (int i = 0; i < roles_.size(); ++i) {
      int label;
      success = ReadInteger(fs, &label);
      CHECK(success);
      roles_[i] = label;
    }
  }

 protected:
  int id_;
  std::vector<int> roles_;
};

#endif /* SEMANTICPREDICATE_H_ */
