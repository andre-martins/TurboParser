#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory>
#include "Instance.h"
#include "SequenceInstance.h"
#include "EntityInstance.h"
#include "DependencyInstance.h"
#include "SemanticInstance.h"
#include "EntitySpan.h"
#include "CoreferenceSentence.h"
#include "CoreferenceDocument.h"
#include "MorphologicalInstance.h"

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
  struct TaggerOptionsOpaque; 
  std::unique_ptr<TaggerOptionsOpaque> options_opaque_;
  struct TaggerPipeOpaque;
  std::unique_ptr<TaggerPipeOpaque> pipe_opaque_;
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
  struct EntityOptionsOpaque;
  std::unique_ptr<EntityOptionsOpaque> options_opaque_;
  struct EntityPipeOpaque;
  std::unique_ptr<EntityPipeOpaque> pipe_opaque_;
};

class TurboParserWorker {
public:
  TurboParserWorker();
  virtual ~TurboParserWorker();

  void LoadParserModel(const std::string &file_model);

  void Parse(const std::string &file_test,
             const std::string &file_prediction);

  void ParseSentence(DependencyInstance *sentence);

private:
  struct DependencyOptionsOpaque;
  std::unique_ptr<DependencyOptionsOpaque> options_opaque_;
  struct DependencyPipeOpaque;
  std::unique_ptr<DependencyPipeOpaque> pipe_opaque_;
};

class TurboSemanticParserWorker {
public:
  TurboSemanticParserWorker();
  virtual ~TurboSemanticParserWorker();

  void LoadSemanticParserModel(const std::string &file_model);

  void ParseSemanticDependencies(const std::string &file_test,
                                 const std::string &file_prediction);

  void ParseSemanticDependenciesFromSentence(SemanticInstance *sentence);

private:
  struct SemanticOptionsOpaque;
  std::unique_ptr<SemanticOptionsOpaque> options_opaque_;
  struct SemanticPipeOpaque;
  std::unique_ptr<SemanticPipeOpaque> pipe_opaque_;
};

class TurboCoreferenceResolverWorker {
public:
  TurboCoreferenceResolverWorker();
  virtual ~TurboCoreferenceResolverWorker();

  void LoadCoreferenceResolverModel(const std::string &file_model);

  void ResolveCoreferences(const std::string &file_test,
                           const std::string &file_prediction);

  void ResolveCoreferencesFromDocument(CoreferenceDocument *document);

private:
  struct CoreferenceOptionsOpaque;
  std::unique_ptr<CoreferenceOptionsOpaque> options_opaque_;
  struct CoreferencePipeOpaque;
  std::unique_ptr<CoreferencePipeOpaque> pipe_opaque_;
};

class TurboMorphologicalTaggerWorker {
public:
  TurboMorphologicalTaggerWorker();
  virtual ~TurboMorphologicalTaggerWorker();

  void LoadMorphologicalTaggerModel(const std::string &file_model);

  void Tag(const std::string &file_test,
           const std::string &file_prediction);

  void TagSentence(MorphologicalInstance *sentence);

private:
  struct MorphologicalOptionsOpaque;
  std::unique_ptr<MorphologicalOptionsOpaque> options_opaque_;
  struct MorphologicalPipeOpaque;
  std::unique_ptr<MorphologicalPipeOpaque> pipe_opaque_;
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
    argv_[1] = strdup("--logtostderr");
  }

  TurboTaggerWorker *CreateTagger() {
    TurboTaggerWorker *tagger = 
      new TurboTaggerWorker();
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
    TurboParserWorker *parser = 
      new TurboParserWorker();
    parsers_.push_back(parser);
    return parser;
  }

  TurboSemanticParserWorker *CreateSemanticParser() {
    TurboSemanticParserWorker *semantic_parser =
      new TurboSemanticParserWorker();
    semantic_parsers_.push_back(semantic_parser);
    return semantic_parser;
  }

  TurboCoreferenceResolverWorker *CreateCoreferenceResolver() {
    TurboCoreferenceResolverWorker *coreference_resolver =
      new TurboCoreferenceResolverWorker();
    coreference_resolvers_.push_back(coreference_resolver);
    return coreference_resolver;
  }

  TurboMorphologicalTaggerWorker *CreateMorphologicalTagger() {
    TurboMorphologicalTaggerWorker *morphological_tagger =
      new TurboMorphologicalTaggerWorker();
    morphological_taggers_.push_back(morphological_tagger);
    return morphological_tagger;
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

  void DeleteAllCoreferenceResolvers() {
    for (int i = 0; i < coreference_resolvers_.size(); ++i) {
      delete coreference_resolvers_[i];
    }
    coreference_resolvers_.clear();
  }

  void DeleteAllMorphologicalTaggers() {
    for (int i = 0; i < morphological_taggers_.size(); ++i) {
      delete morphological_taggers_[i];
    }
    morphological_taggers_.clear();
  }

private:
  int argc_;
  char** argv_;
  std::vector<TurboTaggerWorker*> taggers_;
  std::vector<TurboParserWorker*> parsers_;
  std::vector<TurboSemanticParserWorker*> semantic_parsers_;
  std::vector<TurboEntityRecognizerWorker*> entity_recognizers_;
  std::vector<TurboCoreferenceResolverWorker*> coreference_resolvers_;
  std::vector<TurboMorphologicalTaggerWorker*> morphological_taggers_;
};
} // namespace TurboParserInterface.
