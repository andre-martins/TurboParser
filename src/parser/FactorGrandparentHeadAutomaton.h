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

#ifndef FACTOR_GRANDPARENT_HEAD_AUTOMATON
#define FACTOR_GRANDPARENT_HEAD_AUTOMATON

#include "DependencyPart.h"
#include "ad3/GenericFactor.h"

namespace AD3 {

class FactorGrandparentHeadAutomaton : public GenericFactor {
 public:
  FactorGrandparentHeadAutomaton() {}
  virtual ~FactorGrandparentHeadAutomaton() {}

  // Print as a string.
  void Print(ostream& stream) {
    stream << "GRANDPARENT_HEAD_AUTOMATON";
    Factor::Print(stream);
    // Print number of grandparents.
    stream << " " << index_grandparents_.size();
    int total = 0; // Delete this later.
    for (int g = 0; g < index_grandparents_.size(); ++g) {
      for (int m = 1; m < index_siblings_.size(); ++m) {
        CHECK_GE(index_grandparents_[g][m], 0);
        int index = index_grandparents_[g][m];
        ++total;
        stream << " " << setprecision(9)
               << additional_log_potentials_[index];
      }
    }
    for (int m = 0; m < index_siblings_.size(); ++m) {
      for (int s = m+1; s <= index_siblings_.size(); ++s) {
        CHECK_GE(index_siblings_[m][s], 0);
        int index = index_siblings_[m][s];
        ++total;
        stream << " " << setprecision(9)
               << additional_log_potentials_[index];
      }
    }
    if (use_grandsiblings_) {
      for (int g = 0; g < index_grandparents_.size(); ++g) {
        for (int m = 0; m < index_siblings_.size(); ++m) {
          for (int s = m+1; s <= index_siblings_.size(); ++s) {
            CHECK_GE(index_grandsiblings_[g][m][s], 0);
            int index = index_grandsiblings_[g][m][s];
            ++total;
            stream << " " << setprecision(9)
                   << additional_log_potentials_[index];
          }
        }
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
    // Decode maximizing over the grandparents and using the Viterbi algorithm
    // as an inner loop.
    int num_grandparents = index_grandparents_.size();
    int best_grandparent = -1;
    int length = length_;
    vector<vector<double> > values(length);
    vector<vector<int> > path(length);
    vector<int> best_path(length);

    // Run Viterbi for each possible grandparent.
    for (int g = 0; g < num_grandparents; ++g) {
      // The start state is m = 0.
      values[0].resize(1);
      values[0][0] = 0.0;
      path[0].resize(1);
      path[0][0] = 0;
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
          if (use_grandsiblings_) {
            index = index_grandsiblings_[g][j][m];
            if (index >= 0) score += additional_log_potentials[index];
          }
          if (path[m][m] < 0 || score > values[m][m]) {
            values[m][m] = score;
            path[m][m] = j;
          }
        }
        int index = index_grandparents_[g][m];
        values[m][m] += variable_log_potentials[num_grandparents+m-1] +
          additional_log_potentials[index];
      }

      // The end state is m = length.
      int best_last_state = -1;
      double best_score = -1e12;
      for (int j = 0; j < length; ++j) {
        int index = index_siblings_[j][length];
        double score = values[length-1][j] + additional_log_potentials[index];
        if (use_grandsiblings_) {
          index = index_grandsiblings_[g][j][length];
          if (index >= 0) score += additional_log_potentials[index];
        }
        if (best_last_state < 0 || score > best_score) {
          best_score = score;
          best_last_state = j;
        }
      }

      // Add the score of the arc (g-->h).
      best_score += variable_log_potentials[g];

      // Only backtrack if the solution is the best so far.
      if (best_grandparent < 0 || best_score > *value) {
        // This is the best grandparent so far.
        best_grandparent = g;
        *value = best_score;
        best_path[length-1] = best_last_state;

        // Backtrack.
        for (int m = length-1; m > 0; --m) {
          best_path[m-1] = path[m][best_path[m]];
        }
      }
    }

    // Now write the configuration.
    vector<int> *grandparent_modifiers =
      static_cast<vector<int>*>(configuration);
    grandparent_modifiers->push_back(best_grandparent);
    for (int m = 1; m < length; ++m) {
      if (best_path[m] == m) {
        grandparent_modifiers->push_back(m);
      }
    }
  }

