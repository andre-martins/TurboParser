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

#include "SemanticPipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#include <sys/time.h>

using namespace std;

DEFINE_bool(use_only_labeled_arc_features, true,
            "True for not using unlabeled arc features in addition to labeled ones.");
DEFINE_bool(use_only_labeled_sibling_features, true,
            "True for not using unlabeled sibling features in addition to labeled ones.");
DEFINE_bool(use_labeled_sibling_features, true,
            "True for using labels in sibling features.");


void SemanticPipe::SaveModel(FILE* fs) {
  token_dictionary_->Save(fs);
  dependency_dictionary_->Save(fs);
  Pipe::SaveModel(fs);
  pruner_parameters_->Save(fs);
}

void SemanticPipe::LoadModel(FILE* fs) {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<SemanticDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->Load(fs);
  CreateDependencyDictionary();
  dependency_dictionary_->SetTokenDictionary(token_dictionary_);
  static_cast<SemanticDictionary*>(dictionary_)->
    SetDependencyDictionary(dependency_dictionary_);
  dependency_dictionary_->Load(fs);
  Pipe::LoadModel(fs);
  pruner_parameters_->Load(fs);
}

void SemanticPipe::LoadPrunerModel(FILE* fs) {
  LOG(INFO) << "Loading pruner model...";
  // This will be ignored but must be passed to the pruner pipe constructor,
  // so that when loading the pruner model the actual options are not
  // overwritten.
  SemanticOptions pruner_options; // = *options_;
  SemanticPipe* pipe = new SemanticPipe(&pruner_options);
  //SemanticPipe* pipe = new SemanticPipe(options_);
  pipe->Initialize();
  pipe->LoadModel(fs);
  delete pruner_parameters_;
  pruner_parameters_ = pipe->parameters_;
  pipe->parameters_ = NULL;
  delete pipe;
  LOG(INFO) << "Done.";
}

void SemanticPipe::LoadPrunerModelByName(const string &model_name) {
  FILE *fs = fopen(model_name.c_str(), "rb");
  CHECK(fs) << "Could not open pruner model file for reading: " << model_name;
  LoadPrunerModel(fs);
  fclose(fs);
}

void SemanticPipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<SemanticDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->InitializeFromReader(GetSemanticReader());
  delete dependency_dictionary_;
  CreateDependencyDictionary();
  dependency_dictionary_->SetTokenDictionary(token_dictionary_);
  static_cast<SemanticDictionary*>(dictionary_)->
    SetDependencyDictionary(dependency_dictionary_);
  dependency_dictionary_->CreateLabelDictionary(GetSemanticReader());
  static_cast<SemanticDictionary*>(dictionary_)->
    CreatePredicateRoleDictionaries(GetSemanticReader());
}

void SemanticPipe::ComputeScores(Instance *instance, Parts *parts,
                                 Features *features,
                                 bool pruner,
                                 vector<double> *scores) {
  Parameters *parameters;
  SemanticDictionary *semantic_dictionary =
    static_cast<SemanticDictionary*>(dictionary_);
  SemanticFeatures *semantic_features =
    static_cast<SemanticFeatures*>(features);
  if (pruner) {
    parameters = pruner_parameters_;
  } else {
    parameters = parameters_;
  }
  scores->resize(parts->size());
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  for (int r = 0; r < parts->size(); ++r) {
    bool has_unlabeled_features =
      (semantic_features->GetNumPartFeatures(r) > 0);
    bool has_labeled_features =
      (semantic_features->GetNumLabeledPartFeatures(r) > 0);

    if (pruner) CHECK((*parts)[r]->type() == SEMANTICPART_ARC ||
                      (*parts)[r]->type() == SEMANTICPART_PREDICATE);
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDSIBLING) continue;

    // Compute scores for the unlabeled features.
    if (has_unlabeled_features) {
      const BinaryFeatures &part_features =
        semantic_features->GetPartFeatures(r);
      (*scores)[r] = parameters->ComputeScore(part_features);
    } else {
      (*scores)[r] = 0.0;
    }

    // Compute scores for the labeled features.
    if ((*parts)[r]->type() == SEMANTICPART_ARC && !pruner &&
        GetSemanticOptions()->labeled()) {
      // Labeled arcs will be treated by looking at the unlabeled arcs and
      // conjoining with the label.
      CHECK(has_labeled_features);
      SemanticPartArc *arc = static_cast<SemanticPartArc*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
          semantic_parts->FindLabeledArcs(arc->predicate(),
                                          arc->argument(),
                                          arc->sense());
      vector<int> allowed_labels(index_labeled_parts.size());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        CHECK_GE(index_labeled_parts[k], 0);
        CHECK_LT(index_labeled_parts[k], parts->size());
        SemanticPartLabeledArc *labeled_arc =
            static_cast<SemanticPartLabeledArc*>(
                (*parts)[index_labeled_parts[k]]);
        CHECK(labeled_arc != NULL);
        allowed_labels[k] = labeled_arc->role();
      }
      vector<double> label_scores;
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      parameters->ComputeLabelScores(part_features, allowed_labels,
          &label_scores);
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        (*scores)[index_labeled_parts[k]] = label_scores[k];
      }
    } else if ((*parts)[r]->type() == SEMANTICPART_SIBLING &&
               has_labeled_features) {
      // Labeled siblings will be treated by looking at the unlabeled ones and
      // conjoining with the label.
      CHECK(!pruner);
      CHECK(GetSemanticOptions()->labeled());
      SemanticPartSibling *sibling =
        static_cast<SemanticPartSibling*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
        semantic_parts->GetLabeledParts(r);
      vector<int> bigram_labels(index_labeled_parts.size());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        CHECK_GE(index_labeled_parts[k], 0);
        CHECK_LT(index_labeled_parts[k], parts->size());
        SemanticPartLabeledSibling *labeled_sibling =
            static_cast<SemanticPartLabeledSibling*>(
                (*parts)[index_labeled_parts[k]]);
        CHECK(labeled_sibling != NULL);
        bigram_labels[k] = semantic_dictionary->GetRoleBigramLabel(
                               labeled_sibling->first_role(),
                               labeled_sibling->second_role());
      }
      vector<double> label_scores;
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      parameters->ComputeLabelScores(part_features, bigram_labels,
          &label_scores);
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        (*scores)[index_labeled_parts[k]] = label_scores[k];
      }
    }
  }
}

