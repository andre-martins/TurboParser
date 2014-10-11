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

#ifndef ENTITYDICTIONARY_H_
#define ENTITYDICTIONARY_H_

#include "SequenceDictionary.h"

class EntityDictionary : public SequenceDictionary {
 public:
  EntityDictionary() {}
  EntityDictionary(Pipe* pipe) : SequenceDictionary(pipe) {}
  virtual ~EntityDictionary() {}

  void Clear() {
    SequenceDictionary::Clear();
    //word_tags_.clear();
  }

  void Save(FILE *fs) {
    SequenceDictionary::Save(fs);

    bool success;
    int length = allowed_bigrams_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int j = 0; j < allowed_bigrams_.size(); ++j) {
      length = allowed_bigrams_[j].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int k = 0; k < allowed_bigrams_[j].size(); ++k) {
        bool allowed = allowed_bigrams_[j][k];
        success = WriteBool(fs, allowed);
        CHECK(success);
      }
    }

    /*
    bool success;
    int length = unknown_word_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int j = 0; j < unknown_word_tags_.size(); ++j) {
      int tag = unknown_word_tags_[j];
      success = WriteInteger(fs, tag);
      CHECK(success);
    }

    length = word_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < word_tags_.size(); ++i) {
      length = word_tags_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < word_tags_[i].size(); ++j) {
        int tag = word_tags_[i][j];
        success = WriteInteger(fs, tag);
        CHECK(success);
      }
    }
    */
  }

  void Load(FILE *fs) {
    SequenceDictionary::Load(fs);

    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    allowed_bigrams_.resize(length);
    for (int j = 0; j < allowed_bigrams_.size(); ++j) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      allowed_bigrams_[j].resize(length);
      for (int k = 0; k < allowed_bigrams_[j].size(); ++k) {
        bool allowed;
        success = ReadBool(fs, &allowed);
        CHECK(success);
        allowed_bigrams_[j][k] = allowed;
      }
    }

    /*
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    unknown_word_tags_.resize(length);
    for (int j = 0; j < unknown_word_tags_.size(); ++j) {
      int tag;
      success = ReadInteger(fs, &tag);
      CHECK(success);
      unknown_word_tags_[j] = tag;
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    word_tags_.resize(length);
    for (int i = 0; i < word_tags_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      word_tags_[i].resize(length);
      for (int j = 0; j < word_tags_[i].size(); ++j) {
        int tag;
        success = ReadInteger(fs, &tag);
        CHECK(success);
        word_tags_[i][j] = tag;
      }
    }
    */
  }

  void CreateTagDictionary(SequenceReader *reader);

  void ReadGazetteerFiles();

  bool IsAllowedBigram(int left_tag, int tag) {
    CHECK_GE(left_tag, -1);
    CHECK_GE(tag, -1);
    return allowed_bigrams_[tag + 1][left_tag + 1];
  }

 protected:
  std::vector<std::vector<bool> > allowed_bigrams_;
  Alphabet gazetteer_word_alphabet_;
  Alphabet gazetteer_entity_tag_alphabet_;
  std::vector<std::vector<int> > gazetteer_word_entity_tags_;
  //vector<vector<int> > word_tags_;
  //vector<int> unknown_word_tags_;
};

#endif /* ENTITYDICTIONARY_H_ */
