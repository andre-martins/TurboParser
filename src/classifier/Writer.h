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

#ifndef WRITER_H_
#define WRITER_H_

#include "Instance.h"
#include <fstream>
using namespace std;

// Abstract class for the writer. Task-specific parts should derive
// from this class and implement the pure virtual methods.
// The writer writes instances to a file.
class Writer {
 public:
  Writer() {};
  virtual ~Writer() {};

 public:
  virtual void Open(const string &filepath);
  virtual void Close();
  virtual void Write(Instance *instance) = 0;

protected:
  ofstream os_;
};

#endif /* SHWRITER_H_ */

