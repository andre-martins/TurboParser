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

#include "SequencePipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <gettimeofday.h>
#else
#include <sys/time.h>
#endif

using namespace std;

void SequencePipe::SaveModel(FILE* fs) {
  token_dictionary_->Save(fs);
  Pipe::SaveModel(fs);
}

void SequencePipe::LoadModel(FILE* fs) {
  delete token_dictionary_;
  CreateTokenDictionary();
  token_dictionary_->Load(fs);
  Pipe::LoadModel(fs);
  static_cast<SequenceDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
}

void SequencePipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<SequenceDictionary*>(dictionary_)->
    SetTokenDictionary(token_dictionary_);
  token_dictionary_->InitializeFromReader(GetSequenceReader());
  static_cast<SequenceDictionary*>(dictionary_)->
    CreateTagDictionary(GetSequenceReader());
}

void SequencePipe::ComputeScores(Instance *instance, Parts *parts,
                                 Features *features,
                                 vector<double> *scores) {
  SequenceInstanceNumeric *sentence =
    static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  SequenceFeatures *sequence_features =
    static_cast<SequenceFeatures*>(features);
  SequenceDictionary *sequence_dictionary = GetSequenceDictionary();
  scores->resize(parts->size());

  // Compute scores for the unigram parts.
  for (int i = 0; i < sentence->size(); ++i) {
    // Conjoin unigram features with the tag.
    const BinaryFeatures &unigram_features =
      sequence_features->GetUnigramFeatures(i);

    const vector<int> &index_unigram_parts =
      sequence_parts->FindUnigramParts(i);
    vector<int> allowed_tags(index_unigram_parts.size());
    for (int k = 0; k < index_unigram_parts.size(); ++k) {
      SequencePartUnigram *unigram =
          static_cast<SequencePartUnigram*>((*parts)[index_unigram_parts[k]]);
      allowed_tags[k] = unigram->tag();
    }
    vector<double> tag_scores;
    parameters_->ComputeLabelScores(unigram_features, allowed_tags,
        &tag_scores);
    for (int k = 0; k < index_unigram_parts.size(); ++k) {
      (*scores)[index_unigram_parts[k]] = tag_scores[k];
    }
  }

  // Compute scores for the bigram parts.
  if (GetSequenceOptions()->markov_order() >= 1) {
    for (int i = 0; i < sentence->size() + 1; ++i) {
      // Conjoin bigram features with the pair of tags.
      const BinaryFeatures &bigram_features =
        sequence_features->GetBigramFeatures(i);

      const vector<int> &index_bigram_parts = sequence_parts->FindBigramParts(i);
      vector<int> bigram_tags(index_bigram_parts.size());
      for (int k = 0; k < index_bigram_parts.size(); ++k) {
        SequencePartBigram *bigram =
            static_cast<SequencePartBigram*>((*parts)[index_bigram_parts[k]]);
        bigram_tags[k] = sequence_dictionary->GetBigramLabel(bigram->tag_left(),
                                                             bigram->tag());
      }

      vector<double> tag_scores;
      parameters_->ComputeLabelScores(bigram_features, bigram_tags, &tag_scores);
      for (int k = 0; k < index_bigram_parts.size(); ++k) {
        (*scores)[index_bigram_parts[k]] = tag_scores[k];
      }
    }
  }

  // Compute scores for the trigram parts.
  if (GetSequenceOptions()->markov_order() >= 2) {
    for (int i = 1; i < sentence->size() + 1; ++i) {
      // Conjoin trigram features with the triple of tags.
      const BinaryFeatures &trigram_features =
        sequence_features->GetTrigramFeatures(i);

      const vector<int> &index_trigram_parts = sequence_parts->FindTrigramParts(i);
      vector<int> trigram_tags(index_trigram_parts.size());
      for (int k = 0; k < index_trigram_parts.size(); ++k) {
        SequencePartTrigram *trigram =
            static_cast<SequencePartTrigram*>((*parts)[index_trigram_parts[k]]);
        trigram_tags[k] = sequence_dictionary->GetTrigramLabel(
          trigram->tag_left_left(),
          trigram->tag_left(),
          trigram->tag());
      }

      vector<double> tag_scores;
      parameters_->ComputeLabelScores(trigram_features, trigram_tags, &tag_scores);
      for (int k = 0; k < index_trigram_parts.size(); ++k) {
        (*scores)[index_trigram_parts[k]] = tag_scores[k];
      }
    }
  }
}

