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

#ifndef FACTOR_PREDICATE_AUTOMATON
#define FACTOR_PREDICATE_AUTOMATON

#include "SemanticPart.h"
#include "ad3/GenericFactor.h"
#ifdef _WIN32
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif

namespace AD3 {

typedef std::tr1::unordered_map<int, int> HashMapIntInt;

class FactorPredicateAutomaton : public GenericFactor {
 public:
  FactorPredicateAutomaton() {}
  virtual ~FactorPredicateAutomaton() { ClearActiveSet(); }

  // Print as a string.
  void Print(ostream& stream) {
    stream << "PREDICATE_AUTOMATON";
    Factor::Print(stream);
    // Print number of senses.
    stream << " " << GetNumSenses();
    int total = 0; // Delete this later.
    for (int s = 0; s < GetNumSenses(); ++s) {
      for (int a1 = 0; a1 < index_siblings_[s].size(); ++a1) {
        for (int a2 = a1+1; a2 <= index_siblings_[s][a1].size(); ++a2) {
          CHECK_GE(index_siblings_[s][a1][a2], 0);
          int index = index_siblings_[s][a1][a2];
          ++total;
          stream << " " << setprecision(9)
                 << additional_log_potentials_[index];
        }
      }
    }
    stream << endl;
    CHECK_EQ(additional_log_potentials_.size(), total);
  }

  int GetNumSenses() const { return index_arguments_.size(); }
  int GetLength(int sense) const { return index_arguments_[sense].size(); }
  double GetSenseScore(int sense,
                       const vector<double> &variable_log_potentials,
                       const vector<double> &additional_log_potentials) const {
    return variable_log_potentials[sense];
  }
  double GetArgumentScore(
      int sense,
      int argument,
      const vector<double> &variable_log_potentials,
      const vector<double> &additional_log_potentials) const {
    int index = index_arguments_[sense][argument];
    return variable_log_potentials[index];
  }
  double GetSiblingScore(
      int sense,
      int first_argument,
      int second_argument,
      const vector<double> &variable_log_potentials,
      const vector<double> &additional_log_potentials) const {
    int index = index_siblings_[sense][first_argument][second_argument];
    return additional_log_potentials[index];
  }
  void AddSensePosterior(int sense,
                         double weight,
                         vector<double> *variable_posteriors,
                         vector<double> *additional_posteriors) const {
    (*variable_posteriors)[sense] += weight;
  }
  void AddArgumentPosterior(int sense,
                            int argument,
                            double weight,
                            vector<double> *variable_posteriors,
                            vector<double> *additional_posteriors) const {
    int index = index_arguments_[sense][argument];
    (*variable_posteriors)[index] += weight;
  }
  void AddSiblingPosterior(int sense,
                           int first_argument,
                           int second_argument,
                           double weight,
                           vector<double> *variable_posteriors,
                           vector<double> *additional_posteriors) const {
    int index = index_siblings_[sense][first_argument][second_argument];
    (*additional_posteriors)[index] += weight;
  }

