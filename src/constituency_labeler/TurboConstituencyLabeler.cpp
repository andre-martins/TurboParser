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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <glog/logging.h>
#include <gflags/gflags.h>
#include "Utils.h"
#include "ConstituencyLabelerPipe.h"

void TrainConstituencyLabeler();
void TestConstituencyLabeler();

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_train) {
    LOG(INFO) << "Training constituency labeler..." << endl;
    TrainConstituencyLabeler();
  } else if (FLAGS_test) {
    LOG(INFO) << "Running constituency labeler..." << endl;
    TestConstituencyLabeler();
  }

  // Destroy allocated memory regarding line flags.
  google::ShutDownCommandLineFlags();
  google::ShutdownGoogleLogging();
  return 0;
}

void TrainConstituencyLabeler() {
  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  ConstituencyLabelerOptions *options = new ConstituencyLabelerOptions;
  options->Initialize();

  ConstituencyLabelerPipe *pipe = new ConstituencyLabelerPipe(options);
  pipe->Initialize();

  LOG(INFO) << "Training the constituency labeler...";
  pipe->Train();
  pipe->SaveModelFile();

  delete pipe;
  delete options;

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Training took " << time << " sec." << endl;
}

void TestConstituencyLabeler() {
  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  ConstituencyLabelerOptions *options = new ConstituencyLabelerOptions;
  options->Initialize();

  ConstituencyLabelerPipe *pipe = new ConstituencyLabelerPipe(options);
  pipe->Initialize();
  pipe->LoadModelFile();
  pipe->Run();

  delete pipe;
  delete options;

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Testing took " << time << " sec." << endl;
}
