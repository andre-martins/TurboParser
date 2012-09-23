// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.0.
//
// TurboParser 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.0.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SEQUENCEINSTANCE_H_
#define SEQUENCEINSTANCE_H_

#include <string>
#include <vector>
#include "Instance.h"

class SequenceInstance : public Instance {
 public:
  SequenceInstance() {}
  virtual ~SequenceInstance() {}

  void Initialize(const vector<string> &forms,
                  const vector<string> &tags);

  int size() { return forms_.size(); };

  const string &GetForm(int i) { return forms_[i]; }
  const string &GetTag(int i) { return tags_[i]; }

  void SetTag(int i, const string &tag) { tags_[i] = tag; }

 protected:
  vector<string> forms_;
  vector<string> tags_;
};

#endif /* SEQUENCEINSTANCE_H_*/
