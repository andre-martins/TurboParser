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

#include "SemanticOptions.h"
#include "StringUtils.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

// TODO: Implement the text format.
DEFINE_string(srl_file_format, "conll",
              "Format of the input file containing the data. Use ""conll"" for "
              "the format used in CONLL 2008, ""sdp"" for the format in "
              "SemEval 2014, and ""text"" for tokenized sentences"
              "(one per line, with tokens separated by white-spaces.");
DEFINE_string(srl_model_type, "standard",
              "Model type. This a string formed by the one or several of the "
              "following pieces:"
              "af enables arc-factored parts (required), "
              "+as enables arbitrary sibling parts,"
              "+cs enables consecutive sibling parts, "
              "+gp enables grandparent parts,"
              "+cp enables co-parent parts,"
              "+ccp enables consecutive co-parent parts, "
              "+gs enables grandsibling parts,"
              "+ts enables trisibling parts,"
              "+gs enables grand-sibling (third-order) parts,"
              "+ts enables tri-sibling (third-order) parts."
              "The following alias are predefined:"
              "basic is af, "
              "standard is af+cs+gp, "
              "full is af+cs+gp+as+gs+ts.");
DEFINE_bool(srl_use_dependency_syntactic_features, true,
            "True for using features from the dependency syntactic tree. "
            "This should be false for the closed track in SemEval 2014.");
DEFINE_bool(srl_labeled, true,
            "True for training a parser with labeled arcs (if false, the "
            "parser outputs just the backbone dependencies.)");
DEFINE_bool(srl_deterministic_labels, true,
            "True for forcing a set of labels (found in the training set) to be "
            "deterministic (i.e. to not occur in more than one argument for the "
            "same predicate).");
DEFINE_bool(srl_allow_self_loops, true,
            "True for allowing self-loops (a predicate being its own argument.)");
DEFINE_bool(srl_allow_root_predicate, false,
            "True for allowing the root to be a predicate (useful for handling "
            "top nodes).)");
DEFINE_bool(srl_allow_unseen_predicates, false,
            "True for allowing an unseen predicate to be have a predicate sense "
            "(assumes --use_predicate_senses=true.)");
DEFINE_bool(srl_use_predicate_senses, true,
            "True for using predicate senses (e.g. temperature.01). If false, "
            "any word can be a predicate and (eventual) sense information will "
            "be ignored.");
DEFINE_bool(srl_prune_labels, true,
            "True for pruning the set of possible labels taking into account "
            "the labels that have occured for each pair of POS tags in the "
            "training data.");
DEFINE_bool(srl_prune_labels_with_relation_paths, false, //true,
            "True for pruning the set of possible labels taking into account "
            "the labels that have occured for syntactic dependency relation "
            "paths in the training data.");
DEFINE_bool(srl_prune_distances, true,
            "True for pruning the set of possible left/right distances taking "
            "into account the distances that have occured for each pair of POS "
            "tags in the training data.");
DEFINE_bool(srl_prune_basic, true,
            "True for using a basic pruner from a probabilistic first-order "
            "model.");
DEFINE_bool(srl_use_pretrained_pruner, false,
            "True if using a pre-trained basic pruner. Must specify the file "
            "path through --file_pruner_model. If this flag is set to false "
            "and train=true and prune_basic=true, a pruner will be trained "
            "along with the parser.");
DEFINE_string(srl_file_pruner_model, "",
              "Path to the file containing the pre-trained pruner model. Must "
              "activate the flag --use_pretrained_pruner");
DEFINE_double(srl_pruner_posterior_threshold, 0.0001,
            "Posterior probability threshold for an arc to be pruned, in basic "
            "pruning. For each word p, if "
            "P(p,a) < pruner_posterior_threshold * P(p,a'), "
            "where a' is the best scored argument, then (p,a) will be pruned out.");
DEFINE_int32(srl_pruner_max_arguments, 20,
            "Maximum number of possible arguments for a given word, in basic "
            "pruning.");

// Options for pruner training.
// TODO: implement these options.
DEFINE_string(srl_pruner_train_algorithm, "crf_mira",
             "Training algorithm for the pruner. Options are perceptron, mira, "
             "svm_mira, crf_mira, svm_sgd, crf_sgd.");
DEFINE_bool(srl_pruner_only_supported_features, true,
            "True for the pruner to use supported features only (should be true" 
            "for CRFs).");
DEFINE_bool(srl_pruner_use_averaging, true,
            "True for the pruner to average the weight vector at the end of"
            "training.");
DEFINE_int32(srl_pruner_train_epochs, 10,
             "Number of training epochs for the pruner.");
DEFINE_double(srl_pruner_train_regularization_constant, 0.001,
              "Regularization parameter C for the pruner.");
DEFINE_bool(srl_pruner_labeled, false,
            "True if pruner is a labeled parser. Currently, must be set to false.");
