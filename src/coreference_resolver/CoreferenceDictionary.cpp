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

#if 0
void CoreferenceDictionary::CreateMentionWordDictionaries(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating mention word dictionary...";

  int mention_lexical_cutoff = FLAGS_mention_lexical_cutoff;

  std::vector<int> head_word_freqs;
  std::vector<int> first_word_freqs;
  std::vector<int> last_word_freqs;
  std::vector<int> previous_word_freqs;
  std::vector<int> next_word_freqs;

  Alphabet head_word_alphabet;
  Alphabet first_word_alphabet;
  Alphabet last_word_alphabet;
  Alphabet previous_word_alphabet;
  Alphabet next_word_alphabet;

  string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kTokenUnknown;
  special_symbols[TOKEN_START] = kTokenStart;
  special_symbols[TOKEN_STOP] = kTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    head_word_alphabet.Insert(special_symbols[i]);
    first_word_alphabet.Insert(special_symbols[i]);
    last_word_alphabet.Insert(special_symbols[i]);
    previous_word_alphabet.Insert(special_symbols[i]);
    next_word_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    head_word_freqs.push_back(-1);
    first_word_freqs.push_back(-1);
    last_word_freqs.push_back(-1);
    previous_word_freqs.push_back(-1);
    next_word_freqs.push_back(-1);
  }

  // Go through the corpus and build the dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    // Need to create a numeric instance since mentions are generated there.
    CoreferenceSentenceNumeric sentence;
    sentence.Initialize(this, instance, true);
    const std::vector<Mention*> &mentions = sentence.GetMentions();
    for (int j = 0; j < mentions.size(); ++j) {
      int id;
      int i;
      Mention *mention = (*mentions)[j];

      // Add head word to alphabet.
      i = mention->head_index();
      std::string form = instance->GetForm(i);
      transform(form.begin(), form.end(), form.begin(), ::tolower);
      id = head_word_alphabet_.Insert(form);
      if (id >= head_word_freqs.size()) {
        CHECK_EQ(id, head_word_freqs.size());
        head_word_freqs.push_back(0);
      }
      ++head_word_freqs[id];

      // Add first word to alphabet.
      i = mention->start();
      std::string form = instance->GetForm(i);
      transform(form.begin(), form.end(), form.begin(), ::tolower);
      id = first_word_alphabet_.Insert(form);
      if (id >= first_word_freqs.size()) {
        CHECK_EQ(id, first_word_freqs.size());
        first_word_freqs.push_back(0);
      }
      ++first_word_freqs[id];

      // Add last word to alphabet.
      i = mention->end();
      std::string form = instance->GetForm(i);
      transform(form.begin(), form.end(), form.begin(), ::tolower);
      id = last_word_alphabet_.Insert(form);
      if (id >= last_word_freqs.size()) {
        CHECK_EQ(id, last_word_freqs.size());
        last_word_freqs.push_back(0);
      }
      ++last_word_freqs[id];

      // Add previous word to alphabet.
      i = mention->start()-1;
      if (i >= 0) {
        std::string form = instance->GetForm(i);
        transform(form.begin(), form.end(), form.begin(), ::tolower);
        id = previous_word_alphabet_.Insert(form);
        if (id >= previous_word_freqs.size()) {
          CHECK_EQ(id, previous_word_freqs.size());
          previous_word_freqs.push_back(0);
        }
        ++previous_word_freqs[id];
      }

      // Add next word to alphabet.
      i = mention->end()+1;
      if (i < instance->size()) {
        std::string form = instance->GetForm(i);
        transform(form.begin(), form.end(), form.begin(), ::tolower);
        id = next_word_alphabet_.Insert(form);
        if (id >= next_word_freqs.size()) {
          CHECK_EQ(id, next_word_freqs.size());
          next_word_freqs.push_back(0);
        }
        ++next_word_freqs[id];
      }
    }

    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();

  // Now adjust the cutoffs if necessary.
  while (true) {
    head_word_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      head_word_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = head_word_alphabet.begin();
         iter != head_word_alphabet.end();
         ++iter) {
      if (head_word_freqs[iter->second] > head_word_cutoff) {
        head_word_alphabet_.Insert(iter->first);
      }
    }
    if (head_word_alphabet_.size() < kMaxMentionWordAlphabetSize) break;
    ++head_word_cutoff;
    LOG(INFO) << "Incrementing head word cutoff to " << head_word_cutoff
              << "...";
  }

  while (true) {
    first_word_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      first_word_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = first_word_alphabet.begin();
         iter != first_word_alphabet.end();
         ++iter) {
      if (first_word_freqs[iter->second] > first_word_cutoff) {
        first_word_alphabet_.Insert(iter->first);
      }
    }
    if (first_word_alphabet_.size() < kMaxMentionWordAlphabetSize) break;
    ++first_word_cutoff;
    LOG(INFO) << "Incrementing first word cutoff to " << first_word_cutoff
              << "...";
  }

  while (true) {
    last_word_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      last_word_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = last_word_alphabet.begin();
         iter != last_word_alphabet.end();
         ++iter) {
      if (last_word_freqs[iter->second] > last_word_cutoff) {
        last_word_alphabet_.Insert(iter->first);
      }
    }
    if (last_word_alphabet_.size() < kMaxMentionWordAlphabetSize) break;
    ++last_word_cutoff;
    LOG(INFO) << "Incrementing head word cutoff to " << last_word_cutoff
              << "...";
  }

  while (true) {
    previous_word_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      previous_word_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = previous_word_alphabet.begin();
         iter != previous_word_alphabet.end();
         ++iter) {
      if (previous_word_freqs[iter->second] > previous_word_cutoff) {
        previous_word_alphabet_.Insert(iter->first);
      }
    }
    if (previous_word_alphabet_.size() < kMaxMentionWordAlphabetSize) break;
    ++previous_word_cutoff;
    LOG(INFO) << "Incrementing head word cutoff to " << previous_word_cutoff
              << "...";
  }

  while (true) {
    next_word_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      next_word_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = next_word_alphabet.begin();
         iter != next_word_alphabet.end();
         ++iter) {
      if (next_word_freqs[iter->second] > next_word_cutoff) {
        next_word_alphabet_.Insert(iter->first);
      }
    }
    if (next_word_alphabet_.size() < kMaxMentionWordAlphabetSize) break;
    ++next_word_cutoff;
    LOG(INFO) << "Incrementing head word cutoff to " << next_word_cutoff
              << "...";
  }

  head_word_alphabet_.StopGrowth();
  first_word_alphabet_.StopGrowth();
  last_word_alphabet_.StopGrowth();
  previous_word_alphabet_.StopGrowth();
  next_word_alphabet_.StopGrowth();

  LOG(INFO) << "Number of head words: " << head_word_alphabet_.size() << endl
            << "Number of first words: " << first_word_alphabet_.size() << endl
            << "Number of last words: " << last_word_alphabet_.size() << endl
            << "Number of previous words: " << previous_word_alphabet_.size() << endl
            << "Number of next words: " << next_word_alphabet_.size() << endl;

  CHECK_LT(head_word_alphabet_.size(), 0xffff);
  CHECK_LT(first_word_alphabet_.size(), 0xffff);
  CHECK_LT(last_word_alphabet_.size(), 0xffff);
  CHECK_LT(previous_word_alphabet_.size(), 0xffff);
  CHECK_LT(next_word_alphabet_.size(), 0xffff);
}
#endif

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

