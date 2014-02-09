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

#include "SemanticInstance.h"

void SemanticInstance::Initialize(const vector<string> &forms,
                                  const vector<string> &lemmas,
                                  const vector<string> &cpos,
                                  const vector<string> &pos,
                                  const vector<vector<string> > &feats,
                                  const vector<string> &deprels,
                                  const vector<int> &heads,
                                  const vector<string> &predicate_names,
                                  const vector<int> &predicate_indices,
                                  const vector<vector<string> > &argument_roles,
                                  const vector<vector<int> > &argument_indices) {
  DependencyInstance::Initialize(forms, lemmas, cpos, pos, feats, deprels,
                                 heads);
  predicate_names_ = predicate_names;
  predicate_indices_ = predicate_indices;
  argument_roles_ = argument_roles;
  argument_indices_ = argument_indices;
}
