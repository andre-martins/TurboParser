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
#include "SemanticFeatures.h"
#include "SemanticPart.h"
#include "SemanticFeatureTemplates.h"
#include <set>

// Flags for specific options in feature definitions.
// Note: this will be deprecated soon.
// Note 2: these flags don't get saved in the model file!!! So we need to call
// them at test time too.
// TODO: deprecate this.
DEFINE_bool(srl_use_contextual_features, true,
            "True for using contextual arc-factored features.");
DEFINE_bool(srl_use_predicate_features, true, //false,
            "True for using predicate features.");
DEFINE_bool(srl_use_pair_features_arbitrary_siblings, false, /*false,*/
            "True for using pair features for arbitrary sibling parts.");
DEFINE_bool(srl_use_pair_features_second_order, true, /*false,*/
            "True for using pair features for second order parts.");
DEFINE_bool(srl_use_pair_features_grandsibling_conjunctions, true, /*false,*/
            "True for using pair features for grandsiblings that are conjunctions.");
// TODO: try setting this true.
DEFINE_bool(srl_use_trilexical_features, false,
            "True for using trilexical features.");

void SemanticFeatures::AddPredicateFeatures(SemanticInstanceNumeric* sentence,
                                            int r,
                                            int predicate,
                                            int predicate_id) {
  //LOG(INFO) << "Adding predicate features";

  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  if (FLAGS_srl_use_predicate_features) {
    AddPredicateFeatures(sentence, false,
                         SemanticFeatureTemplateParts::PREDICATE,
                         r, predicate, predicate_id);
  }
}

#if 0
void SemanticFeatures::AddArcFeatures(SemanticInstanceNumeric* sentence,
                                      int r,
                                      int predicate,
                                      int argument,
                                      int predicate_id) {
  //LOG(INFO) << "Adding arc features";

  SemanticOptions *options = static_cast<class SemanticPipe*>(pipe_)->
      GetSemanticOptions();

  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  // Add arc predicate features.
  AddPredicateFeatures(sentence, SemanticFeatureTemplateParts::ARC_PREDICATE,
                       r, predicate, predicate_id);

  int sentence_length = sentence->size();
  // True if labeled semantic parsing.
  bool labeled =
      static_cast<SemanticOptions*>(pipe_->GetOptions())->labeled();

  bool use_dependency_features = options->use_dependency_syntactic_features();
  bool use_contextual_dependency_features = use_dependency_features;
  bool use_contextual_features = FLAGS_srl_use_contextual_features;
  bool use_between_features = false; // TODO(atm): change this.

  // Only 4 bits are allowed in feature_type.
  //CHECK_LT(pair_type, 16);
  //CHECK_GE(pair_type, 0);
  uint8_t feature_type = SemanticFeatureTemplateParts::ARC;
  CHECK_LT(feature_type, 16);
  CHECK_GE(feature_type, 0);

  int left_position, right_position;
  int arc_length;

  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t binned_length_code; // Binned arc length.

  if (argument < predicate) {
    left_position = argument;
    right_position = predicate;
    direction_code = 0x0;
  } else {
    left_position = predicate;
    right_position = argument;
    direction_code = 0x1;
  }
  arc_length = right_position - left_position;

  // 7 possible values for binned_length_code (3 bits)
  if (arc_length > 40) {
    binned_length_code = 0x3; //0x6;
  } else if (arc_length > 30) {
    binned_length_code = 0x3; //0x5;
  } else if (arc_length > 20) {
    binned_length_code = 0x3; //0x4;
  } else if (arc_length > 10) {
    binned_length_code = 0x3;
  } else if (arc_length > 5) {
    binned_length_code = 0x2;
  } else if (arc_length > 2) {
    binned_length_code = 0x1;
  } else {
    binned_length_code = 0x0;
  }

  // List of argument dependents, left and right siblings.
  const vector<int> &argument_dependents = sentence->GetModifiers(argument);
  int argument_left_sibling = sentence->GetLeftSibling(argument);
  int argument_right_sibling = sentence->GetRightSibling(argument);
  int l = argument_dependents.size();
  int argument_leftmost_dependent = (l > 0)? argument_dependents[0] : -1;
  int argument_rightmost_dependent = (l > 0)? argument_dependents[l-1] : -1;

  // Flag for passive voice (for the predicate).
  // 0x0: No verb.
  // 0x1: Active voice.
  // 0x2: Passive voice.
  // Requires 2 bits.
  uint8_t passive_voice_code = 0x0;
  if (use_dependency_features) {
    // Right now, the detection of passive voice requires dependency syntax
    // information.
    if (sentence->IsVerb(predicate)) {
      if (sentence->IsPassiveVoice(predicate)) {
        passive_voice_code = 0x1;
      } else {
        passive_voice_code = 0x2;
      }
    }
  }

  // Mode codeword.
  // mode = 0: no extra info;
  // mode = 1: direction of attachment.
  uint8_t mode;

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, HLID, HSID;
  uint8_t HPID, MPID, BPID;
  uint8_t pHPID, pMPID, nHPID, nMPID;
  uint16_t ldMWID, rdMWID, lMWID, rMWID;
  uint8_t ldMPID, rdMPID, lMPID, rMPID;
  uint8_t MRID;
  uint16_t PATHRID, PATHPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();
  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetPosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = (*word_ids)[predicate];
  MWID = (*word_ids)[argument];
  HPID = (*pos_ids)[predicate];
  MPID = (*pos_ids)[argument];

  // Predicate lemma/sense.
  HLID = sentence->GetLemmaId(predicate);
  CHECK_GE(predicate_id, 0);
  CHECK_LT(predicate_id, 0xffff);
  HSID = predicate_id;

  // Argument dependency relation.
  if (use_dependency_features) {
    MRID = sentence->GetRelationId(argument);
  } else {
    MRID = 0x0;
  }

  // Dependency paths.
  if (use_dependency_features) {
    PATHRID = sentence->GetRelationPathId(predicate, argument);
    PATHPID = sentence->GetPosPathId(predicate, argument);
  } else {
    PATHRID = 0x0;
    PATHPID = 0x0;
  }

  // Contextual dependency information (argument only).
  if (use_contextual_dependency_features) {
    ldMPID = (argument_leftmost_dependent > 0)?
      (*pos_ids)[argument_leftmost_dependent - 1] : TOKEN_START;
    rdMPID = (argument_rightmost_dependent > 0)?
      (*pos_ids)[argument_rightmost_dependent - 1] : TOKEN_STOP;
    lMPID = (argument_left_sibling > 0)?
      (*pos_ids)[argument_left_sibling - 1] : TOKEN_START;
    rMPID = (argument_right_sibling > 0)?
      (*pos_ids)[argument_right_sibling - 1] : TOKEN_STOP;
    ldMWID = (argument_leftmost_dependent > 0)?
      (*word_ids)[argument_leftmost_dependent - 1] : TOKEN_START;
    rdMWID = (argument_rightmost_dependent > 0)?
      (*word_ids)[argument_rightmost_dependent - 1] : TOKEN_STOP;
    lMWID = (argument_left_sibling > 0)?
      (*word_ids)[argument_left_sibling - 1] : TOKEN_START;
    rMWID = (argument_right_sibling > 0)?
      (*word_ids)[argument_right_sibling - 1] : TOKEN_STOP;
  } else {
    ldMPID = 0x0;
    rdMPID = 0x0;
    lMPID = 0x0;
    rMPID = 0x0;
    ldMWID = 0x0;
    rdMWID = 0x0;
    lMWID = 0x0;
    rMWID = 0x0;
  }

  // Contextual information.
  pHPID = (predicate > 0)? (*pos_ids)[predicate - 1] : TOKEN_START;
  pMPID = (argument > 0)? (*pos_ids)[argument - 1] : TOKEN_START;
  nHPID = (predicate < sentence_length - 1)? (*pos_ids)[predicate + 1] : TOKEN_STOP;
  nMPID = (argument < sentence_length - 1)?
    (*pos_ids)[argument + 1] : TOKEN_STOP;

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateArc::COUNT, 256);

  for (mode = 0; mode < 2; ++mode) {
    // Code for feature type, mode and extended mode.
    flags = feature_type;
    flags |= (mode << 4); // 1 more bit.

    if (mode == 1) {
      flags |= (direction_code << 5); // 1 more bit.
      flags |= (binned_length_code << 6); // 3 more bits.
    }

    // Bias feature.
    fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateArc::BIAS, flags);
    AddFeature(fkey, features);

    // POS features.
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::HP, flags, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::MP, flags, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_MP, flags, HPID, MPID);
    AddFeature(fkey, features);

    // Lexical/Bilexical features.
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HW, flags, HWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::MW, flags, MWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_MW, flags, HWID, MWID);
    AddFeature(fkey, features);

    // Features involving words and POS.
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HWP, flags, HWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::MWP, flags, MWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_MW, flags, MWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_MP, flags, HWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(SemanticFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
    AddFeature(fkey, features);

    // Predicate lemma.
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HL, flags, HLID);
    AddFeature(fkey, features);

    // Predicate sense.
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HS, flags, HSID);
    AddFeature(fkey, features);

    // Predicate voice.
    if (use_dependency_features) {
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::HV, flags,
                                   passive_voice_code);
      AddFeature(fkey, features);

      // Argument dependency relation.
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::MR, flags, MRID);
      AddFeature(fkey, features);

      // Path between argument and predicate in the dependency tree (relations).
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::PATHR, flags, PATHRID);
      AddFeature(fkey, features);

      // Path between argument and predicate in the dependency tree (POS tags).
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::PATHP, flags, PATHPID);
      AddFeature(fkey, features);

      // RelPathToSupport.
      // TODO(atm)

      // VerbChainHasSubj.
      // TODO(atm)

      // ControllerHasObj.
      // TODO(atm)
    }

    if (use_contextual_dependency_features) {
      // Contextual dependency features: argument dependents.
      fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_ldMP, flags, HPID, ldMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_ldMP, flags, HWID, ldMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_ldMW, flags, ldMWID, HPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_ldMW, flags, HWID, ldMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_rdMP, flags, HPID, rdMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_rdMP, flags, HWID, rdMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_rdMW, flags, rdMWID, HPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_rdMW, flags, HWID, rdMWID);
      AddFeature(fkey, features);

      // Contextual dependency features: argument siblings.
      fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_lMP, flags, HPID, lMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_lMP, flags, HWID, lMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_lMW, flags, lMWID, HPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_lMW, flags, HWID, lMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_rMP, flags, HPID, rMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_rMP, flags, HWID, rMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_rMW, flags, rMWID, HPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_rMW, flags, HWID, rMWID);
      AddFeature(fkey, features);
    }

    if (use_contextual_features) {
      // Contextual features.
      fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
      AddFeature(fkey, features);
    }

    if (use_between_features) {
      // In-between features.
      set<int> BPIDs;
      for (int i = left_position + 1; i < right_position; ++i) {
        BPID = (*pos_ids)[i];
        if (BPIDs.find(BPID) == BPIDs.end()) {
          BPIDs.insert(BPID);

          // POS in the middle.
          fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
          AddFeature(fkey, features);
        }
      }
      BPIDs.clear();
    }
  }
}
#else
void SemanticFeatures::AddArcFeatures(SemanticInstanceNumeric* sentence,
                                      int r,
                                      int predicate,
                                      int argument,
                                      int predicate_id) {
  AddArcFeatures(sentence, false, r, predicate, argument, predicate_id);
}