  // Compute the score of a given assignment.
  void Maximize(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                Configuration &configuration,
                double *value) {
    // Decode maximizing over the senses and using the Viterbi algorithm
    // as an inner loop.
    // If sense=-1, the final score is zero (so take the argmax at the end).
    int num_senses = GetNumSenses();
    int best_sense = -1;
    vector<int> best_path;
    *value = 0.0;

    // Run Viterbi for each possible grandparent.
    for (int s = 0; s < num_senses; ++s) {
      int length = GetLength(s);
      vector<vector<double> > values(length);
      vector<vector<int> > path(length);
      CHECK_GE(length, 1);

      // The start state is a1 = 0.
      values[0].resize(1);
      values[0][0] = 0.0;
      path[0].resize(1);
      path[0][0] = 0;
      for (int a = 1; a < length; ++a) {
        // a+1 possible states: either keep the previous state (no arc added)
        // or transition to a new state (arc between p and a).
        values[a].resize(a+1);
        path[a].resize(a+1);
        for (int i = 0; i < a; ++i) {
          // In this case, the previous state must also be i.
          values[a][i] = values[a-1][i];
          path[a][i] = i;
        }
        // For the a-th state, the previous state can be anything up to a-1.
        path[a][a] = -1;
        for (int j = 0; j < a; ++j) {
          double score = values[a-1][j];
          score += GetSiblingScore(s, j, a, variable_log_potentials,
                                   additional_log_potentials);
          if (path[a][a] < 0 || score > values[a][a]) {
            values[a][a] = score;
            path[a][a] = j;
          }
        }
        values[a][a] += GetArgumentScore(s, a, variable_log_potentials,
                                         additional_log_potentials);
      }

      // The end state is a = length.
      int best_last_state = -1;
      double best_score = -1e12;
      for (int j = 0; j < length; ++j) {
        int index = index_siblings_[s][j][length];
        CHECK_GE(index, 0);
        double score = values[length-1][j] +
          GetSiblingScore(s, j, length, variable_log_potentials,
                          additional_log_potentials);
        if (best_last_state < 0 || score > best_score) {
          best_score = score;
          best_last_state = j;
        }
      }

      // Add the score of the predicate sense s.
      // Note: we're allowing senses != -1 with no argument frame.
      best_score += GetSenseScore(s, variable_log_potentials,
                                  additional_log_potentials);

      // Only backtrack if the solution is the best so far.
      if (best_score > *value) {
        // This is the best sense so far.
        best_sense = s;
        *value = best_score;
        best_path.resize(length);
        best_path[length-1] = best_last_state;

        // Backtrack.
        for (int a = length-1; a > 0; --a) {
          best_path[a-1] = path[a][best_path[a]];
        }
      }
    }

    // Now write the configuration.
    vector<int> *sense_arguments =
      static_cast<vector<int>*>(configuration);
    sense_arguments->push_back(best_sense);
    int length = (best_sense >= 0)? GetLength(best_sense) : 0;
    for (int a = 1; a < length; ++a) {
      if (best_path[a] == a) {
        sense_arguments->push_back(a);
      }
    }
  }

  // Compute the score of a given assignment.
  void Evaluate(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                const Configuration configuration,
                double *value) {
    const vector<int> *sense_arguments =
      static_cast<const vector<int>*>(configuration);
    // Sense belong to {-1,0,1,...}
    // Modifiers belong to {1,2,...}
    CHECK_GE(sense_arguments->size(), 1);
    *value = 0.0;
    int s = (*sense_arguments)[0];
    if (s < 0) {
      CHECK_EQ(sense_arguments->size(), 1);
      return;
    }
    *value += GetSenseScore(s, variable_log_potentials,
                            additional_log_potentials);
    int a1 = 0; // Start position.
    for (int i = 1; i < sense_arguments->size(); ++i) {
      int a2 = (*sense_arguments)[i];
      *value += GetArgumentScore(s, a2, variable_log_potentials,
                                 additional_log_potentials);
      *value += GetSiblingScore(s, a1, a2, variable_log_potentials,
                                additional_log_potentials);
      a1 = a2;
    }
    int a2 = GetLength(s); // Stop position.
    *value += GetSiblingScore(s, a1, a2, variable_log_potentials,
                              additional_log_potentials);
  }

