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
//#include "ConstituencyInstanceNumeric.h"
#include "ConstituencyWriter.h"
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
  void CreateWriter() { writer_ = new ConstituencyWriter; }
  void CreateDecoder() { decoder_ = new ConstituencyLabelerDecoder(this); };
  Parts *CreateParts() { return new ConstituencyLabelerParts; };
  Features *CreateFeatures() { return new ConstituencyLabelerFeatures(this); };

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  };

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    return instance->Copy();
#if 0
    SequenceInstanceNumeric *instance_numeric =
          new SequenceInstanceNumeric;
    instance_numeric->Initialize(*GetSequenceDictionary(),
                                 static_cast<SequenceInstance*>(instance));
    return instance_numeric;
#endif
  }

 protected:
  void SaveModel(FILE* fs);
  void LoadModel(FILE* fs);

#if 0
  // Return the allowed tags for the i-th word. An empty vector means that all
  // tags are allowed.
  void GetAllowedTags(Instance *instance, int i,
                      std::vector<int> *allowed_tags) {
    // By default, allow all tags.
    allowed_tags->clear();
  }
#endif

  void MakeParts(Instance *instance, Parts *parts,
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

#if 0
  void BeginEvaluation() {
    num_tag_mistakes_ = 0;
    num_tokens_ = 0;
    gettimeofday(&start_clock_, NULL);
  }
  void EvaluateInstance(Instance *instance,
                        Instance *output_instance,
                        Parts *parts,
                        const std::vector<double> &gold_outputs,
                        const std::vector<double> &predicted_outputs) {
    SequenceInstance *sequence_instance =
      static_cast<SequenceInstance*>(instance);
    SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
    for (int i = 0; i < sequence_instance->size(); ++i) {
      const vector<int>& unigrams = sequence_parts->FindUnigramParts(i);
      for (int k = 0; k < unigrams.size(); ++k) {
        int r = unigrams[k];
        if (!NEARLY_EQ_TOL(gold_outputs[r], predicted_outputs[r], 1e-6)) {
          ++num_tag_mistakes_;
          break;
        }
      }
      ++num_tokens_;
    }
  }
  void EndEvaluation() {
    LOG(INFO) << "Tagging accuracy: " <<
      static_cast<double>(num_tokens_ - num_tag_mistakes_) /
        static_cast<double>(num_tokens_);
    timeval end_clock;
    gettimeofday(&end_clock, NULL);
    double num_seconds =
        static_cast<double>(diff_ms(end_clock,start_clock_)) / 1000.0;
    double tokens_per_second = static_cast<double>(num_tokens_) / num_seconds;
    LOG(INFO) << "Tagging speed: "
              << tokens_per_second << " tokens per second.";
  }
#endif

 protected:
  TokenDictionary *token_dictionary_;
  int num_label_mistakes_;
  int num_constituents_;
  timeval start_clock_;
};

#endif /* CONSTITUENCYLABELERPIPE_H_ */

