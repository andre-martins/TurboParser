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

#ifndef CONSTITUENCYLABELERPART_H_
#define CONSTITUENCYLABELERPART_H_

#include <stdio.h>
#include <vector>
#include "Part.h"

enum {
  CONSTITUENCYLABELERPART_NODE = 0,
  NUM_CONSTITUENCYLABELERPARTS
};

class ConstituencyLabelerPartNode : public Part {
 public:
  ConstituencyLabelerPartNode() { position_ = label_ = -1; };
 ConstituencyLabelerPartNode(int position, int label) :
    position_(position), label_(label) {};
  virtual ~ConstituencyLabelerPartNode() {};

 public:
  int position() { return position_; };
  int label() { return label_; };

 public:
  int type() { return CONSTITUENCYLABELERPART_NODE; };

 private:
  int position_; // Node position.
  int label_; // Label ID.
};

class ConstituencyLabelerParts : public Parts {
 public:
  ConstituencyLabelerParts() {};
  virtual ~ConstituencyLabelerParts() { DeleteAll(); };

  void Initialize() {
    DeleteAll();
    for (int i = 0; i < NUM_CONSTITUENCYLABELERPARTS; ++i) {
      offsets_[i] = -1;
    }
  };

  Part *CreatePartNode(int position, int label) {
    return new ConstituencyLabelerPartNode(position, label);
  }

 public:
  void DeleteAll();

 public:
  void BuildNodeIndices(int num_nodes);
  void BuildIndices(int num_nodes);
  void DeleteNodeIndices();
  void DeleteIndices();
  const std::vector<int> &FindNodeParts(int position) {
    return index_[position];
  }

  // Set/Get offsets:
  void BuildOffsets() {
    for (int i = NUM_CONSTITUENCYLABELERPARTS - 1; i >= 0; --i) {
      if (offsets_[i] < 0) {
        offsets_[i] = (i == NUM_CONSTITUENCYLABELERPARTS - 1)?
          size() : offsets_[i + 1];
      }
    }
  };
  void SetOffsetNode(int offset, int size) {
    SetOffset(CONSTITUENCYLABELERPART_NODE, offset, size);
  };
  void GetOffsetNode(int *offset, int *size) const {
    GetOffset(CONSTITUENCYLABELERPART_NODE, offset, size);
  };

 private:
  // Get offset from part index.
  void GetOffset(int i, int *offset, int *size) const {
    *offset = offsets_[i];
    *size =  (i < NUM_CONSTITUENCYLABELERPARTS - 1)?
      offsets_[i + 1] - (*offset) :
      ConstituencyLabelerParts::size() - (*offset);
  }

  // Set offset from part index.
  void SetOffset(int i, int offset, int size) {
    offsets_[i] = offset;
    if (i < NUM_CONSTITUENCYLABELERPARTS - 1) offsets_[i + 1] = offset + size;
  }

 private:
  std::vector<std::vector<int> >  index_;
  int offsets_[NUM_CONSTITUENCYLABELERPARTS];
};

#endif /* CONSTITUENCYLABELERPART_H_ */
