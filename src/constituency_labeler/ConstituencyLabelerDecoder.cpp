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

#include "ConstituencyLabelerDecoder.h"
#include "ConstituencyLabelerPart.h"
#include "ConstituencyLabelerPipe.h"
#include "AlgUtils.h"
#include <iostream>
#include "logval.h"

// Define a matrix of doubles using Eigen.
typedef LogVal<double> LogValD;

DEFINE_double(constituency_labeler_train_cost_false_positives, 0.5,
              "Cost for predicting false positives.");
DEFINE_double(constituency_labeler_train_cost_false_negatives, 0.5,
              "Cost for predicting false negatives.");

void ConstituencyLabelerDecoder::DecodeCostAugmented(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    const std::vector<double> &gold_output,
    std::vector<double> *predicted_output,
    double *cost,
    double *loss) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  int num_nodes = sentence->GetNumConstituents();

  int offset_labeled_nodes, num_labeled_nodes;
  labeler_parts->GetOffsetNode(&offset_labeled_nodes,
                               &num_labeled_nodes);

  ////////////////////////////////////////////////////
  // F1: a = 0.5, b = 0.5.
  // Recall: a = 0, b = 1.
  // In general:
  // p = a - (a+b)*z0
  // q = b*sum(z0)
  // p'*z + q = a*sum(z) - (a+b)*z0'*z + b*sum(z0)
  //          = a*(1-z0)'*z + b*(1-z)'*z0.
  ////////////////////////////////////////////////////

  // Penalty for predicting 1 when it is 0 (FP).
  double a = FLAGS_constituency_labeler_train_cost_false_positives;
  // Penalty for predicting 0 when it is 1 (FN).
  double b = FLAGS_constituency_labeler_train_cost_false_negatives;

  // p = 0.5-z0, q = 0.5'*z0, loss = p'*z + q
  double q = 0.0;
  std::vector<double> p(num_labeled_nodes, 0.0);

  std::vector<double> scores_cost = scores;
  for (int r = 0; r < num_labeled_nodes; ++r) {
    p[r] = a - (a+b) * gold_output[offset_labeled_nodes + r];
    scores_cost[offset_labeled_nodes + r] += p[r];
    q += b*gold_output[offset_labeled_nodes + r];
  }

  Decode(instance, parts, scores_cost, predicted_output);

  *cost = q;
  for (int r = 0; r < num_labeled_nodes; ++r) {
    *cost += p[r] * (*predicted_output)[offset_labeled_nodes + r];
  }

  *loss = *cost;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }
}

void ConstituencyLabelerDecoder::Decode(Instance *instance, Parts *parts,
                                        const std::vector<double> &scores,
                                        std::vector<double> *predicted_output) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  int num_nodes = sentence->GetNumConstituents();

  // Create copy of the scores.
  std::vector<double> copied_scores(scores);
  std::vector<int> best_labeled_parts;

  // Decode the labels and update the scores.
  DecodeLabels(instance, parts, copied_scores, &best_labeled_parts);

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  // Write the components of the predicted output that
  // correspond to the labeled parts.
  for (int i = 0; i < num_nodes; ++i) {
    if (best_labeled_parts[i] < 0) continue;
    (*predicted_output)[best_labeled_parts[i]] = 1.0;
  }
}

void ConstituencyLabelerDecoder::DecodeMarginals(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    const std::vector<double> &gold_output,
    std::vector<double> *predicted_output,
    double *entropy,
    double *loss) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeled_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  int num_nodes = sentence->GetNumConstituents();

  // Create copy of the scores.
  std::vector<double> copied_scores(scores);
  std::vector<double> total_scores;
  std::vector<double> label_marginals;
  int offset_labeled_nodes, num_labeled_nodes;
  labeled_parts->GetOffsetNode(&offset_labeled_nodes,
                               &num_labeled_nodes);

  // Decode the labels and update the scores.
  DecodeLabelMarginals(instance, parts, copied_scores, &total_scores,
                       &label_marginals);
  double log_partition_function = 0.0;
  for (int i = 0; i < total_scores.size(); ++i) {
    log_partition_function += total_scores[i];
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  // Write the components of the predicted output that
  // correspond to the labeled parts.
  for (int r = 0; r < num_labeled_nodes; ++r) {
    ConstituencyLabelerPartNode *node =
      static_cast<ConstituencyLabelerPartNode*>(
        (*parts)[offset_labeled_nodes + r]);
    (*predicted_output)[offset_labeled_nodes + r] = label_marginals[r];
  }

  // Recompute the entropy.
  *entropy = log_partition_function;
  for (int r = 0; r < num_labeled_nodes; ++r) {
    *entropy -= (*predicted_output)[offset_labeled_nodes + r] *
      scores[offset_labeled_nodes + r];
  }
  if (*entropy < 0.0) {
    LOG(INFO) << "Entropy truncated to zero (" << *entropy << ")";
    *entropy = 0.0;
  }

  *loss = *entropy;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }
  if (*loss < 0.0) {
    LOG(INFO) << "Loss truncated to zero (" << *loss << ")";
    *loss = 0.0;
  }
}

