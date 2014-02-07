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

#ifndef SEMANTICPIPE_H_
#define SEMANTICPIPE_H_

#include "Pipe.h"
#include "SemanticOptions.h"
#include "SemanticReader.h"
#include "SemanticDictionary.h"
#include "TokenDictionary.h"
#include "SemanticInstanceNumeric.h"
#include "SemanticWriter.h"
#include "SemanticPart.h"
#include "SemanticFeatures.h"
#include "SemanticDecoder.h"

class SemanticPipe : public Pipe {
 public:
  SemanticPipe(Options* options) : Pipe(options) {
    token_dictionary_ = NULL;
    pruner_parameters_ = NULL;
    train_pruner_ = false;
  }
  virtual ~SemanticPipe() {
    delete token_dictionary_;
    delete pruner_parameters_;
  }

  SemanticReader *GetSemanticReader() {
    return static_cast<SemanticReader*>(reader_);
  }
  SemanticDictionary *GetSemanticDictionary() {
    return static_cast<SemanticDictionary*>(dictionary_);
  }
  SemanticDecoder *GetSemanticDecoder() {
    return static_cast<SemanticDecoder*>(decoder_);
  }
  SemanticOptions *GetSemanticOptions() {
    return static_cast<SemanticOptions*>(options_);
  }

  void Initialize() {
    Pipe::Initialize();
    pruner_parameters_ = new Parameters;
  }

  void SetPrunerParameters(Parameters *pruner_parameters) {
    pruner_parameters_ = pruner_parameters;
  }
  void LoadPrunerModelFile() {
    LoadPrunerModelByName(GetSemanticOptions()->GetPrunerModelFilePath());
  }

