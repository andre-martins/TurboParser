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

#ifndef UTILS_H
#define UTILS_H

#include <atomic>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include "TimeUtils.h"
#include "StringUtils.h"

class GlogIsInit {
public:
  static GlogIsInit & Instance() {
    static GlogIsInit instance;
    return instance;
  }

  static bool Value() {
    return Instance().value_;
  }

  static void Set(bool value) {
    Instance().value_.store(value);
  }
private:
  GlogIsInit() {};
  std::atomic_bool value_{ false };
};

static void InitGlog(const char * logging_name) {


  if (!GlogIsInit::Value()) {
    //fLB::FLAGS_colorlogtostderr = true;
    fLB::FLAGS_alsologtostderr = true;
    fLI::FLAGS_stderrthreshold = 0;
    fLI::FLAGS_minloglevel = 0;
    fLI::FLAGS_v = 0;
    google::InitGoogleLogging(logging_name);
    GlogIsInit::Set(true);
  }
}

#endif // UTILS_H
