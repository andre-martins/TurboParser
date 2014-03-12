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
#include "ad3/FactorGraph.h"
#include "FactorSemanticGraph.h"

// Define a matrix of doubles using Eigen.
typedef LogVal<double> LogValD;
namespace Eigen {
typedef Eigen::Matrix<LogValD, Dynamic, Dynamic> MatrixXlogd;
}

using namespace std;

DEFINE_double(train_cost_false_positives, 1.0,
              "Cost for predicting false positives.");
DEFINE_double(train_cost_false_negatives, 1.0,
              "Cost for predicting false negatives.");

void SemanticDecoder::DecodeCostAugmented(Instance *instance, Parts *parts,
                                          const vector<double> &scores,
                                          const vector<double> &gold_output,
                                          vector<double> *predicted_output,
                                          double *cost,
                                          double *loss) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int offset_arcs, num_arcs;

  // TODO(atm): make it possible to penalize wrong predicate parts as well?
  // Or unlabeled arcs in addition to labeled arcs?
  if (pipe_->GetSemanticOptions()->labeled()) {
    semantic_parts->GetOffsetLabeledArc(&offset_arcs, &num_arcs);
  } else {
    semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  }

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
  double a = FLAGS_train_cost_false_positives;
  // Penalty for predicting 0 when it is 1 (FN).
  double b = FLAGS_train_cost_false_negatives;

  // p = 0.5-z0, q = 0.5'*z0, loss = p'*z + q
  double q = 0.0;
  vector<double> p(num_arcs, 0.0);

  vector<double> scores_cost = scores;
  for (int r = 0; r < num_arcs; ++r) {
    p[r] = a - (a+b) * gold_output[offset_arcs + r];
    scores_cost[offset_arcs + r] += p[r];
    q += b*gold_output[offset_arcs + r];
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

#if 0
  for (int k = 0; k < 2; ++k) {
    const vector<double> *output;
    string type;
    if (k == 0) {
      type = "gold";
      output = &gold_output;
    } else {
      type = "predicted";
      output = predicted_output;
    }
    for (int r = 0; r < parts->size(); ++r) {
      int offset_pred_, offset_arcs_, offset_labeled_arcs_, offset_siblings_, num_;
      semantic_parts->GetOffsetPredicate(&offset_pred_, &num_);
      semantic_parts->GetOffsetArc(&offset_arcs_, &num_);
      semantic_parts->GetOffsetLabeledArc(&offset_labeled_arcs_, &num_);
      semantic_parts->GetOffsetSibling(&offset_siblings_, &num_);
      if (r >= offset_siblings_) {
        SemanticPartSibling *sibling =
          static_cast<SemanticPartSibling*>((*parts)[r]);
        if ((*output)[r] > 0) {
          LOG(INFO) << type << " sibling: " << "[" << r << "]" << " "
                    << sibling->predicate() << " "
                    << sibling->sense() << " "
                    << sibling->first_argument() << " "
                    << sibling->second_argument() << " "
                    << (*output)[r];
        }
      } else if (r >= offset_labeled_arcs_) {
        SemanticPartLabeledArc *labeled_arc =
          static_cast<SemanticPartLabeledArc*>((*parts)[r]);
        if ((*output)[r] > 0) {
          LOG(INFO) << type << " labeled_arc: " << "[" << r << "]" << " "
                    << labeled_arc->predicate() << " "
                    << labeled_arc->sense() << " "
                    << labeled_arc->argument() << " "
                    << labeled_arc->role() << " "
                    << (*output)[r];
        }
      } else if (r >= offset_arcs_) {
        SemanticPartArc *arc =
          static_cast<SemanticPartArc*>((*parts)[r]);
        if ((*output)[r] > 0) {
          LOG(INFO) << type << " arc: " << "[" << r << "]" << " "
                    << arc->predicate() << " "
                    << arc->sense() << " "
                    << arc->argument() << " "
                    << (*output)[r];
        }
      } else if (r >= offset_pred_) {
        SemanticPartPredicate *predicate =
          static_cast<SemanticPartPredicate*>((*parts)[r]);
        if ((*output)[r] > 0) {
          LOG(INFO) << type << " predicate: " << "[" << r << "]" << " "
                    << predicate->predicate() << " "
                    << predicate->sense() << " "
                    << (*output)[r];
        }
      } else {
        CHECK(false);
      }
    }
  }
#endif
}

