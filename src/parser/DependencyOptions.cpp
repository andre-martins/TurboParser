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

#include "DependencyOptions.h"
#include "StringUtils.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

// TODO: Implement the text format.
DEFINE_string(file_format, "conll",
              "Format of the input file containing the data. Use ""conll"" for "
              "the format used in CONLL-X, and ""text"" for tokenized sentences"
              "(one per line, with tokens separated by white-spaces.");
DEFINE_string(model_type, "standard",
              "Model type. This a string formed by the one or several of the "
              "following pieces:"
              "af enables arc-factored parts (required), "
              "+cs enables consecutive sibling parts, "
              "+gp enables grandparent parts,"
              "+as enables arbitrary sibling parts,"
              "+np enables non-projectivity parts,"
              "+dp enables directed path parts,"
              "+hb enables head bigram parts."
              "The following alias are predefined:"
              "basic is af, "
              "standard is af+cs+gp, "
              "full is af+cs+gp+as+hb. "
              "NOTE: dp and np are currently not recommended as they make the "
              "parser significantly slower");
DEFINE_bool(large_feature_set, true,
            "True for using a large feature set. Parsers are usually more "
            "accurate but slower and have a larger memory footprint.");
DEFINE_bool(labeled, true,
            "True for training an parser with labeled arcs (if false, the "
            "parser outputs just the backbone dependencies.)");
DEFINE_bool(prune_labels, true,
            "True for pruning the set of possible labels taking into account "
            "the labels that have occured for each pair of POS tags in the "
            "training data.");
DEFINE_bool(prune_distances, true,
            "True for pruning the set of possible left/right distances taking "
            "into account the distances that have occured for each pair of POS "
            "tags in the training data.");
DEFINE_bool(prune_basic, true,
            "True for using a basic pruner from a probabilistic first-order "
            "model.");
DEFINE_bool(use_pretrained_pruner, false,
            "True if using a pre-trained basic pruner. Must specify the file "
            "path through --file_pruner_model. If this flag is set to false "
            "and train=true and prune_basic=true, a pruner will be trained "
            "along with the parser.");
DEFINE_string(file_pruner_model, "",
              "Path to the file containing the pre-trained pruner model. Must "
              "activate the flag --use_pretrained_pruner");
DEFINE_double(pruner_posterior_threshold, 0.0001,
            "Posterior probability threshold for an arc to be pruned, in basic "
            "pruning. For each  word m, if "
            "P(h,m) < pruner_posterior_threshold * P(h',m), "
            "where h' is the best scored head, then (h,m) will be pruned out.");
DEFINE_int32(pruner_max_heads, 10,
            "Maximum number of possible head words for a given word, in basic "
            "pruning.");

// Options for pruner training.
// TODO: implement these options.
DEFINE_string(pruner_train_algorithm, "crf_mira",
             "Training algorithm for the pruner. Options are perceptron, mira, "
             "svm_mira, crf_mira, svm_sgd, crf_sgd.");
DEFINE_bool(pruner_only_supported_features, true,
            "True for the pruner to use supported features only (should be true" 
            "for CRFs).");
DEFINE_bool(pruner_use_averaging, true,
            "True for the pruner to average the weight vector at the end of" 
            "training.");
DEFINE_int32(pruner_train_epochs, 10,
             "Number of training epochs for the pruner.");
DEFINE_double(pruner_train_regularization_constant, 0.001,
              "Regularization parameter C for the pruner.");
DEFINE_bool(pruner_labeled, false,
            "True if pruner is a labeled parser. Currently, must be set to false.");
DEFINE_double(pruner_train_initial_learning_rate, 0.01,
              "Initial learning rate of pruner (for SGD only).");
DEFINE_string(pruner_train_learning_rate_schedule, "invsqrt",
              "Learning rate annealing schedule of pruner (for SGD only). "
              "Options are fixed, lecun, invsqrt, inv.");
DEFINE_bool(pruner_large_feature_set, false,
            "True for using a large feature set. Parsers are usually more "
            "accurate but slower and have a larger memory footprint.");

// TODO: Implement the tagger within the parser.
// DEFINE_bool(train_tagger, true,
//             "True if training a tagger along with the parser.");
//

