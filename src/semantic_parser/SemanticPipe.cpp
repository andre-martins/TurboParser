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
  if (pruner) {
    parameters = pruner_parameters_;
  } else {
    parameters = parameters_;
  }
  scores->resize(parts->size());
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  for (int r = 0; r < parts->size(); ++r) {
    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if (pruner) CHECK((*parts)[r]->type() == SEMANTICPART_ARC ||
                      (*parts)[r]->type() == SEMANTICPART_PREDICATE);
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    const BinaryFeatures &part_features = features->GetPartFeatures(r);
    if ((*parts)[r]->type() == SEMANTICPART_ARC && !pruner &&
        GetSemanticOptions()->labeled()) {
      (*scores)[r] = 0.0;
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

void SemanticPipe::RemoveUnsupportedFeatures(Instance *instance, Parts *parts,
                                             bool pruner,
                                             const vector<bool> &selected_parts,
                                             Features *features) {
  Parameters *parameters;
  if (pruner) {
    parameters = pruner_parameters_;
  } else {
    parameters = parameters_;
  }

  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;
    if (pruner) CHECK((*parts)[r]->type() == SEMANTICPART_ARC ||
                      (*parts)[r]->type() == SEMANTICPART_PREDICATE);
    // Skip labeled arcs, as they use the features from unlabeled arcs.
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) continue;
    BinaryFeatures *part_features =
      static_cast<SemanticFeatures*>(features)->GetMutablePartFeatures(r);
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

void SemanticPipe::MakeGradientStep(Parts *parts,
                                    Features *features,
                                    double eta,
                                    int iteration,
                                    const vector<double> &gold_output,
                                    const vector<double> &predicted_output) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  Parameters *parameters = GetTrainingParameters();

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) {
      SemanticPartLabeledArc *labeled_arc =
                  static_cast<SemanticPartLabeledArc*>((*parts)[r]);
      int index_part = semantic_parts->FindArc(labeled_arc->predicate(),
                                               labeled_arc->argument(),
                                               labeled_arc->sense());
      CHECK_GE(index_part, 0);

      const BinaryFeatures &part_features =
          features->GetPartFeatures(index_part);

      parameters->MakeLabelGradientStep(part_features, eta, iteration,
                                        labeled_arc->role(),
                                        predicted_output[r] - gold_output[r]);
    } else if ((*parts)[r]->type() == SEMANTICPART_ARC && !train_pruner_ &&
                GetSemanticOptions()->labeled()) {
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

void SemanticPipe::TouchParameters(Parts *parts, Features *features,
                                   const vector<bool> &selected_parts) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);
  Parameters *parameters = GetTrainingParameters();

  for (int r = 0; r < parts->size(); ++r) {
    if (!selected_parts[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) {
      SemanticPartLabeledArc *labeled_arc =
                  static_cast<SemanticPartLabeledArc*>((*parts)[r]);
      int index_part = semantic_parts->FindArc(labeled_arc->predicate(),
                                               labeled_arc->argument(),
                                               labeled_arc->sense());
      CHECK_GE(index_part, 0);

      const BinaryFeatures &part_features =
          features->GetPartFeatures(index_part);

      parameters->MakeLabelGradientStep(part_features, 0.0, 0,
                                        labeled_arc->role(),
                                        0.0);
    } else if ((*parts)[r]->type() == SEMANTICPART_ARC && !train_pruner_ &&
                GetSemanticOptions()->labeled()) {
      // TODO: Allow to have standalone features for unlabeled arcs.
      continue;
    } else {
      const BinaryFeatures &part_features =
          features->GetPartFeatures(r);

      parameters->MakeGradientStep(part_features, 0.0, 0, 0.0);
    }
  }
}

void SemanticPipe::MakeFeatureDifference(Parts *parts,
                                         Features *features,
                                         const vector<double> &gold_output,
                                         const vector<double> &predicted_output,
                                         FeatureVector *difference) {
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    // Labeled arcs will be treated by looking at the unlabeled arcs and
    // conjoining with the label.
    if ((*parts)[r]->type() == SEMANTICPART_LABELEDARC) {
      SemanticPartLabeledArc *labeled_arc =
                  static_cast<SemanticPartLabeledArc*>((*parts)[r]);
      int index_part = semantic_parts->FindArc(labeled_arc->predicate(),
                                               labeled_arc->argument(),
                                               labeled_arc->sense());
      CHECK_GE(index_part, 0);
      const BinaryFeatures &part_features =
          features->GetPartFeatures(index_part);

      for (int j = 0; j < part_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(part_features[j],
                                                   labeled_arc->role(),
                                                   predicted_output[r] - gold_output[r]);
      }
    } else if ((*parts)[r]->type() == SEMANTICPART_ARC && !train_pruner_ &&
                GetSemanticOptions()->labeled()) {
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
        semantic_parts->push_back(part);
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
        if (add_labeled_parts) {
          // If no unlabeled arc is there, just skip it.
          // This happens if that arc was pruned out.
          if (0 > semantic_parts->FindArc(p, a, s)) {
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

        if (prune_labels) {
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
            CHECK((*predicates)[s]->HasRole(role));
            Part *part = semantic_parts->CreatePartLabeledArc(p, a, s, role);
            semantic_parts->push_back(part);
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
                if (use_predicate_senses) CHECK_LT(predicate_id, 0);
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
          semantic_parts->push_back(part);
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

void SemanticPipe::MakePartsGlobal(Instance *instance,
                                   Parts *parts,
                                   vector<double> *gold_outputs) {
  SemanticOptions *semantic_options = GetSemanticOptions();
  SemanticParts *semantic_parts = static_cast<SemanticParts*>(parts);

  int num_parts_initial = semantic_parts->size();
#if 0
  if (semantic_options->use_arbitrary_siblings()) {
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
    semantic_features->AddArcFeatures(sentence, r, arc->predicate(),
                                      arc->argument(),
                                      predicate_id);
#if 0
    if (pruner) {
      semantic_features->AddArcFeaturesLight(sentence, r, arc->head(),
                                             arc->modifier());
    } else {
      semantic_features->AddArcFeatures(sentence, r, arc->head(),
                                        arc->modifier());
    }
#endif
  }

#if 0
  // Build features for arbitrary siblings.
  semantic_parts->GetOffsetSibl(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartSibling *part =
      static_cast<SemanticPartSibling*>((*semantic_parts)[r]);
    CHECK_EQ(part->type(), SEMANTICPART_SIBLING);
    semantic_features->AddArbitrarySiblingFeatures(sentence, r,
      part->predicate(), part->first_argument(),
      part->second_argument());
  }

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
#endif

#if 0
  // Build features for grandparents.
  dependency_parts->GetOffsetGrandpar(&offset, &size);
  if (pruner) CHECK_EQ(size, 0);
  for (int r = offset; r < offset + size; ++r) {
    if (!selected_parts[r]) continue;
    SemanticPartGrandpar *part =
      static_cast<SemanticPartGrandpar*>((*dependency_parts)[r]);
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
// If gold_outputs is not NULL, that vector will also be pruned.
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

  double threshold = 0.5;
  int r0 = 0;
  for (int r = 0; r < parts->size(); ++r) {
    // Preserve gold parts (at training time).
    if (predicted_outputs[r] >= threshold ||
        (preserve_gold && (*gold_outputs)[r] >= threshold)) {
      (*parts)[r0] = (*parts)[r];
      if (gold_outputs) (*gold_outputs)[r0] = (*gold_outputs)[r];
      ++r0;
    } else {
      delete (*parts)[r];
    }
  }

  if (gold_outputs) gold_outputs->resize(r0);
  parts->resize(r0);
  semantic_parts->DeleteIndices();
  semantic_parts->SetOffsetArc(0, parts->size());

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
