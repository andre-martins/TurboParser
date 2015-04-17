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

#include "CoreferenceOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

DEFINE_string(coreference_file_mention_tags, "",
              "Path to a file containing lists of mention tags (each line "
              "starts with exclude_entity_tags|noun_phrase_tags|"
              "proper_noun_tags|pronominal_tags and then has zero or more tags "
              "separated by tabs.");
DEFINE_string(coreference_file_pronouns, "",
              "Path to a file containing a list of pronouns classified by "
              "types.");
DEFINE_double(false_anaphor_cost, 0.1, "Cost of predicting anaphor when it "
              "should be a new cluster.");
DEFINE_double(false_new_cost, 3.0, "Cost of predicting new cluster when it "
              "should be anaphoric.");
DEFINE_double(false_wrong_link_cost, 1.0, "Cost of predicting an antecedent "
              "which is not coreferent (but assuming it is actually "
              "anaphoric.");

// Save current option flags to the model file.
void CoreferenceOptions::Save(FILE* fs) {
  Options::Save(fs);

  bool success;
  //success = WriteInteger(fs, model_type_);
  //CHECK(success);
  //success = WriteBool(fs, prune_tags_);
  //CHECK(success);
  success = WriteDouble(fs, false_anaphor_cost_);
  CHECK(success);
  success = WriteDouble(fs, false_new_cost_);
  CHECK(success);
  success = WriteDouble(fs, false_wrong_link_cost_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void CoreferenceOptions::Load(FILE* fs) {
  Options::Load(fs);

  bool success;
  //success = ReadInteger(fs, &FLAGS_sequence_model_type);
  //CHECK(success);
  //LOG(INFO) << "Setting --sequence_model_type=" << FLAGS_sequence_model_type;
  //success = ReadBool(fs, &FLAGS_tagger_large_feature_set);
  //CHECK(success);
  //LOG(INFO) << "Setting --tagger_large_feature_set="
  //          << FLAGS_tagger_large_feature_set;
  success = ReadDouble(fs, &FLAGS_false_anaphor_cost);
  CHECK(success);
  LOG(INFO) << "Setting --false_anaphor_cost=" << FLAGS_false_anaphor_cost;
  success = ReadDouble(fs, &FLAGS_false_new_cost);
  CHECK(success);
  LOG(INFO) << "Setting --false_new_cost=" << FLAGS_false_new_cost;
  success = ReadDouble(fs, &FLAGS_false_wrong_link_cost);
  CHECK(success);
  LOG(INFO) << "Setting --false_wrong_link_cost="
            << FLAGS_false_wrong_link_cost;

  Initialize();
}

void CoreferenceOptions::Initialize() {
  Options::Initialize();

  file_mention_tags_ = FLAGS_coreference_file_mention_tags;
  file_pronouns_ = FLAGS_coreference_file_pronouns;

  //model_type_ = FLAGS_sequence_model_type;
  //prune_tags_ = FLAGS_sequence_prune_tags;
  false_anaphor_cost_ = FLAGS_false_anaphor_cost;
  false_new_cost_ = FLAGS_false_new_cost;
  false_wrong_link_cost_ = FLAGS_false_wrong_link_cost;
}

