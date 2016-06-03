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
#include "TaggerPipe.h"
#include "EntityPipe.h"
#include "DependencyPipe.h"
#include "SemanticPipe.h"
#include "CoreferencePipe.h"
#include "MorphologicalPipe.h"


namespace TurboParserInterface {
struct TurboTaggerWorker::TaggerOptionsOpaque {
  TaggerOptions options_;
};
struct TurboTaggerWorker::TaggerPipeOpaque {
  TaggerPipe pipe_;
};

struct TurboEntityRecognizerWorker::EntityOptionsOpaque {
  EntityOptions options_;
};
struct TurboEntityRecognizerWorker::EntityPipeOpaque {
  EntityPipe pipe_;
};

struct TurboParserWorker::DependencyOptionsOpaque {
  DependencyOptions options_;
};
struct TurboParserWorker::DependencyPipeOpaque {
  DependencyPipe pipe_;
};

struct TurboSemanticParserWorker::SemanticOptionsOpaque {
  SemanticOptions options_;
};
struct TurboSemanticParserWorker::SemanticPipeOpaque {
  SemanticPipe pipe_;
};

struct TurboCoreferenceResolverWorker::CoreferenceOptionsOpaque {
  CoreferenceOptions options_;
};
struct TurboCoreferenceResolverWorker::CoreferencePipeOpaque {
  CoreferencePipe pipe_;
};

struct TurboMorphologicalTaggerWorker::MorphologicalOptionsOpaque {
  MorphologicalOptions options_;
};
struct TurboMorphologicalTaggerWorker::MorphologicalPipeOpaque {
  MorphologicalPipe pipe_;
};

TurboTaggerWorker::TurboTaggerWorker() {
  options_opaque_ = std::unique_ptr<TaggerOptionsOpaque>
    (new TaggerOptionsOpaque());
  options_opaque_->options_.Initialize();

  pipe_opaque_ = 
    std::unique_ptr<TaggerPipeOpaque>
    (new TaggerPipeOpaque({ &(options_opaque_->options_) }));
  pipe_opaque_->pipe_.Initialize();
}

TurboTaggerWorker::~TurboTaggerWorker() {}

void TurboTaggerWorker::LoadTaggerModel(const std::string &file_model) {
  options_opaque_->options_.SetModelFilePath(file_model);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.LoadModelFile();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboTaggerWorker::Tag(const std::string &file_test,
                            const std::string &file_prediction) {
  options_opaque_->options_.SetTestFilePath(file_test);
  options_opaque_->options_.SetOutputFilePath(file_prediction);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.Run();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboTaggerWorker::TagSentence(SequenceInstance *sentence) {
  pipe_opaque_->pipe_.ClassifyInstance(sentence);
}

TurboEntityRecognizerWorker::TurboEntityRecognizerWorker() {
  options_opaque_ = std::unique_ptr<EntityOptionsOpaque>
    (new EntityOptionsOpaque());
  options_opaque_->options_.Initialize();

  pipe_opaque_ = std::unique_ptr<EntityPipeOpaque>
    (new EntityPipeOpaque({ &(options_opaque_->options_) }));
  pipe_opaque_->pipe_.Initialize();
}

TurboEntityRecognizerWorker::~TurboEntityRecognizerWorker() {}

void TurboEntityRecognizerWorker::LoadEntityRecognizerModel(
  const std::string &file_model) {
  options_opaque_->options_.SetModelFilePath(file_model);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.LoadModelFile();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboEntityRecognizerWorker::Tag(const std::string &file_test,
                                      const std::string &file_prediction) {
  options_opaque_->options_.SetTestFilePath(file_test);
  options_opaque_->options_.SetOutputFilePath(file_prediction);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.Run();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboEntityRecognizerWorker::TagSentence(EntityInstance *sentence) {
  pipe_opaque_->pipe_.ClassifyInstance(sentence);
}

TurboParserWorker::TurboParserWorker() {
  options_opaque_ = std::unique_ptr<DependencyOptionsOpaque>
    (new DependencyOptionsOpaque());
  options_opaque_->options_.Initialize();

  pipe_opaque_ = std::unique_ptr<DependencyPipeOpaque>
    (new DependencyPipeOpaque({ &(options_opaque_->options_) }));
  pipe_opaque_->pipe_.Initialize();
}

TurboParserWorker::~TurboParserWorker() {}

void TurboParserWorker::LoadParserModel(const std::string &file_model) {
  options_opaque_->options_.SetModelFilePath(file_model);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.LoadModelFile();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboParserWorker::Parse(const std::string &file_test,
                              const std::string &file_prediction) {
  options_opaque_->options_.SetTestFilePath(file_test);
  options_opaque_->options_.SetOutputFilePath(file_prediction);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.Run();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboParserWorker::ParseSentence(DependencyInstance *sentence) {
  pipe_opaque_->pipe_.ClassifyInstance(sentence);
}

TurboSemanticParserWorker::TurboSemanticParserWorker() {
  options_opaque_ = std::unique_ptr<SemanticOptionsOpaque>
    (new SemanticOptionsOpaque());
  options_opaque_->options_.Initialize();

  pipe_opaque_ = std::unique_ptr<SemanticPipeOpaque>
    (new SemanticPipeOpaque({ &(options_opaque_->options_) }));
  pipe_opaque_->pipe_.Initialize();
}

TurboSemanticParserWorker::~TurboSemanticParserWorker() {}

void TurboSemanticParserWorker::LoadSemanticParserModel(
  const std::string &file_model) {
  options_opaque_->options_.SetModelFilePath(file_model);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  LOG(INFO) << "Loading model file " << file_model;

  pipe_opaque_->pipe_.LoadModelFile();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboSemanticParserWorker::ParseSemanticDependencies(
  const std::string &file_test,
  const std::string &file_prediction) {
  options_opaque_->options_.SetTestFilePath(file_test);
  options_opaque_->options_.SetOutputFilePath(file_prediction);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.Run();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboSemanticParserWorker::ParseSemanticDependenciesFromSentence(
  SemanticInstance *sentence) {
  pipe_opaque_->pipe_.ClassifyInstance(sentence);
}

TurboCoreferenceResolverWorker::TurboCoreferenceResolverWorker() {
  options_opaque_ = std::unique_ptr<CoreferenceOptionsOpaque>
    (new CoreferenceOptionsOpaque());
  options_opaque_->options_.Initialize();

  pipe_opaque_ = std::unique_ptr<CoreferencePipeOpaque>
    (new CoreferencePipeOpaque({ &(options_opaque_->options_) }));
  pipe_opaque_->pipe_.Initialize();
}

TurboCoreferenceResolverWorker::~TurboCoreferenceResolverWorker() {}

void TurboCoreferenceResolverWorker::LoadCoreferenceResolverModel(
  const std::string &file_model) {
  options_opaque_->options_.SetModelFilePath(file_model);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  LOG(INFO) << "Loading model file " << file_model;

  pipe_opaque_->pipe_.LoadModelFile();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboCoreferenceResolverWorker::ResolveCoreferences(
  const std::string &file_test,
  const std::string &file_prediction) {
  options_opaque_->options_.SetTestFilePath(file_test);
  options_opaque_->options_.SetOutputFilePath(file_prediction);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.Run();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboCoreferenceResolverWorker::ResolveCoreferencesFromDocument(
  CoreferenceDocument *document) {
  pipe_opaque_->pipe_.ClassifyInstance(document);
}

TurboMorphologicalTaggerWorker::TurboMorphologicalTaggerWorker() {
  options_opaque_ = std::unique_ptr<MorphologicalOptionsOpaque>
    (new MorphologicalOptionsOpaque());
  options_opaque_->options_.Initialize();

  pipe_opaque_ = std::unique_ptr<MorphologicalPipeOpaque>
    (new MorphologicalPipeOpaque({ &(options_opaque_->options_) }));
  pipe_opaque_->pipe_.Initialize();
}

TurboMorphologicalTaggerWorker::~TurboMorphologicalTaggerWorker() {}

void TurboMorphologicalTaggerWorker::LoadMorphologicalTaggerModel(
  const std::string
  &file_model) {
  options_opaque_->options_.SetModelFilePath(file_model);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.LoadModelFile();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboMorphologicalTaggerWorker::Tag(const std::string &file_test,
                                         const std::string &file_prediction) {
  options_opaque_->options_.SetTestFilePath(file_test);
  options_opaque_->options_.SetOutputFilePath(file_prediction);

  double time;
  chronowrap::Chronometer chrono;
  chrono.GetTime();

  pipe_opaque_->pipe_.Run();

  chrono.StopTime();
  time = chrono.GetElapsedTime();

  LOG(INFO) << "Took " << time << " sec." << endl;
}

void TurboMorphologicalTaggerWorker::TagSentence(
  MorphologicalInstance *sentence) {
  pipe_opaque_->pipe_.ClassifyInstance(sentence);
}

TurboParserInterface::TurboParserInterface() {
  argc_ = 0;
  argv_ = NULL;
  BuildArgumentList();

  InitGlog(argv_[0]);

  // Initialize Google's logging library.
  //google::InitGoogleLogging(argv_[0]);

  // Parse command line flags.
  //google::ParseCommandLineFlags(&argc_, &argv_, true);

//#ifdef _WIN32
//  google::LogToStderr();
//#endif
}

TurboParserInterface::~TurboParserInterface() {
  LOG(INFO) << "Deleting tagger workers.";
  DeleteAllTaggers();

  LOG(INFO) << "Deleting morphological tagger workers.";
  DeleteAllMorphologicalTaggers();

  LOG(INFO) << "Deleting entity recognizer workers.";
  DeleteAllEntityRecognizers();

  LOG(INFO) << "Deleting parser workers.";
  DeleteAllParsers();

  LOG(INFO) << "Deleting semantic parser workers.";
  DeleteAllSemanticParsers();

  LOG(INFO) << "Deleting coreference resolver workers.";
  DeleteAllCoreferenceResolvers();

  LOG(INFO) << "Clearing argument list.";
  ClearArgumentList();

  LOG(INFO) << "Done.";
}
} // namespace TurboParserInterface.
