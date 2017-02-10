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

#ifndef SEQUENCEPIPE_H_
#define SEQUENCEPIPE_H_

#include "Pipe.h"
#include "TimeUtils.h"
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
  SequencePipe(Options* options) : Pipe(options) { token_dictionary_ = NULL; }
  virtual ~SequencePipe() { delete token_dictionary_; }

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
  virtual void CreateDictionary() {
    dictionary_ = new SequenceDictionary(this);
    GetSequenceDictionary()->SetTokenDictionary(token_dictionary_);
  }
  virtual void CreateReader() { reader_ = new SequenceReader; }
  virtual void CreateWriter() { writer_ = new SequenceWriter; }
  void CreateDecoder() { decoder_ = new SequenceDecoder(this); };
  Parts *CreateParts() { return new SequenceParts; };
  virtual Features *CreateFeatures() { return new SequenceFeatures(this); };

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  };

  virtual void PreprocessData();

  virtual Instance *GetFormattedInstance(Instance *instance) {
    SequenceInstanceNumeric *instance_numeric =
      new SequenceInstanceNumeric;
    instance_numeric->Initialize(*GetSequenceDictionary(),
                                 static_cast<SequenceInstance*>(instance));
    return instance_numeric;
  }

protected:
  virtual void SaveModel(FILE* fs);
  virtual void LoadModel(FILE* fs);

  // Return the allowed tags for the i-th word. An empty vector means that all
  // tags are allowed.
  virtual void GetAllowedTags(Instance *instance, int i,
                              vector<int> *allowed_tags) {
    // By default, allow all tags.
    allowed_tags->clear();
  }

  void MakeParts(Instance *instance, Parts *parts,
                 vector<double> *gold_outputs);
  void MakeUnigramParts(Instance *instance, Parts *parts,
                        vector<double> *gold_outputs);
  void MakeBigramParts(Instance *instance, Parts *parts,
                       vector<double> *gold_outputs);
  void MakeTrigramParts(Instance *instance, Parts *parts,
                        vector<double> *gold_outputs);

  void MakeSelectedFeatures(Instance *instance, Parts *parts,
                            const vector<bool> &selected_parts,
                            Features *features);

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
    chrono.GetTime();
  }
  virtual void EvaluateInstance(Instance *instance,
                                Instance *output_instance,
                                Parts *parts,
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
    LOG(INFO) << "Correct predictions: " << (num_tokens_ - num_tag_mistakes_)
      << " out of " << static_cast<double>(num_tokens_);
    LOG(INFO) << "Tagging accuracy: " <<
      static_cast<double>(num_tokens_ - num_tag_mistakes_) /
      static_cast<double>(num_tokens_);
    chrono.StopTime();
    double num_seconds = chrono.GetElapsedTime();
    double tokens_per_second = static_cast<double>(num_tokens_) / num_seconds;
    LOG(INFO) << "Tagging speed: "
      << tokens_per_second << " tokens per second.";
  }

protected:
  TokenDictionary *token_dictionary_;
  int num_tag_mistakes_;
  int num_tokens_;
  chronowrap::Chronometer chrono;
};

#endif /* SEQUENCEPIPE_H_ */
