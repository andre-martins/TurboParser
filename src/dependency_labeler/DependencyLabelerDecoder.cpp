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

#include "DependencyLabelerDecoder.h"
#include "DependencyLabelerPart.h"
#include "DependencyLabelerPipe.h"
#include "AlgUtils.h"
#include <iostream>
#include "logval.h"

void DependencyLabelerDecoder::DecodeCostAugmented(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    const std::vector<double> &gold_output,
    std::vector<double> *predicted_output,
    double *cost,
    double *loss) {
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  int offset_arcs, num_arcs;

  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

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

    DependencyLabelerPartArc *arc =
      static_cast<DependencyLabelerPartArc*>((*dependency_parts)[offset_arcs + r]);
#if 0
    LOG(INFO) << arc->head() << " -> "  << arc->modifier() << " " << arc->label()
              << " gold=" << gold_output[offset_arcs + r]
              << " predicted=" << (*predicted_output)[offset_arcs + r];
#endif
  }

  *loss = *cost;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }
}

void DependencyLabelerDecoder::Decode(Instance *instance, Parts *parts,
                                      const std::vector<double> &scores,
                                      std::vector<double> *predicted_output) {
  //LOG(INFO) << "Decode";
  DependencyLabelerOptions *options =
    static_cast<DependencyLabelerPipe*>(pipe_)->GetDependencyLabelerOptions();
  if (!options->use_sibling_parts()) {
    DecodeBasic(instance, parts, scores, predicted_output);
  } else {
    DependencyLabelerParts *dependency_parts =
      static_cast<DependencyLabelerParts*>(parts);
    int instance_length =
      static_cast<DependencyInstanceNumeric*>(instance)->size();
    const std::vector<std::vector<int> > &siblings =
      dependency_parts->siblings();

    predicted_output->clear();
    predicted_output->resize(parts->size(), 0.0);

    for (int h = 0; h < instance_length; ++h) {
      if (siblings[h].size() == 0) continue;
      std::vector<std::vector<double> > node_scores(
        siblings[h].size(), std::vector<double>(0));
      std::vector<std::vector<std::vector<double> > > edge_scores(
        siblings[h].size()-1, std::vector<std::vector<double> >(0));
      for (int i = 0; i < siblings[h].size() + 1; ++i) {
        const std::vector<int> &sibling_parts =
          dependency_parts->FindSiblings(h, i);
        int m = (i < siblings[h].size())? siblings[h][i] : -1;
        int s = (i > 0)? siblings[h][i-1] : -1;
        if (m >= 0) {
          const std::vector<int> &current_arcs = dependency_parts->FindArcs(m);
          node_scores[i].resize(current_arcs.size());
          for (int k = 0; k < current_arcs.size(); ++k) {
            int r = current_arcs[k];
            node_scores[i][k] = scores[r];
          }
        }
        if (s < 0) {
          const std::vector<int> &current_arcs = dependency_parts->FindArcs(m);
          for (int j = 0; j < sibling_parts.size(); ++j) {
            int r = sibling_parts[j];
            DependencyLabelerPartSibling *sibling =
              static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
            int modifier_label = sibling->modifier_label();
            CHECK_EQ(sibling->sibling_label(), -1);
            int k = -1;
            for (k = 0; k < current_arcs.size(); ++k) {
              DependencyLabelerPartArc *arc =
                static_cast<DependencyLabelerPartArc*>(
                  (*parts)[current_arcs[k]]);
              if (modifier_label == arc->label()) break;
            }
            CHECK_GE(k, 0);
            node_scores[i][k] += scores[r];
          }
        } else if (m < 0) {
          const std::vector<int> &previous_arcs = dependency_parts->FindArcs(s);
          for (int j = 0; j < sibling_parts.size(); ++j) {
            int r = sibling_parts[j];
            DependencyLabelerPartSibling *sibling =
              static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
            CHECK_EQ(sibling->modifier_label(), -1);
            int sibling_label = sibling->sibling_label();
            // This is very inefficient...
            int l = -1;
            for (l = 0; l < previous_arcs.size(); ++l) {
              DependencyLabelerPartArc *arc =
                static_cast<DependencyLabelerPartArc*>(
                  (*parts)[previous_arcs[l]]);
              if (sibling_label == arc->label()) break;
            }
            CHECK_GE(l, 0);
            node_scores[i-1][l] += scores[r];
          }
        } else if (m >= 0 && s >= 0) {
          const std::vector<int> &current_arcs = dependency_parts->FindArcs(m);
          const std::vector<int> &previous_arcs = dependency_parts->FindArcs(s);
          edge_scores[i-1].resize(current_arcs.size(),
                                  std::vector<double>(previous_arcs.size(),
                                                      0.0));
          for (int j = 0; j < sibling_parts.size(); ++j) {
            int r = sibling_parts[j];
            DependencyLabelerPartSibling *sibling =
              static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
            int modifier_label = sibling->modifier_label();
            int sibling_label = sibling->sibling_label();
            // This is very inefficient...
            int k = -1;
            for (k = 0; k < current_arcs.size(); ++k) {
              DependencyLabelerPartArc *arc =
                static_cast<DependencyLabelerPartArc*>(
                  (*parts)[current_arcs[k]]);
              if (modifier_label == arc->label()) break;
            }
            CHECK_GE(k, 0);
            int l = -1;
            for (l = 0; l < previous_arcs.size(); ++l) {
              DependencyLabelerPartArc *arc =
                static_cast<DependencyLabelerPartArc*>(
                  (*parts)[previous_arcs[l]]);
              if (sibling_label == arc->label()) break;
            }
            CHECK_GE(l, 0);
            edge_scores[i-1][k][l] = scores[r];
          }
        }
      }

      std::vector<int> best_path(node_scores.size(), -1);
      RunViterbi(node_scores, edge_scores, &best_path);

      for (int i = 0; i < siblings[h].size() + 1; ++i) {
        const std::vector<int> &sibling_parts =
          dependency_parts->FindSiblings(h, i);
        int m = (i < siblings[h].size())? siblings[h][i] : -1;
        int s = (i > 0)? siblings[h][i-1] : -1;
        if (m >= 0) {
          const std::vector<int> &current_arcs = dependency_parts->FindArcs(m);
          int best_label = best_path[i];
          CHECK_GE(best_label, 0);
          (*predicted_output)[current_arcs[best_label]] = 1.0;
        }

        if (s < 0) {
          const std::vector<int> &current_arcs = dependency_parts->FindArcs(m);
          for (int j = 0; j < sibling_parts.size(); ++j) {
            int r = sibling_parts[j];
            DependencyLabelerPartSibling *sibling =
              static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
            int modifier_label = sibling->modifier_label();
            CHECK_EQ(sibling->sibling_label(), -1);
            int best_current_label = best_path[i];
            DependencyLabelerPartArc *current_arc =
              static_cast<DependencyLabelerPartArc*>(
                (*parts)[current_arcs[best_current_label]]);

            if (modifier_label == current_arc->label()) {
              (*predicted_output)[r] = 1.0;
            }
          }
        } else if (m < 0) {
          const std::vector<int> &previous_arcs = dependency_parts->FindArcs(s);
          for (int j = 0; j < sibling_parts.size(); ++j) {
            int r = sibling_parts[j];
            DependencyLabelerPartSibling *sibling =
              static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
            int sibling_label = sibling->sibling_label();
            CHECK_EQ(sibling->modifier_label(), -1);
            int best_previous_label = best_path[i-1];
            DependencyLabelerPartArc *previous_arc =
              static_cast<DependencyLabelerPartArc*>(
                (*parts)[previous_arcs[best_previous_label]]);

            if (sibling_label == previous_arc->label()) {
              (*predicted_output)[r] = 1.0;
            }
          }
        } else if (m >= 0 && s >= 0) {
          const std::vector<int> &current_arcs = dependency_parts->FindArcs(m);
          const std::vector<int> &previous_arcs = dependency_parts->FindArcs(s);
          for (int j = 0; j < sibling_parts.size(); ++j) {
            int r = sibling_parts[j];
            DependencyLabelerPartSibling *sibling =
              static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
            int modifier_label = sibling->modifier_label();
            int sibling_label = sibling->sibling_label();

            int best_current_label = best_path[i];
            int best_previous_label = best_path[i-1];
            DependencyLabelerPartArc *current_arc =
              static_cast<DependencyLabelerPartArc*>(
                (*parts)[current_arcs[best_current_label]]);
            DependencyLabelerPartArc *previous_arc =
              static_cast<DependencyLabelerPartArc*>(
                (*parts)[previous_arcs[best_previous_label]]);

            if (modifier_label == current_arc->label() &&
                sibling_label == previous_arc->label()) {
              (*predicted_output)[r] = 1.0;
            }
          }
        }
      }
    }
  }
  //LOG(INFO) << "End Decode";
}

