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

#ifndef ENTITY_OPTIONS_H_
#define ENTITY_OPTIONS_H_

#include "SequenceOptions.h"

struct EntityTaggingSchemes {
  enum {
    IO = 0,
    BIO,
    BILOU
  };
};

class EntityOptions : public SequenceOptions {
 public:
  EntityOptions() {};
  virtual ~EntityOptions() {};

  // Serialization functions.
  void Load(FILE* fs);
  void Save(FILE* fs);

  // Initialization: set options based on the flags.
  void Initialize();

  // Get option flags.
  int tagging_scheme() { return tagging_scheme_; }
  const std::string &file_gazetteer() { return file_gazetteer_; }

 protected:
  std::string file_format_;
  std::string tagging_scheme_name_;
  std::string file_gazetteer_;
  int tagging_scheme_;
};

#endif // ENTITY_OPTIONS_H_
