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

#ifndef COREFERENCESENTENCENUMERIC_H_
#define COREFERENCESENTENCENUMERIC_H_

#include "SemanticInstanceNumeric.h"
#include "CoreferenceSentence.h"
#include "CoreferenceDictionary.h"
#include "Mention.h"

class CoreferenceSentenceNumeric : public SemanticInstanceNumeric {
 public:
  CoreferenceSentenceNumeric() {};
  virtual ~CoreferenceSentenceNumeric() { Clear(); };

  Instance* Copy() {
    CHECK(false) << "Not implemented.";
    return NULL;
  }

  void Clear() {
    SemanticInstanceNumeric::Clear();
    DeleteAllSpans();
    DeleteMentions();
  }

  void Initialize(const CoreferenceDictionary &dictionary,
                  CoreferenceSentence *instance);

  const std::vector<NumericSpan*> &GetEntitySpans() {
    return entity_spans_;
  }

  // TODO(atm): this is duplicated from SequenceInstanceNumeric. It should
  // be inherited from that class.
  bool FirstUpper(int i) { return first_upper_[i]; }

 protected:
  void DeleteAllSpans() {
    for (int i = 0; i < entity_spans_.size(); ++i) {
      delete entity_spans_[i];
    }
    entity_spans_.clear();
    for (int i = 0; i < constituent_spans_.size(); ++i) {
      delete constituent_spans_[i];
    }
    constituent_spans_.clear();
    for (int i = 0; i < coreference_spans_.size(); ++i) {
      delete coreference_spans_[i];
    }
    coreference_spans_.clear();
  }

  void DeleteMentions() {
    for (int i = 0; i < mentions_.size(); ++i) {
      delete mentions_[i];
    }
    mentions_.clear();
  }

  // TODO(atm): this is duplicated from SequenceInstanceNumeric. It should
  // be inherited from that class.
  bool IsUpperCase(char c) { return (c >= 'A' && c <= 'Z'); }
  bool IsCapitalized(const char* word, int len) {
    if (len <= 0) return false;
    return IsUpperCase(word[0]);
  }

  void GenerateMentions(const CoreferenceDictionary &dictionary,
                        CoreferenceSentence *instance);

  void AddMention(const CoreferenceDictionary &dictionary,
                  CoreferenceSentence* instance,
                  int start, int end, int id);

 private:
  std::vector<NumericSpan*> entity_spans_;
  std::vector<NumericSpan*> constituent_spans_;
  std::vector<NumericSpan*> coreference_spans_;
  std::vector<Mention*> mentions_;
  // TODO(atm): this is duplicated from SequenceInstanceNumeric. It should
  // be inherited from that class.
  std::vector<bool> first_upper_;
};

#endif /* COREFERENCESENTENCENUMERIC_H_ */