void SemanticFeatures::AddLabeledArcFeatures(SemanticInstanceNumeric* sentence,
                                             int r,
                                             int predicate,
                                             int argument,
                                             int predicate_id) {
  AddArcFeatures(sentence, true, r, predicate, argument, predicate_id);
}

void SemanticFeatures::AddArcFeatures(SemanticInstanceNumeric* sentence,
                                      bool labeled,
                                      int r,
                                      int predicate,
                                      int argument,
                                      int predicate_id) {
  //LOG(INFO) << "Adding arc features";

  SemanticOptions *options = static_cast<class SemanticPipe*>(pipe_)->
      GetSemanticOptions();

  BinaryFeatures *features = new BinaryFeatures;
  if (labeled) {
    CHECK_GE(r, 0);
    CHECK_LT(r, input_labeled_features_.size());
    CHECK(!input_labeled_features_[r]);
    input_labeled_features_[r] = features;
  } else {
    CHECK(!input_features_[r]);
    input_features_[r] = features;
  }

  // Add arc predicate features.
  AddPredicateFeatures(sentence, labeled,
                       SemanticFeatureTemplateParts::ARC_PREDICATE,
                       r, predicate, predicate_id);

  int sentence_length = sentence->size();
  bool use_dependency_features = options->use_dependency_syntactic_features();
  bool use_contextual_dependency_features = use_dependency_features;
  bool use_contextual_features = FLAGS_srl_use_contextual_features;
  bool use_lemma_features = true;
  bool use_between_features = true;

  // Only 4 bits are allowed in feature_type.
  // TODO(atm): allow other pair types.
  int pair_type = SemanticFeatureTemplateParts::ARC;
  CHECK_LT(pair_type, 16);
  CHECK_GE(pair_type, 0);
  uint8_t feature_type = pair_type;

  int max_token_context = 2; //1; //FLAGS_semantic_token_context; // 1.

  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t binned_length_code; // Binned arc length.
  uint8_t exact_length_code; // Exact arc length.
  int left_position, right_position;
  if (argument < predicate) {
    left_position = argument;
    right_position = predicate;
    direction_code = 0x0;
  } else {
    left_position = predicate;
    right_position = argument;
    direction_code = 0x1;
  }
  int arc_length = right_position - left_position;

  // 7 possible values for binned_length_code (3 bits).
  exact_length_code = (arc_length > 0xff)? 0xff : arc_length;
  if (arc_length > 40) {
    binned_length_code = 0x6;
  } else if (arc_length > 30) {
    binned_length_code = 0x5;
  } else if (arc_length > 20) {
    binned_length_code = 0x4;
  } else if (arc_length > 10) {
    binned_length_code = 0x3;
  } else if (arc_length > 5) {
    binned_length_code = 0x2;
  } else if (arc_length > 2) {
    binned_length_code = 0x1;
  } else {
    binned_length_code = 0x0;
  }

  // List of argument dependents, left and right siblings.
  const vector<int> &argument_dependents = sentence->GetModifiers(argument);
  int argument_left_sibling = sentence->GetLeftSibling(argument);
  int argument_right_sibling = sentence->GetRightSibling(argument);
  int l = argument_dependents.size();
  int argument_leftmost_dependent = (l > 0)? argument_dependents[0] : -1;
  int argument_rightmost_dependent = (l > 0)? argument_dependents[l-1] : -1;

  // Flag for passive voice (for the predicate).
  // 0x0: No verb.
  // 0x1: Active voice.
  // 0x2: Passive voice.
  // Requires 2 bits.
  uint8_t passive_voice_code = 0x0;
  if (use_dependency_features) {
    // Right now, the detection of passive voice requires dependency syntax
    // information.
    if (sentence->IsVerb(predicate)) {
      if (sentence->IsPassiveVoice(predicate)) {
        passive_voice_code = 0x1;
      } else {
        passive_voice_code = 0x2;
      }
    }
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, BWID;
  uint16_t HLID, MLID;
  uint16_t HSID, MSID;
  uint8_t HPID, MPID, BPID;
  uint16_t pHWID, pMWID, nHWID, nMWID;
  uint16_t pHLID, pMLID, nHLID, nMLID;
  uint8_t pHPID, pMPID, nHPID, nMPID;
  uint16_t ppHWID, ppMWID, nnHWID, nnMWID;
  uint16_t ppHLID, ppMLID, nnHLID, nnMLID;
  uint8_t ppHPID, ppMPID, nnHPID, nnMPID;

  // Codewords for dependency-based features.
  uint16_t ldMWID, rdMWID, lMWID, rMWID;
  uint8_t ldMPID, rdMPID, lMPID, rMPID;
  uint8_t MRID;
  uint16_t PATHRID, PATHPID;

  // Several flags.
  // 4 bits to denote the kind of flag.
  // Maximum will be 16 flags.
  uint8_t flag_between_verb = 0x0;
  uint8_t flag_between_punc = 0x1;
  uint8_t flag_between_coord = 0x2;

  // TODO: This is expensive and could be precomputed.
  int num_between_verb = 0;
  int num_between_punc = 0;
  int num_between_coord = 0;
  for (int i = left_position + 1; i < right_position; ++i) {
    if (sentence->IsVerb(i)) {
      ++num_between_verb;
    } else if (sentence->IsPunctuation(i)) {
      ++num_between_punc;
    } else if (sentence->IsCoordination(i)) {
      ++num_between_coord;
    }
  }

  // 4 bits to denote the number of occurrences for each flag.
  // Maximum will be 15 occurrences.
  int max_occurrences = 15;
  if (num_between_verb > max_occurrences) num_between_verb = max_occurrences;
  if (num_between_punc > max_occurrences) num_between_punc = max_occurrences;
  if (num_between_coord > max_occurrences) num_between_coord = max_occurrences;
  flag_between_verb |= (num_between_verb << 4);
  flag_between_punc |= (num_between_punc << 4);
  flag_between_coord |= (num_between_coord << 4);

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateArc::COUNT, 256);

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = sentence->GetFormId(predicate);
  MWID = sentence->GetFormId(argument);
  HPID = sentence->GetPosId(predicate);
  MPID = sentence->GetPosId(argument);

  // Predicate lemma/sense.
  HLID = sentence->GetLemmaId(predicate);
  CHECK_GE(predicate_id, 0);
  CHECK_LT(predicate_id, 0xffff);
  HSID = predicate_id;

  // Argument dependency relation.
  if (use_dependency_features) {
    MRID = sentence->GetRelationId(argument);
  } else {
    MRID = 0x0;
  }

  // Dependency paths.
  if (use_dependency_features) {
    PATHRID = sentence->GetRelationPathId(predicate, argument);
    PATHPID = sentence->GetPosPathId(predicate, argument);
  } else {
    PATHRID = 0x0;
    PATHPID = 0x0;
  }

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();
  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetPosIds();

  // Contextual dependency information (argument only).
  if (use_contextual_dependency_features) {
    ldMPID = (argument_leftmost_dependent > 0)?
      (*pos_ids)[argument_leftmost_dependent - 1] : TOKEN_START;
    rdMPID = (argument_rightmost_dependent > 0)?
      (*pos_ids)[argument_rightmost_dependent - 1] : TOKEN_STOP;
    lMPID = (argument_left_sibling > 0)?
      (*pos_ids)[argument_left_sibling - 1] : TOKEN_START;
    rMPID = (argument_right_sibling > 0)?
      (*pos_ids)[argument_right_sibling - 1] : TOKEN_STOP;
    ldMWID = (argument_leftmost_dependent > 0)?
      (*word_ids)[argument_leftmost_dependent - 1] : TOKEN_START;
    rdMWID = (argument_rightmost_dependent > 0)?
      (*word_ids)[argument_rightmost_dependent - 1] : TOKEN_STOP;
    lMWID = (argument_left_sibling > 0)?
      (*word_ids)[argument_left_sibling - 1] : TOKEN_START;
    rMWID = (argument_right_sibling > 0)?
      (*word_ids)[argument_right_sibling - 1] : TOKEN_STOP;
  } else {
    ldMPID = 0x0;
    rdMPID = 0x0;
    lMPID = 0x0;
    rMPID = 0x0;
    ldMWID = 0x0;
    rdMWID = 0x0;
    lMWID = 0x0;
    rMWID = 0x0;
  }

  // Contextual information.
  // Context size = 1:
  pHLID = (predicate > 0)? sentence->GetLemmaId(predicate - 1) : TOKEN_START;
  pMLID = (argument > 0)? sentence->GetLemmaId(argument - 1) : TOKEN_START;
  pHWID = (predicate > 0)? sentence->GetFormId(predicate - 1) : TOKEN_START;
  pMWID = (argument > 0)? sentence->GetFormId(argument - 1) : TOKEN_START;
  pHPID = (predicate > 0)? sentence->GetPosId(predicate - 1) : TOKEN_START;
  pMPID = (argument > 0)? sentence->GetPosId(argument - 1) : TOKEN_START;

  nHLID = (predicate < sentence_length - 1)?
      sentence->GetLemmaId(predicate + 1) : TOKEN_STOP;
  nMLID = (argument < sentence_length - 1)?
      sentence->GetLemmaId(argument + 1) : TOKEN_STOP;
  nHWID = (predicate < sentence_length - 1)?
      sentence->GetFormId(predicate + 1) : TOKEN_STOP;
  nMWID = (argument < sentence_length - 1)?
      sentence->GetFormId(argument + 1) : TOKEN_STOP;
  nHPID = (predicate < sentence_length - 1)?
      sentence->GetPosId(predicate + 1) : TOKEN_STOP;
  nMPID = (argument < sentence_length - 1)?
      sentence->GetPosId(argument + 1) : TOKEN_STOP;

  // Context size = 2:
  ppHLID = (predicate > 1)? sentence->GetLemmaId(predicate - 2) : TOKEN_START;
  ppMLID = (argument > 1)? sentence->GetLemmaId(argument - 2) : TOKEN_START;
  ppHWID = (predicate > 1)? sentence->GetFormId(predicate - 2) : TOKEN_START;
  ppMWID = (argument > 1)? sentence->GetFormId(argument - 2) : TOKEN_START;
  ppHPID = (predicate > 1)? sentence->GetPosId(predicate - 2) : TOKEN_START;
  ppMPID = (argument > 1)? sentence->GetPosId(argument - 2) : TOKEN_START;

  nnHLID = (predicate < sentence_length - 2)?
      sentence->GetLemmaId(predicate + 2) : TOKEN_STOP;
  nnMLID = (argument < sentence_length - 2)?
      sentence->GetLemmaId(argument + 2) : TOKEN_STOP;
  nnHWID = (predicate < sentence_length - 2)?
      sentence->GetFormId(predicate + 2) : TOKEN_STOP;
  nnMWID = (argument < sentence_length - 2)?
      sentence->GetFormId(argument + 2) : TOKEN_STOP;
  nnHPID = (predicate < sentence_length - 2)?
      sentence->GetPosId(predicate + 2) : TOKEN_STOP;
  nnMPID = (argument < sentence_length - 2)?
      sentence->GetPosId(argument + 2) : TOKEN_STOP;

  // Code for feature type.
  flags = feature_type; // 4 bits.
  flags |= (direction_code << 4); // 1 more bit.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateArc::BIAS, flags);
  AddFeature(fkey, features);

  /////////////////////////////////////////////////////////////////////////////
  // Token features.
  /////////////////////////////////////////////////////////////////////////////

  // POS features.
  fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::HP, flags, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::MP, flags, MPID);
  AddFeature(fkey, features);

  // Lexical features.
  fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HW, flags, HWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::MW, flags, MWID);
  AddFeature(fkey, features);

  // Features involving words and POS.
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HWP, flags, HWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::MWP, flags, MWID, MPID);
  AddFeature(fkey, features);

  // Predicate and argument lemma.
  if (use_lemma_features) {
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HL, flags, HLID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::ML, flags, MLID);
    AddFeature(fkey, features);
  }

  // Predicate sense.
  fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HS, flags, HSID);
  AddFeature(fkey, features);

  /////////////////////////////////////////////////////////////////////////////
  // Features that depend on syntactic dependencies.
  /////////////////////////////////////////////////////////////////////////////

  // Predicate voice.
  if (use_dependency_features) {
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::HV, flags,
                                 passive_voice_code);
    AddFeature(fkey, features);

    // Argument dependency relation.
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::MR, flags, MRID);
    AddFeature(fkey, features);

    // Path between argument and predicate in the dependency tree (relations).
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::PATHR, flags, PATHRID);
    AddFeature(fkey, features);

    // Path between argument and predicate in the dependency tree (POS tags).
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::PATHP, flags, PATHPID);
    AddFeature(fkey, features);

    // RelPathToSupport.
    // TODO(atm)

    // VerbChainHasSubj.
    // TODO(atm)

    // ControllerHasObj.
    // TODO(atm)
  }

  /////////////////////////////////////////////////////////////////////////////
  // Features that depend on syntactic dependency context.
  /////////////////////////////////////////////////////////////////////////////

  if (use_contextual_dependency_features) {
    // Contextual dependency features: argument dependents.
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_ldMP, flags, HPID, ldMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_ldMP, flags, HWID, ldMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_ldMW, flags, ldMWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_ldMW, flags, HWID, ldMWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_rdMP, flags, HPID, rdMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_rdMP, flags, HWID, rdMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_rdMW, flags, rdMWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_rdMW, flags, HWID, rdMWID);
    AddFeature(fkey, features);

    // Contextual dependency features: argument siblings.
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_lMP, flags, HPID, lMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_lMP, flags, HWID, lMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_lMW, flags, lMWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_lMW, flags, HWID, lMWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_rMP, flags, HPID, rMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_rMP, flags, HWID, rMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_rMW, flags, rMWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_rMW, flags, HWID, rMWID);
    AddFeature(fkey, features);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Token contextual features.
  /////////////////////////////////////////////////////////////////////////////

  if (use_contextual_features) {
    // Contextual features.
    if (max_token_context >= 1) {
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::pHP, flags, pHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::nHP, flags, nHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::pMP, flags, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::nMP, flags, nMPID);
      AddFeature(fkey, features);

      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::pHW, flags, pHWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nHW, flags, nHWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::pMW, flags, pMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nMW, flags, nMWID);
      AddFeature(fkey, features);

      if (use_lemma_features) {
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::pHL, flags, pHLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nHL, flags, nHLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::pML, flags, pMLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nML, flags, nMLID);
        AddFeature(fkey, features);
      }

      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::pHWP, flags, pHWID, pHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::nHWP, flags, nHWID, nHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::pMWP, flags, pMWID, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::nMWP, flags, nMWID, nMPID);
      AddFeature(fkey, features);
    }

    if (max_token_context >= 2) {
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::ppHP, flags, ppHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::nnHP, flags, nnHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::ppMP, flags, ppMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::nnMP, flags, nnMPID);
      AddFeature(fkey, features);

      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::ppHW, flags, ppHWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nnHW, flags, nnHWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::ppMW, flags, ppMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nnMW, flags, nnMWID);
      AddFeature(fkey, features);

      if (use_lemma_features) {
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::ppHL, flags, ppHLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nnHL, flags, nnHLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::ppML, flags, ppMLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::nnML, flags, nnMLID);
        AddFeature(fkey, features);
      }
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::ppHWP, flags, ppHWID, ppHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::nnHWP, flags, nnHWID, nnHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::ppMWP, flags, ppMWID, ppMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::nnMWP, flags, nnMWID, nnMPID);
      AddFeature(fkey, features);
    }

    // Contextual bigram and trigram features involving POS.
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_pHP, flags, HPID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::MP_pMP, flags, MPID, pMPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_pHP_ppHP, flags, HPID, pHPID, ppHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::MP_pMP_ppMP, flags, MPID, pMPID, ppMPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_nHP, flags, HPID, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::MP_nMP, flags, MPID, nMPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_nHP_nnHP, flags, HPID, nHPID, nnHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::MP_nMP_nnMP, flags, MPID, nMPID, nnMPID);
    AddFeature(fkey, features);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Arc features.
  // Everything goes with direction flags and with POS.
  /////////////////////////////////////////////////////////////////////////////

  // POS features.
  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  // Lexical/Bilexical features.
  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);

  // Features involving words and POS.
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWPP(SemanticFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
  AddFeature(fkey, features);

  // Contextual features.
  if (use_contextual_features) {
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
    AddFeature(fkey, features);

    // Features for adjacent arcs.
    if (predicate != 0 && predicate == argument - 1) {
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID, 0x1);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID, 0x1);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID, 0x1);
      AddFeature(fkey, features);
    } else if (predicate != 0 && predicate == argument + 1) {
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID, 0x1);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID, 0x1);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PPPPP(SemanticFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID, 0x1);
      AddFeature(fkey, features);
    }
  }

  // Exact arc length.
  fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::DIST, flags, exact_length_code);
  AddFeature(fkey, features);

  // Binned arc length.
  for (uint8_t bin = 0; bin <= binned_length_code; ++bin) {
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::BIAS, flags, bin);
    AddFeature(fkey, features);
  }

  // POS features conjoined with binned arc length.
  for (uint8_t bin = 0; bin <= binned_length_code; bin++) {
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP, flags, HPID, bin);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::MP, flags, MPID, bin);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP, flags, HPID, MPID, bin);
    AddFeature(fkey, features);
  }

  if (use_between_features) {
    // In-between flags.
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::BFLAG, flags, flag_between_verb);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::BFLAG, flags, flag_between_punc);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::BFLAG, flags, flag_between_coord);
    AddFeature(fkey, features);

    // POS features conjoined with in-between flag.
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_verb);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_punc);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_coord);
    AddFeature(fkey, features);

    set<int> BPIDs;
    set<int> BWIDs;
    for (int i = left_position + 1; i < right_position; ++i) {
      BPID = sentence->GetPosId(i);
      if (BPIDs.find(BPID) == BPIDs.end()) {
        BPIDs.insert(BPID);

        // POS in the middle.
        fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateArc::HW_MW_BP, flags, HWID, MWID, BPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HW_MP_BP, flags, HWID, MPID, BPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HP_MW_BP, flags, MWID, HPID, BPID);
        AddFeature(fkey, features);
      }
