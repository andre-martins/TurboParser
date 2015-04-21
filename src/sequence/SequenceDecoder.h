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

#ifndef SEQUENCEDECODER_H_
#define SEQUENCEDECODER_H_

#include "Decoder.h"

class SequencePipe;

class SequenceDecoderNodeScores {
 public:
  SequenceDecoderNodeScores() {}
  virtual ~SequenceDecoderNodeScores() {}

  // Get the number of states.
  int GetNumStates() const { return scores_.size(); }

  // Set the number of states.
  void SetNumStates(int num_states) { scores_.resize(num_states); }

  // Get the score of a state by its zero-based index.
  double GetScore(int state_index) const { return scores_[state_index].second; }

  // Get a state by its zero-based index.
  int GetState(int state_index) const { return scores_[state_index].first; }

  // Set the score of a state (assume SetNumStates(...) has been called before).
  void SetStateScore(int state_index, int state, double score) {
    scores_[state_index].first = state;
    scores_[state_index].second = score;
  }

  // Increment the score of a state by its zero-based index.
  void IncrementScore(int state_index, double score) {
    scores_[state_index].second += score;
  }

  // Add the score of a state.
  void AddStateScore(int state, double score) {
    scores_.push_back(std::pair<int, double>(state, score));
  }

  // Finds the index corresponding to a given state. Returns -1 if not found.
  // Note: this function is inefficient.
  int FindState(int state) const {
    for (int k = 0; k < scores_.size(); ++k) {
      if (state == scores_[k].first) return k;
    }
    return -1;
  }

 private:
  std::vector<std::pair<int, double> > scores_;
};


class SequenceDecoderEdgeScores {
 public:
  SequenceDecoderEdgeScores() {}
  virtual ~SequenceDecoderEdgeScores() {}

  // Get/Set the number of states for the current node. The states must be
  // numbered 0, 1, 2, ...
  int GetNumCurrentStates() const { return scores_.size(); }
  void SetNumCurrentStates(int num_current_states) {
    scores_.resize(num_current_states);
  }

  // Get/set the number of previous states compatible with the current node.
  int GetNumPreviousStates(int current_state) const {
    return scores_[current_state].size();
  }
  void SetNumPreviousStates(int current_state, int num_previous_states) {
    scores_[current_state].resize(num_previous_states);
  }

  // Finds the index corresponding to a given previous state, with respect to
  // the current state. Returns -1 if not found.
  // Note: this function is inefficient.
  int FindPreviousState(int current_state, int previous_state) const {
    for (int k = 0; k < scores_[current_state].size(); ++k) {
      if (previous_state == scores_[current_state][k].first) return k;
    }
    return -1;
  }

  // Set the previous state and the score with respect to the current state.
  // (Assumes SetNumPreviousStates(...) has been called first.)
  void SetPreviousStateScore(int current_state, int k, int previous_state,
                             double score) {
    scores_[current_state][k] = std::pair<int, double>(previous_state, score);
  }

  // Add a previous state and a score with respect to the current state.
  void AddPreviousStateScore(int current_state, int previous_state,
                             double score) {
    scores_[current_state].
      push_back(std::pair<int, double>(previous_state, score));
  }

  // Increment the score of a previous state by its index.
  void IncrementScore(int current_state, int k, double score) {
    scores_[current_state][k].second += score;
  }

  // Get the previous state and the score with respect to the current state.
  // (Assumes GetNumPreviousStates(...) has been called first.)
  const std::pair<int, double> &GetPreviousStateScore(int current_state,
                                                      int k) const {
    return scores_[current_state][k];
  }

  // Get all previous states and their scores with respect to the current state.
  const std::vector<std::pair<int, double> > &GetAllPreviousStateScores(
      int current_state) const {
    return scores_[current_state];
  }

  // Same, but return a mutable pointer.
  std::vector<std::pair<int, double> > *GetMutableAllPreviousStateScores(
      int current_state) {
    return &scores_[current_state];
  }

  // Compute bigram index for the state pair.
  int GetStatePairIndex(int current_state, int k) const {
    int bigram_index = k;
    for (int state = 0; state < current_state; ++state) {
      bigram_index += scores_[state].size();
    }
    return bigram_index;
  }

  // Compute number of bigram indices.
  int GetNumStatePairs() const { return GetStatePairIndex(scores_.size(), 0); }

 private:
  std::vector<std::vector<std::pair<int, double> > > scores_;
};


class SequenceDecoder : public Decoder {
 public:
  SequenceDecoder() {};
  SequenceDecoder(SequencePipe *pipe) : pipe_(pipe) {};
  virtual ~SequenceDecoder() {};

  void Decode(Instance *instance, Parts *parts,
              const vector<double> &scores,
              vector<double> *predicted_output);

  void DecodeCostAugmented(Instance *instance, Parts *parts,
                           const vector<double> &scores,
                           const vector<double> &gold_output,
                           vector<double> *predicted_output,
                           double *cost,
                           double *loss);

  virtual void DecodeMarginals(Instance *instance, Parts *parts,
                                 const vector<double> &scores,
                                 const vector<double> &gold_output,
                                 vector<double> *predicted_output,
                                 double *entropy,
                                 double *loss) {
    // TODO: Implement forward-backward.
    CHECK(false) << "Not implemented yet.";
  }

  void ConvertToFirstOrderModel(
    const std::vector<SequenceDecoderNodeScores> &node_scores,
    const std::vector<SequenceDecoderEdgeScores> &edge_scores,
    const std::vector<SequenceDecoderEdgeScores> &triplet_scores,
    std::vector<SequenceDecoderNodeScores> *transformed_node_scores,
    std::vector<SequenceDecoderEdgeScores> *transformed_edge_scores);

  void RecoverBestPath(const std::vector<int> &best_path,
                       std::vector<int> *transformed_best_path);

  double RunViterbi(const vector<vector<double> > &node_scores,
                    const vector<vector<vector<double > > >
                     &edge_scores,
                    vector<int> *best_path);

  double RunViterbi(const std::vector<SequenceDecoderNodeScores> &node_scores,
                    const std::vector<SequenceDecoderEdgeScores> &edge_scores,
                    std::vector<int> *best_path);

#ifdef USE_CPLEX
  double DecodeCPLEX(Instance *instance, Parts *parts,
      const vector<double> &scores,
      bool relax,
      vector<double> *predicted_output);
#endif

 protected:
  SequencePipe *pipe_;
};

#endif /* SEQUENCEDECODER_H_ */
