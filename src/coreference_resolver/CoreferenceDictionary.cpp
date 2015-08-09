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

#include "CoreferenceDictionary.h"
#include "CoreferencePipe.h"
#include "Mention.h"
#include <algorithm>
#include <iostream>
#include <sstream>

int GenderNumberStatistics::ComputeNumber(const std::vector<int> &phrase,
                                          int head_index) const {
  std::map<std::vector<int>, std::vector<int> >::const_iterator it =
    phrase_counts_.find(phrase);
  std::vector<int> counts;
  if (it != phrase_counts_.end()) {
    counts = it->second;
  } else {
    std::vector<int> word;
    word.push_back(phrase[head_index]);
    it = phrase_counts_.find(word);
    if (it != phrase_counts_.end()) {
      counts = it->second;
    }
  }
  if (counts.size() > 0) {
    CHECK_EQ(counts.size(), 4);
    if (counts[0] + counts[1] + counts[2] >= counts[3]) {
      return MentionNumber::SINGULAR;
    } else {
      return MentionNumber::PLURAL;
    }
  } else {
    return MentionNumber::SINGULAR;
  }
}

int GenderNumberStatistics::ComputeGender(const std::vector<int> &phrase,
                                          int head_index) const {
  std::map<std::vector<int>, std::vector<int> >::const_iterator it =
    phrase_counts_.find(phrase);
  std::vector<int> counts;
  if (it != phrase_counts_.end()) {
    counts = it->second;
  } else {
    std::vector<int> word;
    word.push_back(phrase[head_index]);
    it = phrase_counts_.find(word);
    if (it != phrase_counts_.end()) {
      counts = it->second;
    }
  }
  if (counts.size() > 0) {
    CHECK_EQ(counts.size(), 4);
    // Require some confidence to decide (taken from the Stanford system).
    if (counts[0] >= 2*(counts[1] + counts[2]) && counts[0] >= 3) {
      return MentionGender::MALE;
    } else if (counts[1] >= 2*(counts[0] + counts[2]) && counts[1] >= 3) {
      return MentionGender::FEMALE;
    } else if (counts[2] >= 2*(counts[0] + counts[1]) && counts[2] >= 3) {
      return MentionGender::NEUTRAL;
    } else {
      return MentionGender::UNKNOWN;
    }
  } else {
    return MentionGender::UNKNOWN;
  }
}

void CoreferenceDictionary::CreateEntityDictionary(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating entity dictionary...";
  std::vector<int> entity_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    for (int i = 0; i < instance->GetEntitySpans().size(); ++i) {
      int id;

      // Add constituent to alphabet.
      id = entity_alphabet_.
        Insert(instance->GetEntitySpans()[i]->name());
      if (id >= entity_freqs.size()) {
        CHECK_EQ(id, entity_freqs.size());
        entity_freqs.push_back(0);
      }
      ++entity_freqs[id];
    }
    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();
  entity_alphabet_.StopGrowth();

  LOG(INFO) << "Number of entities: " << entity_alphabet_.size();
  for (Alphabet::iterator it = entity_alphabet_.begin();
       it != entity_alphabet_.end();
       ++it) {
    LOG(INFO) << it->first;
  }
}

void CoreferenceDictionary::CreateConstituentDictionary(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating constituent dictionary...";
  std::vector<int> constituent_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    for (int i = 0; i < instance->GetConstituentSpans().size(); ++i) {
      int id;

      // Add constituent to alphabet.
      id = constituent_alphabet_.
        Insert(instance->GetConstituentSpans()[i]->name());
      if (id >= constituent_freqs.size()) {
        CHECK_EQ(id, constituent_freqs.size());
        constituent_freqs.push_back(0);
      }
      ++constituent_freqs[id];
    }
    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();
  constituent_alphabet_.StopGrowth();

  LOG(INFO) << "Number of constituents: " << constituent_alphabet_.size();
  for (Alphabet::iterator it = constituent_alphabet_.begin();
       it != constituent_alphabet_.end();
       ++it) {
    LOG(INFO) << it->first;
  }
}

