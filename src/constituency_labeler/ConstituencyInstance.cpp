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

#include "ConstituencyInstance.h"
#include <glog/logging.h>

void ConstituencyInstance::Initialize(
    const std::vector<std::string> &forms,
    const std::vector<std::string> &lemmas,
    const std::vector<std::string> &tags,
    const std::vector<std::vector<std::string> > &morph,
    const ParseTree &parse_tree) {
  SequenceInstance::Initialize(forms, tags);
  lemmas_ = lemmas;
  morph_ = morph;
  std::string info;
  parse_tree.SaveToString(&info);
  parse_tree_.LoadFromString(info);
}

