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

#include "Alphabet.h"
#include "SerializationUtils.h"

Alphabet::Alphabet() {
  num_entries_ = 0;
  growth_stopped_ = false;
}

Alphabet::~Alphabet() {
  clear();
}

// Insert a new entry to the dictionary (if it does not exist already)
// and retrieves its ID.
// If the dictionary is locked and the entry does not exist, no insertion
// takes place and -1 will be returned.
int Alphabet::Insert(const string &entry) {
  int ret = -1;
  iterator iter = find(entry);
  if (iter == end()) {
    if (!growth_stopped_) {
      ret = num_entries_;
      operator[](entry) = ret;
      ++num_entries_;
    }
  } else {
    ret = iter->second;
  }
  return ret;
}

// Lookup an entry in the dictionary, retrieving its ID.
// If the entry does not exist, -1 will be returned.
int Alphabet::Lookup(const string &entry) const {
  const_iterator iter = find(entry);
  if (iter == end()) return -1;
  return iter->second;
}

// Checks whether an entry is already in the dictionary.
bool Alphabet::Contains(const string &entry) const {
  return (find(entry) != end());
}

// Saves the alphabet to a file.
int Alphabet::Save(FILE* fs) const {
  bool success;
  success = WriteInteger(fs, num_entries_);
  CHECK(success);
  for (const_iterator iter = begin(); iter != end(); ++iter) {
    success = WriteString(fs, iter->first);
    CHECK(success);
    success = WriteInteger(fs, iter->second);
    CHECK(success);
  }
  return 0;
}

// Loads the alphabet from a file.
int Alphabet::Load(FILE* fs) {
  bool success;
  success = ReadInteger(fs, &num_entries_);
  CHECK(success);
  for (int i = 0; i < num_entries_; ++i) {
    string key;
    int value;
    success = ReadString(fs, &key);
    CHECK(success);
    success = ReadInteger(fs, &value);
    CHECK(success);
    operator[](key) = value;
  }
  return 0;
}
