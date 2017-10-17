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

#include "SequenceOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

using namespace std;

DEFINE_int32(sequence_model_type, 2,
             "Model type. 1 is a bigram model, 2 is a trigram model.");
DEFINE_string(file_lexicon, "",
              "Path to the lexicon file.");

// Save current option flags to the model file.
void SequenceOptions::Save(FILE* fs) {
  Options::Save(fs);

  bool success;
  success = WriteInteger(fs, model_type_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void SequenceOptions::Load(FILE* fs) {
  Options::Load(fs);

  bool success;
  success = ReadInteger(fs, &FLAGS_sequence_model_type);
  CHECK(success);
  LOG(INFO) << "Setting --sequence_model_type=" << FLAGS_sequence_model_type;

  Initialize();
}

void SequenceOptions::Initialize() {
  Options::Initialize();

  model_type_ = FLAGS_sequence_model_type;
  file_lexicon_ = FLAGS_file_lexicon;
}