 protected:
  void CreateDictionary() {
    dictionary_ = new SemanticDictionary(this);
    GetSemanticDictionary()->SetTokenDictionary(token_dictionary_);
  };
  void CreateReader() { reader_ = new SemanticReader; };
  void CreateWriter() { writer_ = new SemanticWriter; };
  void CreateDecoder() { decoder_ = new SemanticDecoder(this); };
  Parts *CreateParts() { return new SemanticParts; };
  Features *CreateFeatures() { return new SemanticFeatures(this); };

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  };

  Parameters *GetTrainingParameters() {
    if (train_pruner_) return pruner_parameters_;
    return parameters_;
  }

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    SemanticInstanceNumeric *instance_numeric =
          new SemanticInstanceNumeric;
    instance_numeric->Initialize(*GetSemanticDictionary(),
                                 static_cast<SemanticInstance*>(instance));
    return instance_numeric;
  }

  void SaveModel(FILE* fs);
  void LoadModel(FILE* fs);

  void LoadPrunerModel(FILE* fs);
  void LoadPrunerModelByName(const string &model_name);

  void MakeParts(Instance *instance, Parts *parts,
                 vector<double> *gold_outputs);
  void MakePartsBasic(Instance *instance, Parts *parts,
                      vector<double> *gold_outputs);
  void MakePartsBasic(Instance *instance, bool add_labeled_parts, Parts *parts,
                      vector<double> *gold_outputs);
  void MakePartsGlobal(Instance *instance, Parts *parts,
                       vector<double> *gold_outputs);

  void MakeFeatures(Instance *instance, Parts *parts, bool pruner,
                    Features *features) {
    vector<bool> selected_parts(parts->size(), true);
    MakeSelectedFeatures(instance, parts, pruner, selected_parts, features);
  }
  void MakeSelectedFeatures(Instance *instance, Parts *parts,
      const vector<bool>& selected_parts, Features *features) {
    // Set pruner = false unless we're training the pruner.
    MakeSelectedFeatures(instance, parts, train_pruner_, selected_parts,
                         features);
  }
  void MakeSelectedFeatures(Instance *instance,
                            Parts *parts,
                            bool pruner,
                            const vector<bool>& selected_parts,
                            Features *features);

  void ComputeScores(Instance *instance, Parts *parts, Features *features,
                     vector<double> *scores) {
    // Set pruner = false unless we're training the pruner.
    ComputeScores(instance, parts, features, train_pruner_, scores);
  }
  void ComputeScores(Instance *instance, Parts *parts, Features *features,
                     bool pruner, vector<double> *scores);

  void RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                 bool pruner,
                                 const vector<bool> &selected_parts,
                                 Features *features);

  void RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                 const vector<bool> &selected_parts,
                                 Features *features) {
    // Set pruner = false unless we're training the pruner.
    RemoveUnsupportedFeatures(instance, parts, train_pruner_, selected_parts,
                              features);
  }

  void MakeFeatureDifference(Parts *parts,
                             Features *features,
                             const vector<double> &gold_output,
                             const vector<double> &predicted_output,
                             FeatureVector *difference);

  void MakeGradientStep(Parts *parts,
                        Features *features,
                        double eta,
                        int iteration,
                        const vector<double> &gold_output,
                        const vector<double> &predicted_output);

  void TouchParameters(Parts *parts, Features *features,
                       const vector<bool> &selected_parts);

  void LabelInstance(Parts *parts, const vector<double> &output,
                     Instance *instance);

  void Prune(Instance *instance, Parts *parts, vector<double> *gold_outputs,
             bool preserve_gold);

  virtual void BeginEvaluation() {
#if 0
    num_head_mistakes_ = 0;
    num_head_pruned_mistakes_ = 0;
    num_heads_after_pruning_ = 0;
    num_tokens_ = 0;
    gettimeofday(&start_clock_, NULL);
#endif
  }
  virtual void EvaluateInstance(Instance *instance, Parts *parts,
                                const vector<double> &gold_outputs,
                                const vector<double> &predicted_outputs) {
#if 0
    SemanticInstance *semantic_instance =
      static_cast<SemanticInstance*>(instance);
    SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
    for (int p = 1; p < semantic_instance->size(); ++p) {
      int head = -1;
      int num_possible_heads = 0;
      for (int a = 0; a < dependency_instance->size(); ++a) {
        int r = dependency_parts->FindArc(h, m);
        if (r < 0) continue;
        ++num_possible_heads;
        if (gold_outputs[r] >= 0.5) {
          CHECK_EQ(gold_outputs[r], 1.0);
          if (!NEARLY_EQ_TOL(gold_outputs[r], predicted_outputs[r], 1e-6)) {
            ++num_head_mistakes_;
          }
          head = h;
          //break;
        }
      }
      if (head < 0) {
        VLOG(2) << "Pruned gold part...";
        ++num_head_mistakes_;
        ++num_head_pruned_mistakes_;
      }
      ++num_tokens_;
      num_heads_after_pruning_ += num_possible_heads;
    }
#endif
  }
  virtual void EndEvaluation() {
#if 0
    LOG(INFO) << "Parsing accuracy: " <<
      static_cast<double>(num_tokens_ - num_head_mistakes_) /
        static_cast<double>(num_tokens_);
    LOG(INFO) << "Pruning recall: " <<
      static_cast<double>(num_tokens_ - num_head_pruned_mistakes_) /
        static_cast<double>(num_tokens_);
    LOG(INFO) << "Pruning efficiency: " <<
      static_cast<double>(num_heads_after_pruning_) /
        static_cast<double>(num_tokens_)
              << " possible heads per token.";
    timeval end_clock;
    gettimeofday(&end_clock, NULL);
    double num_seconds =
        static_cast<double>(diff_ms(end_clock,start_clock_)) / 1000.0;
    double tokens_per_second = static_cast<double>(num_tokens_) / num_seconds;
    LOG(INFO) << "Parsing speed: "
              << tokens_per_second << " tokens per second.";
#endif
  }

#if 0
  void GetAllAncestors(const vector<int> &heads,
                       int descend,
                       vector<int>* ancestors);
  bool ExistsPath(const vector<int> &heads,
                  int ancest,
                  int descend);
#endif
 protected:
  TokenDictionary *token_dictionary_;
  bool train_pruner_;
  Parameters *pruner_parameters_;
#if 0
  int num_head_mistakes_;
  int num_head_pruned_mistakes_;
  int num_heads_after_pruning_;
#endif
  int num_tokens_;
  timeval start_clock_;
};

#endif /* SEMANTICPIPE_H_ */
