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

#include "CoreferenceDocument.h"

void CoreferenceDocument::DeleteAllSentences() {
  for (int i = 0; i < sentences_.size(); ++i) {
    delete sentences_[i];
  }
  sentences_.clear();
}

void CoreferenceDocument::Initialize(
    const string &name,
    int part_number,
    const std::vector<CoreferenceSentence*> &sentences) {
  name_ = name;
  part_number_ = part_number;

  // This makes sense for the Ontonotes corpus only (got this from the Berkeley
  // coreference system).
  if (name_.compare(0, 2, "bc") == 0 || name_.compare(0, 2, "wb") == 0) {
    conversation_ = true;
  } else {
    conversation_ = false;
  }

  DeleteAllSentences();
  for (int i = 0; i < sentences.size(); ++i) {
    CoreferenceSentence* sentence =
      static_cast<CoreferenceSentence*>(sentences[i]->Copy());
    sentences_.push_back(sentence);
  }
}
