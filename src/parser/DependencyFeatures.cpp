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

#include "DependencyPipe.h"
#include "DependencyFeatures.h"
#include "DependencyPart.h"
#include "DependencyFeatureTemplates.h"
#include <set>

// Flags for specific options in feature definitions.
// Note: this will be deprecated soon.
// Note 2: these flags don't get saved in the model file!!! So we need to call
// them at test time too.
// TODO: deprecate this.
DEFINE_bool(use_upper_dependencies, false,
            "True for using upper dependencies.");
DEFINE_int32(dependency_token_context, 1,
            "Size of the context in token features.");
//DEFINE_bool(use_nonprojective_grandparent, true,
//            "True for using nonprojective grandparent features.");
DEFINE_bool(use_nonprojective_grandparent, false,
            "True for using nonprojective grandparent features.");
DEFINE_bool(use_pair_features_arbitrary_siblings, false, /*false,*/
            "True for using pair features for arbitrary sibling parts.");
DEFINE_bool(use_pair_features_second_order, true, /*false,*/
            "True for using pair features for second order parts.");
DEFINE_bool(use_pair_features_grandsibling_conjunctions, true, /*false,*/
            "True for using pair features for grandsiblings that are conjunctions.");
// TODO: try setting this true.
DEFINE_bool(use_trilexical_features, false,
            "True for using trilexical features.");

// Define this macro to have features similar to McDonald et al. MSTPARSER.
// #define USE_MST_FEATURES

// Add arc-factored features without looking at lemmas and morpho-syntactic
// feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyFeatures::AddArcFeaturesLight(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

#if USE_MST_FEATURES
  AddWordPairFeaturesMST(sentence, DependencyFeatureTemplateParts::ARC,
                      head, modifier, features);
#else
  AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::ARC,
                      head, modifier, false, false, features);
#endif
}

// Add arc-factored features including lemmas and morpho-syntactic
// feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyFeatures::AddArcFeatures(DependencyInstanceNumeric* sentence,
                                       int r,
                                       int head,
                                       int modifier) {
  DependencyOptions *options = static_cast<class DependencyPipe*>(pipe_)->
      GetDependencyOptions();
  if (!options->large_feature_set()) {
    AddArcFeaturesLight(sentence, r, head, modifier);
    return;
  }

  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::ARC,
                      head, modifier, true, true, features);
}

// Add features for arbitrary siblings.
void DependencyFeatures::AddArbitrarySiblingFeatures(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier,
                          int sibling) {
  AddSiblingFeatures(sentence, r, head, modifier, sibling, false);
}

// Add features for consecutive siblings.
void DependencyFeatures::AddConsecutiveSiblingFeatures(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier,
                          int sibling) {
  AddSiblingFeatures(sentence, r, head, modifier, sibling, true);
}

// Add features for siblings.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyFeatures::AddSiblingFeatures(DependencyInstanceNumeric* sentence,
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

      AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::NEXTSIBL_M_S,
                          m, s, true, true, features);
    } else {
      if (FLAGS_use_pair_features_arbitrary_siblings) {
        // Add word pair features for modifier and sibling.
        AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::ALLSIBL_M_S,
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
    flags = DependencyFeatureTemplateParts::NEXTSIBL;
  } else {
    flags = DependencyFeatureTemplateParts::ALLSIBL;
  }

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyFeatureTemplateSibling::COUNT, 256);

  // Add direction information.
  if (!consecutive) flags |= (direction_code_first << 6); // 1 more bit.
  flags |= (direction_code_second << 7); // 1 more bit.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateSibling::BIAS, flags);
  AddFeature(fkey, features);

  // Triplet POS feature.
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateSibling::HP_MP_SP, flags, HPID, MPID, SPID);
  AddFeature(fkey, features);

  // Triplet unilexical features.
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateSibling::HW_MP_SP, flags, HWID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateSibling::HP_MW_SP, flags, MWID, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateSibling::HP_MP_SW, flags, SWID, HPID, MPID);
  AddFeature(fkey, features);

  // Triplet bilexical features.
  fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateSibling::HW_MW_SP, flags, HWID, MWID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateSibling::HW_MP_SW, flags, HWID, SWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateSibling::HP_MW_SW, flags, MWID, SWID, HPID);
  AddFeature(fkey, features);

  // Trilexical features.
  if (FLAGS_use_trilexical_features) {
    // Triplet trilexical features.
    fkey = encoder_.CreateFKey_WWW(DependencyFeatureTemplateSibling::HW_MW_SW, flags, HWID, MWID, SWID);
    AddFeature(fkey, features);
  }

  // Pairwise POS features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateSibling::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateSibling::HP_SP, flags, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateSibling::MP_SP, flags, MPID, SPID);
  AddFeature(fkey, features);

  // Pairwise unilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateSibling::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateSibling::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateSibling::HW_SP, flags, HWID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateSibling::HP_SW, flags, SWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateSibling::MW_SP, flags, MWID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateSibling::MP_SW, flags, SWID, MPID);
  AddFeature(fkey, features);

  // Pairwise bilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateSibling::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateSibling::HW_SW, flags, HWID, SWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateSibling::MW_SW, flags, MWID, SWID);
  AddFeature(fkey, features);
}

