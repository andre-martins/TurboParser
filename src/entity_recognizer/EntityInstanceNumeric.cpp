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

#include "EntityInstanceNumeric.h"
#include <iostream>
#include <algorithm>

void EntityInstanceNumeric::Initialize(const EntityDictionary &dictionary,
                                       EntityInstance* instance) {
  SequenceInstanceNumeric::Initialize(dictionary, instance);

  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  int length = instance->size();

  pos_ids_.resize(length);
  gazetteer_ids_.clear();
  gazetteer_ids_.resize(length);
  for (int i = 0; i < length; i++) {
    int id = token_dictionary->GetPosTagId(instance->GetPosTag(i));
    CHECK_LT(id, 0xff);
    if (id < 0) id = TOKEN_UNKNOWN;
    pos_ids_[i] = id;

    std::string form = instance->GetForm(i);
    // Uncomment next 'if's to allow different-case occurences
    // to map to the same entry.
    if (!FLAGS_form_case_sensitive) {
      transform(form.begin(), form.end(), form.begin(), ::tolower);
    }
    dictionary.GetWordGazetteerIds(form,
                                   &gazetteer_ids_[i]);
    //LOG(INFO) << instance->GetForm(i) << ": " << gazetteer_ids_[i].size();
  }
}