#if 1
      BWID = sentence->GetFormId(i);
      if (BWIDs.find(BWID) == BWIDs.end()) {
        BWIDs.insert(BWID);

        // Word in the middle (useful for handling prepositions lying between
        // predicates and arguments).
        fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HP_MP_BW, flags, BWID, HPID, MPID);
        AddFeature(fkey, features);
        //fkey = encoder_.CreateFKey_WWW(SemanticFeatureTemplateArc::HW_MW_BW, flags, HWID, MWID, BWID);
        //AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateArc::HW_MP_BW, flags, HWID, BWID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateArc::HP_MW_BW, flags, MWID, BWID, HPID);
        AddFeature(fkey, features);
      }
#endif
    }
    BPIDs.clear();
    BWIDs.clear();
  }
}
#endif


// Add features for arbitrary siblings.
void SemanticFeatures::AddArbitrarySiblingFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int predicate,
                          int sense,
                          int first_argument,
                          int second_argument) {
  AddSiblingFeatures(sentence, false, r, predicate, sense, first_argument,
                     second_argument, false);
}

// Add features for arbitrary labeled siblings.
void SemanticFeatures::AddArbitraryLabeledSiblingFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int predicate,
                          int sense,
                          int first_argument,
                          int second_argument) {
  AddSiblingFeatures(sentence, true, r, predicate, sense, first_argument,
                     second_argument, false);
}

