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

#include "CoreferenceDecoder.h"
#include "CoreferencePart.h"
#include "CoreferencePipe.h"
#include <Eigen/Dense>
#include "logval.h"

// Define a matrix of doubles using Eigen.
typedef LogVal<double> LogValD;
namespace Eigen {
typedef Eigen::Matrix<LogValD, Dynamic, Dynamic> MatrixXlogd;
}

void CoreferenceDecoder::ComputeLinearCostFunction(
  Instance *instance,
  Parts *parts,
  const std::vector<double> &gold_output,
  std::vector<double> *p,
  double *q) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);
  int num_parts = coreference_parts->size();
  const std::vector<Mention*> &mentions = document->GetMentions();

  // The cost fucntion is = p'*z + q.
  *q = 0.0; // Always 0.0 in this case.
  p->assign(num_parts, 0.0);

  for (int j = 0; j < mentions.size(); ++j) {
    int entity_id = mentions[j]->id(); // Gold entity (-1 if not gold mention).
    // List all possible antecedents.
    const std::vector<int> &arcs = coreference_parts->FindArcParts(j);
    for (int k = 0; k < arcs.size(); ++k) {
      int r = arcs[k];
      int i = static_cast<CoreferencePartArc*>((*parts)[r])->parent_mention();
      // Gold parent entity.
      int parent_entity_id = (i >= 0) ? mentions[i]->id() : -1;
      if (!document->IsMentionAnaphoric(j) && i >= 0) {
        // Mention j starts a gold cluster, but this part doesn't.
        (*p)[r] = pipe_->GetCoreferenceOptions()->false_anaphor_cost();
      } else if (document->IsMentionAnaphoric(j) && i < 0) {
        // Mention j does not start a gold cluster, but this part does.
        (*p)[r] = pipe_->GetCoreferenceOptions()->false_new_cost();
      } else if (document->IsMentionAnaphoric(j) &&
                 entity_id != parent_entity_id) {
        // Neither mention j or this part start a gold cluster, but the
        // antecedents are different.
        CHECK_GE(i, 0);
        (*p)[r] = pipe_->GetCoreferenceOptions()->false_wrong_link_cost();
      } else {
        (*p)[r] = 0.0;
      }
    }
  }
}

void CoreferenceDecoder::DecodeCostAugmented(
  Instance *instance, Parts *parts,
  const std::vector<double> &scores,
  const std::vector<double> &gold_output,
  std::vector<double> *predicted_output,
  double *cost,
  double *loss) {
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);
  int num_parts = parts->size();

  std::vector<double> p;
  double q;
  ComputeLinearCostFunction(instance, parts, gold_output, &p, &q);

  std::vector<double> scores_cost = scores;
  for (int r = 0; r < num_parts; ++r) {
    scores_cost[r] += p[r];
  }

  Decode(instance, parts, scores_cost, predicted_output);

  *cost = q;
  for (int r = 0; r < num_parts; ++r) {
    *cost += p[r] * (*predicted_output)[r];
  }

  *loss = *cost;
  for (int r = 0; r < num_parts; ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }
}

void CoreferenceDecoder::DecodeCostAugmentedMarginals(
  Instance *instance,
  Parts *parts,
  const std::vector<double> &scores,
  const std::vector<double> &gold_output,
  std::vector<double> *predicted_output,
  double *entropy,
  double *cost,
  double *loss) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);
  int num_parts = coreference_parts->size();

  predicted_output->clear();
  predicted_output->resize(num_parts, 0.0);

  std::vector<double> p;
  double q;
  ComputeLinearCostFunction(instance, parts, gold_output, &p, &q);

  std::vector<double> scores_cost = scores;
  for (int r = 0; r < num_parts; ++r) {
    scores_cost[r] += p[r];
  }

  double log_partition_function;
  DecodeBasicMarginals(instance, parts, scores_cost, predicted_output,
                       &log_partition_function, entropy);

  *cost = q;
  for (int r = 0; r < num_parts; ++r) {
    *cost += p[r] * (*predicted_output)[r];
  }

  *loss = *cost;
  for (int r = 0; r < num_parts; ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }

  *loss += *entropy;
  if (*loss < 0.0) {
    LOG(INFO) << "Loss truncated to zero (" << *loss << ")";
    *loss = 0.0;
  }
}

