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

#ifndef CONSTITUENCYLABELERFEATURETEMPLATES_H_
#define CONSTITUENCYLABELERFEATURETEMPLATES_H_

struct ConstituencyLabelerFeatureTemplateParts {
  enum types {
    NODE = 0
  };
};

struct ConstituencyLabelerFeatureTemplateNode {
  enum types {
    // There is a cross-product between these features and an indicator of
    // whether the node is pre-terminal.
    // Features for a constituent node.
    BIAS = 0,  // Bias.
    RULEUP_CHIDX, // Production rule above and child index.
    RULEDOWN,  // Production rule below.
    CID,       // Current node.
    CID_pCID,  // Current node, previous sibling node.
    CID_nCID,  // Current node, next sibling node.
    CID_PCID,  // Current node, parent node.
    CID_WID,   // Current node and its word (terminal nodes only).
    CID_LID,   // Current node and its lemma (terminal nodes only).
    CID_MFID,   // Current node and its morpho-syntactic feature (terminal nodes only).
    CID_CHIDX, // Current node and child index.
    CID_PCID_CHIDX, // Current node, parent node and child index.
    CID_pWID,   // Current node and previous word (if previous node is terminal).
    CID_nWID,   // Current node and next word (if previous node is terminal).
    CID_pLID,   // Current node and previous lemma (if previous node is terminal).
    CID_nLID,   // Current node and next lemma (if previous node is terminal).
    CID_pMFID,   // Current node and previous morpho-syntactic feature (if previous node is terminal).
    CID_nMFID,   // Current node and next morpho-sytactic feature (if previous node is terminal).

    COUNT
  };
};

#endif /* CONSTITUENCYLABELERFEATURETEMPLATES_H_ */
