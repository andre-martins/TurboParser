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

#ifndef SEMANTICPART_H_
#define SEMANTICPART_H_

#include <stdio.h>
#include <vector>
#include "Part.h"

using namespace std;

enum {
  SEMANTICPART_PREDICATE = 0,
  SEMANTICPART_ARC,
  SEMANTICPART_LABELEDARC,
  SEMANTICPART_ARGUMENT,
  SEMANTICPART_SIBLING,
  NUM_SEMANTICPARTS
};

// Part for an unlabeled arc linking a predicate and an argument word.
class SemanticPartArc : public Part {
 public:
  SemanticPartArc() { p_ = a_ = s_ = -1; }
  SemanticPartArc(int predicate, int argument, int sense) :
    p_(predicate), a_(argument), s_(sense) {}
  virtual ~SemanticPartArc() {}

 public:
  int predicate() { return p_; }
  int argument() { return a_; }
  int sense() { return s_; }

 public:
  int type() { return SEMANTICPART_ARC; }

 private:
  int p_; // Index of the predicate.
  int a_; // Index of the argument.
  int s_; // Predicate sense.
};

// Part for a labeled arc linking a predicate and an argument word.
class SemanticPartLabeledArc : public Part {
 public:
  SemanticPartLabeledArc() { p_ = a_ = s_ = r_ = -1; }
  SemanticPartLabeledArc(int predicate, int argument, int sense, int role) :
    p_(predicate), a_(argument), s_(sense), r_(role) {}
  virtual ~SemanticPartLabeledArc() {}

 public:
  int predicate() { return p_; }
  int argument() { return a_; }
  int sense() { return s_; }
  int role() { return r_; }

 public:
  int type() { return SEMANTICPART_LABELEDARC; }

 private:
  int p_; // Index of the predicate.
  int a_; // Index of the argument.
  int s_; // Predicate sense.
  int r_; // Role label.
};


// Part for the event that a word is a predicate.
class SemanticPartPredicate : public Part {
 public:
  SemanticPartPredicate() { p_ = -1; s_ = -1; }
  SemanticPartPredicate(int predicate, int sense) :
    p_(predicate), s_(sense) {}
  virtual ~SemanticPartPredicate() {}

 public:
  int predicate() { return p_; }
  int sense() { return s_; }

 public:
  int type() { return SEMANTICPART_PREDICATE; }

 private:
  int p_; // Index of the predicate.
  int s_; // Index of the sense.
};


// Part for the event that a word is an argument of at least one predicate.
class SemanticPartArgument : public Part {
 public:
  SemanticPartArgument() { a_ = -1; }
  SemanticPartArgument(int argument) :
    a_(argument) {}
  virtual ~SemanticPartArgument() {}

 public:
  int argument() { return a_; }

 public:
  int type() { return SEMANTICPART_ARGUMENT; }

 private:
  int a_; // Index of the argument.
};


class SemanticPartSibling : public Part {
 public:
  SemanticPartSibling() { p_ = s_ = a1_ = a2_ = -1; };
  SemanticPartSibling(int predicate, int sense, int first_argument,
                      int second_argument) {
    p_ = predicate;
    s_ = sense;
    a1_ = first_argument;
    a2_ = second_argument;
  }
  virtual ~SemanticPartSibling() {};

 public:
  int type() { return SEMANTICPART_SIBLING; };

 public:
  int predicate() { return p_; };
  int sense() { return s_; };
  int first_argument() { return a1_; };
  int second_argument() { return a2_; };

 private:
  int p_; // Index of the predicate.
  int s_; // Index of the sense.
  int a1_; // Index of the first argument.
  int a2_; // Index of the second_argument.
};


class SemanticParts : public Parts {
 public:
  SemanticParts() {};
  virtual ~SemanticParts() { DeleteAll(); };

  void Initialize() {
    DeleteAll();
    for (int i = 0; i < NUM_SEMANTICPARTS; ++i) {
      offsets_[i] = -1;
    }
  }

  Part *CreatePartArc(int predicate, int argument, int sense) {
    return new SemanticPartArc(predicate, argument, sense);
  }
  Part *CreatePartLabeledArc(int predicate, int argument, int sense, int role) {
    return new SemanticPartLabeledArc(predicate, argument, sense, role);
  }
  Part *CreatePartPredicate(int predicate, int sense) {
    return new SemanticPartPredicate(predicate, sense);
  }
  Part *CreatePartArgument(int argument) {
    return new SemanticPartArgument(argument);
  }
  Part *CreatePartSibling(int predicate,
                          int sense,
                          int first_argument,
                          int second_argument) {
    return new SemanticPartSibling(predicate, sense, first_argument,
                                   second_argument);
  }

 public:
  void DeleteAll();

