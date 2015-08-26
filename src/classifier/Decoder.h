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

#ifndef DECODER_H_
#define DECODER_H_

#include "Part.h"
#include "Instance.h"
#include <vector>

using namespace std;

// Abstract class for a decoder. Task-specific decoders should derive from
// this class and implement the pure virtual methods.
class Decoder {
public:
  Decoder() {};
  virtual ~Decoder() {};

  // Given an instance, a set of parts, and a score for each part, find the
  // most likely output (MAP decoding). This is used at test time, and when
  // training with perceptron.
  virtual void Decode(Instance *instance, Parts *parts, 
                      const vector<double> &scores,
                      vector<double> *predicted_output) = 0;

  // Given an instance, a set of parts, a score for each part, and the gold
  // output, find the output that maximizes a combination of the score and
  // a cost function with respect to the gold output (cost-augmented MAP
  // decoding). This is used when training with cost-augmented MIRA or a
  // subgradient algorithm for a structured SVM.
  // This function should output the cost of the predicted output, and
  // also the "loss" for this instance, i.e., the score difference plus the
  // cost ("predicted_output" maximizes this loss).
  // NOTE: the loss must be non-negative.
  virtual void DecodeCostAugmented(Instance *instance, Parts *parts,
                                   const vector<double> &scores,
                                   const vector<double> &gold_output,
                                   vector<double> *predicted_output,
                                   double *cost,
                                   double *loss) = 0;

  // Given an instance, a set of parts, a score for each part, and the gold
  // output, find the posterior marginals for a combination of the score and
  // a cost function with respect to the gold output (cost-augmented marginal
  // decoding). This is used when training with softmax-margin MIRA or a
  // subgradient algorithm for a structured softmax-margin classifier.
  // This function should output the entropy of the distribution, the cost of
  // the predicted output, and also the "loss" for this instance, i.e., the
  // score difference plus the entropy and the cost ("predicted_output"
  // maximizes this loss).
  // NOTE: the loss must be non-negative.
  virtual void DecodeCostAugmentedMarginals(Instance *instance, Parts *parts,
                                   const vector<double> &scores,
                                   const vector<double> &gold_output,
                                   vector<double> *predicted_output,
                                   double *entropy,
                                   double *cost,
                                   double *loss) = 0;

  // Given an instance, a set of parts, and a score for each part, find the
  // posterior marginals for the parts (marginal decoding). This is used when
  // training a CRF.
  // This function should output the entropy of the distribution, and
  // also the "loss" for this instance, i.e., the negative log-probability of
  // the gold output. The latter quantity equals the score of the gold output
  // minus the expected cost, plus the entropy.
  // NOTE: the loss must be non-negative.
  virtual void DecodeMarginals(Instance *instance, Parts *parts,
                               const vector<double> &scores,
                               const vector<double> &gold_output,
                               vector<double> *predicted_output,
                               double *entropy,
                               double *loss) = 0;
};

#endif /* DECODER_H_ */