void SemanticPipe::RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                             bool pruner,
                                             const vector<bool> &selected_parts,
                                             Features *features) {
  Parameters *parameters;
  SemanticFeatures *semantic_features =
    static_cast<SemanticFeatures*>(features);
  if (pruner) {
    parameters = pruner_parameters_;
  } else {
    parameters = parameters_;
  }

  for (int r = 0; r < parts->size(); ++r) {
    // TODO: Make sure we can do this continue for the labeled parts...
    if (!selected_parts[r]) continue;

    bool has_unlabeled_features =
      (semantic_features->GetNumPartFeatures(r) > 0);
    bool has_labeled_features =
      (semantic_features->GetNumLabeledPartFeatures(r) > 0);

    if (pruner) CHECK((*parts)[r]->type() == SEMANTICPART_ARC ||
                      (*parts)[r]->type() == SEMANTICPART_PREDICATE);

    // TODO(atm): I think this is handling the case there can be labeled
    // features, but was never tested.
    CHECK(!has_labeled_features);

    // Skip labeled arcs, as they use the features from unlabeled arcs.
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDSIBLING) continue;

    if (has_unlabeled_features) {
      BinaryFeatures *part_features =
        semantic_features->GetMutablePartFeatures(r);
      int num_supported = 0;
      for (int j = 0; j < part_features->size(); ++j) {
        if (parameters->Exists((*part_features)[j])) {
          (*part_features)[num_supported] = (*part_features)[j];
          ++num_supported;
        }
      }
      part_features->resize(num_supported);
    }

    if (has_labeled_features) {
      BinaryFeatures *part_features =
        semantic_features->GetMutableLabeledPartFeatures(r);
      int num_supported = 0;
      for (int j = 0; j < part_features->size(); ++j) {
        if (parameters->ExistsLabeled((*part_features)[j])) {
          (*part_features)[num_supported] = (*part_features)[j];
          ++num_supported;
        }
      }
      part_features->resize(num_supported);
    }
  }
}

void SemanticPipe::MakeGradientStep(Parts *parts,
                                    Features *features,
                                    double eta,
                                    int iteration,
                                    const vector<double> &gold_output,
                                    const vector<double> &predicted_output) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticDictionary *semantic_dictionary =
    static_cast<SemanticDictionary*>(dictionary_);
  SemanticFeatures *semantic_features =
    static_cast<SemanticFeatures*>(features);
  Parameters *parameters = GetTrainingParameters();

  for (int r = 0; r < parts->size(); ++r) {
    bool has_unlabeled_features =
      (semantic_features->GetNumPartFeatures(r) > 0);
    bool has_labeled_features =
      (semantic_features->GetNumLabeledPartFeatures(r) > 0);

    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDSIBLING) continue;

    // Make updates for the unlabeled features.
    if (has_unlabeled_features) {
      if (predicted_output[r] != gold_output[r]) {
        const BinaryFeatures &part_features =
            semantic_features->GetPartFeatures(r);
        parameters->MakeGradientStep(part_features, eta, iteration,
                                     predicted_output[r] - gold_output[r]);
      }
    }

    // Make updates for the labeled features.
    if ((*parts)[r]->type() == SEMANTICPART_ARC && has_labeled_features) {
      // Labeled arcs will be treated by looking at the unlabeled arcs and
      // conjoining with the label.
      CHECK(has_labeled_features);
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      SemanticPartArc *arc = static_cast<SemanticPartArc*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
          semantic_parts->FindLabeledArcs(arc->predicate(),
                                          arc->argument(),
                                          arc->sense());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(index_part, parts->size());
        SemanticPartLabeledArc *labeled_arc =
            static_cast<SemanticPartLabeledArc*>((*parts)[index_part]);
        CHECK(labeled_arc != NULL);
        double value = predicted_output[index_part] - gold_output[index_part];
        if (value != 0.0) {
          parameters->MakeLabelGradientStep(part_features, eta, iteration,
                                            labeled_arc->role(),
                                            value);
        }
      }
    } else if ((*parts)[r]->type() == SEMANTICPART_SIBLING &&
               has_labeled_features) {
      // Labeled siblings will be treated by looking at the unlabeled ones and
      // conjoining with the label.
      CHECK(GetSemanticOptions()->labeled());
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      SemanticPartSibling *sibling =
        static_cast<SemanticPartSibling*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
        semantic_parts->GetLabeledParts(r);
      vector<int> bigram_labels(index_labeled_parts.size());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(index_part, parts->size());
        SemanticPartLabeledSibling *labeled_sibling =
            static_cast<SemanticPartLabeledSibling*>(
                (*parts)[index_part]);
        CHECK(labeled_sibling != NULL);
        int bigram_label = semantic_dictionary->GetRoleBigramLabel(
                               labeled_sibling->first_role(),
                               labeled_sibling->second_role());
        double value = predicted_output[index_part] - gold_output[index_part];
        if (value != 0.0) {
          parameters->MakeLabelGradientStep(part_features, eta, iteration,
                                            bigram_label, value);
        }
      }
    }
  }
}

void SemanticPipe::TouchParameters(Parts *parts, Features *features,
                                   const vector<bool> &selected_parts) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticDictionary *semantic_dictionary =
    static_cast<SemanticDictionary*>(dictionary_);
  SemanticFeatures *semantic_features =
    static_cast<SemanticFeatures*>(features);
  Parameters *parameters = GetTrainingParameters();

  for (int r = 0; r < parts->size(); ++r) {
    // TODO: Make sure we can do this continue for the labeled parts...
    if (!selected_parts[r]) continue;

    bool has_unlabeled_features =
      (semantic_features->GetNumPartFeatures(r) > 0);
    bool has_labeled_features =
      (semantic_features->GetNumLabeledPartFeatures(r) > 0);

    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDSIBLING) continue;

    // Make updates for the unlabeled features.
    if (has_unlabeled_features) {
      const BinaryFeatures &part_features =
        semantic_features->GetPartFeatures(r);
      parameters->MakeGradientStep(part_features, 0.0, 0, 0.0);
    }

    // Make updates for the labeled features.
    if ((*parts)[r]->type() == SEMANTICPART_ARC && has_labeled_features) {
      // Labeled arcs will be treated by looking at the unlabeled arcs and
      // conjoining with the label.
      CHECK(has_labeled_features);
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      SemanticPartArc *arc = static_cast<SemanticPartArc*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
          semantic_parts->FindLabeledArcs(arc->predicate(),
                                          arc->argument(),
                                          arc->sense());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(index_part, parts->size());
        SemanticPartLabeledArc *labeled_arc =
            static_cast<SemanticPartLabeledArc*>((*parts)[index_part]);
        CHECK(labeled_arc != NULL);
        parameters->MakeLabelGradientStep(part_features, 0.0, 0,
                                          labeled_arc->role(), 0.0);
      }
    } else if ((*parts)[r]->type() == SEMANTICPART_SIBLING &&
               has_labeled_features) {
      // Labeled siblings will be treated by looking at the unlabeled ones and
      // conjoining with the label.
      CHECK(GetSemanticOptions()->labeled());
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      SemanticPartSibling *sibling =
        static_cast<SemanticPartSibling*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
        semantic_parts->GetLabeledParts(r);
      vector<int> bigram_labels(index_labeled_parts.size());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(index_part, parts->size());
        SemanticPartLabeledSibling *labeled_sibling =
            static_cast<SemanticPartLabeledSibling*>(
                (*parts)[index_part]);
        CHECK(labeled_sibling != NULL);
        int bigram_label = semantic_dictionary->GetRoleBigramLabel(
                               labeled_sibling->first_role(),
                               labeled_sibling->second_role());
        parameters->MakeLabelGradientStep(part_features, 0.0, 0,
                                          bigram_label, 0.0);
      }
    }
  }
}