void CoreferenceDictionary::CreateWordDictionaries(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating word dictionary...";
  std::vector<int> word_freqs;
  std::vector<int> word_lower_freqs;

  /*
  string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kTokenUnknown;
  special_symbols[TOKEN_START] = kTokenStart;
  special_symbols[TOKEN_STOP] = kTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    word_alphabet.Insert(special_symbols[i]);
    word_freqs.push_back(0);
    word_lower_alphabet.Insert(special_symbols[i]);
    word_lower_freqs.push_back(0);
  }
  */

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    for (int i = 0; i < instance->size(); ++i) {
      int id;

      // Add form to alphabet.
      std::string form = instance->GetForm(i);
      std::string form_lower(form);
      transform(form_lower.begin(), form_lower.end(), form_lower.begin(),
                ::tolower);
      id = word_alphabet_.Insert(form);
      if (id >= word_freqs.size()) {
        CHECK_EQ(id, word_freqs.size());
        word_freqs.push_back(0);
      }
      ++word_freqs[id];

      // Add lower-case form to the alphabet.
      // TODO(atm): "sanitize" words, by escaping digit sequences:
      // word = re.sub('[\d]+', '#', word.lower())
      id = word_lower_alphabet_.Insert(form_lower);
      if (id >= word_lower_freqs.size()) {
        CHECK_EQ(id, word_lower_freqs.size());
        word_lower_freqs.push_back(0);
      }
      ++word_lower_freqs[id];
    }
    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();
  word_alphabet_.StopGrowth();
  word_lower_alphabet_.StopGrowth();

  LOG(INFO) << "Number of words: " << word_alphabet_.size();
  LOG(INFO) << "Number of lower-case words: " << word_lower_alphabet_.size();
}

void CoreferenceDictionary::ReadGenderNumberStatistics() {
  // TODO(atm): implement this.
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  word_alphabet_.AllowGrowth();
  word_lower_alphabet_.AllowGrowth();

  gender_number_statistics_.Clear();

  if (options->file_gender_number_statistics() != "") {
    LOG(INFO) << "Loading gender/number statistics file "
              << options->file_gender_number_statistics() << "...";
    std::ifstream is;
    std::string line;

    // Read the pronouns, one per line.
    is.open(options->file_gender_number_statistics().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->file_gender_number_statistics() << ".";
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line == "") continue; // Ignore blank lines.
        std::vector<std::string> fields;
        StringSplit(line, "\t", &fields); // Break on tabs.
        CHECK_EQ(fields.size(), 2);
        const std::string &phrase = fields[0];
        const std::string &statistics = fields[1];
        std::vector<std::string> words;
        StringSplit(phrase, " ", &words); // Break on spaces.
        std::vector<int> phrase_ids;
        for (int i = 0; i < words.size(); ++i) {
          const std::string &word = words[i];
          std::string word_lower(word);
          transform(word_lower.begin(), word_lower.end(), word_lower.begin(),
                    ::tolower);

          int word_id = word_alphabet_.Insert(word);

          // Add lower-case form to the alphabet.
          // TODO(atm): "sanitize" words, by escaping digit sequences:
          // word = re.sub('[\d]+', '#', word.lower())
          int word_lower_id = word_lower_alphabet_.Insert(word_lower);
          phrase_ids.push_back(word_lower_id);
        }

        std::vector<std::string> subfields;
        StringSplit(statistics, " ", &subfields); // Break on spaces.
        CHECK_EQ(subfields.size(), 4);
        std::vector<int> counts;
        for (int i = 0; i < subfields.size(); ++i) {
          std::stringstream ss(subfields[i]);
          int count;
          ss >> count;
          counts.push_back(count);
        }

        if (!gender_number_statistics_.AddPhrase(phrase_ids, counts)) {
          LOG(INFO) << "Repeated phrase: " << phrase;
        }
      }
    }
    is.close();
  }

  word_alphabet_.StopGrowth();
  word_lower_alphabet_.StopGrowth();

  LOG(INFO) << "Number of words: " << word_alphabet_.size();
  LOG(INFO) << "Number of lower-case words: " << word_lower_alphabet_.size();
}

