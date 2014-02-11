// Copyright (c) 2012-2013 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.1.
//
// TurboParser 2.1 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.1.  If not, see <http://www.gnu.org/licenses/>.

#include "SemanticInstanceNumeric.h"
//#include "DependencyDictionary.h"
#include <iostream>
#include <algorithm>

using namespace std;

void SemanticInstanceNumeric::Initialize(
    const SemanticDictionary &dictionary,
    SemanticInstance* instance) {
  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  DependencyDictionary *dependency_dictionary =
    dictionary.GetDependencyDictionary();
  DependencyInstance *dependency_instance =
    static_cast<DependencyInstance*>(instance);

  Clear();

  DependencyInstanceNumeric::Initialize(*dependency_dictionary,
                                        dependency_instance);

  int num_predicates = instance->GetNumPredicates();
  predicate_ids_.resize(num_predicates);
  predicate_indices_.resize(num_predicates);
  argument_role_ids_.resize(num_predicates);
  argument_indices_.resize(num_predicates);
  for (int k = 0; k < instance->GetNumPredicates(); k++) {
    const string &name = instance->GetPredicateName(k);
    int id = dictionary.GetPredicateAlphabet().Lookup(name);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = TOKEN_UNKNOWN;
    predicate_ids_[k] = id;
    predicate_indices_[k] = instance->GetPredicateIndex(k);

    int num_arguments = instance->GetNumArgumentsPredicate(k);
    argument_role_ids_[k].resize(num_arguments);
    argument_indices_[k].resize(num_arguments);
    for (int l = 0; l < num_arguments; ++l) {
      const string &name = instance->GetArgumentRole(k, l);
      id = dictionary.GetRoleAlphabet().Lookup(name);
      CHECK_LT(id, 0xffff);
      if (id < 0) id = TOKEN_UNKNOWN;
      argument_role_ids_[k][l] = id;
      argument_indices_[k][l] = instance->GetArgumentIndex(k, l);
    }
  }

  ComputeDependencyInformation(dictionary, instance);

  BuildIndices();
}

void SemanticInstanceNumeric::ComputeDependencyInformation(
    const SemanticDictionary &dictionary,
    SemanticInstance* instance) {
  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  DependencyDictionary *dependency_dictionary =
    dictionary.GetDependencyDictionary();
  DependencyInstance *dependency_instance =
    static_cast<DependencyInstance*>(instance);

  int instance_length = instance->size();
  modifiers_.resize(instance_length);
  left_siblings_.resize(instance_length);
  right_siblings_.resize(instance_length);

  // List of dependents, left and right siblings.
  for (int h = 0; h < instance_length; ++h) {
    modifiers_[h].clear();
    left_siblings_[h] = -1;
    right_siblings_[h] = -1;
  }
  for (int m = 1; m < instance_length; ++m) {
    int h = instance->GetHead(m);
    modifiers_[h].push_back(m);
  }
  for (int h = 0; h < instance_length; ++h) {
    for (int k = 0; k < modifiers_[h].size(); ++k) {
      int m = modifiers_[h][k];
      if (k > 0) left_siblings_[m] = modifiers_[h][k-1];
      if (k+1 < modifiers_[h].size()) right_siblings_[m] = modifiers_[h][k+1];
    }
  }

  // Select passive/active voice.
  is_passive_voice_.assign(instance_length, false);
  for (int i = 0; i < instance_length; ++i) {
    is_passive_voice_[i] = ComputePassiveVoice(instance, i);
  }
}

bool SemanticInstanceNumeric::ComputePassiveVoice(
    SemanticInstance* instance,
    int index) {
  const string &form = instance->GetForm(index);
  const string &tag = instance->GetPosTag(index);
  if (!IsVerb(index)) {
    return false; // Not even a verb.
  }

  std::string form_lower = form;
  std::transform(form_lower.begin(),
                 form_lower.end(),
                 form_lower.begin(),
                 ::tolower);

  if (0 == form_lower.compare("been")) return false;
  if (0 != tag.compare("VBN")) return false;

  // Find passive in parents.
  int head = instance->GetHead(index);
  while (true) {
    if (head <= 0) return true;

    const string &head_form = instance->GetForm(head);
    const string &head_tag = instance->GetPosTag(head);

    if (0 == head_tag.compare(0, 2, "NN")) return true;

    std::string head_form_lower = form;
    std::transform(head_form_lower.begin(),
                   head_form_lower.end(),
                   head_form_lower.begin(),
                   ::tolower);

    if (0 == head_form_lower.compare("am") ||
        0 == head_form_lower.compare("are") ||
        0 == head_form_lower.compare("is") ||
        0 == head_form_lower.compare("was") ||
        0 == head_form_lower.compare("were") ||
        0 == head_form_lower.compare("be") ||
        0 == head_form_lower.compare("been") ||
        0 == head_form_lower.compare("being")) {
      return true;
    }

    if (0 == head_form_lower.compare("have") ||
        0 == head_form_lower.compare("has") ||
        0 == head_form_lower.compare("had") ||
        0 == head_form_lower.compare("having")) {
      return false;
    }

    if (0 == head_tag.compare("VBZ") ||
        0 == head_tag.compare("VBD") ||
        0 == head_tag.compare("VBP") ||
        0 == head_tag.compare("MD")) {
      return true;
    }

    head = instance->GetHead(head);
  }

  return false;
}
