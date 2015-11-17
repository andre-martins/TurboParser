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

#include "DependencyLabelerPipe.h"
#include "DependencyLabelerFeatures.h"
//#include "DependencyLabelerPart.h"
#include "DependencyLabelerFeatureTemplates.h"
#include <set>

// Flags for specific options in feature definitions.
// Note: this will be deprecated soon.
// Note 2: these flags don't get saved in the model file!!! So we need to call
// them at test time too.
// TODO: deprecate this.
DEFINE_int32(dependency_token_context, 1,
             "Size of the context in token features.");
DEFINE_bool(use_dependency_word_pair_features, true,
            "True for using intensive word pair features");
DEFINE_bool(use_dependency_sibling_features, true,
            "True for using sibling features");
DEFINE_bool(use_dependency_grandparent_features, true,
            "True for using grandparent features");
DEFINE_bool(use_dependency_span_features, true,
            "True for using span features");

// Add arc-factored features including lemmas and morpho-syntactic
// feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.

void DependencyLabelerFeatures::AddArcFeatures(
  DependencyInstanceNumeric *sentence,
  const std::vector<std::vector<int> > &descendents,
  const std::vector<std::vector<int> > &siblings,
  int modifier) {
  CHECK(!input_arc_features_[modifier]);
  BinaryFeatures *features = new BinaryFeatures;
  input_arc_features_[modifier] = features;

  const std::vector<int> &heads = sentence->GetHeads();
  int head = heads[modifier];

  if (FLAGS_use_dependency_word_pair_features) {
#if 0
    AddWordPairFeaturesMST(sentence, DependencyLabelerFeatureTemplateParts::ARC,
                           head, modifier, features);
#else
    AddWordPairFeatures(sentence, DependencyLabelerFeatureTemplateParts::ARC,
                        head, modifier, true, true, features);
#endif
  }

  AddArcSiblingFeatures(sentence, descendents, head, modifier,
                        siblings[head],
                        features);
}

// Add asibling features.
void DependencyLabelerFeatures::AddSiblingFeatures(
  DependencyInstanceNumeric* sentence,
  const std::vector<std::vector<int> > &descendents,
  const std::vector<std::vector<int> > &siblings,
  int head,
  int sibling_index) {
  CHECK(!input_sibling_features_[head][sibling_index]);
  BinaryFeatures *features = new BinaryFeatures;
  input_sibling_features_[head][sibling_index] = features;

  uint64_t fkey;
  uint8_t flags = 0x0;
  flags |= DependencyLabelerFeatureTemplateParts::SIBLING;

  int modifier = -1;
  int sibling = -1;
  if (sibling_index < siblings[head].size()) {
    modifier = siblings[head][sibling_index];
  }
  if (sibling_index > 0) {
    sibling = siblings[head][sibling_index - 1];
  }

  // 0x1 if right attachment, 0x0 otherwise, 0x2 if head in the middle.
  uint8_t direction_code;
  // modifier = -1, sibling = 2, head = 3.
  if (sibling < 0) {
    if (modifier < head) {
      direction_code = 0x0;
    } else {
      direction_code = 0x1;
    }
  } else if (modifier < 0) {
    if (sibling < head) {
      direction_code = 0x0;
    } else {
      direction_code = 0x1;
    }
  } else {
    if (modifier < head) {
      CHECK_LT(sibling, head);
      direction_code = 0x0;
    } else if (sibling > head) {
      direction_code = 0x1;
    } else {
      CHECK_LT(sibling, head);
      CHECK_GT(modifier, head);
      direction_code = 0x2;
    }
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID, SWID;
  uint8_t HPID, MPID, SPID;

  // Maximum is 255 feature templates.
  //LOG(INFO) << DependencyFeatureTemplateArc::COUNT;
  CHECK_LT(DependencyLabelerFeatureTemplateSibling::COUNT, 256);

  uint8_t original_flags = flags;
  flags |= (direction_code << 4); // 2 more bits.

  // Words/POS.
  HWID = sentence->GetFormId(head);
  MWID = (modifier >= 0) ? sentence->GetFormId(modifier) : TOKEN_STOP;
  SWID = (sibling >= 0) ? sentence->GetFormId(sibling) : TOKEN_START;
  HPID = sentence->GetCoarsePosId(head);
  MPID = (modifier >= 0) ? sentence->GetCoarsePosId(modifier) : TOKEN_STOP;
  SPID = (sibling >= 0) ? sentence->GetCoarsePosId(sibling) : TOKEN_START;

#if 0
  // Bias feature without the direction flags.
  fkey = encoder_.CreateFKey_NONE(DependencyLabelerFeatureTemplateSibling::BIAS_NO_DIRECTION, original_flags);
  AddFeature(fkey, features);
#endif

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(DependencyLabelerFeatureTemplateSibling::BIAS, flags);
  AddFeature(fkey, features);

  // POS triplet.
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateSibling::HP_MP_SP, flags,
                                 HPID, MPID, SPID);
  AddFeature(fkey, features);

  // Unilexical features.
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateSibling::HW_MP_SP, flags,
                                 HWID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateSibling::HP_MW_SP, flags,
                                 MWID, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateSibling::HP_MP_SW, flags,
                                 SWID, HPID, MPID);
  AddFeature(fkey, features);
}

//#define PRINT_INFO

