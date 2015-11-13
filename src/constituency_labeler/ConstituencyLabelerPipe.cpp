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

#include "ConstituencyLabelerPipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <time.h> //<gettimeofday.h>
#else
#include <sys/time.h>
#endif

void ConstituencyLabelerPipe::SaveModel(FILE* fs) {
  token_dictionary_->Save(fs);
  Pipe::SaveModel(fs);
}

void ConstituencyLabelerPipe::LoadModel(FILE* fs) {
  delete token_dictionary_;
  CreateTokenDictionary();
  token_dictionary_->Load(fs);
  Pipe::LoadModel(fs);
  static_cast<ConstituencyLabelerDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
}

void ConstituencyLabelerPipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<ConstituencyLabelerDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->Initialize(GetConstituencyReader());
  static_cast<ConstituencyLabelerDictionary*>(dictionary_)->
    CreateConstituentDictionary(GetConstituencyReader());
  static_cast<ConstituencyLabelerDictionary*>(dictionary_)->
    CreateLabelDictionary(GetConstituencyReader());
}

void ConstituencyLabelerPipe::ComputeScores(Instance *instance, Parts *parts,
                                            Features *features,
                                            std::vector<double> *scores) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerOptions *labeler_options = GetConstituencyLabelerOptions();
  ConstituencyLabelerFeatures *labeler_features =
    static_cast<ConstituencyLabelerFeatures*>(features);
  ConstituencyLabelerDictionary *labeler_dictionary =
    GetConstituencyLabelerDictionary();
  int num_nodes = sentence->GetNumConstituents();

  scores->resize(parts->size());

  // Compute scores for the node parts.
  for (int i = 0; i<num_nodes; ++i) {
    // Conjoin node features with the label.
    const BinaryFeatures &node_features =
      labeler_features->GetNodeFeatures(i);

    const std::vector<int> &index_node_parts =
      labeler_parts->FindNodeParts(i);
    std::vector<int> allowed_labels(index_node_parts.size());
    for (int k = 0; k<index_node_parts.size(); ++k) {
      ConstituencyLabelerPartNode *node =
        static_cast<ConstituencyLabelerPartNode*>(
          (*parts)[index_node_parts[k]]);
      allowed_labels[k] = node->label();
    }
    std::vector<double> label_scores;
    parameters_->ComputeLabelScores(node_features, allowed_labels,
                                    &label_scores);
    for (int k = 0; k<index_node_parts.size(); ++k) {
      (*scores)[index_node_parts[k]] = label_scores[k];
    }
  }
}

void ConstituencyLabelerPipe::MakeGradientStep(
  Parts *parts,
  Features *features,
  double eta,
  int iteration,
  const std::vector<double> &gold_output,
  const std::vector<double> &predicted_output) {
  ConstituencyLabelerFeatures *labeler_features =
    static_cast<ConstituencyLabelerFeatures*>(features);

  for (int r = 0; r<parts->size(); ++r) {
    if (predicted_output[r]==gold_output[r]) continue;

    if ((*parts)[r]->type()==CONSTITUENCYLABELERPART_NODE) {
      ConstituencyLabelerPartNode *node =
        static_cast<ConstituencyLabelerPartNode*>((*parts)[r]);
      const BinaryFeatures &node_features =
        labeler_features->GetNodeFeatures(node->position());
      parameters_->MakeLabelGradientStep(node_features, eta, iteration,
                                         node->label(),
                                         predicted_output[r]-gold_output[r]);
    } else {
      CHECK(false);
    }
  }
}

void ConstituencyLabelerPipe::MakeFeatureDifference(
  Parts *parts,
  Features *features,
  const std::vector<double> &gold_output,
  const std::vector<double> &predicted_output,
  FeatureVector *difference) {
  ConstituencyLabelerFeatures *labeler_features =
    static_cast<ConstituencyLabelerFeatures*>(features);

  CHECK_EQ(predicted_output.size(), parts->size());
  CHECK_EQ(gold_output.size(), parts->size());

  for (int r = 0; r<parts->size(); ++r) {
    if (predicted_output[r]==gold_output[r]) continue;

    if ((*parts)[r]->type()==CONSTITUENCYLABELERPART_NODE) {
      ConstituencyLabelerPartNode *node =
        static_cast<ConstituencyLabelerPartNode*>((*parts)[r]);
      const BinaryFeatures &node_features =
        labeler_features->GetNodeFeatures(node->position());
      for (int j = 0; j<node_features.size(); ++j) {
        difference->mutable_labeled_weights()->
          Add(node_features[j],
              node->label(),
              predicted_output[r]-gold_output[r]);
      }
    } else {
      CHECK(false);
    }
  }
}

