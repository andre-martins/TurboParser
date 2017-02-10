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

#ifndef DEPENDENCYLABELERPIPE_H_
#define DEPENDENCYLABELERPIPE_H_

#include "Pipe.h"
#include "TimeUtils.h"
#include "DependencyLabelerOptions.h"
#include "DependencyReader.h"
#include "DependencyDictionary.h"
#include "TokenDictionary.h"
#include "DependencyInstanceNumeric.h"
#include "DependencyWriter.h"
#include "DependencyLabelerPart.h"
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
  Parts *CreateParts() { return new DependencyLabelerParts; };
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
  void MakeArcParts(Instance *instance, Parts *parts,
                    std::vector<double> *gold_outputs);
  void MakeSiblingParts(Instance *instance,
                        Parts *parts,
                        std::vector<double> *gold_outputs);

  void MakeSelectedFeatures(Instance *instance,
                            Parts *parts,
                            const std::vector<bool>& selected_parts,
                            Features *features);

  void ComputeScores(Instance *instance, Parts *parts, Features *features,
                     std::vector<double> *scores);

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

  void LabelInstance(Parts *parts, const std::vector<double> &output,
                     Instance *instance);

  virtual void BeginEvaluation() {
    num_head_mistakes_ = 0;
    num_head_pruned_mistakes_ = 0;
    num_heads_after_pruning_ = 0;
    num_tokens_ = 0;
    chrono.GetTime();
  }
  virtual void EvaluateInstance(Instance *instance,
                                Instance *output_instance,
                                Parts *parts,
                                const std::vector<double> &gold_outputs,
                                const std::vector<double> &predicted_outputs) {
    DependencyInstance *dependency_instance =
      static_cast<DependencyInstance*>(instance);
    DependencyInstance *dependency_output_instance =
      static_cast<DependencyInstance*>(output_instance);
    DependencyLabelerParts *dependency_parts =
      static_cast<DependencyLabelerParts*>(parts);
    for (int m = 1; m < dependency_instance->size(); ++m) {
      int head = -1;
      int h = dependency_output_instance->GetHead(m);
      int num_possible_heads = 0;
      const vector<int> &index_labeled_parts =
        dependency_parts->FindArcs(m);
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int r = index_labeled_parts[k];
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
    LOG(INFO) << "Labeling accuracy: " <<
      static_cast<double>(num_tokens_ - num_head_mistakes_) /
      static_cast<double>(num_tokens_);
    LOG(INFO) << "Pruning recall: " <<
      static_cast<double>(num_tokens_ - num_head_pruned_mistakes_) /
      static_cast<double>(num_tokens_);
    LOG(INFO) << "Pruning efficiency: " <<
      static_cast<double>(num_heads_after_pruning_) /
      static_cast<double>(num_tokens_)
      << " possible labels per token.";
    chrono.StopTime();
    double num_seconds = chrono.GetElapsedTime();
    double tokens_per_second = static_cast<double>(num_tokens_) / num_seconds;
    LOG(INFO) << "Labeling speed: "
      << tokens_per_second << " tokens per second.";
  }

  void ComputeDescendents(const std::vector<int> &heads,
                          std::vector<std::vector<int> >* descendents) const;

  void GetAllAncestors(const std::vector<int> &heads,
                       int descend,
                       std::vector<int>* ancestors) const;

  int GetSiblingLabel(int sibling, int modifier) {
    CHECK_GE(sibling, -1);
    CHECK_GE(modifier, -1);
    int num_labels = GetDependencyDictionary()->GetLabelAlphabet().size();
    return ((1 + sibling) * (1 + num_labels) + (1 + modifier));
  }

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
  chronowrap::Chronometer chrono;
};

#endif /* DEPENDENCYLABELERPIPE_H_ */
