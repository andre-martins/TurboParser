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

#ifndef DEPENDENCYFEATURETEMPLATES_H_
#define DEPENDENCYFEATURETEMPLATES_H_

struct DependencyFeatureTemplateParts {
  enum types {
    ARC = 0,
    NEXTSIBL,
    ALLSIBL,
    GRANDPAR,
    NONPROJARC,
    PATH,
    BIGRAM,
    NEXTSIBL_M_S,
    ALLSIBL_M_S,
    GRANDPAR_G_H, /* Not used. TODO: remove. */
    GRANDPAR_G_M,
    GRANDPAR_NONPROJ_H_M, /* Not used. TODO: remove. */
    GRANDSIBL,
    TRISIBL,
    GRANDSIBL_G_S,
  };
};

struct DependencyFeatureTemplateArc {
  enum types {
    // There is a cross-product between these and direction, distance, {word form}, {pos cpos}
    // Features for head and modifier [add prefixes and suffixes]
    BIAS = 0,				/* bias */
    DIST,					/* exact distance */
    BFLAG,					/* X-between flag */
    HQ,           /* head fine POS */
    MQ,           /* modifier fine POS [useless in unlabeled parsing] */
    HP,           /* head POS */
    MP,           /* modifier POS [useless in unlabeled parsing] */
    HW,           /* head word */
    MW,           /* modifier word [useless in unlabeled parsing] */
    HWP,          /* head word and POS */
    MWP,          /* modifier word and POS [useless in unlabeled parsing] */
    HP_MP,					/* head POS, modifier POS */
    HP_MW,					/* head POS, modifier word */
    HP_MWP,					/* head POS, modifier word and POS */
    HW_MW,					/* head word, modifier word */
    HW_MP,					/* head word, modifier POS */
    HWP_MP,					/* head word and POS, modifier POS */
    HWP_MWP,				/* head word and POS, modifier word and POS */
    HP_MP_BFLAG,			/* head POS, modifier POS,  X-between flag */
    HW_MW_BFLAG,			/* head word, modifier word,  X-between flag */

    HP_MA,					/* head POS, modifier prefix */
    HP_MAP,					/* head POS, modifier prefix and POS */
    HW_MA,					/* head word, modifier prefix */
    HW_MAP,					/* head word, modifier prefix and POS */
    HA_MP,					/* head prefix, modifier POS */
    HAP_MP,					/* head prefix and POS, modifier POS */
    HA_MW,					/* head prefix, modifier word */
    HAP_MW,					/* head prefix and POS, modifier word */
    HP_MZ,					/* head POS, modifier suffix */
    HP_MZP,					/* head POS, modifier suffix and POS */
    HW_MZ,					/* head word, modifier suffix */
    HW_MZP,					/* head word, modifier suffix and POS */
    HZ_MP,					/* head suffix, modifier POS */
    HZP_MP,					/* head suffix and POS, modifier POS */
    HZ_MW,					/* head suffix, modifier word */
    HZP_MW,					/* head suffix and POS, modifier word */

    HF,           /* head feature */
    HFP,          /* head feature and POS */
    MF,           /* modifier feature [useless in unlabeled parsing] */
    MFP,          /* modifier feature and POS [useless in unlabeled parsing] */
    HF_MF,					/* head feature, modifier feature */
    HF_MP,					/* head feature, modifier POS */
    HF_MFP,					/* head feature, modifier feature and POS */
    HP_MF,					/* head POS, modifier feature */
    HFP_MF,         /* head feature and POS, modifier feature */
    HFP_MFP,        /* head feature and POS, modifier feature and POS */
    HFP_MP,         /* head feature and POS, modifier POS */
    HP_MFP,        /* head POS, modifier feature and POS */
    HWF,
    MWF,
    HL,
    ML,

    // Contextual features
    pHP,
    nHP,
    ppHP,
    nnHP,
    pHQ,
    nHQ,
    ppHQ,
    nnHQ,
    pHW,
    nHW,
    ppHW,
    nnHW,
    pHL,
    nHL,
    ppHL,
    nnHL,
    pHWP,
    nHWP,
    ppHWP,
    nnHWP,
    pMP,
    nMP,
    ppMP,
    nnMP,
    pMQ,
    nMQ,
    ppMQ,
    nnMQ,
    pMW,
    nMW,
    ppMW,
    nnMW,
    pML,
    nML,
    ppML,
    nnML,
    pMWP,
    nMWP,
    ppMWP,
    nnMWP,
    HP_pHP,
    HP_nHP,
    HP_pHP_ppHP,
    HP_nHP_nnHP,
    MP_pMP,
    MP_nMP,
    MP_pMP_ppMP,
    MP_nMP_nnMP,

