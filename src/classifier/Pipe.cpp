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

#include "Pipe.h"
#include "Utils.h"
#include <math.h>
#include <iostream>
#include <sstream>

Pipe::Pipe(Options* options) {
  options_ = options;
  dictionary_ = NULL;
  reader_ = NULL;
  writer_ = NULL;
  decoder_ = NULL;
  parameters_ = NULL;
}

Pipe::~Pipe() {
  delete dictionary_;
  delete reader_;
  delete writer_;
  delete decoder_;
  delete parameters_;
}

void Pipe::Initialize() {
  CreateDictionary();
  CreateReader();
  CreateWriter();
  CreateDecoder();
  parameters_ = new Parameters;
}

void Pipe::SaveModelByName(const string &model_name) {
  FILE *fs = fopen(model_name.c_str(), "wb");
  CHECK(fs) << "Could not open model file for writing: " << model_name;
  SaveModel(fs);
  fclose(fs);
}

void Pipe::LoadModelByName(const string &model_name) {
  FILE *fs = fopen(model_name.c_str(), "rb");
  CHECK(fs) << "Could not open model file for reading: " << model_name;
  LoadModel(fs);
  fclose(fs);
}

void Pipe::SaveModel(FILE* fs) {
  options_->Save(fs);
  dictionary_->Save(fs);
  parameters_->Save(fs);
}

void Pipe::LoadModel(FILE* fs) {
  options_->Load(fs);
  dictionary_->Load(fs);
  parameters_->Load(fs);
}

// TODO: Implement ComputeScores as follows:
//
// void Pipe::ComputeScores(Places *places, Features *features,
//                                  vector<vector<int> > *labels,
//                                  vector<vector<double> > *scores);
void Pipe::ComputeScores(Instance *instance, Parts *parts, Features *features,
                         vector<double> *scores) {
  scores->resize(parts->size());
  for (int r = 0; r < parts->size(); ++r) {
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    (*scores)[r] = parameters_->ComputeScore(part_features);
  }
}

void Pipe::MakeGradientStep(Parts *parts, Features *features, double eta,
                            int iteration,
                            const vector<double> &gold_output,
                            const vector<double> &predicted_output) {
  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    parameters_->MakeGradientStep(part_features, eta, iteration,
      predicted_output[r] - gold_output[r]);
  }
}

void Pipe::TouchParameters(Parts *parts, Features *features,
                           const vector<bool> &selected_parts) {
  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    parameters_->MakeGradientStep(part_features, 0.0, 0, 0.0);
  }
}

void Pipe::MakeFeatureDifference(Parts *parts,
                                 Features *features,
                                 const vector<double> &gold_output,
                                 const vector<double> &predicted_output,
                                 FeatureVector *difference) {
  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    for (int j = 0; j < part_features.size(); ++j) {
      difference->mutable_weights()->Add(part_features[j],
                                         predicted_output[r] - gold_output[r]);
    }
  }
}

void Pipe::RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                     const vector<bool> &selected_parts,
                                     Features *features) {
  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;
    BinaryFeatures *part_features = features->GetMutablePartFeatures(r);
    int num_supported = 0;
    for (int j = 0; j < part_features->size(); ++j) {
      if (parameters_->Exists((*part_features)[j])) {
        (*part_features)[num_supported] = (*part_features)[j];
        ++num_supported;
      }
    }
    part_features->resize(num_supported);
  }
}

void Pipe::Train() {

  PreprocessData();
  CreateInstances();
  parameters_->Initialize(options_->use_averaging());

  if (options_->only_supported_features()) MakeSupportedParameters();

  for (int i = 0; i < options_->GetNumEpochs(); ++i) {
    TrainEpoch(i);
  }

  parameters_->Finalize(options_->GetNumEpochs() * instances_.size());
}

void Pipe::CreateInstances() {
  timeval start, end;
  gettimeofday(&start, NULL);

  LOG(INFO) << "Creating instances...";

  reader_->Open(options_->GetTrainingFilePath());
  DeleteInstances();
  Instance *instance = reader_->GetNext();
  while (instance) {
    AddInstance(instance);
    instance = reader_->GetNext();
  }
  reader_->Close();

  LOG(INFO) << "Number of instances: " << instances_.size();

  gettimeofday(&end, NULL);
  LOG(INFO) << "Time: " << diff_ms(end,start);
}

void Pipe::MakeSupportedParameters() {
  Parts *parts = CreateParts();
  Features *features = CreateFeatures();
  vector<double> gold_outputs;

  LOG(INFO) << "Building supported feature set...";

  dictionary_->StopGrowth();
  parameters_->AllowGrowth();
  for (int i = 0; i < instances_.size(); i++) {
    Instance *instance = instances_[i];
    MakeParts(instance, parts, &gold_outputs);
    vector<bool> selected_parts(gold_outputs.size(), false);
    for (int r = 0; r < gold_outputs.size(); ++r) {
      if (gold_outputs[r] > 0.5) {
        selected_parts[r] = true;
      }
    }
    MakeSelectedFeatures(instance, parts, selected_parts, features);
    TouchParameters(parts, features, selected_parts);
  }

  delete parts;
  delete features;
  parameters_->StopGrowth();

  LOG(INFO) << "Number of Features: " << parameters_->Size();
}

