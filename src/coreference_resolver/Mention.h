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
  Mention() { sentence_ = NULL; }
  Mention(int start, int end, int id) : NumericSpan(start, end, id) {}
  virtual ~Mention() {}

  int type() const { return type_; }
  int gender() const { return gender_; }
  int number() const { return number_; }
  int unigram_ancestry() const { return unigram_ancestry_; }
  int bigram_ancestry() const { return bigram_ancestry_; }

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

  int speaker_id() const { return speaker_id_; }
  void set_speaker_id(int speaker_id) {
    speaker_id_ = speaker_id;
  }

  int head_string_id() const { return head_string_id_; }
  void set_head_string_id(int head_string_id) {
    head_string_id_ = head_string_id;
  }

  int phrase_string_id() const { return phrase_string_id_; }
  void set_phrase_string_id(int phrase_string_id) {
    phrase_string_id_ = phrase_string_id;
  }

  const std::vector<int> &all_word_string_ids() const {
    return all_word_string_ids_;
  }
  void set_all_word_string_ids(const std::vector<int> &all_word_string_ids) {
    all_word_string_ids_ = all_word_string_ids;
  }

  CoreferencePronoun *pronoun() const { return pronoun_; }

public:
  void ComputeProperties(const CoreferenceDictionary &dictionary,
                         CoreferenceSentence* instance,
                         CoreferenceSentenceNumeric *sentence);

  bool ContainsMentionHead(const Mention &mention) const {
    const std::vector<int> &all_word_string_ids = mention.all_word_string_ids();
    int head_string_id = all_word_string_ids[mention.head_index() - start_];
    for (int i = 0; i < all_word_string_ids_.size(); ++i) {
      if (head_string_id == all_word_string_ids_[i]) return true;
    }
    return false;
  }

  bool ContainsMentionString(const Mention &mention) const {
    const std::vector<int> &all_word_string_ids = mention.all_word_string_ids();
    int maximum_start =
      all_word_string_ids_.size() - all_word_string_ids.size();
    for (int i = 0; i <= maximum_start; ++i) {
      bool found_match = true;
      for (int j = 0; j < all_word_string_ids.size(); ++j) {
        if (all_word_string_ids[j] != all_word_string_ids_[i + j]) {
          found_match = false;
          break;
        }
      }
      if (found_match) return true;
    }
    return false;
  }

  // Print debug information about this mention.
  void Print(const CoreferenceDictionary &dictionary,
             CoreferenceSentence *instance);
  void GetSpeaker(CoreferenceSentence *instance,
                  std::string *speaker);
  void GetPhraseString(CoreferenceSentence *instance,
                       std::string *phrase_string);
  void GetHeadString(CoreferenceSentence *instance,
                     std::string *head_string);

protected:
  void ComputeHead();
  int ComputeNumber(const std::vector<int> &words,
                    const std::vector<int> &words_lower,
                    int head_index) {
    return MentionNumber::SINGULAR;
  }
  int ComputePersonGender(const std::vector<int> &words,
                          const std::vector<int> &words_lower,
                          int head_index) {
    return MentionGender::MALE;
  }
  int ComputeNonPersonGender(const std::vector<int> &words,
                             const std::vector<int> &words_lower,
                             int head_index) {
    return MentionGender::MALE;
  }

protected:
  CoreferenceSentenceNumeric *sentence_;
  int type_; // Type of mention (pronominal, proper, or nominal).
  int entity_tag_; // Entity tag, if applicable (otherwise, -1).
  CoreferencePronoun *pronoun_; // Pronoun information, if applicable.
  int gender_; // Gender (male, female, neutral, unknown).
  int number_; // Number (singular, plural, unknown).
  int head_index_; // Position of the head word.
  int unigram_ancestry_; // Dependency (syntactic) unigram ancestry.
  int bigram_ancestry_; // Dependency (syntactic) bigram ancestry.
  std::vector<int> words_; // Mention words.
  std::vector<int> words_lower_; // Mention words in lower case.
  std::vector<int> tags_; // Mention POS tags.
  int offset_; // Global offset position (start of sentence at document level).
  int sentence_index_; // Index of the sentence to which this mention belongs.
  int speaker_id_;
  int head_string_id_; // ID of head word to test head match w/ other mentions.
  int phrase_string_id_; // ID of the entire phrase to test exact match.
  std::vector<int> all_word_string_ids_; // IDs of the all mention words.
};

#endif /* MENTION_H_ */
