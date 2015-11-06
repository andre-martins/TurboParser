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

#include "DependencyPipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <queue>
#ifndef _WIN32
#include <sys/time.h>
#else
#include <time.h>
#endif
using namespace std;

// Define the current model version and the oldest back-compatible version.
// The format is AAAA.BBBB.CCCC, e.g., 2 0003 0000 means "2.3.0".
const uint64_t kParserModelVersion = 200030000;
const uint64_t kOldestCompatibleParserModelVersion = 200030000;
const uint64_t kParserModelCheck = 1234567890;

void DependencyPipe::SaveModel(FILE* fs) {
  bool success;
  success = WriteUINT64(fs, kParserModelCheck);
  CHECK(success);
  success = WriteUINT64(fs, kParserModelVersion);
  CHECK(success);
  token_dictionary_->Save(fs);
  Pipe::SaveModel(fs);
  pruner_parameters_->Save(fs);
}

void DependencyPipe::LoadModel(FILE* fs) {
  bool success;
  uint64_t model_check;
  uint64_t model_version;
  success = ReadUINT64(fs, &model_check);
  CHECK(success);
  CHECK_EQ(model_check, kParserModelCheck)
    << "The model file is too old and not supported anymore.";
  success = ReadUINT64(fs, &model_version);
  CHECK(success);
  CHECK_GE(model_version, kOldestCompatibleParserModelVersion)
    << "The model file is too old and not supported anymore.";
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<DependencyDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->Load(fs);
  Pipe::LoadModel(fs);
  pruner_parameters_->Load(fs);
}

void DependencyPipe::LoadPrunerModel(FILE* fs) {
  LOG(INFO) << "Loading pruner model...";
  // This will be ignored but must be passed to the pruner pipe constructor,
  // so that when loading the pruner model the actual options are not
  // overwritten.
  DependencyOptions pruner_options; // = *options_;
  DependencyPipe* pipe = new DependencyPipe(&pruner_options);
  //DependencyPipe* pipe = new DependencyPipe(options_);
  pipe->Initialize();
  pipe->LoadModel(fs);
  delete pruner_parameters_;
  pruner_parameters_ = pipe->parameters_;
  pipe->parameters_ = NULL;
  delete pipe;
  LOG(INFO) << "Done.";
}

void DependencyPipe::LoadPrunerModelByName(const string &model_name) {
  FILE *fs = fopen(model_name.c_str(), "rb");
  CHECK(fs) << "Could not open pruner model file for reading: " << model_name;
  LoadPrunerModel(fs);
  fclose(fs);
}

void DependencyPipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<DependencyDictionary*>(dictionary_)->SetTokenDictionary(token_dictionary_);
  static_cast<DependencyTokenDictionary*>(token_dictionary_)->InitializeFromDependencyReader(GetDependencyReader());
  static_cast<DependencyDictionary*>(dictionary_)->CreateLabelDictionary(GetDependencyReader());
}

void DependencyPipe::ComputeScores(Instance *instance, Parts *parts,
  Features *features,
  bool pruner,
  vector<double> *scores) {
  Parameters *parameters;
  if (pruner) {
    parameters = pruner_parameters_;
  }
  else {
    parameters = parameters_;
  }
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

void DependencyPipe::RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
  bool pruner,
  const vector<bool> &selected_parts,
  Features *features) {
  Parameters *parameters;
  if (pruner) {
    parameters = pruner_parameters_;
  }
  else {
    parameters = parameters_;
  }

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

void DependencyPipe::MakeGradientStep(Parts *parts,
  Features *features,
  double eta,
  int iteration,
  const vector<double> &gold_output,
  const vector<double> &predicted_output) {
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
    }
    else if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !train_pruner_ &&
      GetDependencyOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    }
    else {
      const BinaryFeatures &part_features =
        features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, eta, iteration,
        predicted_output[r] - gold_output[r]);
    }
  }
}

