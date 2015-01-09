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

#if 0
// Define a matrix of doubles using Eigen.
typedef LogVal<double> LogValD;

void DependencyLabelerDecoder::DecodeCostAugmented(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    const std::vector<double> &gold_output,
    std::vector<double> *predicted_output,
    double *cost,
    double *loss) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int offset_arcs, num_arcs;

  dependency_parts->GetOffsetLabeledArc(&offset_arcs, &num_arcs);

  // p = 0.5-z0, q = 0.5'*z0, loss = p'*z + q
  double q = 0.0;
  vector<double> p(num_arcs, 0.0);

  vector<double> scores_cost = scores;
  for (int r = 0; r < num_arcs; ++r) {
    p[r] = 0.5 - gold_output[offset_arcs + r];
    scores_cost[offset_arcs + r] += p[r];
    q += 0.5*gold_output[offset_arcs + r];
  }

  Decode(instance, parts, scores_cost, predicted_output);

  *cost = q;
  for (int r = 0; r < num_arcs; ++r) {
    *cost += p[r] * (*predicted_output)[offset_arcs + r];
  }

  *loss = *cost;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }
}

void DependencyLabelerDecoder::Decode(Instance *instance, Parts *parts,
                                      const std::vector<double> &scores,
                                      std::vector<double> *predicted_output) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<int> best_labeled_parts;
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  // If labeled parsing, decode the labels and update the scores.
  DecodeLabels(instance, parts, copied_scores, &best_labeled_parts);
  for (int r = 0; r < best_labeled_parts.size(); ++r) {
    copied_scores[offset_arcs + r] += copied_scores[best_labeled_parts[r]];
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  // Write the components of the predicted output that
  // correspond to the labeled parts.
  for (int r = 0; r < num_arcs; ++r) {
    (*predicted_output)[offset_arcs + r] = 1.0;
    (*predicted_output)[best_labeled_parts[r]] = 1.0;
  }
}

void DependencyLabelerDecoder::DecodeMarginals(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    const std::vector<double> &gold_output,
    std::vector<double> *predicted_output,
    double *entropy,
    double *loss) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  // Right now, only allow marginal inference for arc-factored models.
  CHECK(dependency_parts->IsArcFactored());

  // Create copy of the scores.
  std::vector<double> copied_scores(scores);
  std::vector<double> total_scores;
  std::vector<double> label_marginals;
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  int offset_labeled_arcs, num_labeled_arcs;
  dependency_parts->GetOffsetLabeledArc(&offset_labeled_arcs,
                                        &num_labeled_arcs);

  // Decode the labels and update the scores.
  DecodeLabelMarginals(instance, parts, copied_scores, &total_scores,
                       &label_marginals);
  double log_partition_function = 0.0;
  for (int r = 0; r < total_scores.size(); ++r) {
    copied_scores[offset_arcs + r] += total_scores[r];
    log_partition_function += total_scores[r];
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  // If labeled parsing, write the components of the predicted output that
  // correspond to the labeled parts.
  for (int r = 0; r < num_labeled_arcs; ++r) {
    DependencyPartLabeledArc *labeled_arc =
      static_cast<DependencyPartLabeledArc*>((*parts)[offset_labeled_arcs + r]);
    int index_arc = dependency_parts->FindArc(labeled_arc->head(),
                                              labeled_arc->modifier());
    CHECK_GE(index_arc, 0);
    (*predicted_output)[index_arc] = 1.0;
    (*predicted_output)[offset_labeled_arcs + r] = label_marginals[r];
  }

  // Recompute the entropy.
  *entropy = log_partition_function;
  for (int r = 0; r < num_labeled_arcs; ++r) {
    *entropy -= (*predicted_output)[offset_labeled_arcs + r] *
      scores[offset_labeled_arcs + r];
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
void DependencyLabelerDecoder::DecodeLabels(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    std::vector<int> *best_labeled_parts) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  int offset, num_arcs;
  dependency_parts->GetOffsetArc(&offset, &num_arcs);
  best_labeled_parts->resize(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    DependencyPartArc *arc =
        static_cast<DependencyPartArc*>((*parts)[offset + r]);
    const vector<int> &index_labeled_parts =
        dependency_parts->FindLabeledArcs(arc->head(), arc->modifier());
    // Find the best label for each candidate arc.
    int best_label = -1;
    double best_score;
    for (int k = 0; k < index_labeled_parts.size(); ++k) {
      if (best_label < 0 ||
          scores[index_labeled_parts[k]] > best_score) {
        best_label = index_labeled_parts[k];
        best_score = scores[best_label];
      }
    }
    (*best_labeled_parts)[r] = best_label;
  }
}

// Decode the label marginals for each candidate arc. The output vector
// total_scores contains the sum of exp-scores (over the labels) for each arc;
// label_marginals contains those marginals ignoring the tree constraint.
void DependencyLabelerDecoder::DecodeLabelMarginals(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    std::vector<double> *total_scores,
    std::vector<double> *label_marginals) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  int offset, num_arcs;
  int offset_labeled, num_labeled_arcs;
  dependency_parts->GetOffsetArc(&offset, &num_arcs);
  dependency_parts->GetOffsetLabeledArc(&offset_labeled, &num_labeled_arcs);
  total_scores->clear();
  total_scores->resize(num_arcs, 0.0);
  label_marginals->clear();
  label_marginals->resize(num_labeled_arcs, 0.0);

  for (int r = 0; r < num_arcs; ++r) {
    DependencyPartArc *arc =
        static_cast<DependencyPartArc*>((*parts)[offset + r]);
    const vector<int> &index_labeled_parts =
        dependency_parts->FindLabeledArcs(arc->head(), arc->modifier());
    // Find the best label for each candidate arc.
    LogValD total_score = LogValD::Zero();
    for (int k = 0; k < index_labeled_parts.size(); ++k) {
      total_score += LogValD(scores[index_labeled_parts[k]], false);
    }
    (*total_scores)[r] = total_score.logabs();
    double sum = 0.0;
    for (int k = 0; k < index_labeled_parts.size(); ++k) {
      LogValD marginal =
          LogValD(scores[index_labeled_parts[k]], false) / total_score;
      (*label_marginals)[index_labeled_parts[k] - offset_labeled] =
          marginal.as_float();
      sum += marginal.as_float();
    }
    if (!NEARLY_EQ_TOL(sum, 1.0, 1e-9)) {
      LOG(INFO) << "Label marginals don't sum to one: sum = " << sum;
    }
  }
}

#endif
