/// Copyright (c) 2012-2013 Andre Martins
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

#ifndef DEPENDENCYLABELERDECODER_H_
#define DEPENDENCYLABELERDECODER_H_

#include "Decoder.h"
#include "DependencyLabelerPart.h"

class DependencyLabelerPipe;

class DependencyLabelerDecoder : public Decoder {
 public:
  DependencyLabelerDecoder() {};
  DependencyLabelerDecoder(DependencyLabelerPipe *pipe) : pipe_(pipe) {};
  virtual ~DependencyLabelerDecoder() {};

  void Decode(Instance *instance, Parts *parts,
              const std::vector<double> &scores,
              std::vector<double> *predicted_output);

  void DecodeBasic(Instance *instance, Parts *parts,
                   const std::vector<double> &scores,
                   std::vector<double> *predicted_output);

  void DecodeCostAugmented(Instance *instance, Parts *parts,
                           const std::vector<double> &scores,
                           const std::vector<double> &gold_output,
                           std::vector<double> *predicted_output,
                           double *cost,
                           double *loss);

  void DecodeMarginals(Instance *instance, Parts *parts,
                       const std::vector<double> &scores,
                       const std::vector<double> &gold_output,
                       std::vector<double> *predicted_output,
                       double *entropy,
                       double *loss) {
    CHECK(false) << "Not implemented yet.";
  }

 protected:
  double RunViterbi(
      const std::vector<std::vector<double> > &node_scores,
      const std::vector<std::vector<std::vector<double> > > &edge_scores,
      std::vector<int> *best_path);

 protected:
  DependencyLabelerPipe *pipe_;
};

#endif /* DEPENDENCYLABELERDECODER_H_ */