// Add features for grandparents.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyFeatures::AddGrandparentFeatures(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int grandparent,
                          int head,
                          int modifier) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  if (FLAGS_use_pair_features_second_order) {
    if (FLAGS_use_upper_dependencies) {
      AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::GRANDPAR_G_H,
                          grandparent, head, true, true, features);
    }
    AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::GRANDPAR_G_M,
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
      AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::GRANDPAR_NONPROJ_H_M,
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

  flags = DependencyFeatureTemplateParts::GRANDPAR;

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyFeatureTemplateGrandparent::COUNT, 256);

  // Add direction information.
  flags |= (direction_code << 6); // 2 more bits.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateGrandparent::BIAS, flags);
  AddFeature(fkey, features);

  // Triplet POS feature.
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateGrandparent::GP_HP_MP, flags, GPID, HPID, MPID);
  AddFeature(fkey, features);

  // Triplet unilexical features.
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateGrandparent::GW_HP_MP, flags, GWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateGrandparent::GP_HW_MP, flags, HWID, GPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateGrandparent::GP_HP_MW, flags, MWID, GPID, HPID);
  AddFeature(fkey, features);

  // Triplet bilexical features.
  fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateGrandparent::GW_HW_MP, flags, GWID, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateGrandparent::GW_HP_MW, flags, GWID, MWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateGrandparent::GP_HW_MW, flags, HWID, MWID, GPID);
  AddFeature(fkey, features);

  if (FLAGS_use_trilexical_features) {
    // Triplet trilexical features.
    fkey = encoder_.CreateFKey_WWW(DependencyFeatureTemplateGrandparent::GW_HW_MW, flags, GWID, HWID, MWID);
    AddFeature(fkey, features);
  }

  // Pairwise POS features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateGrandparent::GP_HP, flags, GPID, HPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateGrandparent::GP_MP, flags, GPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateGrandparent::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  // Pairwise unilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateGrandparent::GW_HP, flags, GWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateGrandparent::GP_HW, flags, HWID, GPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateGrandparent::GW_MP, flags, GWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateGrandparent::GP_MW, flags, MWID, GPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateGrandparent::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateGrandparent::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);

  // Pairwise bilexical features.
  // This is not redundant w.r.t. the arc features, since the flags may carry important information.
  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateGrandparent::GW_HW, flags, GWID, HWID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateGrandparent::GW_MW, flags, GWID, MWID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateGrandparent::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);
}

// Add features for grand-siblings.
void DependencyFeatures::AddGrandSiblingFeatures(DependencyInstanceNumeric* sentence,
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

  flags = DependencyFeatureTemplateParts::GRANDSIBL;

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyFeatureTemplateGrandSibl::COUNT, 256);

  // Add direction information.
  flags |= (direction_code << 6); // 2 more bits.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateGrandSibl::BIAS, flags);
  AddFeature(fkey, features);

  // Quadruplet POS feature.
  fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateGrandSibl::GP_HP_MP_SP, flags, GPID, HPID, MPID, SPID);
  AddFeature(fkey, features);

  // Quadruplet unilexical features.
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateGrandSibl::GW_HP_MP_SP, flags, GWID, HPID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateGrandSibl::GP_HW_MP_SP, flags, HWID, GPID, MPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateGrandSibl::GP_HP_MW_SP, flags, MWID, GPID, HPID, SPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateGrandSibl::GP_HP_MP_SW, flags, SWID, GPID, HPID, MPID);
  AddFeature(fkey, features);

  if (FLAGS_use_pair_features_grandsibling_conjunctions) {
    if (modifier != head && sentence->IsCoordination(modifier) &&
        sibling > 0 && sibling < sentence->size()) {
      AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::GRANDSIBL_G_S,
                          grandparent, sibling, true, true, features);
    }
  }
}

