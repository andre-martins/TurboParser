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

    // Save null label.
    bool success = WriteInteger(fs, null_label_);
    CHECK(success);

    // Save constituency labels and frequencies.
    int length = constituent_labels_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int j = 0; j < constituent_labels_.size(); ++j) {
      length = constituent_labels_[j].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int k = 0; k < constituent_labels_[j].size(); ++k) {
        int id = constituent_labels_[j][k];
        success = WriteInteger(fs, id);
        CHECK(success);
        int freq = constituent_label_frequencies_[j][k];
        success = WriteInteger(fs, freq);
        CHECK(success);
      }
    }
  }

  void Load(FILE *fs) {
    ConstituencyDictionary::Load(fs);
    if (0 > label_alphabet_.Load(fs)) CHECK(false);
    label_alphabet_.BuildNames();

    // Load null label.
    bool success = ReadInteger(fs, &null_label_);

    // Load constituency labels and frequencies.
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    constituent_labels_.resize(length);
    constituent_label_frequencies_.resize(length);
    for (int j = 0; j < constituent_labels_.size(); ++j) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      constituent_labels_[j].resize(length);
      constituent_label_frequencies_[j].resize(length);
      for (int k = 0; k < constituent_labels_[j].size(); ++k) {
        int id;
        success = ReadInteger(fs, &id);
        CHECK(success);
        constituent_labels_[j][k] = id;
        int freq;
        success = ReadInteger(fs, &freq);
        CHECK(success);
        constituent_label_frequencies_[j][k] = freq;
      }
    }
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

  int null_label() { return null_label_; }

 protected:
  Alphabet label_alphabet_;
  int null_label_;
  std::vector<std::vector<int> > constituent_labels_;
  std::vector<std::vector<int> > constituent_label_frequencies_;
};

#endif /* CONSTITUENCYLABELERDICTIONARY_H_ */
