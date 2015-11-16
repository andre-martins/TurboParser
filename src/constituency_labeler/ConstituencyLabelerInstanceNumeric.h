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

#ifndef CONSTITUENCYLABELERINSTANCENUMERIC_H_
#define CONSTITUENCYLABELERINSTANCENUMERIC_H_

#include "ConstituencyInstanceNumeric.h"
#include "ConstituencyLabelerInstance.h"
#include "ConstituencyLabelerDictionary.h"

class ConstituencyLabelerInstanceNumeric : public ConstituencyInstanceNumeric {
public:
  ConstituencyLabelerInstanceNumeric() {};
  virtual ~ConstituencyLabelerInstanceNumeric() { Clear(); };

  void Clear() {
    ConstituencyInstanceNumeric::Clear();
  }

  void Initialize(const ConstituencyLabelerDictionary &dictionary,
                  ConstituencyLabelerInstance *instance);

  int GetNumConstituents() { return constituent_labels_.size(); }
  int GetConstituentId(int i) {
    return parse_tree_.non_terminals()[i]->label();
  }
  int GetConstituentLabelId(int i) { return constituent_labels_[i]; }
  const std::vector<int> &GetConstituentLabels() { return constituent_labels_; }

protected:
  std::vector<int> constituent_labels_; // One label per non-terminal.
};

#endif /* CONSTITUENCYLABELERINSTANCENUMERIC_H_ */