    HP_MP_pHP, 				/* head POS, mod POS, head left-POS */
    HP_MP_nHP, 				/* head POS, mod POS, head right-POS */
    HP_MP_pMP, 				/* head POS, mod POS, mod left-POS */
    HP_MP_nMP, 				/* head POS, mod POS, mod right-POS */
    HP_MP_pHP_pMP, 			/* head POS, mod POS, head left-POS, mod left-POS */
    HP_MP_nHP_nMP, 			/* head POS, mod POS, head right-POS, mod right-POS */
    HP_MP_pHP_nMP, 			/* head POS, mod POS, head left-POS, mod right-POS */
    HP_MP_nHP_pMP, 			/* head POS, mod POS, head right-POS, mod left-POS */
    HP_MP_pHP_nHP_pMP_nMP,  /* head POS, mod POS, head left-POS, head right-POS, mod left-POS, mod right-POS */

    HW_MP_pHP, 				/* head word, mod POS, head left-POS */
    HW_MP_nHP, 				/* head word, mod POS, head right-POS */
    HW_MP_pMP, 				/* head word, mod POS, mod left-POS */
    HW_MP_nMP, 				/* head word, mod POS, mod right-POS */
    HW_MP_pHP_pMP, 			/* head word, mod POS, head left-POS, mod left-POS */
    HW_MP_nHP_nMP, 			/* head word, mod POS, head right-POS, mod right-POS */
    HW_MP_pHP_nMP, 			/* head word, mod POS, head left-POS, mod right-POS */
    HW_MP_nHP_pMP, 			/* head word, mod POS, head right-POS, mod left-POS */

    HP_MW_pHP, 				/* head POS, mod word, head left-POS */
    HP_MW_nHP, 				/* head POS, mod word, head right-POS */
    HP_MW_pMP, 				/* head POS, mod word, mod left-POS */
    HP_MW_nMP, 				/* head POS, mod word, mod right-POS */
    HP_MW_pHP_pMP, 			/* head POS, mod word, head left-POS, mod left-POS */
    HP_MW_nHP_nMP, 			/* head POS, mod word, head right-POS, mod right-POS */
    HP_MW_pHP_nMP, 			/* head POS, mod word, head left-POS, mod right-POS */
    HP_MW_nHP_pMP, 			/* head POS, mod word, head right-POS, mod left-POS */

    HW_MW_pHP, 				/* head word, mod word, head left-POS */
    HW_MW_nHP, 				/* head word, mod word, head right-POS */
    HW_MW_pMP, 				/* head word, mod word, mod left-POS */
    HW_MW_nMP, 				/* head word, mod word, mod right-POS */
    HW_MW_pHP_pMP, 			/* head word, mod word, head left-POS, mod left-POS */
    HW_MW_nHP_nMP, 			/* head word, mod word, head right-POS, mod right-POS */
    HW_MW_pHP_nMP, 			/* head word, mod word, head left-POS, mod right-POS */
    HW_MW_nHP_pMP, 			/* head word, mod word, head right-POS, mod left-POS */

    HP_MP_pHW, 				/* head POS, mod POS, head left-word */
    HP_MP_nHW, 				/* head POS, mod POS, head right-word */
    HP_MP_pMW, 				/* head POS, mod POS, mod left-word */
    HP_MP_nMW, 				/* head POS, mod POS, mod right-word */
    HW_MP_pHW, 				/* head word, mod POS, head left-word */
    HW_MP_nHW, 				/* head word, mod POS, head right-word */
    HW_MP_pMW, 				/* head word, mod POS, mod left-word */
    HW_MP_nMW, 				/* head word, mod POS, mod right-word */
    HP_MW_pHW, 				/* head POS, mod word, head left-word */
    HP_MW_nHW, 				/* head POS, mod word, head right-word */
    HP_MW_pMW, 				/* head POS, mod word, mod left-word */
    HP_MW_nMW, 				/* head POS, mod word, mod right-word */
    HW_MW_pHW, 				/* head word, mod word, head left-word */
    HW_MW_nHW, 				/* head word, mod word, head right-word */
    HW_MW_pMW, 				/* head word, mod word, mod left-word */
    HW_MW_nMW, 				/* head word, mod word, mod right-word */

