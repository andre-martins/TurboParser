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

#include "SequenceDecoder.h"
#include "SequencePart.h"
#include "SequencePipe.h"

void SequenceDecoder::DecodeCostAugmented(Instance *instance, Parts *parts,
                                          const vector<double> &scores,
                                          const vector<double> &gold_output,
                                          vector<double> *predicted_output,
                                          double *cost,
                                          double *loss) {
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset_unigrams, num_unigrams;

  sequence_parts->GetOffsetUnigram(&offset_unigrams, &num_unigrams);

  // p = 0.5-z0, q = 0.5'*z0, loss = p'*z + q
  double q = 0.0;
  vector<double> p(num_unigrams, 0.0);

  vector<double> scores_cost = scores;
  for (int r = 0; r < num_unigrams; ++r) {
    p[r] = 0.5 - gold_output[offset_unigrams + r];
    scores_cost[offset_unigrams + r] += p[r];
    q += 0.5*gold_output[offset_unigrams + r];
  }

  Decode(instance, parts, scores_cost, predicted_output);

  *cost = q;
  for (int r = 0; r < num_unigrams; ++r) {
    *cost += p[r] * (*predicted_output)[offset_unigrams + r];
  }

  *loss = *cost;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }
}

void SequenceDecoder::Decode(Instance *instance, Parts *parts,
                             const vector<double> &scores,
                             vector<double> *predicted_output) {
#ifdef USE_CPLEX
  return DecodeCPLEX(instance, parts, scores, false, predicted_output);
#endif

  SequenceInstanceNumeric *sentence =
      static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset, size;

  vector<vector<int> > node_tags(sentence->size());
  vector<vector<double> > node_scores(sentence->size());
  vector<vector<vector<double> > > edge_scores(sentence->size() - 1);
  vector<vector<vector<vector<double> > > > triplet_scores;

  // Compute node scores from unigrams.
  sequence_parts->GetOffsetUnigram(&offset, &size);
  for (int r = 0; r < size; ++r) {
    SequencePartUnigram *unigram =
        static_cast<SequencePartUnigram*>((*parts)[offset + r]);
    node_tags[unigram->position()].push_back(unigram->tag());
    node_scores[unigram->position()].push_back(scores[offset + r]);
  }
  for (int i = 0; i < sentence->size(); ++i) {
    CHECK_GE(node_tags[i].size(), 0);
    CHECK_GE(node_scores[i].size(), 0);
  }

  // Compute edge scores from bigrams.
  sequence_parts->GetOffsetBigram(&offset, &size);
  for (int r = 0; r < size; ++r) {
    SequencePartBigram *bigram =
        static_cast<SequencePartBigram*>((*parts)[offset + r]);
    // Get indices corresponding to tag if and left tag id.
    // TODO: Make this more efficient.
    int i = bigram->position();
    int tag_id = -1;
    int tag_left_id = -1;
    if (i < sentence->size()) {
      for (int k = 0; k < node_tags[i].size(); ++k) {
        if (node_tags[i][k] == bigram->tag()) {
          tag_id = k;
          break;
        }
      }
    }
    if (i > 0) {
      for (int k = 0; k < node_tags[i - 1].size(); ++k) {
        if (node_tags[i - 1][k] == bigram->tag_left()) {
          tag_left_id = k;
          break;
        }
      }
    }

    if (i == 0) {
      CHECK_EQ(bigram->tag_left(), -1);
      node_scores[i][tag_id] += scores[offset + r];
    } else if (i == sentence->size()) {
      CHECK_EQ(bigram->tag(), -1);
      node_scores[i - 1][tag_left_id] += scores[offset + r];
    } else {
      //LOG(INFO) << node_tags[i - 1].size();
      //LOG(INFO) << node_tags[i].size();
      //LOG(INFO) << i << " " << sentence->size();
      CHECK_GE(tag_left_id, 0);
      CHECK_LT(tag_left_id, node_tags[i - 1].size());
      CHECK_GE(tag_id, 0);
      CHECK_LT(tag_id, node_tags[i].size());

      edge_scores[i - 1].resize(node_tags[i - 1].size());
      edge_scores[i - 1][tag_left_id].resize(node_tags[i].size(), 0.0);
      edge_scores[i - 1][tag_left_id][tag_id] = scores[offset + r];
    }
  }

  // Compute triplet scores from trigrams.
  vector<vector<vector<std::pair<int, double>>>> transformed_edge_scores;
  if (pipe_->GetSequenceOptions()->markov_order() == 2 &&
      sentence->size() > 1) {
	transformed_edge_scores.clear();
	transformed_edge_scores.resize(sentence->size() - 2);
	triplet_scores.resize(sentence->size() - 2);
    sequence_parts->GetOffsetTrigram(&offset, &size);
	SequencePartTrigram *last_trigram = NULL;
	int tag_id = -1;
	int tag_left_id = -1;
	int tag_left_left_id = -1;
   for (int r = 0; r < size; ++r) {
      //LOG(INFO) << r << " " << size;
      SequencePartTrigram *trigram =
          static_cast<SequencePartTrigram*>((*parts)[offset + r]);
      // Get indices corresponding to tag id, left tag id, and left-left tag id.
      // TODO: Make this more efficient.
	  // don't kow if the tags are ordered, if they are the following loops can be further optimized
      int i = trigram->position();
	  

      if (i < sentence->size()) {
		if (last_trigram == NULL || last_trigram->position() != i || trigram->tag() != last_trigram->tag()) {
			for (int k = 0; k < node_tags[i].size(); ++k) {
			  if (node_tags[i][k] == trigram->tag()) {
				tag_id = k;
				break;
			  }
			}
        }
      }
      if (i > 0) {
		if (last_trigram == NULL || last_trigram->position() != i || trigram->tag() != last_trigram->tag() || trigram->tag_left() != last_trigram->tag_left()) 
		{
			for (int k = 0; k < node_tags[i - 1].size(); ++k) {
			  if (node_tags[i - 1][k] == trigram->tag_left()) {
				tag_left_id = k;
				break;
			  }
			}
		}
      }
      if (i > 1) {
        for (int k = 0; k < node_tags[i - 2].size(); ++k) {
          if (node_tags[i - 2][k] == trigram->tag_left_left()) {
            tag_left_left_id = k;
            break;
          }
        }
      }
	  last_trigram = trigram;
      if (i == 0) {
        // I don't think this ever reaches this point.
        CHECK_EQ(trigram->tag_left(), -1);
        CHECK_EQ(trigram->tag_left_left(), -1);
      } else if (i == 1) {
        CHECK_EQ(trigram->tag_left_left(), -1);
        edge_scores[i - 1][tag_left_id][tag_id] += scores[offset + r];
      } else if (i == sentence->size()) {
        CHECK_EQ(trigram->tag(), -1);
        edge_scores[i - 2][tag_left_left_id][tag_left_id] += scores[offset + r];
      } else {
        CHECK_GE(tag_left_left_id, 0);
        CHECK_LT(tag_left_left_id, node_tags[i - 2].size());
        CHECK_GE(tag_left_id, 0);
        CHECK_LT(tag_left_id, node_tags[i - 1].size());
        CHECK_GE(tag_id, 0);
        CHECK_LT(tag_id, node_tags[i].size());
		int s = tag_left_left_id * node_tags[i - 1].size() + tag_left_id;
		int t = tag_left_id * node_tags[i].size() + tag_id;
		(transformed_edge_scores)[i-2].resize (node_tags[i-1].size() * node_tags[i].size());
		//(transformed_edge_scores)[i-2][t].reserve (node_tags[i - 2].size());
		(transformed_edge_scores)[i-2][t].push_back(std::pair<int, double> (s, scores[offset + r]));
        /*triplet_scores[i - 2].resize(node_tags[i - 2].size());
        triplet_scores[i - 2][tag_left_left_id].resize(node_tags[i - 1].size());
        triplet_scores[i - 2][tag_left_left_id][tag_left_id].resize(
            node_tags[i].size(), 0.0);
        triplet_scores[i - 2][tag_left_left_id][tag_left_id][tag_id] =
            scores[offset + r];*/
      }
    }
  }

  vector<int> best_path;
  double value;
  if (pipe_->GetSequenceOptions()->markov_order() == 2 &&
      sentence->size() > 1) {
    vector<vector<double> > transformed_node_scores;

    // Convert to a first order sequence model.
    ConvertToFirstOrderModel(node_scores,
                             edge_scores,
                             triplet_scores, node_tags,
                             &transformed_node_scores
                             );
    vector<int> transformed_best_path;
    value = RunViterbi(transformed_node_scores, transformed_edge_scores,
        &transformed_best_path);

    // Recover the best path in the original second-order model.
    RecoverBestPath(transformed_best_path, edge_scores, &best_path);
  } else {
    value = RunViterbi(node_scores, edge_scores, &best_path);
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  int active = 0;
  for (int i = 0; i < sentence->size() + 1; ++i) {
    int tag_id = (i < sentence->size())? node_tags[i][best_path[i]] : -1;
    int tag_left_id = (i > 0)? node_tags[i - 1][best_path[i - 1]] : -1;
    int tag_left_left_id = (i > 1)? node_tags[i - 2][best_path[i - 2]] : -1;

    if (i < sentence->size()) {
      const vector<int> &index_unigram_parts =
        sequence_parts->FindUnigramParts(i);
      for (int k = 0; k < index_unigram_parts.size(); ++k) {
        int r = index_unigram_parts[k];
        SequencePartUnigram *unigram =
            static_cast<SequencePartUnigram*>((*parts)[r]);
        if (tag_id == unigram->tag()) {
          (*predicted_output)[r] = 1.0;
          ++active;
        }
      }
    }

    if (pipe_->GetSequenceOptions()->markov_order() >= 1) {
      const vector<int> &index_bigram_parts = 
        sequence_parts->FindBigramParts(i);
      for (int k = 0; k < index_bigram_parts.size(); ++k) {
        int r = index_bigram_parts[k];
        SequencePartBigram *bigram =
            static_cast<SequencePartBigram*>((*parts)[r]);
        if (tag_id == bigram->tag() && tag_left_id == bigram->tag_left()) {
          (*predicted_output)[r] = 1.0;
          ++active;
        }
      }
    }

    bool found = false;
    if (pipe_->GetSequenceOptions()->markov_order() >= 2 && i > 0) {
      const vector<int> &index_trigram_parts = 
        sequence_parts->FindTrigramParts(i);
      for (int k = 0; k < index_trigram_parts.size(); ++k) {
        int r = index_trigram_parts[k];
        SequencePartTrigram *trigram =
            static_cast<SequencePartTrigram*>((*parts)[r]);
        if (tag_id == trigram->tag() && tag_left_id == trigram->tag_left()
            && tag_left_left_id == trigram->tag_left_left()) {
          (*predicted_output)[r] = 1.0;
          ++active;
          found = true;
        }
      }
      CHECK(found || sentence->size() == 1);
    }
  }
}

void SequenceDecoder::RecoverBestPath(
    const vector<int> &best_path,
    const vector<vector<vector<double> > > &edge_scores,
    vector<int> *transformed_best_path) {
  int length = best_path.size() + 1;
  //LOG(INFO) << length;
  transformed_best_path->clear();
  transformed_best_path->resize(length, -1);
  for (int i = 0; i < length - 1; ++i) {
    int bigram_tag = best_path[i];
    bool found = false;
    int s = 0;
    for (int j = 0; j < edge_scores[i].size(); ++j) {
      CHECK_GT(edge_scores[i][j].size(), 0);
      for (int k = 0; k < edge_scores[i][j].size(); ++k, ++s) {
        if (s == bigram_tag) {
          if (i > 0) {
            CHECK_EQ((*transformed_best_path)[i], j);
          }
          (*transformed_best_path)[i] = j;
          (*transformed_best_path)[i+1] = k;
          found = true;
          CHECK_GE(k, 0);
          break;
        }
      }
      if (found) break;
    }
    CHECK_EQ(s, bigram_tag);
    CHECK_GE((*transformed_best_path)[i+1], 0);
  }
}

void SequenceDecoder::ConvertToFirstOrderModel(
    const vector<vector<double> > &node_scores,
    const vector<vector<vector<double> > > &edge_scores,
    const vector<vector<vector<vector<double> > > > &triplet_scores,
	const vector<vector<int> > &node_tags,
    vector<vector<double> > *transformed_node_scores
    ) {

  int length = node_scores.size();

  // The transformed node scores will have one less element.
  transformed_node_scores->resize(length - 1);
  for (int i = 0; i < length - 1; ++i) {
    // Position i.
    for (int j = 0; j < edge_scores[i].size(); ++j) {
      // Tag j at position i.
      for (int k = 0; k < edge_scores[i][j].size(); ++k) {
        // Tag k at position i+1.
        if (i == length - 2) {
          (*transformed_node_scores)[i].push_back(edge_scores[i][j][k] +
                                                  node_scores[i][j] +
                                                  node_scores[i+1][k]);
        } else {
          (*transformed_node_scores)[i].push_back(edge_scores[i][j][k] +
                                                  node_scores[i][j]);
        }
      }
    }
  }

  // The transformed edge scores will have one less element.
  /*transformed_edge_scores->resize(length - 2);
  for (int i = 0; i < length - 2; ++i) {
    // Position i.
	 (*transformed_edge_scores)[i].clear();
    int s = 0;
    for (int j = 0; j < edge_scores[i].size(); ++j) {
      // Tag j at position i.
      int t = 0;
      for (int k = 0; k < edge_scores[i][j].size(); ++k, ++s) {
        // Tag k at position i+1.
        for (int l = 0; l < node_scores[i+2].size(); ++l, ++t) {
          // Tag l at position i+2.
		
          (*transformed_edge_scores)[i][(s << 16) | t] = triplet_scores[i][j][k][l];
		
			
        }
      }
    }
  }*/
}

// Computes the Viterbi path of a sequence model.
// Note: the initial and final transitions are incorporated in the node scores.
// 1) node_scores is a vector whose size is the length of the sequence, where
// each position contains a vector of scores for each label.
// 2) edge_scores is a vector whose size is the sequence length minus one, where
// each position contains a 2D array of scores for each pair of labels (the
// first index is the current label and the second index is the next label).
// 3) best_path is a vector given as output, containing indices that describe
// the path maximizing the score.
// 4) the returned value is the optimal score.
double SequenceDecoder::RunViterbi(const vector<vector<double> > &node_scores,
                                   const vector<vector<vector<double > > >
                                     &edge_scores,
                                   vector<int> *best_path) {
  int length = node_scores.size(); // Length of the sequence.
  vector<vector<double> > deltas(length); // To accommodate the partial scores.
  vector<vector<int> > backtrack(length); // To backtrack.

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
      for (int l = 0; l < node_scores[i].size(); ++l) {
        double value;
		if (l < edge_scores[i].size() && k < edge_scores[i][l].size()  )
			value = deltas[i][l] + edge_scores[i][l][k];
		else
			value = deltas[i][l] + LOG_ZERO;
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
double SequenceDecoder::RunViterbi(const vector<vector<double> > &node_scores,
                                    const vector<vector<vector<std::pair<int, double>>>>
                                     &edge_scores,
                                   vector<int> *best_path) {
  int length = node_scores.size(); // Length of the sequence.
  vector<vector<double> > deltas(length); // To accommodate the partial scores.
  vector<vector<int> > backtrack(length); // To backtrack.

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
	  vector<std::pair<int, double>>::const_iterator it ;
      for (it = edge_scores[i][k].begin(); it != edge_scores[i][k].end(); it++) {
        
		
		double value = deltas[i][it->first] + it->second;
        if (best < 0 || value > best_value) {
          best_value = value;
          best = it->first;
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

#ifdef USE_CPLEX

#include <limits.h>
#define ILOUSESTL
#include <ilcplex/ilocplex.h>
ILOSTLBEGIN
double SequenceDecoder::DecodeCPLEX(Instance *instance, Parts *parts,
    const vector<double> &scores,
    bool relax,
    vector<double> *predicted_output) {
  SequenceInstanceNumeric *sentence =
  static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset, size;

  try {
    IloEnv env;
    IloModel mod(env);
    IloCplex cplex(mod);

    ///////////////////////////////////////////////////////////////////
    // Variable definition
    ///////////////////////////////////////////////////////////////////

    IloNumVar::Type var_type = relax? ILOFLOAT : ILOBOOL;
    IloNumVarArray z(env, parts->size(), 0.0, 1.0, var_type);

    ///////////////////////////////////////////////////////////////////
    // Objective
    ///////////////////////////////////////////////////////////////////

    IloExpr expr_obj(env);
    for (int r = 0; r < parts->size(); ++r) {
      expr_obj += -scores[r] * z[r];
    }
    IloObjective obj(env, expr_obj, IloObjective::Minimize);
    mod.add(obj);
    expr_obj.end();

    ///////////////////////////////////////////////////////////////////
    // Constraints
    ///////////////////////////////////////////////////////////////////

    IloExpr expr(env);

    // One label per word
    // sum_l (z_il) = 1 for all i
    for (int i = 0; i < sentence->size(); ++i) {
      expr = IloExpr(env);
      const vector<int> &unigrams = sequence_parts->FindUnigramParts(i);
      for (int k = 0; k < unigrams.size(); ++k) {
        int r = unigrams[k];
        expr += z[r];
      }
      mod.add(expr == 1.0);
      expr.end();
    }

    ///////////////////////////////////////////////////////////////
    // Add global constraints (if any)
    ///////////////////////////////////////////////////////////////

    if (pipe_->GetSequenceOptions()->markov_order() >= 1) {
      // z_illprev <= z_il for each i,l,lprev
      // z_illprev <= z_(i-1)lprev for each i,l,lprev
      // z_illprev >= z_il + z_(i-1)lprev - 1 for each i,l,lprev
      for (int i = 0; i < sentence->size() + 1; ++i) {
        const vector<int> &index_bigram_parts =
          sequence_parts->FindBigramParts(i);
        for (int k = 0; k < index_bigram_parts.size(); ++k) {
          int r = index_bigram_parts[k];
          SequencePartBigram *bigram =
              static_cast<SequencePartBigram*>((*parts)[r]);

          // Get unigram for current tag.
          int r1 = -1;
          if (i < sentence->size()) {
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i);
            for (int k = 0; k < unigrams.size(); ++k) {
              r1 = unigrams[k];
              SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r1]);
              if (unigram->tag() == bigram->tag()) break;
            }
            CHECK_GE(r1, 0);
          }

          // Get unigram for previous tag.
          int r2 = -1;
          if (i > 0) {
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i-1);
            for (int k = 0; k < unigrams.size(); ++k) {
              r2 = unigrams[k];
              SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r2]);
              if (unigram->tag() == bigram->tag_left()) break;
            }
            CHECK_GE(r2, 0);
          }

          if (i == 0) {
            mod.add(z[r] - z[r1] == 0.0);
          } else if (i == sentence->size()) {
            mod.add(z[r] - z[r2] == 0.0);
          } else {
            mod.add(z[r] - z[r1] <= 0.0);
            mod.add(z[r] - z[r2] <= 0.0);
            mod.add(z[r] - z[r1] - z[r2] >= -1.0);
          }
        }
      }
    }

    if (pipe_->GetSequenceOptions()->markov_order() >= 1 &&
        sentence->size() > 1) {
      // z_illprevlprevprev <= z_il for each i,l,lprev,lprevprev
      // z_illprevlprevprev <= z_(i-1)lprev for each i,l,lprev,lprevprev
      // z_illprevlprevprev <= z_(i-2)lprevlprev for each i,l,lprev,lprevprev
      // z_illprevlprevprev >= z_il + z_(i-1)lprev + z_(i-2)lprevlprev - 2 for each i,l,lprev
      for (int i = 1; i < sentence->size() + 1; ++i) {
        const vector<int> &index_trigram_parts =
          sequence_parts->FindTrigramParts(i);
        for (int k = 0; k < index_trigram_parts.size(); ++k) {
          int r = index_trigram_parts[k];
          SequencePartTrigram *trigram =
              static_cast<SequencePartTrigram*>((*parts)[r]);

          // Get unigram for current tag.
          int r1 = -1;
          if (i < sentence->size()) {
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i);
            for (int k = 0; k < unigrams.size(); ++k) {
              r1 = unigrams[k];
              SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r1]);
              if (unigram->tag() == trigram->tag()) break;
            }
            CHECK_GE(r1, 0);
          }

          // Get unigram for previous tag.
          int r2 = -1;
          if (i > 0) {
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i-1);
            for (int k = 0; k < unigrams.size(); ++k) {
              r2 = unigrams[k];
              SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r2]);
              if (unigram->tag() == trigram->tag_left()) break;
            }
            CHECK_GE(r2, 0);
          }

          // Get unigram for before previous tag.
          int r3 = -1;
          if (i > 1) {
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i-2);
            for (int k = 0; k < unigrams.size(); ++k) {
              r3 = unigrams[k];
              SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r3]);
              if (unigram->tag() == trigram->tag_left_left()) break;
            }
            CHECK_GE(r3, 0);
          }

          if (i == 1) {
            mod.add(z[r] - z[r1] <= 0.0);
            mod.add(z[r] - z[r2] <= 0.0);
            mod.add(z[r] - z[r1] - z[r2] >= -1.0);
          } else if (i == sentence->size()) {
            mod.add(z[r] - z[r2] <= 0.0);
            mod.add(z[r] - z[r3] <= 0.0);
            mod.add(z[r] - z[r2] - z[r3] >= -1.0);
          } else {
            mod.add(z[r] - z[r1] <= 0.0);
            mod.add(z[r] - z[r2] <= 0.0);
            mod.add(z[r] - z[r3] <= 0.0);
            mod.add(z[r] - z[r1] - z[r2] - z[r3] >= -2.0);
          }
        }
      }
    }

    ///////////////////////////////////////////////////////////////////
    // Solve
    ///////////////////////////////////////////////////////////////////
    //CPXsetdblparam(env, CPX_PARAM_TILIM, 120);
    double value = 0.0;
    cplex.setParam (IloCplex::TiLim, 60.0); // 60 seconds
    if (cplex.solve()) {
      cplex.out() << "Solution status = " << cplex.getStatus() << endl;
      cplex.out() << "Solution value = " << cplex.getObjValue() << endl;
      value = -cplex.getObjValue();

      IloNumArray zOpt(env, parts->size());
      double thres = 0.5;

      cplex.getValues(z, zOpt);

      predicted_output->resize(parts->size());
      for (int r = 0; r < parts->size(); ++r) {
        (*predicted_output)[r] = zOpt[r];
        if (relax) {
          double val = (*predicted_output)[r];
          if (val*(1-val) > 0.001) {
            cout << "Fractional value!" << endl;
          }
        }
      }
    } else {
      cout << "Could not solve the LP!" << endl;
      if (cplex.getCplexStatus() == IloCplex::AbortTimeLim) {
        cout << "Time out!" << endl;
        cplex.out() << "Solution best value = " << cplex.getBestObjValue() << endl;
      }
    }

    env.end();

    return value;
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

  return 0.0;
}
#endif

