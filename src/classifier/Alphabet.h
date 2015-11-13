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

#ifndef ALPHABET_H_
#define ALPHABET_H_

#include "Utils.h"
#ifdef _WIN32
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include <stdio.h>
#ifdef _WIN32
#include <glog\logging.h>
#endif

using namespace std;

// This class implements a dictionary of strings, stored as an hash table
// for fast lookup. It allows looking up a string for its ID, and also obtain
// the string with a given ID.
class Alphabet : public std::tr1::unordered_map <std::string, int> {
public:
  Alphabet();
  virtual ~Alphabet();

  // Load/save the dictionary to/from a file.
  int Save(FILE* fs) const;
  int Load(FILE* fs);

  // Clear the dictionary.
  void clear() {
    num_entries_ = 0;
    names_.clear();
    std::tr1::unordered_map <std::string, int>::clear();
  }

  // Return the dictionary size.
  int size() const {
    return num_entries_;
  }

  // Lock/unlock the dictionary. When the dictionary is locked, no insertion
  // is possible.
  void StopGrowth() { growth_stopped_ = true; }
  void AllowGrowth() { growth_stopped_ = false; }

  // Check whether the dictionary is locked.
  bool growth_stopped() const { return growth_stopped_; }

  // Insert/lookup/check existence of an entry.
  int Insert(const std::string &entry);
  int Lookup(const std::string &entry) const;
  bool Contains(const std::string &entry) const;

  // Obtain the string with a given ID.
  const std::string &GetName(int id) const { return names_[id]; }

  // Build the vector of string names. This usually takes place after the
  // dictionary is built and locked.
  void BuildNames() {
    names_.resize(size());
    for (const_iterator iterator = begin(); iterator != end(); ++iterator) {
      CHECK_GE(iterator->second, 0);
      CHECK_LT(iterator->second, names_.size());
      names_[iterator->second] = iterator->first;
    }
  }

private:
  int num_entries_; // Number of entries in the dictionary.
  std::vector<std::string> names_; // Entry names.
  bool growth_stopped_; // True if dictionary is locked.
};

#endif /*ALPHABET_H_*/
