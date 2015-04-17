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

#ifndef COREFERENCESENTENCE_H_
#define COREFERENCESENTENCE_H_

#include <string>
#include <vector>
#include "SemanticInstance.h"
#include "EntitySpan.h"

class CoreferenceSentence : public SemanticInstance {
 public:
  CoreferenceSentence() {}
  virtual ~CoreferenceSentence() { DeleteAllSpans(); }

  Instance* Copy() {
    CoreferenceSentence* instance = new CoreferenceSentence();
    instance->Initialize(name_, forms_, lemmas_, cpostags_, postags_,
                         feats_, deprels_, heads_,
                         predicate_names_, predicate_indices_,
                         argument_roles_, argument_indices_,
                         entity_spans_, constituent_spans_, coreference_spans_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const std::string &name,
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
                  const std::vector<EntitySpan*> &entity_spans,
                  const std::vector<NamedSpan*> &constituent_spans,
                  const std::vector<NamedSpan*> &coreference_spans);

  const std::vector<EntitySpan*>& GetEntitySpans() { return entity_spans_; }
  const std::vector<NamedSpan*>& GetConstituentSpans() {
    return constituent_spans_;
  }
  const std::vector<NamedSpan*>& GetCoreferenceSpans() {
    return coreference_spans_;
  }

 protected:
  void DeleteAllSpans();

 protected:
  // Entity spans: e.g. A "ORG" entity spanning word 6 to 8.
  std::vector<EntitySpan*> entity_spans_;
  // Constituent spans: e.g. Noun phrase "NP" spanning word 6 to 8.
  std::vector<NamedSpan*> constituent_spans_;
  // Coreference spans: e.g. Mention named "(12)" spanning word 6 to 8.
  std::vector<NamedSpan*> coreference_spans_;
};

#endif /* COREFERENCESENTENCE_H_*/
