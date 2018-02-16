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

#include "ConstituencyLabelerWriter.h"
#include "ConstituencyLabelerInstance.h"
#include <iostream>
#include <sstream>

void ConstituencyLabelerWriter::Write(Instance *instance) {
  ConstituencyLabelerInstance *labeler_instance =
    static_cast<ConstituencyLabelerInstance*>(instance);
  std::string info;
  ParseTree *parse_tree = labeler_instance->GetMutableParseTree();
  for (int i = 0; i < labeler_instance->GetNumConstituents(); ++i) {
    ParseTreeNode *node = parse_tree->non_terminals()[i];
    const std::string label = labeler_instance->GetConstituentLabel(i);
    if (label != "") {
      node->set_label(label + kParseTreeLabelSeparator + node->label());
    }
  }
  // Expand unary spines.
  parse_tree->ExpandSingletonSpines();
  parse_tree->SaveToString(&info);
  os_ << info << endl;
}

void ConstituencyLabelerWriter::WriteFormatted(Pipe * pipe, Instance *instance) {}