void Pipe::TrainEpoch(int epoch) {
  Instance *instance;
  Parts *parts = CreateParts();
  Features *features = CreateFeatures();
  vector<double> scores;
  vector<double> gold_outputs;
  vector<double> predicted_outputs;
  double total_cost = 0.0;
  double total_loss = 0.0;
  double eta;
  int num_instances = instances_.size();
  double lambda = 1.0/(options_->GetRegularizationConstant() *
                       (static_cast<double>(num_instances)));
  timeval start, end;
  gettimeofday(&start, NULL);
  int time_decoding = 0;
  int time_scores = 0;
  int num_mistakes = 0;

  LOG(INFO) << " Iteration #" << epoch + 1;

  dictionary_->StopGrowth();

  for (int i = 0; i < instances_.size(); i++) {
    int t = num_instances * epoch + i;
    instance = instances_[i];
    MakeParts(instance, parts, &gold_outputs);
    MakeFeatures(instance, parts, features);

    // If using only supported features, must remove the unsupported ones.
    // This is necessary not to mess up the computation of the squared norm
    // of the feature difference vector in MIRA.
    if (options_->only_supported_features()) {
      RemoveUnsupportedFeatures(instance, parts, features);
    }

    timeval start_scores, end_scores;
    gettimeofday(&start_scores, NULL);
    ComputeScores(instance, parts, features, &scores);
    gettimeofday(&end_scores, NULL);
    time_scores += diff_ms(end_scores, start_scores);

    // This is a no-op by default. But it's convenient to have it here to build
    // latent-variable structured classifiers (e.g. for coreference resolution).
    double inner_loss = 0.0;
    TransformGold(instance, parts, scores, &gold_outputs, &inner_loss);

    if (options_->GetTrainingAlgorithm() == "perceptron" ||
        options_->GetTrainingAlgorithm() == "mira" ) {
      timeval start_decoding, end_decoding;
      gettimeofday(&start_decoding, NULL);
      decoder_->Decode(instance, parts, scores, &predicted_outputs);
      gettimeofday(&end_decoding, NULL);
      time_decoding += diff_ms(end_decoding, start_decoding);

      if (options_->GetTrainingAlgorithm() == "perceptron") {
        for (int r = 0; r < parts->size(); ++r) {
          if (!NEARLY_EQ_TOL(gold_outputs[r], predicted_outputs[r], 1e-6)) {
            ++num_mistakes;
          }
        }
        eta = 1.0;
      } else {
        CHECK(false) << "Plain mira is not implemented yet.";
      }

      MakeGradientStep(parts, features, eta, t, gold_outputs,
                       predicted_outputs);

    } else if (options_->GetTrainingAlgorithm() == "svm_mira" ||
               options_->GetTrainingAlgorithm() == "crf_mira" ||
               options_->GetTrainingAlgorithm() == "crf_margin_mira" ||
               options_->GetTrainingAlgorithm() == "svm_sgd" ||
               options_->GetTrainingAlgorithm() == "crf_sgd" ||
               options_->GetTrainingAlgorithm() == "crf_margin_sgd") {
      double loss;
      timeval start_decoding, end_decoding;
      gettimeofday(&start_decoding, NULL);
      if (options_->GetTrainingAlgorithm() == "svm_mira" ||
          options_->GetTrainingAlgorithm() == "svm_sgd") {
        // Do cost-augmented inference.
        double cost;
        decoder_->DecodeCostAugmented(instance, parts, scores, gold_outputs,
                                      &predicted_outputs, &cost, &loss);
        total_cost += cost;
      } else if (options_->GetTrainingAlgorithm() == "crf_margin_mira" ||
                 options_->GetTrainingAlgorithm() == "crf_margin_sgd") {
        // Do cost-augmented marginal inference.
        double entropy;
        double cost;
        decoder_->DecodeCostAugmentedMarginals(instance, parts, scores,
                                               gold_outputs, &predicted_outputs,
                                               &entropy, &cost, &loss);
        total_cost += cost;
      } else {
        // Do marginal inference.
        double entropy;
        decoder_->DecodeMarginals(instance, parts, scores, gold_outputs,
                                  &predicted_outputs, &entropy, &loss);
        CHECK_GE(entropy, 0.0);
      }
      gettimeofday(&end_decoding, NULL);
      time_decoding += diff_ms(end_decoding, start_decoding);

      loss -= inner_loss;
      if (loss < 0.0) {
        if (!NEARLY_EQ_TOL(loss, 0.0, 1e-9)) {
          LOG(INFO) << "Warning: negative loss set to zero: " << loss;
        }
        loss = 0.0;
      }
      total_loss += loss;

      // Compute difference between predicted and gold feature vectors.
      FeatureVector difference;
      MakeFeatureDifference(parts, features, gold_outputs, predicted_outputs,
                            &difference);

      // Get the stepsize.
      if (options_->GetTrainingAlgorithm() == "svm_mira" ||
          options_->GetTrainingAlgorithm() == "crf_mira" ||
          options_->GetTrainingAlgorithm() == "crf_margin_mira") {
        double squared_norm = difference.GetSquaredNorm();
        double threshold = 1e-9;
        if (loss < threshold || squared_norm < threshold) {
          eta = 0.0;
        } else {
          eta = loss / squared_norm;
          if (eta > options_->GetRegularizationConstant()) {
            eta = options_->GetRegularizationConstant();
          }
        }
      } else {
        if (options_->GetLearningRateSchedule() == "fixed") {
          eta = options_->GetInitialLearningRate();
        } else if (options_->GetLearningRateSchedule() == "invsqrt") {
          eta = options_->GetInitialLearningRate() /
            sqrt(static_cast<double>(t+1));
        } else if (options_->GetLearningRateSchedule() == "inv") {
          eta = options_->GetInitialLearningRate() /
            static_cast<double>(t+1);
        } else if (options_->GetLearningRateSchedule() == "lecun") {
          eta = options_->GetInitialLearningRate() /
            (1.0 + (static_cast<double>(t) / static_cast<double>(num_instances)));
        } else {
          CHECK(false) << "Unknown learning rate schedule: "
                       << options_->GetLearningRateSchedule();
        }

        // Scale the parameter vector (only for SGD).
        double decay = 1 - eta * lambda;
        CHECK_GT(decay, 0.0);
        parameters_->Scale(decay);
      }

      MakeGradientStep(parts, features, eta, t, gold_outputs,
                       predicted_outputs);
    } else {
      CHECK(false) << "Unknown algorithm: " << options_->GetTrainingAlgorithm();
    }
  }

  // Compute the regularization value (halved squared L2 norm of the weights).
  double regularization_value =
      lambda * static_cast<double>(num_instances) *
      parameters_->GetSquaredNorm() / 2.0;

  delete parts;
  delete features;

  gettimeofday(&end, NULL);
  LOG(INFO) << "Time: " << diff_ms(end,start);
  LOG(INFO) << "Time to score: " << time_scores;
  LOG(INFO) << "Time to decode: " << time_decoding;
  LOG(INFO) << "Number of Features: " << parameters_->Size();
  if (options_->GetTrainingAlgorithm() == "perceptron" ||
      options_->GetTrainingAlgorithm() == "mira") {
    LOG(INFO) << "Number of mistakes: " << num_mistakes;
  }
  LOG(INFO) << "Total Cost: " << total_cost << "\t"
            << "Total Loss: " << total_loss << "\t"
            << "Total Reg: " << regularization_value << "\t"
            << "Total Loss+Reg: " << total_loss + regularization_value << endl;
}

