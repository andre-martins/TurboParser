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

#ifndef COREFERENCEDICTIONARY_H_
#define COREFERENCEDICTIONARY_H_

#include <map>
#include "Dictionary.h"
#include "TokenDictionary.h"
#include "DependencyDictionary.h"
#include "SemanticDictionary.h"
#include "SerializationUtils.h"
#include "CoreferenceReader.h"
#include "CoreferencePronoun.h"
#include "CoreferenceDeterminer.h"

class Pipe;

class GenderNumberStatistics {
 public:
  GenderNumberStatistics() {}
  virtual ~GenderNumberStatistics() { Clear(); }

  void Clear() { phrase_counts_.clear(); }

  void Save(FILE *fs) {
    bool success;
    success = WriteInteger(fs, phrase_counts_.size());
    CHECK(success);
    for (std::map<std::vector<int>, std::vector<int> >::iterator it =
           phrase_counts_.begin();
         it != phrase_counts_.end();
         ++it) {
      success = WriteIntegerVector(fs, it->first);
      CHECK(success);
      success = WriteIntegerVector(fs, it->second);
      CHECK(success);
    }
  }

  void Load(FILE *fs) {
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      std::vector<int> phrase;
      success = ReadIntegerVector(fs, &phrase);
      CHECK(success);
      std::vector<int> counts;
      success = ReadIntegerVector(fs, &counts);
      CHECK(success);
      AddPhrase(phrase, counts);
    }
  }

  bool AddPhrase(const std::vector<int> &phrase,
                 const std::vector<int> &counts) {
    if (phrase_counts_.find(phrase) == phrase_counts_.end()) {
      phrase_counts_[phrase] = counts;
      return true;
    } else {
      return false;
    }
  }

  int ComputeNumber(const std::vector<int> &phrase, int head_index) const;
  int ComputeGender(const std::vector<int> &phrase, int head_index) const;

 protected:
  std::map<std::vector<int>, std::vector<int> > phrase_counts_;
};

class CoreferenceDictionary : public Dictionary {
 public:
  CoreferenceDictionary() {}
  CoreferenceDictionary(Pipe* pipe) : pipe_(pipe) {}
  virtual ~CoreferenceDictionary() { Clear(); }

  void Clear() {
    // Don't clear token_dictionary, since this class does not own it.
    entity_alphabet_.clear();
    constituent_alphabet_.clear();
    word_alphabet_.clear();
    word_lower_alphabet_.clear();
    unigram_ancestry_alphabet_.clear();
    bigram_ancestry_alphabet_.clear();

    // TODO(atm): clear all the other stuff!!!
  }

  void Save(FILE *fs) {
    if (0 > entity_alphabet_.Save(fs)) CHECK(false);
    if (0 > constituent_alphabet_.Save(fs)) CHECK(false);
    if (0 > word_alphabet_.Save(fs)) CHECK(false);
    if (0 > word_lower_alphabet_.Save(fs)) CHECK(false);
    if (0 > unigram_ancestry_alphabet_.Save(fs)) CHECK(false);
    if (0 > bigram_ancestry_alphabet_.Save(fs)) CHECK(false);

    // Save gender/number statistics.
    gender_number_statistics_.Save(fs);

    // Save pronouns.
    bool success;
    int length = all_pronouns_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::map<int, CoreferencePronoun*>::iterator it =
           all_pronouns_.begin();
         it != all_pronouns_.end();
         ++it) {
      int id = it->first;
      CoreferencePronoun *pronoun = it->second;
      success = WriteInteger(fs, id);
      CHECK(success);
      pronoun->Save(fs);
    }

