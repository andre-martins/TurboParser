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

#ifndef DEPENDENCYFEATURES_H_
#define DEPENDENCYFEATURES_H_

#include "Features.h"
#include "DependencyInstanceNumeric.h"
#include "FeatureEncoder.h"

class DependencyOptions;

// This class implements the features for dependency parsing.
// The feature templates are largely inspired by the ones used in MSTParser
// (http://sourceforge.net/projects/mstparser/) and egstra
// (http://groups.csail.mit.edu/nlp/egstra/), which are described in
// the following papers:
//
// Non-Projective Dependency Parsing using Spanning Tree Algorithms.
// R. McDonald, F. Pereira, K. Ribarov and J. Hajič.
// Human Language Technologies and Empirical Methods in Natural Language
// Processing (HLT-EMNLP), 2005.
//
// M. Collins, A. Glober­son, T. Koo, X. Car­reras, and P. Bartlett.
// Ex­po­nen­ti­at­ed Gra­di­ent Al­go­rithms for Con­di­tion­al Ran­dom Fields and
// Max-​Mar­gin Markov Net­works.
// Jour­nal of Ma­chine Learn­ing Re­search 9(Aug):​1775–1822, 2008.

class DependencyFeatures : public Features {
public:
  DependencyFeatures() {};
  DependencyFeatures(Pipe* pipe) { pipe_ = pipe; }
  virtual ~DependencyFeatures() { Clear(); }

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
  void AddArcFeaturesLight(DependencyInstanceNumeric *sentence,
                           int r,
                           int head,
                           int modifier);

  void AddArcFeatures(DependencyInstanceNumeric *sentence,
                      int r,
                      int head,
                      int modifier);

  void AddArbitrarySiblingFeatures(DependencyInstanceNumeric* sentence,
                                   int r,
                                   int head,
                                   int modifier,
                                   int sibling);

  void AddConsecutiveSiblingFeatures(DependencyInstanceNumeric* sentence,
                                     int r,
                                     int head,
                                     int modifier,
                                     int sibling);

  void AddSiblingFeatures(DependencyInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier,
                          int sibling,
                          bool consecutive);

  void AddGrandparentFeatures(DependencyInstanceNumeric* sentence,
                              int r,
                              int grandparent,
                              int head,
                              int modifier);

  void AddGrandSiblingFeatures(DependencyInstanceNumeric* sentence,
                               int r,
                               int grandparent,
                               int head,
                               int modifier,
                               int sibling);

  void AddTriSiblingFeatures(DependencyInstanceNumeric* sentence,
                             int r,
                             int head,
                             int modifier,
                             int sibling,
                             int other_sibling);

  void AddNonprojectiveArcFeatures(DependencyInstanceNumeric* sentence,
                                   int r,
                                   int head,
                                   int modifier);

  void AddDirectedPathFeatures(DependencyInstanceNumeric* sentence,
                               int r,
                               int ancestor,
                               int descendant);

  void AddHeadBigramFeatures(DependencyInstanceNumeric* sentence,
                             int r,
                             int head,
                             int modifier,
                             int previous_head);

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

  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

protected:
  vector<BinaryFeatures*> input_features_; // Vector of input features.
  FeatureEncoder encoder_; // Encoder that converts features into a codeword.
};

#endif /* DEPENDENCYFEATURES_H_ */
