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

#ifndef ENTITYDECODER_H_
#define ENTITYDECODER_H_

#include "../sequence/SequenceDecoder.h"


class EntityDecoder : public SequenceDecoder {
public:
  EntityDecoder() {};
  EntityDecoder(SequencePipe *pipe) : SequenceDecoder(pipe) {}; // EntityPipe SequencePipe??  
  virtual ~EntityDecoder() {};

  virtual void Decode(Instance *instance, Parts *parts,
              const vector<double> &scores,
              vector<double> *predicted_output);

  virtual void DecodeCostAugmented(Instance *instance, Parts *parts,
                           const vector<double> &scores,
                           const vector<double> &gold_output,
                           vector<double> *predicted_output,
                           double *cost,
                           double *loss);

  virtual void DecodeCostAugmentedMarginals(Instance *instance, Parts *parts,
                                            const vector<double> &scores,
                                            const vector<double> &gold_output,
                                            vector<double> *predicted_output,
                                            double *entropy,
                                            double *cost,
                                            double *loss) {
    CHECK(false) << "Not implemented yet.";
  }

  virtual void DecodeMarginals(Instance *instance, Parts *parts,
                               const vector<double> &scores,
                               const vector<double> &gold_output,
                               vector<double> *predicted_output,
                               double *entropy,
                               double *loss) {
    // TODO: Implement forward-backward.
    CHECK(false) << "Not implemented yet.";
  }

#if 0
  void ConvertToFirstOrderModel(
    const std::vector<SequenceDecoderNodeScores> &node_scores,
    const std::vector<SequenceDecoderEdgeScores> &edge_scores,
    const std::vector<SequenceDecoderEdgeScores> &triplet_scores,
    std::vector<SequenceDecoderNodeScores> *transformed_node_scores,
    std::vector<SequenceDecoderEdgeScores> *transformed_edge_scores);

  void RecoverBestPath(const std::vector<int> &best_path,
                       std::vector<int> *transformed_best_path);

  double SolveMarkovZeroOrder(const std::vector<SequenceDecoderNodeScores> &node_scores,
                              std::vector<int> *best_path);

  double RunViterbi(const vector<vector<double> > &node_scores,
                    const vector<vector<vector<double > > >
                    &edge_scores,
                    vector<int> *best_path);

  double RunViterbi(const std::vector<SequenceDecoderNodeScores> &node_scores,
                    const std::vector<SequenceDecoderEdgeScores> &edge_scores,
                    std::vector<int> *best_path);

#ifdef USE_CPLEX
  double DecodeCPLEX(Instance *instance, Parts *parts,
                     const vector<double> &scores,
                     bool relax,
                     vector<double> *predicted_output);
#endif  //
#endif  // #if 0

protected:
  //EntityPipe *pipe_; //SequencePipe *pipe_;
};

#endif /* SEQUENCEDECODER_H_ */