    // In-between features
    BP,               /* in-between POS */
    HP_MP_BP,         /* head POS, mod POS, in-between POS */
    HW_MW_BP, 				/* head word, mod word, in-between POS */
    HW_MP_BP, 				/* head word, mod POS, in-between POS */
    HP_MW_BP, 				/* head POS, mod word, in-between POS */
    HP_MP_BW, 				/* head POS, mod POS, in-between word */
    HW_MW_BW, 				/* head word, mod word, in-between word */
    SHAPE,					/* shape features */

    COUNT
  };
};

struct DependencyFeatureTemplateSibling {
  enum types {
    // There is a cross-product between these and direction, pos, cpos
    BIAS = 0,				/* bias */

    // Head-modifier-sibling features
    HP_MP_SP,				/* head POS, mod POS, sib POS */
    HW_MP_SP,				/* head word, mod POS, sib POS */
    HP_MW_SP,				/* head POS, mod word, sib POS */
    HP_MP_SW,				/* head POS, mod POS, sib word */
    HW_MW_SP,				/* head word, mod word, sib POS */
    HW_MP_SW,				/* head word, mod POS, sib word */
    HP_MW_SW,				/* head POS, mod word, sib word */
    HW_MW_SW,				/* head word, mod word, sib word */

    HP_MP,				/* head POS, mod POS */
    HW_MP,				/* head word, mod POS */
    HP_MW,				/* head POS, mod word */
    HW_MW,				/* head word, mod word */
    MP_SP,				/* mod POS, sib POS */
    MW_SP,				/* mod word, sib POS */
    MP_SW,				/* mod POS, sib word */
    MW_SW,				/* mod word, sib word */
    HP_SP,				/* head POS, sib POS */
    HW_SP,				/* head word, sib POS */
    HP_SW,				/* head POS, sib word */
    HW_SW,				/* head word, sib word */

    //		HP_SP,				/* head POS, sib POS */
    //		HW_SP,				/* head word, sib POS */
    //		HP_SW,				/* head POS, sib word */
    //		HW_SW,				/* head word, sib word */
    //		HP_MP,				/* head POS, mod POS */
    //		HW_MP,				/* head word, mod POS */
    //		HP_MW,				/* head POS, mod word */
    //		HW_MW,				/* head word, mod word */

    // Contextual features
    HP_MP_SP_pHP, 				/* head POS, mod POS, sib POS, head left-POS */
    HP_MP_SP_nHP, 				/* head POS, mod POS, sib POS, head right-POS */
    HP_MP_SP_pMP, 				/* head POS, mod POS, sib POS, mod left-POS */
    HP_MP_SP_nMP, 				/* head POS, mod POS, sib POS, mod right-POS */
    HP_MP_SP_pSP, 				/* head POS, mod POS, sib POS, sib left-POS */
    HP_MP_SP_nSP, 				/* head POS, mod POS, sib POS, sib right-POS */
    HP_MP_SP_pHP_pMP, 			/* head POS, mod POS, sib POS, head left-POS, mod left-POS */
    HP_MP_SP_pHP_pSP, 			/* head POS, mod POS, sib POS, head left-POS, sib left-POS */
    HP_MP_SP_pMP_pSP, 			/* head POS, mod POS, sib POS, mod left-POS, sib left-POS */
    HP_MP_SP_nHP_nMP, 			/* head POS, mod POS, sib POS, head right-POS, mod right-POS */
    HP_MP_SP_nHP_nSP, 			/* head POS, mod POS, sib POS, head right-POS, sib right-POS */
    HP_MP_SP_nMP_nSP, 			/* head POS, mod POS, sib POS, mod right-POS, sib right-POS */
    HP_MP_SP_pHP_nMP, 			/* head POS, mod POS, sib POS, head left-POS, mod right-POS */
    HP_MP_SP_pHP_nSP, 			/* head POS, mod POS, sib POS, head left-POS, sib right-POS */
    HP_MP_SP_pMP_nSP, 			/* head POS, mod POS, sib POS, mod left-POS, sib right-POS */
    HP_MP_SP_nHP_pMP, 			/* head POS, mod POS, sib POS, head right-POS, mod left-POS */
    HP_MP_SP_nHP_pSP, 			/* head POS, mod POS, sib POS, head right-POS, sib left-POS */
    HP_MP_SP_nMP_pSP, 			/* head POS, mod POS, sib POS, mod right-POS, sib left-POS */
    HP_MP_SP_pHP_pMP_pSP, 			/* head POS, mod POS, sib POS, head left-POS, mod left-POS, sib left-POS */
    HP_MP_SP_nHP_pMP_pSP, 			/* head POS, mod POS, sib POS, head right-POS, mod left-POS, sib left-POS */
    HP_MP_SP_pHP_nMP_pSP, 			/* head POS, mod POS, sib POS, head left-POS, mod right-POS, sib left-POS */
    HP_MP_SP_nHP_nMP_pSP, 			/* head POS, mod POS, sib POS, head right-POS, mod right-POS, sib left-POS */
    HP_MP_SP_pHP_pMP_nSP, 			/* head POS, mod POS, sib POS, head left-POS, mod left-POS, sib right-POS */
    HP_MP_SP_nHP_pMP_nSP, 			/* head POS, mod POS, sib POS, head right-POS, mod left-POS, sib right-POS */
    HP_MP_SP_pHP_nMP_nSP, 			/* head POS, mod POS, sib POS, head left-POS, mod right-POS, sib right-POS */
    HP_MP_SP_nHP_nMP_nSP, 			/* head POS, mod POS, sib POS, head right-POS, mod right-POS, sib right-POS */

