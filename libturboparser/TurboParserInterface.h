#include <stdio.h>
#include <stdlib.h>
#include "TaggerPipe.h"
#include "EntityPipe.h"
#include "DependencyPipe.h"
#include "SemanticPipe.h"

namespace TurboParserInterface {

class TurboTaggerWorker {
 public:
  TurboTaggerWorker();
  virtual ~TurboTaggerWorker();

  void LoadTaggerModel(const std::string &file_model);

  void Tag(const std::string &file_test,
           const std::string &file_prediction);

  void TagSentence(SequenceInstance *sentence);

 private:
  TaggerOptions *tagger_options_;
  TaggerPipe *tagger_pipe_;
};

class TurboEntityRecognizerWorker {
 public:
  TurboEntityRecognizerWorker();
  virtual ~TurboEntityRecognizerWorker();

  void LoadEntityRecognizerModel(const std::string &file_model);

  void Tag(const std::string &file_test,
           const std::string &file_prediction);

  void TagSentence(EntityInstance *sentence);

 private:
  EntityOptions *entity_options_;
  EntityPipe *entity_pipe_;
};

class TurboParserWorker {
 public:
  TurboParserWorker();
  virtual ~TurboParserWorker();

  void LoadParserModel(const std::string &file_model);

  void Parse(const std::string &file_test,
             const std::string &file_prediction);

 private:
  DependencyOptions *parser_options_;
  DependencyPipe *parser_pipe_;
};

class TurboSemanticParserWorker {
 public:
  TurboSemanticParserWorker();
  virtual ~TurboSemanticParserWorker();

  void LoadSemanticParserModel(const std::string &file_model);

  void ParseSemanticDependencies(const std::string &file_test,
                                 const std::string &file_prediction);

 private:
  SemanticOptions *semantic_options_;
  SemanticPipe *semantic_pipe_;
};

class TurboParserInterface {
 public:
  TurboParserInterface();
  virtual ~TurboParserInterface();

  void ClearArgumentList() {
    for (int i = 0; i < argc_; ++i) {
      if (argv_[i]) free(argv_[i]);
    }
    delete[] argv_;
    argc_ = 0;
  }

  void BuildArgumentList() {
    argc_ = 2;
    argv_ = new char*[argc_];
    argv_[0] = strdup("TurboParser");
    //argv_[1] = strdup("--minloglevel=10");
	argv_[1] = strdup("--logtostderr");
  }

  TurboTaggerWorker *CreateTagger() {
    TurboTaggerWorker *tagger = new TurboTaggerWorker();
    taggers_.push_back(tagger);
    return tagger;
  }

  TurboEntityRecognizerWorker *CreateEntityRecognizer() {
    TurboEntityRecognizerWorker *entity_recognizer =
      new TurboEntityRecognizerWorker();
    entity_recognizers_.push_back(entity_recognizer);
    return entity_recognizer;
  }

  TurboParserWorker *CreateParser() {
    TurboParserWorker *parser = new TurboParserWorker();
    parsers_.push_back(parser);
    return parser;
  }

  TurboSemanticParserWorker *CreateSemanticParser() {
    TurboSemanticParserWorker *semantic_parser = new TurboSemanticParserWorker();
    semantic_parsers_.push_back(semantic_parser);
    return semantic_parser;
  }

  void DeleteAllTaggers() {
    for (int i = 0; i < taggers_.size(); ++i) {
      delete taggers_[i];
    }
    taggers_.clear();
  }

  void DeleteAllEntityRecognizers() {
    for (int i = 0; i < entity_recognizers_.size(); ++i) {
      delete entity_recognizers_[i];
    }
    entity_recognizers_.clear();
  }

  void DeleteAllParsers() {
    for (int i = 0; i < parsers_.size(); ++i) {
      delete parsers_[i];
    }
    parsers_.clear();
  }

  void DeleteAllSemanticParsers() {
    for (int i = 0; i < semantic_parsers_.size(); ++i) {
      delete semantic_parsers_[i];
    }
    semantic_parsers_.clear();
  }

 private:
  int argc_;
  char** argv_;
  std::vector<TurboTaggerWorker*> taggers_;
  std::vector<TurboParserWorker*> parsers_;
  std::vector<TurboSemanticParserWorker*> semantic_parsers_;
  std::vector<TurboEntityRecognizerWorker*> entity_recognizers_;
};

} // namespace TurboParserInterface.
