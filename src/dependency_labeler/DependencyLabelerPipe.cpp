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

void DependencyLabelerPipe::ComputeScores(Instance *instance, Parts *parts,
                                          Features *features,
                                          std::vector<double> *scores) {
  Parameters *parameters = parameters_;
  scores->resize(parts->size());
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  for (int r = 0; r < parts->size(); ++r) {
    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) continue;
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    if ((*parts)[r]->type() == DEPENDENCYPART_ARC) {
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

void DependencyLabelerPipe::RemoveUnsupportedFeatures(
    Instance *instance, Parts *parts,
    const std::vector<bool> &selected_parts,
    Features *features) {
  Parameters *parameters = parameters_;

  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;
    // Skip labeled arcs, are they use the features from unlabeled arcs.
    if ((*parts)[r]->type() == DEPENDENCYPART_LABELEDARC) continue;
    BinaryFeatures *part_features =
      static_cast<DependencyLabelerFeatures*>(features)->
        GetMutablePartFeatures(r);
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
    } else if ((*parts)[r]->type() == DEPENDENCYPART_ARC) {
      continue;
    } else {
      const BinaryFeatures &part_features =
          features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, eta, iteration,
        predicted_output[r] - gold_output[r]);
    }
  }
}

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
    } else if ((*parts)[r]->type() == DEPENDENCYPART_ARC) {
      continue;
    } else {
      const BinaryFeatures &part_features =
          features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, 0.0, 0, 0.0);
    }
  }
}

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
        difference->mutable_labeled_weights()->
          Add(part_features[j], labeled_arc->label(),
              predicted_output[r] - gold_output[r]);
      }
    } else if ((*parts)[r]->type() == DEPENDENCYPART_ARC) {
      continue;
    } else {
      const BinaryFeatures &part_features = features->GetPartFeatures(r);

      for (int j = 0; j < part_features.size(); ++j) {
        difference->mutable_weights()->
          Add(part_features[j], predicted_output[r] - gold_output[r]);
      }
    }
  }
}

void DependencyLabelerPipe::MakeParts(Instance *instance,
                                      Parts *parts,
                                      std::vector<double> *gold_outputs) {
  int sentence_length =
      static_cast<DependencyInstanceNumeric*>(instance)->size();
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  dependency_parts->Initialize();
  bool make_gold = (gold_outputs != NULL);
  if (make_gold) gold_outputs->clear();

  // Make arc-factored parts and compute indices.
  MakePartsBasic(instance, parts, gold_outputs);
  dependency_parts->BuildOffsets();
  dependency_parts->BuildIndices(sentence_length, true);

  // Make global parts.
  MakePartsGlobal(instance, parts, gold_outputs);
  dependency_parts->BuildOffsets();
}

void DependencyLabelerPipe::MakePartsBasic(Instance *instance,
                                           Parts *parts,
                                           std::vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
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

  // Add parts for the gold unlabeled arcs.
  for (int m = 1; m < sentence_length; ++m) {
    int h = heads[m];
    Part *part = dependency_parts->CreatePartArc(h, m);
    dependency_parts->push_back(part);
    if (make_gold) {
      gold_outputs->push_back(1.0);
    }
  }

  dependency_parts->SetOffsetArc(num_parts_initial,
                                 dependency_parts->size() - num_parts_initial);
  dependency_parts->BuildOffsets();
  dependency_parts->BuildIndices(sentence_length, false);

  // Now, add parts for each possible label.
  num_parts_initial = dependency_parts->size();
  for (int m = 1; m < sentence_length; ++m) {
    int h = heads[m];
    CHECK_GE(dependency_parts->FindArc(h, m), 0);

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
  }

  dependency_parts->
    SetOffsetLabeledArc(num_parts_initial,
                        dependency_parts->size() - num_parts_initial);
}

void DependencyLabelerPipe::MakePartsGlobal(Instance *instance,
                                            Parts *parts,
                                            std::vector<double> *gold_outputs) {
  // TODO(atm): add global parts.
}

void DependencyLabelerPipe::MakeSelectedFeatures(
    Instance *instance,
    Parts *parts,
    const std::vector<bool>& selected_parts,
    Features *features) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyLabelerFeatures *dependency_features =
    static_cast<DependencyLabelerFeatures*>(features);
  int sentence_length = sentence->size();

  dependency_features->Initialize(instance, parts);

  // TODO(atm): make this computation of descendents be part of
  // DependencyInstanceNumeric or a class that derives from it.
  std::vector<std::vector<int> > descendents;
  ComputeDescendents(sentence->GetHeads(), &descendents);

  // Even in the case of labeled parsing, build features for unlabeled arcs
  // only. They will later be conjoined with the labels.
  int offset, size;
  dependency_parts->GetOffsetArc(&offset, &size);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    DependencyPartArc *arc =
      static_cast<DependencyPartArc*>((*dependency_parts)[r]);
    CHECK_GE(arc->head(), 0);
    dependency_features->AddArcFeatures(sentence, descendents, r, arc->head(),
                                        arc->modifier());
  }
}

void DependencyLabelerPipe::LabelInstance(Parts *parts,
                                          const std::vector<double> &output,
                                          Instance *instance) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  DependencyInstance *dependency_instance =
      static_cast<DependencyInstance*>(instance);
  int instance_length = dependency_instance->size();
  for (int m = 0; m < instance_length; ++m) {
    dependency_instance->SetHead(m, -1);
    dependency_instance->SetDependencyRelation(m, "NULL");
  }
  double threshold = 0.5;

  int offset, num_labeled_arcs;
  dependency_parts->GetOffsetLabeledArc(&offset, &num_labeled_arcs);
  for (int r = 0; r < num_labeled_arcs; ++r) {
    DependencyPartLabeledArc *arc =
      static_cast<DependencyPartLabeledArc*>((*dependency_parts)[offset + r]);
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
      dependency_instance->SetDependencyRelation(m, GetDependencyDictionary()->GetLabelName(0));
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
