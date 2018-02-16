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

#include "EntityInstance.h"
#include <glog/logging.h>
#include "EntityOptions.h"

void EntityInstance::Initialize(const std::vector<std::string> &forms,
                                const std::vector<std::string> &pos,
                                const std::vector<std::string> &tags) {
  forms_ = forms;
  pos_ = pos;
  tags_ = tags;
}

void EntityInstance::ConvertToTaggingScheme(int tagging_scheme) {
  std::vector<std::unique_ptr<EntitySpan>> spans;
  CreateSpansFromTags(tags_, &spans);
  CreateTagsFromSpans(tags_.size(), spans, tagging_scheme, &tags_);
}

void EntityInstance::SplitEntityTag(const std::string &tag,
                                    std::string *prefix,
                                    std::string *entity) const {
  if (tag.length() == 1) {
    *prefix = tag;
    *entity = "";
  } else {
    CHECK_GE(tag.length(), 2);
    *prefix = tag.substr(0, 1);
    *entity = tag.substr(2);
  }
}

void EntityInstance::CreateSpansFromTags(
  const std::vector<std::string> &tags,
  std::vector<std::unique_ptr<EntitySpan>> *spans) const {
  spans->clear();
  std::unique_ptr<EntitySpan> span;
  for (int i = 0; i < tags.size(); ++i) {
    std::string prefix, entity;
    SplitEntityTag(tags[i], &prefix, &entity);
    if (prefix == "B" || prefix == "U") {
      if (span) spans->push_back(std::move(span));
      span = std::make_unique<EntitySpan>(i, i, entity);
    } else if (prefix == "I" || prefix == "L") {
      if (span && span->name() == entity) {
        span->set_end(i);
      } else {
        // This I is actually a B (maybe the file has IO encoding).
        if (span) spans->push_back(std::move(span));
        span = std::make_unique<EntitySpan>(i, i, entity);
      }
    } else if (prefix == "O") {
      if (span) spans->push_back(std::move(span));
      span.reset();
    }
  }
  if (span) spans->push_back(std::move(span));
}

void EntityInstance::CreateTagsFromSpans(
  int length,
  const std::vector<std::unique_ptr<EntitySpan>> &spans,
  int tagging_scheme,
  std::vector<std::string> *tags) const {
  tags->assign(length, "O");
  for (int k = 0; k < spans.size(); ++k) {
    EntitySpan *span = spans[k].get();
    if (tagging_scheme == EntityTaggingSchemes::BILOU) {
      if (span->start() == span->end()) {
        (*tags)[span->start()] = "U-" + span->name();
      } else {
        (*tags)[span->start()] = "B-" + span->name();
        (*tags)[span->end()] = "L-" + span->name();
      }
      for (int i = span->start() + 1; i < span->end(); ++i) {
        (*tags)[i] = "I-" + span->name();
      }
    } else if (tagging_scheme == EntityTaggingSchemes::BIO) {
      (*tags)[span->start()] = "B-" + span->name();
      for (int i = span->start() + 1; i <= span->end(); ++i) {
        (*tags)[i] = "I-" + span->name();
      }
    } else {
      CHECK_EQ(tagging_scheme, EntityTaggingSchemes::IO);
      for (int i = span->start(); i <= span->end(); ++i) {
        (*tags)[i] = "I-" + span->name();
      }
    }
  }
}