void CoreferenceDictionary::ReadPronouns() {
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  DeleteAllPronouns();

  if (options->file_pronouns() != "") {
    LOG(INFO) << "Loading pronouns file "
              << options->file_pronouns() << "...";
    std::ifstream is;
    std::string line;

    // Read the pronouns, one per line.
    is.open(options->file_pronouns().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->file_pronouns() << ".";
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line == "") continue; // Ignore blank lines.
        std::vector<std::string> fields;
        StringSplit(line, " \t", &fields); // Break on tabs or spaces.
        CHECK_EQ(fields.size(), 2);
        const std::string &form = fields[0];
        const std::string code_flags = fields[1];
        std::string form_lower(form);
        transform(form_lower.begin(), form_lower.end(), form_lower.begin(),
                  ::tolower);
        int id = token_dictionary_->GetFormLowerId(form_lower);
        CHECK_LT(id, 0xffff);
        if (id < 0) {
          LOG(INFO) << "Ignoring unknown word: "
                    << form_lower;
          continue;
        }
        CHECK(all_pronouns_.find(id) == all_pronouns_.end());
        CoreferencePronoun *pronoun = new CoreferencePronoun(code_flags);
        all_pronouns_[id] = pronoun;
      }
    }
    is.close();
  }
}

void CoreferenceDictionary::ReadMentionTags() {
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  named_entity_tags_.clear();
  person_entity_tags_.clear();
  noun_phrase_tags_.clear();
  proper_noun_tags_.clear();
  pronominal_tags_.clear();

  if (options->file_mention_tags() != "") {
    LOG(INFO) << "Loading mention tags file "
              << options->file_mention_tags() << "...";
    std::ifstream is;
    std::string line;

    // Read the mention tags, one type per line.
    is.open(options->file_mention_tags().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->file_mention_tags() << ".";
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line == "") continue; // Ignore blank lines.
        std::vector<std::string> fields;
        StringSplit(line, " \t", &fields); // Break on tabs or spaces.
        CHECK_GT(fields.size(), 0);
        const std::string &mention_tag_type = fields[0];
        if (mention_tag_type == "named_entity_tags") {
          for (int i = 1; i < fields.size(); ++i) {
            const std::string &tag_name = fields[i];
            int id = entity_alphabet_.Lookup(tag_name);
            CHECK_LT(id, 0xff);
            if (id < 0) {
              LOG(INFO) << "Ignoring unknown entity tag: "
                        << tag_name;
              continue;
            }
            named_entity_tags_.insert(id);
          }
        } else if (mention_tag_type == "person_entity_tags") {
          for (int i = 1; i < fields.size(); ++i) {
            const std::string &tag_name = fields[i];
            int id = entity_alphabet_.Lookup(tag_name);
            CHECK_LT(id, 0xff);
            if (id < 0) {
              LOG(INFO) << "Ignoring unknown entity tag: "
                        << tag_name;
              continue;
            }
            person_entity_tags_.insert(id);
          }
        } else if (mention_tag_type == "noun_phrase_tags") {
          for (int i = 1; i < fields.size(); ++i) {
            const std::string &tag_name = fields[i];
            int id = constituent_alphabet_.Lookup(tag_name);
            CHECK_LT(id, 0xffff);
            if (id < 0) {
              LOG(INFO) << "Ignoring unknown constituent: "
                        << tag_name;
              continue;
            }
            noun_phrase_tags_.insert(id);
          }
        } else if (mention_tag_type == "proper_noun_tags") {
          for (int i = 1; i < fields.size(); ++i) {
            const std::string &tag_name = fields[i];
            int id = token_dictionary_->GetPosTagId(tag_name);
            CHECK_LT(id, 0xff);
            if (id < 0) {
              LOG(INFO) << "Ignoring unknown POS tag: "
                        << tag_name;
              continue;
            }
            proper_noun_tags_.insert(id);
          }
        } else if (mention_tag_type == "pronominal_tags") {
          for (int i = 1; i < fields.size(); ++i) {
            const std::string &tag_name = fields[i];
            int id = token_dictionary_->GetPosTagId(tag_name);
            CHECK_LT(id, 0xff);
            if (id < 0) {
              LOG(INFO) << "Ignoring unknown POS tag: "
                        << tag_name;
              continue;
            }
            pronominal_tags_.insert(id);
          }
        }
      }
    }
    is.close();
  }
}
