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

#include "CoreferenceSentence.h"

void CoreferenceSentence::DeleteAllSpans() {
  for (int i = 0; i < entity_spans_.size(); ++i) {
    delete entity_spans_[i];
  }
  entity_spans_.clear();
  for (int i = 0; i < constituent_spans_.size(); ++i) {
    delete constituent_spans_[i];
  }
  constituent_spans_.clear();
  ClearCoreferenceSpans();
}

void CoreferenceSentence::Initialize(
    const std::string &name,
    const std::vector<std::string> &forms,
    const std::vector<std::string> &lemmas,
    const std::vector<std::string> &cpos,
    const std::vector<std::string> &pos,
    const std::vector<std::vector<std::string> > &feats,
    const std::vector<std::string> &deprels,
    const std::vector<int> &heads,
    const std::vector<std::string> &predicate_names,
    const std::vector<int> &predicate_indices,
    const std::vector<std::vector<std::string> > &argument_roles,
    const std::vector<std::vector<int> > &argument_indices,
    const std::vector<std::string> &speakers,
    const std::vector<EntitySpan*> &entity_spans,
    const std::vector<NamedSpan*> &constituent_spans,
    const std::vector<NamedSpan*> &coreference_spans) {
  SemanticInstance::Initialize(name, forms, lemmas, cpos, pos, feats, deprels,
                               heads, predicate_names, predicate_indices,
                               argument_roles, argument_indices);
  speakers_ = speakers;
  DeleteAllSpans();
  for (int i = 0; i < entity_spans.size(); ++i) {
    EntitySpan *span = new EntitySpan(entity_spans[i]->start(),
                                      entity_spans[i]->end(),
                                      entity_spans[i]->name());
    entity_spans_.push_back(span);
  }
  for (int i = 0; i < constituent_spans.size(); ++i) {
    NamedSpan *span = new NamedSpan(constituent_spans[i]->start(),
                                    constituent_spans[i]->end(),
                                    constituent_spans[i]->name());
    constituent_spans_.push_back(span);
  }
  for (int i = 0; i < coreference_spans.size(); ++i) {
    NamedSpan *span = new NamedSpan(coreference_spans[i]->start(),
                                    coreference_spans[i]->end(),
                                    coreference_spans[i]->name());
    coreference_spans_.push_back(span);
  }
}
