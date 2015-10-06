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

  virtual void Clear() {
    // Don't clear token_dictionary, since this class does not own it.
    tag_alphabet_.clear();
  }

  virtual void Save(FILE *fs) {
    if (0 > tag_alphabet_.Save(fs)) CHECK(false);
  }

  virtual void Load(FILE *fs) {
    if (0 > tag_alphabet_.Load(fs)) CHECK(false);
    tag_alphabet_.BuildNames();
  }

  void AllowGrowth() { token_dictionary_->AllowGrowth(); }
  void StopGrowth() { token_dictionary_->StopGrowth(); }

  virtual void CreateTagDictionary(SequenceReader *reader);

  void BuildTagNames() {
    tag_alphabet_.BuildNames();
  }

  const string &GetTagName(int tag) const {
    return tag_alphabet_.GetName(tag);
  }

  // By default, all bigrams are allowed. Override this function to
  // prevent some bigrams to be feasible (e.g. for BIO tagging).
  virtual bool IsAllowedBigram(int left_tag, int tag) {
    return true;
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

  void GetBigramTags(int bigram_label, int *left_tag, int *tag) {
    *tag = (bigram_label % (1 + tag_alphabet_.size())) - 1;
    *left_tag = (bigram_label / (1 + tag_alphabet_.size())) - 1;
    CHECK_EQ(bigram_label, GetBigramLabel(*left_tag, *tag));
  }

  // TODO(atm): need to test this.
  void GetTrigramTags(int trigram_label, int *left_left_tag, int *left_tag,
                      int *tag) {
    *tag = (trigram_label % (1 + tag_alphabet_.size())) - 1;
    int bigram_label = (trigram_label / (1 + tag_alphabet_.size()));
    *left_tag = (bigram_label % (1 + tag_alphabet_.size())) - 1;
    *left_left_tag = (bigram_label / (1 + tag_alphabet_.size())) - 1;
    CHECK_EQ(trigram_label, GetTrigramLabel(*left_left_tag, *left_tag, *tag));
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
};

#endif /* SEQUENCEDICTIONARY_H_ */
