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

#ifndef COREFERENCEREADER_H_
#define COREFERENCEREADER_H_

#include "CoreferenceDocument.h"
#include "SemanticReader.h"
#include "Options.h"
#include <fstream>

class CoreferenceSentenceReader : public SemanticReader {
 public:
  CoreferenceSentenceReader() {
    options_ = NULL;
    use_sdp_format_ = false;
    use_top_nodes_ = false;
  }
  CoreferenceSentenceReader(Options *options) {
    options_ = options;
    use_sdp_format_ = false;
    use_top_nodes_ = false;
  }
  virtual ~CoreferenceSentenceReader() {}

 public:
  Instance *GetNext();
  void set_options(Options *options) { options_ = options; }

 protected:
  void ConstructSpansFromText(const std::vector<std::string> &span_lines,
                              std::vector<NamedSpan*> *spans);
  void ConstructCoreferenceSpansFromText(
    const std::vector<std::string> &span_lines,
    std::vector<NamedSpan*> *spans);

 protected:
  Options *options_;
};

class CoreferenceReader : public Reader {
 public:
  CoreferenceReader() {
    options_ = NULL;
  }
  CoreferenceReader(Options *options) {
    options_ = options;
    sentence_reader_.set_options(options);
  }
  virtual ~CoreferenceReader() {}

 public:
  void Open(const string &filepath) {
    Reader::Open(filepath);
    sentence_reader_.Open(filepath);
  }
  void Close() {
    Reader::Close();
    sentence_reader_.Close();
  }
  Instance *GetNext();

  CoreferenceSentenceReader *GetSentenceReader() { return &sentence_reader_; }

 protected:
  CoreferenceSentenceReader sentence_reader_;
  Options *options_;
};

#endif /* COREFERENCEREADER_H_ */

