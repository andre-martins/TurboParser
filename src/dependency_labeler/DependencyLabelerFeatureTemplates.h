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

#ifndef DEPENDENCYLABELERFEATURETEMPLATES_H_
#define DEPENDENCYLABELERFEATURETEMPLATES_H_

struct DependencyLabelerFeatureTemplateParts {
  enum types {
    ARC = 0,
  };
};

struct DependencyLabelerFeatureTemplateArc {
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

#endif /* DEPENDENCYLABELERFEATURETEMPLATES_H_ */