// Decode the best label for each candidate arc. The output vector
// best_labeled_parts, indexed by the unlabeled arcs, contains the indices
// of the best labeled part for each arc.
void ConstituencyLabelerDecoder::DecodeLabels(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    std::vector<int> *best_labeled_parts) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeled_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerOptions *labeler_options =
    static_cast<ConstituencyLabelerOptions*>(pipe_->GetOptions());
  int num_nodes = sentence->GetNumConstituents();

  best_labeled_parts->resize(num_nodes);
  for (int i = 0; i < num_nodes; ++i) {
    const std::vector<int> &index_node_parts =
        labeled_parts->FindNodeParts(i);
    // Find the best label for each node.
    int best_label = -1;
    double best_score;
    for (int k = 0; k < index_node_parts.size(); ++k) {
      if (best_label < 0 ||
          scores[index_node_parts[k]] > best_score) {
        best_label = index_node_parts[k];
        best_score = scores[best_label];
      }
    }
    (*best_labeled_parts)[i] = best_label;
    if (labeler_options->ignore_null_labels()) {
      if (best_score <= 0.0) {
        (*best_labeled_parts)[i] = -1;
      }
    }
  }
}

// Decode the label marginals for each candidate arc. The output vector
// total_scores contains the sum of exp-scores (over the labels) for each arc;
// label_marginals contains those marginals ignoring the tree constraint.
void ConstituencyLabelerDecoder::DecodeLabelMarginals(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    std::vector<double> *total_scores,
    std::vector<double> *label_marginals) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeled_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerOptions *labeler_options =
    static_cast<ConstituencyLabelerOptions*>(pipe_->GetOptions());
  int num_nodes = sentence->GetNumConstituents();

  int offset_labeled_nodes, num_labeled_nodes;
  labeled_parts->GetOffsetNode(&offset_labeled_nodes, &num_labeled_nodes);
  total_scores->clear();
  total_scores->resize(num_nodes, 0.0);
  label_marginals->clear();
  label_marginals->resize(num_labeled_nodes, 0.0);

  for (int i = 0; i < num_nodes; ++i) {
    const std::vector<int> &index_node_parts =
        labeled_parts->FindNodeParts(i);
    // If no part for null label, initiliaze log partition to exp(0.0) to
    // account the null label which has score 0.0.
    LogValD total_score = (labeler_options->ignore_null_labels())?
      LogValD::One() : LogValD::Zero();
    for (int k = 0; k < index_node_parts.size(); ++k) {
      total_score += LogValD(scores[index_node_parts[k]], false);
    }
    (*total_scores)[i] = total_score.logabs();
    // If no part for null label, initiliaze sum to exp(0.0)/Z to
    // account the null label which has score 0.0.
    double sum = (labeler_options->ignore_null_labels())?
      (1.0 / total_score.as_float()) : 0.0;
    for (int k = 0; k < index_node_parts.size(); ++k) {
      LogValD marginal =
          LogValD(scores[index_node_parts[k]], false) / total_score;
      (*label_marginals)[index_node_parts[k] - offset_labeled_nodes] =
          marginal.as_float();
      sum += marginal.as_float();
    }
    if (!NEARLY_EQ_TOL(sum, 1.0, 1e-9)) {
      LOG(INFO) << "Label marginals don't sum to one: sum = " << sum;
    }
  }
}