    // Save determiners.
    length = all_determiners_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::map<int, CoreferenceDeterminer*>::iterator it =
           all_determiners_.begin();
         it != all_determiners_.end();
         ++it) {
      int id = it->first;
      CoreferenceDeterminer *determiner = it->second;
      success = WriteInteger(fs, id);
      CHECK(success);
      determiner->Save(fs);
    }

    // Save various tags.
    length = named_entity_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = named_entity_tags_.begin();
         it != named_entity_tags_.end();
         ++it) {
      int id = *it;
      success = WriteInteger(fs, id);
      CHECK(success);
    }

    length = person_entity_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = person_entity_tags_.begin();
         it != person_entity_tags_.end();
         ++it) {
      int id = *it;
      success = WriteInteger(fs, id);
      CHECK(success);
    }

    length = noun_phrase_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = noun_phrase_tags_.begin();
         it != noun_phrase_tags_.end();
         ++it) {
      int id = *it;
      success = WriteInteger(fs, id);
      CHECK(success);
    }

    length = proper_noun_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = proper_noun_tags_.begin();
         it != proper_noun_tags_.end();
         ++it) {
      int id = *it;
      success = WriteInteger(fs, id);
      CHECK(success);
    }

    length = noun_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = noun_tags_.begin();
         it != noun_tags_.end();
         ++it) {
      int id = *it;
      success = WriteInteger(fs, id);
      CHECK(success);
    }

    length = pronominal_tags_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (std::set<int>::iterator it = pronominal_tags_.begin();
         it != pronominal_tags_.end();
         ++it) {
      int id = *it;
      success = WriteInteger(fs, id);
      CHECK(success);
    }
  }

  void Load(FILE *fs) {
    if (0 > entity_alphabet_.Load(fs)) CHECK(false);
    if (0 > constituent_alphabet_.Load(fs)) CHECK(false);
    if (0 > word_alphabet_.Load(fs)) CHECK(false);
    if (0 > word_lower_alphabet_.Load(fs)) CHECK(false);
    if (0 > unigram_ancestry_alphabet_.Load(fs)) CHECK(false);
    if (0 > bigram_ancestry_alphabet_.Load(fs)) CHECK(false);
    entity_alphabet_.BuildNames();
    constituent_alphabet_.BuildNames();
    // TODO(atm): Remove this for memory efficiency.
    word_alphabet_.BuildNames();
    word_lower_alphabet_.BuildNames();
    unigram_ancestry_alphabet_.BuildNames();
    bigram_ancestry_alphabet_.BuildNames();

    // Load gender/number statistics.
    gender_number_statistics_.Load(fs);

    // Load pronouns.
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      CoreferencePronoun *pronoun = new CoreferencePronoun;
      success = ReadInteger(fs, &id);
      CHECK(success);
      pronoun->Load(fs);
      all_pronouns_[id] = pronoun;
    }

    // Load determiners.
    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      CoreferenceDeterminer *determiner = new CoreferenceDeterminer;
      success = ReadInteger(fs, &id);
      CHECK(success);
      determiner->Load(fs);
      all_determiners_[id] = determiner;
    }

    // Load various tags.
    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      success = ReadInteger(fs, &id);
      CHECK(success);
      named_entity_tags_.insert(id);
    }

    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      success = ReadInteger(fs, &id);
      CHECK(success);
      person_entity_tags_.insert(id);
    }

    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      success = ReadInteger(fs, &id);
      CHECK(success);
      noun_phrase_tags_.insert(id);
    }

    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      success = ReadInteger(fs, &id);
      CHECK(success);
      proper_noun_tags_.insert(id);
    }

    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      success = ReadInteger(fs, &id);
      CHECK(success);
      noun_tags_.insert(id);
    }

    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      int id;
      success = ReadInteger(fs, &id);
      CHECK(success);
      pronominal_tags_.insert(id);
    }
  }

  void AllowGrowth() {
    entity_alphabet_.AllowGrowth();
    constituent_alphabet_.AllowGrowth();
    word_alphabet_.AllowGrowth();
    word_lower_alphabet_.AllowGrowth();
    unigram_ancestry_alphabet_.AllowGrowth();
    bigram_ancestry_alphabet_.AllowGrowth();
    token_dictionary_->AllowGrowth();
    dependency_dictionary_->AllowGrowth();
    semantic_dictionary_->AllowGrowth();
  }
  void StopGrowth() {
    entity_alphabet_.StopGrowth();
    constituent_alphabet_.StopGrowth();
    word_alphabet_.StopGrowth();
    word_lower_alphabet_.StopGrowth();
    unigram_ancestry_alphabet_.StopGrowth();
    bigram_ancestry_alphabet_.StopGrowth();
    token_dictionary_->StopGrowth();
    dependency_dictionary_->StopGrowth();
    semantic_dictionary_->StopGrowth();
  }

  void CreateEntityDictionary(CoreferenceSentenceReader *reader);

  void CreateConstituentDictionary(CoreferenceSentenceReader *reader);

  void CreateWordDictionaries(CoreferenceSentenceReader *reader);

  void CreateAncestryDictionaries(CoreferenceSentenceReader *reader);

  void BuildEntityNames() {
    entity_alphabet_.BuildNames();
  }

  void BuildConstituentNames() {
    constituent_alphabet_.BuildNames();
  }

  void BuildWordNames() {
    word_alphabet_.BuildNames();
    word_lower_alphabet_.BuildNames();
  }

  void BuildAncestryNames() {
    unigram_ancestry_alphabet_.BuildNames();
    bigram_ancestry_alphabet_.BuildNames();
  }

  const string &GetEntityName(int tag) const {
    return entity_alphabet_.GetName(tag);
  }

  const string &GetConstituentName(int tag) const {
    return constituent_alphabet_.GetName(tag);
  }

  const string &GetWord(int word) const {
    return word_alphabet_.GetName(word);
  }

  const string &GetWordLower(int word) const {
    return word_lower_alphabet_.GetName(word);
  }

  const string &GetUnigramAncestry(int ancestry) const {
    return unigram_ancestry_alphabet_.GetName(ancestry);
  }

  const string &GetBigramAncestry(int ancestry) const {
    return bigram_ancestry_alphabet_.GetName(ancestry);
  }


  Pipe *GetPipe() const { return pipe_; }

  TokenDictionary *GetTokenDictionary() const { return token_dictionary_; }
  DependencyDictionary *GetDependencyDictionary() const {
    return dependency_dictionary_;
  }
  SemanticDictionary *GetSemanticDictionary() const {
    return semantic_dictionary_;
  }
  void SetTokenDictionary(TokenDictionary *token_dictionary) {
    token_dictionary_ = token_dictionary;
  }
  void SetDependencyDictionary(DependencyDictionary *dependency_dictionary) {
    dependency_dictionary_ = dependency_dictionary;
  }
  void SetSemanticDictionary(SemanticDictionary *semantic_dictionary) {
    semantic_dictionary_ = semantic_dictionary;
  }

  const Alphabet &GetConstituentAlphabet() const {
    return constituent_alphabet_;
  };

  const Alphabet &GetEntityAlphabet() const {
    return entity_alphabet_;
  };

  const Alphabet &GetWordAlphabet() const {
    return word_alphabet_;
  };

  const Alphabet &GetWordLowerAlphabet() const {
    return word_lower_alphabet_;
  };

  const Alphabet &GetUnigramAncestryAlphabet() const {
    return unigram_ancestry_alphabet_;
  };

  const Alphabet &GetBigramAncestryAlphabet() const {
    return bigram_ancestry_alphabet_;
  };

  const GenderNumberStatistics &GetGenderNumberStatistics() const {
    return gender_number_statistics_;
  };

  void ReadGenderNumberStatistics();
  void ReadMentionTags();
  void ReadPronouns();
  void ReadDeterminers();

  bool IsNamedEntity(int entity_tag) const {
    return named_entity_tags_.find(entity_tag) != named_entity_tags_.end();
  }

  bool IsPersonEntity(int entity_tag) const {
    return person_entity_tags_.find(entity_tag) != person_entity_tags_.end();
  }

  bool IsNounPhrase(int constituent_tag) const {
    return noun_phrase_tags_.find(constituent_tag) != noun_phrase_tags_.end();
  }

  bool IsProperNoun(int pos_tag) const {
    return proper_noun_tags_.find(pos_tag) != proper_noun_tags_.end();
  }

  bool IsNoun(int pos_tag) const {
    return noun_tags_.find(pos_tag) != noun_tags_.end();
  }

  bool IsPronounTag(int pos_tag) const {
    return pronominal_tags_.find(pos_tag) != pronominal_tags_.end();
  }

  bool IsPronoun(int word_lower) const {
    std::map<int, CoreferencePronoun*>::const_iterator it =
      all_pronouns_.find(word_lower);
    return it != all_pronouns_.end();
  }

  CoreferencePronoun *GetPronoun(int word_lower) const {
    std::map<int, CoreferencePronoun*>::const_iterator it =
      all_pronouns_.find(word_lower);
    if (it == all_pronouns_.end()) return NULL;
    return it->second;
  }

  bool IsMalePronoun(int word_lower) const {
    CoreferencePronoun *pronoun = GetPronoun(word_lower);
    if (!pronoun) return false;
    return pronoun->IsGenderMale();
  }

  bool IsFemalePronoun(int word_lower) const {
    CoreferencePronoun *pronoun = GetPronoun(word_lower);
    if (!pronoun) return false;
    return pronoun->IsGenderFemale();
  }

  bool IsNeutralPronoun(int word_lower) const {
    CoreferencePronoun *pronoun = GetPronoun(word_lower);
    if (!pronoun) return false;
    return pronoun->IsGenderNeutral();
  }

  bool IsSingularPronoun(int word_lower) const {
    CoreferencePronoun *pronoun = GetPronoun(word_lower);
    if (!pronoun) return false;
    return pronoun->IsNumberSingular();
  }

  bool IsPluralPronoun(int word_lower) const {
    CoreferencePronoun *pronoun = GetPronoun(word_lower);
    if (!pronoun) return false;
    return pronoun->IsNumberPlural();
  }

  bool IsDeterminer(int word_lower) const {
    std::map<int, CoreferenceDeterminer*>::const_iterator it =
      all_determiners_.find(word_lower);
    return it != all_determiners_.end();
  }

  CoreferenceDeterminer *GetDeterminer(int word_lower) const {
    std::map<int, CoreferenceDeterminer*>::const_iterator it =
      all_determiners_.find(word_lower);
    if (it == all_determiners_.end()) return NULL;
    return it->second;
  }

  bool IsMaleDeterminer(int word_lower) const {
    CoreferenceDeterminer *determiner = GetDeterminer(word_lower);
    if (!determiner) return false;
    return determiner->IsGenderMale();
  }

  bool IsFemaleDeterminer(int word_lower) const {
    CoreferenceDeterminer *determiner = GetDeterminer(word_lower);
    if (!determiner) return false;
    return determiner->IsGenderFemale();
  }

  bool IsNeutralDeterminer(int word_lower) const {
    CoreferenceDeterminer *determiner = GetDeterminer(word_lower);
    if (!determiner) return false;
    return determiner->IsGenderNeutral();
  }

  bool IsSingularDeterminer(int word_lower) const {
    CoreferenceDeterminer *determiner = GetDeterminer(word_lower);
    if (!determiner) return false;
    return determiner->IsNumberSingular();
  }

  bool IsPluralDeterminer(int word_lower) const {
    CoreferenceDeterminer *determiner = GetDeterminer(word_lower);
    if (!determiner) return false;
    return determiner->IsNumberPlural();
  }

  // TODO(atm): this should not be here, but let us keep it for now...
  void ComputeDependencyAncestryStrings(
    DependencyInstance *instance,
    int i,
    std::string *unigram_ancestry_string,
    std::string *bigram_ancestry_string) const;

 protected:
  void DeleteAllPronouns() {
    for (std::map<int, CoreferencePronoun*>::iterator it =
           all_pronouns_.begin();
         it != all_pronouns_.end();
         ++it) {
      delete it->second;
    }
    all_pronouns_.clear();
  }

  void DeleteAllDeterminers() {
    for (std::map<int, CoreferenceDeterminer*>::iterator it =
           all_determiners_.begin();
         it != all_determiners_.end();
         ++it) {
      delete it->second;
    }
    all_determiners_.clear();
  }

 protected:
  Pipe *pipe_;
  TokenDictionary *token_dictionary_;
  DependencyDictionary *dependency_dictionary_;
  SemanticDictionary *semantic_dictionary_;
  Alphabet entity_alphabet_;
  Alphabet constituent_alphabet_;
  // The two form alphabets below come in addition to the TokenDictionary's
  // form alphabet. We have these additional alphabets here since we do not want
  // a cutoff and we want to allow loading a lexicon (for gender/number
  // computation).
  Alphabet word_alphabet_;
  Alphabet word_lower_alphabet_;
  Alphabet unigram_ancestry_alphabet_;
  Alphabet bigram_ancestry_alphabet_;
  GenderNumberStatistics gender_number_statistics_;
  std::map<int, CoreferencePronoun*> all_pronouns_;
  std::map<int, CoreferenceDeterminer*> all_determiners_;
  std::set<int> named_entity_tags_;
  std::set<int> person_entity_tags_;
  std::set<int> noun_tags_;
  std::set<int> noun_phrase_tags_;
  std::set<int> proper_noun_tags_;
  std::set<int> pronominal_tags_;
  //Alphabet tag_alphabet_;
};

#endif /* COREFERENCEDICTIONARY_H_ */