// Add features for consecutive siblings.
void SemanticFeatures::AddConsecutiveSiblingFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int predicate,
                          int sense,
                          int first_argument,
                          int second_argument) {
  AddSiblingFeatures(sentence, false, r, predicate, sense, first_argument,
                     second_argument, true);
}

// Add features for siblings.
void SemanticFeatures::AddSiblingFeatures(SemanticInstanceNumeric* sentence,
                                          bool labeled,
                                          int r,
                                          int predicate,
                                          int sense,
                                          int first_argument,
                                          int second_argument,
                                          bool consecutive) {
  BinaryFeatures *features = new BinaryFeatures;
  if (labeled) {
    CHECK(!input_labeled_features_[r]);
    input_labeled_features_[r] = features;
  } else {
    CHECK(!input_features_[r]);
    input_features_[r] = features;
  }

  int sentence_length = sentence->size();
  // Note: unlike the dependency parser case, here the first child
  // does not have a1 == p (which would be ambiguous with the
  // case where there is a self-loop), but a1 == -1.
  bool first_child = consecutive && (first_argument < 0);
  bool last_child = consecutive &&
                    (second_argument == sentence_length || second_argument <= 0);

  CHECK_NE(second_argument, 0) << "Currently, last child is encoded as a2 = -1.";

#if 0
  if (FLAGS_srl_use_pair_features_second_order) {
    // Add word pair features for head and modifier, and modifier and sibling.
    if (consecutive) {
      int m = modifier;
      int s = sibling;
      if (modifier == head) m = 0; // s is the first child of h.
      if (sibling <= 0 || sibling >= sentence_length) s = 0; // m is the last child of h.

      AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::NEXTSIBL_M_S,
                          m, s, true, true, features);
    } else {
      if (FLAGS_srl_use_pair_features_arbitrary_siblings) {
        // Add word pair features for modifier and sibling.
        AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::ALLSIBL_M_S,
                            modifier, sibling, true, true, features);
      }
    }
  }
