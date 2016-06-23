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

#ifndef SEMANTICFEATURES_H_
#define SEMANTICFEATURES_H_

#include "Features.h"
#include "SemanticInstanceNumeric.h"
#include "FeatureEncoder.h"

class SemanticOptions;

class SemanticFeatures : public Features {
public:
  SemanticFeatures() {};
  SemanticFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~SemanticFeatures() { Clear(); }

public:
  void Clear() {
    CHECK_EQ(input_features_.size(), input_labeled_features_.size());
    for (int r = 0; r < input_features_.size(); ++r) {
      if (input_features_[r]) {
        input_features_[r]->clear();
        delete input_features_[r];
        input_features_[r] = NULL;
      }
      if (input_labeled_features_[r]) {
        input_labeled_features_[r]->clear();
        delete input_labeled_features_[r];
        input_labeled_features_[r] = NULL;
      }
    }
    input_features_.clear();
    input_labeled_features_.clear();
  }

  void Initialize(Instance *instance, Parts *parts) {
    Clear();
    input_features_.resize(parts->size(), static_cast<BinaryFeatures*>(NULL));
    input_labeled_features_.resize(parts->size(),
                                   static_cast<BinaryFeatures*>(NULL));
  }

  int GetNumPartFeatures(int r) const {
    return (NULL == input_features_[r]) ? 0 : (int)(input_features_[r]->size());
  };

  int GetNumLabeledPartFeatures(int r) const {
    return (NULL == input_labeled_features_[r]) ?
      0 : (int)(input_labeled_features_[r]->size());
  };

  int GetPartFeature(int r, int j) const {
    return (*input_features_[r])[j];
  }

  int GetLabeledPartFeature(int r, int j) const {
    return (*input_labeled_features_[r])[j];
  }

  const BinaryFeatures &GetPartFeatures(int r) const {
    CHECK(input_features_[r] != NULL);
    return *(input_features_[r]);
  };

  const BinaryFeatures &GetLabeledPartFeatures(int r) const {
    CHECK(input_labeled_features_[r] != NULL);
    return *(input_labeled_features_[r]);
  };

  BinaryFeatures *GetMutablePartFeatures(int r) const {
    return input_features_[r];
  };

  BinaryFeatures *GetMutableLabeledPartFeatures(int r) const {
    return input_labeled_features_[r];
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

  void AddLabeledArcFeatures(SemanticInstanceNumeric *sentence,
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

  void AddArbitraryLabeledSiblingFeatures(SemanticInstanceNumeric* sentence,
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

  void AddGrandparentFeatures(SemanticInstanceNumeric* sentence,
                              int r,
                              int grandparent_predicate,
                              int grandparent_sense,
                              int predicate,
                              int sense,
                              int argument);

  void AddCoparentFeatures(SemanticInstanceNumeric* sentence,
                           int r,
                           int first_predicate,
                           int first_sense,
                           int second_predicate,
                           int second_sense,
                           int argument);

  void AddConsecutiveCoparentFeatures(SemanticInstanceNumeric* sentence,
                                      int r,
                                      int first_predicate,
                                      int first_sense,
                                      int second_predicate,
                                      int second_sense,
                                      int argument);

  void AddSecondOrderFeatures(SemanticInstanceNumeric* sentence,
                              int r,
                              int first_predicate,
                              int first_sense,
                              int second_predicate,
                              int second_sense,
                              int argument,
                              bool coparents,
                              bool consecutive);

#if 0
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
                            bool labeled,
                            uint8_t feature_type,
                            int r,
                            int predicate,
                            int predicate_id);

  void AddArcFeatures(SemanticInstanceNumeric *sentence,
                      bool labeled,
                      int r,
                      int predicate,
                      int argument,
                      int predicate_id);

  void AddSiblingFeatures(SemanticInstanceNumeric* sentence,
                          bool labeled,
                          int r,
                          int predicate,
                          int sense,
                          int first_argument,
                          int second_argument,
                          bool consecutive);

  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

protected:
  // Vector of input features.
  vector<BinaryFeatures*> input_features_;
  // Vector of input features to be conjoined with a label to produce a
  // "labeled" feature.
  vector<BinaryFeatures*> input_labeled_features_;
  // Encoder that converts features into a codeword.
  FeatureEncoder encoder_;
};

#endif /* SEMANTICFEATURES_H_ */
