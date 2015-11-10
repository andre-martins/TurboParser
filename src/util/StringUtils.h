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

#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>

using namespace std;

extern void StringSplit(const std::string &str,
                        const std::string &delim,
                        std::vector<std::string> *results,
                        bool ignore_multiple_separators);

extern void StringJoin(const std::vector<std::string> &fields,
                       const char delim,
                       std::string *result);

extern void GetFileNameFromPath(const std::string &delim, std::string *file_name);

extern void TrimComments(const std::string &delim, std::string *line);

extern void TrimLeft(const std::string &delim, std::string *line);

extern void TrimRight(const std::string &delim, std::string *line);

extern void Trim(const std::string &delim, std::string *line);

#endif // STRINGUTILS_H
