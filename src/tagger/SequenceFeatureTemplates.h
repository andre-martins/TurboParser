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

#ifndef SEQUENCEFEATURETEMPLATES_H_
#define SEQUENCEFEATURETEMPLATES_H_

struct SequenceFeatureTemplateParts {
  enum types {
    UNIGRAM = 0,
    BIGRAM,
    TRIGRAM,
  };
};

struct SequenceFeatureTemplateUnigram {
  enum types {
    BIAS = 0,				/* bias */
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

struct SequenceFeatureTemplateBigram {
  enum types {
    BIAS = 0,       /* bias */
  };
};

struct SequenceFeatureTemplateTrigram {
  enum types {
    BIAS = 0,       /* bias */
  };
};

#endif /* SEQUENCEFEATURETEMPLATES_H_ */