void SemanticDecoder::DecodeMarginals(Instance *instance, Parts *parts,
                                      const vector<double> &scores,
                                      const vector<double> &gold_output,
                                      vector<double> *predicted_output,
                                      double *entropy,
                                      double *loss) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  // Right now, only allow marginal inference for arc-factored models.
  CHECK(semantic_parts->IsArcFactored());

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<double> total_scores;
  vector<double> label_marginals;
  int offset_predicate_parts, num_predicate_parts;
  int offset_arcs, num_arcs;
  int offset_labeled_arcs, num_labeled_arcs;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  semantic_parts->GetOffsetLabeledArc(&offset_labeled_arcs,
                                      &num_labeled_arcs);

  // If labeled parsing, decode the labels and update the scores.
  if (pipe_->GetSemanticOptions()->labeled()) {
    DecodeLabelMarginals(instance, parts, copied_scores, &total_scores,
        &label_marginals);
    for (int r = 0; r < total_scores.size(); ++r) {
      // Sum the "labeled" scores to the (eventually) already existing
      // "unlabeled" scores.
      copied_scores[offset_arcs + r] += total_scores[r];
    }
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  double log_partition_function;
  DecodeBasicMarginals(instance, parts, copied_scores, predicted_output,
                       &log_partition_function, entropy);

  // If labeled parsing, write the components of the predicted output that
  // correspond to the labeled parts.
  if (pipe_->GetSemanticOptions()->labeled()) {
    for (int r = 0; r < num_labeled_arcs; ++r) {
      SemanticPartLabeledArc *labeled_arc =
          static_cast<SemanticPartLabeledArc*>(
              (*parts)[offset_labeled_arcs + r]);
      int index_arc = semantic_parts->FindArc(labeled_arc->predicate(),
                                              labeled_arc->argument(),
                                              labeled_arc->sense());
      CHECK_GE(index_arc, 0);
      (*predicted_output)[offset_labeled_arcs + r] =
          label_marginals[r] * (*predicted_output)[index_arc];
    }

    // Recompute the entropy.
    *entropy = log_partition_function;
    for (int r = 0; r < num_predicate_parts; ++r) {
      *entropy -= (*predicted_output)[offset_predicate_parts + r] *
          scores[offset_predicate_parts + r];
    }
    for (int r = 0; r < num_arcs; ++r) {
      *entropy -= (*predicted_output)[offset_arcs + r] *
          scores[offset_arcs + r];
    }
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
void SemanticDecoder::DecodeLabels(Instance *instance, Parts *parts,
                                   const vector<double> &scores,
                                   vector<int> *best_labeled_parts) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  int offset, num_arcs;
  semantic_parts->GetOffsetArc(&offset, &num_arcs);
  best_labeled_parts->resize(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    SemanticPartArc *arc =
        static_cast<SemanticPartArc*>((*parts)[offset + r]);
    const vector<int> &index_labeled_parts =
        semantic_parts->FindLabeledArcs(arc->predicate(),
                                        arc->argument(),
                                        arc->sense());
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
void SemanticDecoder::DecodeLabelMarginals(Instance *instance, Parts *parts,
                                           const vector<double> &scores,
                                           vector<double> *total_scores,
                                           vector<double> *label_marginals) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  int offset, num_arcs;
  int offset_labeled, num_labeled_arcs;
  semantic_parts->GetOffsetArc(&offset, &num_arcs);
  semantic_parts->GetOffsetLabeledArc(&offset_labeled, &num_labeled_arcs);
  total_scores->clear();
  total_scores->resize(num_arcs, 0.0);
  label_marginals->clear();
  label_marginals->resize(num_labeled_arcs, 0.0);

  for (int r = 0; r < num_arcs; ++r) {
    SemanticPartArc *arc =
        static_cast<SemanticPartArc*>((*parts)[offset + r]);
    const vector<int> &index_labeled_parts =
        semantic_parts->FindLabeledArcs(arc->predicate(),
                                        arc->argument(),
                                        arc->sense());
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

void SemanticDecoder::Decode(Instance *instance, Parts *parts,
                             const vector<double> &scores,
                             vector<double> *predicted_output) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<int> best_labeled_parts;
  int offset_labeled_arcs, num_labeled_arcs;
  semantic_parts->GetOffsetLabeledArc(&offset_labeled_arcs,
                                      &num_labeled_arcs);
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  int offset_predicate_parts, num_predicate_parts;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);

  bool labeled_decoding = false;
  // TODO: change this test.
  int offset_labeled_siblings, num_labeled_siblings;
  semantic_parts->GetOffsetLabeledSibling(&offset_labeled_siblings,
                                          &num_labeled_siblings);
  if (num_labeled_siblings > 0) labeled_decoding = true;

  if (labeled_decoding) {
    predicted_output->clear();
    predicted_output->resize(parts->size(), 0.0);

    DecodeFactorGraph(instance, parts, copied_scores, labeled_decoding, true,
                      predicted_output);

    // At test time, run a basic decoder on top of the outcome of AD3
    // as a rounding heuristic to make sure we get a valid graph.
    // TODO: maybe change the interface to AD3 to let us implement
    // a primal rounding heuristic.
    if (pipe_->GetSemanticOptions()->test()) {
      double threshold = 0.5;
      for (int r = 0; r < num_arcs; ++r) {
        copied_scores[offset_arcs + r] = 0.0;
      }
      for (int r = 0; r < num_predicate_parts; ++r) {
        copied_scores[offset_predicate_parts + r] = 0.0;
      }
      for (int r = 0; r < num_labeled_arcs; ++r) {
        copied_scores[offset_labeled_arcs + r] =
          (*predicted_output)[offset_labeled_arcs + r] - threshold;
      }

      DecodeLabels(instance, parts, copied_scores, &best_labeled_parts);
      for (int r = 0; r < best_labeled_parts.size(); ++r) {
        // Sum the "labeled" scores to the (eventually) already existing
        // "unlabeled" scores.
        copied_scores[offset_arcs + r] += copied_scores[best_labeled_parts[r]];
      }

      double value;
      predicted_output->assign(parts->size(), 0.0);
      DecodeBasic(instance, parts, copied_scores, predicted_output, &value);

      // Write the components of the predicted output that
      // correspond to the labeled parts.
      for (int r = 0; r < num_arcs; ++r) {
        CHECK_GE(best_labeled_parts[r], offset_arcs + num_arcs);
        (*predicted_output)[best_labeled_parts[r]] =
          (*predicted_output)[offset_arcs + r];
      }
    }
  } else {
    // If labeled parsing, decode the labels and update the scores.
    if (pipe_->GetSemanticOptions()->labeled()) {
      DecodeLabels(instance, parts, copied_scores, &best_labeled_parts);
      for (int r = 0; r < best_labeled_parts.size(); ++r) {
        // Sum the "labeled" scores to the (eventually) already existing
        // "unlabeled" scores.
        copied_scores[offset_arcs + r] += copied_scores[best_labeled_parts[r]];
      }
    }

    predicted_output->clear();
    predicted_output->resize(parts->size(), 0.0);

    if (semantic_parts->IsArcFactored() ||
        semantic_parts->IsLabeledArcFactored()) {
      double value;
      DecodeBasic(instance, parts, copied_scores, predicted_output, &value);
    } else {
      DecodeFactorGraph(instance, parts, copied_scores, labeled_decoding, true,
                        predicted_output);

      // At test time, run a basic decoder on top of the outcome of AD3
      // as a rounding heuristic to make sure we get a valid graph.
      // TODO: maybe change the interface to AD3 to let us implement
      // a primal rounding heuristic.
      if (pipe_->GetSemanticOptions()->test()) {
        double threshold = 0.5;
        for (int r = 0; r < num_arcs; ++r) {
          copied_scores[offset_arcs + r] =
            (*predicted_output)[offset_arcs + r] - threshold;
        }
        for (int r = 0; r < num_predicate_parts; ++r) {
          copied_scores[offset_predicate_parts + r] = 0.0;
        }
        // This is not strictly necessary (since labeled arcs are not used by
        // DecodeBasic), but should not harm.
        for (int r = 0; r < num_labeled_arcs; ++r) {
          copied_scores[offset_labeled_arcs + r] = 0.0;
        }
        predicted_output->assign(parts->size(), 0.0);
        double value;
        DecodeBasic(instance, parts, copied_scores, predicted_output, &value);
      }
    }

    // If labeled parsing, write the components of the predicted output that
    // correspond to the labeled parts.
    if (pipe_->GetSemanticOptions()->labeled()) {
      for (int r = 0; r < num_arcs; ++r) {
        CHECK_GE(best_labeled_parts[r], offset_arcs + num_arcs);
        (*predicted_output)[best_labeled_parts[r]] =
          (*predicted_output)[offset_arcs + r];
      }
    }
  }
}

// Build predicate and arc indices.
void SemanticDecoder::BuildBasicIndices(
    int sentence_length,
    const vector<SemanticPartPredicate*> &predicate_parts,
    const vector<SemanticPartArc*> &arcs,
    vector<vector<int> > *index_predicates,
    vector<vector<vector<int> > > *arcs_by_predicate) {
  int num_arcs = arcs.size();
  int num_predicate_parts = predicate_parts.size();

  arcs_by_predicate->assign(sentence_length, vector<vector<int> >());
  for (int r = 0; r < num_arcs; ++r) {
    int p = arcs[r]->predicate();
    int s = arcs[r]->sense();
    if (s >= (*arcs_by_predicate)[p].size()) {
      (*arcs_by_predicate)[p].resize(s+1);
    }
    (*arcs_by_predicate)[p][s].push_back(r);
  }

  index_predicates->assign(sentence_length, vector<int>());
  for (int r = 0; r < num_predicate_parts; ++r) {
    CHECK_LT(r, predicate_parts.size());
    int p = predicate_parts[r]->predicate();
    int s = predicate_parts[r]->sense();
    if (s >= (*index_predicates)[p].size()) {
      (*index_predicates)[p].resize(s+1, -1);
    }
    (*index_predicates)[p][s] = r;
  }
}


void SemanticDecoder::DecodePruner(Instance *instance, Parts *parts,
                                   const vector<double> &scores,
                                   vector<double> *predicted_output) {
  //DecodePrunerNaive(instance, parts, scores, predicted_output);
  //return;

  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  double posterior_threshold =
      pipe_->GetSemanticOptions()->GetPrunerPosteriorThreshold();
  int max_arguments = pipe_->GetSemanticOptions()->GetPrunerMaxArguments();
  if (max_arguments < 0) max_arguments = sentence_length;

  int offset_predicate_parts, num_predicate_parts;
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  vector<SemanticPartArc*> arcs(num_arcs);
  vector<double> scores_arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    arcs[r] = static_cast<SemanticPartArc*>((*parts)[offset_arcs + r]);
    scores_arcs[r] = scores[offset_arcs + r];
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  CHECK(semantic_parts->IsArcFactored());

  vector<vector<vector<int> > > arcs_by_predicate;
  arcs_by_predicate.resize(sentence_length);
  for (int r = 0; r < num_arcs; ++r) {
    int p = arcs[r]->predicate();
    int s = arcs[r]->sense();
    if (s >= arcs_by_predicate[p].size()) {
      arcs_by_predicate[p].resize(s+1);
    }
    arcs_by_predicate[p][s].push_back(r);
  }

  double entropy;
  double log_partition_function;
  vector<double> posteriors;
  DecodeBasicMarginals(instance, parts, scores, &posteriors,
                       &log_partition_function, &entropy);

  // Get max_arguments argumens per predicate.
  int num_used_parts = 0;
  for (int p = 0; p < sentence_length; ++p) {
    for (int s = 0; s < arcs_by_predicate[p].size(); ++s) {
      vector<pair<double,int> > scores_arguments;
      for (int a = 1; a < sentence_length; ++a) {
        int r = semantic_parts->FindArc(p, a, s);
        if (r < 0) continue;
        scores_arguments.push_back(pair<double,int>(-posteriors[r], r));
      }
      if (scores_arguments.size() == 0) continue;
      sort(scores_arguments.begin(), scores_arguments.end());
      double max_posterior = 1.0; // -scores_arguments[0].first;
      for (int k = 0; k < max_arguments && k < scores_arguments.size(); ++k) {
        int r = scores_arguments[k].second;
        if (-scores_arguments[k].first >= posterior_threshold * max_posterior) {
          ++num_used_parts;
          (*predicted_output)[r] = 1.0;
        } else {
          break;
        }
      }
    }
  }

  VLOG(2) << "Pruning reduced to "
          << static_cast<double>(num_used_parts) /
                static_cast<double>(sentence_length)
          << " candidate heads per word.";
}

void SemanticDecoder::DecodePrunerNaive(Instance *instance, Parts *parts,
    const vector<double> &scores,
    vector<double> *predicted_output) {
  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int max_arguments = pipe_->GetSemanticOptions()->GetPrunerMaxArguments();
  int offset_predicate_parts, num_predicate_parts;
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  vector<SemanticPartArc*> arcs(num_arcs);
  vector<double> scores_arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    arcs[r] = static_cast<SemanticPartArc*>((*parts)[offset_arcs + r]);
    scores_arcs[r] = scores[offset_arcs + r];
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);
  for (int r = 0; r < num_predicate_parts; ++r) {
    // Don't prune any of the predicate parts.
    (*predicted_output)[offset_predicate_parts + r] = 1.0;
  }

  CHECK(semantic_parts->IsArcFactored());

  vector<vector<vector<int> > > arcs_by_predicate;
  arcs_by_predicate.resize(sentence_length);
  for (int r = 0; r < num_arcs; ++r) {
    int p = arcs[r]->predicate();
    int s = arcs[r]->sense();
    if (s >= arcs_by_predicate[p].size()) {
      arcs_by_predicate[p].resize(s+1);
    }
    arcs_by_predicate[p][s].push_back(r);
  }

  // Get max_arguments argumens per predicate.
  for (int p = 0; p < sentence_length; ++p) {
    for (int s = 0; s < arcs_by_predicate[p].size(); ++s) {
      vector<pair<double,int> > scores_arguments;
      for (int a = 1; a < sentence_length; ++a) {
        int r = semantic_parts->FindArc(p, a, s);
        if (r < 0) continue;
        scores_arguments.push_back(pair<double,int>(-scores[r], r));
      }
      sort(scores_arguments.begin(), scores_arguments.end());
      for (int k = 0; k < max_arguments && k < scores_arguments.size(); ++k) {
        int r = scores_arguments[k].second;
        (*predicted_output)[r] = 1.0;
        //LOG(INFO) << "Keeping arc (" << p << ", "
        //          << s << ", "
        //          << static_cast<SemanticPartArc*>((*parts)[r])->argument()
        //          << ").";
      }
    }
  }
}


// Decoder for the basic model. For each predicate, choose the best
// sense and the best set of arcs independently.
void SemanticDecoder::DecodeBasic(Instance *instance, Parts *parts,
                                  const vector<double> &scores,
                                  vector<double> *predicted_output,
                                  double *value) {
  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int offset_predicate_parts, num_predicate_parts;
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  vector<SemanticPartArc*> arcs(num_arcs);
  vector<double> scores_arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    arcs[r] = static_cast<SemanticPartArc*>((*parts)[offset_arcs + r]);
    scores_arcs[r] = scores[offset_arcs + r];
  }

  vector<SemanticPartPredicate*> predicate_parts(num_predicate_parts);
  vector<double> scores_predicates(num_predicate_parts);
  for (int r = 0; r < num_predicate_parts; ++r) {
    predicate_parts[r] =
      static_cast<SemanticPartPredicate*>((*parts)[offset_predicate_parts + r]);
    scores_predicates[r] = scores[offset_predicate_parts + r];
  }

  vector<vector<vector<int> > > arcs_by_predicate;
  vector<vector<int> > index_predicates;
  vector<bool> selected_predicates;
  vector<bool> selected_arcs;
  BuildBasicIndices(sentence_length, predicate_parts, arcs, &index_predicates,
                    &arcs_by_predicate);

  DecodeSemanticGraph(sentence_length, predicate_parts, arcs, index_predicates,
                      arcs_by_predicate, scores_predicates, scores_arcs,
                      &selected_predicates, &selected_arcs, value);

  predicted_output->resize(parts->size());
  for (int r = 0; r < num_predicate_parts; ++r) {
    if (selected_predicates[r]) {
      (*predicted_output)[offset_predicate_parts + r] = 1.0;
    } else {
      (*predicted_output)[offset_predicate_parts + r] = 0.0;
    }
  }
  for (int r = 0; r < num_arcs; ++r) {
    if (selected_arcs[r]) {
      (*predicted_output)[offset_arcs + r] = 1.0;
    } else {
      (*predicted_output)[offset_arcs + r] = 0.0;
    }
  }


#if 0
  arcs_by_predicate.resize(sentence_length);
  for (int r = 0; r < num_arcs; ++r) {
    int p = arcs[r]->predicate();
    int s = arcs[r]->sense();
    if (s >= arcs_by_predicate[p].size()) {
      arcs_by_predicate[p].resize(s+1);
    }
    arcs_by_predicate[p][s].push_back(r);
  }

  vector<double> scores_predicates(num_predicate_parts);
  //vector<vector<int> > index_predicates(sentence_length);
  for (int r = 0; r < num_predicate_parts; ++r) {
    scores_predicates[r] = scores[offset_predicate_parts + r];
    SemanticPartPredicate *predicate_part =
      static_cast<SemanticPartPredicate*>((*parts)[offset_predicate_parts + r]);
    int p = predicate_part->predicate();
    int s = predicate_part->sense();
    if (s >= index_predicates[p].size()) {
      index_predicates[p].resize(s+1, -1);
    }
    index_predicates[p][s] = r;
  }

  predicted_output->resize(parts->size());
  for (int r = 0; r < num_predicate_parts; ++r) {
    (*predicted_output)[offset_predicate_parts + r] = 0.0;
  }
  for (int r = 0; r < num_arcs; ++r) {
    (*predicted_output)[offset_arcs + r] = 0.0;
  }

  double total_score = 0.0;
  for (int p = 0; p < sentence_length; ++p) {
    int best_sense = -1;
    double best_score = 0.0;
    vector<vector<bool> > selected_arcs;
    selected_arcs.resize(arcs_by_predicate[p].size());
    for (int s = 0; s < arcs_by_predicate[p].size(); ++s) {
      // Compute the best assignment of arcs departing from this
      // predicate word.
      selected_arcs[s].resize(arcs_by_predicate[p][s].size());
      int r = index_predicates[p][s];
      double score = scores_predicates[r];
      for (int k = 0; k < arcs_by_predicate[p][s].size(); ++k) {
        int r = arcs_by_predicate[p][s][k];
        if (scores_arcs[r] > 0.0) {
          selected_arcs[s][k] = true;
          score += scores_arcs[r];
        } else {
          selected_arcs[s][k] = false;
        }
      }
      if (score > best_score) {
        best_sense = s;
        best_score = score;
      }
    }
    if (best_sense >= 0) {
      total_score += best_score;
      int s = best_sense;
      int r = index_predicates[p][s];
      CHECK_GE(r, 0);
      (*predicted_output)[offset_predicate_parts + r] = 1.0;
      for (int k = 0; k < arcs_by_predicate[p][s].size(); ++k) {
        if (!selected_arcs[s][k]) continue;
        int r = arcs_by_predicate[p][s][k];
        (*predicted_output)[offset_arcs + r] = 1.0;
        //LOG(INFO) << "Selected arc "
        //          << arcs[r]->predicate() << " "
        //          << arcs[r]->argument() << " "
        //          << arcs[r]->sense();
      }
    }
  }
#endif
}

