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

#ifndef DICTIONARY_H_
#define DICTIONARY_H_

#include <fstream>

// Abstract class for a dictionary. Task-specific dictionaries should derive
// from this class and implement the pure virtual methods.
class Dictionary {
public:
  Dictionary() {};
  virtual ~Dictionary() {};

  // Clear the dictionary.
  virtual void Clear() = 0;

  // Save/load the dictionary.
  virtual void Save(FILE *fs) = 0;
  virtual void Load(FILE *fs) = 0;

  // Unlock/lock the dictionary.
  virtual void AllowGrowth() = 0;
  virtual void StopGrowth() = 0;
};

#endif /* DICTIONARY_H_ */
