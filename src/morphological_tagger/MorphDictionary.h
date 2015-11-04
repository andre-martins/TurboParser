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

#ifndef MORPHDICTIONARY_H_
#define MORPHDICTIONARY_H_

#include "SequenceDictionary.h"
#include "TokenDictionary.h"
#include "MorphReader.h"

class MorphDictionary : public SequenceDictionary {
public:
  MorphDictionary() {}
  MorphDictionary(Pipe* pipe) : SequenceDictionary(pipe) {}
  virtual ~MorphDictionary() {}

  void Clear() {
    SequenceDictionary::Clear();
    cpostag_morphtags_.clear();
  }

  void Save(FILE *fs) {
    SequenceDictionary::Save(fs);
    bool success;
    int length = unknown_cpostag_morphtags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int j = 0; j < unknown_cpostag_morphtags_.size(); ++j) {
      int tag = unknown_cpostag_morphtags_[j];
      success = WriteInteger(fs, tag);
      CHECK(success);
    }

    length = cpostag_morphtags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < cpostag_morphtags_.size(); ++i) {
      length = cpostag_morphtags_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < cpostag_morphtags_[i].size(); ++j) {
        int tag = cpostag_morphtags_[i][j];
        success = WriteInteger(fs, tag);
        CHECK(success);
      }
    }
  }

  void Load(FILE *fs) {
    SequenceDictionary::Load(fs);
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    unknown_cpostag_morphtags_.resize(length);
    for (int j = 0; j < unknown_cpostag_morphtags_.size(); ++j) {
      int tag;
      success = ReadInteger(fs, &tag);
      CHECK(success);
      unknown_cpostag_morphtags_[j] = tag;
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    cpostag_morphtags_.resize(length);
    for (int i = 0; i < cpostag_morphtags_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      cpostag_morphtags_[i].resize(length);
      for (int j = 0; j < cpostag_morphtags_[i].size(); ++j) {
        int tag;
        success = ReadInteger(fs, &tag);
        CHECK(success);
        cpostag_morphtags_[i][j] = tag;
      }
    }
  }

  void CreateTagDictionary(MorphReader *reader);

  const vector<int> &GetCpostagMorphtags(int cpostag) {
    // return cpostag_morphtags_[cpostag];
    // TODO: Not sure is this should be done here...
    // It may be cleaner to return an empty vector here and 
    // fill it with the unknown tags elsewhere.
    if (!cpostag_morphtags_[cpostag].empty()) {
      return cpostag_morphtags_[cpostag];
    }
    else {
      return unknown_cpostag_morphtags_;
    }
  }

protected:
  vector<vector<int> > cpostag_morphtags_;
  vector<int> unknown_cpostag_morphtags_;
};



class MorphTokenDictionary : public TokenDictionary {
public:
  MorphTokenDictionary() {};
  virtual ~MorphTokenDictionary() {};
  void InitializeFromMorphReader(MorphReader *reader);
};
#endif /* MORPHDICTIONARY_H_ */
