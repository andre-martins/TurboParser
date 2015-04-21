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

#ifndef SEQUENCEINSTANCE_H_
#define SEQUENCEINSTANCE_H_

#include <string>
#include <vector>
#include "Instance.h"

class SequenceInstance : public Instance {
 public:
  SequenceInstance() {}
  virtual ~SequenceInstance() {}

  virtual Instance* Copy() {
    SequenceInstance* instance = new SequenceInstance();
    instance->Initialize(forms_, tags_);
    return static_cast<Instance*>(instance);
  }

  virtual void Initialize(const std::vector<std::string> &forms,
                          const std::vector<std::string> &tags);

  int size() const { return forms_.size(); };

  const std::string &GetForm(int i) const { return forms_[i]; }
  const std::string &GetTag(int i) const { return tags_[i]; }
  const std::vector<std::string> &forms() const { return forms_; }
  const std::vector<std::string> &tags() const { return tags_; }

  void SetTag(int i, const std::string &tag) { tags_[i] = tag; }

 protected:
  std::vector<std::string> forms_;
  std::vector<std::string> tags_;
};

#endif /* SEQUENCEINSTANCE_H_*/
