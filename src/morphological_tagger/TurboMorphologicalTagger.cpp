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
#include "MorphologicalPipe.h"

using namespace std;

void TrainMorphologicalTagger();
void TestMorphologicalTagger();

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc, &argv, true);

#ifdef _WIN32
  google::LogToStderr();
#endif
  if (FLAGS_train) {
    LOG(INFO) << "Training Morphological tagger..." << endl;
    TrainMorphologicalTagger();
  } else if (FLAGS_test) {
    LOG(INFO) << "Running Morphological tagger..." << endl;
    TestMorphologicalTagger();
  }

  // Destroy allocated memory regarding line flags.
  google::ShutDownCommandLineFlags();
  google::ShutdownGoogleLogging();
  return 0;
}

void TrainMorphologicalTagger() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  MorphologicalOptions *options = new MorphologicalOptions;
  options->Initialize();

  MorphologicalPipe *pipe = new MorphologicalPipe(options);
  pipe->Initialize();
  pipe->Train();
  pipe->SaveModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Training took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;

  delete pipe;
  delete options;
}

void TestMorphologicalTagger() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  MorphologicalOptions *options = new MorphologicalOptions;
  options->Initialize();

  MorphologicalPipe *pipe = new MorphologicalPipe(options);
  pipe->Initialize();
  pipe->LoadModelFile();
  pipe->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Testing took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;

  delete pipe;
  delete options;
}
