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
#include "TurboParserInterface.h"

namespace TurboParserInterface {

TurboTaggerWorker::TurboTaggerWorker() {
  tagger_options_ = new SequenceOptions;
  tagger_options_->Initialize();

  tagger_pipe_ = new SequencePipe(tagger_options_);
  tagger_pipe_->Initialize();
}

TurboTaggerWorker::~TurboTaggerWorker() {
  LOG(INFO) << "Deleting tagger pipe.";
  delete tagger_pipe_;
  LOG(INFO) << "Deleting tagger options.";
  delete tagger_options_;
}

void TurboTaggerWorker::LoadTaggerModel(const std::string &file_model) {
  tagger_options_->SetModelFilePath(file_model);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  tagger_pipe_->LoadModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end,start);

  LOG(INFO) << "Took " << static_cast<double>(time)/1000.0
            << " sec." << endl;
}

void TurboTaggerWorker::Tag(const std::string &file_test,
                            const std::string &file_prediction) {
  tagger_options_->SetTestFilePath(file_test);
  tagger_options_->SetOutputFilePath(file_prediction);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  tagger_pipe_->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end,start);

  LOG(INFO) << "Took " << static_cast<double>(time)/1000.0
            << " sec." << endl;
}

TurboParserWorker::TurboParserWorker() {
  parser_options_ = new DependencyOptions;
  parser_options_->Initialize();

  parser_pipe_ = new DependencyPipe(parser_options_);
  parser_pipe_->Initialize();
}

TurboParserWorker::~TurboParserWorker() {
  LOG(INFO) << "Deleting parser pipe.";
  delete parser_pipe_;
  LOG(INFO) << "Deleting parser options.";
  delete parser_options_;
}

void TurboParserWorker::LoadParserModel(const std::string &file_model) {
  parser_options_->SetModelFilePath(file_model);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  parser_pipe_->LoadModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end,start);

  LOG(INFO) << "Took " << static_cast<double>(time)/1000.0
            << " sec." << endl;
}

void TurboParserWorker::Parse(const std::string &file_test,
                              const std::string &file_prediction) {
  parser_options_->SetTestFilePath(file_test);
  parser_options_->SetOutputFilePath(file_prediction);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  parser_pipe_->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end,start);

  LOG(INFO) << "Took " << static_cast<double>(time)/1000.0
            << " sec." << endl;
}

TurboParserInterface::TurboParserInterface() {
  argc_ = 0;
  argv_ = NULL;
  BuildArgumentList();

  // Initialize Google's logging library.
  google::InitGoogleLogging(argv_[0]);

  // Parse command line flags.
  google::ParseCommandLineFlags(&argc_, &argv_, false);

#ifdef _WIN32
  google::LogToStderr();
#endif
}

TurboParserInterface::~TurboParserInterface() {
  LOG(INFO) << "Deleting tagger workers.";
  DeleteAllTaggers();

  LOG(INFO) << "Deleting parser workers.";
  DeleteAllParsers();

  LOG(INFO) << "Clearing argument list.";
  ClearArgumentList();

  LOG(INFO) << "Done.";
}

} // namespace TurboParserInterface.