    //		/* COULD ADD MORE LEXICAL FEATURES */
    HW_MP_SP_pHP, 				/* head word, mod POS, sib POS, head left-POS */
    HW_MP_SP_nHP, 				/* head word, mod POS, sib POS, head right-POS */
    HW_MP_SP_pMP, 				/* head word, mod POS, sib POS, mod left-POS */
    HW_MP_SP_nMP, 				/* head word, mod POS, sib POS, mod right-POS */
    HW_MP_SP_pSP, 				/* head word, mod POS, sib POS, sib left-POS */
    HW_MP_SP_nSP, 				/* head word, mod POS, sib POS, sib right-POS */
    HP_MW_SP_pHP, 				/* head POS, mod word, sib POS, head left-POS */
    HP_MW_SP_nHP, 				/* head POS, mod word, sib POS, head right-POS */
    HP_MW_SP_pMP, 				/* head POS, mod word, sib POS, mod left-POS */
    HP_MW_SP_nMP, 				/* head POS, mod word, sib POS, mod right-POS */
    HP_MW_SP_pSP, 				/* head POS, mod word, sib POS, sib left-POS */
    HP_MW_SP_nSP, 				/* head POS, mod word, sib POS, sib right-POS */
    HP_MP_SW_pHP, 				/* head POS, mod POS, sib word, head left-POS */
    HP_MP_SW_nHP, 				/* head POS, mod POS, sib word, head right-POS */
    HP_MP_SW_pMP, 				/* head POS, mod POS, sib word, mod left-POS */
    HP_MP_SW_nMP, 				/* head POS, mod POS, sib word, mod right-POS */
    HP_MP_SW_pSP, 				/* head POS, mod POS, sib word, sib left-POS */
    HP_MP_SW_nSP, 				/* head POS, mod POS, sib word, sib right-POS */

    COUNT
  };
};

struct DependencyFeatureTemplateGrandparent {
  enum types {
    // There is a cross-product between these and direction, pos, cpos
    BIAS = 0,				/* bias */

    // grandpar-head-modifier features
    GP_HP_MP,				/* grandpar POS, head POS, mod POS */
    GW_HP_MP,				/* grandpar word, head POS, mod POS */
    GP_HW_MP,				/* grandpar POS, head word, mod POS */
    GP_HP_MW,				/* grandpar POS, head POS, mod word */
    GW_HW_MP,				/* grandpar word, head word, mod POS */
    GW_HP_MW,				/* grandpar word, head POS, mod word */
    GP_HW_MW,				/* grandpar POS, head word, mod word */
    GW_HW_MW,				/* grandpar word, head word, mod word */

    GP_HP,				/* grandpar POS, head POS */
    GW_HP,				/* grandpar word, head POS */
    GP_HW,				/* grandpar POS, head word */
    GW_HW,				/* grandpar word, head word */
    GP_MP,				/* grandpar POS, mod POS */
    GW_MP,				/* grandpar word, mod POS */
    GP_MW,				/* grandpar POS, mod word */
    GW_MW,				/* grandpar word, mod word */
    HP_MP,				/* head POS, mod POS */
    HW_MP,				/* head word, mod POS */
    HP_MW,				/* head POS, mod word */
    HW_MW,				/* head word, mod word */


