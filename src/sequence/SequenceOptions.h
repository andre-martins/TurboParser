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

#ifndef SEQUENCE_OPTIONS_H_
#define SEQUENCE_OPTIONS_H_

#include "Options.h"

class SequenceOptions : public Options {
public:
  SequenceOptions() {};
  virtual ~SequenceOptions() {};

  // Serialization functions.
  virtual void Load(FILE* fs);
  virtual void Save(FILE* fs);

  // Initialization: set options based on the flags.
  virtual void Initialize();

  // Get option flags.
  int markov_order() { return model_type_; }

protected:
  int model_type_;
};

#endif // SEQUENCE_OPTIONS_H_