void DependencyPipe::TouchParameters(Parts *parts, Features *features,
  const vector<bool> &selected_parts) {
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
    }
    else if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !train_pruner_ &&
      GetDependencyOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    }
    else {
      const BinaryFeatures &part_features =
        features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, 0.0, 0, 0.0);
    }
  }
}

void DependencyPipe::MakeFeatureDifference(Parts *parts,
  Features *features,
  const vector<double> &gold_output,
  const vector<double> &predicted_output,
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
    }
    else if ((*parts)[r]->type() == DEPENDENCYPART_ARC && !train_pruner_ &&
      GetDependencyOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    }
    else {
      const BinaryFeatures &part_features = features->GetPartFeatures(r);

      for (int j = 0; j < part_features.size(); ++j) {
        difference->mutable_weights()->Add(part_features[j],
          predicted_output[r] - gold_output[r]);
      }
    }
  }
}

void DependencyPipe::MakeParts(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
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
  }
  else {
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

void DependencyPipe::MakePartsBasic(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
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
    }
    else {
      Prune(instance, parts, gold_outputs, false);
    }
    dependency_parts->BuildOffsets();
    dependency_parts->BuildIndices(sentence_length, false);
  }

  if (GetDependencyOptions()->labeled()) {
    MakePartsBasic(instance, true, parts, gold_outputs);
  }
}

// Make sure the graph formed by the unlabeled arc parts is connected,
// otherwise there is no feasible solution. 
// If necessary, root nodes are added and passed back through the last 
// argument.
void DependencyPipe::EnforceConnectedGraph(Instance *instance,
  const vector<Part*> &arcs,
  vector<int> *inserted_root_nodes) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  inserted_root_nodes->clear();

  // Create a list of children for each node.
  vector<vector<int> > children(sentence->size());
  for (int r = 0; r < arcs.size(); ++r) {
    CHECK_EQ(arcs[r]->type(), DEPENDENCYPART_ARC);
    DependencyPartArc *arc = static_cast<DependencyPartArc*>(arcs[r]);
    int h = arc->head();
    int m = arc->modifier();
    children[h].push_back(m);
  }

  // Check if the root is connected to every node.
  vector<bool> visited(sentence->size(), false);
  queue<int> nodes_to_explore;
  nodes_to_explore.push(0);
  while (!nodes_to_explore.empty()) {
    int h = nodes_to_explore.front();
    nodes_to_explore.pop();
    visited[h] = true;
    for (int k = 0; k < children[h].size(); ++k) {
      int m = children[h][k];
      if (visited[m]) continue;
      nodes_to_explore.push(m);
    }

    // If there are no more nodes to explore, check if all nodes
    // were visited and, if not, add a new edge from the node to
    // the first node that was not visited yet.
    if (nodes_to_explore.empty()) {
      for (int m = 1; m < sentence->size(); ++m) {
        if (!visited[m]) {
          LOG(INFO) << "Inserted root node 0 -> " << m << ".";
          inserted_root_nodes->push_back(m);
          nodes_to_explore.push(m);
          break;
        }
      }
    }
  }
}