// General function to add features for a pair of words (arcs, sibling words,
// etc.) Can optionally use lemma and morpho-syntactic feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyLabelerFeatures::AddArcSiblingFeatures(
  DependencyInstanceNumeric* sentence,
  const std::vector<std::vector<int> > &descendents,
  int head,
  int modifier,
  const std::vector<int> &siblings,
  BinaryFeatures *features) {
  int sentence_length = sentence->size();

  // Only 4 bits are allowed in feature_type.
  uint8_t feature_type = DependencyLabelerFeatureTemplateParts::ARC_SIBLINGS;
  CHECK_LT(feature_type, 16);
  CHECK_GE(feature_type, 0);

  const std::vector<int> &heads = sentence->GetHeads();
  int grandparent = (head <= 0) ? -1 : heads[head];

  int num_modifier_descendents = descendents[modifier].size();
  int leftmost_modifier_descendent = descendents[modifier][0];
  int rightmost_modifier_descendent =
    descendents[modifier][num_modifier_descendents - 1];

#ifdef PRINT_INFO
  TokenDictionary *dictionary = static_cast<DependencyLabelerPipe*>(pipe_)->
    GetDependencyDictionary()->GetTokenDictionary();
  std::string span_text = "";
  for (int i = 0; i < descendents[modifier].size(); ++i) {
    span_text += dictionary->
      GetFormName(sentence->GetFormId(descendents[modifier][i])) + " ";
  }
  LOG(INFO) << "Span headed by "
    << dictionary->GetFormName(sentence->GetFormId(modifier))
    << ": " << span_text;
#endif

  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  int left_position, right_position;
  if (modifier < head) {
    left_position = modifier;
    right_position = head;
    direction_code = 0x0;
  } else {
    left_position = head;
    right_position = modifier;
    direction_code = 0x1;
  }

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID;
  uint8_t HPID, MPID;
  uint16_t pSWID, nSWID;
  uint8_t pSPID, nSPID;
  uint16_t GWID;
  uint8_t GPID;
  uint16_t liDWID, riDWID, loDWID, roDWID;
  uint8_t liDPID, riDPID, loDPID, roDPID;

  // Maximum is 255 feature templates.
  //LOG(INFO) << DependencyFeatureTemplateArc::COUNT;
  CHECK_LT(DependencyLabelerFeatureTemplateArcSiblings::COUNT, 256);

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HWID = sentence->GetFormId(head);
  MWID = sentence->GetFormId(modifier);
  HPID = sentence->GetCoarsePosId(head);
  MPID = sentence->GetCoarsePosId(modifier);

  // Codeword for number of left and right head dependents (8 bits).
  uint8_t head_dependents_code = 0;
  uint8_t head_other_side_dependents_code = 0;
  uint8_t index_modifier_code = 0;
  int num_left_modifiers = 0;
  int num_right_modifiers = 0;
  int index_modifier = -1;
  for (int k = 0; k < siblings.size(); ++k) {
    int m = siblings[k];
    if (m == modifier) index_modifier = k;
    if (m < head) {
      ++num_left_modifiers;
    } else {
      ++num_right_modifiers;
    }
  }
  CHECK_GE(index_modifier, 0);

  int previous_sibling = -1;
  int next_sibling = -1;
  if (modifier < head) {
    // This is a left modifier.
    if (index_modifier < num_left_modifiers - 1) {
      previous_sibling = siblings[index_modifier + 1];
    }
    if (index_modifier > 0) {
      next_sibling = siblings[index_modifier - 1];
    }
  } else {
    // This is a right modifier.
    if (index_modifier > num_left_modifiers) {
      previous_sibling = siblings[index_modifier - 1];
    }
    if (index_modifier < siblings.size() - 1) {
      next_sibling = siblings[index_modifier + 1];
    }
  }

  // Get the modifier span left/rightmost positiion.
  int left_inside_word = -1;
  int right_inside_word = -1;
  int left_outside_word = -1;
  int right_outside_word = -1;

  left_inside_word = leftmost_modifier_descendent;
  right_inside_word = rightmost_modifier_descendent;
  if (left_inside_word > 1) left_outside_word = left_inside_word - 1;
  if (right_inside_word < sentence_length - 1) {
    right_outside_word = right_inside_word + 1;
  }
  // If span touches the head word, mark as such.
  if (left_outside_word == head) left_outside_word = -1;
  if (right_outside_word == head) right_outside_word = -1;

#ifdef PRINT_INFO
  LOG(INFO) << dictionary->GetFormName(sentence->GetFormId(head))
    << " -> "
    << dictionary->GetFormName(sentence->GetFormId(modifier));
  LOG(INFO) << "Left inside word: "
    << dictionary->GetFormName(sentence->GetFormId(left_inside_word));
  if (left_outside_word < 0) {
    LOG(INFO) << "Left outside word: NULL";
  } else {
    LOG(INFO) << "Left outside word: "
      << dictionary->GetFormName(sentence->GetFormId(left_outside_word));
  }
  LOG(INFO) << "Right inside word: "
    << dictionary->GetFormName(sentence->GetFormId(right_inside_word));
  if (right_outside_word < 0) {
    LOG(INFO) << "Right outside word: NULL";
  } else {
    LOG(INFO) << "Right outside word: "
      << dictionary->GetFormName(sentence->GetFormId(right_outside_word));
  }
#endif

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  pSWID = (previous_sibling < 0) ? TOKEN_START : (*word_ids)[previous_sibling];
  pSPID = (previous_sibling < 0) ? TOKEN_START : (*pos_ids)[previous_sibling];
  nSWID = (next_sibling < 0) ? TOKEN_STOP : (*word_ids)[next_sibling];
  nSPID = (next_sibling < 0) ? TOKEN_STOP : (*pos_ids)[next_sibling];
  GWID = (grandparent < 0) ? TOKEN_START : (*word_ids)[grandparent];
  GPID = (grandparent < 0) ? TOKEN_START : (*pos_ids)[grandparent];
  liDWID = (left_inside_word < 0) ?
    TOKEN_START : (*word_ids)[left_inside_word];
  liDPID = (left_inside_word < 0) ?
    TOKEN_START : (*pos_ids)[left_inside_word];
  loDWID = (left_outside_word < 0) ?
    TOKEN_START : (*word_ids)[left_outside_word];
  loDPID = (left_outside_word < 0) ?
    TOKEN_START : (*pos_ids)[left_outside_word];
  riDWID = (right_inside_word < 0) ?
    TOKEN_STOP : (*word_ids)[right_inside_word];
  riDPID = (right_inside_word < 0) ?
    TOKEN_STOP : (*pos_ids)[right_inside_word];
  roDWID = (right_outside_word < 0) ?
    TOKEN_STOP : (*word_ids)[right_outside_word];
  roDPID = (right_outside_word < 0) ?
    TOKEN_STOP : (*pos_ids)[right_outside_word];

  CHECK_EQ(num_left_modifiers + num_right_modifiers, siblings.size());
  if (modifier < head) {
    // This is a left modifier.
    index_modifier = num_left_modifiers - index_modifier; // 1, 2, 3,...
  } else {
    // This is a right modifier.
    index_modifier = index_modifier - num_left_modifiers + 1; // 1, 2, 3,...
  }
  CHECK_GT(index_modifier, 0);

  // Truncate the number of left/right modifiers to 15 (4 bits).
  if (num_left_modifiers > 0xf) num_left_modifiers = 0xf;
  if (num_right_modifiers > 0xf) num_right_modifiers = 0xf;

  // Truncate the index of the modifier to 255 (8 bits).
  if (index_modifier > 0xff) index_modifier = 0xff;

  head_dependents_code = num_right_modifiers; // 4 bits.
  head_dependents_code |= (num_left_modifiers << 4); // 4 more bits.

  uint8_t last_modifier_code = 0x0; // 0x1 if last modifier, 0x0 otherwise.
  if (modifier < head) {
    // This is a left modifier.
    head_other_side_dependents_code = num_right_modifiers; // 4 bits.
    if (index_modifier == num_left_modifiers) last_modifier_code = 0x1;
  } else {
    // This is a right modifier.
    head_other_side_dependents_code = (num_left_modifiers << 4); // 4 bits.
    if (index_modifier == num_right_modifiers) last_modifier_code = 0x1;
  }

  index_modifier_code = index_modifier;

  // Code for feature type.
  flags = feature_type; // 4 bits.
  flags |= (direction_code << 4); // 1 more bit.
  //flags |= (last_modifier_code << 5); // 1 more bit [Hurts performance].

  /////////////////////////////////////////////////////////////////////////////
  // Dependency features.
  // Everything goes with direction flags.
  /////////////////////////////////////////////////////////////////////////////
  // Head dependent features.
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArcSiblings::HMD, flags, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HD_HMD, flags, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HoD_HMD, flags, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);

  // Head dependent features conjoined with head/modifier word and POS.
  fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HW_HMD, flags, HWID, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::MW_HMD, flags, MWID, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HP_HMD, flags, HPID, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::MP_HMD, flags, MPID, index_modifier_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_HMD, flags, HWID, MWID, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_HMD, flags, HWID, MPID, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_HMD, flags, MWID, HPID, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_HMD, flags, HPID, MPID, index_modifier_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_HoD_HMD, flags, HWID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::MW_HoD_HMD, flags, MWID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_HoD_HMD, flags, HPID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::MP_HoD_HMD, flags, MPID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_HoD_HMD, flags, HWID, MWID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_HoD_HMD, flags, HWID, MPID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_HoD_HMD, flags, MWID, HPID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_HoD_HMD, flags, HPID, MPID, head_other_side_dependents_code, index_modifier_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_HD_HMD, flags, HWID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::MW_HD_HMD, flags, MWID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_HD_HMD, flags, HPID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::MP_HD_HMD, flags, MPID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_HD_HMD, flags, HWID, MWID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_HD_HMD, flags, HWID, MPID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_HD_HMD, flags, MWID, HPID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_HD_HMD, flags, HPID, MPID, head_dependents_code, index_modifier_code);
  AddFeature(fkey, features);

  if (FLAGS_use_dependency_sibling_features) {
    // Previous/next sibling features (conjoined with the modifier index).

    // Bilexical.
#if 0
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_pSP_HMD, flags, HWID, MWID, pSPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_pSW_HMD, flags, HWID, pSWID, MPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_pSW_HMD, flags, MWID, pSWID, HPID, index_modifier_code);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_nSP_HMD, flags, HWID, MWID, nSPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_nSW_HMD, flags, HWID, nSWID, MPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_nSW_HMD, flags, MWID, nSWID, HPID, index_modifier_code);
    AddFeature(fkey, features);