    // Contextual features
    GP_HP_MP_pGP, 				/* grandpar POS, head POS, mod POS, grandpar left-POS */
    GP_HP_MP_nGP, 				/* grandpar POS, head POS, mod POS, grandpar right-POS */
    GP_HP_MP_pHP, 				/* grandpar POS, head POS, mod POS, head left-POS */
    GP_HP_MP_nHP, 				/* grandpar POS, head POS, mod POS, head right-POS */
    GP_HP_MP_pMP, 				/* grandpar POS, head POS, mod POS, mod left-POS */
    GP_HP_MP_nMP, 				/* grandpar POS, head POS, mod POS, mod right-POS */
    GP_HP_MP_pGP_pHP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, head left-POS */
    GP_HP_MP_pGP_pMP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, mod left-POS */
    GP_HP_MP_pHP_pMP, 			/* grandpar POS, head POS, mod POS, head left-POS, mod left-POS */
    GP_HP_MP_nGP_nHP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, head right-POS */
    GP_HP_MP_nGP_nMP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, mod right-POS */
    GP_HP_MP_nHP_nMP, 			/* grandpar POS, head POS, mod POS, head right-POS, mod right-POS */
    GP_HP_MP_pGP_nHP, 			/* grandpar POS, head POS, mod POS, geandpar left-POS, head right-POS */
    GP_HP_MP_pGP_nMP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, mod right-POS */
    GP_HP_MP_pHP_nMP, 			/* grandpar POS, head POS, mod POS, head left-POS, mod right-POS */
    GP_HP_MP_nGP_pHP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, head left-POS */
    GP_HP_MP_nGP_pMP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, mod left-POS */
    GP_HP_MP_nHP_pMP, 			/* grandpar POS, head POS, mod POS, head right-POS, mod left-POS */
    GP_HP_MP_pGP_pHP_pMP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, head left-POS, mod left-POS */
    GP_HP_MP_nGP_pHP_pMP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, head left-POS, mod left-POS */
    GP_HP_MP_pGP_nHP_pMP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, head right-POS, mod left-POS */
    GP_HP_MP_nGP_nHP_pMP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, head right-POS, mod left-POS */
    GP_HP_MP_pGP_pHP_nMP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, head left-POS, mod right-POS */
    GP_HP_MP_nGP_pHP_nMP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, head left-POS, mod right-POS */
    GP_HP_MP_pGP_nHP_nMP, 			/* grandpar POS, head POS, mod POS, grandpar left-POS, head right-POS, mod right-POS */
    GP_HP_MP_nGP_nHP_nMP, 			/* grandpar POS, head POS, mod POS, grandpar right-POS, head right-POS, mod right-POS */

    //		/* COULD ADD MORE LEXICAL FEATURES */
    GW_HP_MP_pGP, 				/* grandpar word, head POS, mod POS, grandpar left-POS */
    GW_HP_MP_nGP, 				/* grandpar word, head POS, mod POS, grandpar right-POS */
    GW_HP_MP_pHP, 				/* grandpar word, head POS, mod POS, head left-POS */
    GW_HP_MP_nHP, 				/* grandpar word, head POS, mod POS, head right-POS */
    GW_HP_MP_pMP, 				/* grandpar word, head POS, mod POS, mod left-POS */
    GW_HP_MP_nMP, 				/* grandpar word, head POS, mod POS, mod right-POS */
    GP_HW_MP_pGP, 				/* grandpar POS, head word, mod POS, grandpar left-POS */
    GP_HW_MP_nGP, 				/* grandpar POS, head word, mod POS, grandpar right-POS */
    GP_HW_MP_pHP, 				/* grandpar POS, head word, mod POS, head left-POS */
    GP_HW_MP_nHP, 				/* grandpar POS, head word, mod POS, head right-POS */
    GP_HW_MP_pMP, 				/* grandpar POS, head word, mod POS, mod left-POS */
    GP_HW_MP_nMP, 				/* grandpar POS, head word, mod POS, mod right-POS */
    GP_HP_MW_pGP, 				/* grandpar POS, head POS, mod word, grandpar left-POS */
    GP_HP_MW_nGP, 				/* grandpar POS, head POS, mod word, grandpar right-POS */
    GP_HP_MW_pHP, 				/* grandpar POS, head POS, mod word, head left-POS */
    GP_HP_MW_nHP, 				/* grandpar POS, head POS, mod word, head right-POS */
    GP_HP_MW_pMP, 				/* grandpar POS, head POS, mod word, mod left-POS */
    GP_HP_MW_nMP, 				/* grandpar POS, head POS, mod word, mod right-POS */

