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
  tagger_options_ = new TaggerOptions;
  tagger_options_->Initialize();

  tagger_pipe_ = new TaggerPipe(tagger_options_);
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
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
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
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboTaggerWorker::TagSentence(SequenceInstance *sentence) {
  tagger_pipe_->ClassifyInstance(sentence);
}

TurboEntityRecognizerWorker::TurboEntityRecognizerWorker() {
  entity_options_ = new EntityOptions;
  entity_options_->Initialize();

  entity_pipe_ = new EntityPipe(entity_options_);
  entity_pipe_->Initialize();
}

TurboEntityRecognizerWorker::~TurboEntityRecognizerWorker() {
  LOG(INFO) << "Deleting entity recognizer pipe.";
  delete entity_pipe_;
  LOG(INFO) << "Deleting entity recognizer options.";
  delete entity_options_;
}

void TurboEntityRecognizerWorker::LoadEntityRecognizerModel(
  const std::string &file_model) {
  entity_options_->SetModelFilePath(file_model);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  entity_pipe_->LoadModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboEntityRecognizerWorker::Tag(const std::string &file_test,
                                      const std::string &file_prediction) {
  entity_options_->SetTestFilePath(file_test);
  entity_options_->SetOutputFilePath(file_prediction);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  entity_pipe_->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboEntityRecognizerWorker::TagSentence(EntityInstance *sentence) {
  entity_pipe_->ClassifyInstance(sentence);
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
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
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
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

TurboSemanticParserWorker::TurboSemanticParserWorker() {
  semantic_options_ = new SemanticOptions;
  semantic_options_->Initialize();

  semantic_pipe_ = new SemanticPipe(semantic_options_);
  semantic_pipe_->Initialize();
}

TurboSemanticParserWorker::~TurboSemanticParserWorker() {
  LOG(INFO) << "Deleting semantic pipe.";
  delete semantic_pipe_;
  LOG(INFO) << "Deleting semantic options.";
  delete semantic_options_;
}

void TurboSemanticParserWorker::LoadSemanticParserModel(
  const std::string &file_model) {
  semantic_options_->SetModelFilePath(file_model);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  LOG(INFO) << "Loading model file " << file_model;

  semantic_pipe_->LoadModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboSemanticParserWorker::ParseSemanticDependencies(
  const std::string &file_test,
  const std::string &file_prediction) {
  semantic_options_->SetTestFilePath(file_test);
  semantic_options_->SetOutputFilePath(file_prediction);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  semantic_pipe_->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

TurboCoreferenceResolverWorker::TurboCoreferenceResolverWorker() {
  coreference_options_ = new CoreferenceOptions;
  coreference_options_->Initialize();

  coreference_pipe_ = new CoreferencePipe(coreference_options_);
  coreference_pipe_->Initialize();
}

TurboCoreferenceResolverWorker::~TurboCoreferenceResolverWorker() {
  LOG(INFO) << "Deleting coreference pipe.";
  delete coreference_pipe_;
  LOG(INFO) << "Deleting coreference options.";
  delete coreference_options_;
}

void TurboCoreferenceResolverWorker::LoadCoreferenceResolverModel(
  const std::string &file_model) {
  coreference_options_->SetModelFilePath(file_model);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  LOG(INFO) << "Loading model file " << file_model;

  coreference_pipe_->LoadModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboCoreferenceResolverWorker::ResolveCoreferences(
  const std::string &file_test,
  const std::string &file_prediction) {
  coreference_options_->SetTestFilePath(file_test);
  coreference_options_->SetOutputFilePath(file_prediction);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  coreference_pipe_->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

TurboMorphologicalTaggerWorker::TurboMorphologicalTaggerWorker() {
  morphological_tagger_options_ = new MorphologicalOptions;
  morphological_tagger_options_->Initialize();

  morphological_tagger_pipe_ =
    new MorphologicalPipe(morphological_tagger_options_);
  morphological_tagger_pipe_->Initialize();
}

TurboMorphologicalTaggerWorker::~TurboMorphologicalTaggerWorker() {
  LOG(INFO) << "Deleting tagger pipe.";
  delete morphological_tagger_pipe_;
  LOG(INFO) << "Deleting tagger options.";
  delete morphological_tagger_options_;
}

void TurboMorphologicalTaggerWorker::LoadMorphologicalTaggerModel(const std::string
                                                                  &file_model) {
  morphological_tagger_options_->SetModelFilePath(file_model);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  morphological_tagger_pipe_->LoadModelFile();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboMorphologicalTaggerWorker::Tag(const std::string &file_test,
                                         const std::string &file_prediction) {
  morphological_tagger_options_->SetTestFilePath(file_test);
  morphological_tagger_options_->SetOutputFilePath(file_prediction);

  int time;
  timeval start, end;
  gettimeofday(&start, NULL);

  morphological_tagger_pipe_->Run();

  gettimeofday(&end, NULL);
  time = diff_ms(end, start);

  LOG(INFO) << "Took " << static_cast<double>(time) / 1000.0
    << " sec." << endl;
}

void TurboMorphologicalTaggerWorker::TagSentence(MorphologicalInstance *sentence) {
  morphological_tagger_pipe_->ClassifyInstance(sentence);
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

  LOG(INFO) << "Deleting entity recognizer workers.";
  DeleteAllEntityRecognizers();

  LOG(INFO) << "Deleting parser workers.";
  DeleteAllParsers();

  LOG(INFO) << "Deleting semantic parser workers.";
  DeleteAllSemanticParsers();

  LOG(INFO) << "Deleting coreference resolver workers.";
  DeleteAllCoreferenceResolvers();

  LOG(INFO) << "Deleting morphlogical tagger workers.";
  DeleteAllMorphologicalTaggers();

  LOG(INFO) << "Clearing argument list.";
  ClearArgumentList();

  LOG(INFO) << "Done.";
}
} // namespace TurboParserInterface.