#endif

    // Unilexical.
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_pSP_HMD, flags, HWID, MPID, pSPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_pSP_HMD, flags, MWID, HPID, pSPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_pSW_HMD, flags, pSWID, HPID, MPID, index_modifier_code);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_nSP_HMD, flags, HWID, MPID, nSPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_nSP_HMD, flags, MWID, HPID, nSPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_nSW_HMD, flags, nSWID, HPID, MPID, index_modifier_code);
    AddFeature(fkey, features);

    // POS trigram.
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_pSP_HMD, flags, HPID, MPID, pSPID, index_modifier_code);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_nSP_HMD, flags, HPID, MPID, nSPID, index_modifier_code);
    AddFeature(fkey, features);

    // Previous/next sibling features (not conjoined with the modifier index).

    // Bilexical.
#if 0
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_pSP, flags, HWID, MWID, pSPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_pSW, flags, HWID, pSWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_pSW, flags, MWID, pSWID, HPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_nSP, flags, HWID, MWID, nSPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_nSW, flags, HWID, nSWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_nSW, flags, MWID, nSWID, HPID);
    AddFeature(fkey, features);
#endif

    // Unilexical.
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_pSP, flags, HWID, MPID, pSPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_pSP, flags, MWID, HPID, pSPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_pSW, flags, pSWID, HPID, MPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_nSP, flags, HWID, MPID, nSPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_nSP, flags, MWID, HPID, nSPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_nSW, flags, nSWID, HPID, MPID);
    AddFeature(fkey, features);

    // POS trigram.
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_pSP, flags, HPID, MPID, pSPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_nSP, flags, HPID, MPID, nSPID);
    AddFeature(fkey, features);