void SequencePipe::MakeGradientStep(Parts *parts,
                                    Features *features,
                                    double eta,
                                    int iteration,
                                    const vector<double> &gold_output,
                                    const vector<double> &predicted_output) {

  SequenceFeatures *sequence_features =
      static_cast<SequenceFeatures*>(features);
  SequenceDictionary *sequence_dictionary = GetSequenceDictionary();

  for (int r = 0; r < parts->size(); ++r) {
    //LOG(INFO) << predicted_output[r] << " " << gold_output[r];
    if (predicted_output[r] == gold_output[r]) continue;

    if ((*parts)[r]->type() == SEQUENCEPART_UNIGRAM) {
      SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r]);
      const BinaryFeatures &unigram_features =
          sequence_features->GetUnigramFeatures(unigram->position());

      parameters_->MakeLabelGradientStep(unigram_features, eta, iteration,
                                         unigram->tag(),
                                         predicted_output[r] - gold_output[r]);
    } else if ((*parts)[r]->type() == SEQUENCEPART_BIGRAM) {
      SequencePartBigram *bigram =
                  static_cast<SequencePartBigram*>((*parts)[r]);
      const BinaryFeatures &bigram_features =
          sequence_features->GetBigramFeatures(bigram->position());
      int bigram_tag = sequence_dictionary->GetBigramLabel(bigram->tag_left(),
                                                           bigram->tag());

      parameters_->MakeLabelGradientStep(bigram_features, eta, iteration,
                                         bigram_tag,
                                         predicted_output[r] - gold_output[r]);
    } else if ((*parts)[r]->type() == SEQUENCEPART_TRIGRAM) {
      SequencePartTrigram *trigram =
                  static_cast<SequencePartTrigram*>((*parts)[r]);
      const BinaryFeatures &trigram_features =
          sequence_features->GetTrigramFeatures(trigram->position());
      int trigram_tag = 
        sequence_dictionary->GetTrigramLabel(trigram->tag_left_left(),
                                             trigram->tag_left(),
                                             trigram->tag());

      parameters_->MakeLabelGradientStep(trigram_features, eta, iteration,
                                         trigram_tag,
                                         predicted_output[r] - gold_output[r]);
    } else {
      CHECK(false);
    }
  }
}