// Add features for tri-siblings.
void DependencyFeatures::AddTriSiblingFeatures(DependencyInstanceNumeric* sentence,
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

  flags = DependencyFeatureTemplateParts::TRISIBL;

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyFeatureTemplateTriSibl::COUNT, 256);

  // Add direction information.
  flags |= (direction_code << 6); // 1 more bit.

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateTriSibl::BIAS, flags);
  AddFeature(fkey, features);

  // Quadruplet POS feature.
  fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateTriSibl::HP_MP_SP_TP, flags, HPID, MPID, SPID, TPID);
  AddFeature(fkey, features);

  // Quadruplet unilexical features.
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateTriSibl::HW_MP_SP_TP, flags, HWID, MPID, SPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateTriSibl::HP_MW_SP_TP, flags, MWID, HPID, SPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateTriSibl::HP_MP_SW_TP, flags, SWID, HPID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPPP(DependencyFeatureTemplateTriSibl::HP_MP_SP_TW, flags, TWID, HPID, MPID, SPID);

  // Triplet POS features.
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateTriSibl::HP_MP_TP, flags, HPID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateTriSibl::MP_SP_TP, flags, MPID, SPID, TPID);
  AddFeature(fkey, features);

  // Triplet unilexical features.
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateTriSibl::HW_MP_TP, flags, HWID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateTriSibl::HP_MW_TP, flags, MWID, HPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateTriSibl::HP_MP_TW, flags, TWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateTriSibl::MW_SP_TP, flags, MWID, SPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateTriSibl::MP_SW_TP, flags, SWID, MPID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateTriSibl::MP_SP_TW, flags, TWID, MPID, SPID);
  AddFeature(fkey, features);

  // Pairwise POS features.
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateTriSibl::MP_TP, flags, MPID, TPID);
  AddFeature(fkey, features);

  // Pairwise unilexical features.
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateTriSibl::MW_TP, flags, MWID, TPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateTriSibl::MP_TW, flags, TWID, MPID);
  AddFeature(fkey, features);
}

// Add features for non-projective arcs.
// Use the same arc-factored features plus a flag to distinguish the two.
void DependencyFeatures::AddNonprojectiveArcFeatures(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier) {
  // TODO: use AddWordPairFeatures instead.
  // TODO: implement AddLightWordPairFeatures?
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  AddWordPairFeatures(sentence, DependencyFeatureTemplateParts::NONPROJARC,
                      head, modifier, true, true, features);
}

// Add features for directed path between two words.
void DependencyFeatures::AddDirectedPathFeatures(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int ancestor,
                          int descendant) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  int left_position, right_position;
  int span_length;

  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t binned_length_code; // Binned arc length.

  if (descendant < ancestor) {
    left_position = descendant;
    right_position = ancestor;
    direction_code = 0x0;
  } else {
    left_position = ancestor;
    right_position = descendant;
    direction_code = 0x1;
  }
  span_length = right_position - left_position;

  // 7 possible values for binned_length_code (3 bits)
  if (span_length > 40) {
    binned_length_code = 0x6;
  } else if (span_length > 30) {
    binned_length_code = 0x5;
  } else if (span_length > 20) {
    binned_length_code = 0x4;
  } else if (span_length > 10) {
    binned_length_code = 0x3;
  } else if (span_length > 5) {
    binned_length_code = 0x2;
  } else if (span_length > 2) {
    binned_length_code = 0x1;
  } else {
    binned_length_code = 0x0;
  }

  // Codewords for accommodating word/POS information.
  uint16_t AWID, DWID;
  uint8_t APID, DPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  AWID = (*word_ids)[ancestor];
  DWID = (*word_ids)[descendant];
  APID = (*pos_ids)[ancestor];
  DPID = (*pos_ids)[descendant];

  if (!sentence->IsVerb(ancestor) && !sentence->IsNoun(ancestor)) return;
  if (!sentence->IsVerb(descendant) && !sentence->IsNoun(descendant)) return;

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyFeatureTemplatePath::COUNT, 256);

  // Code for feature type, mode and extended mode.
  flags = DependencyFeatureTemplateParts::PATH;

  // Bias feature.
  fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplatePath::BIAS, flags);
  AddFeature(fkey, features);

  // Head-modifier features
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplatePath::AP_DP, flags, APID, DPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplatePath::AP_DP, flags, APID, DPID, binned_length_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplatePath::AW_DW, flags, AWID, DWID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplatePath::AP_DW, flags, DWID, APID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplatePath::AW_DP, flags, AWID, DPID);
  AddFeature(fkey, features);

  flags |= (0x1 << 4); // 1 more bit.
  flags |= (direction_code << 5); // 1 more bit.

  // Head-modifier features
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplatePath::AP_DP, flags, APID, DPID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplatePath::AP_DP, flags, APID, DPID, binned_length_code);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplatePath::AP_DW, flags, DWID, APID);
  AddFeature(fkey, features);

  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplatePath::AW_DP, flags, AWID, DPID);
  AddFeature(fkey, features);
}

