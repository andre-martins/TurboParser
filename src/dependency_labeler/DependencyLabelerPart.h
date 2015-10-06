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

#ifndef DEPENDENCYLABELERPART_H_
#define DEPENDENCYLABELERPART_H_

#include <stdio.h>
#include <vector>
#include "Part.h"

using namespace std;

enum {
  DEPENDENCYLABELERPART_ARC = 0,
  DEPENDENCYLABELERPART_SIBLING,
  NUM_DEPENDENCYLABELERPARTS
};

class DependencyLabelerPartArc : public Part {
 public:
  DependencyLabelerPartArc() { h_ = m_ = label_ = -1; };
  DependencyLabelerPartArc(int head, int modifier, int label) {
    h_ = head;
    m_ = modifier;
    label_ = label;
  }
  virtual ~DependencyLabelerPartArc() {};

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int label() { return label_; };

 public:
  int type() { return DEPENDENCYLABELERPART_ARC; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&label_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&label_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier.
  int label_; // Label ID.
};

class DependencyLabelerPartSibling : public Part {
 public:
  DependencyLabelerPartSibling() { h_ = m_ = s_ = -1; };
  DependencyLabelerPartSibling(int head, int modifier, int sibling,
                               int modifier_label, int sibling_label) {
    h_ = head;
    m_ = modifier;
    s_ = sibling;
    modifier_label_ = modifier_label;
    sibling_label_ = sibling_label;
  }
  virtual ~DependencyLabelerPartSibling() {};

 public:
  int type() { return DEPENDENCYLABELERPART_SIBLING; };

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int sibling() { return s_; };
  int modifier_label() { return modifier_label_; };
  int sibling_label() { return sibling_label_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&s_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&modifier_label_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&sibling_label_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&s_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&modifier_label_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&sibling_label_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier (if m_ = h_, s_ encodes the first child).
  int s_; // Index of the next sibling (if s_ = 0 or length, m_ encodes the last child).
  int modifier_label_;
  int sibling_label_;
};

class DependencyLabelerParts : public Parts {
 public:
  DependencyLabelerParts() {};
  virtual ~DependencyLabelerParts() { DeleteAll(); };

  void Initialize() {
    DeleteAll();
    for (int i = 0; i < NUM_DEPENDENCYLABELERPARTS; ++i) {
      offsets_[i] = -1;
    }
  }

  Part *CreatePartArc(int head, int modifier, int label) {
    return new DependencyLabelerPartArc(head, modifier, label);
  }
  Part *CreatePartSibling(int head, int modifier, int sibling,
                          int modifier_label, int sibling_label) {
    return new DependencyLabelerPartSibling(head, modifier, sibling,
                                            modifier_label, sibling_label);
  }

  void Save(FILE* fs) {
    CHECK(false) << "To be implemented.";
  }
  void Load(FILE* fs) {
    CHECK(false) << "To be implemented.";
  }

 public:
  void DeleteAll();

 public:
  void BuildArcIndices(const std::vector<int> &heads);
  void BuildSiblingIndices(const std::vector<int> &heads);
  void DeleteArcIndices();
  void DeleteSiblingIndices();

  // Set/Get offsets:
  void BuildOffsets() {
    for (int i = NUM_DEPENDENCYLABELERPARTS - 1; i >= 0; --i) {
      if (offsets_[i] < 0) {
        offsets_[i] = (i == NUM_DEPENDENCYLABELERPARTS - 1)? size() : offsets_[i + 1];
      }
    }
  };

  void SetOffsetArc(int offset, int size) {
    SetOffset(DEPENDENCYLABELERPART_ARC, offset, size);
  };
  void SetOffsetSibling(int offset, int size) {
    SetOffset(DEPENDENCYLABELERPART_SIBLING, offset, size);
  };

  void GetOffsetArc(int *offset, int *size) const {
    GetOffset(DEPENDENCYLABELERPART_ARC, offset, size);
  };
  void GetOffsetSibling(int *offset, int *size) const {
    GetOffset(DEPENDENCYLABELERPART_SIBLING, offset, size);
  };

  void ComputeSiblings(const std::vector<int> &heads);

  const vector<int> &FindArcs(int m) {
    return index_arcs_[m];
  }
  const vector<int> &FindSiblings(int h, int i) {
    return index_siblings_[h][i];
  }
  int GetSiblingIndex(int h, int m) {
    return (m < 0)? siblings_[h].size() : map_siblings_[h][m];
  }
  const std::vector<std::vector<int> > &siblings() { return siblings_; }

 private:
  // Get offset from part index.
  void GetOffset(int i, int *offset, int *size) const {
    *offset = offsets_[i];
    *size =  (i < NUM_DEPENDENCYLABELERPARTS - 1)? offsets_[i + 1] - (*offset) :
      DependencyLabelerParts::size() - (*offset);
  }

  // Set offset from part index.
  void SetOffset(int i, int offset, int size) {
    offsets_[i] = offset;
    if (i < NUM_DEPENDENCYLABELERPARTS - 1) offsets_[i + 1] = offset + size;
  }

 private:
  std::vector<std::vector<int> > index_arcs_;
  std::vector<std::vector<std::vector<int> > > index_siblings_;
  std::vector<std::vector<int> > map_siblings_;
  std::vector<std::vector<int> > siblings_;
  int offsets_[NUM_DEPENDENCYLABELERPARTS];
};

#endif /* DEPENDENCYLABELERPART_H_ */