DEFINE_double(srl_pruner_train_initial_learning_rate, 0.01,
              "Initial learning rate of pruner (for SGD only).");
DEFINE_string(srl_pruner_train_learning_rate_schedule, "invsqrt",
              "Learning rate annealing schedule of pruner (for SGD only). "
              "Options are fixed, lecun, invsqrt, inv.");

// Save current option flags to the model file.
void SemanticOptions::Save(FILE* fs) {
  Options::Save(fs);

  bool success;
  success = WriteString(fs, model_type_);
  CHECK(success);
  success = WriteBool(fs, use_dependency_syntactic_features_);
  CHECK(success);
  success = WriteBool(fs, labeled_);
  CHECK(success);
  success = WriteBool(fs, deterministic_labels_);
  CHECK(success);
  success = WriteBool(fs, allow_self_loops_);
  CHECK(success);
  success = WriteBool(fs, allow_root_predicate_);
  CHECK(success);
  success = WriteBool(fs, allow_unseen_predicates_);
  CHECK(success);
  success = WriteBool(fs, use_predicate_senses_);
  CHECK(success);
  success = WriteBool(fs, prune_labels_);
  CHECK(success);
  success = WriteBool(fs, prune_labels_with_relation_paths_);
  CHECK(success);
  success = WriteBool(fs, prune_distances_);
  CHECK(success);
  success = WriteBool(fs, prune_basic_);
  CHECK(success);
  success = WriteDouble(fs, pruner_posterior_threshold_);
  CHECK(success);
  success = WriteInteger(fs, pruner_max_arguments_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void SemanticOptions::Load(FILE* fs) {
  Options::Load(fs);

  bool success;
  success = ReadString(fs, &FLAGS_srl_model_type);
  CHECK(success);
  LOG(INFO) << "Setting --srl_model_type=" << FLAGS_srl_model_type;
  success = ReadBool(fs, &FLAGS_srl_use_dependency_syntactic_features);
  CHECK(success);
  LOG(INFO) << "Setting --srl_use_dependency_syntactic_features="
            << FLAGS_srl_use_dependency_syntactic_features;
  success = ReadBool(fs, &FLAGS_srl_labeled);
  CHECK(success);
  LOG(INFO) << "Setting --srl_labeled=" << FLAGS_srl_labeled;
  success = ReadBool(fs, &FLAGS_srl_deterministic_labels);
  CHECK(success);
  LOG(INFO) << "Setting --srl_deterministic_labels="
            << FLAGS_srl_deterministic_labels;
  success = ReadBool(fs, &FLAGS_srl_allow_self_loops);
  CHECK(success);
  LOG(INFO) << "Setting --srl_allow_self_loops=" << FLAGS_srl_allow_self_loops;
  success = ReadBool(fs, &FLAGS_srl_allow_root_predicate);
  CHECK(success);
  LOG(INFO) << "Setting --srl_allow_root_predicate="
            << FLAGS_srl_allow_root_predicate;
  success = ReadBool(fs, &FLAGS_srl_allow_unseen_predicates);
  CHECK(success);
  LOG(INFO) << "Setting --srl_allow_unseen_predicates="
            << FLAGS_srl_allow_unseen_predicates;
  success = ReadBool(fs, &FLAGS_srl_use_predicate_senses);
  CHECK(success);
  LOG(INFO) << "Setting --srl_use_predicate_senses="
            << FLAGS_srl_use_predicate_senses;
  success = ReadBool(fs, &FLAGS_srl_prune_labels);
  CHECK(success);
  LOG(INFO) << "Setting --srl_prune_labels=" << FLAGS_srl_prune_labels;
  success = ReadBool(fs, &FLAGS_srl_prune_labels_with_relation_paths);
  CHECK(success);
  LOG(INFO) << "Setting --srl_prune_labels_with_relation_paths="
            << FLAGS_srl_prune_labels_with_relation_paths;
  success = ReadBool(fs, &FLAGS_srl_prune_distances);
  CHECK(success);
  LOG(INFO) << "Setting --srl_prune_distances=" << FLAGS_srl_prune_distances;
  success = ReadBool(fs, &FLAGS_srl_prune_basic);
  CHECK(success);
  LOG(INFO) << "Setting --srl_prune_basic=" << FLAGS_srl_prune_basic;
  success = ReadDouble(fs, &FLAGS_srl_pruner_posterior_threshold);
  CHECK(success);
  LOG(INFO) << "Setting --srl_pruner_posterior_threshold="
            << FLAGS_srl_pruner_posterior_threshold;
  success = ReadInteger(fs, &FLAGS_srl_pruner_max_arguments);
  CHECK(success);
  LOG(INFO) << "Setting --srl_pruner_max_arguments="
            << FLAGS_srl_pruner_max_arguments;

  Initialize();
}

void SemanticOptions::CopyPrunerFlags() {
  // Flags from base class Options.
  FLAGS_train_algorithm = FLAGS_srl_pruner_train_algorithm;
  FLAGS_only_supported_features = FLAGS_srl_pruner_only_supported_features;
  FLAGS_use_averaging = FLAGS_srl_pruner_use_averaging;
  FLAGS_train_epochs = FLAGS_srl_pruner_train_epochs;
  FLAGS_train_regularization_constant =
    FLAGS_srl_pruner_train_regularization_constant;
  FLAGS_train_initial_learning_rate =
    FLAGS_srl_pruner_train_initial_learning_rate;
  FLAGS_train_learning_rate_schedule =
    FLAGS_srl_pruner_train_learning_rate_schedule;

  // Flags from SemanticOptions.
  CHECK(!FLAGS_srl_pruner_labeled)
    << "Currently, the flag --srl_pruner_labeled must be false.";
  FLAGS_srl_labeled = FLAGS_srl_pruner_labeled;

  // General flags.
  FLAGS_srl_model_type = "af"; // A pruner is always a arc-factored model.
  FLAGS_srl_prune_basic = false; // A pruner has no inner basic pruner.
  // A pruner does not impose deterministic labels.
  FLAGS_srl_deterministic_labels = false;
}

void SemanticOptions::Initialize() {
  Options::Initialize();

  file_format_ = FLAGS_srl_file_format;
  model_type_ = FLAGS_srl_model_type;
  use_dependency_syntactic_features_ =
    FLAGS_srl_use_dependency_syntactic_features;
  labeled_ = FLAGS_srl_labeled;
  deterministic_labels_ = FLAGS_srl_deterministic_labels;
  allow_self_loops_ = FLAGS_srl_allow_self_loops;
  allow_root_predicate_ = FLAGS_srl_allow_root_predicate;
  allow_unseen_predicates_ = FLAGS_srl_allow_unseen_predicates;
  use_predicate_senses_ = FLAGS_srl_use_predicate_senses;
  prune_labels_ = FLAGS_srl_prune_labels;
  prune_labels_with_relation_paths_ =
    FLAGS_srl_prune_labels_with_relation_paths;
  prune_distances_ = FLAGS_srl_prune_distances;
  prune_basic_ = FLAGS_srl_prune_basic;
  use_pretrained_pruner_ = FLAGS_srl_use_pretrained_pruner;
  file_pruner_model_ = FLAGS_srl_file_pruner_model;
  pruner_posterior_threshold_ = FLAGS_srl_pruner_posterior_threshold;
  pruner_max_arguments_ = FLAGS_srl_pruner_max_arguments;

  use_arbitrary_siblings_ = false;
  use_consecutive_siblings_ = false;
  use_grandparents_ = false;
  use_coparents_ = false;
  use_consecutive_coparents_ = false;
  use_grandsiblings_ = false;
  use_trisiblings_ = false;

  // Enable the parts corresponding to the model type.
  string model_type = FLAGS_srl_model_type;
  if (model_type == "basic") {
    model_type = "af";
  } else if (model_type == "standard") {
    model_type = "af+cs+gp";
  } else if (model_type == "full") {
    model_type = "af+cs+gp+as+hb+gs+ts";
  }
  vector<string> enabled_parts;
  bool use_arc_factored = false;
  StringSplit(model_type, "+", &enabled_parts);
  for (int i = 0; i < enabled_parts.size(); ++i) {
    if (enabled_parts[i] == "af") {
      use_arc_factored = true;
      LOG(INFO) << "Arc factored parts enabled.";
    } else if (enabled_parts[i] == "as") {
      use_arbitrary_siblings_ = true;
      LOG(INFO) << "Arbitrary sibling parts enabled.";
    } else if (enabled_parts[i] == "cs") {
      use_consecutive_siblings_ = true;
      LOG(INFO) << "Consecutive sibling parts enabled.";
    } else if (enabled_parts[i] == "gp") {
      use_grandparents_ = true;
      LOG(INFO) << "Grandparent parts enabled.";
    } else if (enabled_parts[i] == "cp") {
      use_coparents_ = true;
      LOG(INFO) << "Co-parent parts enabled.";
    } else if (enabled_parts[i] == "ccp") {
      use_consecutive_coparents_ = true;
      LOG(INFO) << "Consecutive co-parent parts enabled.";
    } else if (enabled_parts[i] == "gs") {
      use_grandsiblings_ = true;
      LOG(INFO) << "Grandsibling parts enabled.";
    } else if (enabled_parts[i] == "ts") {
      use_trisiblings_ = true;
      LOG(INFO) << "Trisibling parts enabled.";
    } else {
      CHECK(false) << "Unknown part in model type: " << enabled_parts[i];
    }
  }

  CHECK(use_arc_factored) << "Arc-factored parts are mandatory in model type";
}
