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

#ifndef CONSTITUENCYLABELERPIPE_H_
#define CONSTITUENCYLABELERPIPE_H_

#include "Pipe.h"
#include "ConstituencyLabelerOptions.h"
#include "ConstituencyLabelerReader.h"
#include "ConstituencyLabelerDictionary.h"
#include "TokenDictionary.h"
#include "ConstituencyLabelerInstanceNumeric.h"
#include "ConstituencyLabelerWriter.h"
#include "ConstituencyLabelerPart.h"
#include "ConstituencyLabelerFeatures.h"
#include "ConstituencyLabelerDecoder.h"

class ConstituencyLabelerPipe : public Pipe {
 public:
  ConstituencyLabelerPipe(Options* options) : Pipe(options) {
    token_dictionary_ = NULL;
  }
  virtual ~ConstituencyLabelerPipe() { delete token_dictionary_; }

  ConstituencyLabelerReader *GetConstituencyReader() {
    return static_cast<ConstituencyLabelerReader*>(reader_);
  };
  ConstituencyLabelerDictionary *GetConstituencyLabelerDictionary() {
    return static_cast<ConstituencyLabelerDictionary*>(dictionary_);
  };
  ConstituencyLabelerOptions *GetConstituencyLabelerOptions() {
    return static_cast<ConstituencyLabelerOptions*>(options_);
  };

