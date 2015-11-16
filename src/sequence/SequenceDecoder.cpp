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

#include "SequenceDecoder.h"
#include "SequencePart.h"
#include "SequencePipe.h"
#include <iostream> // Remove this.

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

  SequenceInstanceNumeric *sentence = static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset, size;

  vector<SequenceDecoderNodeScores> node_scores(sentence->size());
  vector<SequenceDecoderEdgeScores> edge_scores(sentence->size() - 1);
  // The triplets are represented as if they were edges connecting
  // nodes with bigram states.
  vector<SequenceDecoderEdgeScores> triplet_scores;

  // Cache previous state positions in SequenceDecoderEdgeScores.
  // This will avoid expensive FindPreviousState(...) operations
  // when building the triplet scores, but it may consume
  // a lot of memory if there are too many tag bigrams.
  bool cache_edge_previous_states = false;
  if (pipe_->GetSequenceOptions()->markov_order() >= 1) {
    cache_edge_previous_states = true;
  }

  // Compute node scores from unigrams.
  sequence_parts->GetOffsetUnigram(&offset, &size);
  for (int r = 0; r < size; ++r) {
    SequencePartUnigram *unigram =
      static_cast<SequencePartUnigram*>((*parts)[offset + r]);
    node_scores[unigram->position()].AddStateScore(unigram->tag(),
                                                   scores[offset + r]);
  }
  for (int i = 0; i < sentence->size(); ++i) {
    CHECK_GE(node_scores[i].GetNumStates(), 0);
  }

  // Initialize the edge scores to zero.
  // Include all edges allowed by the dictionary that are supported
  // on the unigram states. Edges that do not have a bigram part
  // are given zero scores.
  if (pipe_->GetSequenceOptions()->markov_order() >= 1) {
    for (int i = 0; i < sentence->size() - 1; ++i) {
      edge_scores[i].SetNumCurrentStates(node_scores[i + 1].GetNumStates());
      //LOG(INFO) << "node_scores[" << i + 1  << "].GetNumStates() = " << node_scores[i + 1].GetNumStates();
      for (int j = 0; j < node_scores[i + 1].GetNumStates(); ++j) {
        int tag_id = node_scores[i + 1].GetState(j);

        //const std::vector<int> &allowed_left_tags =
        //  pipe_->GetSequenceDictionary()->GetAllowedPreviousTags(tag);

        //edge_scores[i].SetNumPreviousStates(tag_id,
        //                                    node_scores[i].GetNumStates());
        for (int k = 0; k < node_scores[i].GetNumStates(); ++k) {
          int tag_left_id = node_scores[i].GetState(k);
          if (pipe_->GetSequenceDictionary()->IsAllowedBigram(tag_left_id,
                                                              tag_id)) {
            //edge_scores[i].SetPreviousStateScore(tag_id, tag_left_id,
            //                                     tag_left_id, 0.0);
            edge_scores[i].AddPreviousStateScore(j, k, 0.0);
          }
        }
      }
    }
  }

  // Cache edge previous states.
  // We just use a vector for fast access. In cases where there are a lot of
  // bigram tags, a vector<vector<unordered_map<int, pair<int, int > > > > could
  // be better, but it was slower for POS tagging.
  // The first element of the pair encodes the index of the previous state;
  // the second element encodes the bigram index (index of the edge).
  std::vector<std::vector<std::vector<std::pair<int, int> > > >
    edge_previous_states;
  if (cache_edge_previous_states) {
    edge_previous_states.resize(sentence->size() - 1);
    for (int i = 0; i < sentence->size() - 1; ++i) {
      edge_previous_states[i].resize(node_scores[i + 1].GetNumStates());
      int bigram_index = 0;
      CHECK_EQ(node_scores[i + 1].GetNumStates(),
               edge_scores[i].GetNumCurrentStates());
      for (int tag_id = 0; tag_id < edge_scores[i].GetNumCurrentStates();
      ++tag_id) {
        edge_previous_states[i][tag_id].resize(node_scores[i].GetNumStates(),
                                               std::pair<int, int>(-1, -1));
        for (int k = 0; k < edge_scores[i].GetNumPreviousStates(tag_id); ++k) {
          int tag_left_id =
            edge_scores[i].GetPreviousStateScore(tag_id, k).first;
          edge_previous_states[i][tag_id][tag_left_id].first = k;
          edge_previous_states[i][tag_id][tag_left_id].second = bigram_index;
          //LOG(INFO) << "edge_previous_states[" << i << "][" << tag_id << "]["
          //          << tag_left_id << "].first = "<<  k;
          ++bigram_index;
        }
      }
    }
  }

  // Compute edge scores from bigrams.
  sequence_parts->GetOffsetBigram(&offset, &size);
  for (int r = 0; r < size; ++r) {
    SequencePartBigram *bigram =
      static_cast<SequencePartBigram*>((*parts)[offset + r]);
    // Get indices corresponding to tag id and left tag id.
    // TODO: Make this more efficient.
    // TODO: Cache node states to avoid many calls to FindState?
    int i = bigram->position();
    int tag_id = -1;
    int tag_left_id = -1;
    if (i < sentence->size()) {
      tag_id = node_scores[i].FindState(bigram->tag());
    }
    if (i > 0) {
      tag_left_id = node_scores[i - 1].FindState(bigram->tag_left());
    }

    if (i == 0) {
      CHECK_EQ(bigram->tag_left(), -1);
      node_scores[i].IncrementScore(tag_id, scores[offset + r]);
    } else if (i == sentence->size()) {
      CHECK_EQ(bigram->tag(), -1);
      node_scores[i - 1].IncrementScore(tag_left_id, scores[offset + r]);
    } else {
      CHECK_GE(tag_left_id, 0);
      CHECK_LT(tag_left_id, node_scores[i - 1].GetNumStates());
      CHECK_GE(tag_id, 0);
      CHECK_LT(tag_id, node_scores[i].GetNumStates());

      int k;
      if (cache_edge_previous_states) {
        k = edge_previous_states[i - 1][tag_id][tag_left_id].first;
      } else {
        k = edge_scores[i - 1].FindPreviousState(tag_id, tag_left_id);
      }
      CHECK_GE(k, 0) << tag_id << " " << tag_left_id << " " << i;
      edge_scores[i - 1].SetPreviousStateScore(tag_id, k, tag_left_id,
                                               scores[offset + r]);
    }
  }

  // Compute triplet scores from trigrams.
  if (pipe_->GetSequenceOptions()->markov_order() == 2 &&
      sentence->size() > 1) {
    triplet_scores.resize(sentence->size() - 2);
    sequence_parts->GetOffsetTrigram(&offset, &size);

    // Cache the last trigram to save time.
    SequencePartTrigram *last_trigram = NULL;
    int tag_id = -1;
    int tag_left_id = -1;
    int tag_left_left_id = -1;
    int bigram_index = -1;
    for (int r = 0; r < size; ++r) {
      SequencePartTrigram *trigram =
        static_cast<SequencePartTrigram*>((*parts)[offset + r]);
      // Get indices corresponding to tag id, left tag id, and left-left tag id.
      // TODO: Make this more efficient.
      int i = trigram->position();

      if (i < sentence->size()) {
        if (last_trigram == NULL || last_trigram->position() != i ||
            trigram->tag() != last_trigram->tag()) {
          tag_id = node_scores[i].FindState(trigram->tag());
        }
      }
      if (i > 0) {
        if (last_trigram == NULL || last_trigram->position() != i ||
            trigram->tag() != last_trigram->tag() ||
            trigram->tag_left() != last_trigram->tag_left()) {
          tag_left_id = node_scores[i - 1].FindState(trigram->tag_left());
        }
      }
      if (i > 1) {
        tag_left_left_id = node_scores[i - 2].
          FindState(trigram->tag_left_left());
      }

      if (i == 0) {
        // I don't think this ever reaches this point.
        CHECK_GE(trigram->tag(), 0);
        CHECK_EQ(trigram->tag_left(), -1);
        CHECK_EQ(trigram->tag_left_left(), -1);
      } else if (i == 1) {
        CHECK_GE(trigram->tag(), 0);
        CHECK_GE(trigram->tag_left(), 0);
        CHECK_EQ(trigram->tag_left_left(), -1);
        //edge_scores[i - 1][tag_left_id][tag_id] += scores[offset + r];
        int k;
        if (cache_edge_previous_states) {
          k = edge_previous_states[i - 1][tag_id][tag_left_id].first;
        } else {
          k = edge_scores[i - 1].FindPreviousState(tag_id,
                                                   tag_left_id);
        }
        CHECK_GE(k, 0);
        edge_scores[i - 1].IncrementScore(tag_id, k, scores[offset + r]);
      } else if (i == sentence->size()) {
        CHECK_EQ(trigram->tag(), -1);
        CHECK_GE(trigram->tag_left(), 0);
        CHECK_GE(trigram->tag_left_left(), 0);
        int k;
        if (cache_edge_previous_states) {
          k = edge_previous_states[i - 2][tag_left_id][tag_left_left_id].first;
        } else {
          k = edge_scores[i - 2].FindPreviousState(tag_left_id,
                                                   tag_left_left_id);
        }
        CHECK_GE(k, 0);
        edge_scores[i - 2].IncrementScore(tag_left_id, k, scores[offset + r]);
      } else {
        CHECK_GE(tag_left_left_id, 0);
        CHECK_LT(tag_left_left_id, node_scores[i - 2].GetNumStates());
        CHECK_GE(tag_left_id, 0);
        CHECK_LT(tag_left_id, node_scores[i - 1].GetNumStates());
        CHECK_GE(tag_id, 0);
        CHECK_LT(tag_id, node_scores[i].GetNumStates());

        if (last_trigram == NULL || last_trigram->position() != i ||
            trigram->tag() != last_trigram->tag() ||
            trigram->tag_left() != last_trigram->tag_left()) {
          // Do this in an initialization before.
          triplet_scores[i - 2].
            SetNumCurrentStates(edge_scores[i - 1].GetNumStatePairs());

          if (cache_edge_previous_states) {
            int k = edge_previous_states[i - 1][tag_id][tag_left_id].first;
            bigram_index =
              edge_previous_states[i - 1][tag_id][tag_left_id].second;
          } else {
            int k = edge_scores[i - 1].FindPreviousState(tag_id,
                                                         tag_left_id);
            CHECK_GE(k, 0);
            bigram_index = edge_scores[i - 1].GetStatePairIndex(tag_id, k);
          }
          CHECK_LT(bigram_index, triplet_scores[i - 2].GetNumCurrentStates());
        }

        int l;
        int previous_bigram_index;
        if (cache_edge_previous_states) {
          l = edge_previous_states[i - 2][tag_left_id][tag_left_left_id].first;
          previous_bigram_index =
            edge_previous_states[i - 2][tag_left_id][tag_left_left_id].second;
        } else {
          l = edge_scores[i - 2].FindPreviousState(tag_left_id,
                                                   tag_left_left_id);
          CHECK_GE(l, 0);
          // Not sure we need this; might be fine with just l.
          previous_bigram_index = edge_scores[i - 2].
            GetStatePairIndex(tag_left_id, l);
          CHECK_LT(l, triplet_scores[i - 2].GetNumPreviousStates(bigram_index));
        }

        // Do this in an initialization before.
        triplet_scores[i - 2].
          SetNumPreviousStates(bigram_index, edge_scores[i - 2].
                               GetNumPreviousStates(tag_left_id));

        triplet_scores[i - 2].SetPreviousStateScore(bigram_index,
                                                    l,
                                                    previous_bigram_index,
                                                    scores[offset + r]);
      }

      last_trigram = trigram;
    }
  }

  vector<int> best_path;
  double value;
  if (pipe_->GetSequenceOptions()->markov_order() == 2 &&
      sentence->size() > 1) {
    vector<SequenceDecoderNodeScores> transformed_node_scores;
    vector<SequenceDecoderEdgeScores> transformed_edge_scores;

    // Convert to a first order sequence model.
    ConvertToFirstOrderModel(node_scores,
                             edge_scores,
                             triplet_scores,
                             &transformed_node_scores,
                             &transformed_edge_scores);
    vector<int> transformed_best_path;
    value = RunViterbi(transformed_node_scores, transformed_edge_scores,
                       &transformed_best_path);
    //std::cout << "Value = " << value << endl;

    // Recover the best path in the original second-order model.
    RecoverBestPath(transformed_best_path, &best_path);
  } else if (pipe_->GetSequenceOptions()->markov_order() == 1) {
    value = RunViterbi(node_scores, edge_scores, &best_path);
  } else {
    value = SolveMarkovZeroOrder(node_scores, &best_path);
  }

  predicted_output->clear();
  predicted_output->resize(parts->size(), 0.0);

  int active = 0;
  for (int i = 0; i < sentence->size() + 1; ++i) {
    int tag_id = (i < sentence->size()) ? best_path[i] : -1;
    int tag_left_id = (i > 0) ? best_path[i - 1] : -1;
    int tag_left_left_id = (i > 1) ? best_path[i - 2] : -1;

    if (i < sentence->size()) {
      const vector<int> &index_unigram_parts =
        sequence_parts->FindUnigramParts(i);
      for (int k = 0; k < index_unigram_parts.size(); ++k) {
        int r = index_unigram_parts[k];
        SequencePartUnigram *unigram =
          static_cast<SequencePartUnigram*>((*parts)[r]);
        if (tag_id == unigram->tag()) {
          (*predicted_output)[r] = 1.0;
          //std::cout << "selected unigram " << i << " " << tag_id
          //          << " score=" << scores[r] << endl;
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
          //std::cout << "selected bigram " << i << " " << tag_id << " "
          //          << tag_left_id
          //          << " score=" << scores[r] << endl;
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
          //std::cout << "selected trigram " << i << " " << tag_id << " "
          //          << tag_left_id << " " << tag_left_left_id
          //          << " score=" << scores[r] << endl;
          ++active;
          found = true;
        }
      }
      CHECK(found || sentence->size() == 1);
    }
  }
}

