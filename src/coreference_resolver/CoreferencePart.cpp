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

#include "CoreferencePart.h"

void CoreferenceParts::DeleteAll() {
  DeleteIndices();

  for (iterator iter = begin(); iter != end(); iter++) {
    if ((*iter) != NULL) {
      delete (*iter);
      *iter = NULL;
    }
  }

  clear();
}

void CoreferenceParts::DeleteIndices() {
  for (int i = 0; i < index_.size(); ++i) {
    index_[i].clear();
  }
  index_.clear();
}

void CoreferenceParts::BuildIndices(int num_mentions) {
  DeleteIndices();
  index_.resize(num_mentions);

  int num_arcs = size();
  int offset = 0;
  for (int r = 0; r < num_arcs; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == COREFERENCEPART_ARC);
    int i = static_cast<CoreferencePartArc*>(part)->child_mention();
    CHECK_GE(i, 0);
    CHECK_LT(i, num_mentions);
    index_[i].push_back(offset + r);
  }
}
