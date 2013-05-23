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

#ifndef SEQUENCEPART_H_
#define SEQUENCEPART_H_

#include <stdio.h>
#include <vector>
#include "Part.h"

using namespace std;

enum {
  SEQUENCEPART_UNIGRAM = 0,
  SEQUENCEPART_BIGRAM,
  SEQUENCEPART_TRIGRAM,
  NUM_SEQUENCEPARTS
};

class SequencePartUnigram : public Part {
 public:
  SequencePartUnigram() { position_ = tag_ = -1; };
  SequencePartUnigram(int position, int tag) :
    position_(position), tag_(tag) {};
  virtual ~SequencePartUnigram() {};

 public:
  int position() { return position_; };
  int tag() { return tag_; };

 public:
  int type() { return SEQUENCEPART_UNIGRAM; };

 private:
  int position_; // Word position.
  int tag_; // Tag ID.
};

class SequencePartBigram : public Part {
 public:
  SequencePartBigram() { position_ = tag_ = tag_left_ = -1; };
  SequencePartBigram(int position, int tag, int tag_left) :
    position_(position), tag_(tag), tag_left_(tag_left) {};
  virtual ~SequencePartBigram() {};

 public:
  int position() { return position_; };
  int tag() { return tag_; };
  int tag_left() { return tag_left_; };

 public:
  int type() { return SEQUENCEPART_BIGRAM; };

 private:
  int position_; // Word position.
  int tag_; // Tag ID.
  int tag_left_; // Left tag ID
};

class SequencePartTrigram : public Part {
 public:
  SequencePartTrigram() {
    position_ = tag_ = tag_left_ = tag_left_left_ = -1;
  };
  SequencePartTrigram(int position, int tag, int tag_left, int tag_left_left) :
    position_(position), tag_(tag), tag_left_(tag_left),
    tag_left_left_(tag_left_left) {};
  virtual ~SequencePartTrigram() {};

 public:
  int position() { return position_; };
  int tag() { return tag_; };
  int tag_left() { return tag_left_; };
  int tag_left_left() { return tag_left_left_; };

 public:
  int type() { return SEQUENCEPART_TRIGRAM; };

 private:
  int position_; // Word position.
  int tag_; // Tag ID.
  int tag_left_; // Left tag ID
  int tag_left_left_; // Tag ID two words on the left.
};


class SequenceParts : public Parts {
 public:
  SequenceParts() {};
  virtual ~SequenceParts() { DeleteAll(); };

  void Initialize() {
    DeleteAll();
    for (int i = 0; i < NUM_SEQUENCEPARTS; ++i) {
      offsets_[i] = -1;
    }
  };

  Part *CreatePartUnigram(int position, int tag) {
    return new SequencePartUnigram(position, tag);
  }
  Part *CreatePartBigram(int position, int tag, int tag_left) {
    return new SequencePartBigram(position, tag, tag_left);
  }
  Part *CreatePartTrigram(int position, int tag, int tag_left,
                          int tag_left_left) {
    return new SequencePartTrigram(position, tag, tag_left, tag_left_left);
  }

 public:
  void DeleteAll();

 public:
  void BuildUnigramIndices(int sentence_length);
  void BuildBigramIndices(int sentence_length);
  void BuildTrigramIndices(int sentence_length);
  void BuildIndices(int sentence_length);
  void DeleteUnigramIndices();
  void DeleteBigramIndices();
  void DeleteTrigramIndices();
  void DeleteIndices();
  const vector<int> &FindUnigramParts(int position) {
    return index_[position];
  }
  const vector<int> &FindBigramParts(int position) {
    return index_bigrams_[position];
  }
  const vector<int> &FindTrigramParts(int position) {
    return index_trigrams_[position];
  }

  // Set/Get offsets:
  void BuildOffsets() {
    for (int i = NUM_SEQUENCEPARTS - 1; i >= 0; --i) {
      if (offsets_[i] < 0) {
        offsets_[i] = (i == NUM_SEQUENCEPARTS - 1)? size() : offsets_[i + 1];
      }
    }
  };
  void SetOffsetUnigram(int offset, int size) {
    SetOffset(SEQUENCEPART_UNIGRAM, offset, size);
  };
  void SetOffsetBigram(int offset, int size) {
    SetOffset(SEQUENCEPART_BIGRAM, offset, size);
  };
  void SetOffsetTrigram(int offset, int size) {
    SetOffset(SEQUENCEPART_TRIGRAM, offset, size);
  };
  void GetOffsetUnigram(int *offset, int *size) const {
    GetOffset(SEQUENCEPART_UNIGRAM, offset, size);
  };
  void GetOffsetBigram(int *offset, int *size) const {
    GetOffset(SEQUENCEPART_BIGRAM, offset, size);
  };
  void GetOffsetTrigram(int *offset, int *size) const {
    GetOffset(SEQUENCEPART_TRIGRAM, offset, size);
  };

 private:
  // Get offset from part index.
  void GetOffset(int i, int *offset, int *size) const {
    *offset = offsets_[i];
    *size =  (i < NUM_SEQUENCEPARTS - 1)? offsets_[i + 1] - (*offset) :
      SequenceParts::size() - (*offset);
  }

  // Set offset from part index.
  void SetOffset(int i, int offset, int size) {
    offsets_[i] = offset;
    if (i < NUM_SEQUENCEPARTS - 1) offsets_[i + 1] = offset + size;
  }

 private:
  vector<vector<int> >  index_;
  vector<vector<int> >  index_bigrams_;
  vector<vector<int> >  index_trigrams_;
  int offsets_[NUM_SEQUENCEPARTS];
};

#endif /* SEQUENCEPART_H_ */