// TODO: we need also to specify a map between bigram codes and their two tags.
void SequenceDecoder::RecoverBestPath(const std::vector<int> &best_path,
                                      std::vector<int> *transformed_best_path) {
  int length = best_path.size() + 1;
  transformed_best_path->clear();
  transformed_best_path->resize(length, -1);

  for (int i = 0; i < length - 1; ++i) {
    int bigram_label = best_path[i];
    int left_tag, tag;
    pipe_->GetSequenceDictionary()->GetBigramTags(bigram_label,
                                                  &left_tag,
                                                  &tag);
    if (i == 0) {
      (*transformed_best_path)[i] = left_tag;
    } else {
      CHECK_EQ((*transformed_best_path)[i], left_tag);
    }
    (*transformed_best_path)[i + 1] = tag;

    CHECK_GE((*transformed_best_path)[i + 1], 0);
    //std::cout << (*transformed_best_path)[i] << " ";
  }
  //std::cout << (*transformed_best_path)[length-1] << std::endl;
}

// TODO: edge_scores will need to be in the same format as transformed_edge_scores,
// and we'll also to specify a map between bigram codes and their two tags.
// TODO: implement a suitable structure for the triplets.
void SequenceDecoder::ConvertToFirstOrderModel(
  const std::vector<SequenceDecoderNodeScores> &node_scores,
  const std::vector<SequenceDecoderEdgeScores> &edge_scores,
  const std::vector<SequenceDecoderEdgeScores> &triplet_scores,
  std::vector<SequenceDecoderNodeScores> *transformed_node_scores,
  std::vector<SequenceDecoderEdgeScores> *transformed_edge_scores) {
  int length = node_scores.size();

  // The transformed node scores will have one less element.
  transformed_node_scores->resize(length - 1);
  for (int i = 0; i < length - 1; ++i) {
    // Position i.
    int num_bigram_states = edge_scores[i].GetNumStatePairs();

    (*transformed_node_scores)[i].SetNumStates(num_bigram_states);
    int bigram_index = 0;
    for (int j = 0; j < edge_scores[i].GetNumCurrentStates(); ++j) {
      // Tag j at position i+1.
      int tag = node_scores[i + 1].GetState(j);
      const vector<std::pair<int, double> > &previous_state_scores =
        edge_scores[i].GetAllPreviousStateScores(j);

      for (int t = 0; t < previous_state_scores.size(); ++t) {
        // Tag k at position i.
        int k = previous_state_scores[t].first;
        double score = previous_state_scores[t].second;
        int left_tag = node_scores[i].GetState(k);
        int bigram_label = pipe_->GetSequenceDictionary()->
          GetBigramLabel(left_tag, tag);
        if (i == length - 2) {
          (*transformed_node_scores)[i].
            SetStateScore(bigram_index, bigram_label,
                          score + node_scores[i + 1].GetScore(j) +
                          node_scores[i].GetScore(k));
        } else {
          (*transformed_node_scores)[i].
            SetStateScore(bigram_index, bigram_label,
                          score + node_scores[i].GetScore(k));
        }
        ++bigram_index;
      }
    }
  }

  // The transformed edge scores will have one less element.
  // Just copy the triplet structure and adjust the scores.
  transformed_edge_scores->resize(length - 2);
  for (int i = 0; i < length - 2; ++i) {
    // Position i.
    int num_current_states = triplet_scores[i].GetNumCurrentStates();
    (*transformed_edge_scores)[i].SetNumCurrentStates(num_current_states);
    for (int t = 0; t < num_current_states; ++t) {
      int num_previous_states = triplet_scores[i].GetNumPreviousStates(t);
      const std::vector<std::pair<int, double> > previous_state_scores =
        triplet_scores[i].GetAllPreviousStateScores(t);
      (*transformed_edge_scores)[i].
        SetNumPreviousStates(t, previous_state_scores.size());
      for (int j = 0; j < previous_state_scores.size(); ++j) {
        (*transformed_edge_scores)[i].
          SetPreviousStateScore(t, j, previous_state_scores[j].first,
                                previous_state_scores[j].second);
      }
    }
  }
}