void DependencyLabelerDecoder::DecodeBasic(
    Instance *instance, Parts *parts,
    const std::vector<double> &scores,
    std::vector<double> *predicted_output) {
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  int instance_length =
    static_cast<DependencyInstanceNumeric*>(instance)->size();

  // Create copy of the scores.
  vector<double> copied_scores(scores);
  vector<int> best_labels(instance_length, -1);
  int offset_arcs, num_arcs;
  dependency_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  for (int m = 1; m < instance_length; ++m) {
    // Find the best label for each candidate arc.
    int best_label = -1;
    double best_score;
    const std::vector<int> &arcs = dependency_parts->FindArcs(m);
    for (int k = 0; k < arcs.size(); ++k) {
      if (best_label < 0 ||
          scores[arcs[k]] > best_score) {
        best_label = arcs[k];
        best_score = scores[best_label];
      }
    }
    best_labels[m] = best_label;
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  // Write the components of the predicted output that
  // correspond to the labeled parts.
  for (int m = 1; m < instance_length; ++m) {
    CHECK_GE(best_labels[m], 0);
    (*predicted_output)[best_labels[m]] = 1.0;
  }
}

double DependencyLabelerDecoder::RunViterbi(
    const std::vector<std::vector<double> > &node_scores,
    const std::vector<std::vector<std::vector<double> > > &edge_scores,
    std::vector<int> *best_path) {
  int length = node_scores.size(); // Length of the sequence.
  // To accommodate the partial scores.
  std::vector<std::vector<double> > deltas(length);
  std::vector<std::vector<int> > backtrack(length); // To backtrack.

  // Initialization.
  int num_current_labels = node_scores[0].size();
  deltas[0].resize(num_current_labels);
  backtrack[0].resize(num_current_labels);
  for (int l = 0; l < num_current_labels; ++l) {
    // The score of the first node absorbs the score of a start transition.
    deltas[0][l] = node_scores[0][l];
    backtrack[0][l] = -1; // This won't be used.
  }

  // Recursion.
  for (int i = 0; i < length - 1; ++i) {
    int num_current_labels = node_scores[i+1].size();
    deltas[i + 1].resize(num_current_labels);
    backtrack[i + 1].resize(num_current_labels);
    for (int k = 0; k < num_current_labels; ++k) {
      double best_value = -1e-12;
      int best = -1;
      // Edges from the previous position.
      int num_previous_labels = node_scores[i].size();
      for (int l = 0; l < num_previous_labels; ++l) {
        double edge_score = edge_scores[i][k][l];
        double value = deltas[i][l] + edge_score;
        if (best < 0 || value > best_value) {
          best_value = value;
          best = l;
        }
      }
      CHECK_GE(best, 0) << node_scores[i].size() << " possible tags.";

      deltas[i+1][k] = best_value + node_scores[i+1][k];
      backtrack[i+1][k] = best;
    }
  }

  // Termination.
  double best_value = -1e12;
  int best = -1;
  for (int l = 0; l < node_scores[length - 1].size(); ++l) {
    // The score of the last node had already absorbed the score of a final
    // transition.
    double value = deltas[length - 1][l];
    if (best < 0 || value > best_value) {
      best_value = value;
      best = l;
    }
  }
  CHECK_GE(best, 0);

  // Path (state sequence) backtracking.
  best_path->resize(length);
  (*best_path)[length - 1] = best;
  for (int i = length - 1; i > 0; --i) {
    (*best_path)[i - 1] = backtrack[i][(*best_path)[i]];
  }

  return best_value;
}
