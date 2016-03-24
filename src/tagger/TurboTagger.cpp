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
#include "TaggerPipe.h"
//#include "StringUtils.h"

using namespace std;

void TrainTagger();
void TestTagger();

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc, &argv, true);

#ifdef _WIN32
  google::LogToStderr();
#endif
  if (FLAGS_train) {
    LOG(INFO) << "Training tagger..." << endl;
    TrainTagger();
  } else if (FLAGS_test) {
    LOG(INFO) << "Running tagger..." << endl;
    TestTagger();
  }

  // Destroy allocated memory regarding line flags.
  google::ShutDownCommandLineFlags();
  google::ShutdownGoogleLogging();
  return 0;
}

void TrainTagger() {
  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  TaggerOptions *options = new TaggerOptions;
  options->Initialize();

  TaggerPipe *pipe = new TaggerPipe(options);
  pipe->Initialize();
  pipe->Train();
  pipe->SaveModelFile();

  delete pipe;
  delete options;

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Training took " << time << " sec." << endl;
}

void TestTagger() {
  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  TaggerOptions *options = new TaggerOptions;
  options->Initialize();

  TaggerPipe *pipe = new TaggerPipe(options);
  pipe->Initialize();
  pipe->LoadModelFile();
  pipe->Run();

  delete pipe;
  delete options;

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Testing took " << time << " sec." << endl;
}
