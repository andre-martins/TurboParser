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

#include "DependencyLabelerPipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

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

void DependencyLabelerPipe::ComputeScores(Instance *instance, Parts *parts,
                                          Features *features,
                                          std::vector<double> *scores) {
  //LOG(INFO) << "ComputeScores";
  Parameters *parameters = parameters_;
  scores->resize(parts->size());
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyLabelerFeatures *dependency_features =
    static_cast<DependencyLabelerFeatures*>(features);
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyLabelerOptions *dependency_options = GetDependencyLabelerOptions();
  const std::vector<int> &heads = sentence->GetHeads();
  const std::vector<std::vector<int> > &siblings = dependency_parts->siblings();

  for (int m = 1; m < sentence->size(); ++m) {
    // Conjoin arc features with the label.
    const BinaryFeatures &arc_features = dependency_features->GetArcFeatures(m);
    const std::vector<int> &index_arc_parts =
      dependency_parts->FindArcs(m);
    std::vector<int> allowed_labels(index_arc_parts.size());
    for (int k = 0; k < index_arc_parts.size(); ++k) {
      DependencyLabelerPartArc *arc =
          static_cast<DependencyLabelerPartArc*>((*parts)[index_arc_parts[k]]);
      allowed_labels[k] = arc->label();
    }
    std::vector<double> label_scores;
    parameters_->ComputeLabelScores(arc_features, allowed_labels,
        &label_scores);
    for (int k = 0; k < index_arc_parts.size(); ++k) {
      (*scores)[index_arc_parts[k]] = label_scores[k];
    }
  }

  if (dependency_options->use_sibling_parts()) {
    for (int h = 0; h < sentence->size(); ++h) {
      if (siblings[h].size() == 0) continue;
      for (int i = 0; i < siblings[h].size() + 1; ++i) {
        const BinaryFeatures &sibling_features =
          dependency_features->GetSiblingFeatures(h, i);
        const std::vector<int> &index_sibling_parts =
          dependency_parts->FindSiblings(h, i);
        std::vector<int> sibling_labels(index_sibling_parts.size());
        for (int k = 0; k < index_sibling_parts.size(); ++k) {
          DependencyLabelerPartSibling *sibling =
            static_cast<DependencyLabelerPartSibling*>(
              (*parts)[index_sibling_parts[k]]);
          sibling_labels[k] = GetSiblingLabel(sibling->sibling_label(),
                                              sibling->modifier_label());
        }
        std::vector<double> label_scores;
        parameters_->ComputeLabelScores(sibling_features, sibling_labels,
                                        &label_scores);
        for (int k = 0; k < index_sibling_parts.size(); ++k) {
          (*scores)[index_sibling_parts[k]] = label_scores[k];
        }
      }
    }
  }
  //LOG(INFO) << "End ComputeScores";
}

void DependencyLabelerPipe::MakeGradientStep(
    Parts *parts,
    Features *features,
    double eta,
    int iteration,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output) {
  //LOG(INFO) << "MakeGradientStep";
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyLabelerFeatures *dependency_features =
    static_cast<DependencyLabelerFeatures*>(features);
  Parameters *parameters = GetTrainingParameters();
  DependencyLabelerOptions *dependency_options = GetDependencyLabelerOptions();

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == DEPENDENCYLABELERPART_ARC) {
      DependencyLabelerPartArc *arc =
                  static_cast<DependencyLabelerPartArc*>((*parts)[r]);
      const BinaryFeatures &arc_features =
        dependency_features->GetArcFeatures(arc->modifier());

      parameters->MakeLabelGradientStep(arc_features, eta, iteration,
                                        arc->label(),
                                        predicted_output[r] - gold_output[r]);
    } else if ((*parts)[r]->type() == DEPENDENCYLABELERPART_SIBLING) {
      DependencyLabelerPartSibling *sibling =
                  static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
      int sibling_index = dependency_parts->
        GetSiblingIndex(sibling->head(), sibling->modifier());
      const BinaryFeatures &sibling_features =
        dependency_features->GetSiblingFeatures(sibling->head(),
                                                sibling_index);
      int sibling_label = GetSiblingLabel(sibling->sibling_label(),
                                          sibling->modifier_label());
      parameters->MakeLabelGradientStep(sibling_features, eta, iteration,
                                        sibling_label,
                                        predicted_output[r] - gold_output[r]);
    } else {
      CHECK(false);
    }
  }
  //LOG(INFO) << "End MakeGradientStep";
}

