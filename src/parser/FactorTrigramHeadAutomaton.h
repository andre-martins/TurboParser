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

#ifndef FACTOR_TRIGRAM_HEAD_AUTOMATON
#define FACTOR_TRIGRAM_HEAD_AUTOMATON

#include "DependencyPart.h"
#include "ad3/GenericFactor.h"

namespace AD3 {

class FactorTrigramHeadAutomaton : public GenericFactor {
 public:
  FactorTrigramHeadAutomaton() {}
  virtual ~FactorTrigramHeadAutomaton() { ClearActiveSet(); }

  // Print as a string.
  void Print(ostream& stream) {
    stream << "TRIGRAM_HEAD_AUTOMATON";
    Factor::Print(stream);
    int total = 0; // Delete this later.
    for (int m = 0; m < index_siblings_.size(); ++m) {
      for (int s = m+1; s <= index_siblings_.size(); ++s) {
        //CHECK_GE(index_siblings_[m][s], 0);
        int index = index_siblings_[m][s];
  if (index >= 0) {
    stream << " " << setprecision(9) << additional_log_potentials_[index];
    ++total;
  } else {
    stream << " " << setprecision(9) << 0.0;
  }    
      }
    }
    for (int m = 0; m < index_siblings_.size(); ++m) {
      for (int s = m+1; s < index_siblings_.size(); ++s) {
        for (int t = s+1; t <= index_siblings_.size(); ++t) {
          //CHECK_GE(index_trisiblings_[m][s][t], 0);
          int index = index_trisiblings_[m][s][t];
    if (index >= 0) {
      stream << " " << setprecision(9) << additional_log_potentials_[index];
      ++total;
    } else {
      stream << " " << setprecision(9) << 0.0;
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
    // Decode using the Viterbi algorithm.
    int length = length_;
    vector<vector<vector<double> > > values(length);
    vector<vector<vector<int> > > path(length);
    // Note: states are now pairs of indices (e.g. std::pair<int,int>(j,i));
    // however, we only need to store the older index j to backtrack.
    // The start state is (0,1).

    assert(length > 0);
    values[0].resize(1);
    path[0].resize(1);
    //values[0][0].push_back(0.0);
    //path[0][0].push_back(-1); // This is state (-1,0).
    for (int m = 1; m < length; ++m) {
      // There are (m+1)-choose-2 possible states.
      // We can either keep the previous state (no arc added)
      // or transition to a new state (arc between h and m).
      values[m].resize(m+1);
      path[m].resize(m+1);
      for (int i = 0; i < m; ++i) {
        values[m][i].resize(i);
        path[m][i].resize(i);
        for (int j = 0; j < i; ++j) {
          // In this case, the previous state must also be (j,i).
          assert(i < values[m-1].size());
          assert(j < values[m-1][i].size());
          values[m][i][j] = values[m-1][i][j];
          path[m][i][j] = j; // This is state (j,i).
          /*
          cout << "path[" << m << "][" << i << "][" << j << "] = "
               << j << endl;
          */
        }
      }

      // For the (i,m)-th state, the previous state can be (j,i),
      // for some -1 <= j < i <= m-1.
      values[m][m].resize(m);
      path[m][m].resize(m);
      for (int i = 0; i < m; ++i) {
        path[m][m][i] = -1;
        values[m][m][i] = 0.0;
        for (int j = 0; j < i; ++j) {
          double score = values[m-1][i][j];
          int index = index_trisiblings_[j][i][m];
          if (index >= 0) score += additional_log_potentials[index];
          if (path[m][m][i] < 0 || score > values[m][m][i]) {
            values[m][m][i] = score;
            path[m][m][i] = j; // This is state (j,i).
          }
        }
        int index = index_siblings_[i][m];
        if (index > 0) values[m][m][i] += additional_log_potentials[index];
        values[m][m][i] += variable_log_potentials[m-1];
        /*
        cout << "path[" << m << "][" << m << "][" << i << "] = "
             << path[m][m][i] << endl;
        */
      }
    }

    // The end state is (i,length) with 0 <= i < length.
    vector<int> best_path(length);
    best_path[length-1] = -1;
    for (int i = 0; i < length; ++i) {
      int best = -1;
      double best_score = 0.0;
      for (int j = 0; j < i; ++j) {
        double score = values[length-1][i][j];
        int index = index_trisiblings_[j][i][length];
        if (index >= 0) score += additional_log_potentials[index];
        if (best < 0 || score > best_score) {
          best_score = score;
          best = j;
        }
      }
      double score = best_score;
      int index = index_siblings_[i][length];
      if (index >= 0) score += additional_log_potentials[index];
      if (best_path[length-1] < 0 || score > (*value)) {
        *value = score;
        best_path[length-1] = i;
        if (length > 1) {
          best_path[length-2] = best;
        }
      }
    }
    /*
    cout << "best_path[" << length-1 << "] = " << best_path[length-1] << endl;
    if (length > 1) {
      cout << "best_path[" << length-2 << "] = " << best_path[length-2] << endl;
    }
    */
    int i = best_path[length-1];
    int j = (length>1)? best_path[length-2] : -1;

    for (int m = length-1; m > 0; --m) {
      //CHECK_GE(best_path[m], -1); // can be -1...
      //CHECK_LT(best_path[m], path[m].size());
      //CHECK_GE(best_path[m-1], -1); // can be -1...
      //CHECK_LT(best_path[m-1], path[m][best_path[m]].size()) << m
      //                                                       << " " << best_path[m];

      // Current state is j=best_path[m-1], i=best_path[m].
      if (m == i) {
        CHECK_GE(j, 0);
        i = j;
        j = path[m][m][i];
      }
      best_path[m-1] = i;
      //cout << "best_path[" << m-1 << "] = " << best_path[m-1] << endl;

      /*
      // path[2][1][1] does not exist...
      if (best_path[m-1] == -1) {
        best_path[m-2] = -1;
      } else {
        //best_path[m-2] = path[m][best_path[m]][best_path[m-1]];
        int j = path[m][best_path[m]][best_path[m-1]];
        int i = (m != best_path[m])? best_path[m-1] : m;
      }
      cout << "best_path[" << m-2 << "] = " << best_path[m-2] << endl;
      */
    }

    vector<int> *modifiers = static_cast<vector<int>*>(configuration);
    for (int m = 1; m < length; ++m) {
      if (best_path[m] == m) {
        modifiers->push_back(m);
      }
    }
    //cout << "End Maximizing" << endl;

#if 0
    double value2;
    Evaluate(variable_log_potentials,
             additional_log_potentials,
             configuration,
             &value2);
    cout << *value << " " << value2 << endl;
#endif

#if 0
    vector<vector<int> > modseqs;
    double best_value;
    int best = -1;
    if (false && length == 5) {
      int config1[] = {};
      modseqs.push_back(vector<int>(config1, config1));
      int config2[] = {3};
      modseqs.push_back(vector<int>(config2, config2+1));
      int config3[] = {2};
      modseqs.push_back(vector<int>(config3, config3+1));
      int config4[] = {2,3};
      modseqs.push_back(vector<int>(config4, config4+2));
      int config5[] = {1};
      modseqs.push_back(vector<int>(config5, config5+1));
      int config6[] = {1,3};
      modseqs.push_back(vector<int>(config6, config6+2));
      int config7[] = {1,2};
      modseqs.push_back(vector<int>(config7, config7+2));
      int config8[] = {1,2,3};
      modseqs.push_back(vector<int>(config8, config8+3));
      int config9[] = {4};
      modseqs.push_back(vector<int>(config9, config9+1));
      int config10[] = {3,4};
      modseqs.push_back(vector<int>(config10, config10+2));
      int config11[] = {2,4};
      modseqs.push_back(vector<int>(config11, config11+2));
      int config12[] = {2,3,4};
      modseqs.push_back(vector<int>(config12, config12+3));
      int config13[] = {1,4};
      modseqs.push_back(vector<int>(config13, config13+2));
      int config14[] = {1,3,4};
      modseqs.push_back(vector<int>(config14, config14+3));
      int config15[] = {1,2,4};
      modseqs.push_back(vector<int>(config15, config15+3));
      int config16[] = {1,2,3,4};
      modseqs.push_back(vector<int>(config16, config16+4));
      for (int i = 0; i < modseqs.size(); ++i) {
        double value_tmp;
        Evaluate(variable_log_potentials,
                 additional_log_potentials,
                 &modseqs[i],
                 &value_tmp);
        if (best < 0 || value_tmp > best_value) {
          best = i;
          best_value = value_tmp;
        }
      }
      cout << *value << "->" << best_value << endl;
    } else if (false && length == 4) {
      int config1[] = {};
      modseqs.push_back(vector<int>(config1, config1));
      int config2[] = {3};
      modseqs.push_back(vector<int>(config2, config2+1));
      int config3[] = {2};
      modseqs.push_back(vector<int>(config3, config3+1));
      int config4[] = {2,3};
      modseqs.push_back(vector<int>(config4, config4+2));
      int config5[] = {1};
      modseqs.push_back(vector<int>(config5, config5+1));
      int config6[] = {1,3};
      modseqs.push_back(vector<int>(config6, config6+2));
      int config7[] = {1,2};
      modseqs.push_back(vector<int>(config7, config7+2));
      int config8[] = {1,2,3};
      modseqs.push_back(vector<int>(config8, config8+3));
      for (int i = 0; i < modseqs.size(); ++i) {
        double value_tmp;
        Evaluate(variable_log_potentials,
                 additional_log_potentials,
                 &modseqs[i],
                 &value_tmp);
        if (best < 0 || value_tmp > best_value) {
          best = i;
          best_value = value_tmp;
        }
      }
      cout << *value << "->" << best_value << endl;
    } else if (false && length == 3) {
      int config1[] = {};
      modseqs.push_back(vector<int>(config1, config1));
      int config3[] = {2};
      modseqs.push_back(vector<int>(config3, config3+1));
      int config5[] = {1};
      modseqs.push_back(vector<int>(config5, config5+1));
      int config7[] = {1,2};
      modseqs.push_back(vector<int>(config7, config7+2));
      for (int i = 0; i < modseqs.size(); ++i) {
        double value_tmp;
        Evaluate(variable_log_potentials,
                 additional_log_potentials,
                 &modseqs[i],
                 &value_tmp);
        if (best < 0 || value_tmp > best_value) {
          best = i;
          best_value = value_tmp;
        }
      }
    }
#endif
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
    int s = 0;
    for (int i = 0; i < modifiers->size(); ++i) {
      int t = (*modifiers)[i];
      *value += variable_log_potentials[t-1];
      int index = index_siblings_[s][t];
      if (index >= 0) *value += additional_log_potentials[index];
      if (s != 0) {
        int index = index_trisiblings_[m][s][t];
        if (index >= 0) *value += additional_log_potentials[index];
      }
      m = s;
      s = t;
    }
    int t = index_siblings_.size();
    int index = index_siblings_[s][t];
    if (index >= 0) *value += additional_log_potentials[index];
    if (s != 0) {
      int index = index_trisiblings_[m][s][t];
      if (index >= 0) *value += additional_log_potentials[index];
    }
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
    int s = 0;
    for (int i = 0; i < modifiers->size(); ++i) {
      int t = (*modifiers)[i];
      (*variable_posteriors)[t-1] += weight;
      int index = index_siblings_[s][t];
      if (index >= 0) (*additional_posteriors)[index] += weight;
      if (s != 0) {
        int index = index_trisiblings_[m][s][t];
        if (index >= 0) (*additional_posteriors)[index] += weight;
      }
      m = s;
      s = t;
    }
    int t = index_siblings_.size();
    int index = index_siblings_[s][t];
    if (index >= 0) (*additional_posteriors)[index] += weight;
    if (s != 0) {
      int index = index_trisiblings_[m][s][t];
      if (index >= 0) (*additional_posteriors)[index] += weight;
    }
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
                  const vector<DependencyPartTriSibl*> &trisiblings) {
    vector<DependencyPartNextSibl*> siblings;
    Initialize(arcs, siblings, trisiblings);
  }

  void Initialize(const vector<DependencyPartArc*> &arcs,
                  const vector<DependencyPartNextSibl*> &siblings,
                  const vector<DependencyPartTriSibl*> &trisiblings) {
    length_ = arcs.size() + 1;
    index_siblings_.assign(length_, vector<int>(length_+1, -1));
    index_trisiblings_.assign(length_,
                              vector<vector<int> >(length_,
                                                   vector<int>(length_+1, -1)));

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

    for (int k = 0; k < trisiblings.size(); ++k) {
      if (arcs.size() > 0) CHECK_EQ(h, trisiblings[k]->head());
      h = trisiblings[k]->head();
      m = trisiblings[k]->modifier();
      int s = trisiblings[k]->sibling();
      int t = trisiblings[k]->other_sibling();
      if (arcs.size() > 0) CHECK_EQ(s > h, right);
      if (arcs.size() > 0) CHECK_EQ(t > h, right);
      right = (s > h)? true : false;
      int position_modifier = right? m - h : h - m;
      int position_sibling = right? s - h : h - s;
      int position_other_sibling = right? t - h : h - t;
      CHECK_LT(position_modifier, index_modifiers.size());
      int index_modifier = index_modifiers[position_modifier];
      // Assume sibling is not denoting the stop symbol (only
      // other_sibling can be the stop symbol).
      CHECK_LT(position_sibling, index_modifiers.size());
      int index_sibling = (position_sibling < index_modifiers.size())?
          index_modifiers[position_sibling] : length_;
      int index_other_sibling = (position_other_sibling < index_modifiers.size())?
          index_modifiers[position_other_sibling] : length_;
      CHECK_GE(index_modifier, 0);
      CHECK_LT(index_modifier, length_);
      CHECK_GE(index_sibling, 1) << h << " " << m << " " << s;
      //CHECK_LT(index_sibling, length_+1);
      CHECK_LT(index_sibling, length_);
      CHECK_GE(index_other_sibling, 1) << h << " " << m << " " << s;
      CHECK_LT(index_other_sibling, length_+1);
      // Add an offset to save room for the siblings.
      index_trisiblings_[index_modifier][index_sibling][index_other_sibling] =
        siblings.size() + k;
    }
  }

 private:
  int length_;
  vector<vector<int> > index_siblings_;
  vector<vector<vector<int> > > index_trisiblings_;
};

} // namespace AD3

#endif // FACTOR_TRIGRAM_HEAD_AUTOMATON
