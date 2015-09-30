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
#include "CoreferencePipe.h"
#include <algorithm>

const int kUnknownUnigramAncestry = 0xffff;
const int kUnknownBigramAncestry = 0xffff;

void Mention::ComputeProperties(const CoreferenceDictionary &dictionary,
                                CoreferenceSentence* instance,
                                CoreferenceSentenceNumeric *sentence) {
  CoreferenceOptions *options =
    static_cast<CoreferencePipe*>(dictionary.GetPipe())->
    GetCoreferenceOptions();

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

  int head_tag = sentence->GetPosId(head_index_);
  int head_form = sentence->GetFormLowerId(head_index_);

  const std::string &word = instance->GetForm(head_index_);
  std::string word_lower(word);
  transform(word_lower.begin(), word_lower.end(), word_lower.begin(),
            ::tolower);
  int id = dictionary.GetWordLowerAlphabet().Lookup(word_lower);
  //if (id < 0) id = TOKEN_UNKNOWN;
  int head_word = id; // This uses the extended word dictionary.

  if (dictionary.IsPronoun(head_word) ||
      dictionary.IsPronounTag(head_tag)) {
    type_ = MentionType::PRONOMINAL;
  } else if (dictionary.IsProperNoun(head_tag)) {
    type_ = MentionType::PROPER;
  } else if (type_ < 0) {
    type_ = MentionType::NOMINAL;
  }

  if (type_ == MentionType::PRONOMINAL) {
    pronoun_ = dictionary.GetPronoun(head_word);
  } else {
    pronoun_ = NULL;
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
    if (options->use_gender_number_determiners()) {
      // Compute number and gender by looking at the previous determiner form.
      int i = head_index_ - 1;
      if (i >= 0) {
        const std::string &word = instance->GetForm(i);
        std::string word_lower(word);
        transform(word_lower.begin(), word_lower.end(), word_lower.begin(),
                  ::tolower);
        int word_lower_id =
          dictionary.GetWordLowerAlphabet().Lookup(word_lower);
        number_ = MentionNumber::UNKNOWN;
        gender_ = MentionGender::UNKNOWN;
        if (dictionary.IsDeterminer(word_lower_id)) {
          if (dictionary.IsSingularDeterminer(word_lower_id)) {
            number_ = MentionNumber::SINGULAR;
          } else if (dictionary.IsPluralDeterminer(word_lower_id)) {
            number_ = MentionNumber::PLURAL;
          }
          if (dictionary.IsMaleDeterminer(word_lower_id)) {
            gender_ = MentionGender::MALE;
          } else if (dictionary.IsFemaleDeterminer(word_lower_id)) {
            gender_ = MentionGender::FEMALE;
          } else if (dictionary.IsNeutralDeterminer(word_lower_id)) {
            gender_ = MentionGender::NEUTRAL;
          }
        }
        if (number_ == MentionNumber::UNKNOWN) {
          const std::string &word = instance->GetForm(head_index_);
          // Plurals end in "s" for several languages.
          // TODO(atm): get rid of this hack.
          if (word.length() > 0 && word[word.length()-1] == 's') {
            number_ = MentionNumber::PLURAL;
          }
        }
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

  // Compute syntactic ancestry.
  std::string unigram_ancestry_string;
  std::string bigram_ancestry_string;
  dictionary.ComputeDependencyAncestryStrings(instance, head_index_,
                                              &unigram_ancestry_string,
                                              &bigram_ancestry_string);
  int unigram_ancestry_id =
    dictionary.GetUnigramAncestryAlphabet().Lookup(unigram_ancestry_string);
  CHECK_LT(unigram_ancestry_id, 0xffff);
  if (unigram_ancestry_id < 0) unigram_ancestry_id = kUnknownUnigramAncestry;
  unigram_ancestry_ = unigram_ancestry_id;

  int bigram_ancestry_id =
    dictionary.GetBigramAncestryAlphabet().Lookup(bigram_ancestry_string);
  CHECK_LT(bigram_ancestry_id, 0xffff);
  if (bigram_ancestry_id < 0) bigram_ancestry_id = kUnknownBigramAncestry;
  bigram_ancestry_ = bigram_ancestry_id;
}

void Mention::GetSpeaker(CoreferenceSentence *instance,
                         std::string *speaker) {
  *speaker = instance->GetSpeaker(head_index_);
}

void Mention::GetPhraseString(CoreferenceSentence *instance,
                              std::string *phrase_string) {
  *phrase_string = "";
  for (int i = start_; i <= end_; ++i) {
    *phrase_string += instance->GetForm(i);
    if (i < end_) *phrase_string += " ";
  }
}

void Mention::GetHeadString(CoreferenceSentence *instance,
                            std::string *head_string) {
  *head_string = instance->GetForm(head_index_);
}

void Mention::Print(const CoreferenceDictionary &dictionary,
                    CoreferenceSentence *instance) {
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

