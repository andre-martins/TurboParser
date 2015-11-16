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

#ifndef COREFERENCE_OPTIONS_H_
#define COREFERENCE_OPTIONS_H_

#include "Options.h"

class CoreferenceOptions : public Options {
public:
  CoreferenceOptions() {};
  virtual ~CoreferenceOptions() {};

  // Serialization functions.
  virtual void Load(FILE* fs);
  virtual void Save(FILE* fs);

  // Initialization: set options based on the flags.
  virtual void Initialize();

  // Get option flags.
  const std::string &file_mention_tags() { return file_mention_tags_; }
  const std::string &file_pronouns() { return file_pronouns_; }
  const std::string &file_determiners() { return file_determiners_; }
  const std::string &file_gender_number_statistics() {
    return file_gender_number_statistics_;
  }
  bool train_with_closest_antecedent() {
    return train_with_closest_antecedent_;
  }
  bool use_gender_number_determiners() {
    return use_gender_number_determiners_;
  }
  bool use_gender_number_statistics() {
    return use_gender_number_statistics_;
  }
  bool generate_noun_phrase_mentions_by_dependencies() {
    return generate_noun_phrase_mentions_by_dependencies_;
  }
  double false_anaphor_cost() { return false_anaphor_cost_; }
  double false_new_cost() { return false_new_cost_; }
  double false_wrong_link_cost() { return false_wrong_link_cost_; }

protected:
  std::string file_mention_tags_;
  std::string file_pronouns_;
  std::string file_determiners_;
  std::string file_gender_number_statistics_;
  bool train_with_closest_antecedent_;
  bool use_gender_number_determiners_;
  bool use_gender_number_statistics_;
  bool generate_noun_phrase_mentions_by_dependencies_;
  double false_anaphor_cost_;
  double false_new_cost_;
  double false_wrong_link_cost_;
};

#endif // COREFERENCE_OPTIONS_H_