void CoreferenceDecoder::Decode(Instance *instance, Parts *parts,
                                const std::vector<double> &scores,
                                std::vector<double> *predicted_output) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  const std::vector<Mention*> &mentions = document->GetMentions();
  for (int j = 0; j < mentions.size(); ++j) {
    // List all possible antecedents and pick the one with highest score.
    const std::vector<int> &arcs = coreference_parts->FindArcParts(j);
    int best_antecedent = -1;
    for (int k = 0; k < arcs.size(); ++k) {
      int r = arcs[k];
      if (best_antecedent < 0 || scores[r] > scores[best_antecedent]) {
        best_antecedent = r;
      }
    }
    CHECK_GE(best_antecedent, 0);
    (*predicted_output)[best_antecedent] = 1.0;
  }
}

void CoreferenceDecoder::DecodeMarginals(Instance *instance, Parts *parts,
                                         const std::vector<double> &scores,
                                         const std::vector<double> &gold_output,
                                         std::vector<double> *predicted_output,
                                         double *entropy,
                                         double *loss) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  int offset_arcs = 0;
  int num_arcs = coreference_parts->size();

  double log_partition_function;
  DecodeBasicMarginals(instance, parts, scores, predicted_output,
                       &log_partition_function, entropy);

  *loss = 0.0;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }

  *loss += *entropy;
  if (*loss < 0.0) {
    LOG(INFO) << "Loss truncated to zero (" << *loss << ")";
    *loss = 0.0;
  }
}

// Compute marginals and evaluate log partition function for a coreference tree
// model.
void CoreferenceDecoder::DecodeBasicMarginals(
  Instance *instance, Parts *parts,
  const std::vector<double> &scores,
  std::vector<double> *predicted_output,
  double *log_partition_function,
  double *entropy) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  *log_partition_function = 0.0;
  *entropy = 0.0;
  const std::vector<Mention*> &mentions = document->GetMentions();
  for (int j = 0; j < mentions.size(); ++j) {
    // List all possible antecedents and pick the one with highest score.
    const std::vector<int> &arcs = coreference_parts->FindArcParts(j);
    int best_antecedent = -1;
    // Find the best label for each candidate arc.
    LogValD total_score = LogValD::Zero();
    //LOG(INFO) << "num_arcs = " << arcs.size();
    for (int k = 0; k < arcs.size(); ++k) {
      int r = arcs[k];
      total_score += LogValD(scores[r], false);
      //LOG(INFO) << "scores[" << r << "] = " << scores[r];
    }
    //LOG(INFO) << "total score = " << total_score.logabs();
    *log_partition_function += total_score.logabs();
    double sum = 0.0;
    for (int k = 0; k < arcs.size(); ++k) {
      int r = arcs[k];
      LogValD marginal = LogValD(scores[r], false) / total_score;
      double marginal_value = marginal.as_float();
      (*predicted_output)[r] = marginal_value;
#if 0
      if (marginal_value > 0.0) {
        LOG(INFO) << "Marginal[" << j << ", "
          << static_cast<CoreferencePartArc*>((*parts)[r])->parent_mention()
          << "] = " << marginal_value;
      }
#endif
      if (scores[r] != -std::numeric_limits<double>::infinity()) {
        *entropy -= scores[r] * marginal_value;
      } else {
        CHECK_EQ(marginal_value, 0.0);
      }
      sum += marginal_value;
    }
    if (!NEARLY_EQ_TOL(sum, 1.0, 1e-9)) {
      LOG(INFO) << "Antecedent marginals don't sum to one: sum = " << sum;
    }
  }

  *entropy += *log_partition_function;

#if 0
  LOG(INFO) << "Log-partition function: " << *log_partition_function;
  LOG(INFO) << "Entropy: " << *entropy;
#endif
}
