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

#ifndef SERIALIZATIONUTILS_H_
#define SERIALIZATIONUTILS_H_

#include <stdio.h>
#include <string>
#include <stdint.h>
#include <vector>

extern bool WriteString(FILE *fs, const std::string& data);
extern bool WriteBool(FILE *fs, bool value);
extern bool WriteInteger(FILE *fs, int value);
extern bool WriteUINT8(FILE *fs, uint8_t value);
extern bool WriteUINT64(FILE *fs, uint64_t value);
extern bool WriteDouble(FILE *fs, double value);
extern bool WriteIntegerVector(FILE *fs, const std::vector<int> &values);

extern bool ReadString(FILE *fs, std::string *data);
extern bool ReadBool(FILE *fs, bool *value);
extern bool ReadInteger(FILE *fs, int *value);
extern bool ReadUINT8(FILE *fs, uint8_t *value);
extern bool ReadUINT64(FILE *fs, uint64_t *value);
extern bool ReadDouble(FILE *fs, double *value);
extern bool ReadIntegerVector(FILE *fs, std::vector<int> *values);

#endif // SERIALIZATIONUTILS_H_
