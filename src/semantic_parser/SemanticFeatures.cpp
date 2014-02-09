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

  int sentence_length = sentence->size();
  // True if labeled semantic parsing.
  bool labeled =
      static_cast<SemanticOptions*>(pipe_->GetOptions())->labeled();

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
  uint16_t HWID, MWID;
  uint8_t HPID, MPID, BPID;
  uint8_t pHPID, pMPID, nHPID, nMPID;

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

// Add features for arbitrary siblings.
#if 0
void SemanticFeatures::AddArbitrarySiblingFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier,
                          int sibling) {
  AddSiblingFeatures(sentence, r, head, modifier, sibling, false);
}

// Add features for consecutive siblings.
void SemanticFeatures::AddConsecutiveSiblingFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier,
                          int sibling) {
  AddSiblingFeatures(sentence, r, head, modifier, sibling, true);
}

// Add features for siblings.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void SemanticFeatures::AddSiblingFeatures(SemanticInstanceNumeric* sentence,
                                            int r,
                                            int head,
                                            int modifier,
                                            int sibling,
                                            bool consecutive) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  int sentence_length = sentence->size();
  bool first_child = consecutive && (head == modifier);
  bool last_child = consecutive &&
                    (sibling == sentence_length || sibling <= 0);

  CHECK_NE(sibling, 0) << "Currently, last child is encoded as s = -1.";

  if (FLAGS_use_pair_features_second_order) {
    // Add word pair features for head and modifier, and modifier and sibling.
    if (consecutive) {
      int m = modifier;
      int s = sibling;
      if (modifier == head) m = 0; // s is the first child of h.
      if (sibling <= 0 || sibling >= sentence_length) s = 0; // m is the last child of h.

      AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::NEXTSIBL_M_S,
                          m, s, true, true, features);
    } else {
      if (FLAGS_use_pair_features_arbitrary_siblings) {
        // Add word pair features for modifier and sibling.
        AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::ALLSIBL_M_S,
                            modifier, sibling, true, true, features);
      }
    }
  }

  // Direction of attachment for the first and second children.
  // When consecutive == true, we only look at the second one.
  uint8_t direction_code_first; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_second; // 0x1 if right attachment, 0x0 otherwise.

  if (modifier < head) {
    direction_code_first = 0x0;
  } else {
    direction_code_first = 0x1;
  }

  if (sibling < head) {
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
  HWID = (*word_ids)[head];
  MWID = first_child? TOKEN_START : (*word_ids)[modifier];
  SWID = last_child? TOKEN_STOP : (*word_ids)[sibling];
  HPID = (*pos_ids)[head];
  MPID = first_child? TOKEN_START : (*pos_ids)[modifier];
  SPID = last_child? TOKEN_STOP : (*pos_ids)[sibling];

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
  if (FLAGS_use_trilexical_features) {
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
// The features are very similar to the ones used in Koo et al. EGSTRA.
void SemanticFeatures::AddGrandparentFeatures(
                          SemanticInstanceNumeric* sentence,
                          int r,
                          int grandparent,
                          int head,
                          int modifier) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  if (FLAGS_use_pair_features_second_order) {
    if (FLAGS_use_upper_dependencies) {
      AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::GRANDPAR_G_H,
                          grandparent, head, true, true, features);
    }
    AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::GRANDPAR_G_M,
                        grandparent, modifier, true, true, features);
  }

  // Relative position of the grandparent, head and modifier.
  uint8_t direction_code_gh; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_hm; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_gm; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code; // 0x0, 0x1, or 0x2 (see three cases below).

  if (head < grandparent) {
    direction_code_gh = 0x0;
  } else {
    direction_code_gh = 0x1;
  }

  if (modifier < head) {
    direction_code_hm = 0x0;
  } else {
    direction_code_hm = 0x1;
  }

  if (modifier < grandparent) {
    direction_code_gm = 0x0;
  } else {
    direction_code_gm = 0x1;
  }

  if (direction_code_gh == direction_code_hm) {
    direction_code = 0x0; // Pointing in the same direction: g - h - m.
  } else if (direction_code_hm != direction_code_gm) {
    direction_code = 0x1; // Zig-zag inwards: g - m - h .
  } else {
    direction_code = 0x2; // Non-projective: m - g - h.
  }

  // TODO: Maybe add some of the non-projective arc features for the case
  // where direction_code = 0x2, which implies that (h,m) is non-projective.
  if (FLAGS_use_nonprojective_grandparent) {
    if (direction_code == 0x2) {
      // (h,m) is necessarily non-projective.
      AddWordPairFeatures(sentence, SemanticFeatureTemplateParts::GRANDPAR_NONPROJ_H_M,
                          head, modifier, true, true, features);
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

  // LOG(INFO) << grandparent << " " << head << " " << modifier << " " << sentence_length;
  // Words/POS.
  GWID = (*word_ids)[grandparent];
  HWID = (*word_ids)[head];
  MWID = (*word_ids)[modifier];
  GPID = (*pos_ids)[grandparent];
  HPID = (*pos_ids)[head];
  MPID = (*pos_ids)[modifier];

  flags = SemanticFeatureTemplateParts::GRANDPAR;

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

  if (FLAGS_use_trilexical_features) {
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

  if (FLAGS_use_pair_features_grandsibling_conjunctions) {
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
