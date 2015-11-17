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

#include "DependencyInstanceNumeric.h"
#include <iostream>
#include <algorithm>

void DependencyInstanceNumeric::Initialize(
  const DependencyDictionary &dictionary,
  DependencyInstance* instance) {
  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  int length = instance->size();
  int i;
  int id;

  int prefix_length = FLAGS_prefix_length;
  int suffix_length = FLAGS_suffix_length;
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  Clear();

  form_ids_.resize(length);
  form_lower_ids_.resize(length);
  lemma_ids_.resize(length);
  prefix_ids_.resize(length);
  suffix_ids_.resize(length);
  feats_ids_.resize(length);
  pos_ids_.resize(length);
  cpos_ids_.resize(length);
  //shapes_.resize(length);
  is_noun_.resize(length);
  is_verb_.resize(length);
  is_punc_.resize(length);
  is_coord_.resize(length);
  heads_.resize(length);
  relations_.resize(length);

  for (i = 0; i < length; i++) {
    std::string form = instance->GetForm(i);
    std::string form_lower(form);
    transform(form_lower.begin(), form_lower.end(), form_lower.begin(),
              ::tolower);
    if (!form_case_sensitive) form = form_lower;
    id = token_dictionary->GetFormId(form);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    form_ids_[i] = id;

    id = token_dictionary->GetFormLowerId(form_lower);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    form_lower_ids_[i] = id;

    id = token_dictionary->GetLemmaId(instance->GetLemma(i));
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    lemma_ids_[i] = id;

    std::string prefix = form.substr(0, prefix_length);
    id = token_dictionary->GetPrefixId(prefix);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    prefix_ids_[i] = id;

    int start = form.length() - suffix_length;
    if (start < 0) start = 0;
    std::string suffix = form.substr(start, suffix_length);
    id = token_dictionary->GetSuffixId(suffix);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    suffix_ids_[i] = id;

    id = token_dictionary->GetPosTagId(instance->GetPosTag(i));
    CHECK_LT(id, 0xff);
    if (id < 0) id = TOKEN_UNKNOWN;
    pos_ids_[i] = id;

    id = token_dictionary->GetCoarsePosTagId(instance->GetCoarsePosTag(i));
    CHECK_LT(id, 0xff);
    if (id < 0) id = TOKEN_UNKNOWN;
    cpos_ids_[i] = id;

    feats_ids_[i].resize(instance->GetNumMorphFeatures(i));
    for (int j = 0; j < instance->GetNumMorphFeatures(i); ++j) {
      id = token_dictionary->GetMorphFeatureId(instance->GetMorphFeature(i, j));
      CHECK_LT(id, 0xffff);
      if (id < 0) id = TOKEN_UNKNOWN;
      feats_ids_[i][j] = id;
    }

    //GetWordShape(instance->GetForm(i), &shapes_[i]);

    // Check whether the word is a noun, verb, punctuation or coordination.
    // Note: this depends on the POS tag string.
    // This procedure is taken from EGSTRA
    // (http://groups.csail.mit.edu/nlp/egstra/).
    is_noun_[i] = false;
    is_verb_[i] = false;
    is_punc_[i] = false;
    is_coord_[i] = false;

    const char* tag = instance->GetPosTag(i).c_str();
    if (tag[0] == 'v' || tag[0] == 'V') {
      is_verb_[i] = true;
    } else if (tag[0] == 'n' || tag[0] == 'N') {
      is_noun_[i] = true;
    } else if (strcmp(tag, "Punc") == 0 ||
               strcmp(tag, "$,") == 0 ||
               strcmp(tag, "$.") == 0 ||
               strcmp(tag, "PUNC") == 0 ||
               strcmp(tag, "punc") == 0 ||
               strcmp(tag, "F") == 0 ||
               strcmp(tag, "IK") == 0 ||
               strcmp(tag, "XP") == 0 ||
               strcmp(tag, ",") == 0 ||
               strcmp(tag, ";") == 0) {
      is_punc_[i] = true;
    } else if (strcmp(tag, "Conj") == 0 ||
               strcmp(tag, "KON") == 0 ||
               strcmp(tag, "conj") == 0 ||
               strcmp(tag, "Conjunction") == 0 ||
               strcmp(tag, "CC") == 0 ||
               strcmp(tag, "cc") == 0) {
      is_coord_[i] = true;
    }

    heads_[i] = instance->GetHead(i);
    relations_[i] = dictionary.GetLabelAlphabet().Lookup(
      instance->GetDependencyRelation(i));
  }
}
