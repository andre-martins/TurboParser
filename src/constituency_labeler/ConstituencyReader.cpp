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

#include "ConstituencyReader.h"
#include "ConstituencyInstance.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Instance *ConstituencyReader::GetNext() {
  // Fill all fields for the entire sentence.
  ConstituencyInstance *instance = NULL;
  std::string line;
  if (is_.is_open() && !is_.eof()) {
    getline(is_, line);
    ParseTree tree;
    std::vector<std::string> words;
    std::vector<std::string> tags;
    std::vector<std::string> lemmas;
    std::vector<std::vector<std::string> > morph;
    tree.LoadFromString(line);
    tree.ExtractWordsAndTags(&words, &tags);
    instance = new ConstituencyInstance;
    instance->Initialize(words, lemmas, tags, morph, tree);
  }

  return static_cast<Instance*>(instance);
}
