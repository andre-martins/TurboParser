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

#include "DependencyReader.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

Instance *DependencyReader::GetNext() {
  // Fill all fields for the entire sentence.
  std::vector<std::vector<std::string> > sentence_fields;
  std::string line;
  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      if (line.length() <= 0) break;
      // Ignore comment lines (necessary for CONLLU files).
      if (0 == line.substr(0, 1).compare("#")) continue;
      std::vector<std::string> fields;
      StringSplit(line, "\t", &fields, true);
      // Ignore contraction tokens (necessary for CONLLU files).
      if (fields[0].find_first_of("-") != fields[0].npos) continue;
      sentence_fields.push_back(fields);
    }
  }

  // Sentence length.
  int length = sentence_fields.size();

  // Convert to array of forms, lemmas, etc.
  // Note: the first token is the root symbol.
  std::vector<std::string> forms(length + 1);
  std::vector<std::string> lemmas(length + 1);
  std::vector<std::string> cpos(length + 1);
  std::vector<std::string> pos(length + 1);
  std::vector<std::vector<std::string> > feats(length + 1);
  std::vector<std::string> deprels(length + 1);
  std::vector<int> heads(length + 1);

  forms[0] = "_root_";
  lemmas[0] = "_root_";
  cpos[0] = "_root_";
  pos[0] = "_root_";
  deprels[0] = "_root_";
  heads[0] = -1;
  feats[0] = std::vector<std::string>(1, "_root_");

  for (int i = 0; i < length; i++) {
    const std::vector<std::string> &info = sentence_fields[i];

    int index;
    std::stringstream ss(info[0]);
    ss >> index;
    CHECK_EQ(index, i+1) << "Token indices are not correct.";

    forms[i + 1] = info[1];
    lemmas[i + 1] = info[2];
    cpos[i + 1] = info[3];
    pos[i + 1] = info[4];

    std::string feat_seq = info[5];
    if (0 == feat_seq.compare("_")) {
      feats[i + 1].clear();
    } else {
      StringSplit(feat_seq, "|", &feats[i + 1], true);
    }

    deprels[i + 1] = info[7];
    ss.str("");
    ss.clear();
    ss << info[6];
    ss >> heads[i + 1];
    if (heads[i + 1] < 0 || heads[i + 1] > length) {
      CHECK(false) << "Invalid value of head (" << heads[i + 1]
        << " not in range [0.." << length
        << "]";
    }
  }

  DependencyInstance *instance = NULL;
  if (length > 0) {
    instance = new DependencyInstance;
    instance->Initialize(forms, lemmas, cpos, pos, feats, deprels, heads);
  }

  return static_cast<Instance*>(instance);
}