  // Given a configuration with a probability (weight),
  // increment the vectors of variable and additional posteriors.
  void UpdateMarginalsFromConfiguration(
    const Configuration &configuration,
    double weight,
    vector<double> *variable_posteriors,
    vector<double> *additional_posteriors) {
    const vector<int> *sense_arguments =
      static_cast<const vector<int>*>(configuration);
    // Sense belong to {-1,0,1,...}
    // Modifiers belong to {1,2,...}
    CHECK_GE(sense_arguments->size(), 1);
    int s = (*sense_arguments)[0];
    if (s < 0) {
      CHECK_EQ(sense_arguments->size(), 1);
      return;
    }
    AddSensePosterior(s, weight, variable_posteriors,
                      additional_posteriors);
    int a1 = 0; // Start position.
    for (int i = 1; i < sense_arguments->size(); ++i) {
      int a2 = (*sense_arguments)[i];
      AddArgumentPosterior(s, a2, weight, variable_posteriors,
                           additional_posteriors);
      AddSiblingPosterior(s, a1, a2, weight, variable_posteriors,
                          additional_posteriors);
      a1 = a2;
    }
    int a2 = GetLength(s); // Stop position.
    AddSiblingPosterior(s, a1, a2, weight, variable_posteriors,
                        additional_posteriors);
  }

  // Count how many common values two configurations have.
  int CountCommonValues(const Configuration &configuration1,
                        const Configuration &configuration2) {
    const vector<int> *values1 = static_cast<const vector<int>*>(configuration1);
    const vector<int> *values2 = static_cast<const vector<int>*>(configuration2);
    int count = 0;
    if ((*values1)[0] >= 0 &&
        (*values1)[0] == (*values2)[0]) ++count; // Predicate sense matched.
    // Note: if the predicate sense did not match, shouldn't the final
    // count be zero (return here)?
    // TODO: check this.
    int j = 1;
    for (int i = 1; i < values1->size(); ++i) {
      for (; j < values2->size(); ++j) {
        if ((*values2)[j] >= (*values1)[i]) break;
      }
      if (j < values2->size() && (*values2)[j] == (*values1)[i]) {
        ++count;
        ++j;
      }
    }
    return count;
  }

  // Check if two configurations are the same.
  bool SameConfiguration(
    const Configuration &configuration1,
    const Configuration &configuration2) {
    const vector<int> *values1 = static_cast<const vector<int>*>(configuration1);
    const vector<int> *values2 = static_cast<const vector<int>*>(configuration2);
    if (values1->size() != values2->size()) return false;
    for (int i = 0; i < values1->size(); ++i) {
      if ((*values1)[i] != (*values2)[i]) return false;
    }
    return true;
  }

  // Delete configuration.
  void DeleteConfiguration(
    Configuration configuration) {
    vector<int> *values = static_cast<vector<int>*>(configuration);
    delete values;
  }

  Configuration CreateConfiguration() {
    // The first element is the predicate sense (or -1 if it is not a
    // predicate).
    // The remaining elements are the indices of the arguments.
    vector<int>* sense_arguments = new vector<int>;
    return static_cast<Configuration>(sense_arguments);
  }

