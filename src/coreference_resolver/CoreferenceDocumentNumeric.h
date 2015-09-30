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

#ifndef COREFERENCEDOCUMENTNUMERIC_H_
#define COREFERENCEDOCUMENTNUMERIC_H_

#include "CoreferenceSentenceNumeric.h"
#include "CoreferenceDocument.h"
#include "CoreferenceDictionary.h"

class CoreferenceDocumentNumeric : public Instance {
 public:
  CoreferenceDocumentNumeric() { conversation_ = false; };
  virtual ~CoreferenceDocumentNumeric() { Clear(); };

  Instance* Copy() {
    CHECK(false) << "Not implemented.";
    return NULL;
  }

  void Clear() {
    sentence_cumulative_lengths_.clear();
    for (int i = 0; i < coreference_spans_.size(); ++i) {
      delete coreference_spans_[i];
    }
    coreference_spans_.clear();
    // TODO(atm): if document owns mentions, they should be deleted here.
    mentions_.clear();
    entity_clusters_.clear();
    DeleteAllSentences();
  }

  void Initialize(const CoreferenceDictionary &dictionary,
                  CoreferenceDocument *instance,
                  bool add_gold_mentions);

  // True if document is a conversation.
  bool is_conversation() { return conversation_; }

  // Returns the number of sentences in the document.
  int GetNumSentences() { return sentences_.size(); }

  // Returns the number of words in the document.
  int GetNumWords() { return sentence_cumulative_lengths_.back(); }

  // Returns the i-th sentence.
  CoreferenceSentenceNumeric *GetSentence(int i) { return sentences_[i]; }

  // Returns the mentions.
  const std::vector<Mention*> &GetMentions() { return mentions_; }

  // Returns the mentions.
  const std::vector<std::vector<int> > &GetEntityClusters() {
    return entity_clusters_;
  }

  bool IsMentionAnaphoric(int j) {
    return mentions_[j]->id() >= 0 &&
      entity_clusters_[mentions_[j]->id()][0] != j;
  }

  // Get the sentence to which a word belongs.
  // Note: this takes linear time w.r.t. the number of sentences.
  void FindSentencePosition(int word_document_index, int *sentence_index,
                            int *word_sentence_index) {
    *sentence_index = -1;
    for (int i = 0; i < sentence_cumulative_lengths_.size(); ++i) {
      ++(*sentence_index);
      if (word_document_index < sentence_cumulative_lengths_[i]) break;
    }
    if (*sentence_index > 0) {
      *word_sentence_index =
        word_document_index - sentence_cumulative_lengths_[*sentence_index - 1];
    } else {
      *word_sentence_index = word_document_index;
    }
    // Increment the word index since sentences have a start symbol.
    ++(*word_sentence_index);
  }

  // Return the global word position at document level.
  int GetDocumentPosition(int sentence_index, int word_sentence_index) {
    // Subtract 1 since sentences have a start symbol.
    if (sentence_index == 0) {
      return word_sentence_index - 1;
    } else {
      return word_sentence_index - 1 +
        sentence_cumulative_lengths_[sentence_index - 1];
    }
  }

 protected:
  void DeleteAllSentences() {
    for (int i = 0; i < sentences_.size(); ++i) {
      delete sentences_[i];
    }
    sentences_.clear();
  }

  void ComputeEntityClusters();

  void ComputeGlobalWordPositions(CoreferenceDocument* instance);

 private:
  bool conversation_;
  std::vector<CoreferenceSentenceNumeric*> sentences_;
  std::vector<int> sentence_cumulative_lengths_;
  std::vector<NumericSpan*> coreference_spans_;
  std::vector<Mention*> mentions_;
  std::vector<std::vector<int> > entity_clusters_;
};

#endif /* COREFERENCEDOCUMENTNUMERIC_H_ */
