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

#include "SequenceDictionary.h"
#include "SequencePipe.h"
#include <algorithm>

void SequenceDictionary::CreateTagDictionary(SequenceReader *reader) {
  LOG(INFO) << "Creating tag dictionary...";
  vector<int> tag_freqs;

  // Go through the corpus and build the label dictionary, counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  SequenceInstance *instance = static_cast<SequenceInstance*>(reader->GetNext());
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
    instance = static_cast<SequenceInstance*>(reader->GetNext());
  }
  reader->Close();
  tag_alphabet_.StopGrowth();

  LOG(INFO) << "Number of tags: " << tag_alphabet_.size();
}
