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

#ifndef COREFERENCEPIPE_H_
#define COREFERENCEPIPE_H_

#include "Pipe.h"
#include "TimeUtils.h"
#include "CoreferenceOptions.h"
#include "CoreferenceReader.h"
#include "CoreferenceDictionary.h"
#include "TokenDictionary.h"
#include "CoreferenceDocumentNumeric.h"
#include "CoreferenceWriter.h"
#include "CoreferencePart.h"
#include "CoreferenceFeatures.h"
#include "CoreferenceDecoder.h"

class CoreferencePipe : public Pipe {
public:
  CoreferencePipe(Options* options) : Pipe(options) {
    token_dictionary_ = NULL;
    dependency_dictionary_ = NULL;
    semantic_dictionary_ = NULL;
  }
  virtual ~CoreferencePipe() {
    delete token_dictionary_;
    delete dependency_dictionary_;
    delete semantic_dictionary_;
  }

  CoreferenceOptions *GetCoreferenceOptions() {
    return static_cast<CoreferenceOptions*>(options_);
  };
  CoreferenceReader *GetCoreferenceReader() {
    return static_cast<CoreferenceReader*>(reader_);
  };
  CoreferenceSentenceReader *GetCoreferenceSentenceReader() {
    return GetCoreferenceReader()->GetSentenceReader();
  };
  CoreferenceDictionary *GetCoreferenceDictionary() {
    return static_cast<CoreferenceDictionary*>(dictionary_);
  };
  SemanticDictionary *GetSemanticDictionary() {
    return static_cast<SemanticDictionary*>(semantic_dictionary_);
  };
  DependencyDictionary *GetDependencyDictionary() {
    return static_cast<DependencyDictionary*>(dependency_dictionary_);
  };

protected:
  void CreateDictionary() {
    dictionary_ = new CoreferenceDictionary(this);
    GetCoreferenceDictionary()->SetTokenDictionary(token_dictionary_);
    GetCoreferenceDictionary()->SetDependencyDictionary(dependency_dictionary_);
    GetCoreferenceDictionary()->SetSemanticDictionary(semantic_dictionary_);
  }
  void CreateReader() { reader_ = new CoreferenceReader(options_); }
  void CreateWriter() { writer_ = new CoreferenceWriter(options_); }
  void CreateDecoder() { decoder_ = new CoreferenceDecoder(this); }
  Parts *CreateParts() { return new CoreferenceParts; }
  Features *CreateFeatures() { return new CoreferenceFeatures(this); }

  void CreateTokenDictionary() {
    token_dictionary_ = new TokenDictionary(this);
  }

  void CreateDependencyDictionary() {
    dependency_dictionary_ = new DependencyDictionary(this);
  }

  void CreateSemanticDictionary() {
    semantic_dictionary_ = new SemanticDictionary(this);
  }

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    CoreferenceDocumentNumeric *instance_numeric =
      new CoreferenceDocumentNumeric;
    // Only add gold mentions as candidates if we're training.
    bool add_gold_mentions = options_->train();
    instance_numeric->Initialize(*GetCoreferenceDictionary(),
                                 static_cast<CoreferenceDocument*>(instance),
                                 add_gold_mentions);
    return instance_numeric;
  }

protected:
  void SaveModel(FILE* fs);
  void LoadModel(FILE* fs);

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

  void TransformGold(Instance *instance,
                     Parts *parts,
                     const std::vector<double> &scores,
                     std::vector<double> *gold_output,
                     double *loss_inner);

  void LabelInstance(Parts *parts, const std::vector<double> &output,
                     Instance *instance);

  void BeginEvaluation() {
    num_tokens_ = 0;
    chrono.GetTime();
  }

  void EvaluateInstance(Instance *instance,
                        Instance *output_instance,
                        Parts *parts,
                        const vector<double> &gold_outputs,
                        const vector<double> &predicted_outputs) {
    CoreferenceDocument *document =
      static_cast<CoreferenceDocument*>(instance);
    for (int i = 0; i < document->GetNumSentences(); ++i) {
      num_tokens_ += document->GetSentence(i)->size() - 1;
    }
  }

  void EndEvaluation() {
    chrono.StopTime();
    double num_seconds = chrono.GetElapsedTime();
    double tokens_per_second = static_cast<double>(num_tokens_) / num_seconds;
    LOG(INFO) << "Speed: "
      << tokens_per_second << " tokens per second.";
  }

protected:
  TokenDictionary *token_dictionary_;
  DependencyDictionary *dependency_dictionary_;
  SemanticDictionary *semantic_dictionary_;
  //int num_tag_mistakes_;
  int num_tokens_;
  chronowrap::Chronometer chrono;
};

#endif /* COREFERENCEPIPE_H_ */
