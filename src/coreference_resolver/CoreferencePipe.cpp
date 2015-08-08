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
  Pipe::LoadModel(fs);
  //static_cast<SequenceDictionary*>(dictionary_)->
  //  SetTokenDictionary(token_dictionary_);
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
#if 0
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
#endif
}

void CoreferencePipe::MakeGradientStep(
    Parts *parts,
    Features *features,
    double eta,
    int iteration,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output) {
#if 0
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
#endif
}

void CoreferencePipe::MakeFeatureDifference(
    Parts *parts,
    Features *features,
    const std::vector<double> &gold_output,
    const std::vector<double> &predicted_output,
    FeatureVector *difference) {
#if 0
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
#endif
}

void CoreferencePipe::MakeParts(Instance *instance,
                                Parts *parts,
                                std::vector<double> *gold_outputs) {
#if 0
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
#endif
}

void CoreferencePipe::MakeSelectedFeatures(
    Instance *instance,
    Parts *parts,
    const std::vector<bool> &selected_parts,
    Features *features) {
#if 0
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
#endif
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