#endif

  // Direction of attachment for the first and second children.
  // When consecutive == true, we only look at the second one.
  // TODO: deal with self-cycles.
  uint8_t direction_code_first; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_second; // 0x1 if right attachment, 0x0 otherwise.

  if (first_argument < predicate) {
    direction_code_first = 0x0;
  } else {
    direction_code_first = 0x1;
  }

  if (second_argument < predicate) {
    direction_code_second = 0x0;
  } else {
    direction_code_second = 0x1;
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, SWID;
  uint8_t HPID, MPID, SPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = (*word_ids)[predicate];
  MWID = first_child? TOKEN_START : (*word_ids)[first_argument];
  SWID = last_child? TOKEN_STOP : (*word_ids)[second_argument];
  HPID = (*pos_ids)[predicate];
  MPID = first_child? TOKEN_START : (*pos_ids)[first_argument];
  SPID = last_child? TOKEN_STOP : (*pos_ids)[second_argument];

  if (consecutive) {
    flags = SemanticFeatureTemplateParts::NEXTSIBL;
  } else {
    flags = SemanticFeatureTemplateParts::ALLSIBL;
  }

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateSibling::COUNT, 256);

  // Add direction information.
  if (!consecutive) flags |= (direction_code_first << 6); // 1 more bit.
  flags |= (direction_code_second << 7); // 1 more bit.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateSibling::BIAS, flags);
  AddFeature(fkey, features);

  // Triplet POS feature.
  fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateSibling::HP_MP_SP, flags, HPID, MPID, SPID);
  AddFeature(fkey, features);

  // Triplet unilexical features.
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateSibling::HW_MP_SP, flags, HWID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateSibling::HP_MW_SP, flags, MWID, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateSibling::HP_MP_SW, flags, SWID, HPID, MPID);
  AddFeature(fkey, features);

  // Triplet bilexical features.
  fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateSibling::HW_MW_SP, flags, HWID, MWID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateSibling::HW_MP_SW, flags, HWID, SWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateSibling::HP_MW_SW, flags, MWID, SWID, HPID);
  AddFeature(fkey, features);

  // Trilexical features.
  if (FLAGS_srl_use_trilexical_features) {
    // Triplet trilexical features.
    fkey = encoder_.CreateFKey_WWW(SemanticFeatureTemplateSibling::HW_MW_SW, flags, HWID, MWID, SWID);
    AddFeature(fkey, features);
  }

  // Pairwise POS features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateSibling::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateSibling::HP_SP, flags, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateSibling::MP_SP, flags, MPID, SPID);
  AddFeature(fkey, features);

  // Pairwise unilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateSibling::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateSibling::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateSibling::HW_SP, flags, HWID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateSibling::HP_SW, flags, SWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateSibling::MW_SP, flags, MWID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateSibling::MP_SW, flags, SWID, MPID);
  AddFeature(fkey, features);

  // Pairwise bilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateSibling::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateSibling::HW_SW, flags, HWID, SWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateSibling::MW_SW, flags, MWID, SWID);
  AddFeature(fkey, features);
}


// Add features for grandparents.
void SemanticFeatures::AddGrandparentFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int grandparent_predicate,
                          int grandparent_sense,
                          int predicate,
                          int sense,
                          int argument) {
  AddSecondOrderFeatures(sentence, r, grandparent_predicate, grandparent_sense,
                         predicate, sense, argument, false, false);
}

// Add features for co-parents.
void SemanticFeatures::AddCoparentFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int first_predicate,
                          int first_sense,
                          int second_predicate,
                          int second_sense,
                          int argument) {
  AddSecondOrderFeatures(sentence, r, first_predicate, first_sense,
                         second_predicate, second_sense, argument, true, false);
}

// Add features for co-parents.
void SemanticFeatures::AddConsecutiveCoparentFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int first_predicate,
                          int first_sense,
                          int second_predicate,
                          int second_sense,
                          int argument) {
  AddSecondOrderFeatures(sentence, r, first_predicate, first_sense,
                         second_predicate, second_sense, argument, true, true);
}

// Add second-order features (grandparents or co-parents).
void SemanticFeatures::AddSecondOrderFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int first_predicate,
                          int first_sense,
                          int second_predicate,
                          int second_sense,
                          int argument,
                          bool coparents,
                          bool consecutive) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  int sentence_length = sentence->size();

  // Note: the first parent has p1 = -1.
  bool first_parent = consecutive && (first_predicate < 0);
  bool last_parent = consecutive &&
                    (second_predicate == sentence_length ||
                     second_predicate <= 0);

#if 0
  if (FLAGS_srl_use_pair_features_second_order) {
    if (FLAGS_srl_use_upper_dependencies) {
      AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::GRANDPAR_G_H,
                          grandparent, head, true, true, features);
    }
    AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::GRANDPAR_G_M,
                        grandparent, modifier, true, true, features);
  }
