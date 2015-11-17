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

#include "EntityOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

// TODO: Implement the reader for "text".
DEFINE_string(entity_file_format, "conll",
              "Format of the input file containing the data. Use ""conll"" for "
              "the format used in CONLL-X, and ""text"" for tokenized"
              "sentences (one per line, with tokens separated "
              "by white-spaces.");
DEFINE_string(entity_tagging_scheme, "bio",
              "The encoding scheme to represent entity spans as tags. Either "
              """io"", ""bio"", or ""bilou"".");
DEFINE_string(entity_file_gazetteer, "",
              "Path to a gazetteer file (one entity per line with the "
              "corresponding class, separated by tabs.");
DEFINE_int32(entity_recognizer_large_feature_set, 2,
             "The greater the value, the larger feature set used. Taggers are "
             "usually more accurate but slower and have a larger memory footprint.");

// Save current option flags to the model file.
void EntityOptions::Save(FILE* fs) {
  SequenceOptions::Save(fs);

  bool success;
  success = WriteString(fs, tagging_scheme_name_);
  CHECK(success);
  success = WriteInteger(fs, large_feature_set_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void EntityOptions::Load(FILE* fs) {
  SequenceOptions::Load(fs);

  bool success;
  success = ReadString(fs, &FLAGS_entity_tagging_scheme);
  CHECK(success);
  LOG(INFO) << "Setting --entity_tagging_scheme=" <<
    FLAGS_entity_tagging_scheme;
  success = ReadInteger(fs, &FLAGS_entity_recognizer_large_feature_set);
  CHECK(success);
  LOG(INFO) << "Setting --entity_recognizer_large_feature_set="
    << FLAGS_entity_recognizer_large_feature_set;

  Initialize();
}

void EntityOptions::Initialize() {
  SequenceOptions::Initialize();

  file_format_ = FLAGS_entity_file_format;
  file_gazetteer_ = FLAGS_entity_file_gazetteer;
  tagging_scheme_name_ = FLAGS_entity_tagging_scheme;
  if (tagging_scheme_name_ == "io") {
    tagging_scheme_ = EntityTaggingSchemes::IO;
  } else if (tagging_scheme_name_ == "bio") {
    tagging_scheme_ = EntityTaggingSchemes::BIO;
  } else if (tagging_scheme_name_ == "bilou") {
    tagging_scheme_ = EntityTaggingSchemes::BILOU;
  } else {
    CHECK(false) << "Unknown entity scheme: " << tagging_scheme_name_;
  }
  large_feature_set_ = FLAGS_entity_recognizer_large_feature_set;
}