    COUNT
  };
};

struct DependencyFeatureTemplateGrandSibl {
  enum types {
    // There is a cross-product between these and direction, pos, cpos
    BIAS = 0,				/* bias */

    // Quadruplet features.
    GP_HP_MP_SP,			/* grandpar POS, head POS, mod POS, sib POS */
    GW_HP_MP_SP,			/* grandpar word, head POS, mod POS, sib POS */
    GP_HW_MP_SP,			/* grandpar POS, head word, mod POS, sib POS */
    GP_HP_MW_SP,			/* grandpar POS, head POS, mod word, sib POS */
    GP_HP_MP_SW,			/* grandpar POS, head POS, mod POS, sib word */

    // TODO: features to deal with conjunctions.
    COUNT
  };
};

struct DependencyFeatureTemplateTriSibl {
  enum types {
    // There is a cross-product between these and direction, pos, cpos
    BIAS = 0,				/* bias */

    // Quadruplet features.
    HP_MP_SP_TP,			/* head POS, mod POS, sib POS, other sib POS */
    HW_MP_SP_TP,			/* head word, mod POS, sib POS, other sib POS */
    HP_MW_SP_TP,			/* head POS, mod word, sib POS, other sib POS */
    HP_MP_SW_TP,			/* head POS, mod POS, sib word, other sib POS */
    HP_MP_SP_TW,			/* head POS, mod POS, sib POS, other sib word */

    // Triplet features.
    HP_MP_TP,			/* head POS, mod POS, other sib POS */
    MP_SP_TP,			/* mod POS, sib POS, other sib POS */
    HW_MP_TP,			/* head word, mod POS, other sib POS */
    HP_MW_TP,			/* head POS, mod word, other sib POS */
    HP_MP_TW,			/* head POS, mod POS, other sib word */
    MW_SP_TP,			/* mod word, sib POS, other sib POS */
    MP_SW_TP,			/* mod POS, sib word, other sib POS */
    MP_SP_TW,			/* mod POS, sib POS, other sib word */

    // Pairwise features.
    MP_TP,			/* mod POS, other sib POS */
    MW_TP,			/* mod word, other sib POS */
    MP_TW,			/* mod POS, other sib word */

    // TODO: features to deal with conjunctions.
    COUNT
  };
};

struct DependencyFeatureTemplateNonprojArc {
  enum types {
    // There is a cross-product between these and direction, distance, {word form}, {pos cpos}
    // Features for head and modifier [add prefixes and suffixes]
    BIAS = 0,				/* bias */
    DIST,					/* exact distance */
    BFLAG,					/* X-between flag */
    HP,						/* head POS */
    HW,						/* head word */
    HWP,					/* head word and POS */
    MP,						/* modifier POS */
    MW,						/* modifier word */
    MWP,					/* modifier word and POS */
    HP_MP,					/* head POS, modifier POS */
    HP_MW,					/* head POS, modifier word */
    HP_MWP,					/* head POS, modifier word and POS */
    HW_MW,					/* head word, modifier word */
    HW_MP,					/* head word, modifier POS */
    HWP_MP,					/* head word and POS, modifier POS */
    HWP_MWP,				/* head word and POS, modifier word and POS */
    HP_MP_BFLAG,			/* head POS, modifier POS,  X-between flag */
    HW_MW_BFLAG,			/* head word, modifier word,  X-between flag */

    HP_MA,					/* head POS, modifier prefix */
    HP_MAP,					/* head POS, modifier prefix and POS */
    HW_MA,					/* head word, modifier prefix */
    HW_MAP,					/* head word, modifier prefix and POS */
    HA_MP,					/* head prefix, modifier POS */
    HAP_MP,					/* head prefix and POS, modifier POS */
    HA_MW,					/* head prefix, modifier word */
    HAP_MW,					/* head prefix and POS, modifier word */
    HP_MZ,					/* head POS, modifier suffix */
    HP_MZP,					/* head POS, modifier suffix and POS */
    HW_MZ,					/* head word, modifier suffix */
    HW_MZP,					/* head word, modifier suffix and POS */
    HZ_MP,					/* head suffix, modifier POS */
    HZP_MP,					/* head suffix and POS, modifier POS */
    HZ_MW,					/* head suffix, modifier word */
    HZP_MW,					/* head suffix and POS, modifier word */

