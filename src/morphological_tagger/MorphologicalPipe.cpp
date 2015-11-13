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

#include "MorphologicalPipe.h"
#include <iostream>
#include <sstream>
#include <vector>
#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

void MorphologicalPipe::PreprocessData() {
  delete token_dictionary_;
  CreateTokenDictionary();
  static_cast<SequenceDictionary*>(dictionary_)->SetTokenDictionary(token_dictionary_);
  // To get the right reader (instead of the default sequence reader).
  static_cast<MorphologicalTokenDictionary*>(token_dictionary_)->Initialize(GetMorphologicalReader());
  static_cast<MorphologicalDictionary*>(dictionary_)->CreateTagDictionary(GetMorphologicalReader());
}
