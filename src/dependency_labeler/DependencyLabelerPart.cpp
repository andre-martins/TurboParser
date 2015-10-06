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

#include "DependencyLabelerPart.h"

void DependencyLabelerParts::DeleteAll() {
  for (int i = 0; i < NUM_DEPENDENCYLABELERPARTS; ++i) {
    offsets_[i] = -1;
  }

  DeleteArcIndices();
  DeleteSiblingIndices();

  for (iterator iter = begin(); iter != end(); iter++) {
    if  ((*iter) != NULL) {
      delete (*iter);
      *iter = NULL;
    }
  }

  clear();
}

void DependencyLabelerParts::DeleteArcIndices() {
  for (int i = 0; i < index_arcs_.size(); ++i) {
    index_arcs_[i].clear();
  }
  index_arcs_.clear();
}

void DependencyLabelerParts::DeleteSiblingIndices() {
  for (int h = 0; h < index_siblings_.size(); ++h) {
    for (int i = 0; i < index_siblings_[h].size(); ++i) {
      index_siblings_[h][i].clear();
    }
    index_siblings_[h].clear();
  }
  index_siblings_.clear();
}

void DependencyLabelerParts::BuildArcIndices(const std::vector<int> &heads) {
  DeleteArcIndices();
  index_arcs_.assign(heads.size(), std::vector<int>(0));

  int offset, num_arcs;
  GetOffsetArc(&offset, &num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == DEPENDENCYLABELERPART_ARC);
    int m = static_cast<DependencyLabelerPartArc*>(part)->modifier();
    index_arcs_[m].push_back(offset + r);
  }
}

void DependencyLabelerParts::ComputeSiblings(
    const std::vector<int> &heads) {
  siblings_.assign(heads.size(),
                   std::vector<int>(0));
  for (int m = 1; m < heads.size(); ++m) {
    siblings_[heads[m]].push_back(m);
  }

  map_siblings_.assign(heads.size(),
                       std::vector<int>(heads.size(), -1));
  for (int h = 0; h < heads.size(); ++h) {
    for (int i = 0; i < siblings_[h].size(); ++i) {
      int m = siblings_[h][i];
      map_siblings_[h][m] = i;
    }
  }
}

void DependencyLabelerParts::BuildSiblingIndices(
    const std::vector<int> &heads) {
  DeleteSiblingIndices();
  index_siblings_.assign(heads.size(),
                         std::vector<std::vector<int> >(0));
  for (int h = 0; h < heads.size(); ++h) {
    index_siblings_[h].assign(siblings_[h].size() + 1,
                              std::vector<int>(0));
  }
  int offset, num_siblings;
  GetOffsetSibling(&offset, &num_siblings);
  for (int r = 0; r < num_siblings; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == DEPENDENCYLABELERPART_SIBLING);
    int h = static_cast<DependencyLabelerPartSibling*>(part)->head();
    int m = static_cast<DependencyLabelerPartSibling*>(part)->modifier();
    int i = GetSiblingIndex(h, m);
    index_siblings_[h][i].push_back(offset + r);
  }
}
