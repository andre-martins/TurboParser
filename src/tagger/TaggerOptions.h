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

#ifndef TAGGER_OPTIONS_H_
#define TAGGER_OPTIONS_H_

#include "SequenceOptions.h"

class TaggerOptions : public SequenceOptions {
 public:
  TaggerOptions() {};
  virtual ~TaggerOptions() {};

  // Serialization functions.
  void Load(FILE* fs);
  void Save(FILE* fs);

  // Initialization: set options based on the flags.
  void Initialize();

  // Get option flags.
  bool prune_tags() { return prune_tags_; }
  bool large_feature_set() { return large_feature_set_; }
  const string &GetUnknownWordTagsFilePath() {
    return file_unknown_word_tags_;
  }

 protected:
  bool prune_tags_;
  string file_format_;
  bool large_feature_set_;
  string file_unknown_word_tags_;
};

#endif // TAGGER_OPTIONS_H_