void ConstituencyLabelerPipe::MakeParts(Instance *instance,
                                        Parts *parts,
                                        std::vector<double> *gold_outputs) {
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  int num_nodes = sentence->GetNumConstituents();
  labeler_parts->Initialize();
  bool make_gold = (gold_outputs!=NULL);
  if (make_gold) gold_outputs->clear();

  // Make node parts and compute indices.
  MakeNodeParts(instance, parts, gold_outputs);
  labeler_parts->BuildNodeIndices(num_nodes);

  labeler_parts->BuildOffsets();
}

void ConstituencyLabelerPipe::MakeNodeParts(Instance *instance,
                                            Parts *parts,
                                            std::vector<double> *gold_outputs) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerDictionary *labeler_dictionary =
    GetConstituencyLabelerDictionary();
  ConstituencyLabelerOptions *labeler_options = GetConstituencyLabelerOptions();
  int sentence_length = sentence->size();
  int num_nodes = sentence->GetNumConstituents();
  bool make_gold = (gold_outputs!=NULL);
  std::vector<int> all_labels;
  std::vector<int> allowed_labels;

  all_labels.resize(labeler_dictionary->GetLabelAlphabet().size());
  for (int i = 0; i<all_labels.size(); ++i) {
    all_labels[i] = i;
  }

  int num_parts_initial = labeler_parts->size();

  for (int i = 0; i<num_nodes; ++i) {
    GetAllowedLabels(instance, i, &allowed_labels);
    if (allowed_labels.empty()) {
      allowed_labels = all_labels;
    }

    // Add parts.
    CHECK_GE(allowed_labels.size(), 0);
    for (int k = 0; k<allowed_labels.size(); ++k) {
      int label = allowed_labels[k];
      if (labeler_options->ignore_null_labels()&&
          label==labeler_dictionary->null_label()) {
        continue;
      }
      // TODO: force the NULL label in some cases.
      Part *part = labeler_parts->CreatePartNode(i, label);
      labeler_parts->push_back(part);
      if (make_gold) {
        if (sentence->GetConstituentLabelId(i)==label) {
          gold_outputs->push_back(1.0);
        } else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }
  labeler_parts->SetOffsetNode(num_parts_initial,
                               labeler_parts->size()-num_parts_initial);
}

void ConstituencyLabelerPipe::MakeSelectedFeatures(
  Instance *instance,
  Parts *parts,
  const std::vector<bool> &selected_parts,
  Features *features) {
  ConstituencyLabelerInstanceNumeric *sentence =
    static_cast<ConstituencyLabelerInstanceNumeric*>(instance);
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerOptions *labeler_options = GetConstituencyLabelerOptions();
  int num_nodes = sentence->GetNumConstituents();
  ConstituencyLabelerFeatures *labeler_features =
    static_cast<ConstituencyLabelerFeatures*>(features);

  labeler_features->Initialize(instance, parts);

  // Build features for nodes only. They will later be conjoined with the
  // labels.
  for (int i = 0; i<num_nodes; ++i) {
    labeler_features->AddNodeFeatures(sentence, i);
  }
}

void ConstituencyLabelerPipe::LabelInstance(Parts *parts,
                                            const std::vector<double> &output,
                                            Instance *instance) {
  ConstituencyLabelerParts *labeler_parts =
    static_cast<ConstituencyLabelerParts*>(parts);
  ConstituencyLabelerInstance *labeler_instance =
    static_cast<ConstituencyLabelerInstance*>(instance);
  int num_nodes = labeler_instance->GetNumConstituents();
  for (int i = 0; i<num_nodes; ++i) {
    labeler_instance->SetConstituentLabel(i, "");
  }
  double threshold = 0.5;
  int offset, size;
  labeler_parts->GetOffsetNode(&offset, &size);
  for (int r = 0; r<size; ++r) {
    ConstituencyLabelerPartNode *node =
      static_cast<ConstituencyLabelerPartNode*>((*labeler_parts)[offset+r]);
    if (output[offset+r]>=threshold) {
      int i = node->position();
      int label = node->label();
      labeler_instance->SetConstituentLabel(i,
                                            GetConstituencyLabelerDictionary()->GetLabelName(label));
    }
  }
}

