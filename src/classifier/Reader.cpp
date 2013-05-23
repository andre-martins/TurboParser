// Copyright (c) 2012-2013 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.1.
//
// TurboParser 2.1 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.1.  If not, see <http://www.gnu.org/licenses/>.

#include "Reader.h"
#include <iostream>
#include <sstream>
#include "Utils.h"
#ifdef _WIN32
#include <glog\logging.h>
#endif
using namespace std;

void Reader::Open(const string &filepath) {
  is_.open(filepath.c_str(), ifstream::in);
  CHECK(is_.good()) << "Could not open " << filepath << ".";
}

void Reader::Close() {
  is_.clear();
  is_.close();
}