  // Compute the score of a given assignment.
  void Evaluate(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                const Configuration configuration,
                double *value) {
    const vector<int>* grandparent_modifiers =
      static_cast<const vector<int>*>(configuration);
    // Grandparent belong to {0,1,...}
    // Modifiers belong to {1,2,...}
    *value = 0.0;
    int g = (*grandparent_modifiers)[0];
    *value += variable_log_potentials[g];
    int num_grandparents = index_grandparents_.size();
    int m = 0;
    for (int i = 1; i < grandparent_modifiers->size(); ++i) {
      int s = (*grandparent_modifiers)[i];
      *value += variable_log_potentials[num_grandparents+s-1];
      int index = index_siblings_[m][s];
      *value += additional_log_potentials[index];
      if (use_grandsiblings_) {
        index = index_grandsiblings_[g][m][s];
        if (index >= 0) *value += additional_log_potentials[index];
      }
      m = s;
      index = index_grandparents_[g][m];
      *value += additional_log_potentials[index];
    }
    int s = index_siblings_.size();
    int index = index_siblings_[m][s];
    *value += additional_log_potentials[index];
    if (use_grandsiblings_) {
      index = index_grandsiblings_[g][m][s];
      if (index >= 0) *value += additional_log_potentials[index];
    }
    //cout << "value = " << *value << endl;
  }

  // Given a configuration with a probability (weight),
  // increment the vectors of variable and additional posteriors.
  void UpdateMarginalsFromConfiguration(
    const Configuration &configuration,
    double weight,
    vector<double> *variable_posteriors,
    vector<double> *additional_posteriors) {
    const vector<int> *grandparent_modifiers =
      static_cast<const vector<int>*>(configuration);
    int g = (*grandparent_modifiers)[0];
    (*variable_posteriors)[g] += weight;
    int num_grandparents = index_grandparents_.size();
    int m = 0;
    for (int i = 1; i < grandparent_modifiers->size(); ++i) {
      int s = (*grandparent_modifiers)[i];
      (*variable_posteriors)[num_grandparents+s-1] += weight;
      int index = index_siblings_[m][s];
      (*additional_posteriors)[index] += weight;
      if (use_grandsiblings_) {
        index = index_grandsiblings_[g][m][s];
        if (index >= 0) (*additional_posteriors)[index] += weight;
      }
      m = s;
      index = index_grandparents_[g][m];
      (*additional_posteriors)[index] += weight;
    }
    int s = index_siblings_.size();
    int index = index_siblings_[m][s];
    (*additional_posteriors)[index] += weight;
    if (use_grandsiblings_) {
      index = index_grandsiblings_[g][m][s];
      if (index >= 0) (*additional_posteriors)[index] += weight;
    }
  }

  // Count how many common values two configurations have.
  int CountCommonValues(const Configuration &configuration1,
                        const Configuration &configuration2) {
    const vector<int> *values1 = static_cast<const vector<int>*>(configuration1);
    const vector<int> *values2 = static_cast<const vector<int>*>(configuration2);
    int count = 0;
    if ((*values1)[0] == (*values2)[0]) ++count; // Grandparents matched.
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
    // The first element is the index of the grandparent.
    // The remaining elements are the indices of the modifiers.
    vector<int>* grandparent_modifiers = new vector<int>;
    return static_cast<Configuration>(grandparent_modifiers);
  }

