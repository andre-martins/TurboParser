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

#include "CoreferenceSentenceNumeric.h"
#include "CoreferencePipe.h"
#include <iostream>
#include <algorithm>

const int kUnknownEntity = 0xff;
const int kUnknownConstituent = 0xffff;

void CoreferenceSentenceNumeric::Initialize(
    const CoreferenceDictionary &dictionary,
    CoreferenceSentence* instance) {
  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  DependencyDictionary *dependency_dictionary =
    dictionary.GetDependencyDictionary();
  SemanticDictionary *semantic_dictionary =
    dictionary.GetSemanticDictionary();
  SemanticInstance *semantic_instance =
    static_cast<SemanticInstance*>(instance);
  //CoreferenceOptions *options =
  //  static_cast<CoreferencePipe*>(dictionary.GetPipe())->GetCoreferenceOptions();

  Clear();

  SemanticInstanceNumeric::Initialize(*semantic_dictionary,
                                      semantic_instance);

  const std::vector<EntitySpan*> &entity_spans = instance->GetEntitySpans();
  entity_spans_.resize(entity_spans.size());
  for (int k = 0; k < entity_spans.size(); ++k) {
    int start = entity_spans[k]->start();
    int end = entity_spans[k]->end();
    const std::string &name = entity_spans[k]->name();
    int id = dictionary.GetEntityAlphabet().Lookup(name);
    CHECK_LT(id, 0xff);
    if (id < 0) id = kUnknownEntity;
    entity_spans_[k] = new NumericSpan(start, end, id);
  }

  const std::vector<NamedSpan*> &constituent_spans =
    instance->GetConstituentSpans();
  constituent_spans_.resize(constituent_spans.size());
  for (int k = 0; k < constituent_spans.size(); ++k) {
    int start = constituent_spans[k]->start();
    int end = constituent_spans[k]->end();
    const std::string &name = constituent_spans[k]->name();
    int id = dictionary.GetConstituentAlphabet().Lookup(name);
    CHECK_LT(id, 0xffff);
    if (id < 0) id = kUnknownConstituent;
    constituent_spans_[k] = new NumericSpan(start, end, id);
  }

  // Generate candidate mentions.
  GenerateMentions(dictionary, instance);
}

void CoreferenceSentenceNumeric::GenerateMentions(
    const CoreferenceDictionary &dictionary,
    CoreferenceSentence* instance) {
  DeleteMentions();

  // Generate mentions for named entities.
  for (int k = 0; k < entity_spans_.size(); ++k) {
    if (!dictionary.IsNamedEntity(entity_spans_[k]->id())) continue;
    int mention_start = entity_spans_[k]->start();
    int mention_end = entity_spans_[k]->end();
    // Expand named entities to include possessives (this only makes sense for
    // English).
    if (mention_end < size() - 1 && instance->GetForm(mention_end) == "'s") {
      ++mention_end;
    }
    AddMention(mention_start, mention_end, -1);
  }
  std::vector<Mention*> named_entity_mentions(mentions_.begin(),
                                              mentions_.end());

  // Generate mentions for noun phrases and pronouns *except* those contained in
  // the named entity chunks (the named entity tagger seems more reliable than
  // the parser).
  // Generate mentions for named entities.
  for (int k = 0; k < constituent_spans_.size(); ++k) {
    if (!dictionary.IsNounPhrase(constituent_spans_[k]->id())) continue;
    if (constituent_spans_[k]->LiesInsideAnyOfSpans(named_entity_mentions)) {
      continue;
    }
    AddMention(constituent_spans_[k]->start(), constituent_spans_[k]->end(),
               -1);
  }

  bool generate_noun_phrase_mentions_by_dependencies = false;
  if (generate_noun_phrase_mentions_by_dependencies) {
    // TODO(atm).
    CHECK(false);
  }

  for (int i = 0; i < size(); ++i) {
    if (!dictionary.IsPronominalTag(tags_[i])) continue;
    // TODO(atm): for Portuguese need to ignore "se" and "-se".
    Span span(i, i);
    if (span.LiesInsideAnyOfSpans(named_entity_mentions)) continue;
    AddMention(i, i, -1);
  }
}

void CoreferenceSentenceNumeric::AddMention(int start, int end, int id) {
  Mention *mention = new Mention(start, end, id);
  mention->ComputeProperties();
  mentions_.push_back(mention);
}
