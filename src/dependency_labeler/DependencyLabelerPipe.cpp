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

#include "DependencyLabelerPipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <sys/time.h>

// Define the current model version and the oldest back-compatible version.
// The format is AAAA.BBBB.CCCC, e.g., 2 0003 0000 means "2.3.0".
const uint64_t kDependencyLabelerModelVersion = 200030000;
const uint64_t kOldestCompatibleDependencyLabelerModelVersion = 200030000;
const uint64_t kDependencyLabelerModelCheck = 1234567890;

void DependencyLabelerPipe::SaveModel(FILE* fs) {
  bool success;
  success = WriteUINT64(fs, kDependencyLabelerModelCheck);
  CHECK(success);
  success = WriteUINT64(fs, kDependencyLabelerModelVersion);
  CHECK(success);
  token_dictionary_->Save(fs);
  Pipe::SaveModel(fs);
  //pruner_parameters_->Save(fs);
}

void DependencyLabelerPipe::LoadModel(FILE* fs) {
  bool success;
  uint64_t model_check;
  uint64_t model_version;
  success = ReadUINT64(fs, &model_check);
  CHECK(success);
  CHECK_EQ(model_check, kDependencyLabelerModelCheck)
    << "The model file is too old and not supported anymore.";
  success = ReadUINT64(fs, &model_version);
  CHECK(success);
  CHECK_GE(model_version, kOldestCompatibleDependencyLabelerModelVersion)
    << "The model file is too old and not supported anymore.";
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<DependencyDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->Load(fs);
  Pipe::LoadModel(fs);
  //pruner_parameters_->Load(fs);
}

void DependencyLabelerPipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<DependencyDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->InitializeFromDependencyReader(GetDependencyReader());
  static_cast<DependencyDictionary*>(dictionary_)->
    CreateLabelDictionary(GetDependencyReader());
}

// TODO(atm): implement this.
void DependencyLabelerPipe::ComputeScores(Instance *instance, Parts *parts,
                                          Features *features,
                                          std::vector<double> *scores) {
  Parameters *parameters = parameters_;
  scores->resize(parts->size());
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  for (int r = 0; r < parts->size(); ++r) {
    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if (pruner) CHECK_EQ((*parts)[r]->type(), DEPENDENCYPART_ARC);
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) continue;
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !pruner &&
        GetDependencyOptions()->labeled()) {
      (*scores)[r] = 0.0;
      DependencyPartArc *arc = static_cast<DependencyPartArc*>((*parts)[r]);
      const vector<int> &index_labeled_parts =
          dependency_parts->FindLabeledArcs(arc->head(), arc->modifier());
      vector<int> allowed_labels(index_labeled_parts.size());
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        DependencyPartLabeledArc *labeled_arc =
            static_cast<DependencyPartLabeledArc*>(
                (*parts)[index_labeled_parts[k]]);
        allowed_labels[k] = labeled_arc->label();
      }
      vector<double> label_scores;
      parameters->ComputeLabelScores(part_features, allowed_labels,
          &label_scores);
      for (int k = 0; k < index_labeled_parts.size(); ++k) {
        (*scores)[index_labeled_parts[k]] = label_scores[k];
      }
      continue;
    }
    (*scores)[r] = parameters->ComputeScore(part_features);
  }
}

// TODO(atm): implement this.
void DependencyLabelerPipe::RemoveUnsupportedFeatures(
    Instance *instance, Parts *parts,
    const std::vector<bool> &selected_parts,
    Features *features) {
  Parameters *parameters = parameters_;

  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;
    if (pruner) CHECK_EQ((*parts)[r]->type(), DEPENDENCYPART_ARC);
    // Skip labeled arcs, are they use the features from unlabeled arcs.
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) continue;
    BinaryFeatures *part_features =
      static_cast<DependencyFeatures*>(features)->GetMutablePartFeatures(r);
    int num_supported = 0;
    for (int j = 0; j < part_features->size(); ++j) {
      if (parameters->Exists((*part_features)[j])) {
        (*part_features)[num_supported] = (*part_features)[j];
        ++num_supported;
      }
    }
    part_features->resize(num_supported);
  }
}

