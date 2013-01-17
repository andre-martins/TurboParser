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

#ifndef SEQUENCEDECODER_H_
#define SEQUENCEDECODER_H_
#include <unordered_map>
#include "Decoder.h"

class SequencePipe;

class SequenceDecoder : public Decoder {
 public:
  SequenceDecoder() {};
  SequenceDecoder(SequencePipe *pipe) : pipe_(pipe) {};
  virtual ~SequenceDecoder() {};

  void Decode(Instance *instance, Parts *parts,
              const vector<double> &scores,
              vector<double> *predicted_output);

  void DecodeCostAugmented(Instance *instance, Parts *parts,
                           const vector<double> &scores,
                           const vector<double> &gold_output,
                           vector<double> *predicted_output,
                           double *cost,
                           double *loss);

  virtual void DecodeMarginals(Instance *instance, Parts *parts,
                                 const vector<double> &scores,
                                 const vector<double> &gold_output,
                                 vector<double> *predicted_output,
                                 double *entropy,
                                 double *loss) {
    // TODO: Implement forward-backward.
    CHECK(false) << "Not implemented yet.";
  }

  void ConvertToFirstOrderModel(
      const vector<vector<double> > &node_scores,
      const vector<vector<vector<double> > > &edge_scores,
      const vector<vector<vector<vector<double> > > > &triplet_scores,
	  const vector<vector<int> > &node_tags,
      vector<vector<double> > *transformed_node_scores
      );

  void RecoverBestPath(
      const vector<int> &best_path,
      const vector<vector<vector<double> > > &edge_scores,
      vector<int> *transformed_best_path);

  double RunViterbi(const vector<vector<double> > &node_scores,
                    const vector<vector<vector<double > > >
                     &edge_scores,
                    vector<int> *best_path);
  double RunViterbi(const vector<vector<double> > &node_scores,
                    const vector<vector<vector<std::pair<int, double>>>>
                     &edge_scores,
                    vector<int> *best_path);

#ifdef USE_CPLEX
  double DecodeCPLEX(Instance *instance, Parts *parts,
      const vector<double> &scores,
      bool relax,
      vector<double> *predicted_output);
#endif

 protected:
  SequencePipe *pipe_;
};

#endif /* SEQUENCEDECODER_H_ */
