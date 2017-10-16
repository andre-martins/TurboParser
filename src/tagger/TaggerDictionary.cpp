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

#include "TaggerDictionary.h"
#include "TaggerOptions.h"
#include "TaggerPipe.h"
#include <algorithm>

void TaggerDictionary::CreateTagDictionary(SequenceReader *reader) {
  SequenceDictionary::CreateTagDictionary(reader);

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

  // If there is a lexicon of words and tags, load it.
  lexicon_word_alphabet_.clear();
  lexicon_word_tags_.clear();
  TaggerOptions *options =
    static_cast<TaggerOptions*>(pipe_->GetOptions());
  if (options->GetLexiconFilePath().size() > 0) {
    LOG(INFO) << "Loading lexicon file...";
    std::ifstream is;
    is.open(options->GetLexiconFilePath().c_str(), ifstream::in);
    CHECK(is.good()) << "Could not open "
      << options->GetLexiconFilePath() << ".";
    string line;
    // One pass to get the words from the lexicon.
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line.size() == 0) continue;
        vector<string> fields;
        StringSplit(line, "\t", &fields, true);
        CHECK_EQ(fields.size(), 2);
        string word = fields[0];
        int word_id = lexicon_word_alphabet_.Insert(word);
      }
    }
    lexicon_word_alphabet_.StopGrowth();
    lexicon_word_tags_.clear();
    lexicon_word_tags_.resize(lexicon_word_alphabet_.size());
    if (is.is_open()) {
      while (!is.eof()) {
        getline(is, line);
        if (line.size() == 0) continue;
        vector<string> fields;
        StringSplit(line, "\t", &fields, true);
        CHECK_EQ(fields.size(), 2);
        string word = fields[0];
        string tag = fields[1];
        LOG(INFO) << word << " " << tag;
        int word_id = lexicon_word_alphabet_.Lookup(word);
        int tag_id = tag_alphabet_.Lookup(tag);
        CHECK_GE(word_id, 0);
        CHECK_GE(tag_id, 0) << "Tag " << tag << " does not exist.";
        vector<int> &tags = lexicon_word_tags_[word_id];
        int j;
        for (j = 0; j < tags.size(); ++j) {
          if (tags[j] == tag_id) break;
        }
        if (j == tags.size()) tags.push_back(tag_id);
      }
    }
    LOG(INFO) << "Loaded lexicon with "
              << lexicon_word_alphabet_.size() << " words.";
  }

  // If there is a list of possible tags for the unknown words, load it.
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
}