void SemanticPipe::MakeFeatureDifference(Parts *parts,
                                         Features *features,
                                         const vector<double> &gold_output,
                                         const vector<double> &predicted_output,
                                         FeatureVector *difference) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticDictionary *semantic_dictionary =
    static_cast<SemanticDictionary*>(dictionary_);
  SemanticFeatures *semantic_features =
    static_cast<SemanticFeatures*>(features);

  for (int r = 0; r < parts->size(); ++r) {
    bool has_unlabeled_features =
      (semantic_features->GetNumPartFeatures(r) > 0);
    bool has_labeled_features =
      (semantic_features->GetNumLabeledPartFeatures(r) > 0);

    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDSIBLING) continue;

    // Compute feature difference for the unlabeled features.
    if (has_unlabeled_features) {
      if (predicted_output[r] != gold_output[r]) {
        const BinaryFeatures &part_features =
          semantic_features->GetPartFeatures(r);
        for (int j = 0; j < part_features.size(); ++j) {
          difference->mutable_weights()->Add(part_features[j],
                                             predicted_output[r] -
                                             gold_output[r]);
        }
      }
    }

    // Make updates for the labeled features.
    if ((*parts)[r]->type() == SEMANTICPART_ARC && has_labeled_features) {
      // Labeled arcs will be treated by looking at the unlabeled arcs and
      // conjoining with the label.
      CHECK(has_labeled_features);
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      SemanticPartArc *arc = static_cast<SemanticPartArc*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
          semantic_parts->FindLabeledArcs(arc->predicate(),
                                          arc->argument(),
                                          arc->sense());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(index_part, parts->size());
        SemanticPartLabeledArc *labeled_arc =
            static_cast<SemanticPartLabeledArc*>((*parts)[index_part]);
        CHECK(labeled_arc != NULL);
        double value = predicted_output[index_part] - gold_output[index_part];
        if (value != 0.0) {
          for (int j = 0; j < part_features.size(); ++j) {
            difference->mutable_labeled_weights()->Add(part_features[j],
                                                       labeled_arc->role(),
                                                       value);
          }
        }
      }
    } else if ((*parts)[r]->type() == SEMANTICPART_SIBLING &&
               has_labeled_features) {
      // Labeled siblings will be treated by looking at the unlabeled ones and
      // conjoining with the label.
      CHECK(GetSemanticOptions()->labeled());
      const BinaryFeatures &part_features =
        semantic_features->GetLabeledPartFeatures(r);
      SemanticPartSibling *sibling =
        static_cast<SemanticPartSibling*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
        semantic_parts->GetLabeledParts(r);
      vector<int> bigram_labels(index_labeled_parts.size());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        int index_part = index_labeled_parts[k];
        CHECK_GE(index_part, 0);
        CHECK_LT(index_part, parts->size());
        SemanticPartLabeledSibling *labeled_sibling =
            static_cast<SemanticPartLabeledSibling*>(
                (*parts)[index_part]);
        CHECK(labeled_sibling != NULL);
        int bigram_label = semantic_dictionary->GetRoleBigramLabel(
                               labeled_sibling->first_role(),
                               labeled_sibling->second_role());
        double value = predicted_output[index_part] - gold_output[index_part];
        if (value != 0.0) {
          for (int j = 0; j < part_features.size(); ++j) {
            difference->mutable_labeled_weights()->Add(part_features[j],
                                                       bigram_label,
                                                       value);
          }
        }
      }
    }
  }
}

void SemanticPipe::MakeParts(Instance *instance,
                             Parts *parts,
                             vector<double> *gold_outputs) {
  int sentence_length =
      static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  semantic_parts->Initialize();
  bool make_gold = (gold_outputs != NULL);
  if (make_gold) gold_outputs->clear();

  if (train_pruner_) {
    // For the pruner, make only unlabeled arc-factored and predicate parts and
    // compute indices.
    MakePartsBasic(instance, false, parts, gold_outputs);
    semantic_parts->BuildOffsets();
    semantic_parts->BuildIndices(sentence_length, false);
  } else {
    // Make arc-factored and predicate parts and compute indices.
    MakePartsBasic(instance, parts, gold_outputs);
    semantic_parts->BuildOffsets();
    semantic_parts->BuildIndices(sentence_length,
        GetSemanticOptions()->labeled());

    // Make global parts.
    MakePartsGlobal(instance, parts, gold_outputs);
    semantic_parts->BuildOffsets();
  }
}

void SemanticPipe::MakePartsBasic(Instance *instance,
                                  Parts *parts,
                                  vector<double> *gold_outputs) {
  int sentence_length =
      static_cast<SemanticInstanceNumeric*>(instance)->size();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  MakePartsBasic(instance, false, parts, gold_outputs);
  semantic_parts->BuildOffsets();
  semantic_parts->BuildIndices(sentence_length, false);

  // Prune using a basic first-order model.
  if (GetSemanticOptions()->prune_basic()) {
    if (options_->train()) {
      Prune(instance, parts, gold_outputs, true);
    } else {
      Prune(instance, parts, gold_outputs, false);
    }
    semantic_parts->BuildOffsets();
    semantic_parts->BuildIndices(sentence_length, false);
  }

  if (GetSemanticOptions()->labeled()) {
    MakePartsBasic(instance, true, parts, gold_outputs);
  }
}

