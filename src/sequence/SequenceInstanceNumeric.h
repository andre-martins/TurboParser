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

#ifndef SEQUENCEINSTANCENUMERIC_H_
#define SEQUENCEINSTANCENUMERIC_H_

#include "SequenceInstance.h"
#include "SequenceDictionary.h"
#include <vector>
#include <string>

class SequenceInstanceNumeric : public Instance {
public:
  SequenceInstanceNumeric() {};
  virtual ~SequenceInstanceNumeric() { Clear(); };

  Instance* Copy() {
    CHECK(false) << "Not implemented.";
    return NULL;
  }

  int size() { return form_ids_.size(); };

  virtual void Clear() {
    form_ids_.clear();
    prefix_ids_.clear();
    suffix_ids_.clear();
    shape_ids_.clear();
    has_digit_.clear();
    has_upper_.clear();
    has_hyphen_.clear();
    all_digits_.clear();
    all_digits_with_punctuation_.clear();
    all_upper_.clear();
    first_upper_.clear();
    tag_ids_.clear();
    lexicon_word_ids_.clear();
  }

  void Initialize(const SequenceDictionary &dictionary,
                  SequenceInstance *instance);

  const std::vector<int> &GetFormIds() const { return form_ids_; }
  const std::vector<int> &GetTagIds() const { return tag_ids_; }
  const std::vector<int> &GetLexiconWordIds() const { return lexicon_word_ids_; }

  int GetFormId(int i) { return form_ids_[i]; }
  int GetMaxPrefixLength(int i) { return prefix_ids_[i].size(); }
  int GetMaxSuffixLength(int i) { return suffix_ids_[i].size(); }
  int GetPrefixId(int i, int length) { return prefix_ids_[i][length - 1]; }
  int GetSuffixId(int i, int length) { return suffix_ids_[i][length - 1]; }
  int GetShapeId(int i) { return shape_ids_[i]; }
  bool HasDigit(int i) { return has_digit_[i]; }
  bool HasUpper(int i) { return has_upper_[i]; }
  bool HasHyphen(int i) { return has_hyphen_[i]; }
  bool AllDigits(int i) { return all_digits_[i]; }
  bool AllDigitsWithPunctuation(int i) {
    return all_digits_with_punctuation_[i];
  }
  bool AllUpper(int i) { return all_upper_[i]; }
  bool FirstUpper(int i) { return first_upper_[i]; }
  int GetTagId(int i) { return tag_ids_[i]; }
  int GetLexiconWordId(int i) { return lexicon_word_ids_[i]; }

protected:
  bool IsUpperCase(char c) { return (c >= 'A' && c <= 'Z'); }
  bool IsLowerCase(char c) { return (c >= 'a' && c <= 'z'); }
  bool IsDigit(char c) { return (c >= '0' && c <= '9'); }
  bool IsPeriod(char c) { return (c == '.'); }
  bool IsPunctuation(char c);

  bool AllUpperCase(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (!IsUpperCase(word[i])) return false;
    }
    return true;
  }

  bool AllLowerCase(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (!IsLowerCase(word[i])) return false;
    }
    return true;
  }

  bool IsCapitalized(const char* word, int len) {
    if (len <= 0) return false;
    return IsUpperCase(word[0]);
  }

  bool IsMixedCase(const char* word, int len) {
    if (len <= 0) return false;
    if (!IsLowerCase(word[0])) return false;
    for (int i = 1; i < len; ++i) {
      if (IsUpperCase(word[i])) return true;
    }
    return false;
  }

  bool EndsWithPeriod(const char* word, int len) {
    if (len <= 0) return false;
    return IsPeriod(word[len - 1]);
  }

  bool HasInternalPeriod(const char* word, int len) {
    if (len <= 0) return false;
    for (int i = 0; i < len - 1; ++i) {
      if (IsPeriod(word[i])) return true;
    }
    return false;
  }

  bool HasInternalPunctuation(const char* word, int len) {
    if (len <= 0) return false;
    for (int i = 0; i < len - 1; ++i) {
      if (IsPunctuation(word[i])) return true;
    }
    return false;
  }

  int CountDigits(const char* word, int len) {
    int num_digits = 0;
    for (int i = 0; i < len; ++i) {
      if (IsDigit(word[i])) ++num_digits;
    }
    return num_digits;
  }

  bool HasUpperCaseLetters(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (IsUpperCase(word[i])) return true;
    }
    return false;
  }

  bool HasHyphen(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if ('-' == word[i]) return true;
    }
    return false;
  }

  bool AllDigits(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (!IsDigit(word[i])) return false;
    }
    return true;
  }

  bool AllDigitsWithPunctuation(const char* word, int len) {
    bool has_digits = false;
    bool has_punctuation = false;
    for (int i = 0; i < len; ++i) {
      if (IsDigit(word[i])) {
        has_digits = true;
      } else if (IsPunctuation(word[i])) {
        has_punctuation = true;
      } else {
        return false;
      }
    }
    return has_digits && has_punctuation;
  }

private:
  std::vector<int> form_ids_;
  std::vector<std::vector<int> > prefix_ids_;
  std::vector<std::vector<int> > suffix_ids_;
  std::vector<int> shape_ids_;
  std::vector<bool> has_digit_;
  std::vector<bool> has_upper_;
  std::vector<bool> has_hyphen_;
  std::vector<bool> all_digits_;
  std::vector<bool> all_digits_with_punctuation_;
  std::vector<bool> all_upper_;
  std::vector<bool> first_upper_;
  std::vector<int> tag_ids_;

  // IDs according to the external lexicon.
  // Must be separated from from_ids_ since it's a different
  // dictionary which is not capped by 0xffff.
  std::vector<int> lexicon_word_ids_;
};

#endif /* SEQUENCEINSTANCENUMERIC_H_ */
