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

#include "Utils.h"
#include <iostream>
#include <sstream>
#include "MorphologicalReader.h"
#include "MorphologicalOptions.h"


Instance *MorphologicalReader::GetNext() {
  // Fill all fields for the entire sentence.
  std::vector<std::vector<std::string> > sentence_fields;
  std::string line;
  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      if (line.length() <= 0) break;
      if (0 == line.substr(0, 1).compare("#")) {
        continue;
      }
      //LOG(INFO) << line;
      std::vector<std::string> fields;
      // Also allow to break on spaces for compatibility with CONLL 2002.
      StringSplit(line, "\t", &fields, false);
      sentence_fields.push_back(fields);
    }
  }

  // Sentence length.
  int length = sentence_fields.size();

  // Convert to array of words, POS tags, and entity tags.
  std::vector<std::string> forms(length); //aka, words
  std::vector<std::string> lemmas(length);
  std::vector<std::string> cpostags(length);
  std::vector<std::string> feats(length); //aka, morphological features, feats

  for (int i = 0; i < length; ++i) {
    const std::vector<std::string> &info = sentence_fields[i];
    forms[i] = info[1];
    lemmas[i] = info[2];
    cpostags[i] = info[3];
    feats[i] = info[5];
  }

  MorphologicalInstance *instance = NULL;
  if (length > 0) {
    instance = new MorphologicalInstance;
    instance->Initialize(forms, lemmas, cpostags, feats);
  }
  return static_cast<Instance*>(instance);
}
