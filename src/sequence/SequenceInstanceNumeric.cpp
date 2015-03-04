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

#include "SequenceInstanceNumeric.h"
#include <iostream>
#include <algorithm>

const char *kPunctuationSymbols = "!\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~";
const int kUnknownShape = 0xffff;

void SequenceInstanceNumeric::Initialize(const SequenceDictionary &dictionary,
                                         SequenceInstance* instance) {
  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  int length = instance->size();
  int i;
  int id;

  int prefix_length = FLAGS_prefix_length;
  int suffix_length = FLAGS_suffix_length;
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  Clear();

  form_ids_.resize(length);
  prefix_ids_.resize(length);
  suffix_ids_.resize(length);
  shape_ids_.resize(length);
  has_digit_.resize(length);
  has_upper_.resize(length);
  has_hyphen_.resize(length);
  all_digits_.resize(length);
  all_digits_with_punctuation_.resize(length);
  all_upper_.resize(length);
  first_upper_.resize(length);
  tag_ids_.resize(length);

  for (i = 0; i < length; i++) {
    std::string form = instance->GetForm(i);
    if (!form_case_sensitive) {
      transform(form.begin(), form.end(), form.begin(), ::tolower);
    }
    id = token_dictionary->GetFormId(form);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    form_ids_[i] = id;

    prefix_ids_[i].resize(prefix_length);
    for (int l = 0; l < prefix_length; ++l) {
      std::string prefix = form.substr(0, l+1);
      id = token_dictionary->GetPrefixId(prefix);
      CHECK_LT(id, 0xffff);
      if (id < 0) id = TOKEN_UNKNOWN;
      prefix_ids_[i][l] = id;
    }

    suffix_ids_[i].resize(suffix_length);
    for (int l = 0; l < suffix_length; ++l) {
      int start = form.length() - l - 1;
      if (start < 0) start = 0;
      std::string suffix = form.substr(start, l+1);
      id = token_dictionary->GetSuffixId(suffix);
      CHECK_LT(id, 0xffff);
      if (id < 0) id = TOKEN_UNKNOWN;
      suffix_ids_[i][l] = id;
    }

    // Compute and store the word shape.
    std::string shape;
    dictionary.GetTokenDictionary()->GetWordShape(instance->GetForm(i), &shape);
    int shape_id = dictionary.GetTokenDictionary()->GetShapeId(shape);
    CHECK_LT(shape_id, 0xffff);
    if (shape_id < 0) shape_id = kUnknownShape;
    shape_ids_[i] = shape_id;

    // Compute and store various flags.
    const char* word = instance->GetForm(i).c_str();
    int word_length = instance->GetForm(i).length();
    int num_digits = CountDigits(word, word_length);

    has_digit_[i] = (num_digits > 0);
    has_upper_[i] = HasUpperCaseLetters(word, word_length);
    has_hyphen_[i] = HasHyphen(word, word_length);
    all_digits_[i] = AllDigits(word, word_length);
    all_digits_with_punctuation_[i] = AllDigitsWithPunctuation(word,
                                                               word_length);
    all_upper_[i] = AllUpperCase(word, word_length);
    first_upper_[i] = IsCapitalized(word, word_length);

    //id = token_dictionary->GetPosTagId(instance->GetTag(i));
    id = dictionary.GetTagAlphabet().Lookup(instance->GetTag(i));
    //CHECK_LT(id, 0xff);
    //CHECK_GE(id, 0);
    if (id < 0) {
      id = TOKEN_UNKNOWN;
      LOG(INFO) << "Unknown tag: " << instance->GetTag(i);
    }
    tag_ids_[i] = id;
  }
}

bool SequenceInstanceNumeric::IsPunctuation(char c) {
  const char *symbol = kPunctuationSymbols;
  while (*symbol != '\0') {
    if (c == *symbol) return true;
    ++symbol;
  }
  return false;
  //return (c == '\'') || (c == '-') || (c == '&') || (c == '/');
}