#if 0
    // Previous/next sibling features (pairwise modifier-sibling).
    //LOG(INFO) << head << " " << modifier << " " << previous_sibling << " " << next_sibling << " " << sentence_length;
    int s = (previous_sibling < 0) ? 0 : previous_sibling;
    AddWordPairFeatures(sentence, DependencyLabelerFeatureTemplateParts::ARC_PREVIOUS_SIBLING,
                        s, modifier, true, true, features);

    s = (next_sibling < 0) ? 0 : next_sibling;
    AddWordPairFeatures(sentence, DependencyLabelerFeatureTemplateParts::ARC_NEXT_SIBLING,
                        modifier, s, true, true, features);
#endif
  }

  if (FLAGS_use_dependency_grandparent_features) {
    // Grandparent features (conjoined with the modifier index).

    // Bilexical.
#if 0
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_GP_HMD, flags, HWID, MWID, GPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_GW_HMD, flags, HWID, GWID, MPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_GW_HMD, flags, MWID, GWID, HPID, index_modifier_code);
    AddFeature(fkey, features);
#endif

    // Unilexical.
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_GP_HMD, flags, HWID, MPID, GPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_GP_HMD, flags, MWID, HPID, GPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_GW_HMD, flags, GWID, HPID, MPID, index_modifier_code);
    AddFeature(fkey, features);

    // POS trigram.
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_GP_HMD, flags, HPID, MPID, GPID, index_modifier_code);
    AddFeature(fkey, features);

    // Grandparent features (not conjoined with the modifier index).

    // Bilexical.
#if 0
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MW_GP, flags, HWID, MWID, GPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_GW, flags, HWID, GWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_GW, flags, MWID, GWID, HPID);
    AddFeature(fkey, features);
#endif

    // Unilexical.
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_MP_GP, flags, HWID, MPID, GPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MW_GP, flags, MWID, HPID, GPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_GW, flags, GWID, HPID, MPID);
    AddFeature(fkey, features);

    // POS trigram.
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_MP_GP, flags, HPID, MPID, GPID);
    AddFeature(fkey, features);

#if 0
    // Previous/next sibling features (pairwise modifier-sibling).
    //LOG(INFO) << head << " " << modifier << " " << previous_sibling << " " << next_sibling << " " << sentence_length;
    AddWordPairFeatures(sentence, DependencyLabelerFeatureTemplateParts::ARC_GRANDPARENT,
                        grandparent, modifier, false, false, features);
#endif
  }

  // Span features conjoined with head/modifier word and POS (conjoined with modifier index).
  if (FLAGS_use_dependency_span_features) {
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::liDW_HMD, flags, liDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::liDP_HMD, flags, liDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_liDW_HMD, flags, HWID, liDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_liDP_HMD, flags, HWID, liDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_liDW_HMD, flags, liDWID, HPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_liDP_HMD, flags, HPID, liDPID, index_modifier_code);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::riDW_HMD, flags, riDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::riDP_HMD, flags, riDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_riDW_HMD, flags, HWID, riDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_riDP_HMD, flags, HWID, riDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_riDW_HMD, flags, riDWID, HPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_riDP_HMD, flags, HPID, riDPID, index_modifier_code);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::loDW_HMD, flags, loDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::loDP_HMD, flags, loDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_loDW_HMD, flags, HWID, loDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_loDP_HMD, flags, HWID, loDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_loDW_HMD, flags, loDWID, HPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_loDP_HMD, flags, HPID, loDPID, index_modifier_code);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::roDW_HMD, flags, roDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::roDP_HMD, flags, roDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArcSiblings::HW_roDW_HMD, flags, HWID, roDWID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HW_roDP_HMD, flags, HWID, roDPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArcSiblings::HP_roDW_HMD, flags, roDWID, HPID, index_modifier_code);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArcSiblings::HP_roDP_HMD, flags, HPID, roDPID, index_modifier_code);
    AddFeature(fkey, features);

    // Span features conjoined with head/modifier word and POS (not conjoined with modifier index).
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArcSiblings::liDW, flags, liDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArcSiblings::liDP, flags, liDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArcSiblings::HW_liDW, flags, HWID, liDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HW_liDP, flags, HWID, liDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HP_liDW, flags, liDWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HP_liDP, flags, HPID, liDPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArcSiblings::riDW, flags, riDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArcSiblings::riDP, flags, riDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArcSiblings::HW_riDW, flags, HWID, riDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HW_riDP, flags, HWID, riDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HP_riDW, flags, riDWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HP_riDP, flags, HPID, riDPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArcSiblings::loDW, flags, loDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArcSiblings::loDP, flags, loDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArcSiblings::HW_loDW, flags, HWID, loDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HW_loDP, flags, HWID, loDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HP_loDW, flags, loDWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HP_loDP, flags, HPID, loDPID);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArcSiblings::roDW, flags, roDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArcSiblings::roDP, flags, roDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArcSiblings::HW_roDW, flags, HWID, roDWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HW_roDP, flags, HWID, roDPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArcSiblings::HP_roDW, flags, roDWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArcSiblings::HP_roDP, flags, HPID, roDPID);
    AddFeature(fkey, features);
  }

