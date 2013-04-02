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

#include "Options.h"
#include <glog/logging.h>

using namespace std;

DEFINE_string(file_train, "",
              "Path to the file containing the training data.");
DEFINE_string(file_test, "",
              "Path to the file containing the test data.");
DEFINE_string(file_model, "",
              "Path to the file containing the model.");
DEFINE_string(file_prediction, "",
              "Path to the file where the predictions are output.");
DEFINE_bool(train, false,
            "True for training the parser.");
DEFINE_bool(test, false,
            "True for testing the parser.");
DEFINE_bool(evaluate, false,
            "True for evaluating the parser (requires --test).");
DEFINE_string(train_algorithm, "svm_mira",
             "Training algorithm. Options are perceptron, mira, svm_mira,"
             "crf_mira, svm_sgd, crf_sgd.");
DEFINE_double(train_initial_learning_rate, 0.01,
              "Initial learning rate (for SGD only).");
DEFINE_string(train_learning_rate_schedule, "invsqrt",
              "Learning rate annealing schedule (for SGD only). Options are "
              "fixed, lecun, invsqrt, inv.");
DEFINE_bool(only_supported_features, false,
            "True for using supported features only (should be true for CRFs).");
DEFINE_bool(use_averaging, true,
            "True for averaging the weight vector at the end of training.");
DEFINE_int32(train_epochs, 10,
             "Number of training epochs.");
DEFINE_double(train_regularization_constant, 1e12,
              "Regularization parameter C.");
DEFINE_int32(parameters_max_num_buckets, 50000000,
           "Maximum number of buckets in the hash table that stores the parameters.");

void Options::Initialize() {
  file_train_ = FLAGS_file_train;
  file_test_ = FLAGS_file_test;
  file_model_ = FLAGS_file_model;
  file_prediction_ = FLAGS_file_prediction;
  train_ = FLAGS_train;
  test_ = FLAGS_test;
  evaluate_ = FLAGS_evaluate;
  train_epochs_ = FLAGS_train_epochs;
  train_regularization_constant_ = FLAGS_train_regularization_constant;
  train_algorithm_ = FLAGS_train_algorithm;
  train_initial_learning_rate_ = FLAGS_train_initial_learning_rate;
  train_learning_rate_schedule_ = FLAGS_train_learning_rate_schedule;
  only_supported_features_ = FLAGS_only_supported_features;
  use_averaging_ = FLAGS_use_averaging;
}