void DependencyLabelerPipe::MakeFeatureDifference(
    Parts *parts,
    Features *features,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output,
    FeatureVector *difference) {
  //LOG(INFO) << "MakeFeatureDifference";
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyLabelerFeatures *dependency_features =
    static_cast<DependencyLabelerFeatures*>(features);
  DependencyLabelerOptions *dependency_options = GetDependencyLabelerOptions();

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == DEPENDENCYLABELERPART_ARC) {
      DependencyLabelerPartArc *arc =
                  static_cast<DependencyLabelerPartArc*>((*parts)[r]);
      const BinaryFeatures &arc_features =
        dependency_features->GetArcFeatures(arc->modifier());
      for (int j = 0; j < arc_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(arc_features[j],
            arc->label(), predicted_output[r] - gold_output[r]);
      }
    } else if ((*parts)[r]->type() == DEPENDENCYLABELERPART_SIBLING) {
      DependencyLabelerPartSibling *sibling =
                  static_cast<DependencyLabelerPartSibling*>((*parts)[r]);
      int sibling_index = dependency_parts->
        GetSiblingIndex(sibling->head(), sibling->modifier());
      const BinaryFeatures &sibling_features =
        dependency_features->GetSiblingFeatures(sibling->head(),
                                                sibling_index);
      int sibling_label = GetSiblingLabel(sibling->sibling_label(),
                                          sibling->modifier_label());
      for (int j = 0; j < sibling_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(sibling_features[j],
            sibling_label, predicted_output[r] - gold_output[r]);
      }
    } else {
      CHECK(false);
    }
  }
  //LOG(INFO) << "End MakeFeatureDifference";
}

void DependencyLabelerPipe::MakeParts(Instance *instance,
                                      Parts *parts,
                                      std::vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
      static_cast<DependencyInstanceNumeric*>(instance);
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  dependency_parts->Initialize();
  bool make_gold = (gold_outputs != NULL);
  if (make_gold) gold_outputs->clear();

  // Make labeled arc parts and compute indices.
  MakeArcParts(instance, parts, gold_outputs);
  dependency_parts->BuildArcIndices(sentence->GetHeads());
  dependency_parts->ComputeSiblings(sentence->GetHeads());

  // Make sibling parts.
  if (GetDependencyLabelerOptions()->use_sibling_parts()) {
    MakeSiblingParts(instance, parts, gold_outputs);
    dependency_parts->BuildSiblingIndices(sentence->GetHeads());
  }

  dependency_parts->BuildOffsets();
}

