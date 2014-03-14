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
    stream << " " << index_senses_.size();
    int total = 0; // Delete this later.
    for (int s = 0; s < index_senses_.size(); ++s) {
      for (int a1 = 0; a1 < index_siblings_[s].size(); ++a1) {
        for (int a2 = a1+1; a2 <= index_siblings_[s][a1].size(); ++a2) {
        CHECK_GE(index_siblings_[s][a1][a2], 0);
        int index = index_siblings_[s][a1][a2];
        ++total;
        stream << " " << setprecision(9)
               << additional_log_potentials_[index];
      }
    }
    stream << endl;
    CHECK_EQ(additional_log_potentials_.size(), total);
  }

  // Compute the score of a given assignment.
  void Maximize(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                Configuration &configuration,
                double *value) {
    // Decode maximizing over the senses and using the Viterbi algorithm
    // as an inner loop.
    // If sense=-1, the final score is zero (so take the argmax at the end).
    int num_senses = index_senses_.size();
    int best_sense = -1;
    *value = 0.0;
    int length = length_;
    vector<vector<double> > values(length);
    vector<vector<int> > path(length);
    vector<int> best_path(length);

    // Run Viterbi for each possible grandparent.
    for (int s = 0; s < num_senses; ++s) {
      // The start state is m = 0.
      values[0].resize(1);
      values[0][0] = 0.0;
      path[0].resize(1);
      path[0][0] = 0;
      for (int a = 1; a < length; ++a) {
        // a+1 possible states: either keep the previous state (no arc added)
        // or transition to a new state (arc between p and a1).
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
          int index = index_siblings_[s][j][a];
          CHECK_GE(index, 0);
          score += additional_log_potentials[index];
          if (path[a][a] < 0 || score > values[a][a]) {
            values[a][a] = score;
            path[a][a] = j;
          }
        }
        values[a][a] += variable_log_potentials[num_senses+a-1]; // FIXME: arcs are also indexed by senses.
      }

      // The end state is a1 = length.
      int best_last_state = -1;
      double best_score = -1e12;
      for (int j = 0; j < length; ++j) {
        int index = index_siblings_[s][j][length];
        CHECK_GE(index, 0);
        double score = values[length-1][j] + additional_log_potentials[index];
        if (best_last_state < 0 || score > best_score) {
          best_score = score;
          best_last_state = j;
        }
      }

      // Add the score of the predicate sense s.
      best_score += variable_log_potentials[s];

      // Only backtrack if the solution is the best so far.
      if (best_score > *value) {
        // This is the best sense so far.
        best_sense = s;
        *value = best_score;
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
    grandparent_modifiers->push_back(best_sense);
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
    *value = 0.0;
    int s = (*sense_arguments)[0];
    if (s >= 0) {
      *value += variable_log_potentials[s];
    } else {
      CHECK_EQ(sense_arguments->size(), 1);
    }
    int num_senses = index_senses_.size();
    int a1 = 0;
    for (int i = 1; i < sense_arguments->size(); ++i) {
      int a2 = (*sense_arguments)[i];
      *value += variable_log_potentials[num_senses+a2-1]; // FIXME: arcs are also indexed by senses.
      int index = index_siblings_[s][a1][a2];
      CHECK_GE(index, 0);
      *value += additional_log_potentials[index];
      a1 = a2;
    }
    int a2 = index_siblings_.size();
    int index = index_siblings_[s][a1][a2];
    CHECK_GE(index, 0);
    *value += additional_log_potentials[index];
    //cout << "value = " << *value << endl;
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
    int s = (*sense_arguments)[0];
    if (s >= 0) {
      (*variable_posteriors)[s] += weight;
    } else {
      CHECK_EQ(sense_arguments->size(), 1);
    }
    int num_senses = index_senses_.size();
    int a1 = 0;
    for (int i = 1; i < sense_arguments->size(); ++i) {
      int a2 = (*sense_arguments)[i];
      (*variable_posteriors)[num_senses+a2-1] += weight; // FIXME: arcs are also indexed by senses.
      int index = index_siblings_[s][a1][a2];
      CHECK_GE(index, 0);
      (*additional_posteriors)[index] += weight;
      a1 = a2;
    }
    int a2 = index_siblings_.size();
    int index = index_siblings_[s][a1][a2];
    CHECK_GE(index, 0);
    (*additional_posteriors)[index] += weight;
  }

  // Count how many common values two configurations have.
  int CountCommonValues(const Configuration &configuration1,
                        const Configuration &configuration2) {
    const vector<int> *values1 = static_cast<const vector<int>*>(configuration1);
    const vector<int> *values2 = static_cast<const vector<int>*>(configuration2);
    int count = 0;
    if ((*values1)[0] > 0 &&
        (*values1)[0] == (*values2)[0]) ++count; // Predicate sense matched.
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
  // "Right" indicated the attachment direction (if true, then p <= a1 < a2).
  void Initialize(bool right,
                  const vector<SemanticPartPredicate*> &predicate_senses,
                  const vector<SemanticPartArc*> &outgoing_arcs,
                  const vector<SemanticPartConsecutiveSibling*> &siblings) {

    // length is relative to the head position.
    // E.g. for a right automaton with h=3 and instance_length=10,
    // length = 7. For a left automaton, it would be length = 3.
    // TODO: maybe change the numbering so that self-loops are allowed.

    // Build map of senses.
    HashMapIntInt map_senses;
    int num_senses = predicate_senses.size();
    CHECK_GT(num_senses, 0);
    int p = -1;
    for (int k = 0; k < predicate_senses.size(); ++k) {
      int s = predicate_senses[k]->sense();
      if (p >= 0) CHECK_EQ(p, predicate_senses[k]->predicate());
      CHECK(map_senses.find(s) == map_senses.end());
      map_senses[s] = k;
    }

    //int GetNumSenses() const { return index_senses_.size(); }

    // Create a temporary list of arguments.
    vector<vector<int> > sense_arguments(num_senses);
    for (int k = 0; k < outgoing_arcs.size(); ++k) {
      CHECK_EQ(p == outgoing_arcs[k]->predicate());
      int a = outgoing_arcs[k]->argument();
      int s = outgoing_arcs[k]->sense();
      // TODO: handle a = -1.
      if (right) {
        a = a - p;
      } else {
        a = p - a;
      }
      CHECK(map_senses.find(s) != map_senses.end());
      int sense = map_senses[s];
      if (a >= sense_arguments[sense].size()) {
        sense_arguments[sense].resize(a+1, -1);
      }
      sense_arguments[sense][a] = k;
    }

    index_arguments_.assign(num_senses, vector<int>(0));
    for (int sense = 0; sense < num_senses; ++sense) {
      for (int a = 0; a < sense_arguments[sense].size(); ++a) {
        int k = sense_arguments[sense][a];
        if (k >= 0) {
          index_arguments_[sense].push_back(k); // Or: offset_arcs + k.
        }
      }
    }

    //////////////////////////////////////////////////// here.

    length_ = outgoing_arcs.size() + 1;
    index_arguments_.assign(num_senses, vector<int>(length_, -1));
    index_siblings_.assign(num_senses,
                           vector<vector<int> >(length_,
                                                vector<int>(length_+1, -1)));

    // Create a temporary index of arguments.
    int p = (outgoing_arcs.size() > 0)? outgoing_arcs[0]->predicate() : -1;
    int a = (outgoing_arcs.size() > 0)? outgoing_arcs[0]->argument() : -1;
    vector<int> index_arguments(1, 0);
    bool right = (p < a)? true : false;
    for (int k = 0; k < outgoing_arcs.size(); ++k) {
      int s = outgoing_arcs[k]->sense();
      
      int previous_argument = a;
      CHECK_EQ(h, outgoing_arcs[k]->predicate());
      a = outgoing_arcs[k]->argument();
      if (k > 0) CHECK_EQ((a > previous_argument), right);

      int position = right? a - p : p - a;
      index_arguments.resize(position + 1, -1);
      index_arguments[position] = k + 1;
    }

    // Create a temporary index of predicate senses.
    // Note: outgoing arc variables will start with an offset of num_senses.
    CHECK_GT(predicate_senses.size(), 0);
    int sense = (predicate_senses.size() > 0)?
        predicate_senses[0]->sense() : -1;
    vector<int> index_senses;
    for (int k = 0; k < predicate_senses.size(); ++k) {
      sense = predicate_senses[k]->sense();
      CHECK_EQ(p, predicate_senses[k]->predicate();
      index_senses.resize(sense, -1);
      index_senses[sense] = k;
    }


    // Construct index of siblings.
    for (int k = 0; k < siblings.size(); ++k) {
      CHECK_EQ(p, siblings[k]->predicate());
      p = siblings[k]->predicate();
      int s = siblings[k]->sense();
      int a1 = siblings[k]->first_argument();
      int a2 = siblings[k]->second_argument();

      if (outgoing_arcs.size() > 0) CHECK_EQ(a2 > p, right);
      right = (a2 > p)? true : false;
      int position_first_argument = right? a1 - p : p - a1;
      int position_second_argument = right? a2 - p : p - a2;
      CHECK_LT(s, index_senses.size());
      CHECK_LT(position_first_argument, index_arguments.size());
      int index_sense = index_senses[s];
      int index_first_argument = index_arguments[position_first_argument];
      int index_second_argument =
        (position_second_argument < index_arguments.size())?
          index_arguments[position_second_argument] : length_;
      CHECK_GE(index_sense, 0);
      CHECK_LT(index_sense, num_senses);
      CHECK_GE(index_first_argument, 0);
      CHECK_LT(index_first_argument, length_);
      CHECK_GE(index_second_argument, 1) << p << " " << a1 << " " << a2;
      CHECK_LT(index_second_argument, length_+1);
      // Add an offset to save room for the grandparents and siblings.
      index_siblings_[index_sense][index_first_argument]
        [index_second_argument] = k;
    }
  }

 private:
  int length_;
  vector<int> index_senses_; // Indexed by s.
  vector<vector<int> > index_arguments_; // Indexed by s, a.
  vector<vector<vector<int> > > index_siblings_; // Indexed by s, a1, a2.
};

} // namespace AD3

#endif // FACTOR_PREDICATE_AUTOMATON
