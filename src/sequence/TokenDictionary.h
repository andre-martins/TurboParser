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

#ifndef TOKENDICTIONARY_H_
#define TOKENDICTIONARY_H_

#include "Dictionary.h"
#include "Alphabet.h"
#include "SequenceReader.h"

DECLARE_int32(prefix_length);
DECLARE_int32(suffix_length);
DECLARE_bool(form_case_sensitive);

enum SpecialTokens {
  TOKEN_UNKNOWN = 0,
  TOKEN_START,
  TOKEN_STOP,
  NUM_SPECIAL_TOKENS
};

class Pipe;

class TokenDictionary : public Dictionary {
public:
  TokenDictionary() { pipe_ = NULL; }
  TokenDictionary(Pipe *pipe) : pipe_(pipe) {}
  virtual ~TokenDictionary() { Clear(); }

  void Load(FILE *fs);
  void Save(FILE *fs);

  void Clear() {
    form_alphabet_.clear();
    form_lower_alphabet_.clear();
    lemma_alphabet_.clear();
    prefix_alphabet_.clear();
    suffix_alphabet_.clear();
    feats_alphabet_.clear();
    pos_alphabet_.clear();
    cpos_alphabet_.clear();
    shape_alphabet_.clear();
  }

  void AllowGrowth() {
    form_alphabet_.AllowGrowth();
    form_lower_alphabet_.AllowGrowth();
    lemma_alphabet_.AllowGrowth();
    prefix_alphabet_.AllowGrowth();
    suffix_alphabet_.AllowGrowth();
    feats_alphabet_.AllowGrowth();
    pos_alphabet_.AllowGrowth();
    cpos_alphabet_.AllowGrowth();
    shape_alphabet_.AllowGrowth();
  }
  void StopGrowth() {
    form_alphabet_.StopGrowth();
    form_lower_alphabet_.StopGrowth();
    lemma_alphabet_.StopGrowth();
    prefix_alphabet_.StopGrowth();
    suffix_alphabet_.StopGrowth();
    feats_alphabet_.StopGrowth();
    pos_alphabet_.StopGrowth();
    cpos_alphabet_.StopGrowth();
    shape_alphabet_.StopGrowth();
  }

  int GetNumPosTags() const { return pos_alphabet_.size(); }
  int GetNumCoarsePosTags() const { return cpos_alphabet_.size(); }
  int GetNumForms() const { return form_alphabet_.size(); }
  int GetNumLemmas() const { return lemma_alphabet_.size(); }

  int GetFormId(const std::string &form) const {
    return form_alphabet_.Lookup(form);
  }
  int GetFormLowerId(const std::string &form_lower) const {
    return form_lower_alphabet_.Lookup(form_lower);
  }
  int GetLemmaId(const std::string &lemma) const {
    return lemma_alphabet_.Lookup(lemma);
  }
  int GetPrefixId(const std::string &prefix) const {
    return prefix_alphabet_.Lookup(prefix);
  }
  int GetSuffixId(const std::string &suffix) const {
    return suffix_alphabet_.Lookup(suffix);
  }
  int GetPosTagId(const std::string &pos) const {
    return pos_alphabet_.Lookup(pos);
  }
  int GetCoarsePosTagId(const std::string &cpos) const {
    return cpos_alphabet_.Lookup(cpos);
  }
  int GetMorphFeatureId(const std::string &feats) const {
    return feats_alphabet_.Lookup(feats);
  }
  int GetShapeId(const std::string &shape) const {
    return shape_alphabet_.Lookup(shape);
  }

  int GetNumFeatures() {
    CHECK(false) <<
      "There is no notion of number of features in TokenDictionary.";
  }

  void InitializeStarter();
  void InitializeFromSequenceReader(SequenceReader *reader);

  void BuildNames() {
    pos_alphabet_.BuildNames();
    form_alphabet_.BuildNames();
  }

  const std::string &GetPosTagName(int id) { return pos_alphabet_.GetName(id); }
  const std::string &GetFormName(int id) { return form_alphabet_.GetName(id); }

  void GetWordShape(const std::string &word, std::string *shape) {
    std::string type = "";
    char last = '\0';
    for (int i = 0; i < word.size(); ++i) {
      if (word[i] >= 'A' && word[i] <= 'Z') {
        if (last != 'A') {
          type += 'A';
          last = 'A';
        } else if (type[type.size() - 1] != '+') {
          type += '+';
        }
      } else if (word[i] >= 'a' && word[i] <= 'z') {
        if (last != 'a') {
          type += 'a';
          last = 'a';
        } else if (type[type.size() - 1] != '+') {
          type += '+';
        }
      } else if (word[i] >= '0' && word[i] <= '9') {
        if (last != '0') {
          type += '0';
          last = '0';
        } else if (type[type.size() - 1] != '+') {
          type += '+';
          last = '0';
        }
      } else {
        type += word[i];
        last = '\0';
      }
    }
    *shape = type;
  }

protected: //private:
  Pipe *pipe_;
  Alphabet form_alphabet_;
  Alphabet form_lower_alphabet_;
  Alphabet lemma_alphabet_;
  Alphabet prefix_alphabet_;
  Alphabet suffix_alphabet_;
  Alphabet feats_alphabet_;
  Alphabet pos_alphabet_;
  Alphabet cpos_alphabet_;
  Alphabet shape_alphabet_;

  int form_cutoff;
  int form_lower_cutoff;
  int lemma_cutoff;
  int feats_cutoff;
  int pos_cutoff;
  int cpos_cutoff;
  int shape_cutoff;
  int prefix_length;
  int suffix_length;
  bool form_case_sensitive;

  // Special symbols.
  const std::string kTokenUnknown = "_UNKNOWN_"; // Unknown word/lemma.
  const std::string kTokenStart = "_START_"; // Start symbol.
  const std::string kTokenStop = "_STOP_"; // Stop symbol.

                                      // Maximum alphabet sizes.
  const unsigned int kMaxFormAlphabetSize = 0xffff;
  const unsigned int kMaxLemmaAlphabetSize = 0xffff;
  const unsigned int kMaxShapeAlphabetSize = 0xffff;
  const unsigned int kMaxPosAlphabetSize = 0xff;
  const unsigned int kMaxCoarsePosAlphabetSize = 0xff;
  const unsigned int kMaxFeatsAlphabetSize = 0xfff; //0xffff;
};

#endif /* TOKENDICTIONARY_H_ */
