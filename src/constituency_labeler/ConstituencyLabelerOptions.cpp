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

#include "ConstituencyLabelerOptions.h"
#include "SerializationUtils.h"
#include <glog/logging.h>

// TODO: Implement the text format.
DEFINE_string(file_format, "ptb",
              "Format of the input file containing the data. Use ""ptb"" for "
              "the format used in the Penn Treebank (one sentence per line)");
DEFINE_bool(prune_labels, true,
            "True for pruning the set of possible labels taking into account "
            "the labels that have occured for each constituent node in the "
            "training data.");
DEFINE_string(null_label, "",
              "NULL label (can be treated in a special way, e.g. designing a "
              "suitable cost function or when --ignore_null_label=true.");
DEFINE_bool(ignore_null_labels, false,
            "If true, do not create parts for NULL labels.");

// Save current option flags to the model file.
void ConstituencyLabelerOptions::Save(FILE* fs) {
  Options::Save(fs);

  bool success;
  success = WriteBool(fs, prune_labels_);
  CHECK(success);
  success = WriteString(fs, null_label_);
  CHECK(success);
  success = WriteBool(fs, ignore_null_labels_);
  CHECK(success);
}

// Load current option flags to the model file.
// Note: this will override the user-specified flags.
void ConstituencyLabelerOptions::Load(FILE* fs) {
  Options::Load(fs);

  bool success;
  success = ReadBool(fs, &FLAGS_prune_labels);
  CHECK(success);
  LOG(INFO) << "Setting --prune_labels=" << FLAGS_prune_labels;
  success = ReadString(fs, &FLAGS_null_label);
  CHECK(success);
  LOG(INFO) << "Setting --null_label=" << FLAGS_null_label;
  success = ReadBool(fs, &FLAGS_ignore_null_labels);
  CHECK(success);
  LOG(INFO) << "Setting --ignore_null_labels=" << FLAGS_ignore_null_labels;

  Initialize();
}

void ConstituencyLabelerOptions::Initialize() {
  Options::Initialize();

  file_format_ = FLAGS_file_format;
  prune_labels_ = FLAGS_prune_labels;
  null_label_ = FLAGS_null_label;
  ignore_null_labels_ = FLAGS_ignore_null_labels;
}

