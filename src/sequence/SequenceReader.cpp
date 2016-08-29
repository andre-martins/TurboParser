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

#include "SequenceReader.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

using namespace std;

Instance *SequenceReader::GetNext() {
  // Fill all fields for the entire sentence.
  vector<vector<string> > sentence_fields;
  string line;
  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      if (line.length() <= 0) break;
      vector<string> fields;
      StringSplit(line, "\t", &fields, true);
      sentence_fields.push_back(fields);
    }
  }

  // Sentence length.
  int length = sentence_fields.size();

  // Convert to array of words and tags.
  vector<string> forms(length);
  vector<string> tags(length);

  for (int i = 0; i < length; ++i) {
    const vector<string> &info = sentence_fields[i];
    CHECK_EQ(info.size(), 2);
    forms[i] = info[0];
    tags[i] = info[1];
  }

  SequenceInstance *instance = NULL;
  if (length > 0) {
    instance = new SequenceInstance;
    instance->Initialize(forms, tags);
  }

  return static_cast<Instance*>(instance);
}