// Decoder for the basic model. For each predicate, choose the best
// sense and the best set of arcs independently.
void SemanticDecoder::DecodeBasicMarginals(Instance *instance, Parts *parts,
                                           const vector<double> &scores,
                                           vector<double> *predicted_output,
                                           double *log_partition_function,
                                           double *entropy) {
  int sentence_length =
    static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int offset_predicate_parts, num_predicate_parts;
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  vector<SemanticPartArc*> arcs(num_arcs);
  vector<double> scores_arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    arcs[r] = static_cast<SemanticPartArc*>((*parts)[offset_arcs + r]);
    scores_arcs[r] = scores[offset_arcs + r];
  }

  vector<vector<vector<int> > > arcs_by_predicate;
  arcs_by_predicate.resize(sentence_length);
  for (int r = 0; r < num_arcs; ++r) {
    int p = arcs[r]->predicate();
    int s = arcs[r]->sense();
    if (s >= arcs_by_predicate[p].size()) {
      arcs_by_predicate[p].resize(s+1);
    }
    arcs_by_predicate[p][s].push_back(r);
  }

  vector<double> scores_predicates(num_predicate_parts);
  vector<vector<int> > index_predicates(sentence_length);
  for (int r = 0; r < num_predicate_parts; ++r) {
    scores_predicates[r] = scores[offset_predicate_parts + r];
    SemanticPartPredicate *predicate_part =
      static_cast<SemanticPartPredicate*>((*parts)[offset_predicate_parts + r]);
    int p = predicate_part->predicate();
    int s = predicate_part->sense();
    if (s >= index_predicates[p].size()) {
      index_predicates[p].resize(s+1, -1);
    }
    index_predicates[p][s] = r;
  }

  predicted_output->resize(parts->size());
  for (int r = 0; r < num_predicate_parts; ++r) {
    (*predicted_output)[offset_predicate_parts + r] = 0.0;
  }
  for (int r = 0; r < num_arcs; ++r) {
    (*predicted_output)[offset_arcs + r] = 0.0;
  }

  *log_partition_function = 0.0;
  *entropy = 0.0;
  for (int p = 0; p < sentence_length; ++p) {
    // Initiliaze log partition all senses to exp(0.0) to account for
    // the null sense which implies there are no outgoing arcs.
    LogValD log_partition_all_senses = LogValD::One();
    //LogValD log_partition_all_senses = LogValD::Zero();
    vector<LogValD> log_partition_senses(arcs_by_predicate[p].size(),
                                         LogValD::Zero());
    vector<vector<LogValD> > log_partition_arcs(arcs_by_predicate[p].size());
    for (int s = 0; s < arcs_by_predicate[p].size(); ++s) {
      int r = index_predicates[p][s];
      double score = scores_predicates[r];
      //CHECK_EQ(score, 0.0);
      // Initialize log partition arcs to exp(0.0) to account for the
      // event that the arc does not exist.
      log_partition_arcs[s].assign(arcs_by_predicate[p][s].size(),
                                   LogValD::One());
      for (int k = 0; k < arcs_by_predicate[p][s].size(); ++k) {
        int r = arcs_by_predicate[p][s][k];
        log_partition_arcs[s][k] += LogValD(scores_arcs[r], false);
        //score += log_partition_arcs[s][k].as_float();
        score += log_partition_arcs[s][k].logabs();
      }
      //LOG(INFO) << s << " " << score;
      log_partition_senses[s] = LogValD(score, false);
      //log_partition_senses[s] = LogValD(score);
      //LOG(INFO) << s << " " << log_partition_senses[s].logabs();
      log_partition_all_senses += log_partition_senses[s];
    }

    // This makes sure the log partition function does not become -infty for
    // predicates that do not have any sense.
    if (arcs_by_predicate[p].size() > 0) {
      if (false && sentence_length < 5) {
        LOG(INFO) << "Log partition[" << p << "] = "
                  << log_partition_all_senses.logabs();
      }
      *log_partition_function += log_partition_all_senses.logabs();
    }

    for (int s = 0; s < arcs_by_predicate[p].size(); ++s) {
      int r = index_predicates[p][s];
      double predicate_marginal = LogValD(log_partition_senses[s].logabs() -
                                          log_partition_all_senses.logabs(),
                                          false).as_float();
      //LOG(INFO) << "Predicate marginal[" << p << "][" << s << "] = "
      //          << predicate_marginal;
      (*predicted_output)[offset_predicate_parts + r] = predicate_marginal;
      //CHECK_EQ(scores_predicates[r], 0.0);
      *entropy -= scores_predicates[r] * predicate_marginal;
      for (int k = 0; k < arcs_by_predicate[p][s].size(); ++k) {
        int r = arcs_by_predicate[p][s][k];
        double marginal = LogValD(scores_arcs[r] -
                                  log_partition_arcs[s][k].logabs(),
                                  false).as_float();
        marginal *= predicate_marginal;
        if (false && sentence_length < 5) {
          LOG(INFO) << "marginal[" << p << "][" << s << "][" << k << "] = "
                    << marginal << "\t"
                    << "scores_arcs[" << p << "][" << s << "][" << k << "] = "
                    << scores_arcs[r];
        }
        (*predicted_output)[offset_arcs + r] = marginal;
        *entropy -= scores_arcs[r] * marginal;
      }
    }
  }

  *entropy += *log_partition_function;
  if (false && sentence_length < 5) {
    LOG(INFO) << "Log-partition function:" << *log_partition_function;
    LOG(INFO) << "Entropy:" << *entropy;
  }
}

