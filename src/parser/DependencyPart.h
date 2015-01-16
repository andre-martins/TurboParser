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

#ifndef DEPENDENCYPART_H_
#define DEPENDENCYPART_H_

#include <stdio.h>
#include <vector>
#include "Part.h"

using namespace std;

enum {
  DEPENDENCYPART_ARC = 0,
  DEPENDENCYPART_LABELEDARC,
  DEPENDENCYPART_SIBL,
  DEPENDENCYPART_NEXTSIBL,
  DEPENDENCYPART_GRANDPAR,
  DEPENDENCYPART_GRANDSIBL,
  DEPENDENCYPART_TRISIBL,
  DEPENDENCYPART_NONPROJ,
  DEPENDENCYPART_PATH,
  DEPENDENCYPART_HEADBIGRAM,
  DEPENDENCYPART_LABELEDSIBL,
  NUM_DEPENDENCYPARTS
};

class DependencyPartArc : public Part {
 public:
  DependencyPartArc() { h_ = m_ = -1; };
  DependencyPartArc(int head, int modifier) : h_(head), m_(modifier) {};
  virtual ~DependencyPartArc() {};

 public:
  int head() { return h_; };
  int modifier() { return m_; };

 public:
  int type() { return DEPENDENCYPART_ARC; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier.
};

class DependencyPartLabeledArc : public Part {
 public:
  DependencyPartLabeledArc() { h_ = m_ = label_ = -1; };
  DependencyPartLabeledArc(int head, int modifier, int label) {
    h_ = head;
    m_ = modifier;
    label_ = label;
  }
  virtual ~DependencyPartLabeledArc() {};

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int label() { return label_; };

 public:
  int type() { return DEPENDENCYPART_LABELEDARC; };

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

class DependencyPartSibl : public Part {
 public:
  DependencyPartSibl() { h_ = m_ = s_ = -1; };
  DependencyPartSibl(int head, int modifier, int sibling) {
    h_ = head;
    m_ = modifier;
    s_ = sibling;
  }
  virtual ~DependencyPartSibl() {};

 public:
  int type() { return DEPENDENCYPART_SIBL; };

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int sibling() { return s_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&s_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&s_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier.
  int s_; // Index of the sibling.
};

class DependencyPartNextSibl : public Part {
 public:
  DependencyPartNextSibl() { h_ = m_ = s_ = -1; };
  DependencyPartNextSibl(int head, int modifier, int sibling) {
    h_ = head;
    m_ = modifier;
    s_ = sibling;
  }
  virtual ~DependencyPartNextSibl() {};

 public:
  int type() { return DEPENDENCYPART_NEXTSIBL; };

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int next_sibling() { return s_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&s_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&s_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier (if m_ = h_, s_ encodes the first child).
  int s_; // Index of the next sibling (if s_ = 0 or length, m_ encodes the last child).
};

class DependencyPartGrandpar : public Part {
 public:
  DependencyPartGrandpar() { g_ = h_ = m_ = -1; };
  DependencyPartGrandpar(int grandparent, int head, int modifier) {
    g_ = grandparent;
    h_ = head;
    m_ = modifier;
  }
  virtual ~DependencyPartGrandpar() {};

 public:
  int type() { return DEPENDENCYPART_GRANDPAR; };

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int grandparent() { return g_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&g_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&g_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int g_; // Index of the grandparent.
  int h_; // Index of the head.
  int m_; // Index of the modifier.
};

class DependencyPartGrandSibl : public Part {
 public:
  DependencyPartGrandSibl() { g_ = h_ = m_ = s_ = -1; };
  DependencyPartGrandSibl(int grandparent, int head, int modifier, int sibling) {
    g_ = grandparent;
    h_ = head;
    m_ = modifier;
    s_ = sibling;
  }
  virtual ~DependencyPartGrandSibl() {};

 public:
  int type() { return DEPENDENCYPART_GRANDSIBL; };

 public:
  int grandparent() { return g_; };
  int head() { return h_; };
  int modifier() { return m_; };
  int sibling() { return s_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&g_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&s_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&g_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&s_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int g_; // Index of the grandparent.
  int h_; // Index of the head.
  int m_; // Index of the modifier.
  int s_; // Index of the sibling.
};

class DependencyPartTriSibl : public Part {
 public:
  DependencyPartTriSibl() { h_ = m_ = s_ = t_ = -1; };
  DependencyPartTriSibl(int head, int modifier, int sibling, int other_sibling) {
    h_ = head;
    m_ = modifier;
    s_ = sibling;
    t_ = other_sibling;
  }
  virtual ~DependencyPartTriSibl() {};

