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

#ifndef COREFERENCEWRITER_H_
#define COREFERENCEWRITER_H_

#include "SemanticWriter.h"
#include "EntitySpan.h"

class CoreferenceSentenceWriter : public SemanticWriter {
public:
  CoreferenceSentenceWriter() {
    external_os_ = NULL;
    options_ = NULL;
    use_sdp_format_ = false;
    use_top_nodes_ = false;
  }
  CoreferenceSentenceWriter(Options *options) {
    external_os_ = NULL;
    options_ = options;
    use_sdp_format_ = false;
    use_top_nodes_ = false;
  }
  virtual ~CoreferenceSentenceWriter() {}

public:
  void SetOutputStream(std::ofstream *os) { external_os_ = os; }
  void Write(Instance *instance);
  void WriteFormatted(Pipe * pipe, Instance *instance);
  void set_options(Options *options) { options_ = options; }

protected:
  void ConstructTextFromCoreferenceSpans(int length,
                                         const std::vector<NamedSpan*> &spans,
                                         std::vector<std::string> *span_lines);

protected:
  Options *options_;
  std::ofstream *external_os_;
};

class CoreferenceWriter : public Writer {
public:
  CoreferenceWriter() { options_ = NULL; }
  CoreferenceWriter(Options *options) {
    options_ = options;
    sentence_writer_.set_options(options);
  }
  virtual ~CoreferenceWriter() {};

public:
  void Open(const string &filepath) {
    Writer::Open(filepath);
    sentence_writer_.SetOutputStream(&os_);
  }
  void Close() {
    Writer::Close();
  }
  void Write(Instance *instance);
  void WriteFormatted(Pipe * pipe, Instance *instance);

  CoreferenceSentenceWriter *GetSentenceWriter() { return &sentence_writer_; }

protected:
  CoreferenceSentenceWriter sentence_writer_;
  Options *options_;
};

#endif /* COREFERENCEWRITER_H_ */
