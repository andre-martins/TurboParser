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

  bool use_gender_number_features = true;

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

  uint16_t CWID, PWID; // Child and parent head words.
  uint16_t CfWID, PfWID; // Child and parent first words.
  uint16_t ClWID, PlWID; // Child and parent last words.
  uint16_t CpWID, PpWID; // Child and parent preceding words.
  uint16_t CnWID, PnWID; // Child and parent next words.

  CoreferenceSentenceNumeric *child_sentence =
    document->GetSentence(child->sentence_index());
  CoreferenceSentenceNumeric *parent_sentence = (parent)?
    document->GetSentence(parent->sentence_index()) : NULL;

  // Use POS tags for now (instead of word forms).
  CWID = child_sentence->GetPosId(child->head_index());
  CfWID = child_sentence->GetPosId(child->start());
  ClWID = child_sentence->GetPosId(child->end());
  CpWID = (child->start() > 0)?
    child_sentence->GetPosId(child->start()-1) : TOKEN_START;
  CnWID = (child->end() < child_sentence->size() - 1)?
    child_sentence->GetPosId(child->end()+1) : TOKEN_STOP;
  if (parent) {
    PWID = parent_sentence->GetPosId(parent->head_index());
    PfWID = parent_sentence->GetPosId(parent->start());
    PlWID = parent_sentence->GetPosId(parent->end());
    PpWID = (parent->start() > 0)?
      parent_sentence->GetPosId(parent->start()-1) : TOKEN_START;
    PnWID = (parent->end() < parent_sentence->size() - 1)?
      parent_sentence->GetPosId(parent->end()+1) : TOKEN_STOP;
  }

  uint8_t parent_gender_code = 0xff;
  uint8_t parent_number_code = 0xff;
  if (parent) {
    CHECK_LT(parent->gender(), 0xff);
    parent_gender_code = parent->gender();
    CHECK_LT(parent->number(), 0xff);
    parent_number_code = parent->number();
  }

  int length_threshold = 20;
  uint8_t child_length_code = 0xff;
  int child_length = child->GetLength();
  if (child_length > length_threshold) {
    child_length_code = length_threshold;
  } else {
    child_length_code = child_length;
  }
  uint8_t parent_length_code = 0xff;
  if (parent) {
    int parent_length = parent->GetLength();
    if (parent_length > length_threshold) {
      parent_length_code = length_threshold;
    } else {
      parent_length_code = parent_length;
    }
  }

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

  // Child length.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Cl, flags,
                               child_length_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Cl_Ct, flags,
                                CtID, child_length_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Cl_Ct_Pt, flags,
                                 CtID, PtID, child_length_code);
  AddFeature(fkey, features);

  // Parent length.
  if (parent) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Pl, flags,
                                 parent_length_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Pl_Ct, flags,
                                  CtID, parent_length_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Pl_Ct_Pt, flags,
                                   CtID, PtID, parent_length_code);
    AddFeature(fkey, features);
  }

  // Parent/child mention head word.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CW, flags,
                                 CWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CW_Ct, flags,
                                  CWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CW_Ct_Pt, flags,
                                   CWID, CtID, PtID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PW, flags,
                                 PWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PW_Ct, flags,
                                  PWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PW_Ct_Pt, flags,
                                   PWID, CtID, PtID);
    AddFeature(fkey, features);
  }

  // Parent/child mention first word.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CfW, flags,
                                 CfWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CfW_Ct, flags,
                                  CfWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CfW_Ct_Pt, flags,
                                   CfWID, CtID, PtID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PfW, flags,
                                 PfWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PfW_Ct, flags,
                                  PfWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PfW_Ct_Pt, flags,
                                   PfWID, CtID, PtID);
    AddFeature(fkey, features);
  }

  // Parent/child mention last word.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::ClW, flags,
                                 ClWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::ClW_Ct, flags,
                                  ClWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::ClW_Ct_Pt, flags,
                                   ClWID, CtID, PtID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PlW, flags,
                                 PlWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PlW_Ct, flags,
                                  PlWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PlW_Ct_Pt, flags,
                                   PlWID, CtID, PtID);
    AddFeature(fkey, features);
  }

  // Parent/child mention preceding word.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CpW, flags,
                                 CpWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CpW_Ct, flags,
                                  CpWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CpW_Ct_Pt, flags,
                                   CpWID, CtID, PtID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PpW, flags,
                                 PpWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PpW_Ct, flags,
                                  PpWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PpW_Ct_Pt, flags,
                                   PpWID, CtID, PtID);
    AddFeature(fkey, features);
  }

  // Parent/child mention next word.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CnW, flags,
                                 CnWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CnW_Ct, flags,
                                  CnWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CnW_Ct_Pt, flags,
                                   CnWID, CtID, PtID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PnW, flags,
                                 PnWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PnW_Ct, flags,
                                  PnWID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PnW_Ct_Pt, flags,
                                   PnWID, CtID, PtID);
    AddFeature(fkey, features);
  }

  if (use_gender_number_features) {
    // Parent gender.
    if (parent) {
      fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Pg, flags,
                                   parent_gender_code);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Pg_Ct, flags,
                                    CtID, parent_gender_code);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Pg_Ct_Pt, flags,
                                     CtID, PtID, parent_gender_code);
      AddFeature(fkey, features);
    }

    // Parent number.
    if (parent) {
      fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Pn, flags,
                                   parent_number_code);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Pn_Ct, flags,
                                    CtID, parent_number_code);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Pn_Ct_Pt, flags,
                                     CtID, PtID, parent_number_code);
      AddFeature(fkey, features);
    }
  }

  if (parent) {
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
}
