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

#ifndef ENTITYDICTIONARY_H_
#define ENTITYDICTIONARY_H_

#include "SequenceDictionary.h"
#include "TokenDictionary.h"
#include "EntityReader.h"

class EntityDictionary : public SequenceDictionary {
 public:
  EntityDictionary() {}
  EntityDictionary(Pipe* pipe) : SequenceDictionary(pipe) {}
  virtual ~EntityDictionary() {}

  void Clear() {
    SequenceDictionary::Clear();

    gazetteer_word_alphabet_.clear();
    gazetteer_entity_tag_alphabet_.clear();
    gazetteer_word_entity_tags_.clear();
  }

  void Save(FILE *fs) {
    SequenceDictionary::Save(fs);

    if (0 > gazetteer_word_alphabet_.Save(fs)) CHECK(false);
    if (0 > gazetteer_entity_tag_alphabet_.Save(fs)) CHECK(false);

    bool success;
    int length = gazetteer_word_entity_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int j = 0; j < gazetteer_word_entity_tags_.size(); ++j) {
      length = gazetteer_word_entity_tags_[j].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int k = 0; k < gazetteer_word_entity_tags_[j].size(); ++k) {
        int id = gazetteer_word_entity_tags_[j][k];
        success = WriteInteger(fs, id);
        CHECK(success);
      }
    }

    length = allowed_bigrams_.size();
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
  }

  void Load(FILE *fs) {
    SequenceDictionary::Load(fs);

    if (0 > gazetteer_word_alphabet_.Load(fs)) CHECK(false);
    if (0 > gazetteer_entity_tag_alphabet_.Load(fs)) CHECK(false);

    int length;
    bool success = ReadInteger(fs, &length);
    CHECK(success);
    gazetteer_word_entity_tags_.resize(length);
    for (int j = 0; j < gazetteer_word_entity_tags_.size(); ++j) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      gazetteer_word_entity_tags_[j].resize(length);
      for (int k = 0; k < gazetteer_word_entity_tags_[j].size(); ++k) {
        int id;
        success = ReadInteger(fs, &id);
        CHECK(success);
        gazetteer_word_entity_tags_[j][k] = id;
      }
    }

    gazetteer_word_alphabet_.StopGrowth();
    gazetteer_entity_tag_alphabet_.StopGrowth();
    LOG(INFO) << "Number of gazetteer words: "
              << gazetteer_word_alphabet_.size();
    LOG(INFO) << "Number of gazetteer entity tags: "
              << gazetteer_entity_tag_alphabet_.size();

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
  }

  void CreateTagDictionary(SequenceReader *reader);

  void ReadGazetteerFiles();

  void GetWordGazetteerIds(const std::string &word,
                           std::vector<int> *gazetteer_ids) const {
    gazetteer_ids->clear();
    int id = gazetteer_word_alphabet_.Lookup(word);
    if (id >= 0) {
      gazetteer_ids->assign(gazetteer_word_entity_tags_[id].begin(),
                            gazetteer_word_entity_tags_[id].end());
    }
  }

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
};

class EntityTokenDictionary : public TokenDictionary {
public:
  EntityTokenDictionary() {};
  virtual ~EntityTokenDictionary() {};
  void InitializeFromEntityReader(EntityReader *reader);
};
#endif /* ENTITYDICTIONARY_H_ */