// Add features for head bigrams.
void DependencyFeatures::AddHeadBigramFeatures(
                          DependencyInstanceNumeric* sentence,
                          int r,
                          int head,
                          int modifier,
                          int previous_head) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

	int sentence_length = sentence->size();
	int left_position, right_position;
	int span_length;

  uint8_t crossing_arcs = 0x0; // 0x1 is arcs are crossing, 0x0 otherwise.
  uint8_t direction_code; // 0x1 if right attachment, 0x0 otherwise.
  uint8_t direction_code_previous; // Same for previous arc.
  uint8_t binned_length_code; // Binned arc length.
  uint8_t binned_length_code_previous; // Binned arc length for the previous arc.
  int arc_length;
  int previous_arc_length;

  int previous_modifier = modifier - 1;
  if (previous_modifier < previous_head) {
    direction_code_previous = 0x0;
    previous_arc_length = previous_head - previous_modifier;
  } else {
    direction_code_previous = 0x1;
    previous_arc_length = previous_modifier - previous_head;
  }

  if (modifier < head) {
    direction_code = 0x0;
    arc_length = head - modifier;
  } else {
    direction_code = 0x1;
    arc_length = modifier - head;
  }

  // NOTE: I think this does not include all possible crossing cases.
  if (direction_code == direction_code_previous) {
    if (previous_head < head) crossing_arcs = 0x1; // The two arcs cross.
  }

  // Define flags for special cases where there are not 4 distinct words.
  uint8_t same_head = (previous_head == head)? 0x1 : 0x0;
  uint8_t head_left = (previous_modifier == head)? 0x2 : 0x0;
  uint8_t head_right = (previous_head == modifier)? 0x4 : 0x0;
  uint8_t flags_bigram = same_head | head_left | head_right;

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

  // 7 possible values for binned_length_code_previous (3 bits)
  if (previous_arc_length > 40) {
    binned_length_code_previous = 0x6;
  } else if (previous_arc_length > 30) {
    binned_length_code_previous = 0x5;
  } else if (previous_arc_length > 20) {
    binned_length_code_previous = 0x4;
  } else if (previous_arc_length > 10) {
    binned_length_code_previous = 0x3;
  } else if (previous_arc_length > 5) {
    binned_length_code_previous = 0x2;
  } else if (previous_arc_length > 2) {
    binned_length_code_previous = 0x1;
  } else {
    binned_length_code_previous = 0x0;
  }

  // Codewords for accommodating word/POS information.
  uint16_t JWID, pMWID, HWID, MWID;
  uint8_t JPID, pMPID, HPID, MPID;

  // Array of form/lemma IDs.
  const vector<int>* word_ids = &sentence->GetFormIds();

  // Array of POS/CPOS IDs.
  const vector<int>* pos_ids = &sentence->GetCoarsePosIds();

  uint64_t fkey;
  uint8_t flags = 0;

  // Words/POS.
  JWID = (*word_ids)[previous_head];
  pMWID = (*word_ids)[previous_modifier];
  HWID = (*word_ids)[head];
  MWID = (*word_ids)[modifier];
  JPID = (*pos_ids)[previous_head];
  pMPID = (*pos_ids)[previous_modifier];
  HPID = (*pos_ids)[head];
  MPID = (*pos_ids)[modifier];

  // Maximum is 255 feature templates.
  CHECK_LT(DependencyFeatureTemplateBigram::COUNT, 256);

  for (int mode = 0; mode < 2; ++mode) {
    // Code for feature type, mode and extended mode.
    flags = DependencyFeatureTemplateParts::BIGRAM;
    
    flags |= (mode << 4); // 1 more bit.
    if (mode == 1) {
      flags |= (crossing_arcs << 5); // 1 more bit
      flags |= (direction_code_previous << 6); // 1 more bit
      flags |= (direction_code << 7); // 1 more bit    
    }

    // Bias feature.
    fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateBigram::BIAS, flags);
    AddFeature(fkey, features);

    if (mode == 1) {
      fkey = encoder_.CreateFKey_PPPPP(DependencyFeatureTemplateBigram::pMP_MP_pDIST_DIST, flags,
          pMPID, MPID, binned_length_code_previous, binned_length_code, flags_bigram);
      AddFeature(fkey, features);
    }

    // POS features.
    fkey = encoder_.CreateFKey_PPPPP(DependencyFeatureTemplateBigram::JP_pMP_HP_MP, flags, 
        JPID, pMPID, HPID, MPID, flags_bigram);
    AddFeature(fkey, features);

    // Unilexical features.
    fkey = encoder_.CreateFKey_WPPPP(DependencyFeatureTemplateBigram::JW_pMP_HP_MP, flags, 
        JWID, pMPID, HPID, MPID, flags_bigram);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WPPPP(DependencyFeatureTemplateBigram::JP_pMW_HP_MP, flags, 
        pMWID, JPID, HPID, MPID, flags_bigram);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WPPPP(DependencyFeatureTemplateBigram::JP_pMP_HW_MP, flags, 
        HWID, JPID, pMPID, MPID, flags_bigram);
    AddFeature(fkey, features);

    fkey = encoder_.CreateFKey_WPPPP(DependencyFeatureTemplateBigram::JP_pMP_HP_MW, flags, 
        MWID, JWID, pMPID, HPID, flags_bigram);
    AddFeature(fkey, features);
  }
}

