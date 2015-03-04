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

#ifndef PART_H_
#define PART_H_

#include <vector>
#include <glog/logging.h>

using namespace std;

// Abstract class for a part. Task-specific parts should derive
// from this class and implement the pure virtual methods.
class Part {
 public:
  Part() {};
  virtual ~Part() {};

 public:
  virtual int type() = 0;
};

// A vector of parts.
class Parts : public vector<Part*> {
 public:
  Parts() {};
  virtual ~Parts() {};

 public:
  void DeleteAll() {
    for (iterator iter = begin(); iter != end(); iter++) {
      delete (*iter);
    }
    clear();
  }
};

#endif /* PART_H_ */