#endif

  // Relative position of the grandparent, head and modifier.
  // TODO: deal with self-cycles.
  uint8_t direction_code; // 0x0, 0x1, 0x2, or 0x3 (see four cases below).

  if (coparents) {
    // Direction_code_gp will be the direction between the
    // two predicates that are co-parents.
    // It should always be 0x1 since we assume p1<p2.
    uint8_t direction_code_gp; // 0x1 if right attachment, 0x0 otherwise.
    uint8_t direction_code_pa; // 0x1 if right attachment, 0x0 otherwise.
    uint8_t direction_code_ga; // 0x1 if right attachment, 0x0 otherwise.

    if (second_predicate < first_predicate) {
      direction_code_gp = 0x0;
    } else {
      direction_code_gp = 0x1;
    }

    if (argument < second_predicate) {
      direction_code_pa = 0x0;
    } else {
      direction_code_pa = 0x1;
    }

    if (argument < first_predicate) {
      direction_code_ga = 0x0;
    } else {
      direction_code_ga = 0x1;
    }

    if (direction_code_gp == direction_code_pa) {
      // Argument on the right: p1 - p2 - a.
      // Note: a - p2 - p1 will never happen since p1<p2.
      direction_code = 0x0;
    } else if (direction_code_pa != direction_code_ga) {
      // Argument in the middle: p1 - a - p2.
      direction_code = 0x1;
    } else {
      // Argument on the left: a - p1 - p2.
      direction_code = 0x2;
    }
  } else {
    // Direction_code_gp will be the direction between the
    // grandparent and the parent.
    uint8_t direction_code_gp; // 0x1 if right attachment, 0x0 otherwise.
    uint8_t direction_code_pa; // 0x1 if right attachment, 0x0 otherwise.
    uint8_t direction_code_ga; // 0x1 if right attachment, 0x0 otherwise.

    if (second_predicate < first_predicate) {
      direction_code_gp = 0x0;
    } else {
      direction_code_gp = 0x1;
    }

    if (argument < second_predicate) {
      direction_code_pa = 0x0;
    } else {
      direction_code_pa = 0x1;
    }

    if (argument < first_predicate) {
      direction_code_ga = 0x0;
    } else {
      direction_code_ga = 0x1;
    }

    if (direction_code_gp == direction_code_pa) {
      direction_code = 0x0; // Pointing in the same direction: g - p - a.
    } else if (direction_code_pa != direction_code_ga) {
      direction_code = 0x1; // Zig-zag inwards: g - a - p .
    } else {
      direction_code = 0x2; // Non-projective: a - g - p ("non-projective" for trees).
    }
    // To deal with length-1 cycles.
    if (first_predicate == argument) {
      direction_code = 0x3;
    }
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, GWID;
  uint8_t HPID, MPID, GPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  GWID = first_parent? TOKEN_START : (*word_ids)[first_predicate];
  HWID = last_parent? TOKEN_STOP : (*word_ids)[second_predicate];
  MWID = (*word_ids)[argument];
  GPID = first_parent? TOKEN_START : (*pos_ids)[first_predicate];
  HPID = last_parent? TOKEN_STOP : (*pos_ids)[second_predicate];
  MPID = (*pos_ids)[argument];

  if (coparents) {
    if (consecutive) {
      flags = SemanticFeatureTemplateParts::CONSECUTIVECOPAR;
    } else {
      flags = SemanticFeatureTemplateParts::COPAR;
    }
  } else {
    flags = SemanticFeatureTemplateParts::GRANDPAR;
  }

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateGrandparent::COUNT, 256);

  // Add direction information.
  flags |= (direction_code << 6); // 2 more bits.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateGrandparent::BIAS, flags);
  AddFeature(fkey, features);

  // Triplet POS feature.
  fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateGrandparent::GP_HP_MP, flags, GPID, HPID, MPID);
  AddFeature(fkey, features);

  // Triplet unilexical features.
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateGrandparent::GW_HP_MP, flags, GWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateGrandparent::GP_HW_MP, flags, HWID, GPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateGrandparent::GP_HP_MW, flags, MWID, GPID, HPID);
  AddFeature(fkey, features);

  // Triplet bilexical features.
  fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateGrandparent::GW_HW_MP, flags, GWID, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateGrandparent::GW_HP_MW, flags, GWID, MWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateGrandparent::GP_HW_MW, flags, HWID, MWID, GPID);
  AddFeature(fkey, features);

  if (FLAGS_srl_use_trilexical_features) {
    // Triplet trilexical features.
    fkey = encoder_.CreateFKey_WWW(SemanticFeatureTemplateGrandparent::GW_HW_MW, flags, GWID, HWID, MWID);
    AddFeature(fkey, features);
  }

  // Pairwise POS features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateGrandparent::GP_HP, flags, GPID, HPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateGrandparent::GP_MP, flags, GPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateGrandparent::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  // Pairwise unilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateGrandparent::GW_HP, flags, GWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateGrandparent::GP_HW, flags, HWID, GPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateGrandparent::GW_MP, flags, GWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateGrandparent::GP_MW, flags, MWID, GPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateGrandparent::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateGrandparent::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);

  // Pairwise bilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateGrandparent::GW_HW, flags, GWID, HWID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateGrandparent::GW_MW, flags, GWID, MWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateGrandparent::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);
}


#if 0
// Add features for grand-siblings.
void SemanticFeatures::AddGrandSiblingFeatures(SemanticInstanceNumeric* sentence,
                                                 int r,
                                                 int grandparent,
                                                 int head,
                                                 int modifier,
                                                 int sibling) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  int sentence_length = sentence->size();
  bool first_child = (head == modifier);
  bool last_child = (sibling == sentence_length || sibling <= 0);

  CHECK_NE(sibling, 0) << "Currently, last child is encoded as s = -1.";

  // Relative position of the grandparent, head and modifier.
  uint8_t direction_code_gh; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_hs; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_gs; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code; // 0x0, 0x1, or 0x2 (see three cases below).

  if (head < grandparent) {
    direction_code_gh = 0x0;
  } else {
    direction_code_gh = 0x1;
  }

  if (sibling < head) {
    direction_code_hs = 0x0;
  } else {
    direction_code_hs = 0x1;
  }

  if (sibling < grandparent) {
    direction_code_gs = 0x0;
  } else {
    direction_code_gs = 0x1;
  }

  if (direction_code_gh == direction_code_hs) {
    direction_code = 0x0; // Pointing in the same direction: g - h - m - s.
  } else if (direction_code_hs != direction_code_gs) {
    direction_code = 0x1; // Zig-zag inwards: g - s - m - h.
  } else {
    direction_code = 0x2; // Non-projective: s - m - g - h or s - g - m - h.
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, GWID, SWID;
  uint8_t HPID, MPID, GPID, SPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  GWID = (*word_ids)[grandparent];
  HWID = (*word_ids)[head];
  MWID = first_child? TOKEN_START : (*word_ids)[modifier];
  SWID = last_child? TOKEN_STOP : (*word_ids)[sibling];
  GPID = (*pos_ids)[grandparent];
  HPID = (*pos_ids)[head];
  MPID = first_child? TOKEN_START : (*pos_ids)[modifier];
  SPID = last_child? TOKEN_STOP : (*pos_ids)[sibling];

  flags = SemanticFeatureTemplateParts::GRANDSIBL;

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateGrandSibl::COUNT, 256);

  // Add direction information.
  flags |= (direction_code << 6); // 2 more bits.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateGrandSibl::BIAS, flags);
  AddFeature(fkey, features);

  // Quadruplet POS feature.
  fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateGrandSibl::GP_HP_MP_SP, flags, GPID, HPID, MPID, SPID);
  AddFeature(fkey, features);

  // Quadruplet unilexical features.
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateGrandSibl::GW_HP_MP_SP, flags, GWID, HPID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateGrandSibl::GP_HW_MP_SP, flags, HWID, GPID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateGrandSibl::GP_HP_MW_SP, flags, MWID, GPID, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateGrandSibl::GP_HP_MP_SW, flags, SWID, GPID, HPID, MPID);
  AddFeature(fkey, features);

  if (FLAGS_srl_use_pair_features_grandsibling_conjunctions) {
    if (modifier != head && sentence->IsCoordination(modifier) &&
        sibling > 0 && sibling < sentence->size()) {
      AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::GRANDSIBL_G_S,
                          grandparent, sibling, true, true, features);
    }
  }
}

