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

#ifndef SEQUENCEPIPE_H_
#define SEQUENCEPIPE_H_

#include "Pipe.h"
#include "SequenceOptions.h"
#include "SequenceReader.h"
#include "SequenceDictionary.h"
#include "TokenDictionary.h"
#include "SequenceInstanceNumeric.h"
#include "SequenceWriter.h"
#include "SequencePart.h"
#include "SequenceFeatures.h"
#include "SequenceDecoder.h"

class SequencePipe : public Pipe {
 public:
  SequencePipe(Options* options) : Pipe(options) { token_dictionary_ = NULL; };
  virtual ~SequencePipe() { delete token_dictionary_; };

  SequenceReader *GetSequenceReader() {
    return static_cast<SequenceReader*>(reader_);
  };
  SequenceDictionary *GetSequenceDictionary() {
    return static_cast<SequenceDictionary*>(dictionary_);
  };
  SequenceOptions *GetSequenceOptions() {
    return static_cast<SequenceOptions*>(options_);
  };

 protected:
  void CreateDictionary() { 
    dictionary_ = new SequenceDictionary(this);
    GetSequenceDictionary()->SetTokenDictionary(token_dictionary_);
  }
  void CreateReader() { reader_ = new SequenceReader; }
  void CreateWriter() { writer_ = new SequenceWriter; }
  void CreateDecoder() { decoder_ = new SequenceDecoder(this); };
  Parts *CreateParts() { return new SequenceParts; };
  Features *CreateFeatures() { return new SequenceFeatures(this); };

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  };

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    SequenceInstanceNumeric *instance_numeric =
          new SequenceInstanceNumeric;
    instance_numeric->Initialize(*GetSequenceDictionary(),
                                 static_cast<SequenceInstance*>(instance));
    return instance_numeric;
  }

 protected:
  void SaveModel(FILE* fs);
  void LoadModel(FILE* fs);

  void MakeParts(Instance *instance, Parts *parts,
                 vector<double> *gold_outputs);
  void MakeUnigramParts(Instance *instance, Parts *parts,
                        vector<double> *gold_outputs);
  void MakeBigramParts(Instance *instance, Parts *parts,
                       vector<double> *gold_outputs);
  void MakeTrigramParts(Instance *instance, Parts *parts,
                       vector<double> *gold_outputs);

  void MakeSelectedFeatures(Instance *instance, Parts *parts,
      const vector<bool> &selected_parts, Features *features);

  void ComputeScores(Instance *instance, Parts *parts, Features *features,
                     vector<double> *scores);

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

  void LabelInstance(Parts *parts, const vector<double> &output,
                     Instance *instance);

  virtual void BeginEvaluation() {
    num_tag_mistakes_ = 0;
    num_tokens_ = 0;
    gettimeofday(&start_clock_, NULL);
  }
  virtual void EvaluateInstance(Instance *instance, Parts *parts,
                                const vector<double> &gold_outputs,
                                const vector<double> &predicted_outputs) {
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
  virtual void EndEvaluation() {
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

 protected:
  TokenDictionary *token_dictionary_;
  int num_tag_mistakes_;
  int num_tokens_;
  timeval start_clock_;
};

#endif /* SEQUENCEPIPE_H_ */

