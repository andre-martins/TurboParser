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
#include "MorphPipe.h"

using namespace std;

void TrainMorph();
void TestMorph();

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc, &argv, true);

#ifdef _WIN32
  google::LogToStderr();
#endif
  if (FLAGS_train) {
    LOG(INFO)<<"Training Morphological tagger..."<<endl;
    TrainMorph();
  } else if (FLAGS_test) {
    LOG(INFO)<<"Running Morphological tagger..."<<endl;
    TestMorph();
  }

  // Destroy allocated memory regarding line flags.
  google::ShutDownCommandLineFlags();
  google::ShutdownGoogleLogging();
  return 0;
}

void TrainMorph() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  MorphOptions *options = new MorphOptions;
  options->Initialize();

  MorphPipe *pipe = new MorphPipe(options);
  pipe->Initialize();
  pipe->Train();
  pipe->SaveModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO)<<"Training took "<<static_cast<double>(time)/1000.0
    <<" sec."<<endl;

  delete pipe;
  delete options;
}

void TestMorph() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  MorphOptions *options = new MorphOptions;
  options->Initialize();

  MorphPipe *pipe = new MorphPipe(options);
  pipe->Initialize();
  pipe->LoadModelFile();
  pipe->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO)<<"Testing took "<<static_cast<double>(time)/1000.0
    <<" sec."<<endl;

  delete pipe;
  delete options;
}
