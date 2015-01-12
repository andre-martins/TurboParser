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

#ifndef CONSTITUENCYLABELERFEATURES_H_
#define CONSTITUENCYLABELERFEATURES_H_

#include "Features.h"
#include "ConstituencyLabelerInstanceNumeric.h"
#include "FeatureEncoder.h"

class ConstituencyLabelerOptions;

class ConstituencyLabelerFeatures: public Features {
 public:
  ConstituencyLabelerFeatures() {};
  ConstituencyLabelerFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~ConstituencyLabelerFeatures() { Clear(); }

 public:
  void Clear() {
    for (int i = 0; i <  input_features_nodes_.size(); ++i) {
      if (!input_features_nodes_[i]) continue;
      input_features_nodes_[i]->clear();
      delete input_features_nodes_[i];
      input_features_nodes_[i] = NULL;
    }
    input_features_nodes_.clear();
  }

  void Initialize(Instance *instance, Parts *parts) {
    Clear();
    int num_nodes = static_cast<ConstituencyLabelerInstanceNumeric*>(instance)->
      GetNumConstituents();
    input_features_nodes_.resize(num_nodes,
                                 static_cast<BinaryFeatures*>(NULL));
  }

  const BinaryFeatures &GetPartFeatures(int r) const {
    CHECK(false) << "All part features are specific to nodes.";
    // Do this to avoid compilation error.
    return *new BinaryFeatures;
  }

  BinaryFeatures *GetMutablePartFeatures(int r) const {
    CHECK(false) << "All part features are specific to nodes.";
    return NULL;
  }

  const BinaryFeatures &GetNodeFeatures(int i) const {
    return *(input_features_nodes_[i]);
  }

  void AddNodeFeatures(ConstituencyLabelerInstanceNumeric *sentence,
                       int position);

 protected:
  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

 protected:
  // Vectors of input features.
  std::vector<BinaryFeatures*> input_features_nodes_;
  FeatureEncoder encoder_; // Encoder that converts features into a codeword.
};

#endif /* CONSTITUENCYLABELERFEATURES_H_ */
