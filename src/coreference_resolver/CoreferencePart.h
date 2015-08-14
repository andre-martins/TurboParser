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

#ifndef COREFERENCEPART_H_
#define COREFERENCEPART_H_

#include <stdio.h>
#include <vector>
#include "Part.h"

using namespace std;

enum {
  COREFERENCEPART_ARC = 0,
  NUM_COREFERENCEPARTS
};

class CoreferencePartArc : public Part {
 public:
  CoreferencePartArc() { parent_mention_ = child_mention_ = -1; };
  CoreferencePartArc(int parent_mention, int child_mention) :
    parent_mention_(parent_mention), child_mention_(child_mention) {};
  virtual ~CoreferencePartArc() {};

 public:
  int parent_mention() { return parent_mention_; };
  int child_mention() { return child_mention_; };

 public:
  int type() { return COREFERENCEPART_ARC; };

 private:
  int parent_mention_; // Antecendent mention (-1 if non-anaphoric).
  int child_mention_; // Current mention.
};


class CoreferenceParts : public Parts {
 public:
  CoreferenceParts() {};
  virtual ~CoreferenceParts() { DeleteAll(); };

  void Initialize() {
    DeleteAll();
  };

  Part *CreatePartArc(int parent_mention, int child_mention) {
    return new CoreferencePartArc(parent_mention, child_mention);
  }

 public:
  void DeleteAll();

 public:
  void BuildIndices(int num_mentions);
  void DeleteIndices();
  const std::vector<int> &FindArcParts(int child_mention) {
    return index_[child_mention];
  }

 private:
  std::vector<vector<int> >  index_;
};

#endif /* COREFERENCEPART_H_ */
