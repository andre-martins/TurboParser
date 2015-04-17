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

#include "CoreferenceDictionary.h"
#include "CoreferencePipe.h"
#include <algorithm>

void CoreferenceDictionary::CreateEntityDictionary(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating entity dictionary...";
  std::vector<int> entity_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    for (int i = 0; i < instance->GetEntitySpans().size(); ++i) {
      int id;

      // Add constituent to alphabet.
      id = entity_alphabet_.
        Insert(instance->GetEntitySpans()[i]->name());
      if (id >= entity_freqs.size()) {
        CHECK_EQ(id, entity_freqs.size());
        entity_freqs.push_back(0);
      }
      ++entity_freqs[id];
    }
    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();
  entity_alphabet_.StopGrowth();

  LOG(INFO) << "Number of entities: " << entity_alphabet_.size();
  for (Alphabet::iterator it = entity_alphabet_.begin();
       it != entity_alphabet_.end();
       ++it) {
    LOG(INFO) << it->first;
  }
}

void CoreferenceDictionary::CreateConstituentDictionary(
    CoreferenceSentenceReader *reader) {
  LOG(INFO) << "Creating constituent dictionary...";
  std::vector<int> constituent_freqs;

  // Go through the corpus and build the label dictionary,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  CoreferenceSentence *instance =
    static_cast<CoreferenceSentence*>(reader->GetNext());
  while (instance != NULL) {
    for (int i = 0; i < instance->GetConstituentSpans().size(); ++i) {
      int id;

      // Add constituent to alphabet.
      id = constituent_alphabet_.
        Insert(instance->GetConstituentSpans()[i]->name());
      if (id >= constituent_freqs.size()) {
        CHECK_EQ(id, constituent_freqs.size());
        constituent_freqs.push_back(0);
      }
      ++constituent_freqs[id];
    }
    delete instance;
    instance = static_cast<CoreferenceSentence*>(reader->GetNext());
  }
  reader->Close();
  constituent_alphabet_.StopGrowth();

  LOG(INFO) << "Number of constituents: " << constituent_alphabet_.size();
  for (Alphabet::iterator it = constituent_alphabet_.begin();
       it != constituent_alphabet_.end();
       ++it) {
    LOG(INFO) << it->first;
  }
}
