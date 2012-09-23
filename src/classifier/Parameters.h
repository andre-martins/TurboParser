// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.0.
//
// TurboParser 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.0.  If not, see <http://www.gnu.org/licenses/>.

#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#include "Features.h"
#include "SparseParameterVector.h"
#include "SparseLabeledParameterVector.h"
#include "Utils.h"

// This class implements a feature vector, which is convenient to sum over
// binary features, weight them, etc. It just uses the classes
// SparseParameterVector and SparseLabeledParameterVector, which allow fast
// insertions and lookups.
class FeatureVector {
 public:
  FeatureVector() {
    weights_.Initialize();
    labeled_weights_.Initialize();
  }
  virtual ~FeatureVector() {};
  const SparseParameterVector &weights() { return weights_; }
  const SparseLabeledParameterVector &labeled_weights() {
    return labeled_weights_;
  }
  SparseParameterVector *mutable_weights() { return &weights_; }
  SparseLabeledParameterVector *mutable_labeled_weights() {
    return &labeled_weights_;
  }
  double GetSquaredNorm() const {
    return weights_.GetSquaredNorm() + labeled_weights_.GetSquaredNorm();
  }

 protected:
  SparseParameterVector weights_;
  SparseLabeledParameterVector labeled_weights_;
};

// This class handles the model parameters.
// It contains both "labeled" weights (for features that are conjoined with
// output labels) and regular weights.
// It allows averaging the parameters (as in averaged perceptron), which
// requires keeping around another weight vector of the same size.
class Parameters {
 public:
  Parameters() {
    use_average_ = true;
  };
  virtual ~Parameters() {};

  // Save/load the parameters.
  void Save(FILE *fs);
  void Load(FILE *fs);

  // Initialize the parameters.
  void Initialize(bool use_average) {
    use_average_ = use_average;
    weights_.Initialize(); 
    if (use_average_) averaged_weights_.Initialize();
    labeled_weights_.Initialize();
    if (use_average_) averaged_labeled_weights_.Initialize();
  }

  // Lock/unlock the parameter vector. A locked vector means that no features
  // can be added.
  void StopGrowth () {
    weights_.StopGrowth();
    averaged_weights_.StopGrowth();
    labeled_weights_.StopGrowth();
    averaged_labeled_weights_.StopGrowth();
  }
  void AllowGrowth () {
    weights_.AllowGrowth();
    averaged_weights_.AllowGrowth();
    labeled_weights_.AllowGrowth();
    averaged_labeled_weights_.AllowGrowth();
  }

  // Get the number of parameters.
  // NOTE: this counts the parameters of the features that are conjoined with
  // output labels as a single parameter.
  int Size() const { return weights_.Size() + labeled_weights_.Size(); }

  // Checks if a feature exists.
  bool Exists(uint64_t key) const { return weights_.Exists(key); }

  // Get the weight of a "simple" feature.
  double Get(uint64_t key) const { return weights_.Get(key); }

  // Get the weights of features conjoined with output labels.
  // The vector "labels" contains the labels that we want to conjoin with;
  // label_scores will contain (as output) the weight for each label.
  // Return false if the feature does not exist, in which case the label_scores
  // will be an empty vector.
  bool Get(uint64_t key,
           const vector<int> &labels,
           vector<double> *label_scores) const {
    return labeled_weights_.Get(key, labels, label_scores);
  }

  // Get the squared norm of the parameter vector.
  double GetSquaredNorm() const {
    return weights_.GetSquaredNorm() + labeled_weights_.GetSquaredNorm();
  }

  // Compute the score corresponding to a set of "simple" features.
  double ComputeScore(const BinaryFeatures &features) const {
    double score = 0.0;
    for (int j = 0; j < features.size(); ++j) {
      score += Get(features[j]);
    }
    return score;
  }

  // Compute the scores corresponding to a set of features, conjoined with
  // output labels. The vector scores, provided as output, contains the score
  // for each label.
  void ComputeLabelScores(const BinaryFeatures &features,
                          const vector<int> &labels,
                          vector<double> *scores) const {
    scores->clear();
    scores->resize(labels.size(), 0.0);
    vector<double> label_scores(labels.size(), 0.0);
    for (int j = 0; j < features.size(); ++j) {
      if (!Get(features[j], labels, &label_scores)) continue;
      for (int k = 0; k < labels.size(); ++k) {
        (*scores)[k] += label_scores[k];
      }
    }
  }

  // Scale the parameter vector by scale_factor.
  void Scale(double scale_factor) {
    weights_.Scale(scale_factor);
    labeled_weights_.Scale(scale_factor);
  }

  // Make a gradient step with a stepsize of eta, with respect to a vector
  // of "simple" features.
  // The iteration number is provided as input since it is necessary to
  // update the wanna-be "averaged parameters" in an efficient manner.
  void MakeGradientStep(const BinaryFeatures &features,
                        double eta,
                        int iteration,
                        double gradient) {
    for (int j = 0; j < features.size(); ++j) {
      weights_.Add(features[j], -eta * gradient);
      if (use_average_) {
        // perceptron/mira:
        // T*u1 + (T-1)*u2 + ... u_T = T*(u1 + u2 + ...) - u2 - 2*u3 - (T-1)*u_T
        // = T*w_T - u2 - 2*u3 - (T-1)*u_T
        averaged_weights_.Add(features[j],
          static_cast<double>(iteration) * eta * gradient);
      }
    }
  }

  // Make a gradient step with a stepsize of eta, with respect to a vector
  // of features conjoined with a label.
  // The iteration number is provided as input since it is necessary to
  // update the wanna-be "averaged parameters" in an efficient manner.
  void MakeLabelGradientStep(const BinaryFeatures &features,
                             double eta,
                             int iteration,
                             int label,
                             double gradient) {
    for (int j = 0; j < features.size(); ++j) {
      labeled_weights_.Add(features[j], label, -eta * gradient);
    }

    if (use_average_) {
      for (int j = 0; j < features.size(); ++j) {
        averaged_labeled_weights_.Add(features[j], label,
            static_cast<double>(iteration) * eta * gradient);
      }
    }
  }

  // Finalize training, after a total of num_iterations. This is a no-op unless
  // we are averaging the parameter vector, in which case the averaged
  // parameters are finally computed and replace the original parameters.
  void Finalize(int num_iterations) {
    if (use_average_) {
      LOG(INFO) << "Averaging the weights...";

      averaged_weights_.Scale(1.0 / static_cast<double>(num_iterations));
      weights_.Add(averaged_weights_);

      averaged_labeled_weights_.Scale(
          1.0 / static_cast<double>(num_iterations));

      labeled_weights_.Add(averaged_labeled_weights_);
    }
  }

 protected:
  // Average the parameters as in averaged perceptron.
  bool use_average_;

  // Weights and averaged weights for the "simple" features.
  SparseParameterVector weights_;
  SparseParameterVector averaged_weights_;

  // Weights and averaged weights for the "labeled" features.
  SparseLabeledParameterVector labeled_weights_;
  SparseLabeledParameterVector averaged_labeled_weights_;
};

#endif /*PARAMETERS_H_*/
