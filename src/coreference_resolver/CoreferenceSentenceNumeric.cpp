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
    CoreferenceSentence* instance,
    bool add_gold_mentions) {
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

  std::map<std::string, int> span_names;
  const std::vector<NamedSpan*> &coreference_spans =
    instance->GetCoreferenceSpans();
  coreference_spans_.resize(coreference_spans.size());
  for (int k = 0; k < coreference_spans.size(); ++k) {
    int start = coreference_spans[k]->start();
    int end = coreference_spans[k]->end();
    const std::string &name = coreference_spans[k]->name();
    std::map<std::string, int>::const_iterator it = span_names.find(name);
    int id = -1;
    if (it == span_names.end()) {
      id = span_names.size();
      span_names[name] = id;
    } else {
      id = it->second;
    }
    coreference_spans_[k] = new NumericSpan(start, end, id);
  }

  int length = instance->size();
  first_upper_.resize(length);
  for (int i = 0; i < length; ++i) {
    const char* word = instance->GetForm(i).c_str();
    int word_length = instance->GetForm(i).length();
    first_upper_[i] = IsCapitalized(word, word_length);
  }

  // Generate candidate mentions.
  GenerateMentions(dictionary, instance);

  //LOG(INFO) << mentions_.size() << " found in sentence.";

  // Add gold mentions as candidates (only for training).
  if (add_gold_mentions) AddGoldMentions(dictionary, instance);


#if 0
  LOG(INFO) << mentions_.size() << " found in sentence.";
  for (int i = 0; i < mentions_.size(); ++i) {
    LOG(INFO) << "Mention "
              << mentions_[i]->start() << " "
              << mentions_[i]->end();

  }
#endif

#if 0
  // Print mention information (for debugging purposes).
  LOG(INFO) << mentions_.size() << " found in sentence.";
  for (int i = 0; i < mentions_.size(); ++i) {
    mentions_[i]->Print(dictionary, instance);
  }
#endif
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
    AddMention(dictionary, instance, mention_start, mention_end, -1);
  }
  std::vector<Span*> named_entity_mentions(mentions_.begin(),
                                           mentions_.end());

  // Generate mentions for noun phrases and pronouns *except* those contained in
  // the named entity chunks (the named entity tagger seems more reliable than
  // the parser).
  for (int k = 0; k < constituent_spans_.size(); ++k) {
    if (!dictionary.IsNounPhrase(constituent_spans_[k]->id())) continue;
    if (constituent_spans_[k]->FindCoveringSpan(named_entity_mentions)) {
      continue;
    }
    AddMention(dictionary,
               instance,
               constituent_spans_[k]->start(),
               constituent_spans_[k]->end(),
               -1);
  }

  bool generate_noun_phrase_mentions_by_dependencies = false;
  if (generate_noun_phrase_mentions_by_dependencies) {
    // TODO(atm).
    CHECK(false);
  }

  for (int i = 0; i < size(); ++i) {
    if (!dictionary.IsPronounTag(pos_ids_[i])) continue;
    // TODO(atm): for Portuguese need to ignore "se" and "-se".
    Span span(i, i);
    if (span.FindCoveringSpan(named_entity_mentions)) continue;
    AddMention(dictionary, instance, i, i, -1);
  }

  // Filter and sort mentions.
  FilterAndSortMentions(dictionary, instance);
}

void CoreferenceSentenceNumeric::AddGoldMentions(
    const CoreferenceDictionary &dictionary,
    CoreferenceSentence* instance) {
  int num_mentions = mentions_.size();
  for (int i = 0; i < coreference_spans_.size(); ++i) {
    NumericSpan *span = coreference_spans_[i];
    // TODO(atm): add some specific stuff for PT and ES here.
    // See if this mention exists already; if so, just update its label.
    bool exists = false;
    for (int j = 0; j < mentions_.size(); ++j) {
      if (mentions_[j]->start() == span->start() &&
          mentions_[j]->end() == span->end()) {
        if (mentions_[j]->id() >= 0) {
          std::string mention_string;
          mentions_[j]->GetPhraseString(instance, &mention_string);
          LOG(INFO) << "Repeated gold mention: "
                    << mention_string
                    << " [" << span->start() << ", "
                    << span->end() << "]";
        }
        mentions_[j]->set_id(span->id());
        exists = true;
      }
    }
    // If this mention does not exist, add it.
    if (!exists) {
      AddMention(dictionary, instance, span->start(), span->end(), span->id());
    }
  }
#if 0
  LOG(INFO) << "Added " << mentions_.size() - num_mentions << " mentions.";
#endif
}

void CoreferenceSentenceNumeric::FilterAndSortMentions(
    const CoreferenceDictionary &dictionary,
    CoreferenceSentence* instance) {
  std::vector<Mention*> sorted_mentions;
  std::vector<std::vector<Mention*> > mentions_by_head(instance->size());

  for (int i = 0; i < mentions_.size(); ++i) {
    CHECK_GE(mentions_[i]->head_index(), 0);
    CHECK_LT(mentions_[i]->head_index(), instance->size());
    mentions_by_head[mentions_[i]->head_index()].push_back(mentions_[i]);
  }

  for (int h = 0; h < instance->size(); ++h) {
    Mention *current_biggest = NULL;
    for (int i = 0; i < mentions_by_head[h].size(); ++i) {
      Mention *mention = mentions_by_head[h][i];
      if (current_biggest &&
          mention->OverlapNotNested(*static_cast<Span*>(current_biggest))) {
        std::string mention_string, current_string;
        mention->GetPhraseString(instance, &mention_string);
        current_biggest->GetPhraseString(instance, &current_string);
        LOG(INFO) <<
          "Warning: Mentions with the same head but neither contains the other: "
                  << mention_string
                  << " vs. "
                  << current_string;
      }
      if (!current_biggest ||
          current_biggest->LiesInsideSpan(*static_cast<Span*>(mention))) {
        current_biggest = mention;
      }
    }

    if (current_biggest) sorted_mentions.push_back(current_biggest);

    for (int i = 0; i < mentions_by_head[h].size(); ++i) {
      Mention *mention = mentions_by_head[h][i];
      if (mention == current_biggest) continue; // Inserted already.

      // Don't remove appositives.
      // TODO(atm): Make this English only.
      bool is_appositive_like = ((mention->end() + 1 < instance->size()) &&
        (instance->GetPosTag(mention->end() + 1) == "," ||
         instance->GetPosTag(mention->end() + 1) == "CC"));
      if (is_appositive_like) {
        sorted_mentions.push_back(mention);
        continue;
      }
      // TODO(atm): there are some stuff here to deal with gold mentions and
      // gold speakers. Should we port it?
      delete mention;
    }
  }
  mentions_ = sorted_mentions;
}

void CoreferenceSentenceNumeric::AddMention(
    const CoreferenceDictionary &dictionary,
    CoreferenceSentence* instance,
    int start, int end, int id) {
  // If this mention already exists, do nothing.
  for (int i = 0; i < mentions_.size(); ++i) {
    if (mentions_[i]->start() == start && mentions_[i]->end() == end) return;
  }
  Mention *mention = new Mention(start, end, id);
  mention->ComputeProperties(dictionary, instance, this);
  mentions_.push_back(mention);
}