void SemanticPipe::MakePartsBasic(Instance *instance,
                                  bool add_labeled_parts,
                                  Parts *parts,
                                  vector<double> *gold_outputs) {
  SemanticInstanceNumeric *sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticDictionary *semantic_dictionary = GetSemanticDictionary();
  SemanticOptions *semantic_options = GetSemanticOptions();
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  bool prune_labels = semantic_options->prune_labels();
  bool prune_labels_with_relation_paths =
    semantic_options->prune_labels_with_relation_paths();
  bool prune_distances = semantic_options->prune_distances();
  bool allow_self_loops = semantic_options->allow_self_loops();
  bool allow_root_predicate = semantic_options->allow_root_predicate();
  bool allow_unseen_predicates = semantic_options->allow_unseen_predicates();
  bool use_predicate_senses = semantic_options->use_predicate_senses();
  vector<int> allowed_labels;

  if (add_labeled_parts && !prune_labels) {
    allowed_labels.resize(semantic_dictionary->GetRoleAlphabet().size());
    for (int i = 0; i < allowed_labels.size(); ++i) {
      allowed_labels[i] = i;
    }
  }

  // Add predicate parts.
  int num_parts_initial = semantic_parts->size();
  if (!add_labeled_parts) {
    for (int p = 0; p < sentence_length; ++p) {
      if (p == 0 && !allow_root_predicate) continue;
      int lemma_id = TOKEN_UNKNOWN;
      if (use_predicate_senses) {
        lemma_id = sentence->GetLemmaId(p);
        CHECK_GE(lemma_id, 0);
      }
      const vector<SemanticPredicate*> *predicates =
        &semantic_dictionary->GetLemmaPredicates(lemma_id);
      if (predicates->size() == 0 && allow_unseen_predicates) {
        predicates = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
      }
      for (int s = 0; s < predicates->size(); ++s) {
        Part *part = semantic_parts->CreatePartPredicate(p, s);
        semantic_parts->AddPart(part);
        if (make_gold) {
          bool is_gold = false;
          int k = sentence->FindPredicate(p);
          if (k >= 0) {
            int predicate_id = sentence->GetPredicateId(k);
            if (!use_predicate_senses) {
              CHECK_EQ((*predicates)[s]->id(), PREDICATE_UNKNOWN);
            }
            if (predicate_id < 0 || (*predicates)[s]->id() == predicate_id) {
              is_gold = true;
            }
          }
          if (is_gold) {
            gold_outputs->push_back(1.0);
          } else {
            gold_outputs->push_back(0.0);
          }
        }
      }
    }

    // Compute offsets for predicate parts.
    semantic_parts->SetOffsetPredicate(num_parts_initial,
      semantic_parts->size() - num_parts_initial);
  }

  // Add unlabeled/labeled arc parts.
  num_parts_initial = semantic_parts->size();
  for (int p = 0; p < sentence_length; ++p) {
    if (p == 0 && !allow_root_predicate) continue;
    int lemma_id = TOKEN_UNKNOWN;
    if (use_predicate_senses) {
      lemma_id = sentence->GetLemmaId(p);
      CHECK_GE(lemma_id, 0);
    }
    const vector<SemanticPredicate*> *predicates =
      &semantic_dictionary->GetLemmaPredicates(lemma_id);
    if (predicates->size() == 0 && allow_unseen_predicates) {
      predicates = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
    }
    for (int a = 1; a < sentence_length; ++a) {
      if (!allow_self_loops && p == a) continue;
      for (int s = 0; s < predicates->size(); ++s) {
        int arc_index = -1;
        if (add_labeled_parts) {
          // If no unlabeled arc is there, just skip it.
          // This happens if that arc was pruned out.
          arc_index = semantic_parts->FindArc(p, a, s);
          if (0 > arc_index) {
            continue;
          }
        } else {
          if (prune_distances) {
            int predicate_pos_id = sentence->GetPosId(p);
            int argument_pos_id = sentence->GetPosId(a);
            if (p < a) {
              // Right attachment.
              if (a - p > semantic_dictionary->GetMaximumRightDistance
                  (predicate_pos_id, argument_pos_id)) continue;
            } else {
              // Left attachment.
              if (p - a > semantic_dictionary->GetMaximumLeftDistance
                  (predicate_pos_id, argument_pos_id)) continue;
            }
          }
        }

        if (prune_labels_with_relation_paths) {
          int relation_path_id = sentence->GetRelationPathId(p, a);
          allowed_labels.clear();
          if (relation_path_id >= 0 &&
              relation_path_id < semantic_dictionary->
                GetRelationPathAlphabet().size()) {
            allowed_labels = semantic_dictionary->
              GetExistingRolesWithRelationPath(relation_path_id);
            //LOG(INFO) << "Path: " << relation_path_id << " Roles: " << allowed_labels.size();
          }
          set<int> label_set;
          for (int m = 0; m < allowed_labels.size(); ++m) {
            if ((*predicates)[s]->HasRole(allowed_labels[m])) {
              label_set.insert(allowed_labels[m]);
            }
          }
          allowed_labels.clear();
          for (set<int>::iterator it = label_set.begin();
               it != label_set.end(); ++it) {
            allowed_labels.push_back(*it);
          }
          if (!add_labeled_parts && allowed_labels.empty()) {
            continue;
          }
        } else if (prune_labels) {
          // TODO: allow both kinds of label pruning simultaneously?
          int predicate_pos_id = sentence->GetPosId(p);
          int argument_pos_id = sentence->GetPosId(a);
          allowed_labels.clear();
          allowed_labels = semantic_dictionary->
            GetExistingRoles(predicate_pos_id, argument_pos_id);
          set<int> label_set;
          for (int m = 0; m < allowed_labels.size(); ++m) {
            if ((*predicates)[s]->HasRole(allowed_labels[m])) {
              label_set.insert(allowed_labels[m]);
            }
          }
          allowed_labels.clear();
          for (set<int>::iterator it = label_set.begin();
               it != label_set.end(); ++it) {
            allowed_labels.push_back(*it);
          }
          if (!add_labeled_parts && allowed_labels.empty()) {
            continue;
          }
        }

        // Add parts for labeled/unlabeled arcs.
        if (add_labeled_parts) {
          // If there is no allowed label for this arc, but the unlabeled arc was added, 
          // then it was forced to be present for some reason (e.g. to maintain connectivity of the 
          // graph). In that case (which should be pretty rare) consider all the
          // possible labels.
          if (allowed_labels.empty()) {
            allowed_labels.resize(semantic_dictionary->GetRoleAlphabet().size());
            for (int role = 0; role < allowed_labels.size(); ++role) {
              allowed_labels[role] = role;
            }
          }

          for (int m = 0; m < allowed_labels.size(); ++m) {
            int role = allowed_labels[m];
            if (prune_labels) CHECK((*predicates)[s]->HasRole(role));
            Part *part = semantic_parts->CreatePartLabeledArc(p, a, s, role);
            CHECK_GE(arc_index, 0);
            semantic_parts->AddLabeledPart(part, arc_index);
            if (make_gold) {
              int k = sentence->FindPredicate(p);
              int l = sentence->FindArc(p, a);
              bool is_gold = false;

              if (k >= 0 && l >= 0) {
                int predicate_id = sentence->GetPredicateId(k);
                int argument_id = sentence->GetArgumentRoleId(k, l);
                if (!use_predicate_senses) {
                  CHECK_EQ((*predicates)[s]->id(), PREDICATE_UNKNOWN);
                }
                //if (use_predicate_senses) CHECK_LT(predicate_id, 0);
                if ((predicate_id < 0 ||
                     (*predicates)[s]->id() == predicate_id) &&
                    role == argument_id) {
                  is_gold = true;
                }
              }
              if (is_gold) {
                gold_outputs->push_back(1.0);
              } else {
                gold_outputs->push_back(0.0);
              }
            }
          }
        } else {
          Part *part = semantic_parts->CreatePartArc(p, a, s);
          semantic_parts->AddPart(part);
          if (make_gold) {
            int k = sentence->FindPredicate(p);
            int l = sentence->FindArc(p, a);
            bool is_gold = false;
            if (k >= 0 && l >= 0) {
              int predicate_id = sentence->GetPredicateId(k);
              if (!use_predicate_senses) {
                CHECK_EQ((*predicates)[s]->id(), PREDICATE_UNKNOWN);
              }
              if (predicate_id < 0 || (*predicates)[s]->id() == predicate_id) {
                is_gold = true;
              }
            }
            if (is_gold) {
              gold_outputs->push_back(1.0);
            } else {
              gold_outputs->push_back(0.0);
            }
          }
        }
      }
    }
  }

  // Compute offsets for labeled/unlabeled arcs.
  if (!add_labeled_parts) {
    semantic_parts->SetOffsetArc(num_parts_initial,
        semantic_parts->size() - num_parts_initial);
  } else {
    semantic_parts->SetOffsetLabeledArc(num_parts_initial,
        semantic_parts->size() - num_parts_initial);
  }
}