// Make sure the graph formed by the unlabeled arc parts admits a projective
// tree, otherwise there is no feasible solution when --projective=true.
// If necessary, we add arcs of the form m-1 -> m to make sure the sentence
// has a projective parse.
void DependencyPipe::EnforceProjectiveGraph(Instance *instance,
  const vector<Part*> &arcs,
  vector<int> *inserted_heads,
  vector<int> *inserted_modifiers) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  inserted_heads->clear();
  inserted_modifiers->clear();

  // Create an index of existing arcs.
  vector<vector<int> > index(sentence->size(),
    vector<int>(sentence->size(), -1));
  for (int r = 0; r < arcs.size(); ++r) {
    CHECK_EQ(arcs[r]->type(), DEPENDENCYPART_ARC);
    DependencyPartArc *arc = static_cast<DependencyPartArc*>(arcs[r]);
    int h = arc->head();
    int m = arc->modifier();
    index[h][m] = r;
  }

  // Insert consecutive right arcs if necessary.
  for (int m = 1; m < sentence->size(); ++m) {
    int h = m - 1;
    if (index[h][m] < 0) {
      inserted_heads->push_back(h);
      inserted_modifiers->push_back(m);
    }
  }
}

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
      }
      else {
        if (h != 0 && prune_distances) {
          int modifier_pos_id = sentence->GetPosId(m);
          int head_pos_id = sentence->GetPosId(h);
          if (h < m) {
            // Right attachment.
            if (m - h > dependency_dictionary->GetMaximumRightDistance
              (modifier_pos_id, head_pos_id)) continue;
          }
          else {
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
            }
            else {
              gold_outputs->push_back(0.0);
            }
          }
        }
      }
      else {
        Part *part = dependency_parts->CreatePartArc(h, m);
        dependency_parts->push_back(part);
        if (make_gold) {
          if (sentence->GetHead(m) == h) {
            gold_outputs->push_back(1.0);
          }
          else {
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
          }
          else {
            gold_outputs->push_back(0.0);
          }
        }
      }
    }
    else {
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
          }
          else {
            gold_outputs->push_back(0.0);
          }
        }
      }
    }

    dependency_parts->SetOffsetArc(num_parts_initial,
      dependency_parts->size() - num_parts_initial);
  }
  else {
    dependency_parts->SetOffsetLabeledArc(num_parts_initial,
      dependency_parts->size() - num_parts_initial);
  }
}

void DependencyPipe::MakePartsArbitrarySiblings(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  // Siblings: (h,m) and (h,s).
  for (int h = 0; h < sentence_length; ++h) {
    for (int m = 0; m < sentence_length; ++m) {
      if (h == m) continue;
      int r1 = dependency_parts->FindArc(h, m);
      if (r1 < 0) continue;
      for (int s = m + 1; s < sentence_length; ++s) {
        if (h == s) continue;
        int r2 = dependency_parts->FindArc(h, s);
        if (r2 < 0) continue;
        Part *part = dependency_parts->CreatePartSibl(h, m, s);
        dependency_parts->push_back(part);
        if (make_gold) {
          // Logical AND of the two individual arcs.
          gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
        }
      }
    }
  }
}

void DependencyPipe::MakePartsConsecutiveSiblings(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  // Consecutive siblings: (h,m) and (h,s).
  for (int h = 0; h < sentence_length; ++h) {
    bool first_arc_active;
    bool second_arc_active = false;
    bool arc_between;

    // Right side.
    for (int m = h; m < sentence_length; ++m) {
      int r1 = -1;
      if (m != h) {
        r1 = dependency_parts->FindArc(h, m);
        if (r1 < 0) continue;
      }

      if (make_gold) {
        // Check if the first arc is active.
        if (m == h || NEARLY_EQ_TOL((*gold_outputs)[r1], 1.0, 1e-9)) {
          first_arc_active = true;
        }
        else {
          first_arc_active = false;
        }
        arc_between = false;
      }

      for (int s = m + 1; s <= sentence_length; ++s) {
        int r2 = -1;
        if (s < sentence_length) {
          r2 = dependency_parts->FindArc(h, s);
          if (r2 < 0) continue;
        }
        if (make_gold) {
          // Check if the second arc is active.
          if (s == sentence_length ||
            NEARLY_EQ_TOL((*gold_outputs)[r2], 1.0, 1e-9)) {
            second_arc_active = true;
          }
          else {
            second_arc_active = false;
          }
        }

        Part *part = dependency_parts->CreatePartNextSibl(h, m, s);
        dependency_parts->push_back(part);

        if (make_gold) {
          double value = 0.0;
          if (first_arc_active && second_arc_active && !arc_between) {
            value = 1.0;
            arc_between = true;
          }
          gold_outputs->push_back(value);
        }
      }
    }

    // Left side.
    for (int m = h; m >= 0; --m) {
      int r1 = -1;
      if (m != h) {
        r1 = dependency_parts->FindArc(h, m);
        if (r1 < 0) continue;
      }

      if (make_gold) {
        // Check if the first arc is active.
        if (m == h || NEARLY_EQ_TOL((*gold_outputs)[r1], 1.0, 1e-9)) {
          first_arc_active = true;
        }
        else {
          first_arc_active = false;
        }
        arc_between = false;
      }

      for (int s = m - 1; s >= -1; --s) {
        int r2 = -1;
        if (s > -1) {
          r2 = dependency_parts->FindArc(h, s);
          if (r2 < 0) continue;
        }
        if (make_gold) {
          // Check if the second arc is active.
          if (s == -1 ||
            NEARLY_EQ_TOL((*gold_outputs)[r2], 1.0, 1e-9)) {
            second_arc_active = true;
          }
          else {
            second_arc_active = false;
          }
        }

        Part *part = dependency_parts->CreatePartNextSibl(h, m, s);
        dependency_parts->push_back(part);

        if (make_gold) {
          double value = 0.0;
          if (first_arc_active && second_arc_active && !arc_between) {
            value = 1.0;
            arc_between = true;
          }
          gold_outputs->push_back(value);
        }
      }
    }
  }
}

