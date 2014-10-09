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

#include "EntityReader.h"
#include "EntityOptions.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Instance *EntityReader::GetNext() {
  // Fill all fields for the entire sentence.
  std::vector<std::vector<std::string> > sentence_fields;
  std::string line;
  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      if (line.length() <= 0) break;
      std::vector<std::string> fields;
      // Also allow to break on spaces for compatibility with CONLL 2002.
      StringSplit(line, " \t", &fields);
      sentence_fields.push_back(fields);
    }
  }

  // Sentence length.
  int length = sentence_fields.size();

  // Convert to array of words, POS tags, and entity tags.
  std::vector<std::string> forms(length);
  std::vector<std::string> pos(length);
  std::vector<std::string> entity_tags(length);

  for(int i = 0; i < length; ++i) {
    const vector<string> &info = sentence_fields[i];
    forms[i] = info[0];
    pos[i] = info[1];
    entity_tags[i] = info[2];
  }

  EntityInstance *instance = NULL;
  if (length > 0) {
    instance = new EntityInstance;
    instance->Initialize(forms, pos, entity_tags);
    instance->ConvertToTaggingScheme(static_cast<EntityOptions*>(options_)->
                                     tagging_scheme());
  }

  return static_cast<Instance*>(instance);
}
