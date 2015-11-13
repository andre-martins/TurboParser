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

#ifndef ENTITYREADER_H_
#define ENTITYREADER_H_

#include "EntityInstance.h"
#include "SequenceReader.h"
#include "Options.h"
#include <fstream>

class EntityReader : public SequenceReader {
public:
  EntityReader() { options_ = NULL; };
  EntityReader(Options *options) { options_ = options; };
  virtual ~EntityReader() {};

public:
  Instance *GetNext();

protected:
  Options *options_;
};

#endif /* ENTITYREADER_H_ */