void SequencePipe::MakeFeatureDifference(Parts *parts,
                                         Features *features,
                                         const vector<double> &gold_output,
                                         const vector<double> &predicted_output,
                                         FeatureVector *difference) {
  SequenceFeatures *sequence_features =
      static_cast<SequenceFeatures*>(features);
  SequenceDictionary *sequence_dictionary = GetSequenceDictionary();

  for (int r = 0; r < parts->size(); ++r) {
    if (predicted_output[r] == gold_output[r]) continue;

    if ((*parts)[r]->type() == SEQUENCEPART_UNIGRAM) {
      SequencePartUnigram *unigram =
                  static_cast<SequencePartUnigram*>((*parts)[r]);
      const BinaryFeatures &unigram_features =
          sequence_features->GetUnigramFeatures(unigram->position());
      for (int j = 0; j < unigram_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(unigram_features[j],
            unigram->tag(), predicted_output[r] - gold_output[r]);
      }
    } else if ((*parts)[r]->type() == SEQUENCEPART_BIGRAM) {
      SequencePartBigram *bigram =
                  static_cast<SequencePartBigram*>((*parts)[r]);
      const BinaryFeatures &bigram_features =
          sequence_features->GetBigramFeatures(bigram->position());
      int bigram_tag = sequence_dictionary->GetBigramLabel(bigram->tag_left(),
                                                           bigram->tag());
      for (int j = 0; j < bigram_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(bigram_features[j],
            bigram_tag, predicted_output[r] - gold_output[r]);
      }
    } else if ((*parts)[r]->type() == SEQUENCEPART_TRIGRAM) {
      SequencePartTrigram *trigram =
                  static_cast<SequencePartTrigram*>((*parts)[r]);
      const BinaryFeatures &trigram_features =
          sequence_features->GetTrigramFeatures(trigram->position());
      int trigram_tag =
        sequence_dictionary->GetTrigramLabel(trigram->tag_left_left(),
                                             trigram->tag_left(),
                                             trigram->tag());
      for (int j = 0; j < trigram_features.size(); ++j) {
        difference->mutable_labeled_weights()->Add(trigram_features[j],
            trigram_tag, predicted_output[r] - gold_output[r]);
      }
    } else {
      CHECK(false);
    }
  }
}

void SequencePipe::MakeParts(Instance *instance,
                             Parts *parts,
                             vector<double> *gold_outputs) {
  int sentence_length =
      static_cast<SequenceInstanceNumeric*>(instance)->size();
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  sequence_parts->Initialize();
  bool make_gold = (gold_outputs != NULL);
  if (make_gold) gold_outputs->clear();

  CHECK_GE(GetSequenceOptions()->markov_order(), 0);
  CHECK_LE(GetSequenceOptions()->markov_order(), 2);

  // Make unigram parts and compute indices.
  MakeUnigramParts(instance, parts, gold_outputs);
  sequence_parts->BuildUnigramIndices(sentence_length);

  // Make bigram parts.
  if (GetSequenceOptions()->markov_order() >= 1) {
    MakeBigramParts(instance, parts, gold_outputs);
    sequence_parts->BuildBigramIndices(sentence_length);
  }

  // Make trigram parts.
  if (GetSequenceOptions()->markov_order() >= 2) {
    MakeTrigramParts(instance, parts, gold_outputs);
    sequence_parts->BuildTrigramIndices(sentence_length);
  }

  sequence_parts->BuildOffsets();
}

void SequencePipe::MakeUnigramParts(Instance *instance,
                                    Parts *parts,
                                    vector<double> *gold_outputs) {
  SequenceInstanceNumeric *sentence =
    static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  SequenceDictionary *sequence_dictionary = GetSequenceDictionary();
  SequenceOptions *sequence_options = GetSequenceOptions();
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);
  vector<int> all_tags;
  vector<int> allowed_tags;

  all_tags.resize(sequence_dictionary->GetTagAlphabet().size());
  for (int i = 0; i < all_tags.size(); ++i) {
    all_tags[i] = i;
  }

  int num_parts_initial = sequence_parts->size();

  for (int i = 0; i < sentence_length; ++i) {
    GetAllowedTags(instance, i, &allowed_tags);
    if (allowed_tags.empty()) {
      allowed_tags = all_tags;
    }

    // Add parts.
    CHECK_GE(allowed_tags.size(), 0);
    for (int k = 0; k < allowed_tags.size(); ++k) {
      int tag = allowed_tags[k];
      Part *part = sequence_parts->CreatePartUnigram(i, tag);
      sequence_parts->push_back(part);
      if (make_gold) {
        if (sentence->GetTagId(i) == tag) {
          gold_outputs->push_back(1.0);
        } else {
          gold_outputs->push_back(0.0);
        }
      }
    }
  }
  sequence_parts->SetOffsetUnigram(num_parts_initial,
      sequence_parts->size() - num_parts_initial);
}

