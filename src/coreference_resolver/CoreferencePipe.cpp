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

#include "CoreferencePipe.h"
#include "logval.h"
#include <iostream>
#include <sstream>
#include <vector>

// Define the current model version and the oldest back-compatible version.
// The format is AAAA.BBBB.CCCC, e.g., 2 0003 0000 means "2.3.0".
const uint64_t kCoreferenceModelVersion = 200040000;
const uint64_t kOldestCompatibleCoreferenceModelVersion = 200040000;
const uint64_t kCoreferenceModelCheck = 1234567890;

void CoreferencePipe::SaveModel(FILE* fs) {
  bool success;
  success = WriteUINT64(fs, kCoreferenceModelCheck);
  CHECK(success);
  success = WriteUINT64(fs, kCoreferenceModelVersion);
  CHECK(success);
  token_dictionary_->Save(fs);
  dependency_dictionary_->Save(fs);
  semantic_dictionary_->Save(fs);
  Pipe::SaveModel(fs);
}

void CoreferencePipe::LoadModel(FILE* fs) {
  bool success;
  uint64_t model_check;
  uint64_t model_version;
  success = ReadUINT64(fs, &model_check);
  CHECK(success);
  CHECK_EQ(model_check, kCoreferenceModelCheck)
    << "The model file is too old and not supported anymore.";
  success = ReadUINT64(fs, &model_version);
  CHECK(success);
  CHECK_GE(model_version, kOldestCompatibleCoreferenceModelVersion)
    << "The model file is too old and not supported anymore.";

  delete token_dictionary_;
  CreateTokenDictionary();
  token_dictionary_->Load(fs);

  delete dependency_dictionary_;
  CreateDependencyDictionary();
  dependency_dictionary_->SetTokenDictionary(token_dictionary_);
  dependency_dictionary_->Load(fs);

  delete semantic_dictionary_;
  CreateSemanticDictionary();
  semantic_dictionary_->SetTokenDictionary(token_dictionary_);
  semantic_dictionary_->SetDependencyDictionary(dependency_dictionary_);
  semantic_dictionary_->Load(fs);

  GetCoreferenceDictionary()->SetTokenDictionary(token_dictionary_);
  GetCoreferenceDictionary()->SetDependencyDictionary(dependency_dictionary_);
  GetCoreferenceDictionary()->SetSemanticDictionary(semantic_dictionary_);

  Pipe::LoadModel(fs);
}

void CoreferencePipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  token_dictionary_->
    InitializeFromDependencyReader(GetCoreferenceSentenceReader());

  delete dependency_dictionary_;
  CreateDependencyDictionary();
  dependency_dictionary_->SetTokenDictionary(token_dictionary_);
  dependency_dictionary_->CreateLabelDictionary(GetCoreferenceSentenceReader());

  delete semantic_dictionary_;
  CreateSemanticDictionary();
  semantic_dictionary_->SetTokenDictionary(token_dictionary_);
  semantic_dictionary_->SetDependencyDictionary(dependency_dictionary_);
  semantic_dictionary_->
    CreatePredicateRoleDictionaries(GetCoreferenceSentenceReader());

  GetCoreferenceDictionary()->SetTokenDictionary(token_dictionary_);
  GetCoreferenceDictionary()->SetDependencyDictionary(dependency_dictionary_);
  GetCoreferenceDictionary()->SetSemanticDictionary(semantic_dictionary_);
  GetCoreferenceDictionary()->
    CreateEntityDictionary(GetCoreferenceSentenceReader());
  GetCoreferenceDictionary()->
    CreateConstituentDictionary(GetCoreferenceSentenceReader());
  GetCoreferenceDictionary()->
    CreateWordDictionaries(GetCoreferenceSentenceReader());

  GetCoreferenceDictionary()->ReadMentionTags();
  GetCoreferenceDictionary()->ReadPronouns();
  GetCoreferenceDictionary()->ReadGenderNumberStatistics();
}

void CoreferencePipe::ComputeScores(Instance *instance, Parts *parts,
                                    Features *features,
                                    std::vector<double> *scores) {
  Pipe::ComputeScores(instance, parts, features, scores);
}

void CoreferencePipe::MakeGradientStep(
    Parts *parts,
    Features *features,
    double eta,
    int iteration,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output) {
  Pipe::MakeGradientStep(parts, features, eta, iteration, gold_output,
                         predicted_output);
}