// Add features for tri-siblings.
void SemanticFeatures::AddTriSiblingFeatures(SemanticInstanceNumeric* sentence,
                                               int r,
                                               int head,
                                               int modifier,
                                               int sibling,
                                               int other_sibling) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  // TODO(afm).
  int sentence_length = sentence->size();
  bool first_child = (head == modifier);
  bool last_child = (other_sibling == sentence_length || other_sibling <= 0);

  CHECK_LT(sibling, sentence_length);
  CHECK_GT(sibling, 0);
  CHECK_NE(other_sibling, 0) << "Currently, last child is encoded as s = -1.";

  // Direction of attachment.
  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.

  if (other_sibling < head) {
    direction_code = 0x0;
  } else {
    direction_code = 0x1;
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, SWID, TWID;
  uint8_t HPID, MPID, SPID, TPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = (*word_ids)[head];
  MWID = first_child? TOKEN_START : (*word_ids)[modifier];
  SWID = (*word_ids)[sibling];
  TWID = last_child? TOKEN_STOP : (*word_ids)[other_sibling];
  HPID = (*pos_ids)[head];
  MPID = first_child? TOKEN_START : (*pos_ids)[modifier];
  SPID = (*pos_ids)[sibling];
  TPID = last_child? TOKEN_STOP : (*pos_ids)[other_sibling];

  flags = SemanticFeatureTemplateParts::TRISIBL;

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateTriSibl::COUNT, 256);

  // Add direction information.
  flags |= (direction_code << 6); // 1 more bit.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateTriSibl::BIAS, flags);
  AddFeature(fkey, features);

  // Quadruplet POS feature.
  fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateTriSibl::HP_MP_SP_TP, flags, HPID, MPID, SPID, TPID);
  AddFeature(fkey, features);

  // Quadruplet unilexical features.
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateTriSibl::HW_MP_SP_TP, flags, HWID, MPID, SPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateTriSibl::HP_MW_SP_TP, flags, MWID, HPID, SPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateTriSibl::HP_MP_SW_TP, flags, SWID, HPID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(SemanticFeatureTemplateTriSibl::HP_MP_SP_TW, flags, TWID, HPID, MPID, SPID);

  // Triplet POS features.
  fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateTriSibl::HP_MP_TP, flags, HPID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateTriSibl::MP_SP_TP, flags, MPID, SPID, TPID);
  AddFeature(fkey, features);

  // Triplet unilexical features.
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateTriSibl::HW_MP_TP, flags, HWID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateTriSibl::HP_MW_TP, flags, MWID, HPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateTriSibl::HP_MP_TW, flags, TWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateTriSibl::MW_SP_TP, flags, MWID, SPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateTriSibl::MP_SW_TP, flags, SWID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateTriSibl::MP_SP_TW, flags, TWID, MPID, SPID);
  AddFeature(fkey, features);

  // Pairwise POS features.
  fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateTriSibl::MP_TP, flags, MPID, TPID);
  AddFeature(fkey, features);

  // Pairwise unilexical features.
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateTriSibl::MW_TP, flags, MWID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateTriSibl::MP_TW, flags, TWID, MPID);
  AddFeature(fkey, features);
}
#endif


