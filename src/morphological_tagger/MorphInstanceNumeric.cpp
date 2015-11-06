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

#include "MorphInstanceNumeric.h"
#include <iostream>
#include <algorithm>

void MorphInstanceNumeric::Initialize(const MorphDictionary &dictionary,
                                      MorphInstance* instance) {
  SequenceInstanceNumeric::Initialize(dictionary, instance);

  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  int length = instance->size();

  lemmas_ids_.resize(length);
  cpostags_ids_.resize(length);
  for (int i = 0; i<length; i++) {
    int id;
    //Lemma
    id = token_dictionary->GetLemmaId(instance->GetLemma(i));
    CHECK_LT(id, 0xffff);
    if (id<0) id = TOKEN_UNKNOWN;
    lemmas_ids_[i] = id;
    //CPosTag
    id = token_dictionary->GetCoarsePosTagId(instance->GetCoarsePosTag(i));
    CHECK_LT(id, 0xffff);
    if (id<0) id = TOKEN_UNKNOWN;
    cpostags_ids_[i] = id;
  }
}