void SemanticPipe::MakePartsArbitrarySiblings(Instance *instance,
                                              Parts *parts,
                                              vector<double> *gold_outputs) {
  SemanticInstanceNumeric *sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  SemanticDictionary *semantic_dictionary = GetSemanticDictionary();
  SemanticOptions *semantic_options = GetSemanticOptions();
  //bool allow_self_loops = semantic_options->allow_self_loops();
  bool allow_root_predicate = semantic_options->allow_root_predicate();
  bool allow_unseen_predicates = semantic_options->allow_unseen_predicates();
  bool use_predicate_senses = semantic_options->use_predicate_senses();

  // Siblings: (p,s,a1) and (p,s,a2).
  for (int p = 0; p < sentence_length; ++p) {
    if (p == 0 && !allow_root_predicate) continue;
    int lemma_id = TOKEN_UNKNOWN;
    if (use_predicate_senses) {
      lemma_id = sentence->GetLemmaId(p);
      CHECK_GE(lemma_id, 0);
    }
    const vector<SemanticPredicate*> *predicates =
      &semantic_dictionary->GetLemmaPredicates(lemma_id);
    if (predicates->size() == 0 && allow_unseen_predicates) {
      predicates = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
    }
    for (int s = 0; s < predicates->size(); ++s) {
      for (int a1 = 1; a1 < sentence_length; ++a1) {
        int r1 = semantic_parts->FindArc(p, a1, s);
        if (r1 < 0) continue;
        for (int a2 = a1+1; a2 < sentence_length; ++a2) {
          int r2 = semantic_parts->FindArc(p, a2, s);
          if (r2 < 0) continue;
          Part *part = semantic_parts->CreatePartSibling(p, s, a1, a2);
          semantic_parts->AddPart(part);
          if (make_gold) {
            // Logical AND of the two individual arcs.
            gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
          }
        }
      }
    }
  }
}

void SemanticPipe::MakePartsLabeledArbitrarySiblings(Instance *instance,
                                                     Parts *parts,
                                                     vector<double> *gold_outputs) {
  SemanticInstanceNumeric *sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  SemanticDictionary *semantic_dictionary = GetSemanticDictionary();
  SemanticOptions *semantic_options = GetSemanticOptions();

  int offset, size;
  semantic_parts->GetOffsetSibling(&offset, &size);
  for (int r = offset; r < offset + size; ++r) {
    SemanticPartSibling *sibling =
      static_cast<SemanticPartSibling*>((*semantic_parts)[r]);
    int p = sibling->predicate();
    int s = sibling->sense();
    int a1 = sibling->first_argument();
    int a2 = sibling->second_argument();
    const vector<int> &labeled_first_arc_indices =
      semantic_parts->FindLabeledArcs(p, a1, s);
    const vector<int> &labeled_second_arc_indices =
      semantic_parts->FindLabeledArcs(p, a2, s);
    for (int k = 0; k < labeled_first_arc_indices.size(); ++k) {
      int r1 = labeled_first_arc_indices[k];
      SemanticPartLabeledArc *first_labeled_arc =
        static_cast<SemanticPartLabeledArc*>((*semantic_parts)[r1]);
      int first_role = first_labeled_arc->role();
      for (int l = 0; l < labeled_second_arc_indices.size(); ++l) {
        int r2 = labeled_second_arc_indices[l];
        SemanticPartLabeledArc *second_labeled_arc =
          static_cast<SemanticPartLabeledArc*>((*semantic_parts)[r2]);
        int second_role = second_labeled_arc->role();
        Part *part = semantic_parts->CreatePartLabeledSibling(p, s, a1, a2,
                                                              first_role,
                                                              second_role);
        semantic_parts->AddLabeledPart(part, r);
        if (make_gold) {
          // Logical AND of the two individual labeled arcs.
          gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
        }
      }
    }
  }
}

