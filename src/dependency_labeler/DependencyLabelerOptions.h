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

#ifndef DEPENDENCY_LABELER_OPTIONS_H_
#define DEPENDENCY_LABELER_OPTIONS_H_

#include "Options.h"

class DependencyLabelerOptions : public Options {
public:
  DependencyLabelerOptions() {};
  virtual ~DependencyLabelerOptions() {};

  // Serialization functions.
  void Load(FILE* fs);
  void Save(FILE* fs);

  // Initialization: set options based on the flags.
  void Initialize();

  // Get option values.
  //bool projective() { return projective_; }
  bool prune_labels() { return prune_labels_; }
  bool use_sibling_parts() { return use_sibling_parts_; }

protected:
  std::string file_format_;
  //string model_type_;
  //bool projective_;
  bool prune_labels_;
  bool use_sibling_parts_;
};

#endif // DEPENDENCY_LABELER_OPTIONS_H_
