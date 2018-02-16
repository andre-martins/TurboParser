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

#include "SemanticWriter.h"
#include "SemanticInstance.h"
#include "SemanticOptions.h"
#include "Utils.h"
#include <iostream>
#include <sstream>

void SemanticWriter::Write(Instance *instance) {
  SemanticInstance *semantic_instance =
    static_cast<SemanticInstance*>(instance);
  SemanticOptions *semantic_options =
    static_cast<SemanticOptions*>(options_);
  if (semantic_options->allow_root_predicate()) {
    UseTopNodes(true);
  } else {
    UseTopNodes(false);
  }
  const string &format = semantic_options->file_format();
  SetFormat(format);

  if (semantic_instance->GetName() != "") {
    os_ << semantic_instance->GetName() << endl;
  }

  bool write_semantic_roles = true;
  int first_predicate = 0;
  vector<int> top_nodes;
  if (use_top_nodes_) {
    if (semantic_instance->GetNumPredicates() > 0 &&
        0 == semantic_instance->GetPredicateIndex(0)) {
      int num_top_nodes = semantic_instance->GetNumArgumentsPredicate(0);
      top_nodes.resize(num_top_nodes);
      for (int l = 0; l < num_top_nodes; ++l) {
        top_nodes[l] = semantic_instance->GetArgumentIndex(0, l);
      }
      ++first_predicate; // Skip the root.
    }
  }

  int current_predicate = first_predicate;
  for (int i = 1; i < semantic_instance->size(); ++i) {
    os_ << i << "\t";
    if (!use_sdp_format_) {
      os_ << "_" << "\t"; // Change this later
      os_ << "_" << "\t"; // Change this later
      os_ << "_" << "\t"; // Change this later
      os_ << "_" << "\t"; // Change this later
    }
    os_ << semantic_instance->GetForm(i) << "\t";
    os_ << semantic_instance->GetLemma(i) << "\t";
    os_ << semantic_instance->GetPosTag(i) << "\t";
    os_ << semantic_instance->GetHead(i) << "\t";
    os_ << semantic_instance->GetDependencyRelation(i) << "\t";

    if (write_semantic_roles) {
      bool is_top_node = false;
      for (int l = 0; l < top_nodes.size(); ++l) {
        if (i == top_nodes[l]) {
          is_top_node = true;
          break;
        }
      }
      if (use_sdp_format_) {
        if (is_top_node) {
          os_ << '+' << "\t";
        } else {
          os_ << '-' << "\t";
        }
      }

      bool is_predicate_node = false;
      string predicate_name = "_";
      if (current_predicate < semantic_instance->GetNumPredicates() &&
          i == semantic_instance->GetPredicateIndex(current_predicate)) {
        is_predicate_node = true;
        predicate_name = semantic_instance->GetPredicateName(current_predicate);
        ++current_predicate;
      }
      if (use_sdp_format_) {
        if (is_predicate_node) {
          os_ << "+" << "\t";
        } else {
          os_ << "-" << "\t";
        }
        //os_ << predicate_name << "\t";
        os_ << "_" << "\t";
      } else {
        os_ << predicate_name << "\t";
      }
      for (int k = first_predicate;
      k < semantic_instance->GetNumPredicates();
        ++k) {
        string argument_name = "_";
        for (int l = 0; l < semantic_instance->GetNumArgumentsPredicate(k); ++l) {
          if (i == semantic_instance->GetArgumentIndex(k, l)) {
            int p = semantic_instance->GetPredicateIndex(k);
            //LOG(INFO) << semantic_instance->GetForm(i)
            //          << " <- "
            //          << semantic_instance->GetForm(p);
            argument_name = semantic_instance->GetArgumentRole(k, l);
          }
        }
        os_ << argument_name;
        if (k < semantic_instance->GetNumPredicates() - 1) os_ << "\t";
      }
    }

    os_ << endl;
  }
  os_ << endl;
}

void SemanticWriter::WriteFormatted(Pipe * pipe, Instance *instance) {}