void SemanticPipe::MakePartsGrandparents(Instance *instance,
                                         Parts *parts,
                                         vector<double> *gold_outputs) {
  SemanticInstanceNumeric *sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  SemanticDictionary *semantic_dictionary = GetSemanticDictionary();
  SemanticOptions *semantic_options = GetSemanticOptions();
  //bool allow_self_loops = semantic_options->allow_self_loops();
  bool allow_root_predicate = semantic_options->allow_root_predicate();
  bool allow_unseen_predicates = semantic_options->allow_unseen_predicates();
  bool use_predicate_senses = semantic_options->use_predicate_senses();

  // Grandparents: (g,t,p) and (p,s,a).
  for (int g = 0; g < sentence_length; ++g) {
    if (g == 0 && !allow_root_predicate) continue;
    int lemma_id_g = TOKEN_UNKNOWN;
    if (use_predicate_senses) {
      lemma_id_g = sentence->GetLemmaId(g);
      CHECK_GE(lemma_id_g, 0);
    }
    const vector<SemanticPredicate*> *predicates_g =
      &semantic_dictionary->GetLemmaPredicates(lemma_id_g);
    if (predicates_g->size() == 0 && allow_unseen_predicates) {
      predicates_g = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
    }
    for (int t = 0; t < predicates_g->size(); ++t) {
      for (int p = 1; p < sentence_length; ++p) {
        int r1 = semantic_parts->FindArc(g, p, t);
        if (r1 < 0) continue;
        int lemma_id = TOKEN_UNKNOWN;
        if (use_predicate_senses) {
          lemma_id = sentence->GetLemmaId(p);
          CHECK_GE(lemma_id, 0);
        }
        const vector<SemanticPredicate*> *predicates =
          &semantic_dictionary->GetLemmaPredicates(lemma_id);
        if (predicates->size() == 0 && allow_unseen_predicates) {
          predicates = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
        }
        for (int s = 0; s < predicates->size(); ++s) {
          for (int a = 1; a < sentence_length; ++a) {
            int r2 = semantic_parts->FindArc(p, a, s);
            if (r2 < 0) continue;
            Part *part = semantic_parts->CreatePartGrandparent(g, t, p, s, a);
            semantic_parts->AddPart(part);
            if (make_gold) {
              // Logical AND of the two individual arcs.
              gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
            }
          }
        }
      }
    }
  }
}

void SemanticPipe::MakePartsCoparents(Instance *instance,
                                      Parts *parts,
                                      vector<double> *gold_outputs) {
  SemanticInstanceNumeric *sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  SemanticDictionary *semantic_dictionary = GetSemanticDictionary();
  SemanticOptions *semantic_options = GetSemanticOptions();
  //bool allow_self_loops = semantic_options->allow_self_loops();
  bool allow_root_predicate = semantic_options->allow_root_predicate();
  bool allow_unseen_predicates = semantic_options->allow_unseen_predicates();
  bool use_predicate_senses = semantic_options->use_predicate_senses();

  // Co-parents: (p1,s1,a) and (p2,s2,a).
  // First predicate.
  for (int p1 = 0; p1 < sentence_length; ++p1) {
    if (p1 == 0 && !allow_root_predicate) continue;
    int lemma_id_p1 = TOKEN_UNKNOWN;
    if (use_predicate_senses) {
      lemma_id_p1 = sentence->GetLemmaId(p1);
      CHECK_GE(lemma_id_p1, 0);
    }
    const vector<SemanticPredicate*> *predicates_p1 =
      &semantic_dictionary->GetLemmaPredicates(lemma_id_p1);
    if (predicates_p1->size() == 0 && allow_unseen_predicates) {
      predicates_p1 = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
    }
    for (int s1 = 0; s1 < predicates_p1->size(); ++s1) {
      // Second predicate.
      for (int p2 = p1+1; p2 < sentence_length; ++p2) {
        int lemma_id_p2 = TOKEN_UNKNOWN;
        if (use_predicate_senses) {
          lemma_id_p2 = sentence->GetLemmaId(p2);
          CHECK_GE(lemma_id_p2, 0);
        }
        const vector<SemanticPredicate*> *predicates_p2 =
          &semantic_dictionary->GetLemmaPredicates(lemma_id_p2);
        if (predicates_p2->size() == 0 && allow_unseen_predicates) {
          predicates_p2 = &semantic_dictionary->GetLemmaPredicates(TOKEN_UNKNOWN);
        }
        for (int s2 = 0; s2 < predicates_p2->size(); ++s2) {
          // Common argument.
          for (int a = 1; a < sentence_length; ++a) {
            int r1 = semantic_parts->FindArc(p1, a, s1);
            if (r1 < 0) continue;
            int r2 = semantic_parts->FindArc(p2, a, s2);
            if (r2 < 0) continue;
            Part *part = semantic_parts->CreatePartCoparent(p1, s1, p2, s2, a);
            semantic_parts->AddPart(part);
            if (make_gold) {
              // Logical AND of the two individual arcs.
              gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
            }
          }
        }
      }
    }
  }
}

