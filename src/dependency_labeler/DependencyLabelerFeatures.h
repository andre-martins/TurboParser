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

#ifndef DEPENDENCYLABELERFEATURES_H_
#define DEPENDENCYLABELERFEATURES_H_

#include "Features.h"
#include "DependencyInstanceNumeric.h"
#include "FeatureEncoder.h"

class DependencyLabelerOptions;

class DependencyLabelerFeatures: public Features {
 public:
  DependencyLabelerFeatures() {};
  DependencyLabelerFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~DependencyLabelerFeatures() { Clear(); }

 public:
  void Clear() {
    for (int m = 0; m < input_arc_features_.size(); ++m) {
      if (!input_arc_features_[m]) continue;
        input_arc_features_[m]->clear();
        delete input_arc_features_[m];
        input_arc_features_[m] = NULL;
    }
    input_arc_features_.clear();

    for (int h = 0; h < input_sibling_features_.size(); ++h) {
      for (int i = 0; i < input_sibling_features_[h].size(); ++i) {
        if (!input_sibling_features_[h][i]) continue;
        input_sibling_features_[h][i]->clear();
        delete input_sibling_features_[h][i];
        input_sibling_features_[h][i] = NULL;
      }
      input_sibling_features_[h].clear();
    }
    input_sibling_features_.clear();
  }

  void Initialize(Instance *instance, Parts *parts,
                  const std::vector<std::vector<int> > &siblings) {
    Clear();
    int length = static_cast<DependencyInstanceNumeric*>(instance)->size();
    input_arc_features_.resize(length, static_cast<BinaryFeatures*>(NULL));
    input_sibling_features_.resize(length);
    for (int h = 0; h < length; ++h) {
      if (siblings[h].size() == 0) continue;
      input_sibling_features_[h].resize(siblings[h].size()+1,
                                        static_cast<BinaryFeatures*>(NULL));
    }
  }

  const BinaryFeatures &GetPartFeatures(int r) const {
    CHECK(false) << "All part features are specific to arcs or siblings.";
    // Do this to avoid compilation error.
    return *new BinaryFeatures;
  };

  BinaryFeatures *GetMutablePartFeatures(int r) const {
    CHECK(false) << "All part features are specific to arcs or siblings.";
    return NULL;
  };

 public:
  void AddArcFeatures(DependencyInstanceNumeric *sentence,
                      const std::vector<std::vector<int> > &descendents,
                      const std::vector<std::vector<int> > &siblings,
                      int modifier);

  void AddSiblingFeatures(DependencyInstanceNumeric* sentence,
                          const std::vector<std::vector<int> > &descendents,
                          const std::vector<std::vector<int> > &siblings,
                          int head,
                          int sibling_index);

  const BinaryFeatures &GetArcFeatures(int modifier) {
    return *(input_arc_features_[modifier]);
  }

  const BinaryFeatures &GetSiblingFeatures(int head, int sibling_index) {
    return *(input_sibling_features_[head][sibling_index]);
  }

 protected:
  void AddWordPairFeatures(DependencyInstanceNumeric* sentence,
                           int pair_type,
                           int head,
                           int modifier,
                           bool use_lemma_features,
                           bool use_morphological_features,
                           BinaryFeatures *features);

  void AddWordPairFeaturesMST(DependencyInstanceNumeric* sentence,
                              int pair_type,
                              int head,
                              int modifier,
                              BinaryFeatures *features);

  void AddArcSiblingFeatures(DependencyInstanceNumeric* sentence,
                             const std::vector<std::vector<int> > &descendents,
                             int head,
                             int modifier,
                             const std::vector<int> &siblings,
                             BinaryFeatures *features);

  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

 protected:
  std::vector<BinaryFeatures*> input_arc_features_; // Vector of arc features.
  // Vectors of sibling features.
  std::vector<std::vector<BinaryFeatures*> > input_sibling_features_;
  FeatureEncoder encoder_; // Encoder that converts features into a codeword.
};

#endif /* DEPENDENCYLABELERFEATURES_H_ */
