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

#include "TaggerPipe.h"
#include "TaggerFeatures.h"
#include "SequencePart.h"
#include "TaggerFeatureTemplates.h"

void TaggerFeatures::AddUnigramFeatures(SequenceInstanceNumeric *sentence,
                                        int position) {
  CHECK(!input_features_unigrams_[position]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_unigrams_[position] = features;

  int sentence_length = sentence->size();

  // Array of form IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Words.
  uint16_t WID = (*word_ids)[position]; // Current word.
  // Word on the left.
  uint16_t pWID = (position>0) ? (*word_ids)[position-1] : TOKEN_START;
  // Word on the right.
  uint16_t nWID = (position<sentence_length-1) ? (*word_ids)[position+1] : TOKEN_STOP;
  // Word two positions on the left.
  uint16_t ppWID = (position>1) ? (*word_ids)[position-2] : TOKEN_START;
  // Word two positions on the right.
  uint16_t nnWID = (position<sentence_length-2) ? (*word_ids)[position+2] : TOKEN_STOP;

  // Prefixes/Suffixes.
  vector<uint16_t> AID(sentence->GetMaxPrefixLength(position), 0xffff);
  vector<uint16_t> ZID(sentence->GetMaxSuffixLength(position), 0xffff);
  for (int l = 0; l<AID.size(); ++l) { AID[l] = sentence->GetPrefixId(position, l+1); }
  for (int l = 0; l<ZID.size(); ++l) { ZID[l] = sentence->GetSuffixId(position, l+1); }

  // Several flags.
  uint8_t flag_digit = sentence->HasDigit(position) ? 0x1 : 0x0;
  uint8_t flag_upper = position>0&&sentence->HasUpper(position) ? 0x1 : 0x0;
  uint8_t flag_hyphen = sentence->HasHyphen(position) ? 0x1 : 0x0;

  flag_digit = 0x0|(flag_digit<<4);
  flag_upper = 0x1|(flag_upper<<4);
  flag_hyphen = 0x2|(flag_hyphen<<4);

  uint64_t fkey;
  uint8_t flags = 0x0;

  flags |= TaggerFeatureTemplateParts::UNIGRAM;

  // Maximum is 255 feature templates.
  CHECK_LT(TaggerFeatureTemplateUnigram::COUNT, 256);

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(TaggerFeatureTemplateUnigram::BIAS, flags);
  AddFeature(fkey, features);

  // Lexical features.
  fkey = encoder_.CreateFKey_W(TaggerFeatureTemplateUnigram::W, flags, WID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(TaggerFeatureTemplateUnigram::pW, flags, pWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(TaggerFeatureTemplateUnigram::nW, flags, nWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(TaggerFeatureTemplateUnigram::ppW, flags, ppWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(TaggerFeatureTemplateUnigram::nnW, flags, nnWID);
  AddFeature(fkey, features);

  // Prefix/Suffix features.
  for (int l = 0; l<AID.size(); ++l) {
    uint8_t flag_prefix_length = l;
    fkey = encoder_.CreateFKey_WP(TaggerFeatureTemplateUnigram::A, flags, AID[l], flag_prefix_length);
    AddFeature(fkey, features);
  }
  for (int l = 0; l<ZID.size(); ++l) {
    uint8_t flag_suffix_length = l;
    fkey = encoder_.CreateFKey_WP(TaggerFeatureTemplateUnigram::Z, flags, ZID[l], flag_suffix_length);
    AddFeature(fkey, features);
  }

  // Several flags.
  fkey = encoder_.CreateFKey_P(TaggerFeatureTemplateUnigram::FLAG, flags, flag_digit);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(TaggerFeatureTemplateUnigram::FLAG, flags, flag_upper);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(TaggerFeatureTemplateUnigram::FLAG, flags, flag_hyphen);
  AddFeature(fkey, features);
}

void TaggerFeatures::AddBigramFeatures(SequenceInstanceNumeric *sentence,
                                       int position) {
  CHECK(!input_features_bigrams_[position])<<position<<" "<<sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_bigrams_[position] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= TaggerFeatureTemplateParts::BIGRAM;

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(TaggerFeatureTemplateBigram::BIAS, flags);
  AddFeature(fkey, features);
}

void TaggerFeatures::AddTrigramFeatures(SequenceInstanceNumeric *sentence,
                                        int position) {
  CHECK(!input_features_trigrams_[position])<<position<<" "<<sentence->size();
  BinaryFeatures *features = new BinaryFeatures;
  input_features_trigrams_[position] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= TaggerFeatureTemplateParts::TRIGRAM;

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(TaggerFeatureTemplateTrigram::BIAS, flags);
  AddFeature(fkey, features);
}
