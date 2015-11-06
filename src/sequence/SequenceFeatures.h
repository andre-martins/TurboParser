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

#ifndef SEQUENCEFEATURES_H_
#define SEQUENCEFEATURES_H_

#include "Features.h"
#include "SequenceInstanceNumeric.h"

class SequenceOptions;

class SequenceFeatures : public Features {
public:
  SequenceFeatures() {};
  SequenceFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~SequenceFeatures() { Clear(); }

public:
  void Clear() {
    for (int i = 0; i < input_features_unigrams_.size(); ++i) {
      if (!input_features_unigrams_[i]) continue;
      input_features_unigrams_[i]->clear();
      delete input_features_unigrams_[i];
      input_features_unigrams_[i] = NULL;
    }
    input_features_unigrams_.clear();

    for (int i = 0; i < input_features_bigrams_.size(); ++i) {
      if (!input_features_bigrams_[i]) continue;
      input_features_bigrams_[i]->clear();
      delete input_features_bigrams_[i];
      input_features_bigrams_[i] = NULL;
    }
    input_features_bigrams_.clear();

    for (int i = 0; i < input_features_trigrams_.size(); ++i) {
      if (!input_features_trigrams_[i]) continue;
      input_features_trigrams_[i]->clear();
      delete input_features_trigrams_[i];
      input_features_trigrams_[i] = NULL;
    }
    input_features_trigrams_.clear();
  }

  void Initialize(Instance *instance, Parts *parts) {
    Clear();
    int length = static_cast<SequenceInstanceNumeric*>(instance)->size();
    input_features_unigrams_.resize(length, static_cast<BinaryFeatures*>(NULL));
    input_features_bigrams_.resize(length + 1, static_cast<BinaryFeatures*>(NULL));
    // Make this optional?
    input_features_trigrams_.resize(length + 1, static_cast<BinaryFeatures*>(NULL));
  }

  const BinaryFeatures &GetPartFeatures(int r) const {
    CHECK(false) << "All part features are specific to unigrams, bigrams, "
      "or trigrams.";
    // Do this to avoid compilation error.
    return *new BinaryFeatures;
  };

  BinaryFeatures *GetMutablePartFeatures(int r) const {
    CHECK(false) << "All part features are specific to unigrams, bigrams, "
      "or trigrams.";
    return NULL;
  };

  const BinaryFeatures &GetUnigramFeatures(int i) const {
    return *(input_features_unigrams_[i]);
  };

  const BinaryFeatures &GetBigramFeatures(int i) const {
    return *(input_features_bigrams_[i]);
  };

  const BinaryFeatures &GetTrigramFeatures(int i) const {
    return *(input_features_trigrams_[i]);
  };

public:
  virtual void AddUnigramFeatures(SequenceInstanceNumeric *sentence,
    int position) {
    // Add an empty feature vector.
    CHECK(!input_features_unigrams_[position]);
    BinaryFeatures *features = new BinaryFeatures;
    input_features_unigrams_[position] = features;
  }

  virtual void AddBigramFeatures(SequenceInstanceNumeric *sentence,
    int position) {
    // Add an empty feature vector.
    CHECK(!input_features_bigrams_[position]);
    BinaryFeatures *features = new BinaryFeatures;
    input_features_bigrams_[position] = features;
  }

  virtual void AddTrigramFeatures(SequenceInstanceNumeric *sentence,
    int position) {
    // Add an empty feature vector.
    CHECK(!input_features_trigrams_[position]);
    BinaryFeatures *features = new BinaryFeatures;
    input_features_trigrams_[position] = features;
  }

protected:
  // Vectors of input features.
  vector<BinaryFeatures*> input_features_unigrams_;
  vector<BinaryFeatures*> input_features_bigrams_;
  vector<BinaryFeatures*> input_features_trigrams_;
};

#endif /* SEQUENCEFEATURES_H_ */
