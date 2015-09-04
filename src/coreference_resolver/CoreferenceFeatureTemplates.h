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

#ifndef COREFERENCEFEATURETEMPLATES_H_
#define COREFERENCEFEATURETEMPLATES_H_

struct CoreferenceFeatureTemplateParts {
  enum types {
    ARC = 0,
  };
};

struct CoreferenceFeatureTemplateArc {
  enum types {
    BIAS = 0,       /* bias */

    Cl,             /* mention child length */
    Cl_Ct,          /* mention child length, child type */
    Cl_Ct_Pt,       /* mention child length, child type, parent type */
    Pl,             /* mention parent length */
    Pl_Ct,          /* mention parent length, child type */
    Pl_Ct_Pt,       /* mention parent length, child type, parent type */

    CW,             /* mention child head word */
    CW_Ct,          /* mention child head word, child type */
    CW_Ct_Pt,       /* mention child head word, child type, parent type */
    PW,             /* mention parent head word */
    PW_Ct,          /* mention parent head word, child type */
    PW_Ct_Pt,       /* mention parent head word, child type, parent type */

    CfW,             /* mention child first word */
    CfW_Ct,          /* mention child first word, child type */
    CfW_Ct_Pt,       /* mention child first word, child type, parent type */
    PfW,             /* mention parent first word */
    PfW_Ct,          /* mention parent first word, child type */
    PfW_Ct_Pt,       /* mention parent first word, child type, parent type */

    ClW,             /* mention child last word */
    ClW_Ct,          /* mention child last word, child type */
    ClW_Ct_Pt,       /* mention child last word, child type, parent type */
    PlW,             /* mention parent last word */
    PlW_Ct,          /* mention parent last word, child type */
    PlW_Ct_Pt,       /* mention parent last word, child type, parent type */

    CpW,             /* mention child preceding word */
    CpW_Ct,          /* mention child preceding word, child type */
    CpW_Ct_Pt,       /* mention child preceding word, child type, parent type */
    PpW,             /* mention parent preceding word */
    PpW_Ct,          /* mention parent preceding word, child type */
    PpW_Ct_Pt,       /* mention parent preceding word, child type, parent type */

    CnW,             /* mention child next word */
    CnW_Ct,          /* mention child next word, child type */
    CnW_Ct_Pt,       /* mention child next word, child type, parent type */
    PnW,             /* mention parent next word */
    PnW_Ct,          /* mention parent next word, child type */
    PnW_Ct_Pt,       /* mention parent next word, child type, parent type */

    CP,             /* mention child head tag */
    CP_Ct,          /* mention child head tag, child type */
    CP_Ct_Pt,       /* mention child head tag, child type, parent type */
    PP,             /* mention parent head tag */
    PP_Ct,          /* mention parent head tag, child type */
    PP_Ct_Pt,       /* mention parent head tag, child type, parent type */

    CfP,             /* mention child first tag */
    CfP_Ct,          /* mention child first tag, child type */
    CfP_Ct_Pt,       /* mention child first tag, child type, parent type */
    PfP,             /* mention parent first tag */
    PfP_Ct,          /* mention parent first tag, child type */
    PfP_Ct_Pt,       /* mention parent first tag, child type, parent type */

    ClP,             /* mention child last tag */
    ClP_Ct,          /* mention child last tag, child type */
    ClP_Ct_Pt,       /* mention child last tag, child type, parent type */
    PlP,             /* mention parent last tag */
    PlP_Ct,          /* mention parent last tag, child type */
    PlP_Ct_Pt,       /* mention parent last tag, child type, parent type */

    CpP,             /* mention child preceding tag */
    CpP_Ct,          /* mention child preceding tag, child type */
    CpP_Ct_Pt,       /* mention child preceding tag, child type, parent type */
    PpP,             /* mention parent preceding tag */
    PpP_Ct,          /* mention parent preceding tag, child type */
    PpP_Ct_Pt,       /* mention parent preceding tag, child type, parent type */

    CnP,             /* mention child next tag */
    CnP_Ct,          /* mention child next tag, child type */
    CnP_Ct_Pt,       /* mention child next tag, child type, parent type */
    PnP,             /* mention parent next tag */
    PnP_Ct,          /* mention parent next tag, child type */
    PnP_Ct_Pt,       /* mention parent next tag, child type, parent type */

    Pg,             /* mention parent gender */
    Pg_Ct,          /* mention parent gender, child type */
    Pg_Ct_Pt,       /* mention parent gender, child type, parent type */

    Pn,             /* mention parent number */
    Pn_Ct,          /* mention parent number, child type */
    Pn_Ct_Pt,       /* mention parent number, child type, parent type */

    md,             /* mention distance between mentions */
    md_Ct,          /* mention distance between mentions, child type */
    md_Ct_Pt,       /* mention distance between mentions, child type, parent type */

    sd,             /* sentence distance between mentions */
    sd_Ct,          /* sentence distance between mentions, child type */
    sd_Ct_Pt,       /* sentence distance between mentions, child type, parent type */

    em,             /* exact match between mentions */
    em_Ct,          /* exact match between mentions, child type */
    em_Ct_Pt,       /* exact match between mentions, child type, parent type */

    hm,             /* head match between mentions */
    hm_Ct,          /* head match between mentions, child type */
    hm_Ct_Pt,       /* head match between mentions, child type, parent type */

    COUNT
  };
};

#endif /* COREFERENCEFEATURETEMPLATES_H_ */
