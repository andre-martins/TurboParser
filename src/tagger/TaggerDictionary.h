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

#ifndef TAGGERDICTIONARY_H_
#define TAGGERDICTIONARY_H_

#include "SequenceDictionary.h"

class TaggerDictionary : public SequenceDictionary {
public:
  TaggerDictionary() {}
  TaggerDictionary(Pipe* pipe) : SequenceDictionary(pipe) {}
  virtual ~TaggerDictionary() {}

  void Clear() {
    SequenceDictionary::Clear();
    word_tags_.clear();
    tags_from_lexicon_.clear();
  }

  void Save(FILE *fs) {
    SequenceDictionary::Save(fs);
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

    length = tags_from_lexicon_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < tags_from_lexicon_.size(); ++i) {
      int tag = tags_from_lexicon_[i];
      success = WriteInteger(fs, tag);
      CHECK(success);
    }
  }

  void Load(FILE *fs) {
    SequenceDictionary::Load(fs);
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

    success = ReadInteger(fs, &length);
    CHECK(success);
    tags_from_lexicon_.clear();
    tags_from_lexicon_.resize(length);
    for (int i = 0; i < tags_from_lexicon_.size(); ++i) {
      int tag;
      success = ReadInteger(fs, &tag);
      CHECK(success);
      tags_from_lexicon_[i] = tag;
    }
  }

  void CreateTagDictionary(SequenceReader *reader);

  void GetWordTags(int word, int lexicon_word, std::vector<int> *tags) {
    // The second argument allows returning allowed tags for words that
    // exist in the lexicon but do not occur in the corpus.
    *tags = word_tags_[word];
    std::set<int> tag_set(tags->begin(), tags->end());
    if (lexicon_word >= 0) {
      std::vector<int> lexicon_tags;
      GetLexicon().GetWordTags(lexicon_word, &lexicon_tags);
      for (auto lexicon_tag: lexicon_tags) {
        int tag = tags_from_lexicon_[lexicon_tag];
        if (tag_set.find(tag) == tag_set.end()) {
          tag_set.insert(tag);
          tags->push_back(tag);
        }
      }
    }
    if (tags->empty()) {
      *tags = unknown_word_tags_;
    }
  }

protected:
  std::vector<std::vector<int> > word_tags_;
  std::vector<int> unknown_word_tags_;
  std::vector<int> tags_from_lexicon_;
};

#endif /* TAGGERDICTIONARY_H_ */
