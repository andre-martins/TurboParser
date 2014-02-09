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

  BuildIndices();
}
