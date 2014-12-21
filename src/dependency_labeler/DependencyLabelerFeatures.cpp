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

#include "DependencyLabelerPipe.h"
#include "DependencyLabelerFeatures.h"
#include "DependencyPart.h"
#include "DependencyLabelerFeatureTemplates.h"
#include <set>

// Flags for specific options in feature definitions.
// Note: this will be deprecated soon.
// Note 2: these flags don't get saved in the model file!!! So we need to call
// them at test time too.
// TODO: deprecate this.
DEFINE_int32(dependency_token_context, 1,
             "Size of the context in token features.");

// Add arc-factored features including lemmas and morpho-syntactic
// feature information.
// The features are very similar to the ones used in Koo et al. EGSTRA.
void DependencyLabelerFeatures::AddArcFeatures(
    DependencyInstanceNumeric* sentence,
    int r,
    int head,
    int modifier) {
  CHECK(!input_features_[r]);
  BinaryFeatures *features = new BinaryFeatures;
  input_features_[r] = features;

  AddWordPairFeatures(sentence, DependencyLabelerFeatureTemplateParts::ARC,
                      head, modifier, true, true, features);
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
        HFID = (HFID << 4) | ((uint16_t) 0xf);
      } else {
        HFID = (HFID << 4) | ((uint16_t) j);
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
        MFID = (MFID << 4) | ((uint16_t) 0xf);
      } else {
        MFID = (MFID << 4) | ((uint16_t) k);
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

