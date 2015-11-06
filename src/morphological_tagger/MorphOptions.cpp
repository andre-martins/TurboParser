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

#include "MorphOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

// TODO: Implement the reader for "text".
DEFINE_string(morph_file_format, "conll",
              "Format of the input file containing the data. Use ""conll"" for "
              "the format used in CONLL-X, and ""text"" for tokenized"
              "sentences (one per line, with tokens separated "
              "by white-spaces.");
DEFINE_bool(morph_tagger_large_feature_set, false,
            "True for using a large feature set. Taggers are usually more "
            "accurate but slower and have a larger memory footprint.");
DEFINE_bool(morph_tagger_prune_tags, true,
            "True for pruning the set of possible tags by using a dictionary.");

// Save current option flags to the model file.
void MorphOptions::Save(FILE* fs) {
  SequenceOptions::Save(fs);

  //Express here the storage procedure of option flags, as :
  //bool success;
  //success = Write{String/Double/Bool}(fs, morph_var_);
  //CHECK(success);

  bool success;
  success = WriteBool(fs, large_feature_set_);
  CHECK(success);
  success = WriteBool(fs, prune_tags_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void MorphOptions::Load(FILE* fs) {
  SequenceOptions::Load(fs);

  //Express here the loading procedure of option flags, as :
  //bool success;
  //success = Read{ String / Double / Bool }(fs, &FLAGS_morph_var);
  //CHECK(success);
  //LOG(INFO) << "Setting --morph_var=" <<
  //  FLAGS_morph_var;

  bool success;
  success = ReadBool(fs, &FLAGS_morph_tagger_large_feature_set);
  CHECK(success);
  LOG(INFO)<<"Setting --tagger_large_feature_set="
    <<FLAGS_morph_tagger_large_feature_set;
  success = ReadBool(fs, &FLAGS_morph_tagger_prune_tags);
  CHECK(success);
  LOG(INFO)<<"Setting --tagger_prune_tags="<<FLAGS_morph_tagger_prune_tags;

  Initialize();
}

void MorphOptions::Initialize() {
  SequenceOptions::Initialize();

  file_format_ = FLAGS_morph_file_format;
  large_feature_set_ = FLAGS_morph_tagger_large_feature_set;
  prune_tags_ = FLAGS_morph_tagger_prune_tags;

}