void Pipe::Run() {
  Parts *parts = CreateParts();
  Features *features = CreateFeatures();
  vector<double> scores;
  vector<double> gold_outputs;
  vector<double> predicted_outputs;

  timeval start, end;
  gettimeofday(&start, NULL);

  if (options_->evaluate()) BeginEvaluation();

  reader_->Open(options_->GetTestFilePath());
  writer_->Open(options_->GetOutputFilePath());

  int num_instances = 0;
  Instance *instance = reader_->GetNext();
  while (instance) {
    Instance *formatted_instance = GetFormattedInstance(instance);

    MakeParts(formatted_instance, parts, &gold_outputs);
    MakeFeatures(formatted_instance, parts, features);
    ComputeScores(formatted_instance, parts, features, &scores);
    decoder_->Decode(formatted_instance, parts, scores, &predicted_outputs);

    Instance *output_instance = instance->Copy();
    LabelInstance(parts, predicted_outputs, output_instance);

    if (options_->evaluate()) {
      EvaluateInstance(instance, output_instance,
                       parts, gold_outputs, predicted_outputs);
    }

    writer_->Write(output_instance);

    if (formatted_instance != instance) delete formatted_instance;
    delete output_instance;
    delete instance;

    instance = reader_->GetNext();
    ++num_instances;
  }

  delete parts;
  delete features;

  writer_->Close();
  reader_->Close();

  gettimeofday(&end, NULL);
  LOG(INFO) << "Number of instances: " << num_instances;
  LOG(INFO) << "Time: " << diff_ms(end,start);

  if (options_->evaluate()) EndEvaluation();
}

void Pipe::ClassifyInstance(Instance *instance) {
  Parts *parts = CreateParts();
  Features *features = CreateFeatures();
  std::vector<double> gold_outputs;
  std::vector<double> predicted_outputs;
  std::vector<double> scores;

  Instance *formatted_instance = GetFormattedInstance(instance);

  MakeParts(formatted_instance, parts, &gold_outputs);
  MakeFeatures(formatted_instance, parts, features);
  ComputeScores(formatted_instance, parts, features, &scores);
  decoder_->Decode(formatted_instance, parts, scores, &predicted_outputs);

  LabelInstance(parts, predicted_outputs, instance);

  if (formatted_instance != instance) delete formatted_instance;

  delete parts;
  delete features;
}
