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

  ReadGazetteerFiles();
}

void EntityDictionary::ReadGazetteerFiles() {
  EntityOptions *options =
    static_cast<EntityOptions*>(pipe_->GetOptions());

  gazetteer_word_alphabet_.AllowGrowth();
  gazetteer_entity_tag_alphabet_.AllowGrowth();

  if (options->file_gazetteer() != "") {
    LOG(INFO) << "Loading gazetteer file "
              << options->file_gazetteer() << "...";
    std::ifstream is;
    std::string line;

    // Do a first pass just to count the words and create the
    // dictionaries.
    is.open(options->file_gazetteer().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->file_gazetteer() << ".";
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line == "") continue; // Ignore blank lines.
        std::vector<std::string> fields;
        StringSplit(line, " \t", &fields); // Break on tabs or spaces.
        if (fields.size() < 2) continue;
        const std::string &entity_type = fields[0];
        gazetteer_entity_tag_alphabet_.Insert("B-" + entity_type);
        gazetteer_entity_tag_alphabet_.Insert("I-" + entity_type);
        gazetteer_entity_tag_alphabet_.Insert("L-" + entity_type);
        gazetteer_entity_tag_alphabet_.Insert("U-" + entity_type);
        for (int k = 1; k < fields.size(); ++k) {
          const std::string &word = fields[k];
          gazetteer_word_alphabet_.Insert(word);
        }
      }
    }
    is.close();

    // Now do the second pass to actually fill in the data.
    gazetteer_word_entity_tags_.clear();
    gazetteer_word_entity_tags_.resize(gazetteer_word_alphabet_.size());
    is.open(options->file_gazetteer().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
                     << options->file_gazetteer() << ".";
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line == "") continue; // Ignore blank lines.
        std::vector<std::string> fields;
        StringSplit(line, " \t", &fields); // Break on tabs or spaces.
        if (fields.size() < 2) continue;
        const std::string &entity_type = fields[0];
        int entity_type_begin_id =
          gazetteer_entity_tag_alphabet_.Lookup("B-" + entity_type);
        int entity_type_inside_id =
          gazetteer_entity_tag_alphabet_.Lookup("I-" + entity_type);
        int entity_type_last_id =
          gazetteer_entity_tag_alphabet_.Lookup("L-" + entity_type);
        int entity_type_unique_id =
          gazetteer_entity_tag_alphabet_.Lookup("U-" + entity_type);
        for (int k = 1; k < fields.size(); ++k) {
          const std::string &word = fields[k];
          int word_id = gazetteer_word_alphabet_.Lookup(word);
          CHECK_GE(word_id, 0);
          CHECK_LT(word_id, gazetteer_word_entity_tags_.size());
          int entity_type_id = -1;
          if (fields.size() == 2) {
            entity_type_id = entity_type_unique_id;
          } else if (k == 1) {
            entity_type_id = entity_type_begin_id;
          } else if (k == fields.size() - 1) {
            entity_type_id = entity_type_last_id;
          } else {
            entity_type_id = entity_type_inside_id;
          }
          int l = -1;
          for (l = 0; l < gazetteer_word_entity_tags_[word_id].size();
               ++l) {
            if (gazetteer_word_entity_tags_[word_id][l] == entity_type_id) {
              break;
            }
          }
          if (l == gazetteer_word_entity_tags_[word_id].size()) {
            gazetteer_word_entity_tags_[word_id].
              push_back(entity_type_id);
          }
        }
      }
    }
    is.close();
  }

  gazetteer_word_alphabet_.StopGrowth();
  gazetteer_entity_tag_alphabet_.StopGrowth();
  LOG(INFO) << "Number of gazetteer words: "
            << gazetteer_word_alphabet_.size();
  LOG(INFO) << "Number of gazetteer entity tags: "
            << gazetteer_entity_tag_alphabet_.size();

}



void EntityTokenDictionary::InitializeFromEntityReader(EntityReader *reader) {
  TokenDictionary::InitializeStarter();
  TokenDictionary::InitializeFromSequenceReader(reader);

  std::vector<int> pos_freqs;
  Alphabet pos_alphabet;

  std::string special_symbols[NUM_SPECIAL_TOKENS];
  special_symbols[TOKEN_UNKNOWN] = kTokenUnknown;
  special_symbols[TOKEN_START] = kTokenStart;
  special_symbols[TOKEN_STOP] = kTokenStop;

  for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
    pos_alphabet.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    pos_freqs.push_back(-1);
  }

  // Go through the corpus and build the dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  EntityInstance *instance =
    static_cast<EntityInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;
      // Add POS to alphabet.
      id = pos_alphabet.Insert(instance->GetPosTag(i));
      if (id >= pos_freqs.size()) {
        CHECK_EQ(id, pos_freqs.size());
        pos_freqs.push_back(0);
      }
      ++pos_freqs[id];
    }
    delete instance;
    instance = static_cast<EntityInstance*>(reader->GetNext());
  }
  reader->Close();

  // Now adjust the cutoffs if necessary.
  while (true) {
    pos_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_TOKENS; ++i) {
      pos_alphabet_.Insert(special_symbols[i]);
    }
    for (Alphabet::iterator iter = pos_alphabet.begin();
    iter != pos_alphabet.end();
      ++iter) {
      if (pos_freqs[iter->second] > pos_cutoff) {
        pos_alphabet_.Insert(iter->first);
      }
    }
    if (pos_alphabet_.size() < kMaxPosAlphabetSize) break;
    ++pos_cutoff;
    LOG(INFO) << "Incrementing POS cutoff to " << pos_cutoff << "...";
  }

  form_alphabet_.StopGrowth();
  form_lower_alphabet_.StopGrowth();
  lemma_alphabet_.StopGrowth();
  prefix_alphabet_.StopGrowth();
  suffix_alphabet_.StopGrowth();
  feats_alphabet_.StopGrowth();
  pos_alphabet_.StopGrowth();
  cpos_alphabet_.StopGrowth();

  LOG(INFO) << "Number of pos: " << pos_alphabet_.size();
  CHECK_LT(pos_alphabet_.size(), 0xff);
}
