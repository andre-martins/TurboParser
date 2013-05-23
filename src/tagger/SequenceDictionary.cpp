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

#include "SequenceDictionary.h"
#include "SequencePipe.h"
#include <algorithm>

void SequenceDictionary::CreateTagDictionary(SequenceReader *reader) {
  LOG(INFO) << "Creating tag dictionary...";
  bool form_case_sensitive = FLAGS_form_case_sensitive;

  vector<int> tag_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  SequenceInstance *instance = reader->GetNext();
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 0; i < instance_length; ++i) {
      int id;

      // Add tag to alphabet.
      id = tag_alphabet_.Insert(instance->GetTag(i));
      if (id >= tag_freqs.size()) {
        CHECK_EQ(id, tag_freqs.size());
        tag_freqs.push_back(0);
      }
      ++tag_freqs[id];
    }
    delete instance;
    instance = reader->GetNext();
  }
  reader->Close();
  tag_alphabet_.StopGrowth();

  // Go through the corpus and build the existing labels for each head-modifier
  // POS pair.
  word_tags_.clear();
  word_tags_.resize(token_dictionary_->GetNumForms());

  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  instance = reader->GetNext();
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
    instance = reader->GetNext();
  }
  reader->Close();

  LOG(INFO) << "Number of tags: " << tag_alphabet_.size();

  // If there is a list of possible tags for the unknown words, load it.
  SequenceOptions *options =
    static_cast<SequenceOptions*>(pipe_->GetOptions());
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
}
