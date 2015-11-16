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
#include "CoreferencePipe.h"

void TrainCoreferenceResolver();
void TestCoreferenceResolver();

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc, &argv, true);

  if (FLAGS_train) {
    LOG(INFO) << "Training coreference resolver..." << endl;
    TrainCoreferenceResolver();
  } else if (FLAGS_test) {
    LOG(INFO) << "Running coreference resolver..." << endl;
    TestCoreferenceResolver();
  }

  // Destroy allocated memory regarding line flags.
  google::ShutDownCommandLineFlags();
  google::ShutdownGoogleLogging();
  return 0;
}

void TrainCoreferenceResolver() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  CoreferenceOptions *options = new CoreferenceOptions;
  options->Initialize();

  CoreferencePipe *pipe = new CoreferencePipe(options);
  pipe->Initialize();

  LOG(INFO) << "Training the coreference resolver...";
  pipe->Train();
  pipe->SaveModelFile();

  delete pipe;
  delete options;

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Training took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TestCoreferenceResolver() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  CoreferenceOptions *options = new CoreferenceOptions;
  options->Initialize();

  CoreferencePipe *pipe = new CoreferencePipe(options);
  pipe->Initialize();
  pipe->LoadModelFile();
  pipe->Run();

  delete pipe;
  delete options;

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Testing took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}
