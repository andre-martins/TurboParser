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

#ifndef ENTITYFEATURETEMPLATES_H_
#define ENTITYFEATURETEMPLATES_H_

struct EntityFeatureTemplateParts {
  enum types {
    UNIGRAM = 0,
    BIGRAM,
    TRIGRAM,
  };
};

struct EntityFeatureTemplateUnigram {
  enum types {
    BIAS = 0,   /* bias */                                            //WORD
    W,          /* word */                                            //WORD
    pW,         /* word on the left */                                //CONTEXT
    nW,         /* word on the right */                               //CONTEXT
    ppW,        /* word two positions on the left */                  //CONTEXT
    nnW,        /* word two positions on the right */                 //CONTEXT
    G,          /* gazetteer */                                       //WORD
    pG,         /* gazetteer on the left */                           //CONTEXT
    nG,         /* gazetteer on the right */                          //CONTEXT
    ppG,        /* gazetteer two positions on the left */             //CONTEXT
    nnG,        /* gazetteer two positions on the right */            //CONTEXT
    P,          /* POS */                                             //WORD
    PpP,        /* POS + POS on the left */                           //CONTEXT
    PnP,        /* POS + POS on the right */                          //CONTEXT
    PpPppP,     /* POS trigram on the left */                         //CONTEXT
    PnPnnP,     /* POS trigram on the right */                        //CONTEXT
    PpPnP,      /* POS trigram on the center */                       //CONTEXT
    S,          /* shape */                                           //WORD
    pS,         /* shape on the left */                               //CONTEXT
    nS,         /* shape on the right */                              //CONTEXT
    ppS,        /* shape two positions on the left */                 //CONTEXT
    nnS,        /* shape two positions on the right */                //CONTEXT
    A,          /* prefix */                                          //WORD
    Z,          /* suffix */                                          //WORD
    FLAG,       /* flag indicating presence of special characters */  //WORD
    COUNT
  };
};

struct EntityFeatureTemplateBigram {
  enum types {
    BIAS = 0,   /* bias */
    W,          /* word */
    pW,         /* word on the left */
    nW,         /* word on the right */
    ppW,        /* word two positions on the left */
    nnW,        /* word two positions on the right */
    P,          /* POS */
    PpP,        /* POS + POS on the left */
    PnP,        /* POS + POS on the right */
    PpPppP,     /* POS trigram on the left */
    PnPnnP,     /* POS trigram on the right */
    PpPnP,      /* POS trigram on the center */
    S,          /* shape */
    pS,         /* shape on the left */
    nS,         /* shape on the right */
    ppS,        /* shape two positions on the left */
    nnS,        /* shape two positions on the right */
    COUNT
  };
};

struct EntityFeatureTemplateTrigram {
  enum types {
    BIAS = 0,   /* bias */
  };
};

#endif /* ENTITYFEATURETEMPLATES_H_ */
