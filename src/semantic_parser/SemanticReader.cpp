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

#include "SemanticReader.h"
#include "SemanticOptions.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

using namespace std;

Instance *SemanticReader::GetNext() {
  SemanticOptions *semantic_options =
    static_cast<SemanticOptions*>(options_);
  if (semantic_options->allow_root_predicate()) {
    UseTopNodes(true);
  } else {
    UseTopNodes(false);
  }
  const string &format = semantic_options->file_format();
  SetFormat(format);

  // Fill all fields for the entire sentence.
  string name = "";
  vector<vector<string> > sentence_fields;
  string line;
  if (is_.is_open()) {
    while (!is_.eof()) {
      getline(is_, line);
      if (line.length() <= 0) break;
      if (0 == line.substr(0, 1).compare("#")) {
        //LOG(INFO) << line;
	if (name != "") {
	  name += "\n" + line;
	} else {
	  name = line;
	}
        continue; // Sentence ID.
      }
      vector<string> fields;
      StringSplit(line, "\t", &fields);
      sentence_fields.push_back(fields);
    }
  }

  bool read_next_sentence = false;
  if (!is_.eof()) read_next_sentence = true;

  // Sentence length.
  int length = sentence_fields.size();

  // Convert to array of forms, lemmas, etc.
  // Note: the first token is the root symbol.
  vector<string> forms(length+1);
  vector<string> lemmas(length+1);
  vector<string> cpos(length+1);
  vector<string> pos(length+1);
  vector<vector<string> > feats(length+1);
  vector<string> deprels(length+1);
  vector<int> heads(length+1);
  vector<string> predicate_names; // Names of predicates (e.g. "take.01").
  vector<int> predicate_indices; // Positions of each predicate in the sentence.
  vector<vector<string> > argument_roles; // Semantic roles.
  vector<vector<int> > argument_indices; // Positions of each argument.

  forms[0] = "_root_";
  lemmas[0] = "_root_";
  cpos[0] = "_root_";
  pos[0] = "_root_";
  deprels[0] = "_root_";
  heads[0] = -1;
  feats[0] = vector<string>(1, "_root_");

  bool read_semantic_roles = true;
  int num_predicates = 0;
  for(int i = 0; i < length; i++) {
    const vector<string> &info = sentence_fields[i];

    int offset = 1;
    if (!use_sdp_format_) {
      offset += 4;
    }

    // Use splitted forms.
    forms[i+1] = info[offset];
    ++offset;
    lemmas[i+1] = info[offset];
    ++offset;
    cpos[i+1] = info[offset];
    pos[i+1] = info[offset]; // No distiction between pos and cpos.
    ++offset;

    // No morpho-syntactic information.
    feats[i+1].clear();

    if (!semantic_options->use_dependency_syntactic_features()) {
      heads[i+1] = 0;
      deprels[i+1] = "NULL";
      offset += 2;
    } else {
      stringstream ss(info[offset]);
      ++offset;
      ss >> heads[i+1];
      deprels[i+1] = info[offset];
      ++offset;

      if (heads[i+1] < 0 || heads[i+1] > length) {
        LOG(INFO) << "Invalid value of head (" << heads[i+1]
                  << ") not in range [0.." << length
                  << "] - attaching to the root.";
        heads[i+1] = 0;
      }
    }

    // Semantic role labeling information.
    if (read_semantic_roles) {
      bool is_top = false; // For sdp format only.
      bool is_predicate = false;
      if (use_sdp_format_) {
        string top_name = info[offset];
        ++offset;
        CHECK(0 == top_name.compare("-") || 0 == top_name.compare("+"));
        if (0 == top_name.compare("+")) is_top = true;
        string predicate_flag = info[offset];
        ++offset;
        CHECK(0 == predicate_flag.compare("-") ||
              0 == predicate_flag.compare("+"));
        if (0 == predicate_flag.compare("+")) is_predicate = true;
      }
      string predicate_name = info[offset];
      ++offset;
      if (!use_sdp_format_) {
        if (0 != predicate_name.compare("_")) is_predicate = true;
      }
      if (!use_sdp_format_) CHECK_EQ(offset, 11);
      if (i == 0) {
        // Allocate space for predicates.
        num_predicates = info.size() - offset;
        // Top nodes will be considered arguments of a special root node.
        if (use_top_nodes_) ++num_predicates;
        predicate_names.resize(num_predicates);
        predicate_indices.resize(num_predicates);
        argument_roles.resize(num_predicates);
        argument_indices.resize(num_predicates);
        num_predicates = 0;
        if (use_top_nodes_) {
          predicate_names[num_predicates] = "__ROOT__";
          predicate_indices[num_predicates] = 0;
          ++num_predicates;
        }
      }

      if (is_top) {
        //if (i == 0) {
        //  LOG(INFO) << "Top: " << i+1;
        //}
        argument_roles[0].push_back("__TOP__");
        argument_indices[0].push_back(i+1);
      }

      if (is_predicate) {
        predicate_names[num_predicates] = predicate_name;
        predicate_indices[num_predicates] = i+1;
        ++num_predicates;
      }

      for (int j = offset; j < info.size(); ++j) {
        string argument_role = info[j];
        bool is_argument = false;
        if (0 != argument_role.compare("_")) is_argument = true;
        if (is_argument) {
          int k = j - offset;
          if (use_top_nodes_) ++k;
          argument_roles[k].push_back(argument_role);
          argument_indices[k].push_back(i+1);
        }
      }
    }
  }

  CHECK_EQ(num_predicates, predicate_names.size());

  SemanticInstance *instance = NULL;
  if (read_next_sentence && length >= 0) {
    instance = new SemanticInstance;
    instance->Initialize(name, forms, lemmas, cpos, pos, feats, deprels, heads,
                         predicate_names, predicate_indices, argument_roles,
                         argument_indices);
  }

  return static_cast<Instance*>(instance);
}