    HF,						/* head feature */
    HFP,					/* head feature and POS */
    MF,						/* modifier feature */
    MFP,					/* modifier feature and POS */
    HF_MF,					/* head feature, modifier feature */
    HF_MP,					/* head feature, modifier POS */
    HF_MFP,					/* head feature, modifier feature and POS */
    HP_MF,					/* head POS, modifier feature */
    HFP_MF,					/* head feature and POS, modifier feature */
    HFP_MFP,				/* head feature and POS, modifier feature and POS */

    // Contextual features
    HP_MP_pHP, 				/* head POS, mod POS, head left-POS */
    HP_MP_nHP, 				/* head POS, mod POS, head right-POS */
    HP_MP_pMP, 				/* head POS, mod POS, mod left-POS */
    HP_MP_nMP, 				/* head POS, mod POS, mod right-POS */
    HP_MP_pHP_pMP, 			/* head POS, mod POS, head left-POS, mod left-POS */
    HP_MP_nHP_nMP, 			/* head POS, mod POS, head right-POS, mod right-POS */
    HP_MP_pHP_nMP, 			/* head POS, mod POS, head left-POS, mod right-POS */
    HP_MP_nHP_pMP, 			/* head POS, mod POS, head right-POS, mod left-POS */
    HP_MP_pHP_nHP_pMP_nMP,  /* head POS, mod POS, head left-POS, head right-POS, mod left-POS, mod right-POS */

    HW_MP_pHP, 				/* head word, mod POS, head left-POS */
    HW_MP_nHP, 				/* head word, mod POS, head right-POS */
    HW_MP_pMP, 				/* head word, mod POS, mod left-POS */
    HW_MP_nMP, 				/* head word, mod POS, mod right-POS */
    HW_MP_pHP_pMP, 			/* head word, mod POS, head left-POS, mod left-POS */
    HW_MP_nHP_nMP, 			/* head word, mod POS, head right-POS, mod right-POS */
    HW_MP_pHP_nMP, 			/* head word, mod POS, head left-POS, mod right-POS */
    HW_MP_nHP_pMP, 			/* head word, mod POS, head right-POS, mod left-POS */

    HP_MW_pHP, 				/* head POS, mod word, head left-POS */
    HP_MW_nHP, 				/* head POS, mod word, head right-POS */
    HP_MW_pMP, 				/* head POS, mod word, mod left-POS */
    HP_MW_nMP, 				/* head POS, mod word, mod right-POS */
    HP_MW_pHP_pMP, 			/* head POS, mod word, head left-POS, mod left-POS */
    HP_MW_nHP_nMP, 			/* head POS, mod word, head right-POS, mod right-POS */
    HP_MW_pHP_nMP, 			/* head POS, mod word, head left-POS, mod right-POS */
    HP_MW_nHP_pMP, 			/* head POS, mod word, head right-POS, mod left-POS */

    HW_MW_pHP, 				/* head word, mod word, head left-POS */
    HW_MW_nHP, 				/* head word, mod word, head right-POS */
    HW_MW_pMP, 				/* head word, mod word, mod left-POS */
    HW_MW_nMP, 				/* head word, mod word, mod right-POS */
    HW_MW_pHP_pMP, 			/* head word, mod word, head left-POS, mod left-POS */
    HW_MW_nHP_nMP, 			/* head word, mod word, head right-POS, mod right-POS */
    HW_MW_pHP_nMP, 			/* head word, mod word, head left-POS, mod right-POS */
    HW_MW_nHP_pMP, 			/* head word, mod word, head right-POS, mod left-POS */

    HP_MP_pHW, 				/* head POS, mod POS, head left-word */
    HP_MP_nHW, 				/* head POS, mod POS, head right-word */
    HP_MP_pMW, 				/* head POS, mod POS, mod left-word */
    HP_MP_nMW, 				/* head POS, mod POS, mod right-word */
    HW_MP_pHW, 				/* head word, mod POS, head left-word */
    HW_MP_nHW, 				/* head word, mod POS, head right-word */
    HW_MP_pMW, 				/* head word, mod POS, mod left-word */
    HW_MP_nMW, 				/* head word, mod POS, mod right-word */
    HP_MW_pHW, 				/* head POS, mod word, head left-word */
    HP_MW_nHW, 				/* head POS, mod word, head right-word */
    HP_MW_pMW, 				/* head POS, mod word, mod left-word */
    HP_MW_nMW, 				/* head POS, mod word, mod right-word */
    HW_MW_pHW, 				/* head word, mod word, head left-word */
    HW_MW_nHW, 				/* head word, mod word, head right-word */
    HW_MW_pMW, 				/* head word, mod word, mod left-word */
    HW_MW_nMW, 				/* head word, mod word, mod right-word */

