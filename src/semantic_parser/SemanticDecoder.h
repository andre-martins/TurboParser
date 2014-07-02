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

#ifndef SEMANTICDECODER_H_
#define SEMANTICDECODER_H_

#include "Decoder.h"
#include "SemanticPart.h"
//#include "ad3/FactorGraph.h"

class SemanticPipe;

class SemanticDecoder : public Decoder {
 public:
  SemanticDecoder() {};
  SemanticDecoder(SemanticPipe *pipe) : pipe_(pipe) {};
  virtual ~SemanticDecoder() {};

  void Decode(Instance *instance, Parts *parts,
              const vector<double> &scores,
              vector<double> *predicted_output);

  void DecodePruner(Instance *instance, Parts *parts,
              const vector<double> &scores,
              vector<double> *predicted_output);

  void DecodePrunerNaive(Instance *instance, Parts *parts,
                         const vector<double> &scores,
                         vector<double> *predicted_output);

  void DecodeCostAugmented(Instance *instance, Parts *parts,
                           const vector<double> &scores,
                           const vector<double> &gold_output,
                           vector<double> *predicted_output,
                           double *cost,
                           double *loss);

  void DecodeMarginals(Instance *instance, Parts *parts,
                       const vector<double> &scores,
                       const vector<double> &gold_output,
                       vector<double> *predicted_output,
                       double *entropy,
                       double *loss);

  void DecodeFactorGraph(Instance *instance, Parts *parts,
                         const vector<double> &scores,
                         bool labeled_decoding,
                         bool relax,
                         vector<double> *predicted_output);

  void BuildBasicIndices(int sentence_length,
                         const vector<SemanticPartPredicate*> &predicate_parts,
                         const vector<SemanticPartArc*> &arcs,
                         vector<vector<int> > *index_predicates,
                         vector<vector<vector<int> > > *arcs_by_predicate);

  void DecodeSemanticGraph(int sentence_length,
                           const vector<SemanticPartPredicate*> &predicate_parts,
                           const vector<SemanticPartArc*> &arcs,
                           const vector<vector<int> > &index_predicates,
                           const vector<vector<vector<int> > > &arcs_by_predicate,
                           const vector<double> &predicate_scores,
                           const vector<double> &arc_scores,
                           vector<bool> *selected_predicates,
                           vector<bool> *selected_arcs,
                           double *value);

 protected:
  void DecodeLabels(Instance *instance, Parts *parts,
                    const vector<double> &scores,
                    vector<int> *best_labeled_parts);

  void DecodeLabelMarginals(Instance *instance, Parts *parts,
                            const vector<double> &scores,
                            vector<double> *total_scores,
                            vector<double> *label_marginals);

  void DecodeBasic(Instance *instance, Parts *parts,
                   const vector<double> &scores,
                   vector<double> *predicted_output,
                   double *value);

  void DecodeBasicMarginals(Instance *instance, Parts *parts,
                            const vector<double> &scores,
                            vector<double> *predicted_output,
                            double *log_partition_function,
                            double *entropy);

 protected:
  SemanticPipe *pipe_;
};

#endif /* SEMANTICDECODER_H_ */