 public:
  int type() { return DEPENDENCYPART_TRISIBL; };

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int sibling() { return s_; };
  int other_sibling() { return t_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&s_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&t_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&s_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&t_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier.
  int s_; // Index of the sibling.
  int t_; // Index of the other sibling.
};

class DependencyPartNonproj : public Part {
 public:
  DependencyPartNonproj() { h_ = m_ = -1; };
  DependencyPartNonproj(int head, int modifier) {
    h_ = head;
    m_ = modifier;
  }
  virtual ~DependencyPartNonproj() {};

 public:
  int type() {return DEPENDENCYPART_NONPROJ;};

 public:
  int head() { return h_; };
  int modifier() { return m_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier.
};


class DependencyPartPath : public Part {
 public:
  DependencyPartPath() { a_ = d_ = -1; };
  DependencyPartPath(int ancestor, int descendant) {
    a_ = ancestor;
    d_ = descendant;
  }
  virtual ~DependencyPartPath() {};

 public:
  int type() { return DEPENDENCYPART_PATH; };

 public:
  int ancestor() { return a_; };
  int descendant() { return d_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&a_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&d_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&a_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&d_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int a_; // Index of the ancestor.
  int d_; // Index of the descendant.
};

class DependencyPartHeadBigram : public Part {
 public:
  DependencyPartHeadBigram() { h_ = m_ = -1; };
  DependencyPartHeadBigram(int head, int modifier, int previous_head) {
    h_ = head;
    m_ = modifier;
    hp_ = previous_head;
  }
  virtual ~DependencyPartHeadBigram() {};

 public:
  int type() { return DEPENDENCYPART_HEADBIGRAM; };

 public:
  int head() { return h_; };
  int modifier() { return m_; };
  int previous_head() { return hp_; };

 public:
  void Save(FILE *fs) {
    if (1 != fwrite(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fwrite(&hp_, sizeof(int), 1, fs)) CHECK(false);
  };

  void Load(FILE *fs) {
    if (1 != fread(&h_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&m_, sizeof(int), 1, fs)) CHECK(false);
    if (1 != fread(&hp_, sizeof(int), 1, fs)) CHECK(false);
  };

 private:
  int h_; // Index of the head.
  int m_; // Index of the modifier.
  int hp_; // Index of the head of the previous word (m_ - 1).
};

class DependencyPartLabeledSibl : public Part {
 public:
  DependencyPartLabeledSibl() { h_ = m_ = s_ = -1; };
  DependencyPartLabeledSibl(int head, int modifier, int sibling,
                            int modifier_label, int sibling_label) {
    h_ = head;
    m_ = modifier;
    s_ = sibling;
    modifier_label_ = modifier_label;
    sibling_label_ = sibling_label;
  }
  virtual ~DependencyPartLabeledSibl() {};

 public:
  int type() { return DEPENDENCYPART_LABELEDSIBL; };

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

class DependencyParts : public Parts {
 public:
  DependencyParts() {};
  virtual ~DependencyParts() { DeleteAll(); };

  void Initialize() {
    DeleteAll();
    for (int i = 0; i < NUM_DEPENDENCYPARTS; ++i) {
      offsets_[i] = -1;
    }
  }

