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

#ifndef FACTOR_SEMANTIC_GRAPH_H
#define FACTOR_SEMANTIC_GRAPH_H

#include "SemanticDecoder.h"
#include "ad3/GenericFactor.h"

namespace AD3 {

class FactorSemanticGraph : public GenericFactor {
 public:
  FactorSemanticGraph() {}
  virtual ~FactorSemanticGraph() {
    if (own_parts_) {
      for (int r = 0; r < arcs_.size(); ++r) {
        delete arcs_[r];
      }
      for (int r = 0; r < predicate_parts_.size(); ++r) {
        delete predicate_parts_[r];
      }
    }
    ClearActiveSet();
  }

  // Print as a string.
  void Print(ostream& stream) {
    stream << "SEMANTIC_GRAPH";
    Factor::Print(stream);
    stream << " " << length_;
    stream << " " << predicate_parts_.size();
    stream << " " << arcs_.size();
    for (int k = 0; k < predicate_parts_.size(); ++k) {
      int p = predicate_parts_[k]->predicate();
      int s = predicate_parts_[k]->sense();
      stream << " " << p << " " << s;
    }
    for (int k = 0; k < arcs_.size(); ++k) {
      int p = arcs_[k]->predicate();
      int a = arcs_[k]->argument();
      int s = arcs_[k]->sense();
      stream << " " << p << " " << a << " " << s;
    }
    stream << endl;
  }

  // Compute the score of a given assignment.
  // Note: additional_log_potentials is empty and is ignored.
  void Maximize(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                Configuration &configuration,
                double *value) {
    vector<bool> *selected_parts = static_cast<vector<bool>*>(configuration);
    int num_predicate_parts = predicate_parts_.size();
    int num_arcs = arcs_.size();
    CHECK_EQ(num_predicate_parts + num_arcs, selected_parts->size());
    vector<bool> selected_predicates;
    vector<bool> selected_arcs;
    vector<double> predicate_scores(variable_log_potentials.begin(),
                                    variable_log_potentials.begin() +
                                    num_predicate_parts);
    vector<double> arc_scores(variable_log_potentials.begin() +
                              num_predicate_parts,
                              variable_log_potentials.end());

    decoder_->DecodeSemanticGraph(length_, predicate_parts_, arcs_,
                                  index_predicates_, arcs_by_predicate_,
                                  predicate_scores, arc_scores,
                                  &selected_predicates, &selected_arcs, value);

    for (int r = 0; r < num_predicate_parts; ++r) {
      (*selected_parts)[r] = selected_predicates[r];
    }
    for (int r = 0; r < num_arcs; ++r) {
      (*selected_parts)[num_predicate_parts + r] = selected_arcs[r];
    }
  }

  // Compute the score of a given assignment.
  // Note: additional_log_potentials is empty and is ignored.
  void Evaluate(const vector<double> &variable_log_potentials,
                const vector<double> &additional_log_potentials,
                const Configuration configuration,
                double *value) {
    const vector<bool> *selected_parts =
      static_cast<vector<bool>*>(configuration);
    *value = 0.0;
    for (int r = 0; r < selected_parts->size(); ++r) {
      int index = r;
      if ((*selected_parts)[r]) *value += variable_log_potentials[index];
    }
  }

  // Given a configuration with a probability (weight),
  // increment the vectors of variable and additional posteriors.
  // Note: additional_log_potentials is empty and is ignored.
  void UpdateMarginalsFromConfiguration(
    const Configuration &configuration,
    double weight,
    vector<double> *variable_posteriors,
    vector<double> *additional_posteriors) {
    const vector<bool> *selected_parts =
      static_cast<vector<bool>*>(configuration);
    for (int r = 0; r < selected_parts->size(); ++r) {
      int index = r;
      if ((*selected_parts)[r]) (*variable_posteriors)[index] += weight;
    }
  }

  // Count how many common values two configurations have.
  int CountCommonValues(const Configuration &configuration1,
                        const Configuration &configuration2) {
    const vector<bool> *selected_parts1 =
      static_cast<vector<bool>*>(configuration1);
    const vector<bool> *selected_parts2 =
      static_cast<vector<bool>*>(configuration2);
    CHECK_EQ(selected_parts1->size(), selected_parts2->size());
    int count = 0;
    for (int r = 0; r < selected_parts1->size(); ++r) {
      if ((*selected_parts1)[r] && (*selected_parts2)[r]) {
        ++count;
      }
    }
    return count;
  }

  // Check if two configurations are the same.
  bool SameConfiguration(
    const Configuration &configuration1,
    const Configuration &configuration2) {
    const vector<bool> *selected_parts1 =
      static_cast<vector<bool>*>(configuration1);
    const vector<bool> *selected_parts2 =
      static_cast<vector<bool>*>(configuration2);
    CHECK_EQ(selected_parts1->size(), selected_parts2->size());
    for (int r = 0; r < selected_parts1->size(); ++r) {
      if ((*selected_parts1)[r] != (*selected_parts2)[r]) {
        return false;
      }
    }
    return true;
  }

  // Delete configuration.
  void DeleteConfiguration(
    Configuration configuration) {
    vector<bool> *selected_parts = static_cast<vector<bool>*>(configuration);
    delete selected_parts;
  }

  // Create configuration.
  Configuration CreateConfiguration() {
    int num_predicate_parts = predicate_parts_.size();
    int num_arcs = arcs_.size();
    vector<bool> *selected_parts = new vector<bool>(num_predicate_parts +
                                                    num_arcs);
    return static_cast<Configuration>(selected_parts);
  }

 public:
  void Initialize(int length,
                  const vector<SemanticPartPredicate*> &predicate_parts,
                  const vector<SemanticPartArc*> &arcs,
                  SemanticDecoder *decoder,
                  bool own_parts = false) {
    own_parts_ = own_parts;
    length_ = length;
    predicate_parts_ = predicate_parts;
    arcs_ = arcs;
    decoder_ = decoder;

    decoder_->BuildBasicIndices(length_, predicate_parts_, arcs_,
                                &index_predicates_, &arcs_by_predicate_);
  }

 private:
  bool own_parts_;
  int length_; // Sentence length (including root symbol).
  vector<vector<int> > index_predicates_;
  vector<vector<vector<int> > > arcs_by_predicate_;
  vector<SemanticPartPredicate*> predicate_parts_;
  vector<SemanticPartArc*> arcs_;
  SemanticDecoder *decoder_;
};

} // namespace AD3

#endif // FACTOR_SEMANTIC_GRAPH_H