 public:
  // Incoming arcs are of the form (g,h) for each g.
  // Outgoing arcs are of the form (h,m) for eacg m.
  // The variables linked to this factor must be in the same order as
  // the incoming arcs, followed by the outgoing arcs.
  // The incoming arcs must be sorted for the closest to the farthest
  // away from the root.
  void Initialize(const vector<DependencyPartArc*> &incoming_arcs,
                  const vector<DependencyPartArc*> &outgoing_arcs,
                  const vector<DependencyPartGrandpar*> &grandparents,
                  const vector<DependencyPartNextSibl*> &siblings) {
    //cout << "Initializing grandparent head automaton... ";
    vector<DependencyPartGrandSibl*> grandsiblings;
    Initialize(incoming_arcs, outgoing_arcs, grandparents, siblings,
               grandsiblings);
    //cout << "Done." << endl;
  }

  void Initialize(const vector<DependencyPartArc*> &incoming_arcs,
                  const vector<DependencyPartArc*> &outgoing_arcs,
                  const vector<DependencyPartGrandpar*> &grandparents,
                  const vector<DependencyPartNextSibl*> &siblings,
                  const vector<DependencyPartGrandSibl*> &grandsiblings) {
    /*
    cout << "New grandparent head automaton." << endl;
    cout << "Grandparents: ";
    for (int i = 0; i < grandparents.size(); ++i) {
      cout << "(" << grandparents[i]->grandparent()
           << ", " << grandparents[i]->head()
           << ", " << grandparents[i]->modifier()
           << ") ";
    }
    cout << endl;
    cout << "Siblings: ";
    for (int i = 0; i < siblings.size(); ++i) {
      cout << "(" << siblings[i]->head()
           << ", " << siblings[i]->modifier()
           << ", " << siblings[i]->next_sibling()
           << ") ";
    }
    cout << endl;
    cout << "Grandsiblings: ";
    for (int i = 0; i < grandsiblings.size(); ++i) {
      cout << "(" << grandsiblings[i]->grandparent()
           << ", " << grandsiblings[i]->head()
           << ", " << grandsiblings[i]->modifier()
           << ", " << grandsiblings[i]->sibling()
           << ") ";
    }
    cout << endl;
    */

    // length is relative to the head position.
    // E.g. for a right automaton with h=3 and instance_length=10,
    // length = 7. For a left automaton, it would be length = 3.
    use_grandsiblings_ = (grandsiblings.size() > 0);
    int num_grandparents = incoming_arcs.size();
    length_ = outgoing_arcs.size() + 1;
    index_grandparents_.assign(num_grandparents, vector<int>(length_, -1));
    index_siblings_.assign(length_, vector<int>(length_+1, -1));
    if (use_grandsiblings_) {
      index_grandsiblings_.assign(num_grandparents,
                                  vector<vector<int> >(length_,
                                                       vector<int>(length_+1, -1)));
    }

    // Create a temporary index of modifiers.
    int h = (outgoing_arcs.size() > 0)? outgoing_arcs[0]->head() : -1;
    int m = (outgoing_arcs.size() > 0)? outgoing_arcs[0]->modifier() : -1;
    vector<int> index_modifiers(1, 0);
    bool right = (h < m)? true : false;
    for (int k = 0; k < outgoing_arcs.size(); ++k) {
      int previous_modifier = m;
      CHECK_EQ(h, outgoing_arcs[k]->head());
      m = outgoing_arcs[k]->modifier();
      //cout << "s-arc " << h << " -> " << m << endl;
      if (k > 0) CHECK_EQ((m > previous_modifier), right);

      int position = right? m - h : h - m;
      index_modifiers.resize(position + 1, -1);
      index_modifiers[position] = k + 1;
    }

    // Construct index of siblings.
    for (int k = 0; k < siblings.size(); ++k) {
      if (outgoing_arcs.size() > 0) CHECK_EQ(h, siblings[k]->head());
      h = siblings[k]->head();
      m = siblings[k]->modifier();
      int s = siblings[k]->next_sibling();
      //cout << "sibling " << h << " -> " << m << " -> " << s << endl;
      if (outgoing_arcs.size() > 0) CHECK_EQ(s > h, right);
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
      // Add an offset to save room for the grandparents.
      index_siblings_[index_modifier][index_sibling] =
        grandparents.size() + k;
    }

    // Create a temporary index of grandparents.
    int g = (incoming_arcs.size() > 0)? incoming_arcs[0]->head() : -1;
    h = (incoming_arcs.size() > 0)? incoming_arcs[0]->modifier() : -1;
    vector<int> index_incoming;
    for (int k = 0; k < incoming_arcs.size(); ++k) {
      g = incoming_arcs[k]->head();
      //cout << "g-arc " << g << " -> " 
      //     << incoming_arcs[k]->modifier() << endl;
      CHECK_EQ(h, incoming_arcs[k]->modifier());
      // Allow for the case where g is -1 (the head being the root
      // in this case). To handle this, set the position to g+1.
      int position = g + 1;
      index_incoming.resize(position + 1, -1);
      index_incoming[position] = k;
    }

    // Construct index of grandparents.
    for (int k = 0; k < grandparents.size(); ++k) {
      if (incoming_arcs.size() > 0) CHECK_EQ(h, grandparents[k]->head());
      int g = grandparents[k]->grandparent();
      h = grandparents[k]->head();
      m = grandparents[k]->modifier();
      //cout << "grandparent " << g << " -> " << h << " -> " << m << endl;

      CHECK_NE(h, m);
      right = (m > h)? true : false;
      int position_modifier = right? m - h : h - m;
      int position_grandparent = g + 1;
      CHECK_LT(position_modifier, index_modifiers.size());
      CHECK_LT(position_grandparent, index_incoming.size());
      int index_modifier = index_modifiers[position_modifier];
      int index_grandparent = index_incoming[position_grandparent];
      CHECK_GE(index_modifier, 0);
      CHECK_LT(index_modifier, length_);
      CHECK_GE(index_grandparent, 0);
      CHECK_LT(index_grandparent, num_grandparents);
      index_grandparents_[index_grandparent][index_modifier] = k;
    }

    // Construct index of grandsiblings.
    for (int k = 0; k < grandsiblings.size(); ++k) {
      if (incoming_arcs.size() > 0) CHECK_EQ(h, grandsiblings[k]->head());
      int g = grandsiblings[k]->grandparent();
      h = grandsiblings[k]->head();
      m = grandsiblings[k]->modifier();
      int s = grandsiblings[k]->sibling();
      //cout << "grandsibling " << g << " -> " << h << " -> " << m << " -> " << s << endl;

      //cout << "sibling " << h << " -> " << m << " -> " << s << endl;
      if (outgoing_arcs.size() > 0) CHECK_EQ(s > h, right);
      right = (s > h)? true : false;
      int position_grandparent = g + 1;
      int position_modifier = right? m - h : h - m;
      int position_sibling = right? s - h : h - s;
      CHECK_LT(position_grandparent, index_incoming.size());
      CHECK_LT(position_modifier, index_modifiers.size());
      int index_grandparent = index_incoming[position_grandparent];
      int index_modifier = index_modifiers[position_modifier];
      int index_sibling = (position_sibling < index_modifiers.size())?
          index_modifiers[position_sibling] : length_;
      CHECK_GE(index_grandparent, 0);
      CHECK_LT(index_grandparent, num_grandparents);
      CHECK_GE(index_modifier, 0);
      CHECK_LT(index_modifier, length_);
      CHECK_GE(index_sibling, 1) << h << " " << m << " " << s;
      CHECK_LT(index_sibling, length_+1);
      // Add an offset to save room for the grandparents and siblings.
      index_grandsiblings_[index_grandparent][index_modifier][index_sibling] =
        siblings.size() + grandparents.size() + k;
    }
  }

 private:
  bool use_grandsiblings_;
  int length_;
  vector<vector<int> > index_siblings_;
  vector<vector<int> > index_grandparents_;
  vector<vector<vector<int> > > index_grandsiblings_;
};

} // namespace AD3

#endif // FACTOR_GRANDPARENT_HEAD_AUTOMATON
