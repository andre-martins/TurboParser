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
  static_cast<DependencyTokenDictionary*>(token_dictionary_)->Initialize(GetCoreferenceSentenceReader());

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
  GetCoreferenceDictionary()->
    CreateAncestryDictionaries(GetCoreferenceSentenceReader());

  GetCoreferenceDictionary()->ReadMentionTags();
  GetCoreferenceDictionary()->ReadPronouns();
  GetCoreferenceDictionary()->ReadDeterminers();
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
  CoreferenceOptions *options = static_cast<CoreferenceOptions*>(options_);
  if (options->train_with_closest_antecedent()) {
    *loss_inner = 0.0;
  } else {
    double log_partition_function_inner;
    double entropy_inner;
    std::vector<double> copied_scores = scores;
    for (int r = 0; r < parts->size(); ++r) {
      if ((*gold_output)[r] < 0.5) {
        copied_scores[r] = -std::numeric_limits<double>::infinity();
      } else {
        CoreferencePartArc *arc = static_cast<CoreferencePartArc*>((*parts)[r]);
        //LOG(INFO) << "Part[" << arc->parent_mention() << ", "
        //          << arc->child_mention() << "] is gold.";
      }
    }
    static_cast<CoreferenceDecoder*>(decoder_)->
      DecodeBasicMarginals(instance, parts, copied_scores, gold_output,
                           &log_partition_function_inner, &entropy_inner);
    *loss_inner = entropy_inner;
  }
}

void CoreferencePipe::MakeParts(Instance *instance,
                                Parts *parts,
                                std::vector<double> *gold_outputs) {
  CoreferenceDocumentNumeric *document =
    static_cast<CoreferenceDocumentNumeric*>(instance);
  CoreferenceOptions *options = static_cast<CoreferenceOptions*>(options_);

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
    }
  }

  // Create arc parts involving two mentions.
  int mention_distance_threshold = -1; //100; // TODO(atm): put this in the options.
  for (int j = 0; j < mentions.size(); ++j) {
    bool found_closest = false;
    for (int k = j+1; k < mentions.size(); ++k) {
      if (mention_distance_threshold >= 0 &&
          k - j > mention_distance_threshold &&
          !(make_gold && (mentions[j]->id() >= 0 &&
                          mentions[j]->id() == mentions[k]->id()))) {
        continue;
      }
      Part *part = coreference_parts->CreatePartArc(j, k);
      coreference_parts->push_back(part);
      if (make_gold) {
        if (mentions[j]->id() >= 0 && mentions[j]->id() == mentions[k]->id()) {
          //LOG(INFO) << "Found coreferent mentions: " << j << ", " << k;
          if (!options->train_with_closest_antecedent() || !found_closest) {
            gold_outputs->push_back(1.0);
            found_closest = true;
          } else {
            gold_outputs->push_back(0.0);
          }
        } else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }

  coreference_parts->BuildIndices(mentions.size());
  // Necessary to store this information here for LabelInstance at test time.
  coreference_parts->SetMentions(mentions);
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
  CoreferenceDocument *document =
    static_cast<CoreferenceDocument*>(instance);
  CoreferenceParts *coreference_parts = static_cast<CoreferenceParts*>(parts);

  const std::vector<Mention*> &mentions = coreference_parts->GetMentions();
  std::vector<int> mention_clusters(mentions.size(), -1);
  std::vector<std::vector<int> > entities;

  double threshold = 0.5;
  for (int r = 0; r < coreference_parts->size(); ++r) {
    if (output[r] < threshold) continue;
    CoreferencePartArc *arc = static_cast<CoreferencePartArc*>((*parts)[r]);
    CHECK_EQ(mention_clusters[arc->child_mention()], -1);
    if (arc->parent_mention() < 0) {
      // Non-anaphoric mention; create its own cluster.
      mention_clusters[arc->child_mention()] = entities.size();
      entities.push_back(std::vector<int>(1, arc->child_mention()));
    } else {
      int k = mention_clusters[arc->parent_mention()];
      mention_clusters[arc->child_mention()] = k;
      entities[k].push_back(arc->child_mention());
    }
  }

  // Clear gold coreference spans, if any.
  for (int i = 0; i < document->GetNumSentences(); ++i) {
    CoreferenceSentence *sentence = document->GetSentence(i);
    sentence->ClearCoreferenceSpans();
  }

  // Add predicted coreference spans.
  int num_entities = 0;
  for (int k = 0; k < entities.size(); ++k) {
    if (entities[k].size() > 1) {
      ++num_entities;
    }
  }
  for (int j = 0; j < mentions.size(); ++j) {
    int k = mention_clusters[j];
    if (entities[k].size() > 1) {
      // Not a singleton cluster; add coreference span.
      std::ostringstream ss;
      ss << k;
      const std::string &name(ss.str());
      NamedSpan span(mentions[j]->start(), mentions[j]->end(), name);
      int i = mentions[j]->sentence_index();
      CoreferenceSentence *sentence = document->GetSentence(i);
      sentence->AddCoreferenceSpan(span);
    }
  }

  LOG(INFO) << "Predicted " << num_entities << " entities for " << mentions.size()
            << " mentions.";
}

