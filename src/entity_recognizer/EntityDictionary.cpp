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

#include "EntityDictionary.h"
#include "EntityOptions.h"
#include "EntityPipe.h"
#include <algorithm>

void EntityDictionary::CreateTagDictionary(SequenceReader *reader) {
  SequenceDictionary::CreateTagDictionary(reader);

  // TODO: the SplitEntityTag function should probably be elsewhere and not on
  // EntityInstance.
  EntityInstance instance;
  Alphabet entities;

  // Display information about the entity tags.
  LOG(INFO) << "Found " << tag_alphabet_.size() << " entity tags:";
  for (Alphabet::iterator it = tag_alphabet_.begin();
       it != tag_alphabet_.end(); ++it) {
    std::string entity_tag = it->first;
    LOG(INFO) << entity_tag;

    std::string prefix, entity;
    instance.SplitEntityTag(it->first, &prefix, &entity);
    if (entity != "") entities.Insert(entity);
  }

  LOG(INFO) << "Entities:";
  for (Alphabet::iterator it = entities.begin(); it != entities.end(); ++it) {
    LOG(INFO) << it->first;
  }

  LOG(INFO) << "Computing allowed bigrams...";
  // Every bigram is allowed by default.
  allowed_bigrams_.assign(1 + tag_alphabet_.size(),
                          std::vector<bool>(1 + tag_alphabet_.size(), true));
  // Now add the BIO-like constraints.
  for (Alphabet::iterator it = entities.begin(); it != entities.end(); ++it) {
    std::string entity = it->first;
    LOG(INFO) << "Processing entity " << entity << "...";
    if (static_cast<EntityPipe*>(pipe_)->GetEntityOptions()->tagging_scheme() ==
        EntityTaggingSchemes::BIO) {
      int tag_begin = tag_alphabet_.Lookup("B-" + entity);
      int tag_inside = tag_alphabet_.Lookup("I-" + entity);
      if (tag_inside < 0) continue;
      // An I-tag can only occur after a B-tag or another I-tag of the same
      // entity.
      for (int left_tag = -1; left_tag < tag_alphabet_.size(); ++left_tag) {
        if (left_tag != tag_begin && left_tag != tag_inside) {
          allowed_bigrams_[1+tag_inside][1+left_tag] = false;
        }
      }
    } else if (static_cast<EntityPipe*>(pipe_)->GetEntityOptions()->
               tagging_scheme() == EntityTaggingSchemes::BILOU) {
      int tag_begin = tag_alphabet_.Lookup("B-" + entity);
      int tag_inside = tag_alphabet_.Lookup("I-" + entity);
      int tag_last = tag_alphabet_.Lookup("L-" + entity);
      int tag_unique = tag_alphabet_.Lookup("U-" + entity);
      // I-tags and L-tags can only occur after a B-tag or an I-tag of the same
      // entity.
      for (int left_tag = -1; left_tag < tag_alphabet_.size(); ++left_tag) {
        if (left_tag != tag_begin && left_tag != tag_inside) {
          if (tag_inside >= 0) {
            allowed_bigrams_[1+tag_inside][1+left_tag] = false;
          }
          if (tag_last >= 0) {
            allowed_bigrams_[1+tag_last][1+left_tag] = false;
          }
        }
      }
      // I-tags and B-tags can only occur before an I-tag or an L-tag of the
      // same entity.
      for (int right_tag = -1; right_tag < tag_alphabet_.size(); ++right_tag) {
        if (right_tag != tag_last && right_tag != tag_inside) {
          if (tag_inside >= 0) {
            allowed_bigrams_[1+right_tag][1+tag_inside] = false;
          }
          if (tag_begin >= 0) {
            allowed_bigrams_[1+right_tag][1+tag_begin] = false;
          }
        }
      }
    }
  }

  tag_alphabet_.BuildNames(); // Just to be able to plot readable information...
  int num_allowed_bigrams = 0;
  for (int tag = -1; tag < tag_alphabet_.size(); ++tag) {
    for (int left_tag = -1; left_tag < tag_alphabet_.size(); ++left_tag) {
      if (IsAllowedBigram(left_tag, tag)) {
        std::string left_tag_name = (left_tag >= 0)?
          tag_alphabet_.GetName(left_tag) : "START";
        std::string tag_name = (tag >= 0)?
          tag_alphabet_.GetName(tag) : "STOP";

        LOG(INFO) << "Allowed bigram: "
                  << left_tag_name
                  << " -> "
                  << tag_name;

        ++num_allowed_bigrams;
      }
    }
  }

  LOG(INFO) << "Total allowed bigrams: " << num_allowed_bigrams;

#if 0
  LOG(INFO) << "Creating word-tag dictionary...";
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  // Go through the corpus and build the existing tags for each word.
  word_tags_.clear();
  word_tags_.resize(token_dictionary_->GetNumForms());

  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  SequenceInstance *instance =
    static_cast<SequenceInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;
      string form = instance->GetForm(i);
      if (!form_case_sensitive) {
        transform(form.begin(), form.end(), form.begin(), ::tolower);
      }
      int word_id = token_dictionary_->GetFormId(form);
      //CHECK_GE(word_id, 0);

      id = tag_alphabet_.Lookup(instance->GetTag(i));
      CHECK_GE(id, 0);

      // Insert new tag in the set of word tags, if it is not there
      // already. NOTE: this is inefficient, maybe we should be using a
      // different data structure.
      if (word_id >= 0) {
        vector<int> &tags = word_tags_[word_id];
        int j;
        for (j = 0; j < tags.size(); ++j) {
          if (tags[j] == id) break;
        }
        if (j == tags.size()) tags.push_back(id);
      }
    }
    delete instance;
    instance = static_cast<SequenceInstance*>(reader->GetNext());
  }
  reader->Close();

  // If there is a list of possible tags for the unknown words, load it.
  TaggerOptions *options =
    static_cast<TaggerOptions*>(pipe_->GetOptions());
  if (options->GetUnknownWordTagsFilePath().size() == 0) {
    for (int i = 0; i < tag_alphabet_.size(); ++i) {
      unknown_word_tags_.push_back(i);
    }
  } else {
    LOG(INFO) << "Loading file with unknown word tags...";
    std::ifstream is;
    is.open(options->GetUnknownWordTagsFilePath().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->GetUnknownWordTagsFilePath() << ".";
    vector<vector<string> > sentence_fields;
    string line;
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line.size() == 0) break;
        int tagid = tag_alphabet_.Lookup(line);
        CHECK(tagid >= 0) << "Tag " << line << " does not exist.";
        unknown_word_tags_.push_back(tagid);
        LOG(INFO) << "Unknown word tag: " << line;
      }
    }
  }
  LOG(INFO) << "Number of unknown word tags: " << unknown_word_tags_.size();
#endif
}
