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

#ifndef MORPHOLOGICALFEATURETEMPLATES_H_
#define MORPHOLOGICALFEATURETEMPLATES_H_

struct MorphologicalFeatureTemplateParts {
  enum types {
    UNIGRAM = 0,
    BIGRAM,
    TRIGRAM,
  };
};

struct MorphologicalFeatureTemplateUnigram {
  enum types {
    BIAS = 0,   /* bias */

    W,          /* word */
    pW,         /* word on the left (previous) */
    nW,         /* word on the right (next) */
    ppW,        /* word two positions on the left (previous) */
    nnW,        /* word two positions on the right (next) */

    Z,          /* suffix n letters */
    pZ,         /* suffix n letters word on the left (previous) */
    ppZ,        /* suffix n letters word two positions on the left (previous) */
    nZ,         /* suffix n letters word on the right (next) */
    nnZ,        /* suffix n letters word two positions on the right (next) */
    A,          /* prefix n letters */
    pA,          /* prefix n letters word on the left (previous) */
    ppA,        /* prefix n letters word two positions on the left (previous) */
    nA,          /* prefix n letters word on the right (next) */
    nnA,        /* prefix n letters word two positions on the right (next) */

    WP,          /* word + POS tag */

    P,          /* POS */                               // 'cpostag'
    pP,         /* POS on the left */                   // 'cpostag_minusone'
    nP,         /* POS on the right */                  // 'cpostag_plusone'
    ppP,        /* POS two positions on the left */     // 'cpostag_minustwo'
    nnP,        /* POS two positions on the right */    // 'cpostag_plustwo'
    PpP,        /* POS + POS on the left */             // 'cpostag_bigram'
    PnP,        /* POS + POS on the right */            //
    PpPppP,     /* POS trigram on the left */           // 'cpostag_trigram'
    PnPnnP,     /* POS trigram on the right */          //
    PpPnP,       /* POS trigram on the center */        //

    S,          /* shape */
    pS,         /* shape on the left */
    nS,         /* shape on the right */
    ppS,        /* shape two positions on the left */
    nnS,        /* shape two positions on the right */

    FLAG,       /* flag indicating presence of special characters */
    COUNT
  };
};

struct MorphologicalFeatureTemplateBigram {
  enum types {
    BIAS = 0,   /* bias */
  };
};

struct MorphologicalFeatureTemplateTrigram {
  enum types {
    BIAS = 0,   /* bias */
  };
};

#endif /* MORPHOLOGICALFEATURETEMPLATES_H_ */