#if 0
// General function to add features for a pair of words (arcs, sibling words,
// etc.) No lemma and morpho-syntactic feature information are used.
// The features are very similar to the ones used in McDonald et al. MSTParser.
void SemanticFeatures::AddWordPairFeaturesMST(SemanticInstanceNumeric* sentence,
                                                int pair_type,
                                                int head,
                                                int modifier,
                                                BinaryFeatures *features) {
  int sentence_length = sentence->size();
  // True if labeled dependency parsing.
  bool labeled =
      static_cast<SemanticOptions*>(pipe_->GetOptions())->labeled();

  // True if using morpho-syntactic features.
  bool use_morphological_features = false;

  // Only 4 bits are allowed in feature_type.
  CHECK_LT(pair_type, 16);
  CHECK_GE(pair_type, 0);
  uint8_t feature_type = pair_type;

  int left_position, right_position;
  int arc_length;

  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t binned_length_code; // Binned arc length.

  if (modifier < head) {
    left_position = modifier;
    right_position = head;
    direction_code = 0x0;
  } else {
    left_position = head;
    right_position = modifier;
    direction_code = 0x1;
  }
  arc_length = right_position - left_position;

  // 7 possible values for binned_length_code (3 bits)
  if (arc_length > 40) {
    binned_length_code = 0x6;
  } else if (arc_length > 30) {
    binned_length_code = 0x5;
  } else if (arc_length > 20) {
    binned_length_code = 0x4;
  } else if (arc_length > 10) {
    binned_length_code = 0x3;
  } else if (arc_length > 5) {
    binned_length_code = 0x2;
  } else if (arc_length > 2) {
    binned_length_code = 0x1;
  } else {
    binned_length_code = 0x0;
  }

  // Mode codeword.
  // mode = 0: no extra info;
  // mode = 1: direction of attachment.
  uint8_t mode;

  // Codewords for accommodating word/POS information.
  // TODO: add morpho-syntactic features!!
  uint16_t HWID, MWID;
  uint16_t HFID, MFID;
  uint8_t HPID, MPID, BPID;
  uint8_t pHPID, pMPID, nHPID, nMPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();
  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetPosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = (*word_ids)[head];
  MWID = (*word_ids)[modifier];
  HPID = (*pos_ids)[head];
  MPID = (*pos_ids)[modifier];

  // Contextual information.
  pHPID = (head > 0)? (*pos_ids)[head - 1] : TOKEN_START;
  pMPID = (modifier > 0)? (*pos_ids)[modifier - 1] : TOKEN_START;
  nHPID = (head < sentence_length - 1)? (*pos_ids)[head + 1] : TOKEN_STOP;
  nMPID = (modifier < sentence_length - 1)?
    (*pos_ids)[modifier + 1] : TOKEN_STOP;

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplateArc::COUNT, 256);

  for (mode = 0; mode < 2; ++mode) {
    // Code for feature type, mode and extended mode.
    flags = feature_type;
    flags |= (mode << 4); // 1 more bit.

    if (mode == 1) {
      flags |= (direction_code << 5); // 1 more bit.
      flags |= (binned_length_code << 6); // 3 more bits.
    }

    // Bias feature.
    fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplateArc::BIAS, flags);
    AddFeature(fkey, features);

    // POS features.
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::HP, flags, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplateArc::MP, flags, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplateArc::HP_MP, flags, HPID, MPID);
    AddFeature(fkey, features);

    // Lexical/Bilexical features.
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::HW, flags, HWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplateArc::MW, flags, MWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HW_MW, flags, HWID, MWID);
    AddFeature(fkey, features);

    // Features involving words and POS.
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HWP, flags, HWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::MWP, flags, MWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_MW, flags, MWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HW_MP, flags, HWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(SemanticFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
    AddFeature(fkey, features);

    // Morpho-syntactic features.
    if (use_morphological_features) {
      for (int j = 0; j < sentence->GetNumMorphFeatures(head); ++j) {
        HFID = sentence->GetMorphFeature(head, j);
        CHECK_LT(HFID, 0xfff);
        if (j >= 0xf) {
          LOG(WARNING) << "Too many morphological features (" << j << ")";
          HFID = (HFID << 4) | ((uint16_t) 0xf);
        } else {
          HFID = (HFID << 4) | ((uint16_t) j);
        }
        for (int k = 0; k < sentence->GetNumMorphFeatures(modifier); ++k) {
          MFID = sentence->GetMorphFeature(modifier, k);
          CHECK_LT(MFID, 0xfff);
          if (k >= 0xf) {
            LOG(WARNING) << "Too many morphological features (" << k << ")";
            MFID = (MFID << 4) | ((uint16_t) 0xf);
          } else {
            MFID = (MFID << 4) | ((uint16_t) k);
          }
          // Morphological features.
          fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplateArc::HF_MF, flags, HFID, MFID);
          AddFeature(fkey, features);

          // Morphological features conjoined with POS.
          fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HF_MP, flags, HFID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateArc::HF_MFP, flags, HFID, MFID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplateArc::HP_MF, flags, MFID, HPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWP(SemanticFeatureTemplateArc::HFP_MF, flags, HFID, MFID, HPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HFP_MP, flags, HFID, HPID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WPP(SemanticFeatureTemplateArc::HP_MFP, flags, MFID, HPID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWPP(SemanticFeatureTemplateArc::HFP_MFP, flags, HFID, MFID, HPID, MPID);
          AddFeature(fkey, features);
        }
      }
    }

    // Contextual features.
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(SemanticFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPPP(SemanticFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
    AddFeature(fkey, features);

    // In-between features.
    set<int> BPIDs;
    for (int i = left_position + 1; i < right_position; ++i) {
      BPID = (*pos_ids)[i];
      if (BPIDs.find(BPID) == BPIDs.end()) {
        BPIDs.insert(BPID);

        // POS in the middle.
        fkey = encoder_.CreateFKey_PPP(SemanticFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
        AddFeature(fkey, features);
      }
    }
    BPIDs.clear();
  }
}
#endif



void SemanticFeatures::AddPredicateFeatures(SemanticInstanceNumeric* sentence,
                                            bool labeled,
                                            uint8_t feature_type,
                                            int r,
                                            int predicate,
                                            int predicate_id) {
  //LOG(INFO) << "Adding arc features";

  SemanticOptions *options = static_cast<class SemanticPipe*>(pipe_)->
      GetSemanticOptions();

  //CHECK(!input_features_[r]);
  //BinaryFeatures *features = new BinaryFeatures;
  //input_features_[r] = features;
  BinaryFeatures *features = NULL;
  if (labeled) {
    features = input_labeled_features_[r];
  } else {
    features = input_features_[r];
  }

  int sentence_length = sentence->size();
  bool use_dependency_features = options->use_dependency_syntactic_features();
  bool use_contextual_dependency_features = use_dependency_features;
  bool use_contextual_features = FLAGS_srl_use_contextual_features;

  // Only 4 bits are allowed in feature_type.
  //uint8_t feature_type = SemanticFeatureTemplateParts::PREDICATE;
  CHECK_LT(feature_type, 16);
  CHECK_GE(feature_type, 0);

  // List of predicate dependents, left and right siblings.
  const vector<int> &predicate_dependents = sentence->GetModifiers(predicate);

  // Mode codeword.
  // mode = 0: no extra info;
  uint8_t mode;

  // Codewords for accommodating word/POS information.
  uint16_t HWID, HLID, HSID;
  uint8_t HPID;
  uint16_t hHWID, hHLID;
  uint8_t hHPID;
  uint8_t pHPID, nHPID;
  uint16_t ldMWID, rdMWID, lMWID, rMWID;
  uint8_t ldMPID, rdMPID, lMPID, rMPID;
  uint8_t HRID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();
  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetPosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = (*word_ids)[predicate];
  HPID = (*pos_ids)[predicate];

  // Predicate lemma/sense.
  HLID = sentence->GetLemmaId(predicate);
  CHECK_GE(predicate_id, 0);
  CHECK_LT(predicate_id, 0xffff);
  HSID = predicate_id;

  // Predicate dependency relation.
  if (use_dependency_features) {
    HRID = sentence->GetRelationId(predicate);
  } else {
    HRID = 0x0;
  }

  // Contextual dependency information.
  int head = sentence->GetHead(predicate);
  if (use_dependency_features) {
    hHPID = (*pos_ids)[head];
    hHWID = (*word_ids)[head];
    hHLID = sentence->GetLemmaId(head);
  } else {
    hHPID = 0x0;
    hHWID = 0x0;
    hHLID = 0x0;
  }

  // Maximum is 255 feature templates.
  CHECK_LT(SemanticFeatureTemplatePredicate::COUNT, 256);

  // TODO(atm): remove this for loop.
  for (mode = 0; mode < 1; ++mode) {
    // Code for feature type, mode and extended mode.
    flags = feature_type;
    flags |= (mode << 4); // 1 more bit.

    // Bias feature.
    fkey = encoder_.CreateFKey_NONE(SemanticFeatureTemplatePredicate::BIAS, flags);
    AddFeature(fkey, features);

    // POS/word/lemma/predicate features.
    fkey = encoder_.CreateFKey_P(SemanticFeatureTemplatePredicate::HP, flags, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplatePredicate::HW, flags, HWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplatePredicate::HL, flags, HLID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(SemanticFeatureTemplatePredicate::HS, flags, HSID);
    AddFeature(fkey, features);

    if (use_dependency_features) {
      // Predicate dependency relation.
      fkey = encoder_.CreateFKey_P(SemanticFeatureTemplatePredicate::HR, flags, HRID);
      AddFeature(fkey, features);

      // Predicate head features.
      fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplatePredicate::HW_hHW, flags, HWID, hHWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HW_hHP, flags, HWID, hHPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HP_hHW, flags, hHWID, HPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplatePredicate::HP_hHP, flags, HPID, hHPID);
      AddFeature(fkey, features);
    }

    if (use_contextual_dependency_features) {
      // Contextual dependency features: predicate dependents.
      for (int k = 0; k < predicate_dependents.size(); ++k) {
        int m = predicate_dependents[k];
        uint16_t bdHWID = (*word_ids)[m];
        uint8_t bdHPID = (*pos_ids)[m];
        uint8_t bdHRID = sentence->GetRelationId(m);
        fkey = encoder_.CreateFKey_WW(SemanticFeatureTemplatePredicate::HW_bdHW, flags, HWID, bdHWID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HW_bdHP, flags, HWID, bdHPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HW_bdHR, flags, HWID, bdHRID);
        //fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HW_bdHR, flags, HWID, bdHPID); // Submitted results.
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HP_bdHW, flags, bdHWID, HPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplatePredicate::HP_bdHP, flags, HPID, bdHPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_PP(SemanticFeatureTemplatePredicate::HP_bdHR, flags, HPID, bdHRID);
        //fkey = encoder_.CreateFKey_WP(SemanticFeatureTemplatePredicate::HP_bdHR, flags, HWID, bdHRID); // Submitted results.
        AddFeature(fkey, features);
      }
    }

    if (use_contextual_features) {
      // Contextual features.
      // TODO(atm): implement this.
      //CHECK(false);
    }
  }
}