 public:
  void BuildIndices(int sentence_length, bool labeled);
  void DeleteIndices();
  const vector<int> &GetSenses(int predicate) {
    return index_senses_[predicate];
  }
  int FindArc(int predicate, int argument, int sense) {
    CHECK_GE(predicate, 0);
    CHECK_GE(argument, 0);
    CHECK_GE(sense, 0);
    CHECK_LT(predicate, index_.size());
    CHECK_LT(argument, index_[predicate].size());
    if (sense >= index_[predicate][argument].size()) {
      return -1;
    }
    return index_[predicate][argument][sense];
  }
  const vector<int> &FindLabeledArcs(int predicate, int argument, int sense) {
    CHECK_GE(predicate, 0);
    CHECK_GE(argument, 0);
    CHECK_GE(sense, 0);
    CHECK_LT(predicate, index_labeled_.size());
    CHECK_LT(argument, index_labeled_[predicate].size());
    CHECK_LT(sense, index_labeled_[predicate][argument].size());
    //if (sense >= index_labeled_[predicate][argument].size()) {
    //  return -1;
    //}
    return index_labeled_[predicate][argument][sense];
  }

  // True is model is arc-factored, i.e., all parts are predicate parts or
  // unlabeled arcs.
  // TODO: change this to incorporate predicate parts.
  bool IsArcFactored() {
    int offset, num_predicate_parts, num_arcs;
    GetOffsetPredicate(&offset, &num_predicate_parts);
    GetOffsetArc(&offset, &num_arcs);
    return (num_predicate_parts + num_arcs == size());
  }

  // True is model is arc-factored, i.e., all parts are unlabeled and labeled
  // arcs.
  // TODO: change this to incorporate predicate parts.
  bool IsLabeledArcFactored() {
    int offset, num_predicate_parts, num_arcs, num_labeled_arcs;
    GetOffsetPredicate(&offset, &num_predicate_parts);
    GetOffsetArc(&offset, &num_arcs);
    GetOffsetLabeledArc(&offset, &num_labeled_arcs);
    return (num_predicate_parts + num_arcs + num_labeled_arcs == size());
  }

  // Set/Get offsets:
  void ClearOffsets() {
    for (int i = 0; i < NUM_SEMANTICPARTS; ++i) {
      offsets_[i] = -1;
    }
  }

  void BuildOffsets() {
    for (int i = NUM_SEMANTICPARTS - 1; i >= 0; --i) {
      if (offsets_[i] < 0 || offsets_[i] > size()) {
        offsets_[i] = (i == NUM_SEMANTICPARTS - 1)? size() : offsets_[i + 1];
      }
    }
  };

  void SetOffsetLabeledArc(int offset, int size) {
    SetOffset(SEMANTICPART_LABELEDARC, offset, size);
  };
  void SetOffsetArc(int offset, int size) {
    SetOffset(SEMANTICPART_ARC, offset, size);
  };
  void SetOffsetPredicate(int offset, int size) {
    SetOffset(SEMANTICPART_PREDICATE, offset, size);
  };
  void SetOffsetArgument(int offset, int size) {
    SetOffset(SEMANTICPART_ARGUMENT, offset, size);
  };
  void SetOffsetSibling(int offset, int size) {
    SetOffset(SEMANTICPART_SIBLING, offset, size);
  };

  void GetOffsetLabeledArc(int *offset, int *size) const {
    GetOffset(SEMANTICPART_LABELEDARC, offset, size);
  };
  void GetOffsetArc(int *offset, int *size) const {
    GetOffset(SEMANTICPART_ARC, offset, size);
  };
  void GetOffsetPredicate(int *offset, int *size) const {
    GetOffset(SEMANTICPART_PREDICATE, offset, size);
  };
  void GetOffsetArgument(int *offset, int *size) const {
    GetOffset(SEMANTICPART_ARGUMENT, offset, size);
  };
  void GetOffsetSibling(int *offset, int *size) const {
    GetOffset(SEMANTICPART_SIBLING, offset, size);
  };

 private:
  // Get offset from part index.
  void GetOffset(int i, int *offset, int *size) const {
    *offset = offsets_[i];
    *size =  (i < NUM_SEMANTICPARTS - 1)? offsets_[i + 1] - (*offset) :
      SemanticParts::size() - (*offset);
  }

  // Set offset from part index.
  void SetOffset(int i, int offset, int size) {
    offsets_[i] = offset;
    if (i < NUM_SEMANTICPARTS - 1) offsets_[i + 1] = offset + size;
  }

 private:
  // Sense IDs of each predicate.
  vector<vector<int> > index_senses_;
  // Maps a triple (p, a, s) to a SemanticPartArc index.
  vector<vector<vector<int> > > index_;
  // Maps a quadruple (p, a, s, r) to a SemanticPartLabeledArc index.
  vector<vector<vector<vector<int> > > > index_labeled_;
  int offsets_[NUM_SEMANTICPARTS];
};

#endif /* SEMANTICPART_H_ */