void DependencyPipe::MakePartsGrandparents(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  // Grandparents: (g,h) and (h,m).
  for (int g = 0; g < sentence_length; ++g) {
    for (int h = 0; h < sentence_length; ++h) {
      if (g == h) continue;
      int r1 = dependency_parts->FindArc(g, h);
      if (r1 < 0) continue;
      for (int m = 0; m < sentence_length; ++m) {
        if (h == m) continue;
        int r2 = dependency_parts->FindArc(h, m);
        if (r2 < 0) continue;
        Part *part = dependency_parts->CreatePartGrandpar(g, h, m);
        CHECK_LE(m, sentence_length);
        dependency_parts->push_back(part);
        if (make_gold) {
          // Logical AND of the two individual arcs.
          gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
        }
      }
    }
  }
}

void DependencyPipe::MakePartsGrandSiblings(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  // Grandparents with consecutive siblings: (g,h,m) and (g,h,s).
  for (int g = 0; g < sentence_length; ++g) {
    for (int h = 0; h < sentence_length; ++h) {
      if (g == h) continue;
      int r = dependency_parts->FindArc(g, h);
      if (r < 0) continue;

      bool grandpar_arc_active = false;
      if (NEARLY_EQ_TOL((*gold_outputs)[r], 1.0, 1e-9)) {
        grandpar_arc_active = true;
      }

      bool first_arc_active;
      bool second_arc_active = false;
      bool arc_between;

      // Right side.
      for (int m = h; m < sentence_length; ++m) {
        int r1 = -1;
        if (m != h) {
          r1 = dependency_parts->FindArc(h, m);
          if (r1 < 0) continue;
        }

        if (make_gold) {
          // Check if the first arc is active.
          if (m == h || NEARLY_EQ_TOL((*gold_outputs)[r1], 1.0, 1e-9)) {
            first_arc_active = true;
          }
          else {
            first_arc_active = false;
          }
          arc_between = false;
        }

        for (int s = m + 1; s <= sentence_length; ++s) {
          int r2 = -1;
          if (s < sentence_length) {
            r2 = dependency_parts->FindArc(h, s);
            if (r2 < 0) continue;
          }
          if (make_gold) {
            // Check if the second arc is active.
            if (s == sentence_length ||
              NEARLY_EQ_TOL((*gold_outputs)[r2], 1.0, 1e-9)) {
              second_arc_active = true;
            }
            else {
              second_arc_active = false;
            }
          }

          Part *part = dependency_parts->CreatePartGrandSibl(g, h, m, s);
          dependency_parts->push_back(part);

          if (make_gold) {
            double value = 0.0;
            if (first_arc_active && second_arc_active && !arc_between) {
              if (grandpar_arc_active) value = 1.0;
              arc_between = true;
            }
            gold_outputs->push_back(value);
          }
        }
      }

      // Left side.
      for (int m = h; m >= 0; --m) {
        int r1 = -1;
        if (m != h) {
          r1 = dependency_parts->FindArc(h, m);
          if (r1 < 0) continue;
        }

        if (make_gold) {
          // Check if the first arc is active.
          if (m == h || NEARLY_EQ_TOL((*gold_outputs)[r1], 1.0, 1e-9)) {
            first_arc_active = true;
          }
          else {
            first_arc_active = false;
          }
          arc_between = false;
        }

        for (int s = m - 1; s >= -1; --s) {
          int r2 = -1;
          if (s > -1) {
            r2 = dependency_parts->FindArc(h, s);
            if (r2 < 0) continue;
          }
          if (make_gold) {
            // Check if the second arc is active.
            if (s == -1 ||
              NEARLY_EQ_TOL((*gold_outputs)[r2], 1.0, 1e-9)) {
              second_arc_active = true;
            }
            else {
              second_arc_active = false;
            }
          }

          Part *part = dependency_parts->CreatePartGrandSibl(g, h, m, s);
          dependency_parts->push_back(part);

          if (make_gold) {
            double value = 0.0;
            if (first_arc_active && second_arc_active && !arc_between) {
              if (grandpar_arc_active) value = 1.0;
              arc_between = true;
            }
            gold_outputs->push_back(value);
          }
        }
      }
    }
  }
}

