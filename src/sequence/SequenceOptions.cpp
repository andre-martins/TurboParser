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

#include "SequenceOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

DEFINE_int32(sequence_model_type, 2,
             "Model type. 1 is a bigram model, 2 is a trigram model.");
//DEFINE_bool(tagger_large_feature_set, false,
//            "True for using a large feature set. Taggers are usually more "
//            "accurate but slower and have a larger memory footprint.");
//DEFINE_bool(sequence_prune_tags, true,
//            "True for pruning the set of possible tags by using a dictionary.");

// Save current option flags to the model file.
void SequenceOptions::Save(FILE* fs) {
  Options::Save(fs);

  bool success;
  success = WriteInteger(fs, model_type_);
  CHECK(success);
  //success = WriteBool(fs, large_feature_set_);
  //CHECK(success);
  //success = WriteBool(fs, prune_tags_);
  //CHECK(success);

  // TODO: Maybe we should load/save also the list of tags for unknown
  // words?
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void SequenceOptions::Load(FILE* fs) {
  Options::Load(fs);

  bool success;
  success = ReadInteger(fs, &FLAGS_sequence_model_type);
  CHECK(success);
  LOG(INFO) << "Setting --sequence_model_type=" << FLAGS_sequence_model_type;
  //success = ReadBool(fs, &FLAGS_tagger_large_feature_set);
  //CHECK(success);
  //LOG(INFO) << "Setting --tagger_large_feature_set="
  //          << FLAGS_tagger_large_feature_set;
  //success = ReadBool(fs, &FLAGS_sequence_prune_tags);
  //CHECK(success);
  //LOG(INFO) << "Setting --sequence_prune_tags=" << FLAGS_sequence_prune_tags;

  // TODO: Maybe we should load/save also the list of tags for unknown
  // words?

  Initialize();
}

void SequenceOptions::Initialize() {
  Options::Initialize();

  //file_format_ = FLAGS_tagger_file_format;
  model_type_ = FLAGS_sequence_model_type;
  //large_feature_set_ = FLAGS_tagger_large_feature_set;
  //prune_tags_ = FLAGS_sequence_prune_tags;
  //file_unknown_word_tags_ = FLAGS_file_unknown_word_tags;
}