void SemanticPipe::MakePartsGlobal(Instance *instance,
                                   Parts *parts,
                                   vector<double> *gold_outputs) {
  SemanticOptions *semantic_options = GetSemanticOptions();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  int num_parts_initial = semantic_parts->size();
  if (semantic_options->use_arbitrary_siblings()) {
    MakePartsArbitrarySiblings(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetSibling(num_parts_initial,
                                   semantic_parts->size() - num_parts_initial);

  num_parts_initial = semantic_parts->size();
  if (semantic_options->use_arbitrary_siblings() &&
      FLAGS_use_labeled_sibling_features) {
    MakePartsLabeledArbitrarySiblings(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetLabeledSibling(
      num_parts_initial, semantic_parts->size() - num_parts_initial);

  num_parts_initial = semantic_parts->size();
  if (semantic_options->use_grandparents()) {
    MakePartsGrandparents(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetGrandparent(num_parts_initial,
      semantic_parts->size() - num_parts_initial);

  num_parts_initial = semantic_parts->size();
  if (semantic_options->use_coparents()) {
    MakePartsCoparents(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetCoparent(num_parts_initial,
      semantic_parts->size() - num_parts_initial);

#if 0
  num_parts_initial = semantic_parts->size();
  if (semantic_options->use_consecutive_siblings()) {
    MakePartsConsecutiveSiblings(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetNextSibling(num_parts_initial,
      semantic_parts->size() - num_parts_initial);

  num_parts_initial = semantic_parts->size();
  if (semantic_options->use_grandsiblings()) {
    MakePartsGrandSiblings(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetGrandSiblings(num_parts_initial,
      semantic_parts->size() - num_parts_initial);

  num_parts_initial = semantic_parts->size();
  if (semantic_options->use_trisiblings()) {
    MakePartsTriSiblings(instance, parts, gold_outputs);
  }
  semantic_parts->SetOffsetTriSiblings(num_parts_initial,
      semantic_parts->size() - num_parts_initial);
#endif
}

#if 0
void SemanticPipe::GetAllAncestors(const vector<int> &heads,
                                    int descend,
                                    vector<int>* ancestors) {
  ancestors->clear();
  int h = heads[descend];
  while (h >= 0) {
    h = heads[h];
    ancestors->push_back(h);
  }
}

bool SemanticPipe::ExistsPath(const vector<int> &heads,
                                int ancest,
                                int descend) {
  int h = heads[descend];
  while (h != ancest && h >= 0) {
    h = heads[h];
  }
  if (h != ancest) return false;  // No path from ancest to descend.
  return true;
}
#endif

void SemanticPipe::MakeSelectedFeatures(Instance *instance,
                                        Parts *parts,
                                        bool pruner,
                                        const vector<bool>& selected_parts,
                                        Features *features) {
  SemanticInstanceNumeric *sentence =
    static_cast<SemanticInstanceNumeric*>(instance);
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticFeatures *semantic_features =
    static_cast<SemanticFeatures*>(features);
  int sentence_length = sentence->size();

  semantic_features->Initialize(instance, parts);

  // Build features for predicates.
  int offset, size;
  semantic_parts->GetOffsetPredicate(&offset, &size);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartPredicate *predicate_part =
      static_cast<SemanticPartPredicate*>((*semantic_parts)[r]);
    // Get the predicate id for this part.
    // TODO(atm): store this somewhere, so that we don't need to recompute this
    // all the time.
    int lemma_id = TOKEN_UNKNOWN;
    if (GetSemanticOptions()->use_predicate_senses()) {
      lemma_id = sentence->GetLemmaId(predicate_part->predicate());
    }
    const vector<SemanticPredicate*> *predicates =
      &GetSemanticDictionary()->GetLemmaPredicates(lemma_id);
    if (predicates->size() == 0 &&
        GetSemanticOptions()->allow_unseen_predicates()) {
      predicates = &GetSemanticDictionary()->GetLemmaPredicates(TOKEN_UNKNOWN);
    }
    int predicate_id = (*predicates)[predicate_part->sense()]->id();
    // Add the predicate features.
    semantic_features->AddPredicateFeatures(sentence, r,
                                            predicate_part->predicate(),
                                            predicate_id);
  }

  // Even in the case of labeled parsing, build features for unlabeled arcs
  // only. They will later be conjoined with the labels.
  semantic_parts->GetOffsetArc(&offset, &size);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartArc *arc =
      static_cast<SemanticPartArc*>((*semantic_parts)[r]);
    // Get the predicate id for this part.
    // TODO(atm): store this somewhere, so that we don't need to recompute this
    // all the time. Maybe store this directly in arc->sense()?
    int lemma_id = TOKEN_UNKNOWN;
    if (GetSemanticOptions()->use_predicate_senses()) {
      lemma_id = sentence->GetLemmaId(arc->predicate());
    }
    const vector<SemanticPredicate*> *predicates =
      &GetSemanticDictionary()->GetLemmaPredicates(lemma_id);
    if (predicates->size() == 0 &&
        GetSemanticOptions()->allow_unseen_predicates()) {
      predicates = &GetSemanticDictionary()->GetLemmaPredicates(TOKEN_UNKNOWN);
    }
    int predicate_id = (*predicates)[arc->sense()]->id();
    if (!pruner && GetSemanticOptions()->labeled()) {
      semantic_features->AddLabeledArcFeatures(sentence, r, arc->predicate(),
                                               arc->argument(), predicate_id);
      if (!FLAGS_use_only_labeled_arc_features) {
        semantic_features->AddArcFeatures(sentence, r, arc->predicate(),
                                          arc->argument(), predicate_id);
      }
    } else {
      semantic_features->AddArcFeatures(sentence, r, arc->predicate(),
                                        arc->argument(), predicate_id);
    }
  }

  // Build features for arbitrary siblings.
  semantic_parts->GetOffsetSibling(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartSibling *part =
      static_cast<SemanticPartSibling*>((*semantic_parts)[r]);
    CHECK_EQ(part->type(), SEMANTICPART_SIBLING);
    if (FLAGS_use_labeled_sibling_features) {
      semantic_features->
        AddArbitraryLabeledSiblingFeatures(sentence, r,
                                           part->predicate(),
                                           part->sense(),
                                           part->first_argument(),
                                           part->second_argument());
      if (!FLAGS_use_only_labeled_sibling_features) {
        semantic_features->AddArbitrarySiblingFeatures(sentence, r,
                                                       part->predicate(),
                                                       part->sense(),
                                                       part->first_argument(),
                                                       part->second_argument());
      }
    } else {
      semantic_features->AddArbitrarySiblingFeatures(sentence, r,
                                                     part->predicate(),
                                                     part->sense(),
                                                     part->first_argument(),
                                                     part->second_argument());
    }
  }

  // Build features for grandparents.
  semantic_parts->GetOffsetGrandparent(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartGrandparent *part =
      static_cast<SemanticPartGrandparent*>((*semantic_parts)[r]);
    CHECK_EQ(part->type(), SEMANTICPART_GRANDPARENT);
    semantic_features->AddGrandparentFeatures(sentence, r,
                                              part->grandparent_predicate(),
                                              part->grandparent_sense(),
                                              part->predicate(),
                                              part->sense(),
                                              part->argument());
  }

  // Build features for co-parents.
  semantic_parts->GetOffsetCoparent(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartCoparent *part =
      static_cast<SemanticPartCoparent*>((*semantic_parts)[r]);
    CHECK_EQ(part->type(), SEMANTICPART_COPARENT);
    semantic_features->AddCoparentFeatures(sentence, r,
                                           part->first_predicate(),
                                           part->first_sense(),
                                           part->second_predicate(),
                                           part->second_sense(),
                                           part->argument());
  }

#if 0
  // Build features for consecutive siblings.
  dependency_parts->GetOffsetNextSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartNextSibl *part =
      static_cast<SemanticPartNextSibl*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_NEXTSIBL);
    dependency_features->AddConsecutiveSiblingFeatures(sentence, r,
      part->head(), part->modifier(), part->next_sibling());
  }

  // Build features for grand-siblings.
  dependency_parts->GetOffsetGrandSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartGrandSibl *part =
      static_cast<SemanticPartGrandSibl*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_GRANDSIBL);
    CHECK_LE(part->modifier(), sentence_length);
    CHECK_LE(part->sibling(), sentence_length);
    dependency_features->AddGrandSiblingFeatures(sentence, r,
                                                 part->grandparent(),
                                                 part->head(),
                                                 part->modifier(),
                                                 part->sibling());
  }

  // Build features for tri-siblings.
  dependency_parts->GetOffsetTriSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartTriSibl *part =
      static_cast<SemanticPartTriSibl*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_TRISIBL);
    dependency_features->AddTriSiblingFeatures(sentence, r,
                                               part->head(),
                                               part->modifier(),
                                               part->sibling(),
                                               part->other_sibling());
  }

#endif
}

// Prune basic parts (arcs and labeled arcs) using a first-order model.
// The vectors of basic parts is given as input, and those elements that are
// to be pruned are deleted from the vector.
// If gold_outputs is not NULL that vector will also be pruned.
void SemanticPipe::Prune(Instance *instance, Parts *parts,
                         vector<double> *gold_outputs,
                         bool preserve_gold) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  Features *features = CreateFeatures();
  vector<double> scores;
  vector<double> predicted_outputs;

  // Make sure gold parts are only preserved at training time.
  CHECK(!preserve_gold || options_->train());

  MakeFeatures(instance, parts, true, features);
  ComputeScores(instance, parts, features, true, &scores);
  GetSemanticDecoder()->DecodePruner(instance, parts, scores,
                                     &predicted_outputs);

  int offset_predicate_parts, num_predicate_parts;
  int offset_arcs, num_arcs;
  semantic_parts->GetOffsetPredicate(&offset_predicate_parts,
                                     &num_predicate_parts);
  semantic_parts->GetOffsetArc(&offset_arcs, &num_arcs);

  double threshold = 0.5;
  int r0 = offset_arcs; // Preserve all the predicate parts.
  semantic_parts->ClearOffsets();
  semantic_parts->SetOffsetPredicate(offset_predicate_parts,
                                     num_predicate_parts);
  for (int r = 0; r < num_arcs; ++r) {
    // Preserve gold parts (at training time).
    if (predicted_outputs[offset_arcs + r] >= threshold ||
        (preserve_gold && (*gold_outputs)[offset_arcs + r] >= threshold)) {
      (*parts)[r0] = (*parts)[offset_arcs + r];
      semantic_parts->
        SetLabeledParts(r0, semantic_parts->GetLabeledParts(offset_arcs + r));
      if (gold_outputs) {
        (*gold_outputs)[r0] = (*gold_outputs)[offset_arcs + r];
      }
      ++r0;
    } else {
      delete (*parts)[offset_arcs + r];
    }
  }

  if (gold_outputs) gold_outputs->resize(r0);
  semantic_parts->Resize(r0);
  semantic_parts->DeleteIndices();
  semantic_parts->SetOffsetArc(offset_arcs,
                               parts->size() - offset_arcs);

  delete features;
}

void SemanticPipe::LabelInstance(Parts *parts,
                                 const vector<double> &output,
                                 Instance *instance) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  SemanticInstance *semantic_instance =
      static_cast<SemanticInstance*>(instance);
  SemanticDictionary *semantic_dictionary =
    static_cast<SemanticDictionary*>(dictionary_);
  //bool allow_root_predicate = GetSemanticOptions()->allow_root_predicate();
  int instance_length = semantic_instance->size();
  double threshold = 0.5;
  semantic_instance->ClearPredicates();
  for (int p = 0; p < instance_length; ++p) {
    //if (p == 0 && !allow_root_predicate) continue;
    const vector<int> &senses = semantic_parts->GetSenses(p);
    vector<int> argument_indices;
    vector<string> argument_roles;
    int predicted_sense = -1;
    for (int k = 0; k < senses.size(); k++) {
      int s = senses[k];
      for (int a = 1; a < instance_length; ++a) {
        if (GetSemanticOptions()->labeled()) {
          int r = semantic_parts->FindArc(p, a, s);
          if (r < 0) continue;
          const vector<int> &labeled_arcs =
            semantic_parts->FindLabeledArcs(p, a, s);
          for (int l = 0; l < labeled_arcs.size(); ++l) {
            int r = labeled_arcs[l];
            if (output[r] > threshold) {
              if (predicted_sense != s) {
                CHECK_LT(predicted_sense, 0);
                predicted_sense = s;
              }
              argument_indices.push_back(a);
              SemanticPartLabeledArc *labeled_arc =
                static_cast<SemanticPartLabeledArc*>((*parts)[r]);
              string role =
                semantic_dictionary->GetRoleName(labeled_arc->role());
              argument_roles.push_back(role);
            }
          }
        } else {
          int r = semantic_parts->FindArc(p, a, s);
          if (r < 0) continue;
          if (output[r] > threshold) {
            if (predicted_sense != s) {
              CHECK_LT(predicted_sense, 0);
              predicted_sense = s;
            }
            argument_indices.push_back(a);
            argument_roles.push_back("ARG");
          }
        }
      }
    }

    if (predicted_sense >= 0) {
      int s = predicted_sense;
      // Get the predicate id for this part.
      // TODO(atm): store this somewhere, so that we don't need to recompute this
      // all the time. Maybe store this directly in arc->sense()?
      int lemma_id = TOKEN_UNKNOWN;
      if (GetSemanticOptions()->use_predicate_senses()) {
        lemma_id = semantic_dictionary->GetTokenDictionary()->
          GetLemmaId(semantic_instance->GetLemma(p));
        if (lemma_id < 0) lemma_id = TOKEN_UNKNOWN;
      }
      const vector<SemanticPredicate*> *predicates =
        &GetSemanticDictionary()->GetLemmaPredicates(lemma_id);
      if (predicates->size() == 0 &&
          GetSemanticOptions()->allow_unseen_predicates()) {
        predicates = &GetSemanticDictionary()->GetLemmaPredicates(TOKEN_UNKNOWN);
      }
      int predicate_id = (*predicates)[s]->id();
      string predicate_name =
        semantic_dictionary->GetPredicateName(predicate_id);
      semantic_instance->AddPredicate(predicate_name, p, argument_roles,
                                      argument_indices);
    }
  }
}