double SequenceDecoder::SolveMarkovZeroOrder(const std::vector<SequenceDecoderNodeScores> &node_scores,
                                             std::vector<int> *best_path) {
  int length = node_scores.size(); // Length of the sequence.
  std::vector<int> temp_path(length);
  std::vector<double> temp_scores(length);

  for (int i = 0; i < length; ++i) {
    int num_current_labels = node_scores[i].GetNumStates();

    double current_score;
    double best_value = -1e-12;
    int best = -1;
    for (int l = 0; l < num_current_labels; ++l) {
      current_score = node_scores[i].GetScore(l);
      if (best < 0 || current_score > best_value) {
        best_value = current_score;
        temp_scores[i] = best_value;
        best = l;
        temp_path[i] = best;
      }
    }
    CHECK_GE(best, 0) << node_scores[i].GetNumStates() << " possible tags.";
  }

  // Termination.
  // Convert to the actual node states.
  best_path->resize(length);
  double best_value = 0.0;
  for (int i = 0; i < length; ++i) {
    (*best_path)[i] = node_scores[i].GetState(temp_path[i]);
    best_value += temp_scores[i];
  }
  return best_value;
}

// TODO(atm): adapt description.
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
double SequenceDecoder::RunViterbi(const std::vector<SequenceDecoderNodeScores>
                                   &node_scores,
                                   const std::vector<SequenceDecoderEdgeScores>
                                   &edge_scores,
                                   std::vector<int> *best_path) {
  int length = node_scores.size(); // Length of the sequence.
  // To accommodate the partial scores.
  std::vector<std::vector<double> > deltas(length);
  std::vector<std::vector<int> > backtrack(length); // To backtrack.

  // Initialization.
  int num_current_labels = node_scores[0].GetNumStates();
  deltas[0].resize(num_current_labels);
  backtrack[0].resize(num_current_labels);
  for (int l = 0; l < num_current_labels; ++l) {
    // The score of the first node absorbs the score of a start transition.
    deltas[0][l] = node_scores[0].GetScore(l);
    backtrack[0][l] = -1; // This won't be used.
  }

  // Recursion.
  for (int i = 0; i < length - 1; ++i) {
    int num_current_labels = node_scores[i + 1].GetNumStates();
    deltas[i + 1].resize(num_current_labels);
    backtrack[i + 1].resize(num_current_labels);
    for (int k = 0; k < num_current_labels; ++k) {
      double best_value = -1e-12;
      int best = -1;
      // Edges from the previous position.
      const std::vector<std::pair<int, double> > &previous_state_scores =
        edge_scores[i].GetAllPreviousStateScores(k);
      for (std::vector<std::pair<int, double> >::const_iterator it =
           previous_state_scores.begin();
           it != previous_state_scores.end();
           ++it) {
        int l = it->first;
        double edge_score = it->second;
        double value = deltas[i][l] + edge_score;
        if (best < 0 || value > best_value) {
          best_value = value;
          best = l;
        }
      }
      CHECK_GE(best, 0) << node_scores[i].GetNumStates() << " possible tags.";

      deltas[i + 1][k] = best_value + node_scores[i + 1].GetScore(k);
      backtrack[i + 1][k] = best;
    }
  }

  // Termination.
  double best_value = -1e12;
  int best = -1;
  for (int l = 0; l < node_scores[length - 1].GetNumStates(); ++l) {
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

  // Convert to the actual node states.
  for (int i = 0; i < length; ++i) {
    (*best_path)[i] = node_scores[i].GetState((*best_path)[i]);
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

    IloNumVar::Type var_type = relax ? ILOFLOAT : ILOBOOL;
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
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i - 1);
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
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i - 1);
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
            const vector<int> &unigrams = sequence_parts->FindUnigramParts(i - 2);
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
    cplex.setParam(IloCplex::TiLim, 60.0); // 60 seconds
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
          if (val*(1 - val) > 0.001) {
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
  } catch (IloException& ex) {
    cout << "Error: " << ex << endl;
    cerr << "Error: " << ex << endl;
  } catch (...) {
    cout << "Unspecified error" << endl;
    cerr << "Unspecified error" << endl;
  }

  return 0.0;
}
#endif
