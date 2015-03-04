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

#ifndef DEPENDENCY_OPTIONS_H_
#define DEPENDENCY_OPTIONS_H_

#include "Options.h"

class DependencyOptions : public Options {
 public:
  DependencyOptions() {};
  virtual ~DependencyOptions() {};

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
  bool large_feature_set() { return large_feature_set_; };
  bool labeled() { return labeled_; }
  bool projective() { return projective_; }
  bool prune_labels() { return prune_labels_; }
  bool prune_distances() { return prune_distances_; }
  bool prune_basic() { return prune_basic_; }
  bool use_pretrained_pruner() { return use_pretrained_pruner_; }
  const string &GetPrunerModelFilePath() { return file_pruner_model_; }
  double GetPrunerPosteriorThreshold() { return pruner_posterior_threshold_; }
  double GetPrunerMaxHeads() { return pruner_max_heads_; }

  bool use_arbitrary_siblings() { return use_arbitrary_siblings_; }
  bool use_consecutive_siblings() { return use_consecutive_siblings_; }
  bool use_grandparents() { return use_grandparents_; }
  bool use_grandsiblings() { return use_grandsiblings_; }
  bool use_trisiblings() { return use_trisiblings_; }
  bool use_nonprojective_arcs() { return use_nonprojective_arcs_; }
  bool use_directed_paths() { return use_directed_paths_; }
  bool use_head_bigrams() { return use_head_bigrams_; }

 protected:
  string file_format_;
  string model_type_;
  bool large_feature_set_;
  bool labeled_;
  bool projective_;
  bool prune_labels_;
  bool prune_distances_;
  bool prune_basic_;
  bool use_pretrained_pruner_;
  string file_pruner_model_;
  double pruner_posterior_threshold_;
  int pruner_max_heads_;
  bool use_arbitrary_siblings_;
  bool use_consecutive_siblings_;
  bool use_grandparents_;
  bool use_grandsiblings_;
  bool use_trisiblings_;
  bool use_nonprojective_arcs_;
  bool use_directed_paths_;
  bool use_head_bigrams_;
};

#endif // DEPENDENCY_OPTIONS_H_