  Part *CreatePartArc(int head, int modifier) {
    return new DependencyPartArc(head, modifier);
  }
  Part *CreatePartLabeledArc(int head, int modifier, int label) {
    return new DependencyPartLabeledArc(head, modifier, label);
  }
  Part *CreatePartSibl(int head, int modifier, int sibling) {
    return new DependencyPartSibl(head, modifier, sibling);
  }
  Part *CreatePartNextSibl(int head, int modifier, int sibling) {
    return new DependencyPartNextSibl(head, modifier, sibling);
  }
  Part *CreatePartGrandpar(int grandparent, int head, int modifier) {
    return new DependencyPartGrandpar(grandparent, head, modifier);
  }
  Part *CreatePartGrandSibl(int grandparent, int head, int modifier, int sibling) {
    return new DependencyPartGrandSibl(grandparent, head, modifier, sibling);
  }
  Part *CreatePartTriSibl(int head, int modifier, int sibling, int other_sibling) {
    return new DependencyPartTriSibl(head, modifier, sibling, other_sibling);
  }
  Part *CreatePartNonproj(int head, int modifier) {
    return new DependencyPartNonproj(head, modifier);
  }
  Part *CreatePartPath(int ancestor, int descendant) {
    return new DependencyPartPath(ancestor, descendant);
  }
  Part *CreatePartHeadBigram(int head, int modifier, int previous_head) {
    return new DependencyPartHeadBigram(head, modifier, previous_head);
  }
  Part *CreatePartLabeledSibl(int head, int modifier, int sibling,
                              int modifier_label, int sibling_label) {
    return new DependencyPartLabeledSibl(head, modifier, sibling,
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
  void BuildIndices(int sentence_length, bool labeled);
  void DeleteIndices();
  int FindArc(int head, int modifier) { return index_[head][modifier]; };
  const vector<int> &FindLabeledArcs(int head, int modifier) {
    return index_labeled_[head][modifier];
  }

  // True is model is arc-factored, i.e., all parts are unlabeled arcs.
  bool IsArcFactored() {
    int offset, num_arcs;
    GetOffsetArc(&offset, &num_arcs);
    return (num_arcs == size());
  }

  // True is model is arc-factored, i.e., all parts are unlabeled and labeled
  // arcs.
  bool IsLabeledArcFactored() {
    int offset, num_arcs, num_labeled_arcs;
    GetOffsetArc(&offset, &num_arcs);
    GetOffsetLabeledArc(&offset, &num_labeled_arcs);
    return (num_arcs + num_labeled_arcs == size());
  }

  // Set/Get offsets:
  void BuildOffsets() {
    for (int i = NUM_DEPENDENCYPARTS - 1; i >= 0; --i) {
      if (offsets_[i] < 0) {
        offsets_[i] = (i == NUM_DEPENDENCYPARTS - 1)? size() : offsets_[i + 1];
      }
    }
  };

  void SetOffsetLabeledArc(int offset, int size) {
    SetOffset(DEPENDENCYPART_LABELEDARC, offset, size);
  };
  void SetOffsetArc(int offset, int size) {
    SetOffset(DEPENDENCYPART_ARC, offset, size);
  };
  void SetOffsetSibl(int offset, int size) {
    SetOffset(DEPENDENCYPART_SIBL, offset, size);
  };
  void SetOffsetNextSibl(int offset, int size) {
    SetOffset(DEPENDENCYPART_NEXTSIBL, offset, size);
  };
  void SetOffsetGrandpar(int offset, int size) {
    SetOffset(DEPENDENCYPART_GRANDPAR, offset, size);
  };
  void SetOffsetGrandSibl(int offset, int size) {
    SetOffset(DEPENDENCYPART_GRANDSIBL, offset, size);
  };
  void SetOffsetTriSibl(int offset, int size) {
    SetOffset(DEPENDENCYPART_TRISIBL, offset, size);
  };
  void SetOffsetNonproj(int offset, int size) {
    SetOffset(DEPENDENCYPART_NONPROJ, offset, size);
  };
  void SetOffsetPath(int offset, int size) {
    SetOffset(DEPENDENCYPART_PATH, offset, size);
  };
  void SetOffsetHeadBigr(int offset, int size) {
    SetOffset(DEPENDENCYPART_HEADBIGRAM, offset, size);
  };
  void SetOffsetLabeledSibl(int offset, int size) {
    SetOffset(DEPENDENCYPART_LABELEDSIBL, offset, size);
  };

  void GetOffsetLabeledArc(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_LABELEDARC, offset, size);
  };
  void GetOffsetArc(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_ARC, offset, size);
  };
  void GetOffsetSibl(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_SIBL, offset, size);
  };
  void GetOffsetNextSibl(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_NEXTSIBL, offset, size);
  };
  void GetOffsetGrandpar(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_GRANDPAR, offset, size);
  };
  void GetOffsetGrandSibl(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_GRANDSIBL, offset, size);
  };
  void GetOffsetTriSibl(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_TRISIBL, offset, size);
  };
  void GetOffsetNonproj(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_NONPROJ, offset, size);
  };
  void GetOffsetPath(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_PATH, offset, size);
  };
  void GetOffsetHeadBigr(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_HEADBIGRAM, offset, size);
  };
  void GetOffsetLabeledSibl(int *offset, int *size) const {
    GetOffset(DEPENDENCYPART_LABELEDSIBL, offset, size);
  };

 private:
  // Get offset from part index.
  void GetOffset(int i, int *offset, int *size) const {
    *offset = offsets_[i];
    *size =  (i < NUM_DEPENDENCYPARTS - 1)? offsets_[i + 1] - (*offset) :
      DependencyParts::size() - (*offset);
  }

  // Set offset from part index.
  void SetOffset(int i, int offset, int size) {
    offsets_[i] = offset;
    if (i < NUM_DEPENDENCYPARTS - 1) offsets_[i + 1] = offset + size;
  }

 private:
  vector<vector<int> >  index_;
  vector<vector<vector<int> > > index_labeled_;
  int offsets_[NUM_DEPENDENCYPARTS];
};

#endif /* DEPENDENCYPART_H_ */
