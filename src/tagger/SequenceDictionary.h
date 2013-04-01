// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.0.
//
// TurboParser 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.0.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SEQUENCEDICTIONARY_H_
#define SEQUENCEDICTIONARY_H_

#include "Dictionary.h"
#include "TokenDictionary.h"
#include "SerializationUtils.h"

class Pipe;

class SequenceDictionary : public Dictionary {
 public:
  SequenceDictionary() {}
  SequenceDictionary(Pipe* pipe) : pipe_(pipe) {}
  virtual ~SequenceDictionary() { Clear(); }

  void Clear() {
    if (token_dictionary_) token_dictionary_->Clear();
    tag_alphabet_.clear();
    word_tags_.clear();
  }

  void Save(FILE *fs) {
    if (0 > tag_alphabet_.Save(fs)) CHECK(false);
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
  }

  void Load(FILE *fs) {
    if (0 > tag_alphabet_.Load(fs)) CHECK(false);
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

    tag_alphabet_.BuildNames();
  }

  void AllowGrowth() { token_dictionary_->AllowGrowth(); }
  void StopGrowth() { token_dictionary_->StopGrowth(); }

  void CreateTagDictionary(SequenceReader *reader);

  void BuildTagNames() {
    tag_alphabet_.BuildNames();
  }

  const string &GetTagName(int tag) const {
    return tag_alphabet_.GetName(tag);
  }

  int GetBigramLabel(int left_tag, int tag) {
    CHECK_GE(left_tag, -1);
    CHECK_GE(tag, -1);
    //return (left_tag * tag_alphabet_.size() +  tag);
    return ((1 + left_tag) * (1 + tag_alphabet_.size()) +  (1 + tag));
  }

  int GetTrigramLabel(int left_left_tag, int left_tag, int tag) {
    CHECK_GE(left_left_tag, -1);
    CHECK_GE(left_tag, -1);
    CHECK_GE(tag, -1);
    //return (left_tag * left_tag * tag_alphabet_.size() + 
    //        left_tag * tag_alphabet_.size() +  tag);
    return ((1 + left_left_tag) * (1 + tag_alphabet_.size()) *
            (1 + tag_alphabet_.size()) + 
            (1 + left_tag) * (1 + tag_alphabet_.size()) + (1 + tag));
  }

  const vector<int> &GetWordTags(int word) {
    // return word_tags_[word];
    // TODO: Not sure is this should be done here...
    // It may be cleaner to return an empty vector here and 
    // fill it with the unknown tags elsewhere.
    if (!word_tags_[word].empty()) {
      return word_tags_[word];
    } else {
      return unknown_word_tags_;
    }
  }

  TokenDictionary *GetTokenDictionary() const { return token_dictionary_; }
  void SetTokenDictionary(TokenDictionary *token_dictionary) {
    token_dictionary_ = token_dictionary;
  }

  const Alphabet &GetTagAlphabet() const { return tag_alphabet_; };

 protected:
  Pipe *pipe_;
  TokenDictionary *token_dictionary_;
  Alphabet tag_alphabet_;
  vector<vector<int> > word_tags_;
  vector<int> unknown_word_tags_;
};

#endif /* SEQUENCEDICTIONARY_H_ */
