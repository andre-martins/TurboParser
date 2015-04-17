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

#include "Mention.h"
#include "CoreferenceSentenceNumeric.h"

void Mention::ComputeProperties(const CoreferenceDictionary &dictionary,
                                CoreferenceSentenceNumeric *sentence) {
  sentence_ = sentence;
  ComputeHead();

  // Compute mention type.
  type_ = -1;
  entity_tag_ = -1;
  NumericSpan *entity_span =
    static_cast<NumericSpan*>(FindCoveringSpan(sentence->GetEntitySpans()));
  if (entity_span) {
    type_ = MENTION_PROPER;
    entity_tag_ = entity_span->id();
  }

  int head_word = sentence->GetFormLowerCase(head_index_);
  int head_tag = sentence->GetPosTag(head_index_);
  if (dictionary->IsPronounWord(head_word) ||
      dictionary->IsPronounTag(head_tag)) {
    type_ = MENTION_PRONOMINAL;
  } else if (dictionary->IsProperTag(head_tag)) {
    type_ = MENTION_PROPER;
  } else if (type_ < 0) {
    type_ = MENTION_NOMINAL;
  }

  // Compute gender and number.
  number_ = MENTION_NUMBER_SINGULAR;
  gender_ = MENTION_GENDER_MALE;
  if (type_ == MENTION_PRONOMINAL) {
    if (dictionary->IsMalePronoun(head_word)) {
      gender_ = MENTION_GENDER_MALE;
    } else if (dictionary->IsFemalePronoun(head_word)) {
      gender_ = MENTION_GENDER_FEMALE;
    } else if (dictionary->IsNeutralPronoun(head_word)) {
      gender_ = MENTION_GENDER_NEUTRAL;
    } else {
      gender_ = MENTION_GENDER_UNKNOWN;
    }
  } else {
    number_ = ComputeNumber(words_, words_lower_, head_index_ - start_);
    if (entity_tag_ >= 0 &&
        dictionary->IsEntityPersonTag(entity_tag_)) {
      gender_ = ComputePersonGender(words_, words_lower_, head_index_ - start_);
    } else {
      gender_ = ComputeNonPersonGender(words_, words_lower_,
                                       head_index_ - start_);
    }
  }
}

void Mention::ComputeHead() {
  // If it's a constituent, only one word should have its head outside the span.
  // In general, we define as head the last word in the span whose head is
  // outside the span, and if there is none (which means there is a loop
  // somewhere, so should never happen) use the last word of the span.
  head_index_ = -1;
  for (int i = start_; i <= end_; ++i) {
    if (!ContainsIndex(sentence_->GetHead(i))) head_index_ = i;
  }
  if (head_index_ < 0) head_index_ = end_;
}

