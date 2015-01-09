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

#ifndef CONSTITUENCYLABELERINSTANCE_H_
#define CONSTITUENCYLABELERINSTANCE_H_

#include "ConstituencyInstance.h"

class ConstituencyLabelerInstance : public ConstituencyInstance {
 public:
  ConstituencyLabelerInstance() {}
  virtual ~ConstituencyLabelerInstance() {}

  Instance* Copy() {
    ConstituencyLabelerInstance* instance = new ConstituencyLabelerInstance();
    instance->Initialize(forms_, tags_, parse_tree_, constituent_labels_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const std::vector<std::string> &forms,
                  const std::vector<std::string> &tags,
                  const ParseTree &parse_tree,
                  const std::vector<std::string> &constituent_labels);

  int GetNumConstituents() { return constituent_labels_.size(); }
  const std::string &GetConstituentLabel(int i) {
    return constituent_labels_[i];
  }
  const std::vector<std::string> &GetConstituentLabels() {
    return constituent_labels_;
  }

 protected:
  std::vector<std::string> constituent_labels_; // One label per non-terminal.
};

#endif /* CONSTITUENCYLABELERINSTANCE_H_*/