// Decoder for the basic model. For each predicate, choose the best
// sense and the best set of arcs independently.
void SemanticDecoder::DecodeSemanticGraph(
    int sentence_length,
    const vector<SemanticPartPredicate*> &predicate_parts,
    const vector<SemanticPartArc*> &arcs,
    const vector<vector<int> > &index_predicates,
    const vector<vector<vector<int> > > &arcs_by_predicate,
    const vector<double> &predicate_scores,
    const vector<double> &arc_scores,
    vector<bool> *selected_predicates,
    vector<bool> *selected_arcs,
    double *value) {
  int num_predicate_parts = predicate_parts.size();
  int num_arcs = arcs.size();

  selected_predicates->assign(num_predicate_parts, false);
  selected_arcs->assign(num_arcs, false);

  double total_score = 0.0;
  for (int p = 0; p < sentence_length; ++p) {
    int best_sense = -1;
    double best_score = 0.0;
    vector<vector<bool> > selected;
    selected.resize(arcs_by_predicate[p].size());
    for (int s = 0; s < arcs_by_predicate[p].size(); ++s) {
      // Compute the best assignment of arcs departing from this
      // predicate word.
      selected[s].resize(arcs_by_predicate[p][s].size());
      int r = index_predicates[p][s];
      double score = predicate_scores[r];
      for (int k = 0; k < arcs_by_predicate[p][s].size(); ++k) {
        int r = arcs_by_predicate[p][s][k];
        if (arc_scores[r] > 0.0) {
          selected[s][k] = true;
          score += arc_scores[r];
        } else {
          selected[s][k] = false;
        }
      }
      // Note: we're allowing a non-null sense (!= -1) without outgoing arcs.
      if (score > best_score) {
        best_sense = s;
        best_score = score;
      }
    }
    if (best_sense >= 0) {
      total_score += best_score;
      int s = best_sense;
      int r = index_predicates[p][s];
      CHECK_GE(r, 0);
      (*selected_predicates)[r] = true;
      for (int k = 0; k < arcs_by_predicate[p][s].size(); ++k) {
        if (!selected[s][k]) continue;
        int r = arcs_by_predicate[p][s][k];
        (*selected_arcs)[r] = true;
      }
    }
  }
  *value = total_score;
}

