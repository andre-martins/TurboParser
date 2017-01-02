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

#include "Dictionary.h"
#include "EntityDecoder.h"
#include "SequencePart.h"
#include "EntityPipe.h"
#include <iostream> // Remove this.

DEFINE_double(ner_train_cost_false_positives, 0.5,
              "Cost for 'false positives' -- penalises recall and favours precision in BIO tagging.");
DEFINE_double(ner_train_cost_false_negatives, 0.5,
              "Cost for 'false negatives' -- penalises precision and favours recall in BIO tagging.");

void EntityDecoder::DecodeCostAugmented(Instance *instance, Parts *parts,
                                          const vector<double> &scores,
                                          const vector<double> &gold_output,
                                          vector<double> *predicted_output,
                                          double *cost,
                                          double *loss) {
  //LOG(INFO) << "  [DEBUG]  @EntityDecoder::DecodeCostAugmented";  
  //return SequenceDecoder::DecodeCostAugmented(instance, parts, scores, gold_output,
  //                                      predicted_output, cost, loss);

  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset_unigrams, num_unigrams;
  //int offset_bigrams, num_bigrams;
  //int offset_trigrams, num_trigrams;

  sequence_parts->GetOffsetUnigram(&offset_unigrams, &num_unigrams);
  //sequence_parts->GetOffsetBigram(&offset_bigrams, &num_bigrams);
  //sequence_parts->GetOffsetTrigram(&offset_trigrams, &num_trigrams);

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
  double a = FLAGS_ner_train_cost_false_positives;
  // Penalty for predicting 0 when it is 1 (FN).
  double b = FLAGS_ner_train_cost_false_negatives;
  //double b = 1 - a;

  // p = 0.5-z0, q = 0.5'*z0, loss = p'*z + q
  double q = 0.0;
  vector<double> p(num_unigrams, 0.0);

  vector<double> scores_cost = scores;
  //SequencePartUnigram *sequence_unigram_parts = static_cast<SequencePartUnigram*>(parts);
  for (int r = 0; r < num_unigrams; ++r) {
    
    SequenceDictionary *dictionary;
    dictionary = static_cast<SequenceDictionary*>(pipe_->GetSequenceDictionary());
    SequencePartUnigram *unigram_part = (static_cast<SequencePartUnigram*>((*sequence_parts)[r]));
    int tag = unigram_part->tag();
    const std::string & tag_name = dictionary->GetTagName(tag);
    EntityOptions *entity_options = static_cast<EntityPipe*>(pipe_)->GetEntityOptions();
    /*if (entity_options->tagging_scheme() == EntityTaggingSchemes::BIO){
    // Independent of TaggingSchemme 
    }*/
    if (tag_name[0] != 'O'){ // if inside (not outside)
      p[r] = a - (a+b)*gold_output[offset_unigrams + r];
      q += b*gold_output[offset_unigrams + r];
    } else {
      p[r] = 0;
    }
    scores_cost[offset_unigrams + r] += p[r];    
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


#if 0
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset_unigrams, num_unigrams;

  sequence_parts->GetOffsetUnigram(&offset_unigrams, &num_unigrams);

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
  // double a = FLAGS_ner_train_cost_false_positives;
  // Penalty for predicting 0 when it is 1 (FN).
  // double b = FLAGS_ner_train_cost_false_negatives;

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
#endif
}

void EntityDecoder::Decode(Instance *instance, Parts *parts,
                             const vector<double> &scores,
                             vector<double> *predicted_output) {

  //LOG(INFO) << "  [DEBUG]  @EntityDecoder::Decode";  
  return SequenceDecoder::Decode(instance, parts, scores, predicted_output);

#if 0                                 
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
#endif
}
