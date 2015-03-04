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

#include "DependencyWriter.h"
#include "DependencyInstance.h"
#include <iostream>
#include <sstream>

void DependencyWriter::Write(Instance *instance) {
  DependencyInstance *dependency_instance = 
    static_cast<DependencyInstance*>(instance);

  for (int i = 1; i < dependency_instance->size(); ++i) {
    os_ << i << "\t";
    os_ << dependency_instance->GetForm(i) << "\t";
    os_ << dependency_instance->GetLemma(i) << "\t";
    os_ << dependency_instance->GetCoarsePosTag(i) << "\t";
    os_ << dependency_instance->GetPosTag(i) << "\t";
    os_ << "_" << "\t"; // Change this later
    os_ << dependency_instance->GetHead(i) << "\t";
    os_ << dependency_instance->GetDependencyRelation(i) << endl;
  }
  os_ << endl;
}