#if 0
  // In-between flags.
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BFLAG, flags, flag_between_verb);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BFLAG, flags, flag_between_punc);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BFLAG, flags, flag_between_coord);
  AddFeature(fkey, features);

  // POS features conjoined with in-between flag.
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_verb);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_punc);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_coord);
  AddFeature(fkey, features);

  set<int> BPIDs;
  set<int> BWIDs;
  for (int i = left_position + 1; i < right_position; ++i) {
    BPID = sentence->GetCoarsePosId(i);
    if (BPIDs.find(BPID) == BPIDs.end()) {
      BPIDs.insert(BPID);

      // POS in the middle.
      fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArc::HW_MW_BP, flags, HWID, MWID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HW_MP_BP, flags, HWID, MPID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HP_MW_BP, flags, MWID, HPID, BPID);
      AddFeature(fkey, features);
    }
    BPIDs.clear();
  }
#endif
}

// General function to add features for a pair of words (arcs, sibling words,
// etc.) Can optionally use lemma and morpho-syntactic feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyLabelerFeatures::AddWordPairFeatures(
  DependencyInstanceNumeric* sentence,
  int pair_type,
  int head,
  int modifier,
  bool use_lemma_features,
  bool use_morphological_features,
  BinaryFeatures *features) {
  int sentence_length = sentence->size();
  bool labeled = true;

  // Only 4 bits are allowed in feature_type.
  CHECK_LT(pair_type, 16);
  CHECK_GE(pair_type, 0);
  uint8_t feature_type = pair_type;

  int max_token_context = FLAGS_dependency_token_context; // 1.

  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t binned_length_code; // Binned arc length.
  uint8_t exact_length_code; // Exact arc length.
  int left_position, right_position;
  if (modifier < head) {
    left_position = modifier;
    right_position = head;
    direction_code = 0x0;
  } else {
    left_position = head;
    right_position = modifier;
    direction_code = 0x1;
  }
  int arc_length = right_position - left_position;

  // 7 possible values for binned_length_code (3 bits).
  exact_length_code = (arc_length > 0xff) ? 0xff : arc_length;
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

  // Codewords for accommodating word/POS information.
  uint16_t HWID, MWID;
  uint16_t HLID, MLID;
  uint16_t HFID, MFID;
  uint8_t HPID, MPID, BPID;
  uint8_t HQID, MQID;
  uint16_t pHWID, pMWID, nHWID, nMWID;
  uint16_t pHLID, pMLID, nHLID, nMLID;
  uint8_t pHPID, pMPID, nHPID, nMPID;
  uint8_t pHQID, pMQID, nHQID, nMQID;
  uint16_t ppHWID, ppMWID, nnHWID, nnMWID;
  uint16_t ppHLID, ppMLID, nnHLID, nnMLID;
  uint8_t ppHPID, ppMPID, nnHPID, nnMPID;
  uint8_t ppHQID, ppMQID, nnHQID, nnMQID;

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
  //LOG(INFO) << DependencyFeatureTemplateArc::COUNT;
  CHECK_LT(DependencyLabelerFeatureTemplateArc::COUNT, 256);

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  HLID = sentence->GetLemmaId(head);
  MLID = sentence->GetLemmaId(modifier);
  HWID = sentence->GetFormId(head);
  MWID = sentence->GetFormId(modifier);
  HPID = sentence->GetCoarsePosId(head);
  MPID = sentence->GetCoarsePosId(modifier);
  HQID = sentence->GetPosId(head);
  MQID = sentence->GetPosId(modifier);

  // Contextual information.
  // Context size = 1:
  pHLID = (head > 0) ? sentence->GetLemmaId(head - 1) : TOKEN_START;
  pMLID = (modifier > 0) ? sentence->GetLemmaId(modifier - 1) : TOKEN_START;
  pHWID = (head > 0) ? sentence->GetFormId(head - 1) : TOKEN_START;
  pMWID = (modifier > 0) ? sentence->GetFormId(modifier - 1) : TOKEN_START;
  pHPID = (head > 0) ? sentence->GetCoarsePosId(head - 1) : TOKEN_START;
  pMPID = (modifier > 0) ? sentence->GetCoarsePosId(modifier - 1) : TOKEN_START;
  pHQID = (head > 0) ? sentence->GetPosId(head - 1) : TOKEN_START;
  pMQID = (modifier > 0) ? sentence->GetPosId(modifier - 1) : TOKEN_START;

  nHLID = (head < sentence_length - 1) ?
    sentence->GetLemmaId(head + 1) : TOKEN_STOP;
  nMLID = (modifier < sentence_length - 1) ?
    sentence->GetLemmaId(modifier + 1) : TOKEN_STOP;
  nHWID = (head < sentence_length - 1) ?
    sentence->GetFormId(head + 1) : TOKEN_STOP;
  nMWID = (modifier < sentence_length - 1) ?
    sentence->GetFormId(modifier + 1) : TOKEN_STOP;
  nHPID = (head < sentence_length - 1) ?
    sentence->GetCoarsePosId(head + 1) : TOKEN_STOP;
  nMPID = (modifier < sentence_length - 1) ?
    sentence->GetCoarsePosId(modifier + 1) : TOKEN_STOP;
  nHQID = (head < sentence_length - 1) ?
    sentence->GetPosId(head + 1) : TOKEN_STOP;
  nMQID = (modifier < sentence_length - 1) ?
    sentence->GetPosId(modifier + 1) : TOKEN_STOP;

  // Context size = 2:
  ppHLID = (head > 1) ? sentence->GetLemmaId(head - 2) : TOKEN_START;
  ppMLID = (modifier > 1) ? sentence->GetLemmaId(modifier - 2) : TOKEN_START;
  ppHWID = (head > 1) ? sentence->GetFormId(head - 2) : TOKEN_START;
  ppMWID = (modifier > 1) ? sentence->GetFormId(modifier - 2) : TOKEN_START;
  ppHPID = (head > 1) ? sentence->GetCoarsePosId(head - 2) : TOKEN_START;
  ppMPID = (modifier > 1) ? sentence->GetCoarsePosId(modifier - 2) : TOKEN_START;
  ppHQID = (head > 1) ? sentence->GetPosId(head - 2) : TOKEN_START;
  ppMQID = (modifier > 1) ? sentence->GetPosId(modifier - 2) : TOKEN_START;

  nnHLID = (head < sentence_length - 2) ?
    sentence->GetLemmaId(head + 2) : TOKEN_STOP;
  nnMLID = (modifier < sentence_length - 2) ?
    sentence->GetLemmaId(modifier + 2) : TOKEN_STOP;
  nnHWID = (head < sentence_length - 2) ?
    sentence->GetFormId(head + 2) : TOKEN_STOP;
  nnMWID = (modifier < sentence_length - 2) ?
    sentence->GetFormId(modifier + 2) : TOKEN_STOP;
  nnHPID = (head < sentence_length - 2) ?
    sentence->GetCoarsePosId(head + 2) : TOKEN_STOP;
  nnMPID = (modifier < sentence_length - 2) ?
    sentence->GetCoarsePosId(modifier + 2) : TOKEN_STOP;
  nnHQID = (head < sentence_length - 2) ?
    sentence->GetPosId(head + 2) : TOKEN_STOP;
  nnMQID = (modifier < sentence_length - 2) ?
    sentence->GetPosId(modifier + 2) : TOKEN_STOP;

  // Code for feature type.
  flags = feature_type; // 4 bits.
  flags |= (direction_code << 4); // 1 more bit.

  // Bias feature (not in EGSTRA).
  fkey = encoder_.CreateFKey_NONE(DependencyLabelerFeatureTemplateArc::BIAS, flags);
  AddFeature(fkey, features);

  /////////////////////////////////////////////////////////////////////////////
  // Token features.
  /////////////////////////////////////////////////////////////////////////////

  // Note: in EGSTRA (but not here), token and token contextual features go
  // without direction flags.
  // Coarse POS features.
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::HP, flags, HPID);
  AddFeature(fkey, features);
  // Fine POS features.
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::HQ, flags, HQID);
  AddFeature(fkey, features);
  // Lexical features.
  fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::HW, flags, HWID);
  AddFeature(fkey, features);
  if (use_lemma_features) {
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::HL, flags, HLID);
    AddFeature(fkey, features);
  }
  // Features involving words and POS.
  fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HWP, flags, HWID, HPID);
  AddFeature(fkey, features);
  // Morpho-syntactic features.
  // Technically should add context here too to match egstra, but I don't think it
  // would add much relevant information.
  if (use_morphological_features) {
    for (int j = 0; j < sentence->GetNumMorphFeatures(head); ++j) {
      HFID = sentence->GetMorphFeature(head, j);
      CHECK_LT(HFID, 0xfff);
      if (j >= 0xf) {
        LOG(WARNING) << "Too many morphological features (" << j << ")";
        HFID = (HFID << 4) | ((uint16_t)0xf);
      } else {
        HFID = (HFID << 4) | ((uint16_t)j);
      }
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::HF, flags, HFID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArc::HWF, flags, HWID, HFID);
      AddFeature(fkey, features);
    }
  }

  // If labeled parsing, features involving the modifier only are still useful,
  // since they will be conjoined with the label.
  if (labeled) {
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::MP, flags, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::MQ, flags, MQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::MW, flags, MWID);
    AddFeature(fkey, features);
    if (use_lemma_features) {
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::ML, flags, MLID);
      AddFeature(fkey, features);
    }
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::MWP, flags, MWID, MPID);
    AddFeature(fkey, features);
    for (int k = 0; k < sentence->GetNumMorphFeatures(modifier); ++k) {
      MFID = sentence->GetMorphFeature(modifier, k);
      CHECK_LT(MFID, 0xfff);
      if (k >= 0xf) {
        LOG(WARNING) << "Too many morphological features (" << k << ")";
        MFID = (MFID << 4) | ((uint16_t)0xf);
      } else {
        MFID = (MFID << 4) | ((uint16_t)k);
      }
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::MF, flags, MFID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArc::MWF, flags, MWID, MFID);
      AddFeature(fkey, features);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Token contextual features.
  /////////////////////////////////////////////////////////////////////////////

  if (max_token_context >= 1) {
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::pHP, flags, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nHP, flags, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::pHQ, flags, pHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nHQ, flags, nHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::pHW, flags, pHWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nHW, flags, nHWID);
    AddFeature(fkey, features);
    if (use_lemma_features) {
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::pHL, flags, pHLID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nHL, flags, nHLID);
      AddFeature(fkey, features);
    }
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::pHWP, flags, pHWID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::nHWP, flags, nHWID, nHPID);
    AddFeature(fkey, features);

    // If labeled parsing, features involving the modifier only are still useful,
    // since they will be conjoined with the label.
    if (labeled) {
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::pMP, flags, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nMP, flags, nMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::pMQ, flags, pMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nMQ, flags, nMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::pMW, flags, pMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nMW, flags, nMWID);
      AddFeature(fkey, features);
      if (use_lemma_features) {
        fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::pML, flags, pMLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nML, flags, nMLID);
        AddFeature(fkey, features);
      }
      fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::pMWP, flags, pMWID, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::nMWP, flags, nMWID, nMPID);
      AddFeature(fkey, features);
    }
  }

  if (max_token_context >= 2) {
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::ppHP, flags, ppHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nnHP, flags, nnHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::ppHQ, flags, ppHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nnHQ, flags, nnHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::ppHW, flags, ppHWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nnHW, flags, nnHWID);
    AddFeature(fkey, features);
    if (use_lemma_features) {
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::ppHL, flags, ppHLID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nnHL, flags, nnHLID);
      AddFeature(fkey, features);
    }
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::ppHWP, flags, ppHWID, ppHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::nnHWP, flags, nnHWID, nnHPID);
    AddFeature(fkey, features);

    // If labeled parsing, features involving the modifier only are still useful,
    // since they will be conjoined with the label.
    if (labeled) {
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::ppMP, flags, ppMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nnMP, flags, nnMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::ppMQ, flags, ppMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::nnMQ, flags, nnMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::ppMW, flags, ppMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nnMW, flags, nnMWID);
      AddFeature(fkey, features);
      if (use_lemma_features) {
        fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::ppML, flags, ppMLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::nnML, flags, nnMLID);
        AddFeature(fkey, features);
      }
      fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::ppMWP, flags, ppMWID, ppMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::nnMWP, flags, nnMWID, nnMPID);
      AddFeature(fkey, features);
    }
  }

  // Contextual bigram and trigram features involving POS.
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::HP_pHP, flags, HPID, pHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_pHP_ppHP, flags, HPID, pHPID, ppHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::HP_nHP, flags, HPID, nHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_nHP_nnHP, flags, HPID, nHPID, nnHPID);
  AddFeature(fkey, features);

  // If labeled parsing, features involving the modifier only are still useful,
  // since they will be conjoined with the label.
  if (labeled) {
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::MP_pMP, flags, MPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::MP_pMP_ppMP, flags, MPID, pMPID, ppMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::MP_nMP, flags, MPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::MP_nMP_nnMP, flags, MPID, nMPID, nnMPID);
    AddFeature(fkey, features);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Dependency features.
  // Everything goes with direction flags and with coarse POS.
  /////////////////////////////////////////////////////////////////////////////

  // POS features.
  fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  // Lexical/Bilexical features.
  fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArc::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);

  // Features involving words and POS.
  fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
  AddFeature(fkey, features);

  // Morpho-syntactic features.
  if (use_morphological_features) {
    for (int j = 0; j < sentence->GetNumMorphFeatures(head); ++j) {
      HFID = sentence->GetMorphFeature(head, j);
      CHECK_LT(HFID, 0xfff);
      if (j >= 0xf) {
        LOG(WARNING) << "Too many morphological features (" << j << ")";
        HFID = (HFID << 4) | ((uint16_t)0xf);
      } else {
        HFID = (HFID << 4) | ((uint16_t)j);
      }
      for (int k = 0; k < sentence->GetNumMorphFeatures(modifier); ++k) {
        MFID = sentence->GetMorphFeature(modifier, k);
        CHECK_LT(MFID, 0xfff);
        if (k >= 0xf) {
          LOG(WARNING) << "Too many morphological features (" << k << ")";
          MFID = (MFID << 4) | ((uint16_t)0xf);
        } else {
          MFID = (MFID << 4) | ((uint16_t)k);
        }
        // Morphological features.
        fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArc::HF_MF, flags, HFID, MFID);
        AddFeature(fkey, features);

        // Morphological features conjoined with POS.
        fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HF_MP, flags, HFID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArc::HF_MFP, flags, HFID, MFID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HP_MF, flags, MFID, HPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArc::HFP_MF, flags, HFID, MFID, HPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HFP_MP, flags, HFID, HPID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HP_MFP, flags, MFID, HPID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArc::HFP_MFP, flags, HFID, MFID, HPID, MPID);
        AddFeature(fkey, features);
      }
    }
  }

  // Contextual features.
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
  AddFeature(fkey, features);

  // Features for adjacent dependencies.
  if (head != 0 && head == modifier - 1) {
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID, 0x1);
    AddFeature(fkey, features);
  } else if (head != 0 && head == modifier + 1) {
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID, 0x1);
    AddFeature(fkey, features);
  }

  // Exact arc length.
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::DIST, flags, exact_length_code);
  AddFeature(fkey, features);

  // Binned arc length.
  for (uint8_t bin = 0; bin <= binned_length_code; ++bin) {
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BIAS, flags, bin);
    AddFeature(fkey, features);
  }

  // POS features conjoined with binned arc length.
  for (uint8_t bin = 0; bin <= binned_length_code; bin++) {
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::HP, flags, HPID, bin);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::MP, flags, MPID, bin);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP, flags, HPID, MPID, bin);
    AddFeature(fkey, features);
  }

  // In-between flags.
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BFLAG, flags, flag_between_verb);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BFLAG, flags, flag_between_punc);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::BFLAG, flags, flag_between_coord);
  AddFeature(fkey, features);

  // POS features conjoined with in-between flag.
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_verb);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_punc);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_coord);
  AddFeature(fkey, features);

  set<int> BPIDs;
  set<int> BWIDs;
  for (int i = left_position + 1; i < right_position; ++i) {
    BPID = sentence->GetCoarsePosId(i);
    if (BPIDs.find(BPID) == BPIDs.end()) {
      BPIDs.insert(BPID);

      // POS in the middle.
      fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArc::HW_MW_BP, flags, HWID, MWID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HW_MP_BP, flags, HWID, MPID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HP_MW_BP, flags, MWID, HPID, BPID);
      AddFeature(fkey, features);
    }
    BPIDs.clear();
  }
}

