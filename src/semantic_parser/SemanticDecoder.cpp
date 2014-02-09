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

#include "SemanticDecoder.h"
#include "SemanticPart.h"
#include "SemanticPipe.h"
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

void SemanticDecoder::DecodeCostAugmented(Instance *instance, Parts *parts,
                                            const vector<double> &scores,
                                            const vector<double> &gold_output,
                                            vector<double> *predicted_output,
                                            double *cost,
                                            double *loss) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int offset_arcs, num_arcs;

  if (pipe_->GetSemanticOptions()->labeled()) {
    semantic_parts->GetOffsetLabeledArc(&offset_arcs, &num_arcs);
  } else {
    semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);
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

void SemanticDecoder::DecodeMarginals(Instance *instance, Parts *parts,
                                      const vector<double> &scores,
                                      const vector<double> &gold_output,
                                      vector<double> *predicted_output,
                                      double *entropy,
                                      double *loss) {
#if 0
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);

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
  if (pipe_->GetSemanticOptions()->labeled()) {
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
  if (pipe_->GetSemanticOptions()->labeled()) {
    for (int r = 0; r < num_labeled_arcs; ++r) {
      SemanticPartLabeledArc *labeled_arc =
          static_cast<SemanticPartLabeledArc*>(
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
#endif
}

// Decode the best label for each candidate arc. The output vector
// best_labeled_parts, indexed by the unlabeled arcs, contains the indices
// of the best labeled part for each arc.
void SemanticDecoder::DecodeLabels(Instance *instance, Parts *parts,
                                   const vector<double> &scores,
                                   vector<int> *best_labeled_parts) {
#if 0
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);

  int offset, num_arcs;
  dependency_parts->GetOffsetArc(&offset, &num_arcs);
  best_labeled_parts->resize(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    SemanticPartArc *arc =
        static_cast<SemanticPartArc*>((*parts)[offset + r]);
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
#endif
}

// Decode the label marginals for each candidate arc. The output vector
// total_scores contains the sum of exp-scores (over the labels) for each arc;
// label_marginals contains those marginals ignoring the tree constraint.
void SemanticDecoder::DecodeLabelMarginals(Instance *instance, Parts *parts,
                                           const vector<double> &scores,
                                           vector<double> *total_scores,
                                           vector<double> *label_marginals) {
#if 0
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);

  int offset, num_arcs;
  int offset_labeled, num_labeled_arcs;
  dependency_parts->GetOffsetArc(&offset, &num_arcs);
  dependency_parts->GetOffsetLabeledArc(&offset_labeled, &num_labeled_arcs);
  total_scores->clear();
  total_scores->resize(num_arcs, 0.0);
  label_marginals->clear();
  label_marginals->resize(num_labeled_arcs, 0.0);

  for (int r = 0; r < num_arcs; ++r) {
    SemanticPartArc *arc =
        static_cast<SemanticPartArc*>((*parts)[offset + r]);
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
#endif
}

void SemanticDecoder::Decode(Instance *instance, Parts *parts, 
                             const vector<double> &scores,
                             vector<double> *predicted_output) {
#if 0
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<int> best_labeled_parts;
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  // If labeled parsing, decode the labels and update the scores.
  if (pipe_->GetSemanticOptions()->labeled()) {
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
    // At test time, run Chu-Liu-Edmonds on top of the outcome of AD3
    // as a rounding heuristic to make sure we get a valid tree.
    // TODO: maybe change the interface to AD3 to let us implement
    // a primal rounding heuristic - in this case just getting the output
    // coming from the TREE factor should work well enough.
    if (pipe_->GetSemanticOptions()->test()) {
      for (int r = 0; r < num_arcs; ++r) {
        copied_scores[offset_arcs + r] = (*predicted_output)[offset_arcs + r];
      }
      double value;
      DecodeBasic(instance, parts, copied_scores, predicted_output, &value);
    }
  }

  // If labeled parsing, write the components of the predicted output that
  // correspond to the labeled parts.
  if (pipe_->GetSemanticOptions()->labeled()) {
    for (int r = 0; r < num_arcs; ++r) {
      (*predicted_output)[best_labeled_parts[r]] =
          (*predicted_output)[offset_arcs + r];
    }
  }
#endif
}

void SemanticDecoder::DecodePruner(Instance *instance, Parts *parts,
                                   const vector<double> &scores,
                                   vector<double> *predicted_output) {
#if 0
  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);
  double posterior_threshold =
      pipe_->GetSemanticOptions()->GetPrunerPosteriorThreshold();
  int max_heads = pipe_->GetSemanticOptions()->GetPrunerMaxHeads();
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
#endif
}

#if 0
void SemanticDecoder::DecodePrunerNaive(Instance *instance, Parts *parts,
    const vector<double> &scores,
    vector<double> *predicted_output) {
  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);
  int max_heads = pipe_->GetSemanticOptions()->GetPrunerMaxHeads();
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
#endif

// Decoder for the basic model; it finds a maximum weighted arborescence
// using Edmonds' algorithm (which runs in O(n^2)).
void SemanticDecoder::DecodeBasic(Instance *instance, Parts *parts,
                                    const vector<double> &scores,
                                    vector<double> *predicted_output,
                                    double *value) {
#if 0
  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *dependency_parts = static_cast<SemanticParts*>(parts);
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  vector<SemanticPartArc*> arcs(num_arcs);
  vector<double> scores_arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    arcs[r] = static_cast<SemanticPartArc*>((*parts)[offset_arcs + r]);
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
#endif
}
