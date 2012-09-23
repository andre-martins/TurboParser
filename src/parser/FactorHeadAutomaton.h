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

#ifndef FACTOR_HEAD_AUTOMATON
#define FACTOR_HEAD_AUTOMATON

#include "DependencyPart.h"
#include "ad3/GenericFactor.h"

namespace AD3 {

class FactorHeadAutomaton : public GenericFactor {
 public:
  FactorHeadAutomaton() {}
  virtual ~FactorHeadAutomaton() {}

  // Print as a string.
  void Print(ostream& stream) {
    stream << "HEAD_AUTOMATON";
    Factor::Print(stream);
    int total = 0; // Delete this later.
    for (int m = 0; m < index_siblings_.size(); ++m) {
      for (int s = m+1; s <= index_siblings_.size(); ++s) {
        CHECK_GE(index_siblings_[m][s], 0);
        int index = index_siblings_[m][s];
        ++total;
        stream << " " << setprecision(9) << additional_log_potentials_[index];
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
    // Decode using the Viterbi algorithm.
    int length = length_;
    vector<vector<double> > values(length);
    vector<vector<int> > path(length);
    // The start state is m = 0.
    values[0].push_back(0.0);
    path[0].push_back(0);
    for (int m = 1; m < length; ++m) {
      // m+1 possible states: either keep the previous state (no arc added)
      // or transition to a new state (arc between h and m).
      values[m].resize(m+1);
      path[m].resize(m+1);
      for (int i = 0; i < m; ++i) {
        // In this case, the previous state must also be i.
        values[m][i] = values[m-1][i];
        path[m][i] = i;
      }
      // For the m-th state, the previous state can be anything up to m-1.
      path[m][m] = -1;
      for (int j = 0; j < m; ++j) {
        int index = index_siblings_[j][m];
        double score = values[m-1][j] + additional_log_potentials[index];
        if (path[m][m] < 0 || score > values[m][m]) {
          values[m][m] = score;
          path[m][m] = j;
        } 
      }
      values[m][m] += variable_log_potentials[m-1];
    }
    // The end state is m = length.
    vector<int> best_path(length);
    best_path[length-1] = -1;
    for (int j = 0; j < length; ++j) {
      int index = index_siblings_[j][length];
      assert(index >= 0 && index < additional_log_potentials.size());
      double score = values[length-1][j] + additional_log_potentials[index];
      if (best_path[length-1] < 0 || score > (*value)) {
        *value = score;
        best_path[length-1] = j;
      } 
    }

    // Backtrack.
    for (int m = length-1; m > 0; --m) {
      best_path[m-1] = path[m][best_path[m]];
    }
    vector<int> *modifiers = static_cast<vector<int>*>(configuration);
    for (int m = 1; m < length; ++m) {
      if (best_path[m] == m) {
        modifiers->push_back(m);
      }
    }
  }

  // Compute the score of a given assignment.
  void Evaluate(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                const Configuration configuration,
                double *value) {
    const vector<int>* modifiers =
        static_cast<const vector<int>*>(configuration);
    // Modifiers belong to {1,2,...}
    *value = 0.0;
    int m = 0;
    for (int i = 0; i < modifiers->size(); ++i) {
      int s = (*modifiers)[i];
      *value += variable_log_potentials[s-1];
      int index = index_siblings_[m][s];
      *value += additional_log_potentials[index];
      m = s;
    }
    int s = index_siblings_.size();
    int index = index_siblings_[m][s];
    *value += additional_log_potentials[index];
  }

  // Given a configuration with a probability (weight), 
  // increment the vectors of variable and additional posteriors.
  void UpdateMarginalsFromConfiguration(const Configuration &configuration,
                                        double weight,
                                        vector<double> *variable_posteriors,
                                        vector<double> *additional_posteriors) {
    const vector<int> *modifiers =
        static_cast<const vector<int>*>(configuration);
    int m = 0;
    for (int i = 0; i < modifiers->size(); ++i) {
      int s = (*modifiers)[i];
      (*variable_posteriors)[s-1] += weight;
      int index = index_siblings_[m][s];
      (*additional_posteriors)[index] += weight;
      m = s;
    }
    int s = index_siblings_.size();
    int index = index_siblings_[m][s];
    (*additional_posteriors)[index] += weight;
  }

  // Count how many common values two configurations have.
  int CountCommonValues(const Configuration &configuration1,
                        const Configuration &configuration2) {
    const vector<int> *values1 =
        static_cast<const vector<int>*>(configuration1);
    const vector<int> *values2 =
        static_cast<const vector<int>*>(configuration2);
    int count = 0;
    int j = 0;
    for (int i = 0; i < values1->size(); ++i) {
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
  bool SameConfiguration(const Configuration &configuration1,
                         const Configuration &configuration2) {
    const vector<int> *values1 =
        static_cast<const vector<int>*>(configuration1);
    const vector<int> *values2 =
        static_cast<const vector<int>*>(configuration2);
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
    vector<int>* modifiers = new vector<int>;
    return static_cast<Configuration>(modifiers); 
  }

 public:
  // length is relative to the head position. 
  // E.g. for a right automaton with h=3 and instance_length=10,
  // length = 7. For a left automaton, it would be length = 3.
  void Initialize(const vector<DependencyPartArc*> &arcs,
                  const vector<DependencyPartNextSibl*> &siblings) {
    length_ = arcs.size() + 1;
    index_siblings_.assign(length_, vector<int>(length_+1, -1));

    //CHECK_GT(arcs.size(), 0);
    int h = (arcs.size() > 0)? arcs[0]->head() : -1;
    int m = (arcs.size() > 0)? arcs[0]->modifier() : -1;
    vector<int> index_modifiers(1, 0);
    bool right = (h < m)? true : false;
    for (int k = 0; k < arcs.size(); ++k) {
      int previous_modifier = m;
      CHECK_EQ(h, arcs[k]->head());
      m = arcs[k]->modifier();
      if (k > 0) CHECK_EQ((m > previous_modifier), right);

      int position = right? m - h : h - m;
      index_modifiers.resize(position + 1, -1);
      index_modifiers[position] = k + 1;
    }

    for (int k = 0; k < siblings.size(); ++k) {
      if (arcs.size() > 0) CHECK_EQ(h, siblings[k]->head());
      h = siblings[k]->head();
      m = siblings[k]->modifier();
      int s = siblings[k]->next_sibling();
      if (arcs.size() > 0) CHECK_EQ(s > h, right);
      right = (s > h)? true : false;
      int position_modifier = right? m - h : h - m;
      int position_sibling = right? s - h : h - s;
      CHECK_LT(position_modifier, index_modifiers.size());
      int index_modifier = index_modifiers[position_modifier];
      int index_sibling = (position_sibling < index_modifiers.size())? 
          index_modifiers[position_sibling] : length_;
      CHECK_GE(index_modifier, 0);
      CHECK_LT(index_modifier, length_);
      CHECK_GE(index_sibling, 1) << h << " " << m << " " << s;
      CHECK_LT(index_sibling, length_+1);
      index_siblings_[index_modifier][index_sibling] = k;
    }
  }

 private:
  int length_;
  vector<vector<int> > index_siblings_;
};

} // namespace AD3

#endif // FACTOR_HEAD_AUTOMATON