// General function to add features for a pair of words (arcs, sibling words,
// etc.) Can optionally use lemma and morpho-syntactic feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyFeatures::AddWordPairFeatures(DependencyInstanceNumeric* sentence,
                                             int pair_type,
                                             int head,
                                             int modifier,
                                             bool use_lemma_features,
                                             bool use_morphological_features,
                                             BinaryFeatures *features) {
  int sentence_length = sentence->size();
  // True if labeled dependency parsing.
  bool labeled =
      static_cast<DependencyOptions*>(pipe_->GetOptions())->labeled();

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
  CHECK_LT(DependencyFeatureTemplateArc::COUNT, 256);

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
  pHLID = (head > 0)? sentence->GetLemmaId(head - 1) : TOKEN_START;
  pMLID = (modifier > 0)? sentence->GetLemmaId(modifier - 1) : TOKEN_START;
  pHWID = (head > 0)? sentence->GetFormId(head - 1) : TOKEN_START;
  pMWID = (modifier > 0)? sentence->GetFormId(modifier - 1) : TOKEN_START;
  pHPID = (head > 0)? sentence->GetCoarsePosId(head - 1) : TOKEN_START;
  pMPID = (modifier > 0)? sentence->GetCoarsePosId(modifier - 1) : TOKEN_START;
  pHQID = (head > 0)? sentence->GetPosId(head - 1) : TOKEN_START;
  pMQID = (modifier > 0)? sentence->GetPosId(modifier - 1) : TOKEN_START;

  nHLID = (head < sentence_length - 1)?
      sentence->GetLemmaId(head + 1) : TOKEN_STOP;
  nMLID = (modifier < sentence_length - 1)?
      sentence->GetLemmaId(modifier + 1) : TOKEN_STOP;
  nHWID = (head < sentence_length - 1)?
      sentence->GetFormId(head + 1) : TOKEN_STOP;
  nMWID = (modifier < sentence_length - 1)?
      sentence->GetFormId(modifier + 1) : TOKEN_STOP;
  nHPID = (head < sentence_length - 1)?
      sentence->GetCoarsePosId(head + 1) : TOKEN_STOP;
  nMPID = (modifier < sentence_length - 1)?
      sentence->GetCoarsePosId(modifier + 1) : TOKEN_STOP;
  nHQID = (head < sentence_length - 1)?
      sentence->GetPosId(head + 1) : TOKEN_STOP;
  nMQID = (modifier < sentence_length - 1)?
      sentence->GetPosId(modifier + 1) : TOKEN_STOP;

  // Context size = 2:
  ppHLID = (head > 1)? sentence->GetLemmaId(head - 2) : TOKEN_START;
  ppMLID = (modifier > 1)? sentence->GetLemmaId(modifier - 2) : TOKEN_START;
  ppHWID = (head > 1)? sentence->GetFormId(head - 2) : TOKEN_START;
  ppMWID = (modifier > 1)? sentence->GetFormId(modifier - 2) : TOKEN_START;
  ppHPID = (head > 1)? sentence->GetCoarsePosId(head - 2) : TOKEN_START;
  ppMPID = (modifier > 1)? sentence->GetCoarsePosId(modifier - 2) : TOKEN_START;
  ppHQID = (head > 1)? sentence->GetPosId(head - 2) : TOKEN_START;
  ppMQID = (modifier > 1)? sentence->GetPosId(modifier - 2) : TOKEN_START;

  nnHLID = (head < sentence_length - 2)?
      sentence->GetLemmaId(head + 2) : TOKEN_STOP;
  nnMLID = (modifier < sentence_length - 2)?
      sentence->GetLemmaId(modifier + 2) : TOKEN_STOP;
  nnHWID = (head < sentence_length - 2)?
      sentence->GetFormId(head + 2) : TOKEN_STOP;
  nnMWID = (modifier < sentence_length - 2)?
      sentence->GetFormId(modifier + 2) : TOKEN_STOP;
  nnHPID = (head < sentence_length - 2)?
      sentence->GetCoarsePosId(head + 2) : TOKEN_STOP;
  nnMPID = (modifier < sentence_length - 2)?
      sentence->GetCoarsePosId(modifier + 2) : TOKEN_STOP;
  nnHQID = (head < sentence_length - 2)?
      sentence->GetPosId(head + 2) : TOKEN_STOP;
  nnMQID = (modifier < sentence_length - 2)?
      sentence->GetPosId(modifier + 2) : TOKEN_STOP;

  // Code for feature type.
  flags = feature_type; // 4 bits.
  flags |= (direction_code << 4); // 1 more bit.

  // Bias feature (not in EGSTRA).
  fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateArc::BIAS, flags);
  AddFeature(fkey, features);

  /////////////////////////////////////////////////////////////////////////////
  // Token features.
  /////////////////////////////////////////////////////////////////////////////

  // Note: in EGSTRA (but not here), token and token contextual features go
  // without direction flags.
  // Coarse POS features.
  fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::HP, flags, HPID);
  AddFeature(fkey, features);
  // Fine POS features.
  fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::HQ, flags, HQID);
  AddFeature(fkey, features);
  // Lexical features.
  fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::HW, flags, HWID);
  AddFeature(fkey, features);
  if (use_lemma_features) {
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::HL, flags, HLID);
    AddFeature(fkey, features);
  }
  // Features involving words and POS.
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HWP, flags, HWID, HPID);
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
        HFID = (HFID << 4) | ((uint16_t) 0xf);
      } else {
        HFID = (HFID << 4) | ((uint16_t) j);
      }
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::HF, flags, HFID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateArc::HWF, flags, HWID, HFID);
      AddFeature(fkey, features);
    }
  }

  // If labeled parsing, features involving the modifier only are still useful,
  // since they will be conjoined with the label.
  if (labeled) {
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::MP, flags, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::MQ, flags, MQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::MW, flags, MWID);
    AddFeature(fkey, features);
    if (use_lemma_features) {
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::ML, flags, MLID);
      AddFeature(fkey, features);
    }
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::MWP, flags, MWID, MPID);
    AddFeature(fkey, features);
    for (int k = 0; k < sentence->GetNumMorphFeatures(modifier); ++k) {
      MFID = sentence->GetMorphFeature(modifier, k);
      CHECK_LT(MFID, 0xfff);
      if (k >= 0xf) {
        LOG(WARNING) << "Too many morphological features (" << k << ")";
        MFID = (MFID << 4) | ((uint16_t) 0xf);
      } else {
        MFID = (MFID << 4) | ((uint16_t) k);
      }
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::MF, flags, MFID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateArc::MWF, flags, MWID, MFID);
      AddFeature(fkey, features);
    }
  }

  /////////////////////////////////////////////////////////////////////////////
  // Token contextual features.
  /////////////////////////////////////////////////////////////////////////////

  if (max_token_context >= 1) {
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::pHP, flags, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nHP, flags, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::pHQ, flags, pHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nHQ, flags, nHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::pHW, flags, pHWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nHW, flags, nHWID);
    AddFeature(fkey, features);
    if (use_lemma_features) {
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::pHL, flags, pHLID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nHL, flags, nHLID);
      AddFeature(fkey, features);
    }
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::pHWP, flags, pHWID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::nHWP, flags, nHWID, nHPID);
    AddFeature(fkey, features);

    // If labeled parsing, features involving the modifier only are still useful,
    // since they will be conjoined with the label.
    if (labeled) {
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::pMP, flags, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nMP, flags, nMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::pMQ, flags, pMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nMQ, flags, nMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::pMW, flags, pMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nMW, flags, nMWID);
      AddFeature(fkey, features);
      if (use_lemma_features) {
        fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::pML, flags, pMLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nML, flags, nMLID);
        AddFeature(fkey, features);
      }
      fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::pMWP, flags, pMWID, pMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::nMWP, flags, nMWID, nMPID);
      AddFeature(fkey, features);
    }
  }

  if (max_token_context >= 2) {
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::ppHP, flags, ppHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nnHP, flags, nnHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::ppHQ, flags, ppHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nnHQ, flags, nnHQID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::ppHW, flags, ppHWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nnHW, flags, nnHWID);
    AddFeature(fkey, features);
    if (use_lemma_features) {
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::ppHL, flags, ppHLID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nnHL, flags, nnHLID);
      AddFeature(fkey, features);
    }
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::ppHWP, flags, ppHWID, ppHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::nnHWP, flags, nnHWID, nnHPID);
    AddFeature(fkey, features);

    // If labeled parsing, features involving the modifier only are still useful,
    // since they will be conjoined with the label.
    if (labeled) {
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::ppMP, flags, ppMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nnMP, flags, nnMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::ppMQ, flags, ppMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::nnMQ, flags, nnMQID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::ppMW, flags, ppMWID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nnMW, flags, nnMWID);
      AddFeature(fkey, features);
      if (use_lemma_features) {
        fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::ppML, flags, ppMLID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::nnML, flags, nnMLID);
        AddFeature(fkey, features);
      }
      fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::ppMWP, flags, ppMWID, ppMPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::nnMWP, flags, nnMWID, nnMPID);
      AddFeature(fkey, features);
    }
  }

  // Contextual bigram and trigram features involving POS.
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::HP_pHP, flags, HPID, pHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_pHP_ppHP, flags, HPID, pHPID, ppHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::HP_nHP, flags, HPID, nHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_nHP_nnHP, flags, HPID, nHPID, nnHPID);
  AddFeature(fkey, features);

  // If labeled parsing, features involving the modifier only are still useful,
  // since they will be conjoined with the label.
  if (labeled) {
    fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::MP_pMP, flags, MPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::MP_pMP_ppMP, flags, MPID, pMPID, ppMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::MP_nMP, flags, MPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::MP_nMP_nnMP, flags, MPID, nMPID, nnMPID);
    AddFeature(fkey, features);
  }

  /////////////////////////////////////////////////////////////////////////////
  // Dependency features.
  // Everything goes with direction flags and with coarse POS.
  /////////////////////////////////////////////////////////////////////////////

  // POS features.
  fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::HP_MP, flags, HPID, MPID);
  AddFeature(fkey, features);

  // Lexical/Bilexical features.
  fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateArc::HW_MW, flags, HWID, MWID);
  AddFeature(fkey, features);

  // Features involving words and POS.
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HP_MW, flags, MWID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HW_MP, flags, HWID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_WWPP(DependencyFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
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
        fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateArc::HF_MF, flags, HFID, MFID);
        AddFeature(fkey, features);

        // Morphological features conjoined with POS.
        fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HF_MP, flags, HFID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateArc::HF_MFP, flags, HFID, MFID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HP_MF, flags, MFID, HPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateArc::HFP_MF, flags, HFID, MFID, HPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HFP_MP, flags, HFID, HPID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HP_MFP, flags, MFID, HPID, MPID);
        AddFeature(fkey, features);
        fkey = encoder_.CreateFKey_WWPP(DependencyFeatureTemplateArc::HFP_MFP, flags, HFID, MFID, HPID, MPID);
        AddFeature(fkey, features);
      }
    }
  }

  // Contextual features.
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPPPPP(DependencyFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
  AddFeature(fkey, features);

  // Features for adjacent dependencies.
  if (head != 0 && head == modifier - 1) {
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPP(DependencyFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID, 0x1);
    AddFeature(fkey, features);
  } else if (head != 0 && head == modifier + 1) {
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID, 0x1);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPP(DependencyFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID, 0x1);
    AddFeature(fkey, features);
  }

  // Exact arc length.
  fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::DIST, flags, exact_length_code);
  AddFeature(fkey, features);

  // Binned arc length.
  for (uint8_t bin = 0; bin <= binned_length_code; ++bin) {
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::BIAS, flags, bin);
    AddFeature(fkey, features);
  }

  // POS features conjoined with binned arc length.
  for (uint8_t bin = 0; bin <= binned_length_code; bin++) {
    fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::HP, flags, HPID, bin);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::MP, flags, MPID, bin);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP, flags, HPID, MPID, bin);
    AddFeature(fkey, features);
  }

  // In-between flags.
  fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::BFLAG, flags, flag_between_verb);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::BFLAG, flags, flag_between_punc);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::BFLAG, flags, flag_between_coord);
  AddFeature(fkey, features);

  // POS features conjoined with in-between flag.
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_verb);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_punc);
  AddFeature(fkey, features);
  fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_BFLAG, flags, HPID, MPID, flag_between_coord);
  AddFeature(fkey, features);

  set<int> BPIDs;
  set<int> BWIDs;
  for (int i = left_position + 1; i < right_position; ++i) {
    BPID = sentence->GetCoarsePosId(i);
    if (BPIDs.find(BPID) == BPIDs.end()) {
      BPIDs.insert(BPID);

      // POS in the middle.
      fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateArc::HW_MW_BP, flags, HWID, MWID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HW_MP_BP, flags, HWID, MPID, BPID);
      AddFeature(fkey, features);
      fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HP_MW_BP, flags, MWID, HPID, BPID);
      AddFeature(fkey, features);
    }
    BPIDs.clear();
  }
}