// Save current option flags to the model file.
void DependencyOptions::Save(FILE* fs) {
  Options::Save(fs);

  bool success;
  success = WriteString(fs, model_type_);
  CHECK(success);
  success = WriteBool(fs, large_feature_set_);
  CHECK(success);
  success = WriteBool(fs, labeled_);
  CHECK(success);
  success = WriteBool(fs, prune_labels_);
  CHECK(success);
  success = WriteBool(fs, prune_distances_);
  CHECK(success);
  success = WriteBool(fs, prune_basic_);
  CHECK(success);
  success = WriteDouble(fs, pruner_posterior_threshold_);
  CHECK(success);
  success = WriteInteger(fs, pruner_max_heads_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void DependencyOptions::Load(FILE* fs) {
  Options::Load(fs);

  bool success;
  success = ReadString(fs, &FLAGS_model_type);
  CHECK(success);
  LOG(INFO) << "Setting --model_type=" << FLAGS_model_type;
  success = ReadBool(fs, &FLAGS_large_feature_set);
  CHECK(success);
  LOG(INFO) << "Setting --large_feature_set=" << FLAGS_large_feature_set;
  success = ReadBool(fs, &FLAGS_labeled);
  CHECK(success);
  LOG(INFO) << "Setting --labeled=" << FLAGS_labeled;
  success = ReadBool(fs, &FLAGS_prune_labels);
  CHECK(success);
  LOG(INFO) << "Setting --prune_labels=" << FLAGS_prune_labels;
  success = ReadBool(fs, &FLAGS_prune_distances);
  CHECK(success);
  LOG(INFO) << "Setting --prune_distances=" << FLAGS_prune_distances;
  success = ReadBool(fs, &FLAGS_prune_basic);
  CHECK(success);
  LOG(INFO) << "Setting --prune_basic=" << FLAGS_prune_basic;
  success = ReadDouble(fs, &FLAGS_pruner_posterior_threshold);
  CHECK(success);
  LOG(INFO) << "Setting --pruner_posterior_threshold="
            << FLAGS_pruner_posterior_threshold;
  success = ReadInteger(fs, &FLAGS_pruner_max_heads);
  CHECK(success);
  LOG(INFO) << "Setting --pruner_max_heads=" << FLAGS_pruner_max_heads;

  Initialize();
}

void DependencyOptions::CopyPrunerFlags() {
  // Flags from base class Options.
  FLAGS_train_algorithm = FLAGS_pruner_train_algorithm;
  FLAGS_only_supported_features = FLAGS_pruner_only_supported_features;
  FLAGS_use_averaging = FLAGS_pruner_use_averaging;
  FLAGS_train_epochs = FLAGS_pruner_train_epochs;
  FLAGS_train_regularization_constant = FLAGS_pruner_train_regularization_constant;
  FLAGS_train_initial_learning_rate = FLAGS_pruner_train_initial_learning_rate;
  FLAGS_train_learning_rate_schedule = FLAGS_pruner_train_learning_rate_schedule;

  // Flags from DependencyOptions.
  CHECK(!FLAGS_pruner_labeled) << "Currently, the flag --pruner_labeled must be false.";
  FLAGS_labeled = FLAGS_pruner_labeled;
  FLAGS_large_feature_set = FLAGS_pruner_large_feature_set;

  // General flags.
  FLAGS_model_type = "af"; // A pruner is always a arc-factored model.
  FLAGS_prune_basic = false; // A pruner has no inner basic pruner.
}

void DependencyOptions::Initialize() {
  Options::Initialize();

  file_format_ = FLAGS_file_format;
  model_type_ = FLAGS_model_type;
  large_feature_set_ = FLAGS_large_feature_set;
  labeled_ = FLAGS_labeled;
  prune_labels_ = FLAGS_prune_labels;
  prune_distances_ = FLAGS_prune_distances;
  prune_basic_ = FLAGS_prune_basic;
  use_pretrained_pruner_ = FLAGS_use_pretrained_pruner;
  file_pruner_model_ = FLAGS_file_pruner_model;
  pruner_posterior_threshold_ = FLAGS_pruner_posterior_threshold;
  pruner_max_heads_ = FLAGS_pruner_max_heads;

  use_arbitrary_siblings_ = false;
  use_consecutive_siblings_ = false;
  use_grandparents_ = false;
  use_nonprojective_arcs_ = false;
  use_directed_paths_ = false;
  use_head_bigrams_ = false;

  // Enable the parts corresponding to the model type.
  string model_type = FLAGS_model_type;
  if (model_type == "basic") {
    model_type = "af";
  } else if (model_type == "standard") {
    model_type = "af+cs+gp";
  } else if (model_type == "full") {
    model_type = "af+cs+gp+as+hb";
  }
  vector<string> enabled_parts;
  bool use_arc_factored = false;
  StringSplit(model_type, "+", &enabled_parts);
  for (int i = 0; i < enabled_parts.size(); ++i) {
    if (enabled_parts[i] == "af") {
      use_arc_factored = true;
      LOG(INFO) << "Arc factored parts enabled.";
    } else if (enabled_parts[i] == "cs") {
      use_consecutive_siblings_ = true;
      LOG(INFO) << "Consecutive sibling parts enabled.";
    } else if (enabled_parts[i] == "gp") {
      use_grandparents_ = true;
      LOG(INFO) << "Grandparent parts enabled.";
    } else if (enabled_parts[i] == "as") {
      use_arbitrary_siblings_ = true;
      LOG(INFO) << "Arbitrary sibling parts enabled.";
    } else if (enabled_parts[i] == "np") {
      use_nonprojective_arcs_ = true;
      LOG(INFO) << "Nonprojective arc parts enabled.";
    } else if (enabled_parts[i] == "dp") {
      use_directed_paths_ = true;
      LOG(INFO) << "Directed path parts enabled.";
    } else if (enabled_parts[i] == "hb") {
      use_head_bigrams_ = true;
      LOG(INFO) << "Head bigram parts enabled.";
    } else {
      CHECK(false) << "Unknown part in model type: " << enabled_parts[i];
    }
  }

  CHECK(use_arc_factored) << "Arc-factored parts are mandatory in model type";
}


