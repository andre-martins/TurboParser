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

#ifndef LEXICON_H_
#define LEXICON_H_

#include "Alphabet.h"
#include "SerializationUtils.h"
#include <set>
#include <tuple>
#include <iostream>

class Lexicon {
public:
  Lexicon() {}
  virtual ~Lexicon() {}

  void Save(FILE *fs) {
    if (0 > word_alphabet_.Save(fs)) CHECK(false);
    if (0 > lemma_alphabet_.Save(fs)) CHECK(false);
    if (0 > tag_alphabet_.Save(fs)) CHECK(false);
    if (0 > morph_alphabet_.Save(fs)) CHECK(false);

    bool success;
    int length = lexicon_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (auto entries: lexicon_) {
      length = entries.size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (auto entry: entries) {
        int lemma_id = std::get<0>(entry);
        int tag_id = std::get<1>(entry);
        int morph_id = std::get<2>(entry);
        success = WriteInteger(fs, lemma_id);
        CHECK(success);
        success = WriteInteger(fs, tag_id);
        CHECK(success);
        success = WriteInteger(fs, morph_id);
        CHECK(success);
      }
    }
  }

  void Load(FILE *fs) {
    word_alphabet_.clear();
    word_alphabet_.AllowGrowth();
    lemma_alphabet_.clear();
    lemma_alphabet_.AllowGrowth();
    tag_alphabet_.clear();
    tag_alphabet_.AllowGrowth();
    morph_alphabet_.clear();
    morph_alphabet_.AllowGrowth();

    if (0 > word_alphabet_.Load(fs)) CHECK(false);
    if (0 > lemma_alphabet_.Load(fs)) CHECK(false);
    if (0 > tag_alphabet_.Load(fs)) CHECK(false);
    if (0 > morph_alphabet_.Load(fs)) CHECK(false);

    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    lexicon_.resize(length);
    for (int i = 0; i < length; ++i) {
      int num_entries;
      success = ReadInteger(fs, &num_entries);
      CHECK(success);
      lexicon_[i].resize(num_entries);
      for (int j = 0; j < num_entries; ++j) {
        int lemma_id, tag_id, morph_id;
        success = ReadInteger(fs, &lemma_id);
        CHECK(success);
        success = ReadInteger(fs, &tag_id);
        CHECK(success);
        success = ReadInteger(fs, &morph_id);
        CHECK(success);
        lexicon_[i][j] = std::make_tuple(lemma_id, tag_id, morph_id);
      }
    }

    word_alphabet_.StopGrowth();
    lemma_alphabet_.StopGrowth();
    tag_alphabet_.StopGrowth();
    morph_alphabet_.StopGrowth();

    word_alphabet_.BuildNames();
    lemma_alphabet_.BuildNames();
    tag_alphabet_.BuildNames();
    morph_alphabet_.BuildNames();
  }

  void ReadFromTextFile(const std::string &filepath) {
    word_alphabet_.clear();
    word_alphabet_.AllowGrowth();
    lemma_alphabet_.clear();
    lemma_alphabet_.AllowGrowth();
    tag_alphabet_.clear();
    tag_alphabet_.AllowGrowth();
    morph_alphabet_.clear();
    morph_alphabet_.AllowGrowth();

    LOG(INFO) << "Loading lexicon file...";
    std::ifstream is;
    is.open(filepath.c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open " << filepath << ".";
    std::string line;
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line.size() == 0) continue;
        std::vector<std::string> fields;
        StringSplit(line, "\t", &fields, true);
        CHECK_EQ(fields.size(), 4);
        const std::string &word = fields[0];
        const std::string &lemma = fields[1];
        const std::string &tag = fields[2];
        const std::string &morph = fields[3];
        int word_id = word_alphabet_.Insert(word);
        int lemma_id = lemma_alphabet_.Insert(lemma);
        int tag_id = tag_alphabet_.Insert(tag);
        int morph_id = morph_alphabet_.Insert(morph);
        if (lexicon_.size() <= word_id) {
          lexicon_.resize(word_id+1);
        }
        lexicon_[word_id].
          push_back(std::make_tuple(lemma_id, tag_id, morph_id));
      }
    }
    LOG(INFO) << "Loaded lexicon with "
              << word_alphabet_.size() << " words.";

    word_alphabet_.StopGrowth();
    lemma_alphabet_.StopGrowth();
    tag_alphabet_.StopGrowth();
    morph_alphabet_.StopGrowth();

    word_alphabet_.BuildNames();
    lemma_alphabet_.BuildNames();
    tag_alphabet_.BuildNames();
    morph_alphabet_.BuildNames();
  }

  const Alphabet &GetWordAlphabet() const { return word_alphabet_; }
  const Alphabet &GetLemmaAlphabet() const { return lemma_alphabet_; }
  const Alphabet &GetTagAlphabet() const { return tag_alphabet_; }
  const Alphabet &GetMorphAlphabet() const { return morph_alphabet_; }

  int GetWordId(const std::string &word) const {
    return word_alphabet_.Lookup(word);
  }

  void GetWordTags(std::string &word, std::vector<std::string> *tags) const {
    tags->clear();
    int word_id = word_alphabet_.Lookup(word);
    if (word_id < 0) return;
    CHECK_LT(word_id, word_alphabet_.size());
    std::set<int> tag_ids;
    for (auto entry: lexicon_[word_id]) {
      int tag_id = std::get<1>(entry);
      CHECK_GE(tag_id, 0);
      CHECK_LT(tag_id, tag_alphabet_.size());
      if (tag_ids.find(tag_id) == tag_ids.end()) {
        tag_ids.insert(tag_id);
        const std::string &tag = tag_alphabet_.GetName(tag_id);
        tags->push_back(tag);
      }
    }
  }

  void GetWordTags(int word, std::vector<int> *tags) const {
    tags->clear();
    std::set<int> tag_ids;
    for (auto entry: lexicon_[word]) {
      int tag_id = std::get<1>(entry);
      CHECK_GE(tag_id, 0);
      CHECK_LT(tag_id, tag_alphabet_.size());
      if (tag_ids.find(tag_id) == tag_ids.end()) {
        tag_ids.insert(tag_id);
        tags->push_back(tag_id);
      }
    }
  }

 protected:
  Alphabet word_alphabet_;
  Alphabet lemma_alphabet_;
  Alphabet tag_alphabet_;
  Alphabet morph_alphabet_;
  std::vector<std::vector<std::tuple<int, int, int> > > lexicon_;
};

#endif /* LEXICON_H_ */
