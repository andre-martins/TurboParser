// Copyright (c) 2012-2013 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.1.
//
// TurboParser 2.1 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.1.  If not, see <http://www.gnu.org/licenses/>.

#ifndef CONSTITUENCYLABELERDICTIONARY_H_
#define CONSTITUENCYLABELERDICTIONARY_H_

#include "Dictionary.h"
#include "TokenDictionary.h"
#include "SerializationUtils.h"
#include "ConstituencyLabelerReader.h"
#include "ConstituencyDictionary.h"

class ConstituencyLabelerDictionary : public ConstituencyDictionary {
 public:
  ConstituencyLabelerDictionary() {}
  ConstituencyLabelerDictionary(Pipe* pipe) { pipe_ = pipe; }
  virtual ~ConstituencyLabelerDictionary() { Clear(); }

  void Clear() {
    ConstituencyDictionary::Clear();
    // Don't clear token_dictionary, since this class does not own it.
    label_alphabet_.clear();
    constituent_labels_.clear();
    constituent_label_frequencies_.clear();
  }

  void Save(FILE *fs) {
    ConstituencyDictionary::Save(fs);
    if (0 > label_alphabet_.Save(fs)) CHECK(false);
    // TODO: save constituency labels and frequencies.
  }

  void Load(FILE *fs) {
    ConstituencyDictionary::Load(fs);
    if (0 > label_alphabet_.Load(fs)) CHECK(false);
    label_alphabet_.BuildNames();
    // TODO: load constituency labels and frequencies.
  }

  void AllowGrowth() {
    ConstituencyDictionary::AllowGrowth();
    label_alphabet_.AllowGrowth();
  }
  void StopGrowth() {
    ConstituencyDictionary::StopGrowth();
    label_alphabet_.StopGrowth();
  }

  void CreateConstituentDictionary(ConstituencyReader *reader);
  void CreateLabelDictionary(ConstituencyLabelerReader *reader);

  const string &GetLabelName(int label) const {
    return label_alphabet_.GetName(label);
  }

  const Alphabet &GetLabelAlphabet() const { return label_alphabet_; }

  int GetLabelId(const std::string &name) const {
    return label_alphabet_.Lookup(name);
  }

  const std::vector<int> &GetConstituentLabels(int constituent_id) {
    return constituent_labels_[constituent_id];
  }

 protected:
  Alphabet label_alphabet_;
  std::vector<std::vector<int> > constituent_labels_;
  std::vector<std::vector<int> > constituent_label_frequencies_;
};

#endif /* CONSTITUENCYLABELERDICTIONARY_H_ */
