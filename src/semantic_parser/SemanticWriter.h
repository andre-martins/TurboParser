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

#ifndef SEMANTICWRITER_H_
#define SEMANTICWRITER_H_

#include "DependencyWriter.h"
#include <fstream>

using namespace std;

class SemanticWriter : public DependencyWriter {
public:
  SemanticWriter() {
    use_sdp_format_ = false;
    use_top_nodes_ = false;
  }
  virtual ~SemanticWriter() {}

public:
  void UseTopNodes(bool use_top_nodes) { use_top_nodes_ = use_top_nodes; }
  void SetFormat(const string &format) {
    if (format == "sdp") {
      use_sdp_format_ = true;
    } else {
      use_sdp_format_ = false;
    }
  }
  void Write(Instance *instance);

 protected:
  bool use_sdp_format_;
  bool use_top_nodes_;
};

#endif /* SEMANTICWRITER_H_ */

