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

#ifndef ENTITYINSTANCE_H_
#define ENTITYINSTANCE_H_

#include "SequenceInstance.h"
#include "EntitySpan.h"

class EntityInstance : public SequenceInstance {
 public:
  EntityInstance() {}
  virtual ~EntityInstance() {}

  Instance* Copy() {
    EntityInstance* instance = new EntityInstance();
    instance->Initialize(forms_, pos_, tags_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const std::vector<std::string> &forms,
                  const std::vector<std::string> &pos,
                  const std::vector<std::string> &tags);

  void ConvertToTaggingScheme(int tagging_scheme);

  const std::string &GetPosTag(int i) const { return pos_[i]; }

  // Split a tag like "B_PER" into a prefix ("B") and an entity ("PER").
  void SplitEntityTag(const std::string &tag,
                      std::string *prefix,
                      std::string *entity) const;

  // Convert a sequence of IO/BIO/BILOU tags into spans.
  void CreateSpansFromTags(const std::vector<std::string> &tags,
                           std::vector<EntitySpan*> *spans) const;

  // Convert spans into a sequence of IO/BIO/BILOU tags.
  void CreateTagsFromSpans(int length,
                           const std::vector<EntitySpan*> &spans,
                           int tagging_scheme,
                           std::vector<std::string> *tags) const;
  // Destroy the spans.
  void DeleteSpans(std::vector<EntitySpan*> *spans) const;

 protected:
  std::vector<std::string> pos_;
};

#endif /* ENTITYINSTANCE_H_*/
