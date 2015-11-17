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

#include "DependencyPart.h"

void DependencyParts::DeleteAll() {
  for (int i = 0; i < NUM_DEPENDENCYPARTS; ++i) {
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

void DependencyParts::DeleteIndices() {
  for (int i = 0; i < index_.size(); ++i) {
    index_[i].clear();
  }
  index_.clear();

  for (int i = 0; i < index_labeled_.size(); ++i) {
    for (int j = 0; j < index_labeled_[i].size(); ++j) {
      index_labeled_[i][j].clear();
    }
    index_labeled_[i].clear();
  }
  index_labeled_.clear();
}

void DependencyParts::BuildIndices(int sentence_length, bool labeled) {
  DeleteIndices();
  index_.resize(sentence_length);
  for (int h = 0; h < sentence_length; ++h) {
    index_[h].resize(sentence_length);
    for (int m = 0; m < sentence_length; ++m) {
      index_[h][m] = -1;
    }
  }

  int offset, num_basic_parts;
  GetOffsetArc(&offset, &num_basic_parts);
  for (int r = 0; r < num_basic_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == DEPENDENCYPART_ARC);
    int h = static_cast<DependencyPartArc*>(part)->head();
    int m = static_cast<DependencyPartArc*>(part)->modifier();
    index_[h][m] = offset + r;
  }

  if (labeled) {
    index_labeled_.resize(sentence_length);
    for (int h = 0; h < sentence_length; ++h) {
      index_labeled_[h].resize(sentence_length);
      for (int m = 0; m < sentence_length; ++m) {
        index_labeled_[h][m].clear();
      }
    }

    int offset, num_labeled_arcs;
    GetOffsetLabeledArc(&offset, &num_labeled_arcs);
    for (int r = 0; r < num_labeled_arcs; ++r) {
      Part *part = (*this)[offset + r];
      CHECK(part->type() == DEPENDENCYPART_LABELEDARC);
      int h = static_cast<DependencyPartLabeledArc*>(part)->head();
      int m = static_cast<DependencyPartLabeledArc*>(part)->modifier();
      index_labeled_[h][m].push_back(offset + r);
    }
  }
}