// General function to add features for a pair of words (arcs, sibling words,
// etc.) No lemma and morpho-syntactic feature information are used.
// The features are very similar to the ones used in McDonald et al. MSTParser.
void DependencyLabelerFeatures::AddWordPairFeaturesMST(
  DependencyInstanceNumeric* sentence,
  int pair_type,
  int head,
  int modifier,
  BinaryFeatures *features) {
  int sentence_length = sentence->size();
  // True if labeled dependency parsing.
  bool labeled = true;

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
  pHPID = (head > 0) ? (*pos_ids)[head - 1] : TOKEN_START;
  pMPID = (modifier > 0) ? (*pos_ids)[modifier - 1] : TOKEN_START;
  nHPID = (head < sentence_length - 1) ? (*pos_ids)[head + 1] : TOKEN_STOP;
  nMPID = (modifier < sentence_length - 1) ?
    (*pos_ids)[modifier + 1] : TOKEN_STOP;

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyLabelerFeatureTemplateArc::COUNT, 256);

  for (mode = 0; mode < 2; ++mode) {
    // Code for feature type, mode and extended mode.
    flags = feature_type;
    flags |= (mode << 4); // 1 more bit.

    if (mode == 1) {
      flags |= (direction_code << 5); // 1 more bit.
      // NOTE: This will overflow the 8-bit size (it may take 9 bits)!!!!!
      // TODO(atm): fix this!!!
      flags |= (binned_length_code << 6); // 3 more bits.
    }

    // Bias feature.
    fkey = encoder_.CreateFKey_NONE(DependencyLabelerFeatureTemplateArc::BIAS, flags);
    AddFeature(fkey, features);

    // POS features.
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::HP, flags, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyLabelerFeatureTemplateArc::MP, flags, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyLabelerFeatureTemplateArc::HP_MP, flags, HPID, MPID);
    AddFeature(fkey, features);

    // Lexical/Bilexical features.
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::HW, flags, HWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyLabelerFeatureTemplateArc::MW, flags, MWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArc::HW_MW, flags, HWID, MWID);
    AddFeature(fkey, features);

    // Features involving words and POS.
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HWP, flags, HWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::MWP, flags, MWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HP_MW, flags, MWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HW_MP, flags, HWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
    AddFeature(fkey, features);

    // Morpho-syntactic features.
    if (use_morphological_features) {
      for (int j = 0; j < sentence->GetNumMorphFeatures(head); ++j) {
        HFID = sentence->GetMorphFeature(head, j);
        CHECK_LT(HFID, 0xfff);
        if (j >= 0xf) {
          LOG(WARNING) << "Too many morphological features (" << j << ")";
          HFID = (HFID << 4) | ((uint16_t)0xf);
        } else {
          HFID = (HFID << 4) | ((uint16_t)j);
        }
        for (int k = 0; k < sentence->GetNumMorphFeatures(modifier); ++k) {
          MFID = sentence->GetMorphFeature(modifier, k);
          CHECK_LT(MFID, 0xfff);
          if (k >= 0xf) {
            LOG(WARNING) << "Too many morphological features (" << k << ")";
            MFID = (MFID << 4) | ((uint16_t)0xf);
          } else {
            MFID = (MFID << 4) | ((uint16_t)k);
          }
          // Morphological features.
          fkey = encoder_.CreateFKey_WW(DependencyLabelerFeatureTemplateArc::HF_MF, flags, HFID, MFID);
          AddFeature(fkey, features);

          // Morphological features conjoined with POS.
          fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HF_MP, flags, HFID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArc::HF_MFP, flags, HFID, MFID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WP(DependencyLabelerFeatureTemplateArc::HP_MF, flags, MFID, HPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWP(DependencyLabelerFeatureTemplateArc::HFP_MF, flags, HFID, MFID, HPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HFP_MP, flags, HFID, HPID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WPP(DependencyLabelerFeatureTemplateArc::HP_MFP, flags, MFID, HPID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWPP(DependencyLabelerFeatureTemplateArc::HFP_MFP, flags, HFID, MFID, HPID, MPID);
          AddFeature(fkey, features);
        }
      }
    }

    // Contextual features.
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyLabelerFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPPP(DependencyLabelerFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
    AddFeature(fkey, features);

    // In-between features.
    set<int> BPIDs;
    for (int i = left_position + 1; i < right_position; ++i) {
      BPID = (*pos_ids)[i];
      if (BPIDs.find(BPID) == BPIDs.end()) {
        BPIDs.insert(BPID);

        // POS in the middle.
        fkey = encoder_.CreateFKey_PPP(DependencyLabelerFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
        AddFeature(fkey, features);
      }
    }
    BPIDs.clear();
  }
}
