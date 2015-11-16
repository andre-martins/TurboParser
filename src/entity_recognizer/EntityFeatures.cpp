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

#include "EntityPipe.h"
#include "EntityFeatures.h"
#include "SequencePart.h"
#include "EntityFeatureTemplates.h"

void EntityFeatures::AddUnigramFeatures(SequenceInstanceNumeric *sentence,
                                        int position) {
  CHECK(!input_features_unigrams_[position]);

  BinaryFeatures *features = new BinaryFeatures;
  input_features_unigrams_[position] = features;

  int sentence_length = sentence->size();

  EntityInstanceNumeric *entity_sentence =
    static_cast<EntityInstanceNumeric*>(sentence);

  // Array of form IDs.
  const vector<int>* word_ids = &entity_sentence->GetFormIds();

  // Array of POS IDs.
  const vector<int>* pos_ids = &entity_sentence->GetPosIds();

  // Words.
  uint16_t WID = (*word_ids)[position]; // Current word.
  // Word on the left.
  uint16_t pWID = (position > 0) ? (*word_ids)[position - 1] : TOKEN_START;
  // Word on the right.
  uint16_t nWID = (position < sentence_length - 1) ?
    (*word_ids)[position + 1] : TOKEN_STOP;
  // Word two positions on the left.
  uint16_t ppWID = (position > 1) ? (*word_ids)[position - 2] : TOKEN_START;
  // Word two positions on the right.
  uint16_t nnWID = (position < sentence_length - 2) ?
    (*word_ids)[position + 2] : TOKEN_STOP;

  // Gazetteer tags.
  std::vector<int> empty_GIDs;
  // Current gazetter tag.
  const std::vector<int> &GIDs = entity_sentence->GetGazetteerIds(position);
  // Gazetteer tag on the left.
  const std::vector<int> &pGIDs = (position > 0) ?
    entity_sentence->GetGazetteerIds(position - 1) : empty_GIDs;
  // Gazetteer tag on the right.
  const std::vector<int> &nGIDs = (position < sentence_length - 1) ?
    entity_sentence->GetGazetteerIds(position + 1) : empty_GIDs;
  // Gazetteer tag two positions on the left.
  const std::vector<int> &ppGIDs = (position > 1) ?
    entity_sentence->GetGazetteerIds(position - 2) : empty_GIDs;
  // Gazetteer tag two positions on the right.
  const std::vector<int> &nnGIDs = (position < sentence_length - 2) ?
    entity_sentence->GetGazetteerIds(position + 2) : empty_GIDs;

  // POS tags.
  uint8_t PID = (*pos_ids)[position]; // Current word.
  // POS on the left.
  uint8_t pPID = (position > 0) ? (*pos_ids)[position - 1] : TOKEN_START;
  // POS on the right.
  uint8_t nPID = (position < sentence_length - 1) ?
    (*pos_ids)[position + 1] : TOKEN_STOP;
  // POS two positions on the left.
  uint8_t ppPID = (position > 1) ? (*pos_ids)[position - 2] : TOKEN_START;
  // POS two positions on the right.
  uint8_t nnPID = (position < sentence_length - 2) ?
    (*pos_ids)[position + 2] : TOKEN_STOP;

  // Word shapes.
  uint16_t SID = sentence->GetShapeId(position); // Current shape.
  // Shape on the left.
  uint16_t pSID = (position > 0) ?
    sentence->GetShapeId(position - 1) : TOKEN_START;
  // Shape on the right.
  uint16_t nSID = (position < sentence_length - 1) ?
    sentence->GetShapeId(position + 1) : TOKEN_STOP;
  // Shape two positions on the left.
  uint16_t ppSID = (position > 1) ?
    sentence->GetShapeId(position - 2) : TOKEN_START;
  // Shape two positions on the right.
  uint16_t nnSID = (position < sentence_length - 2) ?
    sentence->GetShapeId(position + 2) : TOKEN_STOP;

  // Prefixes/Suffixes.
  vector<uint16_t> AID(sentence->GetMaxPrefixLength(position), 0xffff);
  vector<uint16_t> ZID(sentence->GetMaxSuffixLength(position), 0xffff);
  for (int l = 0; l < AID.size(); ++l) {
    AID[l] = sentence->GetPrefixId(position, l + 1);
  }
  for (int l = 0; l < ZID.size(); ++l) {
    ZID[l] = sentence->GetSuffixId(position, l + 1);
  }

  // Several flags.
  uint8_t flag_all_digits = sentence->AllDigits(position) ? 0x1 : 0x0;
  uint8_t flag_all_digits_with_punctuation =
    sentence->AllDigitsWithPunctuation(position) ? 0x1 : 0x0;
  uint8_t flag_all_upper = sentence->AllUpper(position) ? 0x1 : 0x0;
  uint8_t flag_first_upper = position > 0 && sentence->FirstUpper(position) ?
    0x1 : 0x0;

  flag_all_digits = 0x0 | (flag_all_digits << 4);
  flag_all_digits_with_punctuation =
    0x1 | (flag_all_digits_with_punctuation << 4);
  flag_all_upper = 0x2 | (flag_all_upper << 4);
  flag_first_upper = 0x3 | (flag_first_upper << 4);

  uint64_t fkey;
  uint8_t flags = 0x0;

  flags |= EntityFeatureTemplateParts::UNIGRAM;

  // Maximum is 255 feature templates.
  CHECK_LT(EntityFeatureTemplateUnigram::COUNT, 256);

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateUnigram::BIAS, flags);
  AddFeature(fkey, features);

  // Lexical features.
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::W, flags, WID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::pW, flags, pWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nW, flags, nWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::ppW, flags, ppWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nnW, flags, nnWID);
  AddFeature(fkey, features);

  // Gazetteer features.
  for (int k = 0; k < GIDs.size(); ++k) {
    uint16_t GID = GIDs[k];
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::G, flags, GID);
    AddFeature(fkey, features);
  }
  for (int k = 0; k < pGIDs.size(); ++k) {
    uint16_t pGID = pGIDs[k];
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::pG, flags, pGID);
    AddFeature(fkey, features);
  }
  for (int k = 0; k < nGIDs.size(); ++k) {
    uint16_t nGID = nGIDs[k];
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nG, flags, nGID);
    AddFeature(fkey, features);
  }
  for (int k = 0; k < ppGIDs.size(); ++k) {
    uint16_t ppGID = ppGIDs[k];
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::ppG, flags,
                                 ppGID);
    AddFeature(fkey, features);
  }
  for (int k = 0; k < nnGIDs.size(); ++k) {
    uint16_t nnGID = nnGIDs[k];
    fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nnG, flags,
                                 nnGID);
    AddFeature(fkey, features);
  }

  // POS features.
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::P,
                               flags, PID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(EntityFeatureTemplateUnigram::PpP,
                                flags, PID, pPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(EntityFeatureTemplateUnigram::PnP,
                                flags, PID, nPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(EntityFeatureTemplateUnigram::PpPppP,
                                 flags, PID, pPID, ppPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(EntityFeatureTemplateUnigram::PnPnnP,
                                 flags, PID, nPID, nnPID);
  AddFeature(fkey, features);

  // Shape features.
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::S,
                               flags, SID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::pS,
                               flags, pSID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nS,
                               flags, nSID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::ppS,
                               flags, ppSID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateUnigram::nnS,
                               flags, nnSID);
  AddFeature(fkey, features);

  // Prefix/Suffix features.
  for (int l = 0; l < AID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(EntityFeatureTemplateUnigram::A,
                                  flags, AID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < ZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(EntityFeatureTemplateUnigram::Z,
                                  flags, ZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }

  // Several flags.
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flags, flag_all_digits);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flags, flag_all_digits_with_punctuation);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flags, flag_all_upper);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateUnigram::FLAG,
                               flags, flag_first_upper);
  AddFeature(fkey, features);
}

