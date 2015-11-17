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
  bool use_ancestry_features = true;
  bool use_contained_features = true;
  bool use_nested_feature = true;
  bool use_speaker_feature = true;

  const vector<Mention*> &mentions = document->GetMentions();
  Mention *parent = (parent_mention >= 0) ? mentions[parent_mention] : NULL;
  Mention *child = mentions[child_mention];
  int sentence_distance = (parent) ?
    child->sentence_index() - parent->sentence_index() : -1;
  int mention_distance = (parent) ? child_mention - parent_mention : -1;

  bool nested = false;
  if (parent && child->sentence_index() == parent->sentence_index() &&
      (child->LiesInsideSpan(*parent) || parent->LiesInsideSpan(*child))) {
    nested = true;
  }
  uint8_t nested_flag = 0x0;
  if (nested) nested_flag = 0x1;

  bool same_speaker = true;
  if (parent && child->speaker_id() != parent->speaker_id()) {
    same_speaker = false;
  }
  bool conversation = document->is_conversation();
  uint8_t speaker_flag = 0x0;
  if (conversation) speaker_flag |= 0x1;
  if (!same_speaker) speaker_flag |= 0x1 << 1;

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

  uint8_t CPID, PPID; // Child and parent head tags.
  uint8_t CfPID, PfPID; // Child and parent first tags.
  uint8_t ClPID, PlPID; // Child and parent last tags.
  uint8_t CpPID, PpPID; // Child and parent preceding tags.
  uint8_t CnPID, PnPID; // Child and parent next tags.

  uint16_t CA1ID, PA1ID; // Child and parent unigram ancestry.
  uint16_t CA2ID, PA2ID; // Child and parent bigram ancestry.

  CoreferenceSentenceNumeric *child_sentence =
    document->GetSentence(child->sentence_index());
  CoreferenceSentenceNumeric *parent_sentence = (parent) ?
    document->GetSentence(parent->sentence_index()) : NULL;

  // Use POS tags for now (instead of word forms).
  CWID = child_sentence->GetFormId(child->head_index());
  CfWID = child_sentence->GetFormId(child->start());
  ClWID = child_sentence->GetFormId(child->end());
  CpWID = (child->start() > 0) ?
    child_sentence->GetFormId(child->start() - 1) : TOKEN_START;
  CnWID = (child->end() < child_sentence->size() - 1) ?
    child_sentence->GetFormId(child->end() + 1) : TOKEN_STOP;

  CPID = child_sentence->GetPosId(child->head_index());
  CfPID = child_sentence->GetPosId(child->start());
  ClPID = child_sentence->GetPosId(child->end());
  CpPID = (child->start() > 0) ?
    child_sentence->GetPosId(child->start() - 1) : TOKEN_START;
  CnPID = (child->end() < child_sentence->size() - 1) ?
    child_sentence->GetPosId(child->end() + 1) : TOKEN_STOP;

  if (parent) {
    PWID = parent_sentence->GetFormId(parent->head_index());
    PfWID = parent_sentence->GetFormId(parent->start());
    PlWID = parent_sentence->GetFormId(parent->end());
    PpWID = (parent->start() > 0) ?
      parent_sentence->GetFormId(parent->start() - 1) : TOKEN_START;
    PnWID = (parent->end() < parent_sentence->size() - 1) ?
      parent_sentence->GetFormId(parent->end() + 1) : TOKEN_STOP;

    PPID = parent_sentence->GetPosId(parent->head_index());
    PfPID = parent_sentence->GetPosId(parent->start());
    PlPID = parent_sentence->GetPosId(parent->end());
    PpPID = (parent->start() > 0) ?
      parent_sentence->GetPosId(parent->start() - 1) : TOKEN_START;
    PnPID = (parent->end() < parent_sentence->size() - 1) ?
      parent_sentence->GetPosId(parent->end() + 1) : TOKEN_STOP;
  }

  CA1ID = child->unigram_ancestry();
  CA2ID = child->bigram_ancestry();
  PA1ID = parent ? parent->unigram_ancestry() : 0xffff;
  PA2ID = parent ? parent->bigram_ancestry() : 0xffff;

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
    (parent && (parent->phrase_string_id() == child->phrase_string_id())) ?
    0x1 : 0x0;
  uint8_t head_match_code =
    (parent && (parent->head_string_id() == child->head_string_id())) ?
    0x1 : 0x0;

  uint8_t child_contained_code =
    (parent && (parent->ContainsMentionString(*child))) ? 0x1 : 0x0;
  uint8_t child_head_contained_code =
    (parent && (parent->ContainsMentionHead(*child))) ? 0x1 : 0x0;
  uint8_t parent_contained_code =
    (parent && (child->ContainsMentionString(*parent))) ? 0x1 : 0x0;
  uint8_t parent_head_contained_code =
    (parent && (child->ContainsMentionHead(*parent))) ? 0x1 : 0x0;

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
  fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CpW, flags,
                               CpWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CpW_Ct, flags,
                                CpWID, CtID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CpW_Ct_Pt, flags,
                                 CpWID, CtID, PtID);
  AddFeature(fkey, features);
  if (parent) {
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
  fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CnW, flags,
                               CnWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CnW_Ct, flags,
                                CnWID, CtID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CnW_Ct_Pt, flags,
                                 CnWID, CtID, PtID);
  AddFeature(fkey, features);
  if (parent) {
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

  // Parent/child mention head tag.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::CP, flags,
                                 CPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::CP_Ct, flags,
                                  CtID, CPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::CP_Ct_Pt, flags,
                                   CtID, PtID, CPID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::PP, flags,
                                 PPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::PP_Ct, flags,
                                  CtID, PPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::PP_Ct_Pt, flags,
                                   CtID, PtID, PPID);
    AddFeature(fkey, features);
  }

  // Parent/child mention first tag.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::CfP, flags,
                                 CfPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::CfP_Ct, flags,
                                  CtID, CfPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::CfP_Ct_Pt, flags,
                                   CtID, PtID, CfPID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::PfP, flags,
                                 PfPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::PfP_Ct, flags,
                                  CtID, PfPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::PfP_Ct_Pt, flags,
                                   CtID, PtID, PfPID);
    AddFeature(fkey, features);
  }

  // Parent/child mention last tag.
  if (child->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::ClP, flags,
                                 ClPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::ClP_Ct, flags,
                                  CtID, ClPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::ClP_Ct_Pt, flags,
                                   CtID, PtID, ClPID);
    AddFeature(fkey, features);
  }
  if (parent && parent->type() != MentionType::PRONOMINAL) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::PlP, flags,
                                 PlPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::PlP_Ct, flags,
                                  CtID, PlPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::PlP_Ct_Pt, flags,
                                   CtID, PtID, PlPID);
    AddFeature(fkey, features);
  }

  // Parent/child mention preceding tag.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::CpP, flags,
                               CpPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::CpP_Ct, flags,
                                CtID, CpPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::CpP_Ct_Pt, flags,
                                 CtID, PtID, CpPID);
  AddFeature(fkey, features);
  if (parent) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::PpP, flags,
                                 PpPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::PpP_Ct, flags,
                                  CtID, PpPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::PpP_Ct_Pt, flags,
                                   CtID, PtID, PpPID);
    AddFeature(fkey, features);
  }

  // Parent/child mention next tag.
  fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::CnP, flags,
                               CnPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::CnP_Ct, flags,
                                CtID, CnPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::CnP_Ct_Pt, flags,
                                 CtID, PtID, CnPID);
  AddFeature(fkey, features);
  if (parent) {
    fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::PnP, flags,
                                 PnPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::PnP_Ct, flags,
                                  CtID, PnPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::PnP_Ct_Pt, flags,
                                   CtID, PtID, PnPID);
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

  if (use_ancestry_features) {
    // Child unigram ancestry.
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CA1, flags,
                                 CA1ID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CA1_Ct, flags,
                                  CA1ID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CA1_Ct_Pt, flags,
                                   CA1ID, CtID, PtID);
    AddFeature(fkey, features);
    if (parent) {
      // Parent unigram ancestry.
      fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PA1, flags,
                                   PA1ID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PA1_Ct, flags,
                                    PA1ID, CtID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PA1_Ct_Pt, flags,
                                     PA1ID, CtID, PtID);
      AddFeature(fkey, features);
    }

    // Child bigram ancestry.
    fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::CA2, flags,
                                 CA2ID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::CA2_Ct, flags,
                                  CA2ID, CtID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::CA2_Ct_Pt, flags,
                                   CA2ID, CtID, PtID);
    AddFeature(fkey, features);
    if (parent) {
      // Parent bigram ancestry.
      fkey = encoder_.CreateFKey_W(CoreferenceFeatureTemplateArc::PA2, flags,
                                   PA2ID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(CoreferenceFeatureTemplateArc::PA2_Ct, flags,
                                    PA2ID, CtID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWW(CoreferenceFeatureTemplateArc::PA2_Ct_Pt, flags,
                                     PA2ID, CtID, PtID);
      AddFeature(fkey, features);
    }
    // TODO(atm): implement joint child/parent ancestry.
  }

  if (use_nested_feature) {
    if (parent) {
      // True if mentions are nested.
      fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::nest, flags,
                                   nested_flag);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::nest_Ct, flags,
                                    CtID, nested_flag);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::nest_Ct_Pt, flags,
                                     CtID, PtID, nested_flag);
      AddFeature(fkey, features);
    }
  }

  if (use_speaker_feature) {
    if (parent && parent->type() == MentionType::PRONOMINAL) {
      // Speaker code (different speaker + document type).
      fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::speak, flags,
                                   speaker_flag);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::speak_Ct, flags,
                                    CtID, speaker_flag);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::speak_Ct_Pt, flags,
                                     CtID, PtID, speaker_flag);
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

    if (child->type() != MentionType::PRONOMINAL) {
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

      if (use_contained_features) {
        // Child head contained.
        fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Chc, flags,
                                     child_head_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Chc_Ct, flags,
                                      CtID, child_head_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Chc_Ct_Pt, flags,
                                       CtID, PtID, child_head_contained_code);
        AddFeature(fkey, features);

        // Parent head contained.
        fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Phc, flags,
                                     parent_head_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Phc_Ct, flags,
                                      CtID, parent_head_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Phc_Ct_Pt, flags,
                                       CtID, PtID, parent_head_contained_code);
        AddFeature(fkey, features);

        // Exact child contained.
        fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Cec, flags,
                                     child_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Cec_Ct, flags,
                                      CtID, child_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Cec_Ct_Pt, flags,
                                       CtID, PtID, child_contained_code);
        AddFeature(fkey, features);

        // Exact parent contained.
        fkey = encoder_.CreateFKey_P(CoreferenceFeatureTemplateArc::Pec, flags,
                                     parent_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(CoreferenceFeatureTemplateArc::Pec_Ct, flags,
                                      CtID, parent_contained_code);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(CoreferenceFeatureTemplateArc::Pec_Ct_Pt, flags,
                                       CtID, PtID, parent_contained_code);
        AddFeature(fkey, features);
      }
    }
  }
}
