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

#ifndef MENTION_H_
#define MENTION_H_

#include "EntitySpan.h"
#include "CoreferenceDictionary.h"

enum MentionTypes {
  MENTION_PRONOMINAL = 0,
  MENTION_PROPER,
  MENTION_NOMINAL,
  NUM_MENTION_TYPES
};

enum MentionGender {
  MENTION_GENDER_MALE = 0,
  MENTION_GENDER_FEMALE,
  MENTION_GENDER_NEUTRAL,
  MENTION_GENDER_UNKNOWN,
  NUM_MENTION_GENDERS
};

enum MentionNumber {
  MENTION_NUMBER_SINGULAR = 0,
  MENTION_NUMBER_PLURAL,
  MENTION_NUMBER_UNKNOWN,
  NUM_MENTION_NUMBERS
};

class CoreferenceSentenceNumeric;

class Mention : public NumericSpan {
 public:
  Mention() { sentence_  = NULL; }
  Mention(int start, int end, int id) : NumericSpan(start, end, id) {}
  virtual ~Mention() {}

 public:
  void ComputeProperties(const CoreferenceDictionary &dictionary,
                         CoreferenceSentenceNumeric *sentence);

 protected:
  void ComputeHead();
  int ComputeNumber(const std::vector<int> &words,
                    const std::vector<int> &words_lower,
                    int head_index) { return -1; }
  int ComputePersonGender(const std::vector<int> &words,
                          const std::vector<int> &words_lower,
                          int head_index) { return -1; }
  int ComputeNonPersonGender(const std::vector<int> &words,
                             const std::vector<int> &words_lower,
                             int head_index) { return -1; }

 protected:
  CoreferenceSentenceNumeric *sentence_;
  int type_; // Type of mention (pronominal, proper, or nominal).
  int entity_tag_; // Entity tag, if applicable (otherwise, -1).
  int gender_; // Gender (male, female, neutral, unknown).
  int number_; // Number (singular, plural, unknown).
  int head_index_; // Position of the head word.
  std::vector<int> words_; // Mention words.
  std::vector<int> words_lower_; // Mention words in lower case.
  std::vector<int> tags_; // Mention POS tags.
};

#endif /* MENTION_H_ */
