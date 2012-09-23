// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.0.
//
// TurboParser 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.0.  If not, see <http://www.gnu.org/licenses/>.

#include "DependencyDictionary.h"
#include "DependencyPipe.h"

void DependencyDictionary::CreateLabelDictionary(DependencyReader *reader) {
  LOG(INFO) << "Creating label dictionary...";

  vector<int> label_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  DependencyInstance *instance = reader->GetNext();
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 1; i < instance_length; ++i) {
      int id;

      // Add dependency label to alphabet.
      id = label_alphabet_.Insert(instance->GetDependencyRelation(i));
      if (id >= label_freqs.size()) {
        CHECK_EQ(id, label_freqs.size());
        label_freqs.push_back(0);
      }
      ++label_freqs[id];
    }
    delete instance;
    instance = reader->GetNext();
  }
  reader->Close();
  label_alphabet_.StopGrowth();

  // Go through the corpus and build the existing labels for each head-modifier
  // POS pair.
  existing_labels_.clear();
  existing_labels_.resize(token_dictionary_->GetNumPosTags(),
                          vector<vector<int> >(
                            token_dictionary_->GetNumPosTags()));

  maximum_left_distances_.clear();
  maximum_left_distances_.resize(token_dictionary_->GetNumPosTags(),
                                 vector<int>(
                                   token_dictionary_->GetNumPosTags(), 0));

  maximum_right_distances_.clear();
  maximum_right_distances_.resize(token_dictionary_->GetNumPosTags(),
                                  vector<int>(
                                    token_dictionary_->GetNumPosTags(), 0));

  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  instance = reader->GetNext();
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int i = 1; i < instance_length; ++i) {
      int id;
      int head = instance->GetHead(i);
      const string &modifier_pos = instance->GetPosTag(i);
      const string &head_pos = instance->GetPosTag(head);
      int modifier_pos_id = token_dictionary_->GetPosTagId(modifier_pos);
      int head_pos_id = token_dictionary_->GetPosTagId(head_pos);
      if (modifier_pos_id < 0) modifier_pos_id = TOKEN_UNKNOWN;
      if (head_pos_id < 0) head_pos_id = TOKEN_UNKNOWN;
      //CHECK_GE(modifier_pos_id, 0);
      //CHECK_GE(head_pos_id, 0);

      id = label_alphabet_.Lookup(instance->GetDependencyRelation(i));
      CHECK_GE(id, 0);

      // Insert new label in the set of existing labels, if it is not there
      // already. NOTE: this is inefficient, maybe we should be using a 
      // different data structure.
      vector<int> &labels = existing_labels_[modifier_pos_id][head_pos_id];
      int j;
      for (j = 0; j < labels.size(); ++j) {
        if (labels[j] == id) break;
      }
      if (j == labels.size()) labels.push_back(id);

      // Update the maximum distances if necessary.
      if (head != 0) {
        if (head < i) {
          // Right attachment.
          if (i - head > maximum_right_distances_[modifier_pos_id][head_pos_id]) {
            maximum_right_distances_[modifier_pos_id][head_pos_id] = i - head;
          }
        } else {
          // Left attachment.
          if (head - i > maximum_left_distances_[modifier_pos_id][head_pos_id]) {
            maximum_left_distances_[modifier_pos_id][head_pos_id] = head - i;
          }
        }
      }
    }
    delete instance;
    instance = reader->GetNext();
  }
  reader->Close();

  LOG(INFO) << "Number of labels: " << label_alphabet_.size();
}