void DependencyPipe::MakePartsTriSiblings(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

#if 0
  for (int h = 0; h < sentence_length; ++h) {
    cout << "h=" << h << ": ";
    for (int m = 1; m < sentence_length; ++m) {
      int r = dependency_parts->FindArc(h, m);
      if (r >= 0 && NEARLY_EQ_TOL((*gold_outputs)[r], 1.0, 1e-9)) {
        cout << m << " ";
      }
    }
    cout << endl;
  }
#endif

  // Three consecutive siblings: (h,m), (h,s), and (h,t).
  for (int h = 0; h < sentence_length; ++h) {
    bool first_arc_active;
    bool second_arc_active = false;
    bool third_arc_active = false;
    bool arc_between;

    // Right side.
    for (int m = h; m < sentence_length; ++m) {
      int r1 = -1;
      if (m != h) {
        r1 = dependency_parts->FindArc(h, m);
        if (r1 < 0) continue;
      }

      if (make_gold) {
        // Check if the first arc is active.
        if (m == h || NEARLY_EQ_TOL((*gold_outputs)[r1], 1.0, 1e-9)) {
          first_arc_active = true;
        }
        else {
          first_arc_active = false;
        }
        arc_between = false;
      }

      // Assume s cannot be the stop symbol.
      for (int s = m + 1; s < sentence_length; ++s) {
        int r2 = -1;
        if (s < sentence_length) {
          r2 = dependency_parts->FindArc(h, s);
          if (r2 < 0) continue;
        }
        if (make_gold) {
          // Check if the second arc is active.
          if (s == sentence_length ||
            NEARLY_EQ_TOL((*gold_outputs)[r2], 1.0, 1e-9)) {
            second_arc_active = true;
          }
          else {
            second_arc_active = false;
          }
        }

        // Assume t can be the stop symbol.
        for (int t = s + 1; t <= sentence_length; ++t) {
          int r3 = -1;
          if (t < sentence_length) {
            r3 = dependency_parts->FindArc(h, t);
            if (r3 < 0) continue;
          }
          if (make_gold) {
            // Check if the third arc is active.
            if (t == sentence_length ||
              NEARLY_EQ_TOL((*gold_outputs)[r3], 1.0, 1e-9)) {
              third_arc_active = true;
            }
            else {
              third_arc_active = false;
            }
          }

          Part *part = dependency_parts->CreatePartTriSibl(h, m, s, t);
          dependency_parts->push_back(part);

          if (make_gold) {
            double value = 0.0;
            if (first_arc_active && second_arc_active && third_arc_active &&
              !arc_between) {
              value = 1.0;
              arc_between = true;
            }
            gold_outputs->push_back(value);
#if 0
            if (value == 1.0) {
              cout << "Gold trisibling: " << h << " " << m << " " << s << " " << t << endl;
            }
#endif
          }
        }
      }
    }

    // Left side.
    for (int m = h; m >= 0; --m) {
      int r1 = -1;
      if (m != h) {
        r1 = dependency_parts->FindArc(h, m);
        if (r1 < 0) continue;
      }

      if (make_gold) {
        // Check if the first arc is active.
        if (m == h || NEARLY_EQ_TOL((*gold_outputs)[r1], 1.0, 1e-9)) {
          first_arc_active = true;
        }
        else {
          first_arc_active = false;
        }
        arc_between = false;
      }

      // Assume s cannot be the stop symbol.
      for (int s = m - 1; s > -1; --s) {
        int r2 = -1;
        if (s > -1) {
          r2 = dependency_parts->FindArc(h, s);
          if (r2 < 0) continue;
        }
        if (make_gold) {
          // Check if the second arc is active.
          if (s == -1 ||
            NEARLY_EQ_TOL((*gold_outputs)[r2], 1.0, 1e-9)) {
            second_arc_active = true;
          }
          else {
            second_arc_active = false;
          }
        }

        // Assume t can be the stop symbol.
        for (int t = s - 1; t >= -1; --t) {
          int r3 = -1;
          if (t > -1) {
            r3 = dependency_parts->FindArc(h, t);
            if (r3 < 0) continue;
          }
          if (make_gold) {
            // Check if the third arc is active.
            if (t == -1 ||
              NEARLY_EQ_TOL((*gold_outputs)[r3], 1.0, 1e-9)) {
              third_arc_active = true;
            }
            else {
              third_arc_active = false;
            }
          }

          Part *part = dependency_parts->CreatePartTriSibl(h, m, s, t);
          dependency_parts->push_back(part);

          if (make_gold) {
            double value = 0.0;
            if (first_arc_active && second_arc_active && third_arc_active &&
              !arc_between) {
              value = 1.0;
              arc_between = true;
            }
            gold_outputs->push_back(value);
#if 0
            if (value == 1.0) {
              cout << "Gold trisibling: " << h << " " << m << " " << s << " " << t << endl;
            }
#endif
          }
        }
      }
    }
  }
}

