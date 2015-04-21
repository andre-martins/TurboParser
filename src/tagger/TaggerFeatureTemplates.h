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

#ifndef TAGGERFEATURETEMPLATES_H_
#define TAGGERFEATURETEMPLATES_H_

struct TaggerFeatureTemplateParts {
  enum types {
    UNIGRAM = 0,
    BIGRAM,
    TRIGRAM,
  };
};

struct TaggerFeatureTemplateUnigram {
  enum types {
    BIAS = 0,       /* bias */
    W,              /* word */
    pW,             /* word on the left */
    nW,             /* word on the right */
    ppW,            /* word two positions on the left */
    nnW,            /* word two positions on the right */
    A,              /* prefix */
    Z,              /* suffix */
    FLAG,           /* flag indicating presence of special characters */
    COUNT
  };
};

struct TaggerFeatureTemplateBigram {
  enum types {
    BIAS = 0,       /* bias */
  };
};

struct TaggerFeatureTemplateTrigram {
  enum types {
    BIAS = 0,       /* bias */
  };
};

#endif /* TAGGERFEATURETEMPLATES_H_ */
