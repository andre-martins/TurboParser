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

#ifndef DEPENDENCYLABELERPIPE_H_
#define DEPENDENCYLABELERPIPE_H_

#include "Pipe.h"
#include "DependencyLabelerOptions.h"
#include "DependencyReader.h"
#include "DependencyDictionary.h"
#include "TokenDictionary.h"
#include "DependencyInstanceNumeric.h"
#include "DependencyWriter.h"
#include "DependencyPart.h"
#include "DependencyLabelerFeatures.h"
#include "DependencyLabelerDecoder.h"

class DependencyLabelerPipe : public Pipe {
 public:
  DependencyLabelerPipe(Options* options) : Pipe(options) {
    token_dictionary_ = NULL;
    //pruner_parameters_ = NULL;
    //train_pruner_ = false;
  }
  virtual ~DependencyLabelerPipe() {
    //delete token_dictionary_;
    //delete pruner_parameters_;
  }

  // Same reader as the dependency parser.
  DependencyReader *GetDependencyReader() {
    return static_cast<DependencyReader*>(reader_);
  }
  // Same dictionary as the dependency parser.
  DependencyDictionary *GetDependencyDictionary() {
    return static_cast<DependencyDictionary*>(dictionary_);
  }
  DependencyLabelerDecoder *GetDependencyLabelerDecoder() {
    return static_cast<DependencyLabelerDecoder*>(decoder_);
  }
  DependencyLabelerOptions *GetDependencyLabelerOptions() {
    return static_cast<DependencyLabelerOptions*>(options_);
  }

  void Initialize() {
    Pipe::Initialize();
    //pruner_parameters_ = new Parameters;
  }

  //void SetPrunerParameters(Parameters *pruner_parameters) {
  //  pruner_parameters_ = pruner_parameters;
  //}
  //void LoadPrunerModelFile() {
  //  LoadPrunerModelByName(GetDependencyOptions()->GetPrunerModelFilePath());
  //}

 protected:
  void CreateDictionary() {
    dictionary_ = new DependencyDictionary(this);
    GetDependencyDictionary()->SetTokenDictionary(token_dictionary_);
  };
  void CreateReader() { reader_ = new DependencyReader; };
  void CreateWriter() { writer_ = new DependencyWriter; };
  void CreateDecoder() { decoder_ = new DependencyLabelerDecoder(this); };
  Parts *CreateParts() { return new DependencyParts; };
  Features *CreateFeatures() { return new DependencyLabelerFeatures(this); };

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  };

  Parameters *GetTrainingParameters() {
    //if (train_pruner_) return pruner_parameters_;
    return parameters_;
  }

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    DependencyInstanceNumeric *instance_numeric =
          new DependencyInstanceNumeric;
    instance_numeric->Initialize(*GetDependencyDictionary(),
                                 static_cast<DependencyInstance*>(instance));
    return instance_numeric;
  }

  void SaveModel(FILE* fs);
  void LoadModel(FILE* fs);

  //void LoadPrunerModel(FILE* fs);
  //void LoadPrunerModelByName(const string &model_name);

  void MakeParts(Instance *instance, Parts *parts,
                 std::vector<double> *gold_outputs);
  void MakePartsBasic(Instance *instance, Parts *parts,
                      std::vector<double> *gold_outputs);
  void MakeSelectedFeatures(Instance *instance,
                            Parts *parts,
                            const std::vector<bool>& selected_parts,
                            Features *features);

  void ComputeScores(Instance *instance, Parts *parts, Features *features,
                     std::vector<double> *scores);

  void RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                 const vector<bool> &selected_parts,
                                 Features *features);

  void MakeFeatureDifference(Parts *parts,
                             Features *features,
                             const std::vector<double> &gold_output,
                             const std::vector<double> &predicted_output,
                             FeatureVector *difference);

  void MakeGradientStep(Parts *parts,
                        Features *features,
                        double eta,
                        int iteration,
                        const std::vector<double> &gold_output,
                        const std::vector<double> &predicted_output);

  void TouchParameters(Parts *parts, Features *features,
                       const std::vector<bool> &selected_parts);

  void LabelInstance(Parts *parts, const std::vector<double> &output,
                     Instance *instance);

  //void Prune(Instance *instance, Parts *parts, vector<double> *gold_outputs,
  //           bool preserve_gold);

  virtual void BeginEvaluation() {
    num_head_mistakes_ = 0;
    num_head_pruned_mistakes_ = 0;
    num_heads_after_pruning_ = 0;
    num_tokens_ = 0;
    gettimeofday(&start_clock_, NULL);
  }
  virtual void EvaluateInstance(Instance *instance,
                                Instance *output_instance,
                                Parts *parts,
                                const std::vector<double> &gold_outputs,
                                const std::vector<double> &predicted_outputs) {
    DependencyInstance *dependency_instance =
      static_cast<DependencyInstance*>(instance);
    DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
    for (int m = 1; m < dependency_instance->size(); ++m) {
      int head = -1;
      int num_possible_heads = 0;
      for (int h = 0; h < dependency_instance->size(); ++h) {
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
  }
  virtual void EndEvaluation() {
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
  }

  //void GetAllAncestors(const vector<int> &heads,
  //                     int descend,
  //                     vector<int>* ancestors) const;
  //bool ExistsPath(const vector<int> &heads,
  //                int ancest,
  //                int descend) const;
  //bool IsProjectiveArc(const vector<int> &heads, int par, int ch) const;

 protected:
  TokenDictionary *token_dictionary_;
  //bool train_pruner_;
  //Parameters *pruner_parameters_;
  int num_head_mistakes_;
  int num_head_pruned_mistakes_;
  int num_heads_after_pruning_;
  int num_tokens_;
  timeval start_clock_;
};

#endif /* DEPENDENCYLABELERPIPE_H_ */