void CoreferencePipe::MakeFeatureDifference(
    Parts *parts,
    Features *features,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output,
    FeatureVector *difference) {
  Pipe::MakeFeatureDifference(parts, features, gold_output, predicted_output,
                              difference);
}

void CoreferencePipe::TransformGold(Instance *instance,
                                    Parts *parts,
                                    const std::vector<double> &scores,
                                    std::vector<double> *gold_output,
                                    double *loss_inner) {
  double log_partition_function_inner;
  double entropy_inner;
  std::vector<double> copied_scores = scores;
  for (int r = 0; r < parts->size(); ++r) {
    if ((*gold_output)[r] < 0.5) {
      copied_scores[r] = -std::numeric_limits<double>::infinity();
    }
  }
  static_cast<CoreferenceDecoder*>(decoder_)->
    DecodeBasicMarginals(instance, parts, copied_scores, gold_output,
                         &log_partition_function_inner, &entropy_inner);
  *loss_inner = entropy_inner;
}

void CoreferencePipe::MakeParts(Instance *instance,
                                Parts *parts,
                                std::vector<double> *gold_outputs) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);

  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);
  coreference_parts->Initialize();
  bool make_gold = (gold_outputs != NULL);
  if (make_gold) gold_outputs->clear();

  const std::vector<Mention*> &mentions = document->GetMentions();
  //std::set<int> entities;

  // Create arc parts departing from the artifical root (non-anaphoric
  // mentions).
  for (int j = 0; j < mentions.size(); ++j) {
    Part *part = coreference_parts->CreatePartArc(-1, j);
    coreference_parts->push_back(part);
    if (make_gold) {
      if (!document->IsMentionAnaphoric(j)) {
        gold_outputs->push_back(1.0);
      } else {
        gold_outputs->push_back(0.0);
      }
#if 0
      if (mentions[j]->id() >= 0) {
        if (entities.find(mentions[j]->id()) == entities.end()) {
          entities.insert(mentions[j]->id()); // First instance of this entity.
          gold_outputs->push_back(1.0);
        } else {
          gold_outputs->push_back(0.0);
        }
      } else {
        gold_outputs->push_back(1.0); // Singleton mention.
      }
#endif
    }
  }

  // Create arc parts involving two mentions.
  for (int j = 0; j < mentions.size(); ++j) {
    for (int k = j+1; k < mentions.size(); ++k) {
      Part *part = coreference_parts->CreatePartArc(j, k);
      coreference_parts->push_back(part);
      if (make_gold) {
        if (mentions[j]->id() >= 0 && mentions[j]->id() == mentions[k]->id()) {
          gold_outputs->push_back(1.0);
          //LOG(INFO) << "Found coreferent mentions: " << j << ", " << k;
        } else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }

  coreference_parts->BuildIndices(mentions.size());
}

void CoreferencePipe::MakeSelectedFeatures(
    Instance *instance,
    Parts *parts,
    const std::vector<bool> &selected_parts,
    Features *features) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceFeatures *coreference_features =
    static_cast<CoreferenceFeatures*>(features);

  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);
  const std::vector<Mention*> &mentions = document->GetMentions();

  coreference_features->Initialize(instance, parts);

  // Build features for coreference arcs.
  for (int r = 0; r < coreference_parts->size(); ++r) {
    CoreferencePartArc *arc =
      static_cast<CoreferencePartArc*>((*coreference_parts)[r]);
    coreference_features->AddArcFeatures(document, r, arc->parent_mention(),
                                         arc->child_mention());
  }
}

void CoreferencePipe::LabelInstance(Parts *parts,
                                    const std::vector<double> &output,
                                    Instance *instance) {
#if 0
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  SequenceInstance *sequence_instance =
      static_cast<SequenceInstance*>(instance);
  int instance_length = sequence_instance->size();
  for (int i = 0; i < instance_length; ++i) {
    sequence_instance->SetTag(i, "NULL");
  }
  double threshold = 0.5;
  int offset, size;
  sequence_parts->GetOffsetUnigram(&offset, &size);
  for (int r = 0; r < size; ++r) {
    SequencePartUnigram *unigram =
        static_cast<SequencePartUnigram*>((*sequence_parts)[offset + r]);
    if (output[offset + r] >= threshold) {
      int i = unigram->position();
      int tag = unigram->tag();
      CHECK(GetSequenceDictionary());
      sequence_instance->SetTag(i,
        GetSequenceDictionary()->GetTagName(tag));
    }
  }
  for (int i = 0; i < instance_length; ++i) {
    CHECK(sequence_instance->GetTag(i) != "NULL");
  }
#endif
}

