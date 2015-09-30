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

#include "CoreferenceDocumentNumeric.h"
#include "CoreferencePipe.h"

void CoreferenceDocumentNumeric::Initialize(
    const CoreferenceDictionary &dictionary,
    CoreferenceDocument* instance,
    bool add_gold_mentions) {
  Clear();

  // True if document is a conversation.
  conversation_ = instance->is_conversation();

  // Temporary dictionary to hold coreference span names.
  std::map<std::string, int> span_names;
  sentences_.resize(instance->GetNumSentences());
  for (int i = 0; i < instance->GetNumSentences(); ++i) {
    CoreferenceSentence *sentence_instance = instance->GetSentence(i);
    CoreferenceSentenceNumeric *sentence = new CoreferenceSentenceNumeric;
    sentence->Initialize(dictionary, sentence_instance, add_gold_mentions,
                         &span_names);
    sentences_[i] = sentence;
  }

  ComputeGlobalWordPositions(instance);

  // Compute coreference information.
  Alphabet coreference_labels;
  for (int i = 0; i < instance->GetNumSentences(); ++i) {
    CoreferenceSentence *sentence_instance = instance->GetSentence(i);
    const std::vector<NamedSpan*> &coreference_spans =
      sentence_instance->GetCoreferenceSpans();
    for (int k = 0; k < coreference_spans.size(); ++k) {
      const std::string &label = coreference_spans[k]->name();
      int id = coreference_labels.Insert(label);
      int start = GetDocumentPosition(i, coreference_spans[k]->start());
      int end = GetDocumentPosition(i, coreference_spans[k]->end());
      NumericSpan *span = new NumericSpan(start, end, id);
      coreference_spans_.push_back(span);
    }
  }

  // Compute mention information.
  // Note: mentions are owned by CoreferenceSentenceNumeric.
  // TODO(atm): maybe copy the sentences' mentions?
  mentions_.clear();
  Alphabet mention_speakers;
  Alphabet mention_head_strings;
  Alphabet mention_phrase_strings;
  Alphabet mention_word_strings;
  for (int i = 0; i < sentences_.size(); ++i) {
    CoreferenceSentenceNumeric *sentence = sentences_[i];
    const vector<Mention*> &mentions = sentence->GetMentions();
    mentions_.insert(mentions_.end(), mentions.begin(),
                     mentions.end());
    for (int j = 0; j < mentions.size(); ++j) {
      int offset = GetDocumentPosition(i, 0);
      mentions[j]->set_offset(offset);
      //LOG(INFO) << "Sentence " << i << " -> Offset " << offset;

      int sentence_index = -1;
      int word_sentence_index = -1;
      // Pass offset+1 since the sentence starts with a special symbol which
      // is not accounted for in the document global positions.
      FindSentencePosition(offset+1, &sentence_index, &word_sentence_index);
      mentions[j]->set_sentence_index(sentence_index);
      //LOG(INFO) << "* Sentence " << sentence_index
      //          << " Word " << word_sentence_index;
      CHECK_EQ(word_sentence_index, 1); // First word in the sentence.
      //int start = GetDocumentPosition(i, mentions[j]->start());
      //int end = GetDocumentPosition(i, mentions[j]->end());
      //mentions[j]->set_head_index(mentions[j]->head_index()
      //                            - mentions[j]->start() + start);
      //mentions[j]->set_global_start(start);
      //mentions[j]->set_global_end(end);

      // Set head and phrase string IDs.
      CoreferenceSentence *sentence_instance =
        instance->GetSentence(sentence_index);

      std::string speaker;
      mentions[j]->GetSpeaker(sentence_instance, &speaker);
      int speaker_id = mention_speakers.Insert(speaker);
      mentions[j]->set_speaker_id(speaker_id);

      std::string phrase_string;
      mentions[j]->GetPhraseString(sentence_instance, &phrase_string);
      int phrase_string_id = mention_phrase_strings.Insert(phrase_string);
      mentions[j]->set_phrase_string_id(phrase_string_id);

      std::string head_string;
      mentions[j]->GetHeadString(sentence_instance, &head_string);
      int head_string_id = mention_head_strings.Insert(head_string);
      mentions[j]->set_head_string_id(head_string_id);

      std::vector<int> all_word_string_ids;
      for (int k = mentions[j]->start(); k <= mentions[j]->end(); ++k) {
        const std::string &word_string = sentence_instance->GetForm(k);
        int word_string_id = mention_word_strings.Insert(word_string);
        all_word_string_ids.push_back(word_string_id);
      }
      mentions[j]->set_all_word_string_ids(all_word_string_ids);
    }
  }

  ComputeEntityClusters();

#if 1
  LOG(INFO) << "Found " << coreference_spans_.size()
            << " gold mentions organized into "
            << coreference_labels.size() << " entities.";
  LOG(INFO) << "Total mentions: " << mentions_.size();
#endif

  // GenerateMentions()?
}

void CoreferenceDocumentNumeric::ComputeEntityClusters() {
  entity_clusters_.clear();
  for (int j = 0; j < mentions_.size(); ++j) {
    int entity_id = mentions_[j]->id();
    if (entity_id < 0) continue;
    if (entity_id >= entity_clusters_.size()) {
      entity_clusters_.resize(entity_id + 1);
    }
    entity_clusters_[entity_id].push_back(j);
  }
}

void CoreferenceDocumentNumeric::ComputeGlobalWordPositions(
    CoreferenceDocument* instance) {
  sentence_cumulative_lengths_.clear();
  sentence_cumulative_lengths_.resize(instance->GetNumSentences());
  int offset = 0;
  for (int i = 0; i < GetNumSentences(); ++i) {
    CoreferenceSentenceNumeric *sentence = GetSentence(i);
    // Subtract 1 since there is an extra start symbol.
    sentence_cumulative_lengths_[i] = offset + sentence->size() - 1;
    offset += sentence->size() - 1;
    //LOG(INFO) << "sentence_cumulative_lengths_[" << i << "] = "
    //          << sentence_cumulative_lengths_[i];
  }
}
