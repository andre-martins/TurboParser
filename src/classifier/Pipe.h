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

#ifndef PIPE_H_
#define PIPE_H_

#include "Dictionary.h"
#include "Features.h"
#include "Part.h"
#include "Reader.h"
#include "Writer.h"
#include "Options.h"
#include "Decoder.h"
#include "Parameters.h"
#include "AlgUtils.h"

// Abstract class for the structured classifier mainframe.
// It requires parts, features, a dictionary, a reader and writer, and
// instances, all of which are abstract classes.
// Task-specific classifiers should derive from this class and implement the
// pure virtual methods.
class Pipe {
 public:
  // Constructor/destructor.
  Pipe() {};
  Pipe(Options* options);
  virtual ~Pipe();

  // Save/load the model to/from a file.
  void SaveModelFile() { SaveModelByName(options_->GetModelFilePath()); }
  void LoadModelFile() { LoadModelByName(options_->GetModelFilePath()); }

  // Initialize. Override this method for task-specific initialization.
  virtual void Initialize();

  // Get options.
  Options* GetOptions() { return options_; };

  // Get/Set parameters.
  Parameters *GetParameters() { return parameters_; }
  void SetParameters(Parameters *parameters) { parameters_ = parameters; }

  // Train the classifier.
  void Train();

  // Run a previously trained classifier on new data.
  void Run();

  // Run a previously trained classifier on a single instance.
  void ClassifyInstance(Instance *instance);

 protected:
  // Create basic objects.
  virtual void CreateDictionary() = 0;
  virtual void CreateReader() = 0;
  virtual void CreateWriter() = 0;
  virtual void CreateDecoder() = 0;
  virtual Parts *CreateParts() = 0;
  virtual Features *CreateFeatures() = 0;

  // Save/load model.
  void SaveModelByName(const string &model_name);
  void LoadModelByName(const string &model_name);
  virtual void SaveModel(FILE* fs);
  virtual void LoadModel(FILE* fs);

  // Create/add/delete instances.
  void DeleteInstances() {
    for (int i = 0; i < instances_.size(); ++i) {
      delete instances_[i];
    }
    instances_.clear();
  }
  void AddInstance(Instance *instance) {
    Instance *formatted_instance = GetFormattedInstance(instance);
    instances_.push_back(formatted_instance);
    if (instance != formatted_instance) delete instance;
  }

  // Obtain a "formatted" instance. Override this function for task-specific
  // formatted instances, which may be different from instance since they
  // may have extra information, data in numeric format for faster
  // processing, etc.
  virtual Instance *GetFormattedInstance(Instance *instance) {
    return instance;
  }

  // Create a vector of instances by reading the training data.
  void CreateInstances();

  // Construct the vector of parts for a particular instance.
  // Eventually, obtain the binary vector of gold outputs (one entry per part)
  // if this information is available.
  // Note: this function is task-specific and needs to be implemented by the
  // deriving class.
  virtual void MakeParts(Instance *instance, Parts *parts,
                         vector<double> *gold_outputs) = 0;

  // Construct the vector of features for a particular instance and given the
  // parts. The vector will be of the same size as the vector of parts.
  void MakeFeatures(Instance *instance, Parts *parts, Features *features) {
    vector<bool> selected_parts(parts->size(), true);
    MakeSelectedFeatures(instance, parts, selected_parts, features);
  }

  // Construct the vector of features for a particular instance and given a
  // selected set of parts (parts which are selected as marked as true). The
  // vector will be of the same size as the vector of parts.
  // Note: this function is task-specific and needs to be implemented by the
  // deriving class.
  virtual void MakeSelectedFeatures(Instance *instance, Parts *parts,
      const vector<bool> &selected_parts, Features *features) = 0;

  // Given an instance, parts, and features, compute the scores. This will
  // look at the current parameters. Each part will receive a score, so the
  // vector of scores will be of the same size as the vector of parts.
  // NOTE: Override this method for task-specific score computation (e.g.
  // to handle labeled features, etc.).
  // TODO: handle labeled features here instead of having to override.
  virtual void ComputeScores(Instance *instance, Parts *parts,
                             Features *features,
                             vector<double> *scores);

