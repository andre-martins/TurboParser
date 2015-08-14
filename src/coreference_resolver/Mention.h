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

  int type() const { return type_; }

  int head_index() const { return head_index_; }
  void set_head_index(int head_index) { head_index_ = head_index; }

  int sentence_index() const { return sentence_index_; }
  void set_sentence_index(int sentence_index) {
    sentence_index_ = sentence_index;
  }

  int offset() const { return offset_; }
  void set_offset(int offset) { offset_ = offset; }

  int global_start() const { return offset_ + start_; }
  int global_end() const { return offset_ + end_; }
  int global_head_index() { return offset_ + head_index_; }

  int head_string_id() const { return head_string_id_; }
  void set_head_string_id(int head_string_id) {
    head_string_id_ = head_string_id;
  }

  int phrase_string_id() const { return phrase_string_id_; }
  void set_phrase_string_id(int phrase_string_id) {
    phrase_string_id_ = phrase_string_id;
  }

  CoreferencePronoun *pronoun() const { return pronoun_; }

 public:
  void ComputeProperties(const CoreferenceDictionary &dictionary,
                         CoreferenceSentence* instance,
                         CoreferenceSentenceNumeric *sentence);

  // Print debug information about this mention.
  void Print(const CoreferenceDictionary &dictionary,
             CoreferenceSentence *instance);
  void GetPhraseString(CoreferenceSentence *instance,
                       std::string *phrase_string);
  void GetHeadString(CoreferenceSentence *instance,
                     std::string *head_string);

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
  CoreferencePronoun *pronoun_; // Pronoun information, if applicable.
  int gender_; // Gender (male, female, neutral, unknown).
  int number_; // Number (singular, plural, unknown).
  int head_index_; // Position of the head word.
  std::vector<int> words_; // Mention words.
  std::vector<int> words_lower_; // Mention words in lower case.
  std::vector<int> tags_; // Mention POS tags.
  int offset_; // Global offset position (start of sentence at document level).
  int sentence_index_; // Index of the sentence to which this mention belongs.
  int head_string_id_; // ID of head word to test head match w/ other mentions.
  int phrase_string_id_; // ID of the entire phrase to test exact match.
};

#endif /* MENTION_H_ */
