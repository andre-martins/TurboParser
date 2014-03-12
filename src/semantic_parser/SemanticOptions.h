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

#ifndef SEMANTIC_OPTIONS_H_
#define SEMANTIC_OPTIONS_H_

#include "Options.h"

class SemanticOptions : public Options {
 public:
  SemanticOptions() {};
  virtual ~SemanticOptions() {};

  // Serialization functions.
  void Load(FILE* fs);
  void Save(FILE* fs);

  // Initialization: set options based on the flags.
  void Initialize();

  // Replace the flags by the pruner flags. This will overwrite some
  // of the flags. This function is called when training the pruner
  // along with the parser (rather than using an external pruner).
  void CopyPrunerFlags();

  // Get option values.
  const string &file_format() { return file_format_; }
  bool labeled() { return labeled_; }
  bool deterministic_labels() { return deterministic_labels_; }
  bool use_dependency_syntactic_features() {
    return use_dependency_syntactic_features_;
  }
  bool allow_self_loops() { return allow_self_loops_; }
  bool allow_root_predicate() { return allow_root_predicate_; }
  bool allow_unseen_predicates() { return allow_unseen_predicates_; }
  bool use_predicate_senses() { return use_predicate_senses_; }
  bool prune_labels() { return prune_labels_; }
  bool prune_labels_with_relation_paths() {
    return prune_labels_with_relation_paths_;
  }
  bool prune_distances() { return prune_distances_; }
  bool prune_basic() { return prune_basic_; }
  bool use_pretrained_pruner() { return use_pretrained_pruner_; }
  const string &GetPrunerModelFilePath() { return file_pruner_model_; }
  double GetPrunerPosteriorThreshold() { return pruner_posterior_threshold_; }
  double GetPrunerMaxArguments() { return pruner_max_arguments_; }

  bool use_arbitrary_siblings() { return use_arbitrary_siblings_; }
  bool use_grandparents() { return use_grandparents_; }
  bool use_coparents() { return use_coparents_; }
  bool use_consecutive_siblings() { return use_consecutive_siblings_; }
  bool use_grandsiblings() { return use_grandsiblings_; }
  bool use_trisiblings() { return use_trisiblings_; }

 protected:
  string file_format_;
  string model_type_;
  bool use_dependency_syntactic_features_;
  bool labeled_;
  bool deterministic_labels_;
  bool allow_self_loops_;
  bool allow_root_predicate_;
  bool allow_unseen_predicates_;
  bool use_predicate_senses_;
  bool prune_labels_;
  bool prune_labels_with_relation_paths_;
  bool prune_distances_;
  bool prune_basic_;
  bool use_pretrained_pruner_;
  string file_pruner_model_;
  double pruner_posterior_threshold_;
  int pruner_max_arguments_;
  bool use_arbitrary_siblings_;
  bool use_grandparents_;
  bool use_coparents_;
  bool use_consecutive_siblings_;
  bool use_grandsiblings_;
  bool use_trisiblings_;
};

#endif // SEMANTIC_OPTIONS_H_