// General function to add features for a pair of words (arcs, sibling words,
// etc.) No lemma and morpho-syntactic feature information are used.
// The features are very similar to the ones used in McDonald et al. MSTParser.
void DependencyFeatures::AddWordPairFeaturesMST(DependencyInstanceNumeric* sentence,
                                                int pair_type,
                                                int head,
                                                int modifier,
                                                BinaryFeatures *features) {
  int sentence_length = sentence->size();
  // True if labeled dependency parsing.
  bool labeled =
      static_cast<DependencyOptions*>(pipe_->GetOptions())->labeled();

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
  CHECK_LT(DependencyFeatureTemplateArc::COUNT, 256);

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
    fkey = encoder_.CreateFKey_NONE(DependencyFeatureTemplateArc::BIAS, flags);
    AddFeature(fkey, features);

    // POS features.
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::HP, flags, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_P(DependencyFeatureTemplateArc::MP, flags, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PP(DependencyFeatureTemplateArc::HP_MP, flags, HPID, MPID);
    AddFeature(fkey, features);

    // Lexical/Bilexical features.
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::HW, flags, HWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_W(DependencyFeatureTemplateArc::MW, flags, MWID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateArc::HW_MW, flags, HWID, MWID);
    AddFeature(fkey, features);

    // Features involving words and POS.
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HWP, flags, HWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::MWP, flags, MWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HP_MW, flags, MWID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HP_MWP, flags, MWID, MPID, HPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HW_MP, flags, HWID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HWP_MP, flags, HWID, HPID, MPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_WWPP(DependencyFeatureTemplateArc::HWP_MWP, flags, HWID, MWID, HPID, MPID);
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
          fkey = encoder_.CreateFKey_WW(DependencyFeatureTemplateArc::HF_MF, flags, HFID, MFID);
          AddFeature(fkey, features);

          // Morphological features conjoined with POS.
          fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HF_MP, flags, HFID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateArc::HF_MFP, flags, HFID, MFID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WP(DependencyFeatureTemplateArc::HP_MF, flags, MFID, HPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWP(DependencyFeatureTemplateArc::HFP_MF, flags, HFID, MFID, HPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HFP_MP, flags, HFID, HPID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WPP(DependencyFeatureTemplateArc::HP_MFP, flags, MFID, HPID, MPID);
          AddFeature(fkey, features);
          fkey = encoder_.CreateFKey_WWPP(DependencyFeatureTemplateArc::HFP_MFP, flags, HFID, MFID, HPID, MPID);
          AddFeature(fkey, features);
        }
      }
    }

    // Contextual features.
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_pHP, flags, HPID, MPID, pHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_nHP, flags, HPID, MPID, nHPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_pMP, flags, HPID, MPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_nMP, flags, HPID, MPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_pHP_pMP, flags, HPID, MPID, pHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_nHP_nMP, flags, HPID, MPID, nHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_pHP_nMP, flags, HPID, MPID, pHPID, nMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPP(DependencyFeatureTemplateArc::HP_MP_nHP_pMP, flags, HPID, MPID, nHPID, pMPID);
    AddFeature(fkey, features);
    fkey = encoder_.CreateFKey_PPPPPP(DependencyFeatureTemplateArc::HP_MP_pHP_nHP_pMP_nMP, flags, HPID, MPID, pHPID, nHPID, pMPID, nMPID);
    AddFeature(fkey, features);

    // In-between features.
    set<int> BPIDs;
    for (int i = left_position + 1; i < right_position; ++i) {
      BPID = (*pos_ids)[i];
      if (BPIDs.find(BPID) == BPIDs.end()) {
        BPIDs.insert(BPID);

        // POS in the middle.
        fkey = encoder_.CreateFKey_PPP(DependencyFeatureTemplateArc::HP_MP_BP, flags, HPID, MPID, BPID);
        AddFeature(fkey, features);
      }
    }
    BPIDs.clear();
  }
}