void DependencyLabelerPipe::MakeArcParts(Instance *instance,
                                         Parts *parts,
                                         std::vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyDictionary *dependency_dictionary = GetDependencyDictionary();
  DependencyLabelerOptions *dependency_options = GetDependencyLabelerOptions();
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  bool prune_labels = dependency_options->prune_labels();
  vector<int> allowed_labels;

  if (!prune_labels) {
    allowed_labels.resize(dependency_dictionary->GetLabelAlphabet().size());
    for (int i = 0; i < allowed_labels.size(); ++i) {
      allowed_labels[i] = i;
    }
  }

  const vector<int> &heads = sentence->GetHeads();
  int num_parts_initial = dependency_parts->size();

  // Add parts for the labeled arcs.
  num_parts_initial = dependency_parts->size();
  for (int m = 1; m < sentence_length; ++m) {
    int h = heads[m];
    if (prune_labels) {
      int modifier_pos_id = sentence->GetPosId(m);
      int head_pos_id = sentence->GetPosId(h);
      allowed_labels.clear();
      allowed_labels = dependency_dictionary->
        GetExistingLabels(modifier_pos_id, head_pos_id);
    }

    // If there is no allowed label for this arc, but the unlabeled arc was
    // added, consider all the possible labels.
    if (allowed_labels.empty()) {
      allowed_labels.resize(dependency_dictionary->GetLabelAlphabet().size());
      for (int l = 0; l < allowed_labels.size(); ++l) {
        allowed_labels[l] = l;
      }
    }
    for (int k = 0; k < allowed_labels.size(); ++k) {
      int l = allowed_labels[k];
      Part *part = dependency_parts->CreatePartArc(h, m, l);
      dependency_parts->push_back(part);
      if (make_gold) {
        if (sentence->GetRelationId(m) == l) {
          gold_outputs->push_back(1.0);
        } else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }

  dependency_parts->SetOffsetArc(num_parts_initial,
                                 dependency_parts->size() - num_parts_initial);
}

void DependencyLabelerPipe::MakeSiblingParts(
    Instance *instance,
    Parts *parts,
    std::vector<double> *gold_outputs) {
  //LOG(INFO) << "MakeSiblingParts";
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyDictionary *dependency_dictionary = GetDependencyDictionary();
  DependencyLabelerOptions *dependency_options = GetDependencyLabelerOptions();
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  bool prune_labels = dependency_options->prune_labels();
  vector<int> allowed_labels;
  const vector<int> &heads = sentence->GetHeads();

  int num_parts_initial = dependency_parts->size();

  const std::vector<std::vector<int> > &siblings = dependency_parts->siblings();

  for (int h = 0; h < sentence_length; ++h) {
    // Don't create parts for heads without modifiers.
    if (siblings[h].size() == 0) continue;

    // Start position.
    int m = siblings[h][0];
    const std::vector<int>& initial_parts = dependency_parts->FindArcs(m);
    for (int j = 0; j < initial_parts.size(); ++j) {
      DependencyLabelerPartArc *initial_part =
        static_cast<DependencyLabelerPartArc*>(
          (*dependency_parts)[initial_parts[j]]);
      // TODO: Don't create a bigram part if this bigram is not allowed.
      Part *part = dependency_parts->
        CreatePartSibling(h, m, -1, initial_part->label(), -1);
      dependency_parts->push_back(part);
      if (make_gold) {
        gold_outputs->push_back((*gold_outputs)[initial_parts[j]]);
      }
    }

    // Intermediate position.
    for (int i = 1; i < siblings[h].size(); ++i) {
      int m = siblings[h][i];
      int s = siblings[h][i-1];
      const std::vector<int>& current_parts =
        dependency_parts->FindArcs(m);
      const std::vector<int>& previous_parts =
        dependency_parts->FindArcs(s);
      for (int j = 0; j < current_parts.size(); ++j) {
        DependencyLabelerPartArc *current_part =
          static_cast<DependencyLabelerPartArc*>(
            (*dependency_parts)[current_parts[j]]);
        for (int k = 0; k < previous_parts.size(); ++k) {
          DependencyLabelerPartArc *previous_part =
            static_cast<DependencyLabelerPartArc*>(
              (*dependency_parts)[previous_parts[k]]);
          // TODO: Don't create a bigram part if this bigram is not allowed.
          Part *part = dependency_parts->
            CreatePartSibling(h, m, s, current_part->label(),
                              previous_part->label());
          dependency_parts->push_back(part);
          if (make_gold) {
            gold_outputs->push_back(
              (*gold_outputs)[current_parts[j]] *
                (*gold_outputs)[previous_parts[k]]);
          }
        }
      }
    }

    // Final position.
    m = siblings[h][siblings[h].size()-1];
    const std::vector<int>& final_parts =
      dependency_parts->FindArcs(m);
    for (int j = 0; j < final_parts.size(); ++j) {
      DependencyLabelerPartArc *final_part =
        static_cast<DependencyLabelerPartArc*>(
          (*dependency_parts)[final_parts[j]]);
      // TODO: Don't create a bigram part if this bigram is not allowed.
      Part *part = dependency_parts->
        CreatePartSibling(h, -1, m, -1, final_part->label());
      dependency_parts->push_back(part);
      if (make_gold) {
        gold_outputs->push_back((*gold_outputs)[final_parts[j]]);
      }
    }
  }

  dependency_parts->SetOffsetSibling(
      num_parts_initial,
      dependency_parts->size() - num_parts_initial);
  //LOG(INFO) << "End MakeSiblingParts";
}

void DependencyLabelerPipe::MakeSelectedFeatures(
    Instance *instance,
    Parts *parts,
    const std::vector<bool>& selected_parts,
    Features *features) {
  //LOG(INFO) << "MakeSelectedFeatures";
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyLabelerFeatures *dependency_features =
    static_cast<DependencyLabelerFeatures*>(features);
  int sentence_length = sentence->size();

  // TODO(atm): make this computation of descendents be part of
  // DependencyInstanceNumeric or a class that derives from it.
  std::vector<std::vector<int> > descendents;
  const std::vector<int> &heads = sentence->GetHeads();
  ComputeDescendents(heads, &descendents);

  const std::vector<std::vector<int> > &siblings = dependency_parts->siblings();
  dependency_features->Initialize(instance, parts, siblings);

  // Build features for arcs/siblings only. They will later be conjoined with
  // the tags.
  for (int m = 1; m < sentence_length; ++m) {
    dependency_features->AddArcFeatures(sentence, descendents,
                                        siblings, m);
  }

  // Make sibling parts.
  if (GetDependencyLabelerOptions()->use_sibling_parts()) {
    for (int h = 0; h < sentence_length; ++h) {
      if (siblings[h].size() == 0) continue;
      for (int i = 0; i < siblings[h].size() + 1; ++i) {
        dependency_features->AddSiblingFeatures(sentence, descendents,
                                                siblings, h, i);
      }
    }
  }
  //LOG(INFO) << "End MakeSelectedFeatures";
}

void DependencyLabelerPipe::LabelInstance(Parts *parts,
                                          const std::vector<double> &output,
                                          Instance *instance) {
  DependencyLabelerParts *dependency_parts =
    static_cast<DependencyLabelerParts*>(parts);
  DependencyInstance *dependency_instance =
      static_cast<DependencyInstance*>(instance);
  int instance_length = dependency_instance->size();
  for (int m = 0; m < instance_length; ++m) {
    dependency_instance->SetHead(m, -1);
    dependency_instance->SetDependencyRelation(m, "NULL");
  }
  double threshold = 0.5;

  int offset, num_labeled_arcs;
  dependency_parts->GetOffsetArc(&offset, &num_labeled_arcs);
  for (int r = 0; r < num_labeled_arcs; ++r) {
    DependencyLabelerPartArc *arc =
      static_cast<DependencyLabelerPartArc*>((*dependency_parts)[offset + r]);
    if (output[offset + r] >= threshold) {
      dependency_instance->SetHead(arc->modifier(), arc->head());
      dependency_instance->SetDependencyRelation(arc->modifier(),
                                                 GetDependencyDictionary()->
                                                   GetLabelName(arc->label()));
    }
  }

  for (int m = 1; m < instance_length; ++m) {
    if (dependency_instance->GetHead(m) < 0) {
      VLOG(2) << "Word without head.";
      dependency_instance->SetHead(m, 0);
      dependency_instance->SetDependencyRelation(m,
            GetDependencyDictionary()->GetLabelName(0));
    }
  }
}

void DependencyLabelerPipe::ComputeDescendents(
    const std::vector<int> &heads,
    std::vector<std::vector<int> >* descendents) const {
  //LOG(INFO) << "Computing descendents";
  descendents->resize(heads.size());
  for (int h = 0; h < descendents->size(); ++h) {
    (*descendents)[h].clear();
  }
  for (int m = 1; m < heads.size(); ++m) {
    (*descendents)[m].push_back(m);
    std::vector<int> ancestors;
    GetAllAncestors(heads, m, &ancestors);
    for (int k = 0; k < ancestors.size(); ++k) {
      int h = ancestors[k];
      CHECK_GE(h, 0);
      //LOG(INFO) << h << " " << descendents->size();
      (*descendents)[h].push_back(m);
    }
  }
  //LOG(INFO) << "End computing descendents";
}

void DependencyLabelerPipe::GetAllAncestors(const std::vector<int> &heads,
                                            int descend,
                                            std::vector<int>* ancestors) const {
  ancestors->clear();
  int h = heads[descend];
  while (h >= 0) {
    ancestors->push_back(h);
    h = heads[h];
  }
}
