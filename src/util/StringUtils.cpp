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

#include "StringUtils.h"

// Split string str on any delimiting character in delim, and write the result
// as a vector of strings.
void StringSplit(const string &str,
                 const string &delim,
                 vector<string> *results) {
  size_t cutAt;
  string tmp = str;
  while ((cutAt = tmp.find_first_of(delim)) != tmp.npos) {
    if(cutAt > 0) {
      results->push_back(tmp.substr(0,cutAt));
    }
    tmp = tmp.substr(cutAt+1);
  }
  if(tmp.length() > 0) results->push_back(tmp);
}

// Join fields into a single string using a delimiting character.
void StringJoin(const vector<string> &fields,
                const char delim,
                string *result) {
  *result = "";
  for (int i = 0; i < fields.size()-1; ++i) {
    *result += fields[i] + delim;
  }
  if (fields.size() > 0) *result += fields.back();
}

// Extract the file name given the file path and a delimiting character "delim"
// (typically "/"). The path is provided as input in the string "file_name",
// which is overwritten with the file name.
void GetFileNameFromPath(const string &delim, string *file_name) {
  size_t cutAt = file_name->find_last_of(delim);
  if (cutAt != file_name->npos) {
    *file_name = file_name->substr(cutAt + 1);
  }
}

// Deletes any tail in the string "line" after the first occurrence of any
// delimiting character in "delim".
void TrimComments(const string &delim, string *line) {
  size_t cutAt = line->find_first_of(delim);
  if (cutAt != line->npos) {
    *line = line->substr(0, cutAt);
  }
}

// Deletes any head in the string "line" after the first occurrence of any
// non-delimiting character (e.g. whitespaces).
void TrimLeft(const string &delim, string *line) {
  size_t cutAt = line->find_first_not_of(delim);
  if (cutAt != line->npos) {
    *line = line->substr(cutAt);
  }
}

// Deletes any tail in the string "line" after the last occurrence of any
// non-delimiting character (e.g. whitespaces).
void TrimRight(const string &delim, string *line) {
  size_t cutAt = line->find_last_not_of(delim);
  if (cutAt != line->npos) {
    *line = line->substr(0, cutAt+1);
  }
}

// Trims left and right (see above).
void Trim(const string &delim, string *line) {
  TrimLeft(delim, line);
  TrimRight(delim, line);
}