void SequencePipe::MakeBigramParts(Instance *instance,
                                   Parts *parts,
                                   vector<double> *gold_outputs) {
  SequenceInstanceNumeric *sentence =
    static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  int num_parts_initial = sequence_parts->size();

  // Start position.
  const vector<int> &initial_parts = sequence_parts->FindUnigramParts(0);
  for (int j = 0; j < initial_parts.size(); ++j) {
    SequencePartUnigram *initial_part = static_cast<SequencePartUnigram *>(
            (*sequence_parts)[initial_parts[j]]);
    Part *part = sequence_parts->CreatePartBigram(0, initial_part->tag(), -1);
    sequence_parts->push_back(part);
    if (make_gold) {
      gold_outputs->push_back((*gold_outputs)[initial_parts[j]]);
    }
  }

  // Intermediate position.
  for (int i = 1; i < sentence_length; ++i) {
    const vector<int> &current_parts = sequence_parts->FindUnigramParts(i);
    const vector<int> &previous_parts = sequence_parts->FindUnigramParts(i - 1);
    for (int j = 0; j < current_parts.size(); ++j) {
      SequencePartUnigram *current_part = static_cast<SequencePartUnigram *>(
              (*sequence_parts)[current_parts[j]]);
      for (int k = 0; k < previous_parts.size(); ++k) {
        SequencePartUnigram *previous_part = static_cast<SequencePartUnigram *>(
                (*sequence_parts)[previous_parts[k]]);
        Part *part = sequence_parts->CreatePartBigram(i,
                                                      current_part->tag(),
                                                      previous_part->tag());
        sequence_parts->push_back(part);
        if (make_gold) {
          gold_outputs->push_back(
              (*gold_outputs)[current_parts[j]] *
                (*gold_outputs)[previous_parts[k]]);
        }
      }
    }
  }

  // Final position.
  const vector<int> &final_parts =
      sequence_parts->FindUnigramParts(sentence_length - 1);
  for (int j = 0; j < final_parts.size(); ++j) {
    SequencePartUnigram *final_part = static_cast<SequencePartUnigram *>(
            (*sequence_parts)[final_parts[j]]);
    Part *part = sequence_parts->CreatePartBigram(sentence_length,
                                                  -1,
                                                  final_part->tag());
    sequence_parts->push_back(part);
    if (make_gold) {
      gold_outputs->push_back((*gold_outputs)[final_parts[j]]);
    }
  }

  sequence_parts->SetOffsetBigram(num_parts_initial,
      sequence_parts->size() - num_parts_initial);
}

