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

#ifndef SEMANTICDICTIONARY_H_
#define SEMANTICDICTIONARY_H_

#include "Dictionary.h"
#include "TokenDictionary.h"
#include "SerializationUtils.h"

class Pipe;

class SemanticDictionary : public Dictionary {
 public:
  SemanticDictionary() { token_dictionary_ = NULL; }
  SemanticDictionary(Pipe* pipe) : pipe_(pipe) {}
  virtual ~SemanticDictionary() {
    Clear();
  }

  void CreateRoleDictionary(SemanticReader *reader);

  void Clear() {
    // Don't clear token_dictionary, since this class does not own it.
    role_alphabet_.clear();
    existing_roles_.clear();
    maximum_left_distances_.clear();
    maximum_right_distances_.clear();
  }

  void BuildRoleNames() {
    role_alphabet_.BuildNames();
  }

  const string &GetRoleName(int role) const {
    return role_alphabet_.GetName(role);
  }

  void AllowGrowth() { token_dictionary_->AllowGrowth(); }
  void StopGrowth() { token_dictionary_->StopGrowth(); }

  void Save(FILE *fs) {
    if (0 > role_alphabet_.Save(fs)) CHECK(false);
    bool success;
    int length = existing_roles_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < existing_roles_.size(); ++i) {
      length = existing_roles_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < existing_roles_[i].size(); ++j) {
        length = existing_roles_[i][j].size();
        success = WriteInteger(fs, length);
        CHECK(success);
        for (int k = 0; k < existing_roles_[i][j].size(); ++k) {
          int label = existing_roles_[i][j][k];
          success = WriteInteger(fs, label);
          CHECK(success);
        }
        int distance;
        distance = maximum_left_distances_[i][j];
        success = WriteInteger(fs, distance);
        CHECK(success);
        distance = maximum_right_distances_[i][j];
        success = WriteInteger(fs, distance);
        CHECK(success);
      }
    }
  }

  void Load(FILE *fs) {
    if (0 > role_alphabet_.Load(fs)) CHECK(false);
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    existing_roles_.resize(length);
    maximum_left_distances_.resize(length);
    maximum_right_distances_.resize(length);
    for (int i = 0; i < existing_roles_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      existing_roles_[i].resize(length);
      maximum_left_distances_[i].resize(length);
      maximum_right_distances_[i].resize(length);
      for (int j = 0; j < existing_roles_[i].size(); ++j) {
        success = ReadInteger(fs, &length);
        CHECK(success);
        existing_roles_[i][j].resize(length);
        for (int k = 0; k < existing_roles_[i][j].size(); ++k) {
          int label;
          success = ReadInteger(fs, &label);
          CHECK(success);
          existing_roles_[i][j][k] = label;
        }
        int distance;
        success = ReadInteger(fs, &distance);
        CHECK(success);
        maximum_left_distances_[i][j] = distance;
        success = ReadInteger(fs, &distance);
        CHECK(success);
        maximum_right_distances_[i][j] = distance;
      }
    }
    BuildRoleNames();
  }

  TokenDictionary *GetTokenDictionary() const { return token_dictionary_; }
  void SetTokenDictionary(TokenDictionary *token_dictionary) {
    token_dictionary_ = token_dictionary;
    //CHECK(token_dictionary_ == NULL);
  }

  const vector<int> &GetExistingRoles(int predicate_pos_id, int argument_pos_id) {
    return existing_roles_[predicate_pos_id][argument_pos_id];
  }

  int GetMaximumLeftDistance(int predicate_pos_id, int argument_pos_id) {
    return maximum_left_distances_[predicate_pos_id][argument_pos_id];
  }

  int GetMaximumRightDistance(int predicate_pos_id, int argument_pos_id) {
    return maximum_right_distances_[predicate_pos_id][argument_pos_id];
  }

  const Alphabet &GetRoleAlphabet() const { return role_alphabet_; };


 protected:
  Pipe *pipe_;
  TokenDictionary *token_dictionary_;
  Alphabet role_alphabet_;
  vector<vector<vector<int> > > existing_roles_;
  vector<vector<int> > maximum_left_distances_;
  vector<vector<int> > maximum_right_distances_;
};

#endif /* SEMANTICDICTIONARY_H_ */