// Decode building a factor graph and calling the AD3 algorithm.
void SemanticDecoder::DecodeFactorGraph(Instance *instance, Parts *parts,
                                        const vector<double> &scores,
                                        bool labeled_decoding,
                                        bool relax,
                                        vector<double> *predicted_output) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticInstanceNumeric* sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  CHECK(relax);

  // Get the offsets for the different parts.
  int offset_predicate_parts, num_predicate_parts;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);
  int offset_labeled_arcs, num_labeled_arcs;
  semantic_parts->GetOffsetLabeledArc(&offset_labeled_arcs,
                                      &num_labeled_arcs);
  int offset_siblings, num_siblings;
  semantic_parts->GetOffsetSibling(&offset_siblings, &num_siblings);
  int offset_labeled_siblings, num_labeled_siblings;
  semantic_parts->GetOffsetLabeledSibling(&offset_labeled_siblings,
                                          &num_labeled_siblings);
  int offset_grandparents, num_grandparents;
  semantic_parts->GetOffsetGrandparent(&offset_grandparents, &num_grandparents);
  int offset_coparents, num_coparents;
  semantic_parts->GetOffsetCoparent(&offset_coparents, &num_coparents);

#if 0
  int offset_next_siblings, num_next_siblings;
  semantic_parts->GetOffsetNextSibling(&offset_next_siblings,
                                       &num_next_siblings);
  int offset_grandsiblings, num_grandsiblings;
  semantic_parts->GetOffsetGrandSibling(&offset_grandsiblings, &num_grandsiblings);
  int offset_trisiblings, num_trisiblings;
  dependency_parts->GetOffsetTriSibling(&offset_trisiblings, &num_trisiblings);