 public:
  // Predicate senses are of the form (p,s) for each s.
  // Outgoing arcs are of the form (p,s,a) for each s, a.
  // The variables linked to this factor must be in the same order as
  // the predicate senses, followed by the outgoing arcs.
  // "Right" indicates the attachment direction (if true, then p <= a1 < a2).
  void Initialize(bool right,
                  const vector<SemanticPartPredicate*> &predicate_senses,
                  const vector<SemanticPartArc*> &outgoing_arcs,
                  const vector<SemanticPartConsecutiveSibling*> &siblings) {
    // Build map of senses.
    HashMapIntInt map_senses;
    int num_senses = predicate_senses.size();
    CHECK_GT(num_senses, 0);
    int p = -1;
    for (int k = 0; k < predicate_senses.size(); ++k) {
      int s = predicate_senses[k]->sense();
      if (p >= 0) CHECK_EQ(p, predicate_senses[k]->predicate());
      p = predicate_senses[k]->predicate();
      CHECK(map_senses.find(s) == map_senses.end());
      map_senses[s] = k;
    }

    // Create a temporary list of arguments.
    // Each argument position will be mapped to a one-based array, in
    // which sense_arguments[s][1] = p, sense_arguments[s][2] = p+1 (p-1),
    // etc. for the right (left) automaton case. Self-loops are possible.
    // The zero position is reserved to denote a a1 = -1 (in a2 will be the
    // first argument of the predicate).
    // E.g. for a right automaton with p=3, the argument a1=7 will be stored in
    // position 7-3+1=5.
    int offset_outgoing_arcs = num_senses;
    vector<vector<int> > sense_arguments(num_senses,
                                         vector<int>(1, -1));
    for (int k = 0; k < outgoing_arcs.size(); ++k) {
      CHECK_EQ(p, outgoing_arcs[k]->predicate());
      int a = outgoing_arcs[k]->argument();
      int s = outgoing_arcs[k]->sense();
      int position = (right)? a-p : p-a;
      ++position; // Position 0 is reserved for the case a1=-1.
      CHECK_GE(position, 1) << p << " " << a;
      CHECK(map_senses.find(s) != map_senses.end());
      int sense = map_senses[s];
      if (position >= sense_arguments[sense].size()) {
        sense_arguments[sense].resize(position+1, -1);
      }
      // This will be replaced by the index of the argument.
      sense_arguments[sense][position] = offset_outgoing_arcs + k;
    }

    index_arguments_.assign(num_senses, vector<int>(0));
    for (int sense = 0; sense < num_senses; ++sense) {
      for (int position = 0; position < sense_arguments[sense].size();
           ++position) {
        int index = sense_arguments[sense][position];
        // Always put something in the zero position (which is special).
        // In this case, index will be -1.
        if (position == 0 || index >= 0) {
          // Replace sense_arguments[sense][position] by the index of the
          // argument.
          sense_arguments[sense][position] = index_arguments_[sense].size();
          // Index of outgoing arc in the variables array.
          index_arguments_[sense].push_back(index);
        }
      }
    }

    // Construct index of siblings.
    index_siblings_.assign(num_senses, vector<vector<int> >(0));
    for (int sense = 0; sense < num_senses; ++sense) {
      int length = GetLength(sense);
      index_siblings_[sense].resize(length, vector<int>(length+1, -1));
    }
    for (int k = 0; k < siblings.size(); ++k) {
      CHECK_EQ(p, siblings[k]->predicate());
      int s = siblings[k]->sense();
      int a1 = siblings[k]->first_argument();
      int a2 = siblings[k]->second_argument();
      int position1 = right? a1-p : p-a1;
      int position2 = right? a2-p : p-a2;
      if (a1 < 0) position1 = -1; // To handle a1=-1.
      ++position1; // Position 0 is reserved for the case a1=-1.
      ++position2;
      CHECK_GE(position1, 0);
      CHECK_GE(position2, 1);
      CHECK(map_senses.find(s) != map_senses.end());
      int sense = map_senses[s];
      CHECK_LT(position1, sense_arguments[sense].size());
      //CHECK_LT(position2, sense_arguments[sense].size()+1) << p << " " << a1 << " " << a2 << " " << GetLength(sense);
      CHECK_EQ(GetLength(sense), index_arguments_[sense].size());
      int first_argument = sense_arguments[sense][position1];
      int second_argument = (position2 < sense_arguments[sense].size())?
        sense_arguments[sense][position2] : GetLength(sense);
      CHECK_GE(first_argument, 0);
      CHECK_GE(second_argument, 1);
      CHECK_LT(first_argument, index_siblings_[sense].size());
      CHECK_LT(second_argument, index_siblings_[sense][first_argument].size());
      // Index of siblings in the additional_variables array.
      index_siblings_[sense][first_argument][second_argument] = k;
    }
  }

 private:
  vector<vector<int> > index_arguments_; // Indexed by s, a.
  vector<vector<vector<int> > > index_siblings_; // Indexed by s, a1, a2.
};

} // namespace AD3

#endif // FACTOR_PREDICATE_AUTOMATON
