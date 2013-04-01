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

#ifndef FEATURES_H_
#define FEATURES_H_

#include "Part.h"
#include "Instance.h"
#ifdef _WIN32
#include <stdint.h>
#endif

// A vector of binary features. Each feature is represented as a 64-bit key.
typedef vector<uint64_t> BinaryFeatures;

class Pipe;

// Abstract class for the feature handler. Task-specific handlers should derive
// from this class and implement the pure virtual methods.
class Features {
 public:
  Features() {};
  virtual ~Features() {};

 public:
  // Set a pointer to the pipe that owns this feature handler.
  void SetPipe(Pipe *pipe) { pipe_ = pipe; };

  // Get the binary features corresponding to the r-th part.
  virtual const BinaryFeatures &GetPartFeatures(int r) const = 0;
  // Get the binary features corresponding to the r-th part (mutable).
  virtual BinaryFeatures *GetMutablePartFeatures(int r) const = 0;

 protected:
  Pipe *pipe_; // The pipe that owns this feature handler.
};

#endif /* FEATURES_H_ */