#endif

  // Define what parts are used.
  bool use_arbitrary_sibling_parts = (num_siblings > 0);
  bool use_labeled_arbitrary_sibling_parts = (num_labeled_siblings > 0);
  bool use_grandparent_parts = (num_grandparents > 0);
  bool use_coparent_parts = (num_coparents > 0);
#if 0
  bool use_next_sibling_parts = (num_next_siblings > 0);
  bool use_grandsibling_parts = (num_grandsiblings > 0);
  bool use_trisibling_parts = (num_trisiblings > 0);
#endif

  if (!labeled_decoding) {
    CHECK_EQ(num_labeled_siblings, 0);
  }

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

  // Build predicate part variables.
  int offset_predicate_variables = variables.size();
  for (int r = 0; r < num_predicate_parts; ++r) {
    AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
    variable->SetLogPotential(scores[offset_predicate_parts + r]);
    variables.push_back(variable);
    part_indices_.push_back(offset_predicate_parts + r);
  }

  // Build arc variables.
  int offset_arc_variables = variables.size();
  for (int r = 0; r < num_arcs; ++r) {
    AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
    variable->SetLogPotential(scores[offset_arcs + r]);
    variables.push_back(variable);
    part_indices_.push_back(offset_arcs + r);
  }

  int offset_labeled_arc_variables = variables.size();
  if (labeled_decoding) {
    // Build labeled arc variables.
    for (int r = 0; r < num_labeled_arcs; ++r) {
      AD3::BinaryVariable* variable = factor_graph->CreateBinaryVariable();
      variable->SetLogPotential(scores[offset_labeled_arcs + r]);
      variables.push_back(variable);
      part_indices_.push_back(offset_labeled_arcs + r);
    }
  }

  // Build basic semantic graph factor.
  vector<AD3::BinaryVariable*> local_variables(num_predicate_parts + num_arcs);
  vector<SemanticPartPredicate*> predicate_parts(num_predicate_parts);
  for (int r = 0; r < num_predicate_parts; ++r) {
    local_variables[r] = variables[offset_predicate_variables + r];
    predicate_parts[r] =
      static_cast<SemanticPartPredicate*>((*parts)[offset_predicate_parts + r]);
  }
  vector<SemanticPartArc*> arcs(num_arcs);
  for (int r = 0; r < num_arcs; ++r) {
    local_variables[num_predicate_parts + r] =
      variables[offset_arc_variables + r];
    arcs[r] = static_cast<SemanticPartArc*>((*parts)[offset_arcs + r]);
  }
  AD3::FactorSemanticGraph *factor = new AD3::FactorSemanticGraph;
  factor->Initialize(sentence->size(), predicate_parts, arcs, this);
  factor_graph->DeclareFactor(factor, local_variables, true);
  factor_part_indices_.push_back(-1);

  if (labeled_decoding) {
    // Build XOR-OUT factors to impose that each arc has a unique label.
    for (int r = 0; r < num_arcs; ++r) {
      const vector<int> &index_labeled_parts =
        semantic_parts->GetLabeledParts(offset_arcs + r);
      vector<AD3::BinaryVariable*>
        local_variables(index_labeled_parts.size() + 1);
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(offset_labeled_arc_variables + index_part - offset_labeled_arcs,
                 variables.size());
        local_variables[k] = variables[offset_labeled_arc_variables +
                                       index_part - offset_labeled_arcs];
      }
      CHECK_GE(offset_arc_variables + r, 0);
      CHECK_LT(offset_arc_variables + r, variables.size());
      local_variables[index_labeled_parts.size()] =
        variables[offset_arc_variables + r];
      factor_graph->CreateFactorXOROUT(local_variables);
      factor_part_indices_.push_back(-1);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Build sibling factors.
  //////////////////////////////////////////////////////////////////////
  if (use_arbitrary_sibling_parts) {
    for (int r = 0; r < num_siblings; ++r) {
      SemanticPartSibling *part = static_cast<SemanticPartSibling*>(
          (*semantic_parts)[offset_siblings + r]);
      int r1 = semantic_parts->FindArc(part->predicate(),
                                       part->first_argument(),
                                       part->sense());
      int r2 = semantic_parts->FindArc(part->predicate(),
                                       part->second_argument(),
                                       part->sense());
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_arcs +
                                          offset_arc_variables]);
      local_variables.push_back(variables[r2 - offset_arcs +
                                          offset_arc_variables]);

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
  // Build labeled sibling factors.
  //////////////////////////////////////////////////////////////////////
  if (use_labeled_arbitrary_sibling_parts) {
    CHECK(labeled_decoding);
    for (int r = 0; r < num_labeled_siblings; ++r) {
      SemanticPartLabeledSibling *part =
          static_cast<SemanticPartLabeledSibling*>(
              (*semantic_parts)[offset_labeled_siblings + r]);
      int r1 = semantic_parts->FindLabeledArc(part->predicate(),
                                              part->first_argument(),
                                              part->sense(),
                                              part->first_role());
      int r2 = semantic_parts->FindLabeledArc(part->predicate(),
                                              part->second_argument(),
                                              part->sense(),
                                              part->second_role());
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_labeled_arcs +
                                          offset_labeled_arc_variables]);
      local_variables.push_back(variables[r2 - offset_labeled_arcs +
                                          offset_labeled_arc_variables]);

      factor_graph->CreateFactorPAIR(local_variables,
                                     scores[offset_labeled_siblings + r]);
      // TODO: set these global indices at the end after all variables/factors
      // are created.
      //factor->SetGlobalIndex(...);
      additional_part_indices.push_back(offset_labeled_siblings + r);
      factor_part_indices_.push_back(offset_labeled_siblings + r);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Build grandparent factors.
  //////////////////////////////////////////////////////////////////////
  if (use_grandparent_parts) {
    for (int r = 0; r < num_grandparents; ++r) {
      SemanticPartGrandparent *part = static_cast<SemanticPartGrandparent*>(
          (*semantic_parts)[offset_grandparents + r]);
      int r1 = semantic_parts->FindArc(part->grandparent_predicate(),
                                       part->predicate(),
                                       part->grandparent_sense());
      int r2 = semantic_parts->FindArc(part->predicate(),
                                       part->argument(),
                                       part->sense());
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_arcs +
                                          offset_arc_variables]);
      local_variables.push_back(variables[r2 - offset_arcs +
                                          offset_arc_variables]);

      factor_graph->CreateFactorPAIR(local_variables,
                                     scores[offset_grandparents + r]);
      // TODO: set these global indices at the end after all variables/factors
      // are created.
      //factor->SetGlobalIndex(...);
      additional_part_indices.push_back(offset_grandparents + r);
      factor_part_indices_.push_back(offset_grandparents + r);
    }
  }

  //////////////////////////////////////////////////////////////////////
  // Build co-parent factors.
  //////////////////////////////////////////////////////////////////////
  if (use_coparent_parts) {
    for (int r = 0; r < num_coparents; ++r) {
      SemanticPartCoparent *part = static_cast<SemanticPartCoparent*>(
          (*semantic_parts)[offset_coparents + r]);
      int r1 = semantic_parts->FindArc(part->first_predicate(),
                                       part->argument(),
                                       part->first_sense());
      int r2 = semantic_parts->FindArc(part->second_predicate(),
                                       part->argument(),
                                       part->second_sense());
      CHECK_GE(r1, 0);
      CHECK_GE(r2, 0);
      vector<AD3::BinaryVariable*> local_variables;
      local_variables.push_back(variables[r1 - offset_arcs +
                                          offset_arc_variables]);
      local_variables.push_back(variables[r2 - offset_arcs +
                                          offset_arc_variables]);

      factor_graph->CreateFactorPAIR(local_variables,
                                     scores[offset_coparents + r]);
      // TODO: set these global indices at the end after all variables/factors
      // are created.
      //factor->SetGlobalIndex(...);
      additional_part_indices.push_back(offset_coparents + r);
      factor_part_indices_.push_back(offset_coparents + r);
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

  VLOG(2) << "Number of factors: " << factor_graph->GetNumFactors();
  VLOG(2) << "Number of variables: " << factor_graph->GetNumVariables();

#if 0
  LOG(INFO) << "Number of factors: " << factor_graph->GetNumFactors();
  LOG(INFO) << "Number of variables: " << factor_graph->GetNumVariables();
  LOG(INFO) << "Number of siblings: " << num_siblings;
  LOG(INFO) << "part_indices_.size() = " << part_indices_.size();
  LOG(INFO) << "additional_part_indices.size() = " << additional_part_indices.size();
  LOG(INFO) << "factor_part_indices_.size() = " << factor_part_indices_.size();
#endif

  vector<int> recomputed_indices(part_indices_.size(), -1);
  bool solved = false;

  //#define PRINT_GRAPH
#ifdef PRINT_GRAPH
  ofstream stream;
  stream.open("tmp.fg", ofstream::out | ofstream::app);
  CHECK(stream.good());
  factor_graph->Print(stream);
  stream << endl;
  stream.flush();
  stream.clear();
  stream.close();
#endif

  vector<double> posteriors;
  vector<double> additional_posteriors;
  double value_ref;
  double *value = &value_ref;

  //factor_graph->SetMaxIterationsAD3(2000);
  factor_graph->SetMaxIterationsAD3(500);
  factor_graph->SetEtaAD3(0.05);
  factor_graph->AdaptEtaAD3(true);
  factor_graph->SetResidualThresholdAD3(1e-3);
  //factor_graph->SetResidualThresholdAD3(1e-6);

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
    if (i < posteriors.size()) {
      (*predicted_output)[r] = posteriors[i];
    } else {
      int j = i - posteriors.size();
      (*predicted_output)[r] = additional_posteriors[j];
#if 0
      ///
      CHECK_GE(r, offset_siblings);
      CHECK_LT(j+1, factor_graph->GetNumFactors());
      AD3::Factor *factor = factor_graph->GetFactor(j+1);
      CHECK_EQ(factor->Degree(), 2) << j+1 << " " << factor->GetId();
      AD3::BinaryVariable *var1 = factor->GetVariable(0);
      AD3::BinaryVariable *var2 = factor->GetVariable(1);
      int i1 = var1->GetId();
      int i2 = var2->GetId();
      if (posteriors[i1] > 0.9 && posteriors[i2] > 0.9) {
        CHECK_GE(additional_posteriors[j], 0.9);
      }
      int r1 = offset_arcs + i1 - offset_arc_variables;
      int r2 = offset_arcs + i2 - offset_arc_variables;
      CHECK_EQ(posteriors[i1], (*predicted_output)[r1]);
      CHECK_EQ(posteriors[i2], (*predicted_output)[r2]);
      SemanticPartArc* arc1 = static_cast<SemanticPartArc*>((*semantic_parts)[r1]);
      SemanticPartArc* arc2 = static_cast<SemanticPartArc*>((*semantic_parts)[r2]);
      SemanticPartSibling* part = static_cast<SemanticPartSibling*>((*semantic_parts)[r]);
      CHECK_EQ(arc1->predicate(), part->predicate());
      CHECK_EQ(arc1->sense(), part->sense());
      CHECK_EQ(arc1->argument(), part->first_argument());
      CHECK_EQ(arc2->predicate(), part->predicate());
      CHECK_EQ(arc2->sense(), part->sense());
      CHECK_EQ(arc2->argument(), part->second_argument());

      if (part->predicate() == 9) {
        LOG(INFO) << "*** sibling "
                  << "[" << r << " " << r1 << " " << r2 << "] "
                  << part->predicate() << " "
                  << part->sense() << " "
                  << part->first_argument() << " "
                  << part->second_argument() << " = "
                  << (*predicted_output)[r] << " "
                  << (*predicted_output)[r1] << " "
                  << (*predicted_output)[r2];
      }
      ///
#endif
    }
    *value += (*predicted_output)[r] * scores[r];
  }

#if 0
  //////
  for (int r = 0; r < parts->size(); ++r) {
    if (r >= offset_labeled_arcs && r < offset_labeled_arcs + num_labeled_arcs) {
      (*predicted_output)[r] = 0.0;
    } else {
      CHECK_GE((*predicted_output)[r], -1e-12);
    }
  }
  ///////

  delete factor_graph;
#endif

  VLOG(2) << "Solution value (AD3) = " << *value;
}
