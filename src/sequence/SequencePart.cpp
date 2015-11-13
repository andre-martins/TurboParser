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

#include "SequencePart.h"

void SequenceParts::DeleteAll() {
  for (int i = 0; i < NUM_SEQUENCEPARTS; ++i) {
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

void SequenceParts::DeleteUnigramIndices() {
  for (int i = 0; i < index_.size(); ++i) {
    index_[i].clear();
  }
  index_.clear();
}

void SequenceParts::DeleteBigramIndices() {
  for (int i = 0; i < index_bigrams_.size(); ++i) {
    index_bigrams_[i].clear();
  }
  index_bigrams_.clear();
}

void SequenceParts::DeleteTrigramIndices() {
  for (int i = 0; i < index_trigrams_.size(); ++i) {
    index_trigrams_[i].clear();
  }
  index_trigrams_.clear();
}

void SequenceParts::DeleteIndices() {
  DeleteUnigramIndices();
  DeleteBigramIndices();
  DeleteTrigramIndices();
}

void SequenceParts::BuildUnigramIndices(int sentence_length) {
  DeleteUnigramIndices();
  index_.resize(sentence_length);

  int offset, num_unigram_parts;
  GetOffsetUnigram(&offset, &num_unigram_parts);
  for (int r = 0; r < num_unigram_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == SEQUENCEPART_UNIGRAM);
    int i = static_cast<SequencePartUnigram*>(part)->position();
    index_[i].push_back(offset + r);
  }
}

void SequenceParts::BuildBigramIndices(int sentence_length) {
  DeleteBigramIndices();
  index_bigrams_.resize(sentence_length + 1);

  int offset, num_bigram_parts;
  GetOffsetBigram(&offset, &num_bigram_parts);
  for (int r = 0; r < num_bigram_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == SEQUENCEPART_BIGRAM);
    int i = static_cast<SequencePartBigram*>(part)->position();
    index_bigrams_[i].push_back(offset + r);
  }
}

void SequenceParts::BuildTrigramIndices(int sentence_length) {
  DeleteTrigramIndices();
  index_trigrams_.resize(sentence_length + 1);

  int offset, num_trigram_parts;
  GetOffsetTrigram(&offset, &num_trigram_parts);
  for (int r = 0; r < num_trigram_parts; ++r) {
    Part *part = (*this)[offset + r];
    CHECK(part->type() == SEQUENCEPART_TRIGRAM);
    int i = static_cast<SequencePartTrigram*>(part)->position();
    index_trigrams_[i].push_back(offset + r);
  }
}

void SequenceParts::BuildIndices(int sentence_length) {
  BuildUnigramIndices(sentence_length);
  BuildBigramIndices(sentence_length);
  BuildTrigramIndices(sentence_length);
}
