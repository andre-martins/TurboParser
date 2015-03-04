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

#include "TaggerOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

// TODO: Implement the reader for "text".
DEFINE_string(tagger_file_format, "conll",
              "Format of the input file containing the data. Use ""conll"" for "
              "the format used in CONLL-X, and ""text"" for tokenized"
              "sentences (one per line, with tokens separated "
              "by white-spaces.");
//DEFINE_int32(tagger_model_type, 2,
//             "Model type. 1 is a bigram model, 2 is a trigram model.");
DEFINE_bool(tagger_large_feature_set, false,
            "True for using a large feature set. Taggers are usually more "
            "accurate but slower and have a larger memory footprint.");
DEFINE_bool(tagger_prune_tags, true,
            "True for pruning the set of possible tags by using a dictionary.");
DEFINE_string(file_unknown_word_tags, "",
              "Path to the file containing the possible tags to be assigned "
              "to out-of-vocabulary words.");

// Save current option flags to the model file.
void TaggerOptions::Save(FILE* fs) {
  SequenceOptions::Save(fs);

  bool success;
  //success = WriteInteger(fs, model_type_);
  //CHECK(success);
  success = WriteBool(fs, large_feature_set_);
  CHECK(success);
  success = WriteBool(fs, prune_tags_);
  CHECK(success);

  // TODO: Maybe we should load/save also the list of tags for unknown
  // words?
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void TaggerOptions::Load(FILE* fs) {
  SequenceOptions::Load(fs);

  bool success;
  //success = ReadInteger(fs, &FLAGS_tagger_model_type);
  //CHECK(success);
  //LOG(INFO) << "Setting --tagger_model_type=" << FLAGS_tagger_model_type;
  success = ReadBool(fs, &FLAGS_tagger_large_feature_set);
  CHECK(success);
  LOG(INFO) << "Setting --tagger_large_feature_set="
            << FLAGS_tagger_large_feature_set;
  success = ReadBool(fs, &FLAGS_tagger_prune_tags);
  CHECK(success);
  LOG(INFO) << "Setting --tagger_prune_tags=" << FLAGS_tagger_prune_tags;

  // TODO: Maybe we should load/save also the list of tags for unknown
  // words?

  Initialize();
}

void TaggerOptions::Initialize() {
  SequenceOptions::Initialize();

  file_format_ = FLAGS_tagger_file_format;
  //model_type_ = FLAGS_tagger_model_type;
  large_feature_set_ = FLAGS_tagger_large_feature_set;
  prune_tags_ = FLAGS_tagger_prune_tags;
  file_unknown_word_tags_ = FLAGS_file_unknown_word_tags;
}

