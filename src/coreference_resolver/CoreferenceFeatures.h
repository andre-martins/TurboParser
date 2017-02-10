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

#ifndef COREFERENCEFEATURES_H_
#define COREFERENCEFEATURES_H_

#include "Features.h"
#include "CoreferenceDocumentNumeric.h"
#include "FeatureEncoder.h"

class CoreferenceOptions;

class CoreferenceFeatures : public Features {
public:
  CoreferenceFeatures() {};
  CoreferenceFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~CoreferenceFeatures() { Clear(); }

public:
  void Clear() {
    for (int r = 0; r < input_features_.size(); ++r) {
      if (!input_features_[r]) continue;
      input_features_[r]->clear();
      delete input_features_[r];
      input_features_[r] = NULL;
    }
    input_features_.clear();
  }

  void Initialize(Instance *instance, Parts *parts) {
    Clear();
    input_features_.resize(parts->size(), static_cast<BinaryFeatures*>(NULL));
  }

  int GetNumPartFeatures(int r) const {
    return (NULL == input_features_[r]) ? 0 : (int)(input_features_[r]->size());
  };

  int GetPartFeature(int r, int j) const {
    return (*input_features_[r])[j];
  }

  const BinaryFeatures &GetPartFeatures(int r) const {
    return *(input_features_[r]);
  };

  BinaryFeatures *GetMutablePartFeatures(int r) const {
    return input_features_[r];
  };

public:
  void AddArcFeatures(CoreferenceDocumentNumeric *document,
                      int r,
                      int parent_mention,
                      int child_mention);

  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

protected:
  vector<BinaryFeatures*> input_features_; // Vector of input features.
  FeatureEncoder encoder_; // Encoder that converts features into a codeword.
};

#endif /* COREFERENCEFEATURES_H_ */
