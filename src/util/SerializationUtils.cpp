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

#include "SerializationUtils.h"
#include <cstring>

bool WriteString(FILE *fs, const std::string& data) {
  const char *buffer = data.c_str();
  int length = strlen(buffer);
  if (1 != fwrite(&length, sizeof(int), 1, fs)) return false;
  if (length != fwrite(buffer, sizeof(char), length, fs)) return false;
  return true;
}

bool WriteBool(FILE *fs, bool value) {
  if (1 != fwrite(&value, sizeof(bool), 1, fs)) return false;
  return true;
}

bool WriteInteger(FILE *fs, int value) {
  if (1 != fwrite(&value, sizeof(int), 1, fs)) return false;
  return true;
}

bool WriteUINT8(FILE *fs, uint8_t value) {
  if (1 != fwrite(&value, sizeof(uint8_t), 1, fs)) return false;
  return true;
}

bool WriteUINT64(FILE *fs, uint64_t value) {
  if (1 != fwrite(&value, sizeof(uint64_t), 1, fs)) return false;
  return true;
}

bool WriteDouble(FILE *fs, double value) {
  if (1 != fwrite(&value, sizeof(double), 1, fs)) return false;
  return true;
}

bool WriteIntegerVector(FILE *fs, const std::vector<int> &values) {
  int length = values.size();
  if (!WriteInteger(fs, length)) return false;
  for (int i = 0; i < length; ++i) {
    int value = values[i];
    if (!WriteInteger(fs, value)) return false;
  }
  return true;
}

bool ReadString(FILE *fs, std::string *data) {
  int length;
  if (1 != fread(&length, sizeof(int), 1, fs)) return false;
  char *buffer = new char[length + 1];
  if (length != fread(buffer, sizeof(char), length, fs)) return false;
  buffer[length] = '\0';
  *data = buffer;
  delete[] buffer;
  return true;
}

bool ReadBool(FILE *fs, bool *value) {
  if (1 != fread(value, sizeof(bool), 1, fs)) return false;
  return true;
}

bool ReadInteger(FILE *fs, int *value) {
  if (1 != fread(value, sizeof(int), 1, fs)) return false;
  return true;
}

bool ReadUINT8(FILE *fs, uint8_t *value) {
  if (1 != fread(value, sizeof(uint8_t), 1, fs)) return false;
  return true;
}

bool ReadUINT64(FILE *fs, uint64_t *value) {
  if (1 != fread(value, sizeof(uint64_t), 1, fs)) return false;
  return true;
}

bool ReadDouble(FILE *fs, double *value) {
  if (1 != fread(value, sizeof(double), 1, fs)) return false;
  return true;
}

bool ReadIntegerVector(FILE *fs, std::vector<int> *values) {
  int length;
  if (!ReadInteger(fs, &length)) return false;
  values->resize(length);
  for (int i = 0; i < length; ++i) {
    int value;
    if (!ReadInteger(fs, &value)) return false;
    (*values)[i] = value;
  }
  return true;
}