void SequencePipe::MakeTrigramParts(Instance *instance,
                                    Parts *parts,
                                    vector<double> *gold_outputs) {
  SequenceInstanceNumeric *sentence =
    static_cast<SequenceInstanceNumeric*>(instance);
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int sentence_length = sentence->size();
  bool make_gold = (gold_outputs != NULL);

  int num_parts_initial = sequence_parts->size();

  if (sentence_length <= 1) return;

  // Start position.
  const vector<int> &initial_parts = sequence_parts->FindUnigramParts(0);
  const vector<int> &next_initial_parts = sequence_parts->FindUnigramParts(1);
  for (int j = 0; j < next_initial_parts.size(); ++j) {
    SequencePartUnigram *current_part = static_cast<SequencePartUnigram *>(
            (*sequence_parts)[next_initial_parts[j]]);
    for (int k = 0; k < initial_parts.size(); ++k) {
      SequencePartUnigram *previous_part = static_cast<SequencePartUnigram *>(
              (*sequence_parts)[initial_parts[k]]);
      Part *part = sequence_parts->CreatePartTrigram(1,
                                                     current_part->tag(),
                                                     previous_part->tag(), 
                                                     -1 /* start symbol */);
      sequence_parts->push_back(part);
      if (make_gold) {
          gold_outputs->push_back((*gold_outputs)[next_initial_parts[j]] *
                                  (*gold_outputs)[initial_parts[k]]);
      }
    }
  }

  // Intermediate position.
  for (int i = 2; i < sentence_length; ++i) {
    const vector<int> &current_parts = sequence_parts->FindUnigramParts(i);
    const vector<int> &previous_parts = sequence_parts->FindUnigramParts(i - 1);
    const vector<int> &before_previous_parts =
      sequence_parts->FindUnigramParts(i - 2);
    for (int j = 0; j < current_parts.size(); ++j) {
      SequencePartUnigram *current_part = static_cast<SequencePartUnigram *>(
              (*sequence_parts)[current_parts[j]]);
      for (int k = 0; k < previous_parts.size(); ++k) {
        SequencePartUnigram *previous_part = static_cast<SequencePartUnigram *>(
                (*sequence_parts)[previous_parts[k]]);
        for (int l = 0; l < before_previous_parts.size(); ++l) {
          SequencePartUnigram *before_previous_part =
              static_cast<SequencePartUnigram *>(
                  (*sequence_parts)[before_previous_parts[l]]);
          Part *part =
            sequence_parts->CreatePartTrigram(i,
                                             current_part->tag(),
                                             previous_part->tag(),
                                             before_previous_part->tag());
          sequence_parts->push_back(part);
          if (make_gold) {
            gold_outputs->push_back(
                (*gold_outputs)[current_parts[j]] *
                  (*gold_outputs)[previous_parts[k]] *
                    (*gold_outputs)[before_previous_parts[l]]);
          }
        }
      }
    }
  }

  // Final position.
  const vector<int> &final_parts =
      sequence_parts->FindUnigramParts(sentence_length - 1);
  const vector<int> &before_final_parts = 
      sequence_parts->FindUnigramParts(sentence_length - 2);
  for (int j = 0; j < final_parts.size(); ++j) {
    SequencePartUnigram *current_part = static_cast<SequencePartUnigram *>(
            (*sequence_parts)[final_parts[j]]);
    for (int k = 0; k < before_final_parts.size(); ++k) {
      SequencePartUnigram *previous_part = static_cast<SequencePartUnigram *>(
              (*sequence_parts)[before_final_parts[k]]);
      Part *part = sequence_parts->CreatePartTrigram(sentence_length,
                                                     -1, /* stop symbol */
                                                     current_part->tag(),
                                                     previous_part->tag());
      sequence_parts->push_back(part);
      if (make_gold) {
          gold_outputs->push_back((*gold_outputs)[final_parts[j]] *
                                  (*gold_outputs)[before_final_parts[k]]);
      }
    }
  }

  sequence_parts->SetOffsetTrigram(num_parts_initial,
      sequence_parts->size() - num_parts_initial);
}


void SequencePipe::MakeSelectedFeatures(Instance *instance,
                                        Parts *parts,
                                        const vector<bool> &selected_parts,
                                        Features *features) {
  SequenceInstanceNumeric *sentence = 
    static_cast<SequenceInstanceNumeric*>(instance);
  SequenceFeatures *sequence_features =
    static_cast<SequenceFeatures*>(features);

  int sentence_length = sentence->size();

  sequence_features->Initialize(instance, parts);

  // Build features for words only. They will later be conjoined with the tags.
  for (int i = 0; i < sentence_length; ++i) {
    sequence_features->AddUnigramFeatures(sentence, i);
  }

  if (GetSequenceOptions()->markov_order() >= 1) {
    for (int i = 0; i < sentence_length + 1; ++i) {
      sequence_features->AddBigramFeatures(sentence, i);
    }
  }

  if (GetSequenceOptions()->markov_order() >= 2) {
    for (int i = 1; i < sentence_length + 1; ++i) {
      sequence_features->AddTrigramFeatures(sentence, i);
    }
  }
}

void SequencePipe::LabelInstance(Parts *parts, const vector<double> &output,
                   Instance *instance) {
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
}