    // In-between features
    HP_MP_BP, 				/* head POS, mod POS, in-between POS */
    HW_MW_BP, 				/* head word, mod word, in-between POS */
    HW_MP_BP, 				/* head word, mod POS, in-between POS */
    HP_MW_BP, 				/* head POS, mod word, in-between POS */
    HP_MP_BW, 				/* head POS, mod POS, in-between word */
    HW_MW_BW, 				/* head word, mod word, in-between word */
    SHAPE,					/* shape features */

    COUNT
  };
};

struct DependencyFeatureTemplatePath {
  enum types {
    // There is a cross-product between these and direction, distance, {word form}, {pos cpos}
    // Features for head and modifier [add prefixes and suffixes]
    BIAS = 0,				/* bias */
    DIST,					/* exact distance */
    AP,						/* ancestor POS */
    AW,						/* ancestor word */
    AWP,					/* ancestor word and POS */
    DP,						/* descendant POS */
    DW,						/* descendant word */
    DWP,					/* descendant word and POS */
    AP_DP,					/* ancestor POS, descendent POS */
    AP_DW,					/* ancestor POS, descendent word */
    AP_DWP,					/* ancestor POS, descendent word and POS */
    AW_DW,					/* ancestor word, descendent word */
    AW_DP,					/* ancestor word, descendent POS */
    AWP_DP,					/* ancestor word and POS, descendent POS */
    AWP_DWP,				/* ancestor word and POS, descendent word and POS */


    AF,						/* ancestor feature */
    AFP,					/* ancestor feature and POS */
    DF,						/* descendent feature */
    DFP,					/* descendent feature and POS */
    AF_DF,					/* ancestor feature, descendent feature */
    AF_DP,					/* ancestor feature, descendent POS */
    AF_DFP,					/* ancestor feature, descendent feature and POS */
    AP_DF,					/* ancestor POS, descendent feature */
    AFP_DF,					/* ancestor feature and POS, descendent feature */
    AFP_DFP,				/* ancestor feature and POS, descendent feature and POS */


    // In-between features
    AP_DP_BP, 				/* ancestor POS, descendent POS, in-between POS */
    AW_DW_BP, 				/* ancestor word, descendent word, in-between POS */
    AW_DP_BP, 				/* ancestor word, descendent POS, in-between POS */
    AP_DW_BP, 				/* ancestor POS, descendent word, in-between POS */
    AP_DP_BW, 				/* ancestor POS, descendent POS, in-between word */
    AW_DW_BW, 				/* ancestor word, descendent word, in-between word */
    SHAPE,					/* shape features */

    COUNT
  };
};

struct DependencyFeatureTemplateBigram {
  enum types {
    // There is a cross-product between these and directions, pos, cpos,
    // a flag indicating if (1) J = H, (2) J = M, (3) H = pM?
    BIAS = 0,				/* bias */
    pDIST_DIST,					/* prev distance, distance */

    pMP_MP_pDIST_DIST, 			/* pos, prev pos, prev distance, distance */

    // prev head-prev mod-head-modifier features
    JP_pMP_HP_MP,				/* prev head POS, prev POS, head POS, mod POS */

    JW_pMP_HP_MP,				/* prev head word, prev POS, head POS, mod POS */
    JP_pMW_HP_MP,				/* prev head POS, prev word, head POS, mod POS */
    JP_pMP_HW_MP,				/* prev head POS, prev POS, head word, mod POS */
    JP_pMP_HP_MW,				/* prev head POS, prev POS, head POS, mod word */

    JW_pMP_HW_MP,				/* prev head word, prev POS, head word, mod POS */
    JW_pMP_HP_MW,				/* prev head word, prev POS, head POS, mod word */
    JP_pMW_HP_MW,				/* prev head POS, prev word, head POS, mod word */
    JP_pMW_HW_MP,				/* prev head POS, prev word, head word, mod POS */

    JP_HP,						/* prev head POS, head POS */
    JW_HP,						/* prev head word, head POS */
    JP_HW,						/* prev head POS, head word */
    JW_HW,						/* prev head word, head word */

    COUNT
  };
};

#endif /* DEPENDENCYFEATURETEMPLATES_H_ */