 protected:
  void CreateDictionary() {
    dictionary_ = new ConstituencyLabelerDictionary(this);
    GetConstituencyLabelerDictionary()->SetTokenDictionary(token_dictionary_);
  }
  void CreateReader() { reader_ = new ConstituencyLabelerReader; }
  void CreateWriter() { writer_ = new ConstituencyLabelerWriter; }
  void CreateDecoder() { decoder_ = new ConstituencyLabelerDecoder(this); };
  Parts *CreateParts() { return new ConstituencyLabelerParts; };
  Features *CreateFeatures() { return new ConstituencyLabelerFeatures(this); };

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  };

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    ConstituencyLabelerInstanceNumeric *instance_numeric =
          new ConstituencyLabelerInstanceNumeric;
    instance_numeric->Initialize(
        *GetConstituencyLabelerDictionary(),
        static_cast<ConstituencyLabelerInstance*>(instance));
    return instance_numeric;
  }

 protected:
  void SaveModel(FILE* fs);
  void LoadModel(FILE* fs);

  // Return the allowed labels for the i-th node. An empty vector means that all
  // tags are allowed.
  void GetAllowedLabels(Instance *instance, int i,
                        std::vector<int> *allowed_labels) {
    // Make constituent-label dictionary pruning.
    allowed_labels->clear();
    bool prune_labels = GetConstituencyLabelerOptions()->prune_labels();
    if (!prune_labels) return;

    ConstituencyLabelerInstanceNumeric *sentence =
      static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
    ConstituencyLabelerDictionary *labeler_dictionary =
      GetConstituencyLabelerDictionary();

    int constituent_id = sentence->GetConstituentId(i);
    *allowed_labels =
      labeler_dictionary->GetConstituentLabels(constituent_id);
  }

  void MakeParts(Instance *instance, Parts *parts,
                 std::vector<double> *gold_outputs);

  void MakeNodeParts(Instance *instance,
                     Parts *parts,
                     std::vector<double> *gold_outputs);

  void MakeSelectedFeatures(Instance *instance, Parts *parts,
                            const std::vector<bool> &selected_parts,
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

  void BeginEvaluation() {
    num_tokens_ = 0;
    num_constituents_ = 0;
    num_matched_labels_ = 0;
    num_predicted_labels_ = 0;
    num_gold_labels_ = 0;
    num_pruned_gold_labels_ = 0;
    num_possible_labels_ = 0;
    gettimeofday(&start_clock_, NULL);
  }
  void EvaluateInstance(Instance *instance,
                        Instance *output_instance,
                        Parts *parts,
                        const std::vector<double> &gold_outputs,
                        const std::vector<double> &predicted_outputs) {
    ConstituencyLabelerInstance *labeler_instance =
      static_cast<ConstituencyLabelerInstance*>(instance);
    ConstituencyLabelerParts *labeler_parts =
      static_cast<ConstituencyLabelerParts*>(parts);
    ConstituencyLabelerOptions *labeler_options =
      static_cast<ConstituencyLabelerOptions*>(options_);
    ConstituencyLabelerDictionary *labeler_dictionary =
      static_cast<ConstituencyLabelerDictionary*>(dictionary_);
    const std::string &null_label = labeler_options->null_label();

    int num_possible_labels = 0;
    int num_gold_labels = 0;
    int num_actual_gold_labels = 0;
    for (int i = 0; i < labeler_instance->GetNumConstituents(); ++i) {
      if (labeler_instance->GetConstituentLabel(i) != null_label) {
        ++num_actual_gold_labels;
      }
    }

    for (int i = 0; i < labeler_instance->GetNumConstituents(); ++i) {
      const vector<int>& nodes = labeler_parts->FindNodeParts(i);
      for (int k = 0; k < nodes.size(); ++k) {
        int r = nodes[k];
        int label =
          static_cast<ConstituencyLabelerPartNode*>((*parts)[r])->label();

        // Ignore if this is the null label.
        if (label == labeler_dictionary->null_label()) continue;

        ++num_possible_labels;
        if (gold_outputs[r] >= 0.5) {
          CHECK_EQ(gold_outputs[r], 1.0);
          if (NEARLY_EQ_TOL(gold_outputs[r], predicted_outputs[r], 1e-6)) {
            ++num_matched_labels_;
          }
          ++num_gold_labels;
        }
        if (predicted_outputs[r] >= 0.5) {
          CHECK_EQ(predicted_outputs[r], 1.0);
          ++num_predicted_labels_;
        }
      }
      ++num_constituents_;
    }
    num_tokens_ += labeler_instance->size();
    num_gold_labels_ += num_actual_gold_labels;
    int missed_labels = num_actual_gold_labels - num_gold_labels;

    num_pruned_gold_labels_ += missed_labels;
    num_possible_labels_ += num_possible_labels;
  }
  void EndEvaluation() {
    double precision =
      static_cast<double>(num_matched_labels_) /
        static_cast<double>(num_predicted_labels_);
    double recall =
      static_cast<double>(num_matched_labels_) /
        static_cast<double>(num_gold_labels_);
    double F1 = 2.0 * precision * recall / (precision + recall);
    double pruning_recall =
      static_cast<double>(num_gold_labels_ -
                          num_pruned_gold_labels_) /
        static_cast<double>(num_gold_labels_);
    double pruning_efficiency =
      static_cast<double>(num_possible_labels_) /
        static_cast<double>(num_constituents_);

    LOG(INFO) << "Precision: " << precision
              << " (" << num_matched_labels_ << "/"
              << num_predicted_labels_ << ")";
    LOG(INFO) << "Recall: " << recall
              << " (" << num_matched_labels_ << "/"
              << num_gold_labels_ << ")";
    LOG(INFO) << "F1: " << F1;
    LOG(INFO) << "Pruning recall: " << pruning_recall
              << " ("
              << num_gold_labels_ - num_pruned_gold_labels_
              << "/"
              << num_gold_labels_ << ")";
    LOG(INFO) << "Pruning efficiency: " << pruning_efficiency
              << " possible labels per node"
              << " (" << num_possible_labels_ << "/"
              << num_constituents_ << ")";

    timeval end_clock;
    gettimeofday(&end_clock, NULL);
    double num_seconds =
        static_cast<double>(diff_ms(end_clock,start_clock_)) / 1000.0;
    double tokens_per_second = static_cast<double>(num_tokens_) / num_seconds;
    LOG(INFO) << "Speed: "
              << tokens_per_second << " tokens per second.";
  }

 protected:
  TokenDictionary *token_dictionary_;
  int num_tokens_;
  int num_constituents_;
  int num_matched_labels_;
  int num_predicted_labels_;
  int num_gold_labels_;
  int num_pruned_gold_labels_;
  int num_possible_labels_;
  timeval start_clock_;
};

#endif /* CONSTITUENCYLABELERPIPE_H_ */