void EntityFeatures::AddBigramFeatures(SequenceInstanceNumeric *sentence,
                                       int position) {
  CHECK(!input_features_bigrams_[position]) << position
    << " " << sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_bigrams_[position] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= EntityFeatureTemplateParts::BIGRAM;

  // Note: position ranges between 0 and N (inclusive), where N is the number
  // of words. If position = N, we need to be careful not to access invalid
  // memory in arrays.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateBigram::BIAS, flags);
  AddFeature(fkey, features);

  // Add other bigram features.
  int sentence_length = sentence->size();

  EntityInstanceNumeric *entity_sentence =
    static_cast<EntityInstanceNumeric*>(sentence);

  // Array of form IDs.
  const vector<int>* word_ids = &entity_sentence->GetFormIds();

  // Array of POS IDs.
  const vector<int>* pos_ids = &entity_sentence->GetPosIds();

  // Words.
  uint16_t WID = (position < sentence_length) ?
    (*word_ids)[position] : TOKEN_STOP; // Current word.
  // Word on the left.
  uint16_t pWID = (position > 0) ?
    (*word_ids)[position - 1] : TOKEN_START;
  // Word on the right.
  uint16_t nWID = (position < sentence_length - 1) ?
    (*word_ids)[position + 1] : TOKEN_STOP;
  // Word two positions on the left.
  uint16_t ppWID = (position > 1) ?
    (*word_ids)[position - 2] : TOKEN_START;
  // Word two positions on the right.
  uint16_t nnWID = (position < sentence_length - 2) ?
    (*word_ids)[position + 2] : TOKEN_STOP;

  // POS tags.
  uint8_t PID = (position < sentence_length) ?
    (*pos_ids)[position] : TOKEN_STOP; // Current POS.
  // POS on the left.
  uint8_t pPID = (position > 0) ?
    (*pos_ids)[position - 1] : TOKEN_START;
  // POS on the right.
  uint8_t nPID = (position < sentence_length - 1) ?
    (*pos_ids)[position + 1] : TOKEN_STOP;
  // POS two positions on the left.
  uint8_t ppPID = (position > 1) ?
    (*pos_ids)[position - 2] : TOKEN_START;
  // POS two positions on the right.
  uint8_t nnPID = (position < sentence_length - 2) ?
    (*pos_ids)[position + 2] : TOKEN_STOP;

  // Maximum is 255 feature templates.
  CHECK_LT(EntityFeatureTemplateBigram::COUNT, 256);

  // Bias feature.
  //fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateBigram::BIAS, flags);
  //AddFeature(fkey, features);

  // Lexical features.
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::W,
                               flags, WID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::pW,
                               flags, pWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::nW,
                               flags, nWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::ppW,
                               flags, ppWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(EntityFeatureTemplateBigram::nnW,
                               flags, nnWID);
  AddFeature(fkey, features);

  // POS features.
  fkey = encoder_.CreateFKey_P(EntityFeatureTemplateBigram::P,
                               flags, PID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(EntityFeatureTemplateBigram::PpP,
                                flags, PID, pPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(EntityFeatureTemplateBigram::PnP,
                                flags, PID, nPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(EntityFeatureTemplateBigram::PpPppP,
                                 flags, PID, pPID, ppPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(EntityFeatureTemplateBigram::PnPnnP,
                                 flags, PID, nPID, nnPID);
  AddFeature(fkey, features);
}

void EntityFeatures::AddTrigramFeatures(SequenceInstanceNumeric *sentence,
                                        int position) {
  CHECK(!input_features_trigrams_[position]) << position
    << " " << sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_trigrams_[position] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= EntityFeatureTemplateParts::TRIGRAM;

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(EntityFeatureTemplateTrigram::BIAS, flags);
  AddFeature(fkey, features);
}
