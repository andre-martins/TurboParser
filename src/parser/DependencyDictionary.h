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

#ifndef DEPENDENCYDICTIONARY_H_
#define DEPENDENCYDICTIONARY_H_

#include "Dictionary.h"
#include "TokenDictionary.h"
#include "SerializationUtils.h"

class Pipe;

class DependencyDictionary : public Dictionary {
 public:
  DependencyDictionary() { token_dictionary_ = NULL; }
  DependencyDictionary(Pipe* pipe) : pipe_(pipe) {}
  virtual ~DependencyDictionary() {
    Clear();
  }

  void CreateLabelDictionary(DependencyReader *reader);

  void Clear() {
    // Don't clear token_dictionary, since this class does not own it.
    label_alphabet_.clear();
    existing_labels_.clear();
    maximum_left_distances_.clear();
    maximum_right_distances_.clear();
  }

  void BuildLabelNames() {
    label_alphabet_.BuildNames();
  }

  const string &GetLabelName(int label) const {
    return label_alphabet_.GetName(label);
  }

  void AllowGrowth() { token_dictionary_->AllowGrowth(); }
  void StopGrowth() { token_dictionary_->StopGrowth(); }

  void Save(FILE *fs) {
    if (0 > label_alphabet_.Save(fs)) CHECK(false);
    bool success;
    int length = existing_labels_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < existing_labels_.size(); ++i) {
      length = existing_labels_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < existing_labels_[i].size(); ++j) {
        length = existing_labels_[i][j].size();
        success = WriteInteger(fs, length);
        CHECK(success);
        for (int k = 0; k < existing_labels_[i][j].size(); ++k) {
          int label = existing_labels_[i][j][k];
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
    if (0 > label_alphabet_.Load(fs)) CHECK(false);
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    existing_labels_.resize(length);
    maximum_left_distances_.resize(length);
    maximum_right_distances_.resize(length);
    for (int i = 0; i < existing_labels_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      existing_labels_[i].resize(length);
      maximum_left_distances_[i].resize(length);
      maximum_right_distances_[i].resize(length);
      for (int j = 0; j < existing_labels_[i].size(); ++j) {
        success = ReadInteger(fs, &length);
        CHECK(success);
        existing_labels_[i][j].resize(length);
        for (int k = 0; k < existing_labels_[i][j].size(); ++k) {
          int label;
          success = ReadInteger(fs, &label);
          CHECK(success);
          existing_labels_[i][j][k] = label;
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
    BuildLabelNames();
  }

  TokenDictionary *GetTokenDictionary() const { return token_dictionary_; }
  void SetTokenDictionary(TokenDictionary *token_dictionary) {
    token_dictionary_ = token_dictionary; 
    //CHECK(token_dictionary_ == NULL);
  }

  const vector<int> &GetExistingLabels(int modifier_pos_id, int head_pos_id) {
    return existing_labels_[modifier_pos_id][head_pos_id];
  }

  int GetMaximumLeftDistance(int modifier_pos_id, int head_pos_id) {
    return maximum_left_distances_[modifier_pos_id][head_pos_id];
  }

  int GetMaximumRightDistance(int modifier_pos_id, int head_pos_id) {
    return maximum_right_distances_[modifier_pos_id][head_pos_id];
  }

  const Alphabet &GetLabelAlphabet() const { return label_alphabet_; };


 protected:
  Pipe *pipe_;
  TokenDictionary *token_dictionary_;
  Alphabet label_alphabet_;
  vector<vector<vector<int> > > existing_labels_;
  vector<vector<int> > maximum_left_distances_;
  vector<vector<int> > maximum_right_distances_;
};

#endif /* DEPENDENCYDICTIONARY_H_ */