void DependencyPipe::MakePartsNonprojectiveArcs(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  // Check if each arc (h,m) is non-projective.
  for (int h = 0; h < sentence_length; ++h) {
    for (int m = 0; m < sentence_length; ++m) {
      if (h == m) continue;
      int r = dependency_parts->FindArc(h, m);
      if (r < 0) continue;
      bool nonprojective = false;
      if (make_gold) {
        // Check if arc is active.
        if (NEARLY_EQ_TOL((*gold_outputs)[r], 1.0, 1e-9)) {
          // Check if active arc is nonprojective.
          if (!IsProjectiveArc(sentence->GetHeads(), h, m)) {
            nonprojective = true;
          }
        }
      }

      Part *part = dependency_parts->CreatePartNonproj(h, m);
      dependency_parts->push_back(part);
      if (make_gold) {
        if (nonprojective) {
          gold_outputs->push_back(1.0);
        }
        else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }
}

void DependencyPipe::MakePartsDirectedPaths(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  for (int a = 1; a < sentence_length; ++a) {
    for (int d = 1; d < sentence_length; ++d) {
      if (a == d) continue;

      Part *part = dependency_parts->CreatePartPath(a, d);
      dependency_parts->push_back(part);

      if (make_gold) {
        // Check if there is a directed path from a to d.
        if (ExistsPath(sentence->GetHeads(), a, d)) {
          gold_outputs->push_back(1.0);
        }
        else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }
}

void DependencyPipe::MakePartsHeadBigrams(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
  DependencyInstanceNumeric *sentence =
    static_cast<DependencyInstanceNumeric*>(instance);
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  // Bigrams: (h',m-1) and (h,m).
  for (int h = 0; h < sentence_length; ++h) {
    for (int m = 1; m < sentence_length; ++m) {
      if (h == m) continue;
      int r1 = dependency_parts->FindArc(h, m);
      if (r1 < 0) continue;
      for (int h_prev = 0; h_prev < sentence_length; ++h_prev) {
        if (h_prev == m - 1) continue;
        int r2 = dependency_parts->FindArc(h_prev, m - 1);
        if (r2 < 0) continue;
        Part *part = dependency_parts->CreatePartHeadBigram(h, m, h_prev);
        dependency_parts->push_back(part);
        if (make_gold) {
          // Logical AND of the two individual arcs.
          gold_outputs->push_back((*gold_outputs)[r1] * (*gold_outputs)[r2]);
        }
      }
    }
  }
}

void DependencyPipe::MakePartsGlobal(Instance *instance,
  Parts *parts,
  vector<double> *gold_outputs) {
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

#if 0
void DependencyPipe::GetAllAncestors(const vector<int> &heads,
  int descend,
  vector<int>* ancestors) const {
  ancestors->clear();
  int h = heads[descend];
  while (h >= 0) {
    ancestors->push_back(h);
    h = heads[h];
  }
}
#endif

bool DependencyPipe::ExistsPath(const vector<int> &heads,
  int ancest,
  int descend) const {
  int h = heads[descend];
  while (h != ancest && h >= 0) {
    h = heads[h];
  }
  if (h != ancest) return false;  // No path from ancest to descend.
  return true;
}

bool DependencyPipe::IsProjectiveArc(const vector<int> &heads,
  int par,
  int ch) const {
  int i0 = par;
  int j0 = ch;
  if (i0 > j0) {
    i0 = ch;
    j0 = par;
  }

  for (int k = i0 + 1; k < j0; k++) {
    // if (i,j) is projective, then all k must descend from i
    if (!ExistsPath(heads, par, k)) return false;
  }
  return true;
}

void DependencyPipe::MakeSelectedFeatures(Instance *instance,
  Parts *parts,
  bool pruner,
  const vector<bool>& selected_parts,
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
    }
    else {
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
    /*
    LOG(INFO) << "AddGrandSiblingFeatures: " << part->grandparent() << " "
              << part->head() << " "
              << part->modifier() << " "
              << part->sibling();
    */
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
    /*
    LOG(INFO) << "AddTriSiblingFeatures: " << part->head() << " "
              << part->modifier() << " "
              << part->sibling() << " "
              << part->other_sibling();
    */
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

// Prune basic parts (arcs and labeled arcs) using a first-order model.
// The vectors of basic parts is given as input, and those elements that are
// to be pruned are deleted from the vector.
// If gold_outputs is not NULL, that vector will also be pruned.
void DependencyPipe::Prune(Instance *instance, Parts *parts,
  vector<double> *gold_outputs,
  bool preserve_gold) {
  DependencyParts *dependency_parts = static_cast<DependencyParts*>(parts);
  Features *features = CreateFeatures();
  vector<double> scores;
  vector<double> predicted_outputs;

  // Make sure gold parts are only preserved at training time.
  CHECK(!preserve_gold || options_->train());

  MakeFeatures(instance, parts, true, features);
  ComputeScores(instance, parts, features, true, &scores);
  GetDependencyDecoder()->DecodePruner(instance, parts, scores,
    &predicted_outputs);

  double threshold = 0.5;
  int r0 = 0;
  for (int r = 0; r < parts->size(); ++r) {
    // Preserve gold parts (at training time).
    if (predicted_outputs[r] >= threshold ||
      (preserve_gold && (*gold_outputs)[r] >= threshold)) {
      (*parts)[r0] = (*parts)[r];
      if (gold_outputs) (*gold_outputs)[r0] = (*gold_outputs)[r];
      ++r0;
    }
    else {
      delete (*parts)[r];
    }
  }

  if (gold_outputs) gold_outputs->resize(r0);
  parts->resize(r0);
  dependency_parts->DeleteIndices();
  dependency_parts->SetOffsetArc(0, parts->size());

  delete features;
}

void DependencyPipe::LabelInstance(Parts *parts, const vector<double> &output,
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
  }
  else {
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

