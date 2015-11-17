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

#ifndef CONSTITUENCYDICTIONARY_H_
#define CONSTITUENCYDICTIONARY_H_

#include "SequenceDictionary.h"
#include "ConstituencyReader.h"

class ConstituencyDictionary : public SequenceDictionary {
public:
  ConstituencyDictionary() {}
  ConstituencyDictionary(Pipe* pipe) { pipe_ = pipe; }
  virtual ~ConstituencyDictionary() { Clear(); }

  virtual void Clear() {
    // Don't clear token_dictionary, since this class does not own it.
    SequenceDictionary::Clear();
    lemma_alphabet_.clear();
    morph_alphabet_.clear();
    constituent_alphabet_.clear();
    rule_alphabet_.clear();
  }

  virtual void Save(FILE *fs) {
    SequenceDictionary::Save(fs);
    if (0 > lemma_alphabet_.Save(fs)) CHECK(false);
    if (0 > morph_alphabet_.Save(fs)) CHECK(false);
    if (0 > constituent_alphabet_.Save(fs)) CHECK(false);
    if (0 > rule_alphabet_.Save(fs)) CHECK(false);
  }

  virtual void Load(FILE *fs) {
    SequenceDictionary::Load(fs);
    if (0 > lemma_alphabet_.Load(fs)) CHECK(false);
    if (0 > morph_alphabet_.Load(fs)) CHECK(false);
    if (0 > constituent_alphabet_.Load(fs)) CHECK(false);
    constituent_alphabet_.BuildNames();
    if (0 > rule_alphabet_.Load(fs)) CHECK(false);
    rule_alphabet_.BuildNames();
  }

  void AllowGrowth() {
    SequenceDictionary::AllowGrowth();
    lemma_alphabet_.AllowGrowth();
    morph_alphabet_.AllowGrowth();
    constituent_alphabet_.AllowGrowth();
    rule_alphabet_.AllowGrowth();
  }
  void StopGrowth() {
    SequenceDictionary::StopGrowth();
    lemma_alphabet_.StopGrowth();
    morph_alphabet_.StopGrowth();
    constituent_alphabet_.StopGrowth();
    rule_alphabet_.StopGrowth();
  }

  virtual void CreateConstituentDictionary(ConstituencyReader *reader);

  const string &GetConstituentName(int id) const {
    return constituent_alphabet_.GetName(id);
  }

  const Alphabet &GetConstituentAlphabet() const {
    return constituent_alphabet_;
  }

  int GetNumLemmas() const { return lemma_alphabet_.size(); }

  int GetLemmaId(const std::string &lemma) const {
    return lemma_alphabet_.Lookup(lemma);
  }

  int GetMorphFeatureId(const std::string &morph) const {
    return morph_alphabet_.Lookup(morph);
  }

  int GetConstituentId(const std::string &name) const {
    return constituent_alphabet_.Lookup(name);
  }

  int GetRuleId(const std::string &name) const {
    return rule_alphabet_.Lookup(name);
  }

protected:
  Alphabet lemma_alphabet_;
  Alphabet morph_alphabet_;
  Alphabet constituent_alphabet_;
  Alphabet rule_alphabet_;
};

#endif /* CONSTITUENCYDICTIONARY_H_ */
