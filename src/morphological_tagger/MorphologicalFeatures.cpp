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

#include "MorphologicalPipe.h"
#include "MorphologicalFeatures.h"
#include "SequencePart.h"
#include "MorphologicalFeatureTemplates.h"

void MorphologicalFeatures::AddUnigramFeatures(SequenceInstanceNumeric *sentence,
                                       int position) {
  CHECK(!input_features_unigrams_[position]);

  BinaryFeatures *features = new BinaryFeatures;
  input_features_unigrams_[position] = features;

  int sentence_length = sentence->size();

  MorphologicalInstanceNumeric *morph_sentence = static_cast<MorphologicalInstanceNumeric*>(sentence);


  MorphologicalOptions *options = static_cast<class MorphologicalPipe*>(pipe_)->
    GetMorphologicalOptions();

  // Array of form IDs.
  const vector<int>* word_ids = &morph_sentence->GetFormIds();

  // Array of coarse POS IDs.
  const vector<int>* cpostag_ids = &morph_sentence->GetCPosTagIds();

  // Words.
  // Current word.
  uint16_t WID = (position < sentence_length) ? (*word_ids)[position] : TOKEN_STOP; // Current word.
  // Word on the left.
  uint16_t pWID = (position > 0) ? (*word_ids)[position - 1] : TOKEN_START;
  // Word on the right.
  uint16_t nWID = (position < sentence_length - 1) ? (*word_ids)[position + 1] : TOKEN_STOP;
  // Word two positions on the left.
  uint16_t ppWID = (position > 1) ? (*word_ids)[position - 2] : TOKEN_START;
  // Word two positions on the right.
  uint16_t nnWID = (position < sentence_length - 2) ? (*word_ids)[position + 2] : TOKEN_STOP;

  // POS tags.
  uint8_t PID = (*cpostag_ids)[position]; // Current POS.
  // POS on the left.
  uint8_t pPID = (position > 0) ? (*cpostag_ids)[position - 1] : TOKEN_START;
  // POS on the right.
  uint8_t nPID = (position < sentence_length - 1) ? (*cpostag_ids)[position + 1] : TOKEN_STOP;
  // POS two positions on the left.
  uint8_t ppPID = (position > 1) ? (*cpostag_ids)[position - 2] : TOKEN_START;
  // POS two positions on the right.
  uint8_t nnPID = (position < sentence_length - 2) ? (*cpostag_ids)[position + 2] : TOKEN_STOP;

  // Word shapes.
  uint16_t SID = sentence->GetShapeId(position); // Current shape.
  // Shape on the left.
  uint16_t pSID = (position > 0) ? sentence->GetShapeId(position - 1) : TOKEN_START;
  // Shape on the right.
  uint16_t nSID = (position < sentence_length - 1) ? sentence->GetShapeId(position + 1) : TOKEN_STOP;
  // Shape two positions on the left.
  uint16_t ppSID = (position > 1) ? sentence->GetShapeId(position - 2) : TOKEN_START;
  // Shape two positions on the right.
  uint16_t nnSID = (position < sentence_length - 2) ? sentence->GetShapeId(position + 2) : TOKEN_STOP;

  // Prefixes/Suffixes.
  vector<uint16_t> AID((position < sentence_length) ? sentence->GetMaxPrefixLength(position) : 0, 0xffff);
  vector<uint16_t> ZID((position < sentence_length) ? sentence->GetMaxSuffixLength(position) : 0, 0xffff);
  // Prefixes/Suffixes on the left.
  vector<uint16_t> pAID((position > 0) ? sentence->GetMaxPrefixLength(position - 1) : 0, 0xffff);
  vector<uint16_t> pZID((position > 0) ? sentence->GetMaxSuffixLength(position - 1) : 0, 0xffff);
  // Prefixes/Suffixes on the right.
  vector<uint16_t> nAID((position < sentence_length - 1) ? sentence->GetMaxPrefixLength(position + 1) : 0, 0xffff);
  vector<uint16_t> nZID((position < sentence_length - 1) ? sentence->GetMaxSuffixLength(position + 1) : 0, 0xffff);
  // Prefixes/Suffixes two positions on the left.
  vector<uint16_t> ppAID((position > 1) ? sentence->GetMaxPrefixLength(position - 2) : 0, 0xffff);
  vector<uint16_t> ppZID((position > 1) ? sentence->GetMaxSuffixLength(position - 2) : 0, 0xffff);
  // Prefixes/Suffixes two positions on the right.
  vector<uint16_t> nnAID((position < sentence_length - 2) ? sentence->GetMaxPrefixLength(position + 2) : 0, 0xffff);
  vector<uint16_t> nnZID((position < sentence_length - 2) ? sentence->GetMaxSuffixLength(position + 2) : 0, 0xffff);

  for (int l = 0; l < AID.size(); ++l) { AID[l] = sentence->GetPrefixId(position, l + 1); }
  for (int l = 0; l < pAID.size(); ++l) { pAID[l] = sentence->GetPrefixId(position - 1, l + 1); }
  for (int l = 0; l < nAID.size(); ++l) { nAID[l] = sentence->GetPrefixId(position + 1, l + 1); }
  for (int l = 0; l < ppAID.size(); ++l) { ppAID[l] = sentence->GetPrefixId(position - 2, l + 1); }
  for (int l = 0; l < nnAID.size(); ++l) { nnAID[l] = sentence->GetPrefixId(position + 2, l + 1); }

  for (int l = 0; l < ZID.size(); ++l) { ZID[l] = sentence->GetSuffixId(position, l + 1); }
  for (int l = 0; l < pZID.size(); ++l) { pZID[l] = sentence->GetSuffixId(position - 1, l + 1); }
  for (int l = 0; l < nZID.size(); ++l) { nZID[l] = sentence->GetSuffixId(position + 1, l + 1); }
  for (int l = 0; l < ppZID.size(); ++l) { ppZID[l] = sentence->GetSuffixId(position - 2, l + 1); }
  for (int l = 0; l < nnZID.size(); ++l) { nnZID[l] = sentence->GetSuffixId(position + 2, l + 1); }


  // Several flags.
  uint8_t flag_digit = sentence->HasDigit(position) ? 0x1 : 0x0;
  uint8_t flag_upper = position > 0 && sentence->HasUpper(position) ? 0x1 : 0x0;
  uint8_t flag_hyphen = sentence->HasHyphen(position) ? 0x1 : 0x0;

  flag_digit = 0x0 | (flag_digit << 4);
  flag_upper = 0x1 | (flag_upper << 4);
  flag_hyphen = 0x2 | (flag_hyphen << 4);

  uint64_t fkey;
  uint8_t flags = 0x0;

  flags |= MorphologicalFeatureTemplateParts::UNIGRAM;

  // Maximum is 255 feature templates.
  CHECK_LT(MorphologicalFeatureTemplateUnigram::COUNT, 256);

  // Bias feature.
  if (options->large_feature_set() >= 1) {
    fkey = encoder_.CreateFKey_NONE(MorphologicalFeatureTemplateUnigram::BIAS, flags);
    AddFeature(fkey, features);
  }

  // Lexical features.
  fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::W, flags, WID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::pW, flags, pWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::nW, flags, nWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::ppW, flags, ppWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::nnW, flags, nnWID);
  AddFeature(fkey, features);

  //Word + POS
  fkey = encoder_.CreateFKey_PP(MorphologicalFeatureTemplateUnigram::WP, flags, WID, PID);
  AddFeature(fkey, features);

  // POS features.
  fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::P, flags, PID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::pP, flags, pPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::nP, flags, nPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::ppP, flags, ppPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::nnP, flags, nnPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(MorphologicalFeatureTemplateUnigram::PpP, flags, PID, pPID);
  AddFeature(fkey, features);

  if (options->large_feature_set() >= 1) {
    fkey = encoder_.CreateFKey_PP(MorphologicalFeatureTemplateUnigram::PnP, flags, PID, nPID);
    AddFeature(fkey, features);
  }
  fkey = encoder_.CreateFKey_PPP(MorphologicalFeatureTemplateUnigram::PpPppP, flags, PID, pPID, ppPID);
  AddFeature(fkey, features);

  if (options->large_feature_set() >= 1) {
    fkey = encoder_.CreateFKey_PPP(MorphologicalFeatureTemplateUnigram::PnPnnP, flags, PID, nPID, nnPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(MorphologicalFeatureTemplateUnigram::PpPnP, flags, PID, pPID, nPID);
    AddFeature(fkey, features);
  }

  // Shape features.
  if (options->large_feature_set() >= 2) {
    fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::S, flags, SID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::pS, flags, pSID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::nS, flags, nSID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::ppS, flags, ppSID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(MorphologicalFeatureTemplateUnigram::nnS, flags, nnSID);
    AddFeature(fkey, features);
  }

  // Prefix features.
  for (int l = 0; l < AID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::A, flags, AID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < pAID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::pA, flags, pAID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < nAID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::nA, flags, nAID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < ppAID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::ppA, flags, ppAID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < nnAID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::nnA, flags, nnAID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }

  // Suffix features.
  for (int l = 0; l < ZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::Z, flags, ZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < pZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::pZ, flags, pZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < nZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::nZ, flags, nZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < ppZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::ppZ, flags, ppZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l < nnZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(MorphologicalFeatureTemplateUnigram::nnZ, flags, nnZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }


  // Several flags.
  if (options->large_feature_set() >= 1) {
    fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::FLAG, flags, flag_digit);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::FLAG, flags, flag_upper);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(MorphologicalFeatureTemplateUnigram::FLAG, flags, flag_hyphen);
    AddFeature(fkey, features);
  }
}

void MorphologicalFeatures::AddBigramFeatures(SequenceInstanceNumeric *sentence,
                                      int position) {
  CHECK(!input_features_bigrams_[position]) << position << " " << sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_bigrams_[position] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= MorphologicalFeatureTemplateParts::BIGRAM;

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(MorphologicalFeatureTemplateBigram::BIAS, flags);
  AddFeature(fkey, features);

}

void MorphologicalFeatures::AddTrigramFeatures(SequenceInstanceNumeric *sentence,
                                       int position) {
  CHECK(!input_features_trigrams_[position]) << position << " " << sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_trigrams_[position] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= MorphologicalFeatureTemplateParts::TRIGRAM;

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(MorphologicalFeatureTemplateTrigram::BIAS, flags);
  AddFeature(fkey, features);
}
