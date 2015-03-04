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

#include "EntityWriter.h"
#include "EntityInstance.h"
#include "EntityOptions.h"
#include <iostream>
#include <sstream>

void EntityWriter::Write(Instance *instance) {
  EntityInstance *entity_instance =
    static_cast<EntityInstance*>(instance);

  // Always write in BIO format.
  std::vector<EntitySpan*> spans;
  std::vector<std::string> tags;
  entity_instance->CreateSpansFromTags(entity_instance->tags(), &spans);
  entity_instance->CreateTagsFromSpans(entity_instance->tags().size(),
                                       spans,
                                       EntityTaggingSchemes::BIO,
                                       &tags);
  entity_instance->DeleteSpans(&spans);

  for (int i = 0; i < entity_instance->size(); ++i) {
    os_ << entity_instance->GetForm(i) << "\t";
    os_ << entity_instance->GetPosTag(i) << "\t";
    os_ << tags[i] << endl;
  }
  os_ << endl;
}
