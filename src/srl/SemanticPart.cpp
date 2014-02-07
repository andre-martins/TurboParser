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

#include "SemanticPart.h"

void SemanticParts::DeleteAll() {
  for (int i = 0; i < NUM_SEMANTICPARTS; ++i) {
    offsets_[i] = -1;
  }

  DeleteIndices();

  for (iterator iter = begin(); iter != end(); iter++) {
    if  ((*iter) != NULL) {
      delete (*iter);
      *iter = NULL;
    }
  }

  clear();
}

void SemanticParts::DeleteIndices() {
  for (int p = 0; p < index_senses_.size(); ++p) {
    index_senses_[p].clear();
  }
  index_senses_.clear();

  for (int p = 0; p < index_.size(); ++p) {
    for (int a = 0; a < index_[p].size(); ++a) {
      index_[p][a].clear();
    }
    index_[p].clear();
  }
  index_.clear();

  for (int p = 0; p < index_labeled_.size(); ++p) {
    for (int a = 0; a < index_labeled_[p].size(); ++a) {
      for (int s = 0; s < index_labeled_[p][a].size(); ++s) {
        index_labeled_[p][a][s].clear();
      }
      index_labeled_[p][a].clear();
    }
    index_labeled_[p].clear();
  }
  index_labeled_.clear();
}

void SemanticParts::BuildIndices(int sentence_length, bool labeled) {
  DeleteIndices();

  int offset, num_basic_parts;
  GetOffsetArc(&offset, &num_basic_parts);
  index_senses_.resize(sentence_length);
  for (int r = 0; r < num_basic_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == SEMANTICPART_ARC);
    int p = static_cast<SemanticPartArc*>(part)->predicate();
    int s = static_cast<SemanticPartArc*>(part)->sense();
    int k = 0;
    for (; k < index_senses_[p].size(); ++k) {
      if (index_senses_[p][k] == s) break;
    }
    if (k == index_senses_[p].size()) {
      index_senses_[p].push_back(s);
    }
  }

  index_.resize(sentence_length);
  for (int p = 0; p < sentence_length; ++p) {
    index_[p].resize(sentence_length);
    for (int a = 0; a < sentence_length; ++a) {
      index_[p][a].clear();
    }
  }

  for (int r = 0; r < num_basic_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == SEMANTICPART_ARC);
    int p = static_cast<SemanticPartArc*>(part)->predicate();
    int a = static_cast<SemanticPartArc*>(part)->argument();
    int s = static_cast<SemanticPartArc*>(part)->sense();
    if (s >= index_[p][a].size()) index_[p][a].resize(s+1, -1);
    index_[p][a][s] = offset + r;
  }

  if (labeled) {
    index_labeled_.resize(sentence_length);
    for (int p = 0; p < sentence_length; ++p) {
      index_labeled_[p].resize(sentence_length);
      for (int a = 0; a < sentence_length; ++a) {
        index_labeled_[p][a].clear();
      }
    }

    int offset, num_labeled_arcs;
    GetOffsetLabeledArc(&offset, &num_labeled_arcs);
    for (int r = 0; r < num_labeled_arcs; ++r) {
      Part *part = (*this)[offset + r];
      CHECK(part->type() == SEMANTICPART_LABELEDARC);
      int p = static_cast<SemanticPartLabeledArc*>(part)->predicate();
      int a = static_cast<SemanticPartLabeledArc*>(part)->argument();
      int s = static_cast<SemanticPartLabeledArc*>(part)->sense();
      int role = static_cast<SemanticPartLabeledArc*>(part)->role();
      if (s >= index_labeled[p][a].size()) index_labeled_[p][a].resize(s+1, -1);
      if (role >= index_labeled_[p][a][s].size()) {
        index_labeled_[p][a][s].resize(role+1, -1);
      }
      index_labeled_[p][a][s][role] = offset + r;
    }
  }
}
