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
#include <algorithm>

void Mention::ComputeProperties(const CoreferenceDictionary &dictionary,
                                CoreferenceSentence* instance,
                                CoreferenceSentenceNumeric *sentence) {
  sentence_ = sentence;
  ComputeHead();

  // Compute mention type.
  type_ = -1;
  entity_tag_ = -1;
  std::vector<Span*> entity_spans(sentence->GetEntitySpans().begin(),
                                  sentence->GetEntitySpans().end());
  NumericSpan *entity_span =
    static_cast<NumericSpan*>(FindCoveringSpan(entity_spans));
  if (entity_span) {
    type_ = MentionType::PROPER;
    entity_tag_ = entity_span->id();
  }

  words_.assign(sentence->GetFormIds().begin() + start_,
                sentence->GetFormIds().begin() + end_);
  words_lower_.assign(sentence->GetFormLowerIds().begin() + start_,
                      sentence->GetFormLowerIds().begin() + end_);
  tags_.assign(sentence->GetPosIds().begin() + start_,
               sentence->GetPosIds().begin() + end_);

  int head_word = sentence->GetFormLowerId(head_index_);
  int head_tag = sentence->GetPosId(head_index_);
  if (dictionary.IsPronoun(head_word) ||
      dictionary.IsPronounTag(head_tag)) {
    type_ = MentionType::PRONOMINAL;
  } else if (dictionary.IsProperNoun(head_tag)) {
    type_ = MentionType::PROPER;
  } else if (type_ < 0) {
    type_ = MentionType::NOMINAL;
  }

  // Compute gender and number.
  number_ = MentionNumber::SINGULAR;
  gender_ = MentionGender::MALE;
  if (type_ == MentionType::PRONOMINAL) {
    if (dictionary.IsMalePronoun(head_word)) {
      gender_ = MentionGender::MALE;
    } else if (dictionary.IsFemalePronoun(head_word)) {
      gender_ = MentionGender::FEMALE;
    } else if (dictionary.IsNeutralPronoun(head_word)) {
      gender_ = MentionGender::NEUTRAL;
    } else {
      gender_ = MentionGender::UNKNOWN;
    }
    if (dictionary.IsSingularPronoun(head_word)) {
      number_ = MentionNumber::SINGULAR;
    } else if (dictionary.IsPluralPronoun(head_word)) {
      number_ = MentionNumber::PLURAL;
    } else {
      number_ = MentionNumber::UNKNOWN;
    }
  } else {
    std::vector<int> phrase;
    std::vector<int> phrase_lower;
    for (int i = start_; i <= end_; ++i) {
      const std::string &word = instance->GetForm(i);
      std::string word_lower(word);
      transform(word_lower.begin(), word_lower.end(), word_lower.begin(),
                ::tolower);
      int id = dictionary.GetWordAlphabet().Lookup(word);
      //if (id < 0) id = TOKEN_UNKNOWN;
      phrase.push_back(id);
      id = dictionary.GetWordLowerAlphabet().Lookup(word_lower);
      //if (id < 0) id = TOKEN_UNKNOWN;
      phrase_lower.push_back(id);
    }
    //number_ = ComputeNumber(phrase, phrase_lower, head_index_ - start_);
    number_ = dictionary.GetGenderNumberStatistics().
      ComputeNumber(phrase_lower, head_index_ - start_);
    if (entity_tag_ >= 0 &&
        dictionary.IsPersonEntity(entity_tag_)) {
      //gender_ = ComputePersonGender(phrase, phrase_lower,
      //                              head_index_ - start_);
      // If the head is upper case, assume it's a last name.
      if (sentence->FirstUpper(head_index_)) {
        // If the word before the head is upper case, assume it's a first name,
        // and decide based on that.
        if (head_index_ > 0 && sentence->FirstUpper(head_index_ - 1)) {
          gender_ = dictionary.GetGenderNumberStatistics().
            ComputeGender(phrase_lower, head_index_ - 1 - start_);
        } else {
          gender_ = dictionary.GetGenderNumberStatistics().
            ComputeGender(phrase_lower, head_index_ - start_);
        }
      } else {
        gender_ = dictionary.GetGenderNumberStatistics().
          ComputeGender(phrase_lower, head_index_ - start_);
      }
    } else {
      //gender_ = ComputeNonPersonGender(phrase, phrase_lower,
      //                                 head_index_ - start_);
      gender_ = dictionary.GetGenderNumberStatistics().
        ComputeGender(phrase_lower, head_index_ - start_);
    }
  }
}

void Mention::Print(const CoreferenceDictionary &dictionary,
                    CoreferenceSentence* instance) {
  LOG(INFO) << "-----------------------------------------";
  std::string word_sequence = "";
  std::string pos_sequence = "";
  for (int i = start_; i <= end_; ++i) {
    word_sequence += instance->GetForm(i) + " ";
    pos_sequence += instance->GetPosTag(i) + " ";
  }
  LOG(INFO) << word_sequence;
  LOG(INFO) << pos_sequence;
  LOG(INFO) << "Head: " << instance->GetForm(head_index_);
  LOG(INFO) << "Entity type: " << entity_tag_;
  if (type_ == MentionType::PRONOMINAL) {
    LOG(INFO) << "Type: pronominal";
  } else if (type_ == MentionType::PROPER) {
    LOG(INFO) << "Type: proper";
  } else if (type_ == MentionType::NOMINAL) {
    LOG(INFO) << "Type: nominal";
  } else {
    CHECK(false);
  }
  if (gender_ == MentionGender::MALE) {
    LOG(INFO) << "Gender: male";
  } else if (gender_ == MentionGender::FEMALE) {
    LOG(INFO) << "Gender: female";
  } else if (gender_ == MentionGender::NEUTRAL) {
    LOG(INFO) << "Gender: neutral";
  } else if (gender_ == MentionGender::UNKNOWN) {
    LOG(INFO) << "Gender: unknown";
  } else {
    CHECK(false) << gender_;
  }
  if (number_ == MentionNumber::SINGULAR) {
    LOG(INFO) << "Number: singular";
  } else if (number_ == MentionNumber::PLURAL) {
    LOG(INFO) << "Number: plural";
  } else if (number_ == MentionNumber::UNKNOWN) {
    LOG(INFO) << "Number: unknown";
  } else {
    CHECK(false) << number_;
  }
  LOG(INFO) << "-----------------------------------------";
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

