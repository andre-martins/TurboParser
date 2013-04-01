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

#ifndef SEQUENCE_OPTIONS_H_
#define SEQUENCE_OPTIONS_H_

#include "Options.h"

class SequenceOptions : public Options {
 public:
  SequenceOptions() {};
  virtual ~SequenceOptions() {};

  // Serialization functions.
  void Load(FILE* fs);
  void Save(FILE* fs);

  // Initialization: set options based on the flags.
  void Initialize();

  // Get option flags.
  bool large_feature_set() { return large_feature_set_; }
  bool prune_tags() { return prune_tags_; }
  int markov_order() { return model_type_; }
  const string &GetUnknownWordTagsFilePath() {
    return file_unknown_word_tags_;
  }

 protected:
  string file_format_;
  int model_type_;
  bool large_feature_set_;
  bool prune_tags_;
  string file_unknown_word_tags_;
};

#endif // SEQUENCE_OPTIONS_H_
