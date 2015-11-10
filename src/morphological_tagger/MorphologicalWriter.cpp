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

#include "MorphologicalWriter.h"
#include "MorphologicalInstance.h"
#include "MorphologicalOptions.h"
#include <iostream>
#include <sstream>

void MorphWriter::Write(Instance *instance) {
  MorphInstance *morph_instance = static_cast<MorphInstance*>(instance);

  for (int i = 0; i < morph_instance->size(); ++i) {
    os_ << (i + 1) << "\t";
    os_ << morph_instance->GetForm(i) << "\t";
    os_ << morph_instance->GetLemma(i) << "\t";
    os_ << morph_instance->GetCoarsePosTag(i) << "\t";
    os_ << "_" << "\t";
    os_ << morph_instance->GetTag(i) << "\t";
    os_ << "_" << "\t";
    os_ << "_" << "\t";
    os_ << "_" << "\t";
    os_ << "_" << endl;
  }
  os_ << endl;
}
