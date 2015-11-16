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

#include "ConstituencyLabelerPart.h"

void ConstituencyLabelerParts::DeleteAll() {
  for (int i = 0; i < NUM_CONSTITUENCYLABELERPARTS; ++i) {
    offsets_[i] = -1;
  }

  DeleteIndices();

  for (iterator iter = begin(); iter != end(); iter++) {
    if ((*iter) != NULL) {
      delete (*iter);
      *iter = NULL;
    }
  }

  clear();
}

void ConstituencyLabelerParts::DeleteNodeIndices() {
  for (int i = 0; i < index_.size(); ++i) {
    index_[i].clear();
  }
  index_.clear();
}

void ConstituencyLabelerParts::DeleteIndices() {
  DeleteNodeIndices();
}

void ConstituencyLabelerParts::BuildNodeIndices(int num_nodes) {
  DeleteNodeIndices();
  index_.resize(num_nodes);

  int offset, num_node_parts;
  GetOffsetNode(&offset, &num_node_parts);
  for (int r = 0; r < num_node_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == CONSTITUENCYLABELERPART_NODE);
    int i = static_cast<ConstituencyLabelerPartNode*>(part)->position();
    index_[i].push_back(offset + r);
  }
}

void ConstituencyLabelerParts::BuildIndices(int num_nodes) {
  BuildNodeIndices(num_nodes);
}
