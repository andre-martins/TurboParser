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

struct MentionType {
  enum {
    PRONOMINAL = 0,
    PROPER,
    NOMINAL,
    NUM_MENTION_TYPES
  };
};

struct MentionGender {
  enum {
    MALE = 0,
    FEMALE,
    NEUTRAL,
    UNKNOWN,
    NUM_MENTION_GENDERS
  };
};

struct MentionNumber {
  enum {
    SINGULAR = 0,
    PLURAL,
    UNKNOWN,
    NUM_MENTION_NUMBERS
  };
};

class CoreferenceSentenceNumeric;
class CoreferenceSentence;

class Mention : public NumericSpan {
 public:
  Mention() { sentence_  = NULL; }
  Mention(int start, int end, int id) : NumericSpan(start, end, id) {}
  virtual ~Mention() {}

 public:
  void ComputeProperties(const CoreferenceDictionary &dictionary,
                         CoreferenceSentence* instance,
                         CoreferenceSentenceNumeric *sentence);

  // Print debug information about this mention.
  void Print(const CoreferenceDictionary &dictionary,
             CoreferenceSentence *instance);

 protected:
  void ComputeHead();
  int ComputeNumber(const std::vector<int> &words,
                    const std::vector<int> &words_lower,
                    int head_index) { return MentionNumber::SINGULAR; }
  int ComputePersonGender(const std::vector<int> &words,
                          const std::vector<int> &words_lower,
                          int head_index) { return MentionGender::MALE; }
  int ComputeNonPersonGender(const std::vector<int> &words,
                             const std::vector<int> &words_lower,
                             int head_index) { return MentionGender::MALE; }

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
