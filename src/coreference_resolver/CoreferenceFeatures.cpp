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
#include "CoreferenceFeatures.h"
#include "CoreferencePart.h"
#include "CoreferenceFeatureTemplates.h"

void CoreferenceFeatures::AddArcFeatures(CoreferenceDocumentNumeric* document,
                                         int r,
                                         int parent_mention,
                                         int child_mention) {
  CoreferenceOptions *options = static_cast<class CoreferencePipe*>(pipe_)->
      GetCoreferenceOptions();

  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  const vector<Mention*> &mentions = document->GetMentions();
  Mention *parent = (parent_mention >= 0)? mentions[parent_mention] : NULL;
  Mention *child = mentions[child_mention];
  int sentence_distance = (parent)?
    child->sentence_index() - parent->sentence_index() : -1;
  int mention_distance = (parent)? child_mention - parent_mention : -1;

  int num_bits = 0;

  // Child "canonical pronoun" types.
  uint16_t CtID = 0x0;
  // Two bits for the type.
  num_bits = 0;
  CtID = child->type() << num_bits;
  num_bits += 2;
  // If pronoun, 12=4+4+4 bits for the pronoun person, number, and gender.
  if (child->pronoun()) {
    CtID |= child->pronoun()->person_flag() << num_bits;
    num_bits += 4;
    CtID |= child->pronoun()->number_flag() << num_bits;
    num_bits += 4;
    CtID |= child->pronoun()->gender_flag() << num_bits;
    num_bits += 4;
  }
  CHECK_LT(num_bits, 16);

  // Parent "canonical pronoun" types.
  uint16_t PtID = 0x0;
  // Two bits for the type.
  num_bits = 0;
  if (parent) {
    PtID = parent->type() << num_bits;
    num_bits += 2;
    // If pronoun, 12=4+4+4 bits for the pronoun person, number, and gender.
    if (parent->pronoun()) {
      PtID |= parent->pronoun()->person_flag() << num_bits;
      num_bits += 4;
      PtID |= parent->pronoun()->number_flag() << num_bits;
      num_bits += 4;
      PtID |= parent->pronoun()->gender_flag() << num_bits;
      num_bits += 4;
    }
    CHECK_LT(num_bits, 16);
  }

  uint8_t child_length_code, parent_length_code;
  uint16_t CWID, PWID; // Child and parent head words.
  uint16_t CfWID, PfWID; // Child and parent first words.
  uint16_t ClWID, PlWID; // Child and parent last words.
  uint16_t CpWID, PpWID; // Child and parent preceding words.
  uint16_t CnWID, PnWID; // Child and parent next words.
  uint8_t parent_gender_code;
  uint8_t parent_number_code;

  uint8_t mention_distance_code = 0xff;
  int mention_distance_threshold = 10;
  if (parent) {
    if (mention_distance > mention_distance_threshold) {
      mention_distance_code = mention_distance_threshold;
    } else {
      mention_distance_code = mention_distance;
    }
  }

  uint8_t sentence_distance_code = 0xff;
  int sentence_distance_threshold = 10;
  if (parent) {
    if (sentence_distance > sentence_distance_threshold) {
      sentence_distance_code = sentence_distance_threshold;
    } else {
      sentence_distance_code = sentence_distance;
    }
  }

  uint8_t exact_match_code =
    (parent && (parent->phrase_string_id() == child->phrase_string_id()))?
    0x1 : 0x0;
  uint8_t head_match_code =
    (parent && (parent->head_string_id() == child->head_string_id()))?
    0x1 : 0x0;

  uint64_t fkey;
  uint8_t flags = 0x0;

  flags |= CoreferenceFeatureTemplateParts::ARC; // At most 4 bits here.
  if (parent) {
    flags |= (0x1 << 4); // A bit indicating if the mention is non-anaphoric.
  }

  // Maximum is 255 feature templates.
  CHECK_LT(CoreferenceFeatureTemplateArc::COUNT, 256);

  // Mention distance.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::md, flags,
                               mention_distance_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::md_Ct, flags,
                                CtID, mention_distance_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::md_Ct_Pt, flags,
                                 CtID, PtID, mention_distance_code);
  AddFeature(fkey, features);

  // Sentence distance.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::sd, flags,
                               sentence_distance_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::sd_Ct, flags,
                                CtID, sentence_distance_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::sd_Ct_Pt, flags,
                                 CtID, PtID, sentence_distance_code);
  AddFeature(fkey, features);

  // Head match.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::hm, flags,
                               head_match_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::hm_Ct, flags,
                                CtID, head_match_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::hm_Ct_Pt, flags,
                                 CtID, PtID, head_match_code);
  AddFeature(fkey, features);

  // Exact match.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::em, flags,
                               exact_match_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::em_Ct, flags,
                                CtID, exact_match_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::em_Ct_Pt, flags,
                                 CtID, PtID, exact_match_code);
  AddFeature(fkey, features);
}
