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

#ifndef SEMANTICFEATURES_H_
#define SEMANTICFEATURES_H_

#include "Features.h"
#include "SemanticInstanceNumeric.h"
#include "FeatureEncoder.h"

class SemanticOptions;

class SemanticFeatures: public Features {
 public:
  SemanticFeatures() {};
  SemanticFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~SemanticFeatures() { Clear(); }

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
    return (NULL == input_features_[r])? 0 : input_features_[r]->size();
  };

  int GetPartFeature(int r, int j) const {
    return (*input_features_[r])[j];
  }

  const BinaryFeatures &GetPartFeatures(int r) const {
    CHECK(input_features_[r] != NULL);
    return *(input_features_[r]);
  };

  BinaryFeatures *GetMutablePartFeatures(int r) const {
    return input_features_[r];
  };

 public:
  void AddPredicateFeatures(SemanticInstanceNumeric *sentence,
                            int r,
                            int predicate,
                            int predicate_id);

  void AddArcFeatures(SemanticInstanceNumeric *sentence,
                      int r,
                      int predicate,
                      int argument,
                      int predicate_id);

  void AddArbitrarySiblingFeatures(SemanticInstanceNumeric* sentence,
                                   int r,
                                   int predicate,
                                   int sense,
                                   int first_argument,
                                   int second_argument);

  void AddConsecutiveSiblingFeatures(SemanticInstanceNumeric* sentence,
                                     int r,
                                     int predicate,
                                     int sense,
                                     int first_argument,
                                     int second_argument);

  void AddSiblingFeatures(SemanticInstanceNumeric* sentence,
                          int r,
                          int predicate,
                          int sense,
                          int first_argument,
                          int second_argument,
                          bool consecutive);

#if 0
  void AddGrandparentFeatures(SemanticInstanceNumeric* sentence,
                              int r,
                              int grandparent,
                              int head,
                              int modifier);

  void AddGrandSiblingFeatures(SemanticInstanceNumeric* sentence,
                               int r,
                               int grandparent,
                               int head,
                               int modifier,
                               int sibling);

  void AddTriSiblingFeatures(SemanticInstanceNumeric* sentence,
                             int r,
                             int head,
                             int modifier,
                             int sibling,
                             int other_sibling);
#endif

 protected:
  void AddPredicateFeatures(SemanticInstanceNumeric *sentence,
                            uint8_t feature_type,
                            int r,
                            int predicate,
                            int predicate_id);

  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

 protected:
  vector<BinaryFeatures*> input_features_; // Vector of input features.
  FeatureEncoder encoder_; // Encoder that converts features into a codeword.
};

#endif /* SEMANTICFEATURES_H_ */