void CoreferenceDictionary::ComputeDependencyAncestryStrings(
    DependencyInstance *instance,
    int i,
    std::string *unigram_ancestry_string,
    std::string *bigram_ancestry_string) const {
  // Get syntactic head and grandparent.
  int head = instance->GetHead(i);
  int grandparent = (head > 0)? instance->GetHead(head) : -1;
  std::string head_tag = (head > 0)? instance->GetPosTag(head) : "ROOT";
  std::string grandparent_tag = (grandparent > 0)?
    instance->GetPosTag(grandparent) : "ROOT";

  *unigram_ancestry_string = "";
  *bigram_ancestry_string = "";
  if (head < i) {
    *unigram_ancestry_string += "<" + head_tag;
    *bigram_ancestry_string += "<" + head_tag;
  } else {
    *unigram_ancestry_string += ">" + head_tag;
    *bigram_ancestry_string += ">" + head_tag;
  }
  if (grandparent < head) {
    *bigram_ancestry_string += "<" + grandparent_tag;
  } else {
    *bigram_ancestry_string += ">" + grandparent_tag;
  }
}

void CoreferenceDictionary::CreateAncestryDictionaries(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating ancestry dictionary...";
  std::vector<int> unigram_ancestry_freqs;
  std::vector<int> bigram_ancestry_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    for (int i = 1; i < instance->size(); ++i) {
      int id;

      // Compute ancestry string and add it to alphabet.
      std::string unigram_ancestry_string;
      std::string bigram_ancestry_string;
      ComputeDependencyAncestryStrings(instance, i, &unigram_ancestry_string,
                                       &bigram_ancestry_string);
      id = unigram_ancestry_alphabet_.Insert(unigram_ancestry_string);
      if (id >= unigram_ancestry_freqs.size()) {
        CHECK_EQ(id, unigram_ancestry_freqs.size());
        unigram_ancestry_freqs.push_back(0);
      }
      ++unigram_ancestry_freqs[id];

      id = bigram_ancestry_alphabet_.Insert(bigram_ancestry_string);
      if (id >= bigram_ancestry_freqs.size()) {
        CHECK_EQ(id, bigram_ancestry_freqs.size());
        bigram_ancestry_freqs.push_back(0);
      }
      ++bigram_ancestry_freqs[id];
    }
    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();
  unigram_ancestry_alphabet_.StopGrowth();
  bigram_ancestry_alphabet_.StopGrowth();

  CHECK_LT(unigram_ancestry_alphabet_.size(), 0xffff);
  LOG(INFO) << "Number of syntactic unigram ancestry strings: "
            << unigram_ancestry_alphabet_.size();
  CHECK_LT(bigram_ancestry_alphabet_.size(), 0xffff);
  LOG(INFO) << "Number of syntactic bigram ancestry strings: "
            << bigram_ancestry_alphabet_.size();
}

