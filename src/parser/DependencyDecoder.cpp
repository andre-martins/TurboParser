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

#include "DependencyDecoder.h"
#include "DependencyPart.h"
#include "DependencyPipe.h"
#include "FactorTree.h"
#include "FactorHeadAutomaton.h"
#include "FactorGrandparentHeadAutomaton.h"
#include "FactorSequence.h"
#include "AlgUtils.h"
#include <iostream>
#include <Eigen/Dense>
#include "logval.h"

// Define a matrix of doubles using Eigen.
typedef LogVal<double> LogValD;
namespace Eigen {
typedef Eigen::Matrix<LogValD, Dynamic, Dynamic> MatrixXlogd;
}

using namespace std;

void DependencyDecoder::DecodeCostAugmented(Instance *instance, Parts *parts,
                                            const vector<double> &scores,
                                            const vector<double> &gold_output,
                                            vector<double> *predicted_output,
                                            double *cost,
                                            double *loss) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int offset_arcs, num_arcs;

  if (pipe_->GetDependencyOptions()->labeled()) {
    dependency_parts->GetOffsetLabeledArc(&offset_arcs, &num_arcs);
  } else {
    dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  }

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

void DependencyDecoder::DecodeMarginals(Instance *instance, Parts *parts,
                                        const vector<double> &scores,
                                        const vector<double> &gold_output,
                                        vector<double> *predicted_output,
                                        double *entropy,
                                        double *loss) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  // Right now, only allow marginal inference for arc-factored models.
  CHECK(dependency_parts->IsArcFactored());

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<double> total_scores;
  vector<double> label_marginals;
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  int offset_labeled_arcs, num_labeled_arcs;
  dependency_parts->GetOffsetLabeledArc(&offset_labeled_arcs,
                                        &num_labeled_arcs);

  // If labeled parsing, decode the labels and update the scores.
  if (pipe_->GetDependencyOptions()->labeled()) {
    DecodeLabelMarginals(instance, parts, copied_scores, &total_scores,
        &label_marginals);
    for (int r = 0; r < total_scores.size(); ++r) {
      copied_scores[offset_arcs + r] += total_scores[r];
    }
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  double log_partition_function;
  DecodeMatrixTree(instance, parts, copied_scores, predicted_output,
                   &log_partition_function, entropy);

  // If labeled parsing, write the components of the predicted output that
  // correspond to the labeled parts.
  if (pipe_->GetDependencyOptions()->labeled()) {
    for (int r = 0; r < num_labeled_arcs; ++r) {
      DependencyPartLabeledArc *labeled_arc =
          static_cast<DependencyPartLabeledArc*>(
              (*parts)[offset_labeled_arcs + r]);
      int index_arc = dependency_parts->FindArc(labeled_arc->head(),
                                                labeled_arc->modifier());
      CHECK_GE(index_arc, 0);
      (*predicted_output)[offset_labeled_arcs + r] =
          label_marginals[r] * (*predicted_output)[index_arc];
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
void DependencyDecoder::DecodeLabels(Instance *instance, Parts *parts,
                                     const vector<double> &scores,
                                     vector<int> *best_labeled_parts) {
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
void DependencyDecoder::DecodeLabelMarginals(Instance *instance, Parts *parts,
                                     const vector<double> &scores,
                                     vector<double> *total_scores,
                                     vector<double> *label_marginals) {
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

void DependencyDecoder::Decode(Instance *instance, Parts *parts, 
                               const vector<double> &scores,
                               vector<double> *predicted_output) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<int> best_labeled_parts;
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  // If labeled parsing, decode the labels and update the scores.
  if (pipe_->GetDependencyOptions()->labeled()) {
    DecodeLabels(instance, parts, copied_scores, &best_labeled_parts);
    for (int r = 0; r < best_labeled_parts.size(); ++r) {
      copied_scores[offset_arcs + r] += copied_scores[best_labeled_parts[r]];
    }
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  if (dependency_parts->IsArcFactored() ||
      dependency_parts->IsLabeledArcFactored()) {
    double value;
    DecodeBasic(instance, parts, copied_scores, predicted_output, &value);
  } else {
#ifdef USE_CPLEX
    DecodeCPLEX(instance, parts, copied_scores, false, true, predicted_output);
#else
    DecodeFactorGraph(instance, parts, copied_scores, false, true,
                      predicted_output);
#endif
  }

  // If labeled parsing, write the components of the predicted output that
  // correspond to the labeled parts.
  if (pipe_->GetDependencyOptions()->labeled()) {
    for (int r = 0; r < num_arcs; ++r) {
      (*predicted_output)[best_labeled_parts[r]] =
          (*predicted_output)[offset_arcs + r];
    }
  }
}

void DependencyDecoder::DecodePruner(Instance *instance, Parts *parts,
    const vector<double> &scores,
    vector<double> *predicted_output) {
  int sentence_length =
    static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  double posterior_threshold =
      pipe_->GetDependencyOptions()->GetPrunerPosteriorThreshold();
  int max_heads = pipe_->GetDependencyOptions()->GetPrunerMaxHeads();
  if (max_heads < 0) max_heads = sentence_length;
  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  CHECK(dependency_parts->IsArcFactored());

  double entropy;
  double log_partition_function;
  vector<double> posteriors;
  DecodeMatrixTree(instance, parts, scores, &posteriors,
                   &log_partition_function, &entropy);

  int num_used_parts = 0;
  for (int m = 1; m < sentence_length; ++m) {
    vector<pair<double,int> > scores_heads;
    for (int h = 0; h < sentence_length; ++h) {
      int r = dependency_parts->FindArc(h, m);
      if (r < 0) continue;
      scores_heads.push_back(pair<double,int>(-posteriors[r], r));
    }
    if (scores_heads.size() == 0) continue;
    sort(scores_heads.begin(), scores_heads.end());
    double max_posterior = -scores_heads[0].first;
    for (int k = 0; k < max_heads && k < scores_heads.size(); ++k) {
      int r = scores_heads[k].second;
      if (-scores_heads[k].first >= posterior_threshold * max_posterior) {
        ++num_used_parts;
        (*predicted_output)[r] = 1.0;
      } else {
        break;
      }
    }
  }

  VLOG(2) << "Pruning reduced to "
          << static_cast<double>(num_used_parts) /
                static_cast<double>(sentence_length)
          << " candidate heads per word.";
}

void DependencyDecoder::DecodePrunerNaive(Instance *instance, Parts *parts,
    const vector<double> &scores,
    vector<double> *predicted_output) {
  int sentence_length =
    static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int max_heads = pipe_->GetDependencyOptions()->GetPrunerMaxHeads();
  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  CHECK(dependency_parts->IsArcFactored());

  // Get max_heads heads per modifier.
  for (int m = 1; m < sentence_length; ++m) {
    vector<pair<double,int> > scores_heads;
    for (int h = 0; h < sentence_length; ++h) {
      int r = dependency_parts->FindArc(h, m);
      if (r < 0) continue;
      scores_heads.push_back(pair<double,int>(-scores[r], r));
    }
    sort(scores_heads.begin(), scores_heads.end());
    for (int k = 0; k < max_heads && k < scores_heads.size(); ++k) {
      int r = scores_heads[k].second;
      (*predicted_output)[r] = 1.0;
    }
  }
}

// Decoder for the basic model; it finds a maximum weighted arborescence
// using Edmonds' algorithm (which runs in O(n^2)).
void DependencyDecoder::DecodeBasic(Instance *instance, Parts *parts,
                                    const vector<double> &scores,
                                    vector<double> *predicted_output,
                                    double *value) {
  int sentence_length = 
    static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  vector<DependencyPartArc*> arcs(num_arcs, NULL);
  vector<double> scores_arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    arcs[r] = static_cast<DependencyPartArc*>((*parts)[offset_arcs + r]);
    scores_arcs[r] = scores[offset_arcs + r];
  }

  vector<int> heads;
  RunChuLiuEdmonds(sentence_length, arcs, scores_arcs, &heads, value);

  predicted_output->resize(parts->size());
  for (int r = 0; r < num_arcs; ++r) {
    (*predicted_output)[offset_arcs + r] = 0.0;
  }
  for (int m = 1; m < sentence_length; ++m) {
    int h = heads[m];
    int r = dependency_parts->FindArc(h, m);
    if (r < 0) {
      LOG(INFO) << "No arc " << h << " -> " << m;
    } else {
      (*predicted_output)[offset_arcs + r] = 1.0;
    }
  }
}

// Decoder for the basic model; it finds a maximum weighted arborescence
// using Edmonds' algorithm (which runs in O(n^2)).
void DependencyDecoder::RunChuLiuEdmondsIteration(
    vector<bool> *disabled,
    vector<vector<int> > *candidate_heads,
    vector<vector<double> > *candidate_scores,
    vector<int> *heads,
    double *value) {
  // Original number of nodes (including the root).
  int length = disabled->size();

  // Pick the best incoming arc for each node.
  heads->resize(length);
  vector<double> best_scores(length);
  for (int m = 1; m < length; ++m) {
    if ((*disabled)[m]) continue;
    int best = -1;
    for (int k = 0; k < (*candidate_heads)[m].size(); ++k) {
      if (best < 0 ||
          (*candidate_scores)[m][k] > (*candidate_scores)[m][best]) {
        best = k;
      }
    }
    if (best < 0) {
      // No spanning tree exists. Assign the parent of this node
      // to the root, and give it a minus infinity score.
      (*heads)[m] = 0;
      best_scores[m] = -std::numeric_limits<double>::infinity();
    } else {
      (*heads)[m] = (*candidate_heads)[m][best]; //best;
      best_scores[m] = (*candidate_scores)[m][best]; //best;
    }
  }

  // Look for cycles. Return after the first cycle is found.
  vector<int> cycle;
  vector<int> visited(length, 0);
  for (int m = 1; m < length; ++m) {
    if ((*disabled)[m]) continue;
    // Examine all the ancestors of m until the root or a cycle is found.
    int h = m;
    while (h != 0) {
      // If already visited, break and check if it is part of a cycle.
      // If visited[h] < m, the node was visited earlier and seen not
      // to be part of a cycle.
      if (visited[h]) break;
      visited[h] = m;
      h = (*heads)[h];
    }

    // Found a cycle to which h belongs.
    // Obtain the full cycle.
    if (visited[h] == m) {
      m = h;
      do {
        cycle.push_back(m);
        m = (*heads)[m];
      } while (m != h);
      break;
    }
  }

  // If there are no cycles, then this is a well formed tree.
  if (cycle.empty()) {
    *value = 0.0;
    for (int m = 1; m < length; ++m) {
      *value += best_scores[m];
    }
    return;
  }

  // Build a cycle membership vector for constant-time querying and compute the
  // score of the cycle.
  // Nominate a representative node for the cycle and disable all the others.
  double cycle_score = 0.0;
  vector<bool> in_cycle(length, false);
  int representative = cycle[0];
  for (int k = 0; k < cycle.size(); ++k) {
    int m = cycle[k];
    in_cycle[m] = true;
    cycle_score += best_scores[m];
    if (m != representative) (*disabled)[m] = true;
  }

  // Contract the cycle.
  // 1) Update the score of each child to the maximum score achieved by a parent
  // node in the cycle.
  vector<int> best_heads_cycle(length);
  for (int m = 1; m < length; ++m) {
    if ((*disabled)[m] || m == representative) continue;
    double best_score;
    // If the list of candidate parents of m is shorter than the length of
    // the cycle, use that. Otherwise, loop through the cycle.
    int best = -1;
    for (int k = 0; k < (*candidate_heads)[m].size(); ++k) {
      if (!in_cycle[(*candidate_heads)[m][k]]) continue;
      if (best < 0 || (*candidate_scores)[m][k] > best_score) {
        best = k;
        best_score = (*candidate_scores)[m][best];
      }
    }
    if (best < 0) continue;
    best_heads_cycle[m] = (*candidate_heads)[m][best];

    // Reconstruct the list of candidate heads for this m.
    int l = 0;
    for (int k = 0; k < (*candidate_heads)[m].size(); ++k) {
      int h = (*candidate_heads)[m][k];
      double score = (*candidate_scores)[m][k];
      if (!in_cycle[h]) {
        (*candidate_heads)[m][l] = h;
        (*candidate_scores)[m][l] = score;
        ++l;
      }
    }
    // If h is in the cycle and is not the representative node,
    // it will be dropped from the list of candidate heads.
    (*candidate_heads)[m][l] = representative;
    (*candidate_scores)[m][l] = best_score;
    (*candidate_heads)[m].resize(l+1);
    (*candidate_scores)[m].resize(l+1);
  }

  // 2) Update the score of each candidate parent of the cycle supernode.
  vector<int> best_modifiers_cycle(length, -1);
  vector<int> candidate_heads_representative;
  vector<double> candidate_scores_representative;

  vector<double> best_scores_cycle(length);
  // Loop through the cycle.
  for (int k = 0; k < cycle.size(); ++k) {
    int m = cycle[k];
    for (int l = 0; l < (*candidate_heads)[m].size(); ++l) {
      // Get heads out of the cycle.
      int h = (*candidate_heads)[m][l];
      if (in_cycle[h]) continue;

      double score = (*candidate_scores)[m][l] - best_scores[m];
      if (best_modifiers_cycle[h] < 0 || score > best_scores_cycle[h]) {
        best_modifiers_cycle[h] = m;
        best_scores_cycle[h] = score;
      }
    }
  }
  for (int h = 0; h < length; ++h) {
    if (best_modifiers_cycle[h] < 0) continue;
    double best_score = best_scores_cycle[h] + cycle_score;
    candidate_heads_representative.push_back(h);
    candidate_scores_representative.push_back(best_score);
  }

  // Reconstruct the list of candidate heads for the representative node.
  (*candidate_heads)[representative] = candidate_heads_representative;
  (*candidate_scores)[representative] = candidate_scores_representative;

  // Save the current head of the representative node (it will be overwritten).
  int head_representative = (*heads)[representative];

  // Call itself recursively.
  RunChuLiuEdmondsIteration(disabled,
                            candidate_heads,
                            candidate_scores,
                            heads,
                            value);

  // Uncontract the cycle.
  int h = (*heads)[representative];
  (*heads)[representative] = head_representative;
  (*heads)[best_modifiers_cycle[h]] = h;

  for (int m = 1; m < length; ++m) {
    if ((*disabled)[m]) continue;
    if ((*heads)[m] == representative) {
      // Get the right parent from within the cycle.
      (*heads)[m] = best_heads_cycle[m];
    }
  }
  for (int k = 0; k < cycle.size(); ++k) {
    int m = cycle[k];
    (*disabled)[m] = false;
  }
}

void DependencyDecoder::RunChuLiuEdmonds(int sentence_length,
                                         const vector<DependencyPartArc*> &arcs,
                                         const vector<double> &scores,
                                         vector<int> *heads,
                                         double *value) {
  vector<vector<int> > candidate_heads(sentence_length);
  vector<vector<double> > candidate_scores(sentence_length);
  vector<bool> disabled(sentence_length, false);
  for (int r = 0; r < arcs.size(); ++r) {
    int h = arcs[r]->head();
    int m = arcs[r]->modifier();
    candidate_heads[m].push_back(h);
    candidate_scores[m].push_back(scores[r]);
  }

  heads->assign(sentence_length, -1);
  RunChuLiuEdmondsIteration(&disabled, &candidate_heads,
                            &candidate_scores, heads, value);
}

// Marginal decoder for the basic model; it invokes the matrix-tree theorem.
void DependencyDecoder::DecodeMatrixTree(Instance *instance, Parts *parts,
                                    const vector<double> &scores,
                                    vector<double> *predicted_output,
                                    double *log_partition_function,
                                    double *entropy) {
  int sentence_length =
    static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  // Matrix for storing the potentials.
  Eigen::MatrixXlogd potentials(sentence_length, sentence_length);
  // Kirchhoff matrix.
  Eigen::MatrixXlogd kirchhoff(sentence_length-1, sentence_length-1);

  // Compute an offset to improve numerical stability. This is a constant that
  // is subtracted from all scores.
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  double constant = 0.0;
  for (int r = 0; r < num_arcs; ++r) {
    constant += scores[offset_arcs + r];
  }
  constant /= static_cast<double>(num_arcs);

  // Set the potentials.
  for (int h = 0; h < sentence_length; ++h) {
    for (int m = 0; m < sentence_length; ++m) {
      potentials(m, h) = LogValD::Zero();
      int r = dependency_parts->FindArc(h, m);
      if (r >= 0) {
        potentials(m, h) = LogValD(scores[r] - constant, false);
      }
    }
  }

  // Set the Kirchhoff matrix.
  for (int h = 0; h < sentence_length - 1; ++h) {
    for (int m = 0; m < sentence_length - 1; ++m) {
      kirchhoff(h, m) = -potentials(m+1, h+1);
    }
  }
  for (int m = 1; m < sentence_length; ++m) {
    LogValD sum = LogValD::Zero();
    for (int h = 0; h < sentence_length; ++h) {
      sum += potentials(m, h);
    }
    kirchhoff(m-1, m-1) = sum;
  }

  // Inverse of the Kirchoff matrix.
  Eigen::FullPivLU<Eigen::MatrixXlogd> lu(kirchhoff);
  Eigen::MatrixXlogd inverted_kirchhoff = lu.inverse();
  *log_partition_function = lu.determinant().logabs() +
      constant * (sentence_length - 1);

  Eigen::MatrixXlogd marginals(sentence_length, sentence_length);
  for (int h = 0; h < sentence_length; ++h) {
    marginals(0, h) = LogValD::Zero();
  }
  for (int m = 1; m < sentence_length; ++m) {
    marginals(m, 0) = potentials(m, 0) * inverted_kirchhoff(m-1, m-1);
    for (int h = 1; h < sentence_length; ++h) {
      marginals(m, h) = potentials(m, h) * 
        (inverted_kirchhoff(m-1, m-1) - inverted_kirchhoff(m-1, h-1));
    }
  }

  // Compute the entropy.
  predicted_output->resize(parts->size());
  *entropy = *log_partition_function;
  for (int r = 0; r < num_arcs; ++r) {
    int h = static_cast<DependencyPartArc*>((*parts)[offset_arcs + r])->head();
    int m = static_cast<DependencyPartArc*>((*parts)[offset_arcs + r])->modifier();
    if (marginals(m, h).signbit()) {
      if (!NEARLY_ZERO_TOL(marginals(m, h).as_float(), 1e-6)) {
        LOG(INFO) << "Marginals truncated to zero (" << marginals(m, h).as_float() << ")";
      }
      CHECK(!std::isnan(marginals(m, h).as_float()));
    } else if (marginals(m, h).logabs() > 0) {
      if (!NEARLY_ZERO_TOL(marginals(m, h).as_float() - 1.0, 1e-6)) {
        LOG(INFO) << "Marginals truncated to one (" << marginals(m, h).as_float() << ")";
      }
    }
    (*predicted_output)[offset_arcs + r] = marginals(m, h).as_float();
    *entropy -= (*predicted_output)[offset_arcs + r] * scores[offset_arcs + r];
  }
  if (*entropy < 0.0) {
    if (!NEARLY_ZERO_TOL(*entropy, 1e-6)) {
      LOG(INFO) << "Entropy truncated to zero (" << *entropy << ")";
    }
    *entropy = 0.0;
  }
}

// Decode building a factor graph and calling the AD3 algorithm.
void DependencyDecoder::DecodeFactorGraph(Instance *instance, Parts *parts,
                                          const vector<double> &scores,
                                          bool single_root,
                                          bool relax,
                                          vector<double> *predicted_output) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyInstanceNumeric* sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  CHECK(!single_root);
  CHECK(relax);

  // Get the offsets for the different parts.
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  int offset_labeled_arcs, num_labeled_arcs;
  dependency_parts->GetOffsetLabeledArc(&offset_labeled_arcs,
                                        &num_labeled_arcs);
  int offset_siblings, num_siblings;
  dependency_parts->GetOffsetSibl(&offset_siblings, &num_siblings);
  int offset_next_siblings, num_next_siblings;
  dependency_parts->GetOffsetNextSibl(&offset_next_siblings,
                                      &num_next_siblings);
  int offset_grandparents, num_grandparents;
  dependency_parts->GetOffsetGrandpar(&offset_grandparents, &num_grandparents);
  int offset_nonprojective, num_nonprojective;
  dependency_parts->GetOffsetNonproj(&offset_nonprojective, &num_nonprojective);
  int offset_path, num_path;
  dependency_parts->GetOffsetPath(&offset_path, &num_path);
  int offset_bigrams, num_bigrams;
  dependency_parts->GetOffsetHeadBigr(&offset_bigrams, &num_bigrams);

  // Define what parts are used.
  bool use_arbitrary_sibling_parts = (num_siblings > 0);
  bool use_next_sibling_parts = (num_next_siblings > 0);
  bool use_grandparent_parts = (num_grandparents > 0);
  bool use_nonprojective_arc_parts = (num_nonprojective > 0);
  bool use_path_parts = (num_path > 0);
  bool use_head_bigram_parts = (num_bigrams > 0);

  // Define optional configurations of the factor graph.
  bool use_tree_factor = true;
  bool use_head_automata = true;
  bool use_grandparent_head_automata = true;
  bool use_head_bigram_sequence_factor = true;
  // Evidence vector that allows to assign evidence to each variable.
  bool add_evidence = false;
  vector<int> evidence;
  // If non-projective arc parts or path parts are being used, then we will
  // use flows instead of a tree factor.
  // Currently, if flows are used, no large factors (such as head automata
  // or head bigrams) will be used. That is because adding evidence to those
  // large factors is not yet implemented.
  if (use_nonprojective_arc_parts || use_path_parts) {
    use_tree_factor = false;
    use_head_automata = false;
    use_grandparent_head_automata = false;
    use_head_bigram_sequence_factor = false;
    add_evidence = true;
  }
  bool use_flows = !use_tree_factor;

  // Define zero/infinity potentials (not used if add_evidence = true).
  double log_potential_zero = -50;
  double log_potential_infinity = -log_potential_zero;

  // Variables of the factor graph.
  vector<AD3::BinaryVariable*> variables;

  // Indices that allow to identify the part corresponding to each variable.
  vector<int> part_indices_;
  vector<int> additional_part_indices;
  vector<int> factor_part_indices_;

  // Create factor graph.
  AD3::FactorGraph *factor_graph = new AD3::FactorGraph;
  int verbosity = 1;
  if (VLOG_IS_ON(2)) {
    verbosity = 2;
  }
  factor_graph->SetVerbosity(verbosity);

  // Compute the transitivity closure of the dependency graph to get the set of
  // possible directed paths.
  // In practice, this is always pretty dense, so there is not a big payoff.
  vector<vector<bool> > graph_paths;
  bool prune_paths = true;
  if (use_flows) {
    if (prune_paths) {
      graph_paths.assign(sentence->size(),
                         vector<bool>(sentence->size(), false));
      for (int r = 0; r < num_arcs; ++r) {
        DependencyPartArc *arc = static_cast<DependencyPartArc*>(
            (*dependency_parts)[offset_arcs + r]);
        int h = arc->head();
        int m = arc->modifier();
        graph_paths[h][m] = true;
      }
      timeval start, end;
      gettimeofday(&start, NULL);
      ComputeTransitiveClosure(&graph_paths);
      for (int i = 0; i < sentence->size(); ++i) {
        graph_paths[i][i] = true;
      }
      gettimeofday(&end, NULL);
      double elapsed_time_paths = diff_ms(end,start);
      int num_possible_paths = 0;
      for (int i = 0; i < sentence->size(); ++i) {
        for (int j = 0; j < sentence->size(); ++j) {
          if (i == j || graph_paths[i][j]) ++num_possible_paths;
        }
      }
      VLOG(2) << num_arcs << " possible arcs and "
              << num_possible_paths << " possible paths in "
              << sentence->size() * sentence->size()
              << " (took " << elapsed_time_paths << " ms.)";
    }
  }

  // Build arc variables.
  for (int r = 0; r < num_arcs; ++r) {
    AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
    variable->SetLogPotential(scores[offset_arcs + r]);
    variables.push_back(variable);
    part_indices_.push_back(offset_arcs + r);
    evidence.push_back(-1);
  }

  //////////////////////////////////////////////////////////////////////
  // Build tree factor.
  //////////////////////////////////////////////////////////////////////

  if (use_tree_factor) {
    // Build tree factor.
    vector<AD3::BinaryVariable*> local_variables(num_arcs);
    vector<DependencyPartArc*> arcs(num_arcs);
    for (int r = 0; r < num_arcs; ++r) {
      local_variables[r] = variables[r];
      arcs[r] = static_cast<DependencyPartArc*>((*parts)[offset_arcs + r]);
    }
    AD3::FactorTree *factor = new AD3::FactorTree;
    factor->Initialize(sentence->size(), arcs, this);
    factor_graph->DeclareFactor(factor, local_variables, true);
    factor_part_indices_.push_back(-1);
  } else {
    // Build the "single parent" factors.
    for (int m = 1; m < sentence->size(); ++m) {
      vector<AD3::BinaryVariable*> local_variables;
      for (int h = 0; h < sentence->size(); ++h) {
        int r = dependency_parts->FindArc(h, m);
        if (r < 0) continue;
        local_variables.push_back(variables[r]);
      }      
      factor_graph->CreateFactorXOR(local_variables);
      factor_part_indices_.push_back(-1);
    }
  }

  int offset_path_variables = -1;
  if (use_flows) {
    // Create flow variables.
    int offset_flow_variables = variables.size();
    for (int r = 0; r < num_arcs; ++r) {
      DependencyPartArc *arc =
          static_cast<DependencyPartArc*>((*dependency_parts)[offset_arcs + r]);
      int h = arc->head();
      int m = arc->modifier();
      for (int k = 0; k < sentence->size(); ++k) {
        // Create flow variable denoting that arc (h,m) carries flow to k.
        AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
        // No arc carries flow to the root or to its head.
        int evidence_value = -1;
        if (k == 0 || k == h) {
          if (add_evidence) {
            evidence_value = 0;
          } else {
            variable->SetLogPotential(log_potential_zero);
          }
        } else if (prune_paths && add_evidence && !graph_paths[m][k]) {
          evidence_value = 0;
        }
        variables.push_back(variable);
        part_indices_.push_back(-1); // Auxiliary variable; no part for it.
        evidence.push_back(evidence_value);
      }
    }

    // Create path variables.
    offset_path_variables = variables.size();
    for (int d = 0; d < sentence->size(); ++d) {
      for (int a = 0; a < sentence->size(); ++a) {
        // Create path variable denoting that there is path from a to d. 
        AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
        // Each word descends from the root and from itself.
        int evidence_value = -1;
        if (a == 0 || a == d) {
          if (add_evidence) {
            evidence_value = 1;
          } else {
            variable->SetLogPotential(log_potential_infinity);
          }
        } else if (prune_paths && add_evidence && !graph_paths[a][d]) {
          evidence_value = 0;
        }
        variables.push_back(variable);
        // For now, consider this as an auxiliary variable. Later there can be
        // a path part for it, in which case the index below will be updated.
        part_indices_.push_back(-1);
        evidence.push_back(evidence_value);
      }
    }

    // Create a "path builder" factor. The following constraints will be 
    // imposed:
    // sum_{i=0}^{n} f_{ijk} = p_{jk} for each j,k with j \ne 0.
    // The case j=k is already addressed in the "single parent" factors.
    //int offset_path_builder_factors = factors.size();
    vector<vector<vector<AD3::BinaryVariable*> > > local_variables_path_builder(
        sentence->size(), vector<vector<AD3::BinaryVariable*> >(sentence->size()));
    for (int r = 0; r < num_arcs; ++r) {
      DependencyPartArc *arc =
          static_cast<DependencyPartArc*>((*dependency_parts)[offset_arcs + r]);
      int m = arc->modifier();
      for (int k = 0; k < sentence->size(); ++k) {
        int index = offset_flow_variables + k + r*sentence->size();
        local_variables_path_builder[m][k].push_back(variables[index]);
      }
    }
    for (int d = 0; d < sentence->size(); ++d) {
      for (int a = 1; a < sentence->size(); ++a) {
        int index = offset_path_variables + a + d*sentence->size();
        local_variables_path_builder[a][d].push_back(variables[index]);
        factor_graph->CreateFactorXOROUT(local_variables_path_builder[a][d]);
        factor_part_indices_.push_back(-1);
      }
    }

    // Create the "flow delta" factor. The following constraints will be 
    // imposed:
    // sum_{j=0}^{n} f_{ijk} = p_{ik} for each i,k s.t. i \ne k.
    // TODO: remove the comment below, because that's not really done?
    // Remark: the f_{ijj} variables are replaced by the arc variables z_{ij}.
    //int offset_flow_delta_factors = factors.size();
    vector<vector<vector<AD3::BinaryVariable*> > > local_variables_flow_delta(
        sentence->size(), vector<vector<AD3::BinaryVariable*> >(sentence->size()));
    for (int r = 0; r < num_arcs; ++r) {
      DependencyPartArc *arc =
          static_cast<DependencyPartArc*>((*dependency_parts)[offset_arcs + r]);
      int h = arc->head();
      for (int k = 0; k < sentence->size(); ++k) {
        if (h == k) continue;
        int index = offset_flow_variables + k + r*sentence->size();
        local_variables_flow_delta[h][k].push_back(variables[index]);
      }
    }
    for (int d = 0; d < sentence->size(); ++d) {
      for (int a = 1; a < sentence->size(); ++a) {
        if (a == d) continue;
        int index = offset_path_variables + a + d*sentence->size();
        if (local_variables_flow_delta[a][d].size() == 0) {
          // Useless to create a XOR-OUT factor with no inputs and one output.
          // That is equivalent to impose that the output is zero.
          if (add_evidence) {
            evidence[index] = 0;
          } else {
            variables[index]->SetLogPotential(log_potential_zero);
            // TODO: keep track of the original log-potential.
          }
          continue; 
        }
        local_variables_flow_delta[a][d].push_back(variables[index]);
        factor_graph->CreateFactorXOROUT(local_variables_flow_delta[a][d]);
        factor_part_indices_.push_back(-1);
      }
    }

    // Create the "flow implies arc" factor. The following constraints will be
    // imposed:
    // f_{ijk} <= a_{ij}; equivalently f_{ijk} "implies" a_{ij} for each i,j,k.
    //offset_flow_support_factors = factors.size();
    for (int r = 0; r < num_arcs; ++r) {
      for (int k = 0; k < sentence->size(); ++k) {
        vector<AD3::BinaryVariable*> local_variables;

        // The LHS of the implication relation.
        int index = offset_flow_variables + k + r*sentence->size();
        local_variables.push_back(variables[index]);

        // The RHS of the implication relation.
        local_variables.push_back(variables[r]);
        factor_graph->CreateFactorIMPLY(local_variables);
        factor_part_indices_.push_back(-1);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Build sibling factors.
  //////////////////////////////////////////////////////////////////////
  if (use_arbitrary_sibling_parts) {
    for (int r = 0; r < num_siblings; ++r) {
      DependencyPartSibl *part = static_cast<DependencyPartSibl*>(
          (*dependency_parts)[offset_siblings + r]);
      int r1 = dependency_parts->FindArc(part->head(), part->modifier());
      int r2 = dependency_parts->FindArc(part->head(), part->sibling());
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_arcs]);
      local_variables.push_back(variables[r2 - offset_arcs]);
      factor_graph->CreateFactorPAIR(local_variables,
                                     scores[offset_siblings + r]);
      // TODO: set these global indices at the end after all variables/factors
      // are created.
      //factor->SetGlobalIndex(...);
      additional_part_indices.push_back(offset_siblings + r);
      factor_part_indices_.push_back(offset_siblings + r);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Build grandparent factors.
  //////////////////////////////////////////////////////////////////////
  if (!use_grandparent_head_automata || !use_next_sibling_parts) {
    for (int r = 0; r < num_grandparents; ++r) {
      DependencyPartGrandpar *part = static_cast<DependencyPartGrandpar*>(
        (*dependency_parts)[offset_grandparents + r]);
      int r1 = dependency_parts->FindArc(part->grandparent(), part->head());
      int r2 = dependency_parts->FindArc(part->head(), part->modifier());
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_arcs]);
      local_variables.push_back(variables[r2 - offset_arcs]);
      factor_graph->CreateFactorPAIR(local_variables,
                                     scores[offset_grandparents + r]);
      // TODO: set these global indices at the end after all variables/factors are created.
      //factor->SetGlobalIndex(...);
      additional_part_indices.push_back(offset_grandparents + r);
      factor_part_indices_.push_back(offset_grandparents + r);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Build next sibling factors.
  //////////////////////////////////////////////////////////////////////
  if (use_head_automata || use_grandparent_head_automata) {
    // Get all the grandparents, indices, etc.
    vector<vector<DependencyPartGrandpar*> > left_grandparents(sentence->size());
    vector<vector<DependencyPartGrandpar*> > right_grandparents(sentence->size());
    vector<vector<double> > left_grandparent_scores(sentence->size());
    vector<vector<double> > right_grandparent_scores(sentence->size());
    vector<vector<int> > left_grandparent_indices(sentence->size());
    vector<vector<int> > right_grandparent_indices(sentence->size());
    if (use_grandparent_head_automata && 
        use_grandparent_parts &&
        use_next_sibling_parts) {
      for (int r = 0; r < num_grandparents; ++r) {
        DependencyPartGrandpar *part = static_cast<DependencyPartGrandpar*>(
            (*dependency_parts)[offset_grandparents + r]);
        if (part->head() > part->modifier()) {
          // Left sibling.
          left_grandparents[part->head()].push_back(part);
          left_grandparent_scores[part->head()].push_back(
              scores[offset_grandparents + r]);
          // Save the part index to get the posterior later.
          left_grandparent_indices[part->head()].push_back(
              offset_grandparents + r);
        } else {
          // Right sibling.
          right_grandparents[part->head()].push_back(part);
          right_grandparent_scores[part->head()].push_back(
              scores[offset_grandparents + r]);
          // Save the part index to get the posterior later.
          right_grandparent_indices[part->head()].push_back(
              offset_grandparents + r);
        }
      }
    }

    // Get all the next siblings, indices, etc.
    vector<vector<DependencyPartNextSibl*> > left_siblings(sentence->size());
    vector<vector<DependencyPartNextSibl*> > right_siblings(sentence->size());
    vector<vector<double> > left_scores(sentence->size());
    vector<vector<double> > right_scores(sentence->size());
    vector<vector<int> > left_indices(sentence->size());
    vector<vector<int> > right_indices(sentence->size());
    for (int r = 0; r < num_next_siblings; ++r) {
      DependencyPartNextSibl *sibling = 
          static_cast<DependencyPartNextSibl*>(
              (*parts)[offset_next_siblings + r]);
      if (sibling->head() > sibling->next_sibling()) {
        // Left sibling.
        left_siblings[sibling->head()].push_back(sibling);
        left_scores[sibling->head()].push_back(
            scores[offset_next_siblings + r]);
        // Save the part index to get the posterior later.
        left_indices[sibling->head()].push_back(offset_next_siblings + r);
      } else { 
        // Right sibling. 
        right_siblings[sibling->head()].push_back(sibling);
        right_scores[sibling->head()].push_back(
            scores[offset_next_siblings + r]);
        // Save the part index to get the posterior later.
        right_indices[sibling->head()].push_back(offset_next_siblings + r);
      }
    }

    // Now, go through each head and create left and right automata.
    for (int h = 0; h < sentence->size(); ++h) {
      // Get the incoming arcs, in case we are using grandparents.
      vector<AD3::BinaryVariable*> local_variables_grandparents;
      vector<DependencyPartArc*> incoming_arcs;
      if (use_grandparent_head_automata &&
          use_grandparent_parts &&
          use_next_sibling_parts) {
        for (int g = 0; g < sentence->size(); ++g) {
          int r = dependency_parts->FindArc(g, h);
          if (r < 0) continue;
          int index = r - offset_arcs;
          local_variables_grandparents.push_back(variables[index]);
          DependencyPartArc *arc = 
            static_cast<DependencyPartArc*>((*parts)[offset_arcs + r]);
          incoming_arcs.push_back(arc);
        } 
      }

      // Build left head automaton.
      vector<AD3::BinaryVariable*> local_variables = local_variables_grandparents;
      vector<DependencyPartArc*> arcs;
      for (int m = h-1; m >= 1; --m) {
        int r = dependency_parts->FindArc(h, m);
        if (r < 0) continue;
        int index = r - offset_arcs;
        local_variables.push_back(variables[index]);
        DependencyPartArc *arc = 
            static_cast<DependencyPartArc*>((*parts)[offset_arcs + r]);
        arcs.push_back(arc);
      } 
      //if (arcs.size() == 0) continue; // Do not create an empty factor.

      if (use_grandparent_head_automata &&
          use_grandparent_parts &&
          use_next_sibling_parts && 
          incoming_arcs.size() > 0) {
        AD3::FactorGrandparentHeadAutomaton *factor =
          new AD3::FactorGrandparentHeadAutomaton;
        factor->Initialize(incoming_arcs, arcs, 
                           left_grandparents[h], 
                           left_siblings[h]);
        vector<double> additional_log_potentials = left_grandparent_scores[h];
        additional_log_potentials.insert(additional_log_potentials.end(),
                                         left_scores[h].begin(),
                                         left_scores[h].end());
        factor->SetAdditionalLogPotentials(additional_log_potentials);
        factor_graph->DeclareFactor(factor, local_variables, true);
        factor_part_indices_.push_back(-1);
        additional_part_indices.insert(additional_part_indices.end(),
                                       left_grandparent_indices[h].begin(),
                                       left_grandparent_indices[h].end());
        additional_part_indices.insert(additional_part_indices.end(),
                                       left_indices[h].begin(),
                                       left_indices[h].end());
      } else {
        AD3::FactorHeadAutomaton *factor = new AD3::FactorHeadAutomaton;
        factor->Initialize(arcs, left_siblings[h]);
        factor->SetAdditionalLogPotentials(left_scores[h]);
        factor_graph->DeclareFactor(factor, local_variables, true);
        factor_part_indices_.push_back(-1);
        additional_part_indices.insert(additional_part_indices.end(),
                                       left_indices[h].begin(),
                                       left_indices[h].end());
      }
      // Build right head automaton.
      local_variables.clear();
      local_variables = local_variables_grandparents;
      arcs.clear();
      for (int m = h+1; m < sentence->size(); ++m) {
        int r = dependency_parts->FindArc(h, m);
        if (r < 0) continue;
        int index = r - offset_arcs;
        local_variables.push_back(variables[index]);
        DependencyPartArc *arc = 
            static_cast<DependencyPartArc*>((*parts)[offset_arcs + r]);
        arcs.push_back(arc);
      } 
      //if (arcs.size() == 0) continue; // Do not create an empty factor.

      if (use_grandparent_head_automata &&
          use_grandparent_parts &&
          use_next_sibling_parts &&
          incoming_arcs.size() > 0) {
        AD3::FactorGrandparentHeadAutomaton *factor =
          new AD3::FactorGrandparentHeadAutomaton;
        factor->Initialize(incoming_arcs, arcs, 
                           right_grandparents[h], 
                           right_siblings[h]);
        vector<double> additional_log_potentials = right_grandparent_scores[h];
        additional_log_potentials.insert(additional_log_potentials.end(),
                                         right_scores[h].begin(),
                                         right_scores[h].end());
        factor->SetAdditionalLogPotentials(additional_log_potentials);
        factor_graph->DeclareFactor(factor, local_variables, true);
        factor_part_indices_.push_back(-1);
        additional_part_indices.insert(additional_part_indices.end(),
                                       right_grandparent_indices[h].begin(),
                                       right_grandparent_indices[h].end());
        additional_part_indices.insert(additional_part_indices.end(),
                                       right_indices[h].begin(),
                                       right_indices[h].end());
      } else {
        AD3::FactorHeadAutomaton *factor = new AD3::FactorHeadAutomaton;
        factor->Initialize(arcs, right_siblings[h]);
        factor->SetAdditionalLogPotentials(right_scores[h]);
        factor_graph->DeclareFactor(factor, local_variables, true);
        factor_part_indices_.push_back(-1);
        additional_part_indices.insert(additional_part_indices.end(),
                                       right_indices[h].begin(),
                                       right_indices[h].end());
      }
    }
  } else {
    // Create OMEGA variables.
    int offset_omega_variables = variables.size();
    for (int r = 0; r < num_arcs; ++r) {
      for (int s = 0; s < sentence->size(); ++s) {
        // Create omega variable denoting that... 
        AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
        variables.push_back(variable);
        part_indices_.push_back(-1);
        evidence.push_back(-1);
      }
    }

    // Create RHO variables.
    int offset_rho_variables = variables.size();
    for (int m = 0; m < sentence->size(); ++m) {
      for (int s = 0; s < sentence->size(); ++s) {
        // Create rho variable denoting that... 
        AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
        variables.push_back(variable);
        part_indices_.push_back(-1);
        evidence.push_back(-1);
      }
    }

    // Create NEXT SIBL variables.
    int offset_next_sibling_variables = variables.size();
    for (int r = 0; r < num_next_siblings; ++r) {
      AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
      variable->SetLogPotential(scores[offset_next_siblings + r]);
      variables.push_back(variable);
      part_indices_.push_back(offset_next_siblings + r);
      evidence.push_back(-1);
    }

    // Create the "omega normalizer" factor. The following constraints will be
    // imposed:
    // sum_{k >= ch >= par} omega_{par,ch,k} = 1, for all par, k.
    // REMARK: omega_{par,ch,<q>} = z_{par,ch}.
    //Hence in those cases must substitute omega by z accordingly

    // Create arrays of the variables that partipate in each factor.
    // Each array is in local_variables_rho[par][k].
    vector<vector<vector<AD3::BinaryVariable*> > >
        local_variables_omega_normalizer(sentence->size(),
            vector<vector<AD3::BinaryVariable*> >(sentence->size()));

    // Add the rho variables to the arrays.
    for (int h = 0; h < sentence->size(); ++h) {
      for (int k = 0; k < sentence->size(); ++k) {
        int index = offset_rho_variables + h*sentence->size() + k;
        local_variables_omega_normalizer[h][k].push_back(variables[index]);
      }
    }

    // Add the omega variables to the arrays.
    for (int r = 0; r < num_arcs; ++r) {
      DependencyPartArc *arc =
          static_cast<DependencyPartArc*>((*dependency_parts)[r]);
      int h = arc->head();
      int m = arc->modifier();

      if (h < m) {
        for (int k = m; k < sentence->size(); ++k) {
          int index;
          if (k == m) {
            index = r;
          } else {
            index = offset_omega_variables + r*sentence->size() + k;
          }
          local_variables_omega_normalizer[h][k].push_back(variables[index]);
        }
      } else {
        for (int k = m; k >= 0; k--) {
          int index;
          if (k == m) {
            index = r;
          } else {
            index = offset_omega_variables + r*sentence->size() + k;
          }
          local_variables_omega_normalizer[h][k].push_back(variables[index]);
        }
      }
    }

    // Create the factors.
    for (int h = 0; h < sentence->size(); ++h) {
      for (int k = 0; k < sentence->size(); ++k) {
        factor_graph->CreateFactorXOR(local_variables_omega_normalizer[h][k]);
        factor_part_indices_.push_back(-1);
      }
    }

    // Create the "omega propagate" and "rho propagate" factors. The following
    // constraints will be imposed:
    // omega_{par,ch1,ch2} + z_{par,ch1,ch2} = omega_{par,ch1,ch2-1}.

    // Create arrays of the variables that partipate in each factor.
    // Each array is in local_variables[r][k].
    vector<vector<vector<AD3::BinaryVariable*> > > local_variables_omega_propagate(
        num_arcs, vector<vector<AD3::BinaryVariable*> >(sentence->size()));
    vector<vector<vector<bool> > > negated_omega_propagate(
        num_arcs, vector<vector<bool> >(sentence->size()));

    // Do this first for par != ch1 (omega propagate factors):
    for (int r = 0; r < num_arcs; ++r) {
      DependencyPartArc *arc =
          static_cast<DependencyPartArc*>((*dependency_parts)[r + offset_arcs]);
      int h = arc->head();
      int m = arc->modifier();

      if (h < m) {
        for (int k = m + 1; k < sentence->size(); ++k) {
          int index;
          // Omega.
          if (k == m) {
            index = r;
          } else {
            index = offset_omega_variables + r*sentence->size() + k;
          }
          local_variables_omega_propagate[r][k].push_back(variables[index]);
          negated_omega_propagate[r][k].push_back(false); // negated = false.

          // Previous omega.
          if (k - 1 == m) {
            index = r;
          } else {
            index = offset_omega_variables + r*sentence->size() + k - 1;
          }
          local_variables_omega_propagate[r][k].push_back(variables[index]);
          negated_omega_propagate[r][k].push_back(true); // negated = true.
        }
      } else {
        for (int k = m - 1; k >= 0; k--) {
          int index;
          // Omega.
          if (k == m) {
            index = r;
          } else {
            index = offset_omega_variables + r*sentence->size() + k;
          }
          local_variables_omega_propagate[r][k].push_back(variables[index]);
          negated_omega_propagate[r][k].push_back(false); // negated = false.

          // Previous omega.
          if (k + 1 == m) {
            index = r;
          } else {
            index = offset_omega_variables + r*sentence->size() + k + 1;
          }
          local_variables_omega_propagate[r][k].push_back(variables[index]);
          negated_omega_propagate[r][k].push_back(true); // negated = true.
        }
      }
    }

    // Create arrays of the variables that partipate in each factor.
    // Each array is in local_variables[par][k].
    vector<vector<vector<AD3::BinaryVariable*> > > local_variables_rho_propagate(
        sentence->size(), vector<vector<AD3::BinaryVariable*> >(sentence->size()));
    vector<vector<vector<bool> > > negated_rho_propagate(
        sentence->size(), vector<vector<bool> >(sentence->size()));

    // Now for par == ch1 (rho propagate factors):
    for (int h = 0; h < sentence->size(); ++h) {
      for (int k = 0; k < sentence->size(); ++k) {
        if (h == k) continue;
        if (h < k) {
          int index;
          // Rho.
          index = offset_rho_variables + h*sentence->size() + k;
          local_variables_rho_propagate[h][k].push_back(variables[index]);
          negated_rho_propagate[h][k].push_back(false); // negated = false.

          // Previous rho.
          index = offset_rho_variables + h*sentence->size() + k - 1;
          local_variables_rho_propagate[h][k].push_back(variables[index]);
          negated_rho_propagate[h][k].push_back(true); // negated = true.
        } else {
          int index;
          // Rho.
          index = offset_rho_variables + h*sentence->size() + k;
          local_variables_rho_propagate[h][k].push_back(variables[index]);
          negated_rho_propagate[h][k].push_back(false); // negated = false.

          // Previous rho.
          index = offset_rho_variables + h*sentence->size() + k + 1;
          local_variables_rho_propagate[h][k].push_back(variables[index]);
          negated_rho_propagate[h][k].push_back(true); // negated = true.
        }
      }
    }

    // Now add the prev sibl variables to these factors.
    for (int r = 0; r < num_next_siblings; ++r) {
      DependencyPartNextSibl *part = static_cast<DependencyPartNextSibl*>(
          (*dependency_parts)[offset_next_siblings + r]);
      int h = part->head();
      int m = part->modifier();
      int s = part->next_sibling();

      // Last child (left or right).
      if (s == sentence->size() || s == -1) continue;

      if (h == m) {
        // First child variable.
        int index = offset_next_sibling_variables + r;
        local_variables_rho_propagate[h][s].push_back(variables[index]);
        negated_rho_propagate[h][s].push_back(false); // negated = false.
      } else {
        // Next sibling variable.
        int r1 = dependency_parts->FindArc(h, m);
        int index = offset_next_sibling_variables + r;
        local_variables_omega_propagate[r1 - offset_arcs][s].push_back(
            variables[index]);
        // negated = false.
        negated_omega_propagate[r1 - offset_arcs][s].push_back(false);
      }
    }

    // Create the actual factors.
    for (int r = 0; r < num_arcs; ++r) {
      for (int k = 0; k < sentence->size(); ++k) {
        if (local_variables_omega_propagate[r][k].size() == 0) continue;
        CHECK_GE(local_variables_omega_propagate[r][k].size(), 2);
        CHECK_LE(local_variables_omega_propagate[r][k].size(), 3);
        factor_graph->CreateFactorXOR(local_variables_omega_propagate[r][k],
                                      negated_omega_propagate[r][k]);
        factor_part_indices_.push_back(-1);
      }
    }

    for (int h = 0; h < sentence->size(); ++h) {
      for (int k = 0; k < sentence->size(); ++k) {
        if (local_variables_rho_propagate[h][k].size() == 0) continue;
        CHECK_GE(local_variables_rho_propagate[h][k].size(), 2);
        CHECK_LE(local_variables_rho_propagate[h][k].size(), 3);
        factor_graph->CreateFactorXOR(local_variables_rho_propagate[h][k],
                                      negated_rho_propagate[h][k]);
        factor_part_indices_.push_back(-1);
      }
    }

    // Create the "next sibling consistency" factors. The following constraints
    // will be imposed:
    // sum_{ch1} z_{par,ch1,ch2} = z_{par,ch2}.

    // Create arrays of the variables that participate in each factor.
    // Each array is in local_variables[r].
    vector<vector<AD3::BinaryVariable*> > local_variables_consistency(
        num_arcs);

    for (int r = 0; r < num_next_siblings; ++r) {
      DependencyPartNextSibl *part = static_cast<DependencyPartNextSibl*>(
          (*dependency_parts)[offset_next_siblings + r]);
      int h = part->head();
      int s = part->next_sibling();

      // Last child (left or right).
      if (s == sentence->size() || s == -1) continue;

      // Next sibling variable (negated = false).
      int r2 = dependency_parts->FindArc(h, s);
      int index = offset_next_sibling_variables + r;
      local_variables_consistency[r2 - offset_arcs].push_back(variables[index]);
    }

    for (int r = 0; r < num_arcs; ++r) {
      // Arc variable (negated = true).
      local_variables_consistency[r].push_back(variables[r]);
    }

    // Create the actual factors.
    for (int r = 0; r < num_arcs; ++r) {
      if (local_variables_consistency[r].size() == 0) continue;
      factor_graph->CreateFactorXOROUT(local_variables_consistency[r]);
      factor_part_indices_.push_back(-1);
    }

    // Take care of the leftmost/rightmost children.
    // Create "equality" factors for handling special cases, making sure that
    // certain variables are equivalent.
    for (int r = 0; r < num_next_siblings; ++r) {
      DependencyPartNextSibl *part = static_cast<DependencyPartNextSibl*>(
          (*dependency_parts)[offset_next_siblings + r]);
      int h = part->head();
      int m = part->modifier();
      int s = part->next_sibling();

      // Do something for LAST CHILD:
      // * omega[r*length + (length-1)] if r points to the right
      // * omega[r*length + 0]  if r points to the left
      // * rho[par*length + (length-1)] if r points to the right
      // * rho[par*length + 0]  if r points to the left.
      if (s == sentence->size()) {
        // Rightmost child.
        if (h == m) {
          // No child on right side.
          // Impose the constraint:
          // z[r] == rho[par*length + (length-1)];
          vector<AD3::BinaryVariable*> local_variables(2);

          int index = offset_next_sibling_variables + r;
          local_variables[0] = variables[index];

          index = offset_rho_variables + h * sentence->size() +
              (sentence->size() - 1);
          local_variables[1] = variables[index];

          // Create the factor.
          factor_graph->CreateFactorXOROUT(local_variables);
          factor_part_indices_.push_back(-1);
        } else {
          // Impose the constraint:
          // z[r] == omega[r1*length +  (length-1)];
          int r1 = dependency_parts->FindArc(h, m);
          CHECK_GE(r1, 0);

          vector<AD3::BinaryVariable*> local_variables(2);

          int index = offset_next_sibling_variables + r;
          local_variables[0] = variables[index];

          if (sentence->size() - 1 == m) {
            index = r1 - offset_arcs;
          } else {
            index = offset_omega_variables +
                (r1 - offset_arcs) * sentence->size() + (sentence->size() - 1);
          }
          local_variables[1] = variables[index];

          // Create the factor.
          factor_graph->CreateFactorXOROUT(local_variables);
          factor_part_indices_.push_back(-1);
        }
      } else if (s == -1) {
        // Leftmost child.
        if (h == m) {
          // No child on left side.
          // Impose the constraint:
          // z[r] == rho[par*length + 0];
          vector<AD3::BinaryVariable*> local_variables(2);

          int index = offset_next_sibling_variables + r;
          local_variables[0] = variables[index];

          index = offset_rho_variables + h * sentence->size() + 0;
          local_variables[1] = variables[index];

          // Create the factor.
          factor_graph->CreateFactorXOROUT(local_variables);
          factor_part_indices_.push_back(-1);
        } else {
          // Impose the constraint:
          // z[r] == omega[r1*length +  0];
          int r1 = dependency_parts->FindArc(h, m);
          CHECK_GE(r1, 0);

          vector<AD3::BinaryVariable*> local_variables(2);

          int index = offset_next_sibling_variables + r;
          local_variables[0] = variables[index];

          if (0 == m) {
            CHECK(false);
            index = r1 - offset_arcs;
          } else {
            index = offset_omega_variables +
                (r1 - offset_arcs) * sentence->size() + 0;
          }
          local_variables[1] = variables[index];

          // Create the factor.
          factor_graph->CreateFactorXOROUT(local_variables);
          factor_part_indices_.push_back(-1);
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Handle the non-projective parts.
  //////////////////////////////////////////////////////////////////////

  // Create NONPROJARCEXTRA variables.
  // These indicate that a span is nonprojective, being or not being a arc
  // there.
  int offset_nonproj_extra_variables = variables.size();
  for (int r = 0; r < num_nonprojective; ++r) {
    // NONPROJARCEXTRA variable.
    AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
    variables.push_back(variable);
    part_indices_.push_back(-1);

    DependencyPartNonproj *part = static_cast<DependencyPartNonproj*>(
        (*dependency_parts)[offset_nonprojective + r]);
    int evidence_value = -1;
    if (part->head() == 0) {
      // NONPROJARCEXTRA is necessarily 0.
      if (add_evidence) {
        evidence_value = 0;
      } else {
        variable->SetLogPotential(log_potential_zero);
      }
    }
    evidence.push_back(evidence_value);
  }

  // Create NONPROJARC variables.
  // These are the conjunction of the ARC variables with the NONPROJARCEXTRA
  // variables.
  int offset_nonproj_variables = variables.size();
  for (int r = 0; r < num_nonprojective; ++r) {
    // NONPROJARC variable.
    AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
    variable->SetLogPotential(scores[offset_nonprojective + r]);
    variables.push_back(variable);
    part_indices_.push_back(offset_nonprojective + r);
    evidence.push_back(-1);
  }

  // Create NONPROJARCEXTRA factors.
  for (int r = 0; r < num_nonprojective; ++r) {
    DependencyPartNonproj *part = static_cast<DependencyPartNonproj*>(
        (*dependency_parts)[offset_nonprojective + r]);

    // No factor necessary in this case, as NONPROJARCEXTRA is necessarily 0.
    if (part->head() == 0) continue;

    vector<AD3::BinaryVariable*> local_variables;
    vector<bool> negated;

    int left, right;
    if (part->head() <= part->modifier()) {
      left = part->head();
      right = part->modifier();
    } else {
      left = part->modifier();
      right = part->head();
    }

    CHECK_GE(offset_path_variables, 0);
    for (int k = left; k <= right; ++k) {
      // Add negated path variable.
      int index = offset_path_variables + part->head() + k * sentence->size();
      local_variables.push_back(variables[index]);
      negated.push_back(true);
    }

    // Add NONPROJARCEXTRA variable.
    int index = offset_nonproj_extra_variables + r;
    local_variables.push_back(variables[index]);
    negated.push_back(false);

    // Create the NONPROJARCEXTRA factor.
    factor_graph->CreateFactorOROUT(local_variables, negated);
    factor_part_indices_.push_back(-1);
  }

  // Create NONPROJARC factors.
  for (int r = 0; r < num_nonprojective; ++r) {
    DependencyPartNonproj *part = static_cast<DependencyPartNonproj*>(
        (*dependency_parts)[offset_nonprojective + r]);

    vector<AD3::BinaryVariable*> local_variables;
    int r1 = dependency_parts->FindArc(part->head(), part->modifier());

    // Add the arc variable (negated).
    int index = r1 - offset_arcs;
    local_variables.push_back(variables[index]);

    // Add the NONPROJARCEXTRA variable (negated).
    index = offset_nonproj_extra_variables + r;
    local_variables.push_back(variables[index]);

    // Add the NONPROJARC variable (negated).
    index = offset_nonproj_variables + r;
    local_variables.push_back(variables[index]);

    // Create the NONPROJARCEXTRA factor.
    factor_graph->CreateFactorANDOUT(local_variables);
    factor_part_indices_.push_back(-1);
  }

  //////////////////////////////////////////////////////////////////////
  // Handle the directed path parts.
  //////////////////////////////////////////////////////////////////////

  if (num_path > 0) CHECK_GE(offset_path_variables, 0);
  for (int r = 0; r < num_path; ++r) {
    DependencyPartPath *part = static_cast<DependencyPartPath*>(
        (*dependency_parts)[offset_path + r]);
    int a = part->ancestor();
    int d = part->descendant();
    int index = offset_path_variables + a + d*sentence->size();
    AD3::BinaryVariable *variable = variables[index];
    part_indices_[index] = offset_path + r;
    // TODO: solve this problem.
    // varPath->m_logPotentialOrig = scores[offset + r];
    // Only update the log potential if it was not set to zero or infinity
    // earlier.
    if (variable->GetLogPotential() == 0.0) {
      variable->SetLogPotential(scores[offset_path + r]);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Handle the head bigram parts.
  //////////////////////////////////////////////////////////////////////

  if (use_head_bigram_sequence_factor && use_head_bigram_parts) {
    // Populate local variables and compute the number of states for each 
    // position in the sequence (i.e. each word).
    vector<AD3::BinaryVariable*> local_variables;
    vector<int> num_states(sentence->size() - 1, 0);
    vector<vector<int> > index_heads(sentence->size() - 1,
      vector<int>(sentence->size(), -1));
    for (int m = 1; m < sentence->size(); ++m) {
      for (int h = 0; h < sentence->size(); ++h) {
        int r = dependency_parts->FindArc(h, m);
        if (r < 0) continue;
        local_variables.push_back(variables[r - offset_arcs]);
        index_heads[m - 1][h] = num_states[m - 1];
        ++num_states[m - 1];
      }
    }
    vector<vector<vector<int> > > index_edges(sentence->size());
    int index = 0;
    for (int i = 0; i < sentence->size(); ++i) {
      // If i == 0, the previous state is the start symbol.
      int num_previous_states = (i > 0)? num_states[i - 1] : 1;
      // One state to account for the final symbol.
      int num_current_states = (i < sentence->size() - 1)? num_states[i] : 1;
      index_edges[i].resize(num_previous_states);
      for (int j = 0; j < num_previous_states; ++j) {
        index_edges[i][j].resize(num_current_states);
        for (int k = 0; k < num_current_states; ++k) {
          index_edges[i][j][k] = index;
          ++index;
        }
      }
    }
    vector<double> additional_log_potentials(index, 0.0);
    vector<int> head_bigram_indices(index, -1);
    for (int r = 0; r < num_bigrams; ++r) {
      DependencyPartHeadBigram *part = static_cast<DependencyPartHeadBigram*>
        ((*dependency_parts)[offset_bigrams + r]);
      int m = part->modifier();
      CHECK_GE(m, 1);
      CHECK_LT(m, sentence->size() + 1);
      int previous_state = (m == 1)?
          0 : index_heads[m - 2][part->previous_head()];
      int current_state = (m == sentence->size())? 0 : index_heads[m - 1][part->head()];
      CHECK_GE(previous_state, 0);
      if (m > 1) CHECK_LT(previous_state, num_states[m - 2]);
      CHECK_GE(current_state, 0);
      if (m < sentence->size()) CHECK_LT(current_state, num_states[m - 1]);
      int index = index_edges[m - 1][previous_state][current_state];
      CHECK_GE(index, 0);
      CHECK_LT(index, additional_log_potentials.size());
      additional_log_potentials[index] = scores[offset_bigrams + r];
      head_bigram_indices[index] = offset_bigrams + r;
    }
    AD3::FactorSequence *factor = new AD3::FactorSequence;
    factor->Initialize(num_states);
    factor->SetAdditionalLogPotentials(additional_log_potentials);
    factor_graph->DeclareFactor(factor, local_variables, true);
    factor_part_indices_.push_back(-1);
    additional_part_indices.insert(additional_part_indices.end(),
                                   head_bigram_indices.begin(),
                                   head_bigram_indices.end());
  } else {
    for (int r = 0; r < num_bigrams; ++r) {
      DependencyPartHeadBigram *part = static_cast<DependencyPartHeadBigram*>
        ((*dependency_parts)[offset_bigrams + r]);
      int r1 = dependency_parts->FindArc(part->head(), part->modifier());
      int r2 = dependency_parts->FindArc(part->previous_head(),
                                         part->modifier() - 1);
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_arcs]);
      local_variables.push_back(variables[r2 - offset_arcs]);
      factor_graph->CreateFactorPAIR(local_variables,
                                     scores[offset_bigrams + r]);
      // TODO: set these global indices at the end after all variables/factors are created.
      //factor->SetGlobalIndex(...);
      additional_part_indices.push_back(offset_bigrams + r);
      factor_part_indices_.push_back(offset_bigrams + r);
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  CHECK_EQ(variables.size(), part_indices_.size());
  CHECK_EQ(factor_graph->GetNumFactors(), factor_part_indices_.size());

  // Compute additional_part_indices_.
  int offset = factor_graph->GetNumVariables();
  for (int i = 0; i < factor_graph->GetNumFactors(); ++i) {
    offset += factor_graph->GetFactor(i)->GetAdditionalLogPotentials().size();
  }
  CHECK_EQ(additional_part_indices.size(),
           offset - factor_graph->GetNumVariables());
  // Concatenate part_indices and additional_part_indices.
  part_indices_.insert(part_indices_.end(),
                       additional_part_indices.begin(),
                       additional_part_indices.end());
  evidence.resize(part_indices_.size(), -1);

  VLOG(2) << "Number of factors: " << factor_graph->GetNumFactors();
  VLOG(2) << "Number of variables: " << factor_graph->GetNumVariables();

  vector<int> recomputed_indices(part_indices_.size(), -1);
  bool solved = false;
  if (add_evidence) {
    VLOG(2) << "Adding evidence...";
    timeval start, end;
    gettimeofday(&start, NULL);
    int status = factor_graph->AddEvidence(&evidence, &recomputed_indices);
    gettimeofday(&end, NULL);
    double elapsed_time = diff_ms(end,start);
    VLOG(2) << "Graph simplification took " << elapsed_time << "ms.";
    CHECK_NE(status, AD3::STATUS_INFEASIBLE);
    if (status == AD3::STATUS_OPTIMAL_INTEGER) solved = true;
    VLOG(2) << "Number of factors: " << factor_graph->GetNumFactors();
    VLOG(2) << "Number of variables: " << factor_graph->GetNumVariables();
  }

#ifdef PRINT_GRAPH
  ofstream stream;
  stream.open("tmp.fg", ofstream::out);
  CHECK(stream.good());
  factor_graph->Print(stream);
  stream.flush();
  stream.clear();
  stream.close();
#endif

  vector<double> posteriors;
  vector<double> additional_posteriors;
  double value_ref;
  double *value = &value_ref;

  factor_graph->SetMaxIterationsAD3(500);
  factor_graph->SetEtaAD3(0.05);
  factor_graph->AdaptEtaAD3(true);
  factor_graph->SetResidualThresholdAD3(1e-3);

  // Run AD3.
  timeval start, end;
  gettimeofday(&start, NULL);
  if (!solved) {
    factor_graph->SolveLPMAPWithAD3(&posteriors, &additional_posteriors, value);
  }
  gettimeofday(&end, NULL);
  double elapsed_time = diff_ms(end,start);
  VLOG(2) << "Elapsed time (AD3) = " << elapsed_time
          << " (" << sentence->size() << ") ";

  delete factor_graph;

  *value = 0.0;
  predicted_output->assign(parts->size(), 0.0);
  for (int i = 0; i < part_indices_.size(); ++i) {
    int r = part_indices_[i];
    if (r < 0) continue;
    if (add_evidence) {
      if (recomputed_indices[i] < 0) {
        CHECK_GE(evidence[i], 0) << i;
        (*predicted_output)[r] = evidence[i];
      } else {
        if (recomputed_indices[i] < posteriors.size()) {
          (*predicted_output)[r] = posteriors[recomputed_indices[i]];
        } else {
          int j = recomputed_indices[i] - posteriors.size();
          (*predicted_output)[r] = additional_posteriors[j];
        }
      }
    } else {
      if (i < posteriors.size()) {
        (*predicted_output)[r] = posteriors[i];
      } else {
        int j = i - posteriors.size();
        (*predicted_output)[r] = additional_posteriors[j];
      }
    }
    *value += (*predicted_output)[r] * scores[r];
  }
  
  VLOG(2) << "Solution value (AD3) = " << *value;
}

#ifdef USE_CPLEX

#include <limits.h>
#define ILOUSESTL
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN

void DependencyDecoder::DecodeCPLEX(Instance *instance, Parts *parts, 
                                    const vector<double> &scores,
                                    bool single_root,
                                    bool relax,
                                    vector<double> *predicted_output) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyInstanceNumeric* sentence =
    static_cast<DependencyInstanceNumeric*>(instance);

  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  int offset_labeled_arcs, num_labeled_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  int offset_siblings, num_siblings;
  dependency_parts->GetOffsetSibl(&offset_siblings, &num_siblings);

  int offset_next_siblings, num_next_siblings;
  dependency_parts->GetOffsetNextSibl(&offset_next_siblings, &num_next_siblings);

  int offset_grandparents, num_grandparents;
  dependency_parts->GetOffsetGrandpar(&offset_grandparents, &num_grandparents);

  int offset_nonprojective, num_nonprojective;
  dependency_parts->GetOffsetNonproj(&offset_nonprojective, &num_nonprojective);

  int offset_path, num_path;
  dependency_parts->GetOffsetPath(&offset_path, &num_path);

  int offset_bigrams, num_bigrams;
  dependency_parts->GetOffsetHeadBigr(&offset_bigrams, &num_bigrams);

  bool use_arbitrary_sibling_parts = (num_siblings > 0);
  bool use_next_sibling_parts = (num_next_siblings > 0);
  bool use_grandparent_parts = (num_grandparents > 0);
  bool use_nonprojective_arc_parts = (num_nonprojective > 0);
  bool use_path_parts = (num_path > 0);
  bool use_head_bigram_parts = (num_bigrams > 0);

  // If true, use multi-commodities; otherwise, use single commodities.
  bool use_multicommodity_flows = true;

  int i, j, r;

  try
  {
    timeval start, end;
    gettimeofday(&start, NULL);

    IloEnv env;
    IloModel mod(env);
    IloCplex cplex(mod);

    ///////////////////////////////////////////////////////////////////
    // Variable definition
    ///////////////////////////////////////////////////////////////////

    IloNumVar::Type varType = relax? ILOFLOAT : ILOBOOL;
    IloNumVarArray z(env, parts->size(), 0.0, 1.0, varType);
    IloNumVarArray flow;

    if (use_multicommodity_flows)
      flow = IloNumVarArray(env, num_arcs * sentence->size(), 0.0, 1.0, ILOFLOAT);
    else
      flow = IloNumVarArray(env, num_arcs, 0.0, sentence->size() - 1, ILOFLOAT);

    ///////////////////////////////////////////////////////////////////
    // Objective
    ///////////////////////////////////////////////////////////////////

    IloExpr exprObj(env);
    for (r = 0; r < parts->size(); r++)
    {
      // Skip labeled arcs.
      if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) continue;
      // Add score to the objective.
      exprObj += -scores[r] * z[r];
    }
    IloObjective obj(env, exprObj, IloObjective::Minimize);
    mod.add(obj);
    exprObj.end();

    ///////////////////////////////////////////////////////////////////
    // Constraints
    ///////////////////////////////////////////////////////////////////

    // The root has no parent
    // sum_i (z_i0) = 0
    IloExpr expr(env);
    for (i = 0; i < sentence->size(); i++)
    {
      r = dependency_parts->FindArc(i, 0);
      if (r < 0)
        continue;
      expr += z[r];
    }
    mod.add(expr == 0.0);
    expr.end();

    // afm 10/31/09
    if (single_root)
    {
      // The root has exactly one child
      // sum_i (z_0i) = 1
      IloExpr expr(env);
      for (i = 0; i < sentence->size(); i++)
      {
        r = dependency_parts->FindArc(0, i);
        if (r < 0)
          continue;
        expr += z[r];
      }
      mod.add(expr == 1.0);
      expr.end();
    }

    for (int j = 1; j < sentence->size(); j++)
    {
      // One parent per word (other than the root)
      // sum_i (z_ij) = 1 for all j
      expr = IloExpr(env);
      for (i = 0; i < sentence->size(); i++)
      {
        r = dependency_parts->FindArc(i, j);
        if (r < 0)
          continue;
        expr += z[r];
      }
      mod.add(expr == 1.0);
      expr.end();
    }

    if (use_multicommodity_flows) // Multi-commodity flows
    {
      int k;

      // Root sends one unit of commodity to each node
      // sum_i (f_i0k) - sum_i (f_0ik) = -1, for each k which is not zero
      for (k = 1; k < sentence->size(); k++)
      {
        expr = IloExpr(env);
        for (int j = 0; j < sentence->size(); j++)
        {
          r = dependency_parts->FindArc(0, j);
          if (r < 0)
            continue;
          expr += flow[r*sentence->size() + k];

          r = dependency_parts->FindArc(j, 0);
          if (r < 0)
            continue;
          expr -= flow[r*sentence->size() + k];
        }
        mod.add(expr == 1.0);
        expr.end();
      }

      // Any node consume its own commodity and no other:
      // sum_i (f_ijk) - sum_i (f_jik) = 1(j==k), for each j,k which are not zero
      for (k = 1; k < sentence->size(); k++)
      {
        for (j = 1; j < sentence->size(); j++)
        {
          expr = IloExpr(env);
          for (i = 0; i < sentence->size(); i++)
          {
            r = dependency_parts->FindArc(i, j);
            if (r < 0)
              continue;
            expr += flow[r*sentence->size() + k];
          }

          for (i = 0; i < sentence->size(); i++)
          {
            r = dependency_parts->FindArc(j, i);
            if (r < 0)
              continue;
            expr -= flow[r*sentence->size() + k];
          }
          if (j == k)
            mod.add(expr == 1.0);
          else
            mod.add(expr == 0.0);
          expr.end();
        }
      }

      // Disabled arcs do not carry any flow
      // f_ijk <= z_ij for each i, j, and k
      for (r = 0; r < num_arcs; r++)
      {
        for (k = 0; k < sentence->size(); k++)
        {
          mod.add(flow[r*sentence->size() + k] <= z[r]);
        }
      }
    }
    else // Single commodity flows
    {
      // Root sends flow n
      // sum_j (f_0j) = n
      expr = IloExpr(env);
      for (j = 0; j < sentence->size(); j++)
      {
        r = dependency_parts->FindArc(0, j);
        if (r < 0)
          continue;
        expr += flow[r];
      }
      mod.add(expr == sentence->size() - 1);
      expr.end();

      // Incoming minus outgoing flow is 1 (except for the root)
      // sum_i (f_ij) - sum_i (f_ji) = 1 for each j
      for (j = 1; j < sentence->size(); j++)
      {
        expr = IloExpr(env);
        for (i = 0; i < sentence->size(); i++)
        {
          r = dependency_parts->FindArc(i, j);
          if (r < 0)
            continue;
          expr += flow[r];
        }

        for (i = 0; i < sentence->size(); i++)
        {
          r = dependency_parts->FindArc(j, i);
          if (r < 0)
            continue;
          expr -= flow[r];
        }
        mod.add(expr == 1.0);
        expr.end();
      }

      // Flow on disabled arcs is zero
      // f_ij <= n*z_ij for each i and j
      for (i = 0; i < sentence->size(); i++)
      {
        for (j = 0; j < sentence->size(); j++)
        {
          r = dependency_parts->FindArc(i, j);
          if (r < 0)
            continue;
          mod.add(flow[r] <= (sentence->size()-1) * z[r]);
        }
      }
    }

    ///////////////////////////////////////////////////////////////
    // Add global constraints (if any)
    ///////////////////////////////////////////////////////////////

    if (use_arbitrary_sibling_parts)
    {
      // z_ijk <= z_ij for each i,j,k
      // z_ijk <= z_ik for each i,j,k
      // z_ijk >= z_ij + z_ik - 1 for each i,j,k

      for (int r = 0; r < num_siblings; ++r)
      {
        DependencyPartSibl *part = static_cast<DependencyPartSibl*>(
            (*dependency_parts)[offset_siblings + r]);
        int h = part->head();
        int m = part->modifier();
        int s = part->sibling();
        int r1 = dependency_parts->FindArc(h, m);
        int r2 = dependency_parts->FindArc(h, s);

        CHECK_GE(r1, 0);
        CHECK_GE(r2, 0);

        mod.add(z[offset_siblings + r] - z[r1] <= 0.0);
        mod.add(z[offset_siblings + r] - z[r2] <= 0.0);
        mod.add(z[offset_siblings + r] - z[r1] - z[r2] >= -1.0);
      }
    }

    if (use_next_sibling_parts)
    {
      IloNumVarArray omega(env, num_arcs * sentence->size(),
          0.0, 1.0, ILOFLOAT);
      IloNumVarArray rho(env, sentence->size() * sentence->size(),
          0.0, 1.0, ILOFLOAT); // This stores omega(par,par,k)

      //////////////////////////////////////////////////
      // omega_{par,ch,ch} = z_{par,ch}
      //////////////////////////////////////////////////

      for (int r = 0; r < num_arcs; r++)
      {
        DependencyPartArc *arc = static_cast<DependencyPartArc*>(
            (*dependency_parts)[offset_arcs + r]);
        int par = arc->head();
        int ch = arc->modifier();

        if (par == ch) // This should never happen
          continue;

        mod.add(omega[r*sentence->size() + ch] - z[r] == 0.0);
      }

      //////////////////////////////////////////////////
      // sum_{k >= ch >= par} omega_{par,ch,k} = 1, for all k
      //////////////////////////////////////////////////

      vector<IloExpr> vExpr(sentence->size() * sentence->size());
      for (int ind = 0; ind < vExpr.size(); ind++)
        vExpr[ind] = IloExpr(env);

      for (int par = 0; par < sentence->size(); par++)
      {
        for (int k = 0; k < sentence->size(); k++)
        {
          vExpr[par*sentence->size() + k] += rho[par*sentence->size() + k];
        }
      }

      for (int r = 0; r < num_arcs; r++)
      {
        DependencyPartArc *arc = static_cast<DependencyPartArc*>(
            (*dependency_parts)[offset_arcs + r]);
        int par = arc->head();
        int ch = arc->modifier();

        if (par == ch) // This should never happen
          continue;

        if (par < ch)
        {
          for (int k = ch; k < sentence->size(); k++)
          {
            vExpr[par*sentence->size() + k] += omega[r*sentence->size() + k];
          }
        }
        else
        {
          for (int k = ch; k >= 0; k--)
          {
            vExpr[par*sentence->size() + k] += omega[r*sentence->size() + k];
          }
        }
      }

      for (int ind = 0; ind < vExpr.size(); ind++)
      {
        mod.add(vExpr[ind] == 1.0);
        vExpr[ind].end();
      }

      //////////////////////////////////////////////////
      // omega_{par,ch1,ch2} + z_{par,ch1,ch2} = omega_{par,ch1,ch2-1}
      //////////////////////////////////////////////////

      // Do this first for par != ch1:
      vExpr.clear();
      vExpr.resize(num_arcs * sentence->size());
      for (int ind = 0; ind < vExpr.size(); ind++)
        vExpr[ind] = IloExpr(env);

      for (int r = 0; r < num_arcs; r++)
      {
        DependencyPartArc *arc = static_cast<DependencyPartArc*>(
            (*dependency_parts)[offset_arcs + r]);
        int par = arc->head();
        int ch = arc->modifier();

        if (par == ch) // This should never happen
          continue;

        if (par < ch)
        {
          for (int k = ch + 1; k < sentence->size(); k++)
          {
            vExpr[r*sentence->size() + k] += omega[r*sentence->size() + k] - omega[r*sentence->size() + k - 1];
          }
        }
        else
        {
          for (int k = ch - 1; k >= 0; k--)
          {
            vExpr[r*sentence->size() + k] += omega[r*sentence->size() + k] - omega[r*sentence->size() + k + 1];
          }
        }
      }

      // Now for par == ch1:
      vector<IloExpr> vExpr2(sentence->size() * sentence->size());
      for (int ind = 0; ind < vExpr2.size(); ind++)
        vExpr2[ind] = IloExpr(env);

      for (int par = 0; par < sentence->size(); par++)
      {
        for (int k = 0; k < sentence->size(); k++)
        {
          if (par == k)
            continue;

          if (par < k)
          {
              vExpr2[par*sentence->size() + k] += rho[par*sentence->size() + k] - rho[par*sentence->size() + k - 1];
          }
          else // if (par > k)
          {
              vExpr2[par*sentence->size() + k] += rho[par*sentence->size() + k] - rho[par*sentence->size() + k + 1];
          }
        }
      }

      for (int r = 0; r < num_next_siblings; ++r)
      {
        DependencyPartNextSibl *part = static_cast<DependencyPartNextSibl*>(
            (*dependency_parts)[offset_next_siblings + r]);
        int h = part->head();
        int m = part->modifier();
        int s = part->next_sibling();

        if (s == sentence->size() || s == -1) // Last child (left or right)
          continue;

        if (h == m) // First child
          vExpr2[h*sentence->size() + s] += z[offset_next_siblings + r];
        else
        {
          int r1 = dependency_parts->FindArc(h, m);
          int r2 = dependency_parts->FindArc(h, s);

          vExpr[r1*sentence->size() + s] += z[offset_next_siblings + r];
        }
      }

      for (int ind = 0; ind < vExpr.size(); ind++)
      {
        mod.add(vExpr[ind] == 0.0);
        vExpr[ind].end();
      }

      for (int ind = 0; ind < vExpr2.size(); ind++)
      {
        mod.add(vExpr2[ind] == 0.0);
        vExpr2[ind].end();
      }


      //////////////////////////////////////////////////
      // sum_{ch1} z_{par,ch1,ch2} = z_{par,ch2}
      //////////////////////////////////////////////////

      vExpr.clear();
      vExpr.resize(num_arcs);
      for (int ind = 0; ind < vExpr.size(); ind++)
        vExpr[ind] = IloExpr(env);

      for (int r = 0; r < num_next_siblings; ++r)
      {
        DependencyPartNextSibl *part = static_cast<DependencyPartNextSibl*>(
            (*dependency_parts)[offset_next_siblings + r]);
        int h = part->head();
        int m = part->modifier();
        int s = part->next_sibling();

        if (s == sentence->size() || s == -1) // Last child (left/right)
          continue;

        int r1 = dependency_parts->FindArc(h, m);
        int r2 = dependency_parts->FindArc(h, s);

        vExpr[r2] += z[offset_next_siblings + r];
      }

      for (int ind = 0; ind < vExpr.size(); ind++)
      {
        mod.add(vExpr[ind] == z[ind]);
        vExpr[ind].end();
      }

      // Take care of the leftmost/rightmost children:
      for (int r = 0; r < num_next_siblings; ++r)
      {
        DependencyPartNextSibl *part = static_cast<DependencyPartNextSibl*>(
            (*dependency_parts)[offset_next_siblings + r]);
        int h = part->head();
        int m = part->modifier();
        int s = part->next_sibling();

        // Do something for LAST CHILD:
        // * omega[r*sentence->size() + (sentence->size()-1)] if r points to the right
        // * omega[r*sentence->size() + 0]  if r points to the left
        // * rho[par*sentence->size() + (sentence->size()-1)] if r points to the right
        // * rho[par*sentence->size() + 0]  if r points to the left

        if (s == sentence->size()) // Rightmost child
        {
          if (h == m) // No child on right side
          {
            mod.add(z[offset_next_siblings + r] == rho[h*sentence->size() +  (sentence->size()-1)]);
          }
          else
          {
            int r1 = dependency_parts->FindArc(h, m);
            mod.add(z[offset_next_siblings + r] == omega[r1*sentence->size() +  (sentence->size()-1)]);
          }
        }
        else if (s == -1) // Leftmost child
        {
          if (h == m) // No child on left side
          {
            mod.add(z[offset_next_siblings + r] == rho[h*sentence->size() +  0]);
          }
          else
          {
            int r1 = dependency_parts->FindArc(h, m);
            mod.add(z[offset_next_siblings + r] == omega[r1*sentence->size() +  0]);
          }
        }
      }
    }

    if (use_grandparent_parts)
    {
      // z_ijk <= z_ij for each i,j,k
      // z_ijk <= z_ik for each i,j,k
      // z_ijk >= z_ij + z_ik - 1 for each i,j,k

      for (int r = 0; r < num_grandparents; ++r)
      {
        DependencyPartGrandpar *part = static_cast<DependencyPartGrandpar*>(
            (*dependency_parts)[offset_grandparents + r]);
        int g = part->grandparent();
        int h = part->head();
        int m = part->modifier();
        int r1 = dependency_parts->FindArc(g, h);
        int r2 = dependency_parts->FindArc(h, m);

        CHECK_GE(r1, 0);
        CHECK_GE(r2, 0);

        mod.add(z[offset_grandparents + r] - z[r1] <= 0.0);
        mod.add(z[offset_grandparents + r] - z[r2] <= 0.0);
        mod.add(z[offset_grandparents + r] - z[r1] - z[r2] >= -1.0);
      }
    }

    if (use_nonprojective_arc_parts)
    {
      int j, k;

      // Define auxiliary variables (indicates a path between i and j)
      IloNumVarArray psi(env, sentence->size()*sentence->size(),
          0.0, 1.0, ILOFLOAT);
      vector<IloExpr> vExpr(sentence->size()*sentence->size());
      for (int ind = 0; ind < vExpr.size(); ind++)
        vExpr[ind] = IloExpr(env);

      for (int r = 0; r < num_arcs; r++)
      {
        DependencyPartArc *arc = static_cast<DependencyPartArc*>(
            (*dependency_parts)[offset_arcs + r]);
        j = arc->modifier();
        if (j == 0)
          continue;
        for (k = 0; k < sentence->size(); k++)
        {
          vExpr[j + sentence->size()*k] += flow[r*sentence->size() + k];
        }
      }
      for (k = 0; k < sentence->size(); k++)
      {
        // psi_0k = 1, for each k = 1,...,n
        // psi_jk = sum(i) f_ijk, for each j,k = 1,...,n
        mod.add(psi[0 + sentence->size()*k] == 1);
        for (j = 1; j < sentence->size(); j++)
        {
          //mod.add(psi[j + sentence->size()*k] == vExpr[j + sentence->size()*k]);
          mod.add(psi[j + sentence->size()*k] <= vExpr[j + sentence->size()*k]);
          mod.add(psi[j + sentence->size()*k] >= vExpr[j + sentence->size()*k]);
        }
      }

      for (int ind = 0; ind < vExpr.size(); ind++)
      {
        vExpr[ind].end();
      }
      vExpr.clear();

      // znp_ij <= z_ij for each i,j
      // znp_ij >= z_ij - psi_ik for each i,j, and k in (i,j)
      // znp_ij <= -sum(k in (i,j)) psi_ik + |j-i| - 1, for each i,j
      for (int r = 0; r < num_nonprojective; ++r)
      {
        DependencyPartNonproj *part = static_cast<DependencyPartNonproj*>(
            (*dependency_parts)[offset_nonprojective + r]);
        int par = part->head();
        int ch = part->modifier();
        int r1 = dependency_parts->FindArc(par, ch); // Typically r1 == r - r0

        if (r1 < 0) // Should not happen
        {
          mod.add(z[offset_nonprojective + r] <= 0.0);
          continue;
        }
        else
          mod.add(z[offset_nonprojective + r] - z[r1] <= 0.0);

        int i0, j0;
        if (par < ch)
        {
          i0 = par;
          j0 = ch;
        }
        else if (par > ch)
        {
          i0 = ch;
          j0 = par;
        }
        else
          continue; // If ch==par, znp_ij = z_ij = 0 necessarily

        expr = IloExpr(env);
        for (int k = i0+1; k < j0; k++)
        {
          mod.add(z[offset_nonprojective + r] - z[r1] + psi[par + sentence->size()*k] >= 0.0);
          expr += psi[par + sentence->size()*k];
        }
        expr += z[offset_nonprojective + r];
        mod.add(expr <= j0 - i0 - 1);
        expr.end();
      }
    }

    if (use_path_parts)
    {
      int j, k;

      // Define auxiliary variables (indicates a path between i and j)
      IloNumVarArray psi(env, sentence->size()*sentence->size(),
          0.0, 1.0, ILOFLOAT);
      vector<IloExpr> vExpr(sentence->size()*sentence->size());
      for (int ind = 0; ind < vExpr.size(); ind++)
        vExpr[ind] = IloExpr(env);

      for (int r = 0; r < num_arcs; r++)
      {
        DependencyPartArc *arc = static_cast<DependencyPartArc*>(
            (*dependency_parts)[offset_arcs + r]);
        j = arc->modifier();
        if (j == 0)
          continue;
        for (k = 0; k < sentence->size(); k++)
        {
          vExpr[j + sentence->size()*k] += flow[r*sentence->size() + k];
        }
      }
      for (k = 0; k < sentence->size(); k++)
      {
        // psi_0k = 1, for each k = 1,...,n
        // psi_jk = sum(i) f_ijk, for each j,k = 1,...,n
        mod.add(psi[0 + sentence->size()*k] == 1);
        for (j = 1; j < sentence->size(); j++)
        {
          mod.add(psi[j + sentence->size()*k] == vExpr[j + sentence->size()*k]);
        }
      }

      for (int ind = 0; ind < vExpr.size(); ind++)
      {
        vExpr[ind].end();
      }
      vExpr.clear();

      // zpath_ij = psi_ij for each i,j
      for (int r = 0; r < num_path; ++r)
      {
        DependencyPartPath *part = static_cast<DependencyPartPath*>(
            (*dependency_parts)[offset_path + r]);
        int ancest = part->ancestor();
        int descend = part->descendant();

        mod.add(z[offset_path + r] == psi[ancest + sentence->size()*descend]);
      }
    }

    if (use_head_bigram_parts)
    {
      // z_{prevpar,par,ch} <= z_{par,ch} for each prevpar,par,ch
      // z_{prevpar,par,ch} <= z_{prevpar,ch-1} for each prevpar,par,ch
      // z_{prevpar,par,ch} >= z_{par,ch} + z_{prevpar,ch-1} - 1 for each prevpar,par,ch
      for (int r = 0; r < num_bigrams; ++r)
      {
        DependencyPartHeadBigram *part = static_cast<DependencyPartHeadBigram*>(
            (*dependency_parts)[offset_bigrams + r]);
        int prevpar = part->previous_head();
        int par = part->head();
        int ch = part->modifier();

        assert(ch-1 >= 0);

        int r1 = dependency_parts->FindArc(prevpar, ch-1);
        int r2 = dependency_parts->FindArc(par, ch);

        assert(r1 >= 0 && r2 >= 0);

        mod.add(z[offset_bigrams + r] - z[r1] <= 0.0);
        mod.add(z[offset_bigrams + r] - z[r2] <= 0.0);
        mod.add(z[offset_bigrams + r] - z[r1] - z[r2] >= -1.0);
      }
    }

    ///////////////////////////////////////////////////////////////////
    // Solve
    ///////////////////////////////////////////////////////////////////
    double tilim = pipe_->GetDependencyOptions()->train()? 60.0 : 300.0; // Time limit: 60 seconds training, 300 sec. testing
    cplex.setParam (IloCplex::TiLim, tilim); // Time limit: 60 seconds

    bool hasSolution = false;
    if (cplex.solve())
    {
      hasSolution = true;

      cplex.out() << "Solution status = " << cplex.getStatus() << endl;
      cplex.out() << "Solution value = " << cplex.getObjValue() << endl;

      gettimeofday(&end, NULL);
      double elapsed_time = diff_ms(end,start);
      cout << "Elapsed time (CPLEX) = " << elapsed_time << " (" << sentence->size() << ") " << endl;

    }
    else
    {
      cout << "Could not solve the LP!" << endl;
      if (cplex.getCplexStatus() == IloCplex::AbortTimeLim)
      {
        cout << "Time out!" << endl;

        cplex.out() << "Solution status = " << cplex.getStatus() << endl;

        if (!relax)
          cplex.out() << "Solution best value = " << cplex.getBestObjValue() << endl;
      }

      if (pipe_->GetDependencyOptions()->test())
      {
        if (cplex.isPrimalFeasible())
          hasSolution = true;
        else
        {
          cout << "Trying to solve the LP in primal form..." << endl;

          cplex.setParam(IloCplex::RootAlg, IloCplex::Primal);
          if (cplex.solve())
          {
            hasSolution = true;

            cplex.out() << "Solution status = " << cplex.getStatus() << endl;
            cplex.out() << "Solution value = " << cplex.getObjValue() << endl;
          }
          else
          {
            cout << "Could not solve the LP in primal form!" << endl;

            if (cplex.isPrimalFeasible())
            {
              hasSolution = true;

              cout << "However, a feasible solution was found." << endl;
            }
          }
          cplex.setParam(IloCplex::RootAlg, IloCplex::AutoAlg);
        }
      }
    }

    if (hasSolution)
    {
      IloNumArray zOpt(env,parts->size());

      cplex.getValues(z, zOpt);

      for (r = 0; r < parts->size(); r++) {
        // Skip labeled parts.
        if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) continue;
        (*predicted_output)[r] = zOpt[r];
      }

      ///////
      for (int j = 0; j < sentence->size(); j++)
      {
        double sum = 0.0;
        for (int i = 0; i < sentence->size(); i++)
        {
          r = dependency_parts->FindArc(i,j);
          if (r < 0)
            continue;
          sum += (*predicted_output)[r];

          if (relax)
          {
            double val = (*predicted_output)[r];
            if (val*(1-val) > 0.001)
            {
              cout << "Fractional value!" << endl;
            }
          }
        }

        if (j == 0)
          assert(NEARLY_EQ_TOL(sum, 0.0, 1e-9));
        else
          assert(NEARLY_EQ_TOL(sum, 1.0, 1e-9));
      }
    }

    env.end();
  }
  catch (IloException& ex)
  {
    cout << "Error: " << ex << endl;
    cerr << "Error: " << ex << endl;
  }
  catch (...)
  {
    cout << "Unspecified error" << endl;
    cerr << "Unspecified error" << endl;
  }
}

#endif

