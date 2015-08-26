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

#ifndef COREFERENCEDECODER_H_
#define COREFERENCEDECODER_H_

#include "Decoder.h"

class CoreferencePipe;

class CoreferenceDecoder : public Decoder {
 public:
  CoreferenceDecoder() {};
  CoreferenceDecoder(CoreferencePipe *pipe) : pipe_(pipe) {};
  virtual ~CoreferenceDecoder() {};

  void Decode(Instance *instance, Parts *parts,
              const std::vector<double> &scores,
              std::vector<double> *predicted_output);

  void DecodeCostAugmented(Instance *instance, Parts *parts,
                           const std::vector<double> &scores,
                           const std::vector<double> &gold_output,
                           std::vector<double> *predicted_output,
                           double *cost,
                           double *loss);

  void DecodeCostAugmentedMarginals(Instance *instance, Parts *parts,
                                    const std::vector<double> &scores,
                                    const std::vector<double> &gold_output,
                                    std::vector<double> *predicted_output,
                                    double *entropy,
                                    double *cost,
                                    double *loss);

  void DecodeMarginals(Instance *instance, Parts *parts,
                       const std::vector<double> &scores,
                       const std::vector<double> &gold_output,
                       std::vector<double> *predicted_output,
                       double *entropy,
                       double *loss);

  void DecodeBasicMarginals(Instance *instance, Parts *parts,
                            const std::vector<double> &scores,
                            std::vector<double> *predicted_output,
                            double *log_partition_function,
                            double *entropy);

 protected:
  void ComputeLinearCostFunction(Instance *instance,
                                 Parts *parts,
                                 const std::vector<double> &gold_output,
                                 std::vector<double> *p,
                                 double *q);

 protected:
  CoreferencePipe *pipe_;
};

#endif /* COREFERENCEDECODER_H_ */