  // Perform a gradient step with stepsize eta. The iteration number is
  // provided as input since it may be necessary to keep track of the averaged
  // weights. The gold output and the predicted output are also provided.
  // The meaning of "predicted_output" depends on the training algorithm.
  // In perceptron, it is the most likely output predicted by the model.
  // In cost-augmented MIRA and structured SVMs, it is the cost-augmented
  // prediction.
  // In CRFs, it is the vector of posterior marginals for the parts.
  // TODO: use "FeatureVector *difference" as input (see function
  // MakeFeatureDifference(...) instead of computing on the fly).
  virtual void MakeGradientStep(Parts *parts, Features *features, double eta,
                                int iteration,
                                const vector<double> &gold_output,
                                const vector<double> &predicted_output);

  // Compute the difference between the predicted feature vector and the gold
  // one.
  // The meaning of "predicted_output" depends on the training algorithm.
  // In perceptron, it is the most likely output predicted by the model.
  // In cost-augmented MIRA and structured SVMs, it is the cost-augmented
  // prediction.
  // In CRFs, it is the vector of posterior marginals for the parts.
  virtual void MakeFeatureDifference(Parts *parts,
                                   Features *features,
                                   const vector<double> &gold_output,
                                   const vector<double> &predicted_output,
                                   FeatureVector *difference);

  // Given an instance, a vector of parts, and features for those parts,
  // remove all the features which are not supported, i.e., that were not
  // previously created in the parameter vector. This is used for training
  // with supported features (flag --only_supported_features).
  void RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                 Features *features) {
    vector<bool> selected_parts(parts->size(), true);
    RemoveUnsupportedFeatures(instance, parts, selected_parts, features);
  }

  // Given an instance, a vector of selected parts, and features for those
  // parts, remove all the features which are not supported. See description
  // above.
  virtual void RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                         const vector<bool> &selected_parts,
                                         Features *features);

  // Given a vector of parts, and features for those parts, "touch" all the
  // parameters corresponding to those features. This will be a no-op for the
  // parameters that exist already, and will create a parameter with a zero
  // weight otherwise. This is used in a preprocessing stage for training
  // with supported features (flag --only_supported_features).
  virtual void TouchParameters(Parts *parts, Features *features,
                               const vector<bool> &selected_parts);

  // Given a vector of parts of a desired output, builds the output information
  // in the instance that corresponds to that output.
  // Note: this function is task-specific and needs to be implemented by the
  // deriving class.
  virtual void LabelInstance(Parts *parts, const vector<double> &output,
                             Instance *instance) = 0;

  // Preprocess an instance before training begins. Override this function for
  // task-specific instance preprocessing.
  virtual void PreprocessInstance(Instance* instance) {};

  // Preprocess the data before training begins. Override this function for
  // task-specific instance preprocessing.
  virtual void PreprocessData() {};

  // Build and lock a parameter vector with only supported parameters, by
  // looking at the gold outputs in the training data. This is a preprocessing
  // stage for training with supported features (flag
  // --only_supported_features).
  void MakeSupportedParameters();

  // Run one epoch of training.
  void TrainEpoch(int epoch);

  // Start all the evaluation counters for evaluating the classifier,
  // evaluate each instance, and plot evaluation information at the end.
  // This is done at test time when the flag --evaluate is activated.
  // The version implemented here plots accuracy based on Hamming distance
  // for the predicted and gold parts. Override this function for
  // task-specific evaluation.
  virtual void BeginEvaluation() { num_mistakes_ = 0; num_total_parts_ = 0; }
  virtual void EvaluateInstance(Instance *instance,
                                Instance *output_instance,
                                Parts *parts,
                                const vector<double> &gold_outputs,
                                const vector<double> &predicted_outputs) {
    for (int r = 0; r < parts->size(); ++r) {
      if (!NEARLY_EQ_TOL(gold_outputs[r], predicted_outputs[r], 1e-6)) {
        ++num_mistakes_;
      }
      ++num_total_parts_;
    }
  }
  virtual void EndEvaluation() {
    LOG(INFO) << "Accuracy (parts): " <<
      static_cast<double>(num_total_parts_ - num_mistakes_) /
        static_cast<double>(num_total_parts_);
  }

 protected:
  Options *options_; // Classifier options.
  Dictionary *dictionary_; // Dictionary for the classifier.
  Reader *reader_; // Reader for reading instances from a file.
  Writer* writer_; // Writer for writing instance to a file.
  Decoder* decoder_; // Decoder for this classification task.
  Parameters *parameters_; // Parameter vector.
  vector<Instance*> instances_; // Set of training instances.

  // Number of mistakes and number of total parts at test time (used for
  // evaluation purposes).
  int num_mistakes_;
  int num_total_parts_;
};

#endif /* PIPE_H_ */
