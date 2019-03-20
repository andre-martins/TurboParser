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

#ifndef MORPHOLOGICALDICTIONARY_H_
#define MORPHOLOGICALDICTIONARY_H_

#include "SequenceDictionary.h"
#include "TokenDictionary.h"
#include "MorphologicalReader.h"

class MorphologicalDictionary : public SequenceDictionary {
public:
  MorphologicalDictionary() {}
  MorphologicalDictionary(Pipe* pipe) : SequenceDictionary(pipe) {}
  virtual ~MorphologicalDictionary() {}

  void Clear() {
    SequenceDictionary::Clear();
    cpostag_morphologicaltags_.clear();
    tags_from_lexicon_.clear();
    morphologicaltags_from_lexicon_.clear();
  }

  void Save(FILE *fs) {
    SequenceDictionary::Save(fs);
    // Save the lexicon (optional).
    lexicon_.Save(fs);

    bool success;
    int length = unknown_cpostag_morphologicaltags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int j = 0; j < unknown_cpostag_morphologicaltags_.size(); ++j) {
      int tag = unknown_cpostag_morphologicaltags_[j];
      success = WriteInteger(fs, tag);
      CHECK(success);
    }

    length = cpostag_morphologicaltags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < cpostag_morphologicaltags_.size(); ++i) {
      length = cpostag_morphologicaltags_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < cpostag_morphologicaltags_[i].size(); ++j) {
        int tag = cpostag_morphologicaltags_[i][j];
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

    length = morphologicaltags_from_lexicon_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < morphologicaltags_from_lexicon_.size(); ++i) {
      int tag = morphologicaltags_from_lexicon_[i];
      success = WriteInteger(fs, tag);
      CHECK(success);
    }
  }

  void Load(FILE *fs) {
    SequenceDictionary::Load(fs);

    // Load the lexicon (optional).
    lexicon_.Load(fs);

    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    unknown_cpostag_morphologicaltags_.resize(length);
    for (int j = 0; j < unknown_cpostag_morphologicaltags_.size(); ++j) {
      int tag;
      success = ReadInteger(fs, &tag);
      CHECK(success);
      unknown_cpostag_morphologicaltags_[j] = tag;
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    cpostag_morphologicaltags_.resize(length);
    for (int i = 0; i < cpostag_morphologicaltags_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      cpostag_morphologicaltags_[i].resize(length);
      for (int j = 0; j < cpostag_morphologicaltags_[i].size(); ++j) {
        int tag;
        success = ReadInteger(fs, &tag);
        CHECK(success);
        cpostag_morphologicaltags_[i][j] = tag;
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

    success = ReadInteger(fs, &length);
    CHECK(success);
    morphologicaltags_from_lexicon_.clear();
    morphologicaltags_from_lexicon_.resize(length);
    for (int i = 0; i < morphologicaltags_from_lexicon_.size(); ++i) {
      int tag;
      success = ReadInteger(fs, &tag);
      CHECK(success);
      morphologicaltags_from_lexicon_[i] = tag;
    }
  }

  void CreateTagDictionary(MorphologicalReader *reader);

#if 0
  const std::vector<int> &GetAllowedMorphologicalTags(int cpostag) {
    // return cpostag_morphologicaltags_[cpostag];
    // TODO: Not sure is this should be done here...
    // It may be cleaner to return an empty vector here and
    // fill it with the unknown tags elsewhere.
    if (!cpostag_morphologicaltags_[cpostag].empty()) {
      return cpostag_morphologicaltags_[cpostag];
    } else {
      return unknown_cpostag_morphologicaltags_;
    }
  }
#endif

  void GetAllowedMorphologicalTags(int cpostag, int lexicon_word,
                                   std::vector<int> *morphological_tags) {
    // The second argument allows returning allowed tags for words that
    // exist in the lexicon but do not occur in the corpus.
    //*morphological_tags = cpostag_morphologicaltags_[cpostag];
    std::set<int> morphological_tag_set(morphological_tags->begin(),
                                        morphological_tags->end());
    if (lexicon_word >= 0) {
      std::vector<int> lexicon_tags;
      std::vector<int> lexicon_morphological_tags;
      GetLexicon().GetWordMorphologicalTags(lexicon_word, &lexicon_tags,
                                            &lexicon_morphological_tags);
      int k = 0;
      for (auto lexicon_tag: lexicon_tags) {
        int tag = tags_from_lexicon_[lexicon_tag];
        if (tag == cpostag) {
          int lexicon_morphological_tag = lexicon_morphological_tags[k];
          int morphological_tag =
            morphologicaltags_from_lexicon_[lexicon_morphological_tag];
          if (morphological_tag_set.
              find(morphological_tag) == morphological_tag_set.end()) {
            morphological_tag_set.insert(morphological_tag);
            morphological_tags->push_back(morphological_tag);
          }
        }
        ++k;
      }
    } else {
      *morphological_tags = cpostag_morphologicaltags_[cpostag];
    }
    if (morphological_tags->empty()) {
      *morphological_tags = unknown_cpostag_morphologicaltags_;
    }
  }

protected:
  std::vector<std::vector<int> > cpostag_morphologicaltags_;
  std::vector<int> unknown_cpostag_morphologicaltags_;
  std::vector<int> tags_from_lexicon_;
  std::vector<int> morphologicaltags_from_lexicon_;
};

class MorphologicalTokenDictionary : public TokenDictionary {
public:
  MorphologicalTokenDictionary() {};
  virtual ~MorphologicalTokenDictionary() {};
  void Initialize(MorphologicalReader *reader);
};
#endif /* MORPHOLOGICALDICTIONARY_H_ */