void CoreferenceDictionary::ReadGenderNumberStatistics() {
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  word_alphabet_.AllowGrowth();
  word_lower_alphabet_.AllowGrowth();

  gender_number_statistics_.Clear();

  if (options->use_gender_number_statistics() &&
      options->file_gender_number_statistics() != "") {
    LOG(INFO) << "Loading gender/number statistics file "
              << options->file_gender_number_statistics() << "...";
    std::ifstream is;
    std::string line;

    // Read the gender/number statistics, one per line.
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

#if 0
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
#endif

void CoreferenceDictionary::ReadPronouns() {
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  word_alphabet_.AllowGrowth();
  word_lower_alphabet_.AllowGrowth();

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
        const std::string &word = fields[0];
        const std::string code_flags = fields[1];
        std::string word_lower(word);
        transform(word_lower.begin(), word_lower.end(), word_lower.begin(),
                  ::tolower);
        int id = word_lower_alphabet_.Lookup(word_lower);
        if (id < 0) {
          LOG(INFO) << "Adding unknown pronoun: "
                    << word_lower;
          id = word_lower_alphabet_.Insert(word_lower);
        }
        //int id = token_dictionary_->GetFormLowerId(form_lower);
        //CHECK_LT(id, 0xffff);
        //if (id < 0) {
        //  LOG(INFO) << "Ignoring unknown word: "
        //            << form_lower;
        //  continue;
        //}
        CHECK(all_pronouns_.find(id) == all_pronouns_.end());
        CoreferencePronoun *pronoun = new CoreferencePronoun(code_flags);
        all_pronouns_[id] = pronoun;
      }
    }
    is.close();
  }

  word_alphabet_.StopGrowth();
  word_lower_alphabet_.StopGrowth();

  LOG(INFO) << "Number of words: " << word_alphabet_.size();
  LOG(INFO) << "Number of lower-case words: " << word_lower_alphabet_.size();
}

void CoreferenceDictionary::ReadDeterminers() {
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  word_alphabet_.AllowGrowth();
  word_lower_alphabet_.AllowGrowth();

  DeleteAllDeterminers();

  if (options->use_gender_number_determiners() &&
      options->file_determiners() != "") {
    LOG(INFO) << "Loading determiners file "
              << options->file_determiners() << "...";
    std::ifstream is;
    std::string line;

    // Read the determiners, one per line.
    is.open(options->file_determiners().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->file_determiners() << ".";
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line == "") continue; // Ignore blank lines.
        std::vector<std::string> fields;
        StringSplit(line, " \t", &fields); // Break on tabs or spaces.
        CHECK_EQ(fields.size(), 2);
        const std::string &word = fields[0];
        const std::string code_flags = fields[1];
        std::string word_lower(word);
        transform(word_lower.begin(), word_lower.end(), word_lower.begin(),
                  ::tolower);
        int id = word_lower_alphabet_.Lookup(word_lower);
        if (id < 0) {
          LOG(INFO) << "Adding unknown determiner: "
                    << word_lower;
          id = word_lower_alphabet_.Insert(word_lower);
        }
        CHECK(all_determiners_.find(id) == all_determiners_.end());
        CoreferenceDeterminer *determiner =
          new CoreferenceDeterminer(code_flags);
        all_determiners_[id] = determiner;
      }
    }
    is.close();
  }

  word_alphabet_.StopGrowth();
  word_lower_alphabet_.StopGrowth();

  LOG(INFO) << "Number of words: " << word_alphabet_.size();
  LOG(INFO) << "Number of lower-case words: " << word_lower_alphabet_.size();
}

void CoreferenceDictionary::ReadMentionTags() {
  CoreferenceOptions *options =
    static_cast<CoreferenceOptions*>(pipe_->GetOptions());

  named_entity_tags_.clear();
  person_entity_tags_.clear();
  noun_phrase_tags_.clear();
  proper_noun_tags_.clear();
  noun_tags_.clear();
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
        } else if (mention_tag_type == "noun_tags") {
          for (int i = 1; i < fields.size(); ++i) {
            const std::string &tag_name = fields[i];
            int id = token_dictionary_->GetPosTagId(tag_name);
            CHECK_LT(id, 0xff);
            if (id < 0) {
              LOG(INFO) << "Ignoring unknown POS tag: "
                        << tag_name;
              continue;
            }
            noun_tags_.insert(id);
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
