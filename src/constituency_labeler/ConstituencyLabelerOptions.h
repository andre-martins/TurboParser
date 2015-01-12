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

#ifndef CONSTITUENCY_LABELER_OPTIONS_H_
#define CONSTITUENCY_LABELER_OPTIONS_H_

#include "Options.h"

class ConstituencyLabelerOptions : public Options {
 public:
  ConstituencyLabelerOptions() {};
  virtual ~ConstituencyLabelerOptions() {};

  // Serialization functions.
  virtual void Load(FILE* fs);
  virtual void Save(FILE* fs);

  // Initialization: set options based on the flags.
  virtual void Initialize();

  // Get option flags.
  bool prune_labels() { return prune_labels_; }
  const std::string &null_label() { return null_label_; }
  bool ignore_null_labels() { return ignore_null_labels_; }

 protected:
  std::string file_format_;
  bool prune_labels_;
  std::string null_label_;
  bool ignore_null_labels_;
};

#endif // CONSTITUENCY_LABELER_OPTIONS_H_
