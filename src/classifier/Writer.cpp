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

#include "Writer.h"
#include <iostream>
#include <sstream>
#include "Utils.h"

using namespace std;

void Writer::Open(const string &filepath) {
  os_.open(filepath.c_str(), ifstream::out);
  CHECK(os_.good()) << "Could not open " << filepath << ".";
}

void Writer::Close() {
  os_.flush();
  os_.clear();
  os_.close();
}