// TODO(atm): implement this.
void DependencyLabelerPipe::MakeGradientStep(
    Parts *parts,
    Features *features,
    double eta,
    int iteration,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  Parameters *parameters = GetTrainingParameters();

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) {
      DependencyPartLabeledArc *labeled_arc =
                  static_cast<DependencyPartLabeledArc*>((*parts)[r]);
      int index_part = dependency_parts->FindArc(labeled_arc->head(),
                                                 labeled_arc->modifier());
      CHECK_GE(index_part, 0);

      const BinaryFeatures &part_features =
          features->GetPartFeatures(index_part);

      parameters->MakeLabelGradientStep(part_features, eta, iteration,
                                         labeled_arc->label(),
                                         predicted_output[r] - gold_output[r]);
    } else if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !train_pruner_ &&
                GetDependencyOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    } else {
      const BinaryFeatures &part_features =
          features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, eta, iteration,
        predicted_output[r] - gold_output[r]);
    }
  }
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::TouchParameters(
    Parts *parts,
    Features *features,
    const std::vector<bool> &selected_parts) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  Parameters *parameters = GetTrainingParameters();

  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) {
      DependencyPartLabeledArc *labeled_arc =
                  static_cast<DependencyPartLabeledArc*>((*parts)[r]);
      int index_part = dependency_parts->FindArc(labeled_arc->head(),
                                                 labeled_arc->modifier());
      CHECK_GE(index_part, 0);

      const BinaryFeatures &part_features =
          features->GetPartFeatures(index_part);

      parameters->MakeLabelGradientStep(part_features, 0.0, 0,
                                         labeled_arc->label(),
                                         0.0);
    } else if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !train_pruner_ &&
                GetDependencyOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    } else {
      const BinaryFeatures &part_features =
          features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, 0.0, 0, 0.0);
    }
  }
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::MakeFeatureDifference(
    Parts *parts,
    Features *features,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output,
    FeatureVector *difference) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) {
      DependencyPartLabeledArc *labeled_arc =
                  static_cast<DependencyPartLabeledArc*>((*parts)[r]);
      int index_part = dependency_parts->FindArc(labeled_arc->head(),
                                                 labeled_arc->modifier());
      CHECK_GE(index_part, 0);
      const BinaryFeatures &part_features =
          features->GetPartFeatures(index_part);

      for (int j = 0; j < part_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(part_features[j],
                                          labeled_arc->label(),
                                          predicted_output[r] - gold_output[r]);
      }
    } else if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !train_pruner_ &&
                GetDependencyOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    } else {
      const BinaryFeatures &part_features = features->GetPartFeatures(r);

      for (int j = 0; j < part_features.size(); ++j) {
        difference->mutable_weights()->Add(part_features[j],
                                  predicted_output[r] - gold_output[r]);
      }
    }
  }
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::MakeParts(Instance *instance,
                                      Parts *parts,
                                      std::vector<double> *gold_outputs) {
  int sentence_length =
      static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  dependency_parts->Initialize();
  bool make_gold = (gold_outputs != NULL);
  if (make_gold) gold_outputs->clear();

  if (train_pruner_) {
    // For the pruner, make only unlabeled arc-factored parts and compute
    // indices.
    MakePartsBasic(instance, false, parts, gold_outputs);
    dependency_parts->BuildOffsets();
    dependency_parts->BuildIndices(sentence_length, false);
  } else {
    // Make arc-factored parts and compute indices.
    MakePartsBasic(instance, parts, gold_outputs);
    dependency_parts->BuildOffsets();
    dependency_parts->BuildIndices(sentence_length,
        GetDependencyOptions()->labeled());

    // Make global parts.
    MakePartsGlobal(instance, parts, gold_outputs);
    dependency_parts->BuildOffsets();
  }
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::MakePartsBasic(Instance *instance,
                                           Parts *parts,
                                           std::vector<double> *gold_outputs) {
  int sentence_length =
      static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  MakePartsBasic(instance, false, parts, gold_outputs);
  dependency_parts->BuildOffsets();
  dependency_parts->BuildIndices(sentence_length, false);

  // Prune using a basic first-order model.
  if (GetDependencyOptions()->prune_basic()) {
    if (options_->train()) {
      Prune(instance, parts, gold_outputs, true);
    } else {
      Prune(instance, parts, gold_outputs, false);
    }
    dependency_parts->BuildOffsets();
    dependency_parts->BuildIndices(sentence_length, false);
  }

  if (GetDependencyOptions()->labeled()) {
    MakePartsBasic(instance, true, parts, gold_outputs);
  }
}

// TODO(atm): Merge with above.
void DependencyPipe::MakePartsBasic(Instance *instance,
                                    bool add_labeled_parts,
                                    Parts *parts,
                                    vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence = 
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyDictionary *dependency_dictionary = GetDependencyDictionary();
  DependencyOptions *dependency_options = GetDependencyOptions();
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  bool prune_labels = dependency_options->prune_labels();
  bool prune_distances = dependency_options->prune_distances();
  vector<int> allowed_labels;

  if (add_labeled_parts && !prune_labels) {
    allowed_labels.resize(dependency_dictionary->GetLabelAlphabet().size());
    for (int i = 0; i < allowed_labels.size(); ++i) {
      allowed_labels[i] = i;
    }
  }

  int num_parts_initial = dependency_parts->size();

  for (int h = 0; h < sentence_length; ++h) {
    for (int m = 1; m < sentence_length; ++m) {
      if (h == m) continue;
      if (add_labeled_parts) {
        // If no unlabeled arc is there, just skip it.
        // This happens if that arc was pruned out.
        if (0 > dependency_parts->FindArc(h, m)) continue;
      } else {
        if (h != 0 && prune_distances) {
          int modifier_pos_id = sentence->GetPosId(m);
          int head_pos_id = sentence->GetPosId(h);
          if (h < m) {
            // Right attachment.
            if (m - h > dependency_dictionary->GetMaximumRightDistance
                (modifier_pos_id, head_pos_id)) continue;
          } else {
            // Left attachment.
            if (h - m > dependency_dictionary->GetMaximumLeftDistance
                (modifier_pos_id, head_pos_id)) continue;
          }
        }
      }

      if (prune_labels) {
        int modifier_pos_id = sentence->GetPosId(m);
        int head_pos_id = sentence->GetPosId(h);
        allowed_labels.clear();
        allowed_labels = dependency_dictionary->
          GetExistingLabels(modifier_pos_id, head_pos_id);
        if (!add_labeled_parts && allowed_labels.empty()) {
          VLOG_IF(2, h == 0) << "No allowed labels between "
                               << token_dictionary_->GetPosTagName(head_pos_id)
                               << " and "
                               << token_dictionary_->GetPosTagName(modifier_pos_id);
          continue;
        }
      }

      // Add parts for labeled/unlabeled arcs.
      if (add_labeled_parts) {
        // If there is no allowed label for this arc, but the unlabeled arc was added, 
        // then it was forced to be present to maintain connectivity of the 
        // graph. In that case (which should be pretty rare) consider all the 
        // possible labels.
        if (allowed_labels.empty()) {
          allowed_labels.resize(dependency_dictionary->GetLabelAlphabet().size());
          for (int l = 0; l < allowed_labels.size(); ++l) {
            allowed_labels[l] = l;
          }
        }
        for (int k = 0; k < allowed_labels.size(); ++k) {
          int l = allowed_labels[k];
          Part *part = dependency_parts->CreatePartLabeledArc(h, m, l);
          dependency_parts->push_back(part);
          if (make_gold) {
            if (sentence->GetHead(m) == h && sentence->GetRelationId(m) == l) {
              gold_outputs->push_back(1.0);
            } else {
              gold_outputs->push_back(0.0);
            }
          }
        }
      } else {
        Part *part = dependency_parts->CreatePartArc(h, m);
        dependency_parts->push_back(part);
        if (make_gold) {
          if (sentence->GetHead(m) == h) {
            gold_outputs->push_back(1.0);
          } else {
            gold_outputs->push_back(0.0);
          }
        }
      }
    }
  }

  // When adding unlabeled arcs, make sure the graph stays connected.
  // Otherwise, enforce connectedness by adding some extra arcs
  // that connect words to the root.
  // NOTE: if --projective, enforcing connectedness is not enough,
  // so we add arcs of the form m-1 -> m to make sure the sentence
  // has a projective parse.
  if (!add_labeled_parts) {
    vector<Part*> arcs(dependency_parts->begin() +
                       num_parts_initial,
                       dependency_parts->end());
    if (dependency_options->projective()) {
      vector<int> inserted_heads;
      vector<int> inserted_modifiers;
      EnforceProjectiveGraph(sentence, arcs, &inserted_heads,
                             &inserted_modifiers);
      for (int k = 0; k < inserted_modifiers.size(); ++k) {
        int m = inserted_modifiers[k];
        int h = inserted_heads[k];
        Part *part = dependency_parts->CreatePartArc(h, m);
        dependency_parts->push_back(part);
        if (make_gold) {
          if (sentence->GetHead(m) == h) {
            gold_outputs->push_back(1.0);
          } else {
            gold_outputs->push_back(0.0);
          }
        }
      }
    } else {
      vector<int> inserted_root_nodes;
      EnforceConnectedGraph(sentence, arcs, &inserted_root_nodes);
      for (int k = 0; k < inserted_root_nodes.size(); ++k) {
        int m = inserted_root_nodes[k];
        int h = 0;
        Part *part = dependency_parts->CreatePartArc(h, m);
        dependency_parts->push_back(part);
        if (make_gold) {
          if (sentence->GetHead(m) == h) {
            gold_outputs->push_back(1.0);
          } else {
            gold_outputs->push_back(0.0);
          }
        }
      }
    }

    dependency_parts->SetOffsetArc(num_parts_initial,
        dependency_parts->size() - num_parts_initial);
  } else {
    dependency_parts->SetOffsetLabeledArc(num_parts_initial,
        dependency_parts->size() - num_parts_initial);
  }
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::MakePartsGlobal(Instance *instance,
                                            Parts *parts,
                                            std::vector<double> *gold_outputs) {
  DependencyOptions *dependency_options = GetDependencyOptions();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);

  int num_parts_initial = dependency_parts->size();
  if (dependency_options->use_arbitrary_siblings()) {
    MakePartsArbitrarySiblings(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetSibl(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_consecutive_siblings()) {
    MakePartsConsecutiveSiblings(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetNextSibl(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_grandparents()) {
    MakePartsGrandparents(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetGrandpar(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_grandsiblings()) {
    MakePartsGrandSiblings(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetGrandSibl(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_trisiblings()) {
    MakePartsTriSiblings(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetTriSibl(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_nonprojective_arcs()) {
    MakePartsNonprojectiveArcs(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetNonproj(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_directed_paths()) {
    MakePartsDirectedPaths(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetPath(num_parts_initial,
      dependency_parts->size() - num_parts_initial);

  num_parts_initial = dependency_parts->size();
  if (dependency_options->use_head_bigrams()) {
    MakePartsHeadBigrams(instance, parts, gold_outputs);
  }
  dependency_parts->SetOffsetHeadBigr(num_parts_initial,
      dependency_parts->size() - num_parts_initial);
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::MakeSelectedFeatures(
    Instance *instance,
    Parts *parts,
    bool pruner,
    const std::vector<bool>& selected_parts,
    Features *features) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyFeatures *dependency_features =
    static_cast<DependencyFeatures*>(features);
  int sentence_length = sentence->size();

  dependency_features->Initialize(instance, parts);

  // Even in the case of labeled parsing, build features for unlabeled arcs
  // only. They will later be conjoined with the labels.
  int offset, size;
  dependency_parts->GetOffsetArc(&offset, &size);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartArc *arc =
      static_cast<DependencyPartArc*>((*dependency_parts)[r]);
    CHECK_GE(arc->head(), 0);
    if (pruner) {
      dependency_features->AddArcFeaturesLight(sentence, r, arc->head(),
                                               arc->modifier());
    } else {
      dependency_features->AddArcFeatures(sentence, r, arc->head(),
                                          arc->modifier());
    }
  }

  // Build features for arbitrary siblings.
  dependency_parts->GetOffsetSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartSibl *part =
      static_cast<DependencyPartSibl*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_SIBL);
    dependency_features->AddArbitrarySiblingFeatures(sentence, r,
      part->head(), part->modifier(), part->sibling());
  }

  // Build features for consecutive siblings.
  dependency_parts->GetOffsetNextSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartNextSibl *part =
      static_cast<DependencyPartNextSibl*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_NEXTSIBL);
    dependency_features->AddConsecutiveSiblingFeatures(sentence, r,
      part->head(), part->modifier(), part->next_sibling());
  }

  // Build features for grandparents.
  dependency_parts->GetOffsetGrandpar(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartGrandpar *part =
      static_cast<DependencyPartGrandpar*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_GRANDPAR);
    CHECK_LE(part->modifier(), sentence_length);
    dependency_features->AddGrandparentFeatures(sentence, r,
      part->grandparent(), part->head(), part->modifier());
  }

  // Build features for grand-siblings.
  dependency_parts->GetOffsetGrandSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartGrandSibl *part =
      static_cast<DependencyPartGrandSibl*>((*dependency_parts)[r]);
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
    DependencyPartTriSibl *part =
      static_cast<DependencyPartTriSibl*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_TRISIBL);
    dependency_features->AddTriSiblingFeatures(sentence, r,
                                               part->head(),
                                               part->modifier(),
                                               part->sibling(),
                                               part->other_sibling());
  }

  // Build features for nonprojective arcs.
  dependency_parts->GetOffsetNonproj(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartNonproj *part =
      static_cast<DependencyPartNonproj*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_NONPROJ);
    dependency_features->AddNonprojectiveArcFeatures(sentence, r,
      part->head(), part->modifier());
  }

  // Build features for directed paths.
  dependency_parts->GetOffsetPath(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartPath *part =
      static_cast<DependencyPartPath*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_PATH);
    dependency_features->AddDirectedPathFeatures(sentence, r,
      part->ancestor(), part->descendant());
  }

  // Build features for head bigrams.
  dependency_parts->GetOffsetHeadBigr(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartHeadBigram *part =
      static_cast<DependencyPartHeadBigram*>((*dependency_parts)[r]);
    CHECK_EQ(part->type(), DEPENDENCYPART_HEADBIGRAM);
    dependency_features->AddHeadBigramFeatures(sentence, r,
      part->head(), part->modifier(), part->previous_head());
  }
}

// TODO(atm): Implement this.
void DependencyLabelerPipe::LabelInstance(Parts *parts,
                                          const std::vector<double> &output,
                                          Instance *instance) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyInstance *dependency_instance =
      static_cast<DependencyInstance*>(instance);
  int instance_length = dependency_instance->size();
  for (int m = 0; m < instance_length; ++m) {
    dependency_instance->SetHead(m, -1);
    if (GetDependencyOptions()->labeled()) {
      dependency_instance->SetDependencyRelation(m, "NULL");
    }
  }
  double threshold = 0.5;

  if (GetDependencyOptions()->labeled()) {
    int offset, num_labeled_arcs;
    dependency_parts->GetOffsetLabeledArc(&offset, &num_labeled_arcs);
    for (int r = 0; r < num_labeled_arcs; ++r) {
      DependencyPartLabeledArc *arc =
          static_cast<DependencyPartLabeledArc*>((*dependency_parts)[offset + r]);
      if (output[offset + r] >= threshold) {
        dependency_instance->SetHead(arc->modifier(), arc->head());
        dependency_instance->SetDependencyRelation(arc->modifier(), 
          GetDependencyDictionary()->GetLabelName(arc->label()));
      }
    }
  } else {
    int offset, num_basic_parts;
    dependency_parts->GetOffsetArc(&offset, &num_basic_parts);
    for (int r = 0; r < num_basic_parts; ++r) {
      DependencyPartArc *arc =
          static_cast<DependencyPartArc*>((*dependency_parts)[offset + r]);
      if (output[offset + r] >= threshold) {
        dependency_instance->SetHead(arc->modifier(), arc->head());
      }
    }
  }
  for (int m = 1; m < instance_length; ++m) {
    if (dependency_instance->GetHead(m) < 0) {
      VLOG(2) << "Word without head.";
      dependency_instance->SetHead(m, 0);
      if (GetDependencyOptions()->labeled()) {
        dependency_instance->SetDependencyRelation(m, GetDependencyDictionary()->GetLabelName(0));
      }
    }
  }
}

