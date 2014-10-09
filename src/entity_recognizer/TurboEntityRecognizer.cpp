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
#include "EntityPipe.h"

using namespace std;

void TrainEntityRecognizer();
void TestEntityRecognizer();

int main(int argc, char** argv) {
  // Initialize Google's logging library.
  google::InitGoogleLogging(argv[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc, &argv, true);

#ifdef _WIN32
  google::LogToStderr();
#endif
  if (FLAGS_train) {
    LOG(INFO) << "Training entity recognizer..." << endl;
    TrainEntityRecognizer();
  } else if (FLAGS_test) {
    LOG(INFO) << "Running entity recognizer..." << endl;
    TestEntityRecognizer();
  }

  return 0;
}

void TrainEntityRecognizer() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  EntityOptions *options = new EntityOptions;
  options->Initialize();

  EntityPipe *pipe = new EntityPipe(options);
  pipe->Initialize();
  pipe->Train();
  pipe->SaveModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end,start);

  LOG(INFO) << "Training took " << static_cast<double>(time)/1000.0
            << " sec." << endl;

  delete pipe;
  delete options;
}

void TestEntityRecognizer() {
  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  EntityOptions *options = new EntityOptions;
  options->Initialize();

  EntityPipe *pipe = new EntityPipe(options);
  pipe->Initialize();
  pipe->LoadModelFile();
  pipe->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end,start);

  LOG(INFO) << "Testing took " << static_cast<double>(time)/1000.0
            << " sec." << endl;

  delete pipe;
  delete options;
}
