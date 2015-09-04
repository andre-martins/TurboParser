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

#ifndef ENTITYSPAN_H_
#define ENTITYSPAN_H_

//#include <cstddef>
#include <vector>
#include <string>

typedef class NamedSpan EntitySpan;

class Span {
 public:
  Span() { start_ = -1; end_ = -1; }
  Span(int start, int end) { start_ = start; end_ = end; }
  virtual ~Span() {}

  int start() const { return start_; }
  int end() const { return end_; }
  void set_start(int start) { start_ = start; }
  void set_end(int end) { end_ = end; }

  int GetLength() const { return end_ - start_ + 1; }
  bool ContainsIndex(int i) const { return start_ <= i && end_ >= i; }
  bool LiesInsideSpan(const Span &span) const {
    return start_ >= span.start() && end_ <= span.end();
  }
  bool OverlapNotNested(const Span &span) const {
    return
      ((start_ > span.start() && end_ > span.end() && start_ <= span.end()) ||
       (start_ < span.start() && end_ < span.end() && end_ >= span.start()));
  }
  Span *FindCoveringSpan(const std::vector<Span*> &spans) const {
    for (int i = 0; i < spans.size(); ++i) {
      if (LiesInsideSpan(*(spans[i]))) return spans[i];
    }
    return NULL;
  }

 protected:
  int start_;
  int end_;
};

class NamedSpan : public Span {
 public:
  NamedSpan() : Span() { name_ = ""; }
  NamedSpan(int start, int end, const std::string &name) : Span(start, end) {
    name_ = name;
  }
  virtual ~NamedSpan() {}

  const std::string &name() const { return name_; }
  void set_name(const std::string &name) { name_ = name; }

 protected:
  std::string name_;
};

class NumericSpan : public Span {
 public:
  NumericSpan() : Span() { id_ = -1; }
  NumericSpan(int start, int end, int id) : Span(start, end) {
    id_ = id;
  }
  virtual ~NumericSpan() {}

  int id() const { return id_; }
  void set_id(int id) { id_ = id; }

 protected:
  int id_;
};

#endif /* ENTITYSPAN_H_ */
