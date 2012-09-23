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

#ifndef SEQUENCEINSTANCENUMERIC_H_
#define SEQUENCEINSTANCENUMERIC_H_

#include "SequenceInstance.h"
#include "SequenceDictionary.h"
#include <vector>
#include <string>

using namespace std;

class SequenceInstanceNumeric : public Instance {
public:
  SequenceInstanceNumeric() {};
  virtual ~SequenceInstanceNumeric() { Clear(); };

  int size() { return form_ids_.size(); };

  void Clear() {
    form_ids_.clear();
    prefix_ids_.clear();
    suffix_ids_.clear();
    has_digit_.clear();
    has_upper_.clear();
    has_hyphen_.clear();
    tag_ids_.clear();
  }

  int Initialize(const SequenceDictionary &dictionary,
                 SequenceInstance *instance);

  const vector<int> &GetFormIds() const { return form_ids_; }
  const vector<int> &GetTagIds() const { return tag_ids_; }

  int GetFormId(int i) { return form_ids_[i]; };
  int GetMaxPrefixLength(int i) { return prefix_ids_[i].size(); }
  int GetMaxSuffixLength(int i) { return suffix_ids_[i].size(); }
  int GetPrefixId(int i, int length) { return prefix_ids_[i][length-1]; };
  int GetSuffixId(int i, int length) { return suffix_ids_[i][length-1]; };
  bool HasDigit(int i) { return has_digit_[i]; };
  bool HasUpper(int i) { return has_upper_[i]; };
  bool HasHyphen(int i) { return has_hyphen_[i]; };
  int GetTagId(int i) { return tag_ids_[i]; };

protected:
  bool IsUpperCase(char c) { return (c >= 'A' && c <= 'Z'); }
  bool IsLowerCase(char c) { return (c >= 'a' && c <= 'z'); }
  bool IsDigit(char c) { return (c >= '0' && c <= '9'); }
  bool IsPeriod(char c) { return (c == '.'); }
  bool IsPunctuation(char c) { return (c == '\'') || (c == '-') || (c == '&'); }

  bool AllUpperCase(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (!IsUpperCase(word[i])) return false;
    }
    return true;
  };

  bool AllLowerCase(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (!IsLowerCase(word[i])) return false;
    }
    return true;
  };

  bool IsCapitalized(const char* word, int len) {
    if (len <= 0) return false;
    return IsUpperCase(word[0]);
  };

  bool IsMixedCase(const char* word, int len) {
    if (len <= 0) return false;
    if (!IsLowerCase(word[0])) return false;
    for (int i = 1; i < len; ++i) {
      if (IsUpperCase(word[i])) return true;
    }
    return false;
  };

  bool EndsWithPeriod(const char* word, int len) {
    if (len <= 0) return false;
    return IsPeriod(word[len-1]);
  };

  bool HasInternalPeriod(const char* word, int len) {
    if (len <= 0) return false;
    for (int i = 0; i < len - 1; ++i) {
      if (IsPeriod(word[i])) return true;
    }
    return false;
  };

  bool HasInternalPunctuation(const char* word, int len) {
    if (len <= 0) return false;
    for (int i = 0; i < len - 1; ++i) {
      if (IsPunctuation(word[i])) return true;
    }
    return false;
  };

  int CountDigits(const char* word, int len) {
    int num_digits = 0;
    for (int i = 0; i < len; ++i) {
      if (IsDigit(word[i])) ++num_digits;
    }
    return num_digits;
  };

  bool HasUpperCaseLetters(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if (IsUpperCase(word[i])) return true;
    }
    return false;
  };

  bool HasHyphen(const char* word, int len) {
    for (int i = 0; i < len; ++i) {
      if ('-' == word[i]) return true;
    }
    return false;
  };

private:
  vector<int> form_ids_;
  vector<vector<int> > prefix_ids_;
  vector<vector<int> > suffix_ids_;
  vector<bool> has_digit_;
  vector<bool> has_upper_;
  vector<bool> has_hyphen_;
  vector<int> tag_ids_;
};

#endif /* SEQUENCEINSTANCENUMERIC_H_ */
