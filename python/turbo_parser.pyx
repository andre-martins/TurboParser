from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

import pdb

# Get the classes from the c++ headers.

cdef extern from "../src/sequence/SequenceInstance.h":
    cdef cppclass SequenceInstance:
        SequenceInstance()
        void Initialize(vector[string] forms, vector[string] tags)
        string GetTag(int i)

cdef extern from "../src/morphological_tagger/MorphologicalInstance.h":
    cdef cppclass MorphologicalInstance:
        MorphologicalInstance()
        void Initialize(vector[string] forms, vector[string] lemmas, \
                        vector[string] pos, vector[string] tags)
        string GetTag(int i)

cdef extern from "../src/entity_recognizer/EntityInstance.h":
    cdef cppclass EntityInstance:
        EntityInstance()
        void Initialize(vector[string] forms, vector[string] pos, \
                        vector[string] tags)
        string GetTag(int i)

cdef extern from "../src/parser/DependencyInstance.h":
    cdef cppclass DependencyInstance:
        DependencyInstance()
        void Initialize(vector[string] forms, vector[string] lemmas, \
                        vector[string] cpos, vector[string] pos, \
                        vector[vector[string]] feats, vector[string] deprels, \
                        vector[int] heads)
        int GetHead(int i)
        string GetDependencyRelation(int i)

cdef extern from "../src/semantic_parser/SemanticInstance.h":
    cdef cppclass SemanticInstance:
        SemanticInstance()
        void Initialize(string name, vector[string] forms, vector[string] lemmas, \
                        vector[string] cpos, vector[string] pos, \
                        vector[vector[string]] feats, vector[string] deprels, \
                        vector[int] heads, vector[string] predicate_names, \
                        vector[int] predicate_indices, \
                        vector[vector[string]] argument_roles, \
                        vector[vector[int]] argument_indices)
        int GetNumPredicates()
        string GetPredicateName(int k)
        int GetPredicateIndex(int k)
        int GetNumArgumentsPredicate(int k)
        string GetArgumentRole(int k, int l)
        int GetArgumentIndex(int k, int l)

cdef extern from "../src/entity_recognizer/EntitySpan.h":
    ctypedef NamedSpan EntitySpan
    cdef cppclass NamedSpan:
        NamedSpan(int start, int end, string name)
        int start()
        int end()
        string name()

cdef extern from "../src/coreference_resolver/CoreferenceSentence.h":
    cdef cppclass CoreferenceSentence:
        CoreferenceSentence()
        void Initialize(string name, vector[string] forms, vector[string] lemmas, \
                        vector[string] cpos, vector[string] pos, \
                        vector[vector[string]] feats, vector[string] deprels, \
                        vector[int] heads, vector[string] predicate_names, \
                        vector[int] predicate_indices, \
                        vector[vector[string]] argument_roles, \
                        vector[vector[int]] argument_indices, \
                        vector[string] speakers, \
                        vector[NamedSpan*] entity_spans, \
                        vector[NamedSpan*] constituent_spans, \
                        vector[NamedSpan*] coreference_spans)
        vector[NamedSpan*] GetCoreferenceSpans()

cdef extern from "../src/coreference_resolver/CoreferenceDocument.h":
    cdef cppclass CoreferenceDocument:
        CoreferenceDocument()
        void Initialize(string name, int part_number, \
                        vector[CoreferenceSentence*] sentences)
        int GetNumSentences()
        CoreferenceSentence *GetSentence(int i)

cdef extern from "../libturboparser/TurboParserInterface.h" namespace "TurboParserInterface":
    cdef cppclass TurboTaggerWorker:
        TurboTaggerWorker()
        void LoadTaggerModel(string file_model)
        void Tag(string file_test, string file_prediction)
        void TagSentence(SequenceInstance *sentence)

    cdef cppclass TurboMorphologicalTaggerWorker:
        TurboMorphologicalTaggerWorker()
        void LoadMorphologicalTaggerModel(string file_model)
        void Tag(string file_test, string file_prediction)
        void TagSentence(MorphologicalInstance *sentence)

    cdef cppclass TurboEntityRecognizerWorker:
        TurboEntityRecognizerWorker()
        void LoadEntityRecognizerModel(string file_model)
        void Tag(string file_test, string file_prediction)
        void TagSentence(EntityInstance *sentence)

    cdef cppclass TurboParserWorker:
        TurboParserWorker()
        void LoadParserModel(string file_model)
        void Parse(string file_test, string file_prediction)
        void ParseSentence(DependencyInstance *sentence)

    cdef cppclass TurboSemanticParserWorker:
        TurboSemanticParserWorker()
        void LoadSemanticParserModel(string file_model)
        void ParseSemanticDependencies(string file_test, string file_prediction)
        void ParseSemanticDependenciesFromSentence(SemanticInstance *sentence)

    cdef cppclass TurboCoreferenceResolverWorker:
        TurboCoreferenceResolverWorker()
        void LoadCoreferenceResolverModel(string file_model)
        void ResolveCoreferences(string file_test, string file_prediction)
        void ResolveCoreferencesFromDocument(CoreferenceDocument *document)

    cdef cppclass TurboParserInterface:
        TurboParserInterface()
        TurboTaggerWorker* CreateTagger()
        TurboMorphologicalTaggerWorker* CreateMorphologicalTagger()
        TurboEntityRecognizerWorker* CreateEntityRecognizer()
        TurboParserWorker* CreateParser()
        TurboSemanticParserWorker* CreateSemanticParser()
        TurboCoreferenceResolverWorker* CreateCoreferenceResolver()


# Wrap them into python extension types.

cdef class PTurboParser:
    cdef TurboParserInterface *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboParserInterface()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def create_tagger(self):
        tagger = PTurboTaggerWorker(allocate=False)
        tagger.thisptr = self.thisptr.CreateTagger()
        return tagger

    def create_morphological_tagger(self):
        morphological_tagger = PTurboMorphologicalTaggerWorker(allocate=False)
        morphological_tagger.thisptr = self.thisptr.CreateMorphologicalTagger()
        return morphological_tagger

    def create_entity_recognizer(self):
        entity_recognizer = PTurboEntityRecognizerWorker(allocate=False)
        entity_recognizer.thisptr = self.thisptr.CreateEntityRecognizer()
        return entity_recognizer

    def create_parser(self):
        parser = PTurboParserWorker(allocate=False)
        parser.thisptr = self.thisptr.CreateParser()
        return parser

    def create_semantic_parser(self):
        semantic_parser = PTurboSemanticParserWorker(allocate=False)
        semantic_parser.thisptr = self.thisptr.CreateSemanticParser()
        return semantic_parser

    def create_coreference_resolver(self):
        coreference_resolver = PTurboCoreferenceResolverWorker(allocate=False)
        coreference_resolver.thisptr = self.thisptr.CreateCoreferenceResolver()
        return coreference_resolver

cdef class PSequenceInstance:
    cdef SequenceInstance *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new SequenceInstance()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def initialize(self, vector[string] forms, vector[string] tags):
        self.thisptr.Initialize(forms, tags)

    def get_tag(self, i):
        return self.thisptr.GetTag(i)

cdef class PMorphologicalInstance:
    cdef MorphologicalInstance *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new MorphologicalInstance()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def initialize(self, vector[string] forms, vector[string] lemmas, \
                   vector[string] pos, vector[string] tags):
        self.thisptr.Initialize(forms, lemmas, pos, tags)

    def get_tag(self, i):
        return self.thisptr.GetTag(i)

cdef class PEntityInstance:
    cdef EntityInstance *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new EntityInstance()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def initialize(self, vector[string] forms, vector[string] pos, \
                   vector[string] tags):
        self.thisptr.Initialize(forms, pos, tags)

    def get_tag(self, i):
        return self.thisptr.GetTag(i)

cdef class PDependencyInstance:
    cdef DependencyInstance *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new DependencyInstance()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def initialize(self, vector[string] forms, vector[string] lemmas, \
                   vector[string] cpos, vector[string] pos, \
                   vector[vector[string]] feats, vector[string] deprels, \
                   vector[int] heads):
        self.thisptr.Initialize(forms, lemmas, cpos, pos, feats, deprels, heads)

    def get_head(self, i):
        return self.thisptr.GetHead(i)

    def get_dependency_relation(self, i):
        return self.thisptr.GetDependencyRelation(i)

cdef class PSemanticInstance:
    cdef SemanticInstance *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new SemanticInstance()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def initialize(self, string name, vector[string] forms, \
                   vector[string] lemmas, \
                   vector[string] cpos, vector[string] pos, \
                   vector[vector[string]] feats, vector[string] deprels, \
                   vector[int] heads, vector[string] predicate_names, \
                   vector[int] predicate_indices, \
                   vector[vector[string]] argument_roles, \
                   vector[vector[int]] argument_indices):
        self.thisptr.Initialize(name, forms, lemmas, cpos, pos, feats, \
                                deprels,  heads, predicate_names, \
                                predicate_indices, argument_roles, \
                                argument_indices)

    def get_num_predicates(self):
        return self.thisptr.GetNumPredicates()

    def get_predicate_name(self, k):
        return self.thisptr.GetPredicateName(k)

    def get_predicate_index(self, k):
        return self.thisptr.GetPredicateIndex(k)

    def get_num_arguments_predicate(self, k):
        return self.thisptr.GetNumArgumentsPredicate(k)

    def get_argument_role(self, k, l):
        return self.thisptr.GetArgumentRole(k, l)

    def get_argument_index(self, k, l):
        return self.thisptr.GetArgumentIndex(k, l)

cdef class PNamedSpan:
    cdef NamedSpan *thisptr
    cdef bool allocate
    def __cinit__(self, start=-1, end=-1, name='', allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new NamedSpan(start, end, name)

    def __dealloc__(self):
        if self.allocate:
            del self.thisptr

    def start(self):
        return self.thisptr.start()

    def end(self):
        return self.thisptr.end()

    def name(self):
        return self.thisptr.name()

cdef class PEntitySpan(PNamedSpan):
    def __cinit__(self, start=-1, end=-1, name='', allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new EntitySpan(start, end, name)

cdef class PCoreferenceSentence:
    cdef CoreferenceSentence *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new CoreferenceSentence()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def initialize(self, string name, vector[string] forms, \
                   vector[string] lemmas, \
                   vector[string] cpos, vector[string] pos, \
                   vector[vector[string]] feats, vector[string] deprels, \
                   vector[int] heads, vector[string] predicate_names, \
                   vector[int] predicate_indices, \
                   vector[vector[string]] argument_roles, \
                   vector[vector[int]] argument_indices, \
                   vector[string] speakers, \
                   p_entity_spans, \
                   p_constituent_spans, \
                   p_coreference_spans):
        cdef vector[EntitySpan*] entity_spans
        for p_span in p_entity_spans:
            entity_spans.push_back((<PEntitySpan>p_span).thisptr)
        cdef vector[NamedSpan*] constituent_spans
        for p_span in p_constituent_spans:
            constituent_spans.push_back((<PNamedSpan>p_span).thisptr)
        cdef vector[NamedSpan*] coreference_spans
        for p_span in p_coreference_spans:
            coreference_spans.push_back((<PNamedSpan>p_span).thisptr)
        self.thisptr.Initialize(name, forms, lemmas, cpos, pos, feats, \
                                deprels,  heads, predicate_names, \
                                predicate_indices, argument_roles, \
                                argument_indices, speakers, entity_spans, \
                                constituent_spans, coreference_spans)

    def get_coreference_spans(self):
        cdef vector[NamedSpan*] coreference_spans = \
            self.thisptr.GetCoreferenceSpans()
        p_coreference_spans = []
        for span in coreference_spans:
            p_span = PNamedSpan(allocate=False)
            p_span.thisptr = span
            p_coreference_spans.append(p_span)
        return p_coreference_spans

cdef class PCoreferenceDocument:
    cdef CoreferenceDocument *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=True):
        self.allocate = allocate
        if allocate:
            self.thisptr = new CoreferenceDocument()

    def __dealloc__(self):
        if self.allocate:
            del self.thisptr

    def initialize(self, string name, int part_number, \
                   p_sentences):
        cdef vector[CoreferenceSentence*] sentences
        for p_sentence in p_sentences:
            sentences.push_back((<PCoreferenceSentence>p_sentence).thisptr)
        self.thisptr.Initialize(name, part_number, sentences)

    def get_num_sentences(self):
        self.thisptr.GetNumSentences()

    def get_sentence(self, int i):
        cdef CoreferenceSentence *sentence = self.thisptr.GetSentence(i)
        p_sentence = PCoreferenceSentence(allocate=False)
        p_sentence.thisptr = sentence
        return p_sentence

cdef class PTurboTaggerWorker:
    cdef TurboTaggerWorker *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=False):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboTaggerWorker()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def load_tagger_model(self, file_model):
        self.thisptr.LoadTaggerModel(file_model)

    def tag(self, file_test, file_prediction):
        self.thisptr.Tag(file_test, file_prediction)

    def tag_sentence(self, sequence_instance):
        self.thisptr.TagSentence((<PSequenceInstance>sequence_instance).thisptr)

cdef class PTurboMorphologicalTaggerWorker:
    cdef TurboMorphologicalTaggerWorker *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=False):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboMorphologicalTaggerWorker()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def load_morphological_tagger_model(self, file_model):
        self.thisptr.LoadMorphologicalTaggerModel(file_model)

    def tag(self, file_test, file_prediction):
        self.thisptr.Tag(file_test, file_prediction)

    def tag_sentence(self, sequence_instance):
        self.thisptr.TagSentence( \
            (<PMorphologicalInstance>sequence_instance).thisptr)

cdef class PTurboEntityRecognizerWorker:
    cdef TurboEntityRecognizerWorker *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=False):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboEntityRecognizerWorker()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def load_entity_recognizer_model(self, file_model):
        self.thisptr.LoadEntityRecognizerModel(file_model)

    def tag(self, file_test, file_prediction):
        self.thisptr.Tag(file_test, file_prediction)

    def tag_sentence(self, entity_instance):
        self.thisptr.TagSentence((<PEntityInstance>entity_instance).thisptr)

cdef class PTurboParserWorker:
    cdef TurboParserWorker *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=False):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboParserWorker()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def load_parser_model(self, file_model):
        self.thisptr.LoadParserModel(file_model)

    def parse(self, file_test, file_prediction):
        self.thisptr.Parse(file_test, file_prediction)

    def parse_sentence(self, dependency_instance):
        self.thisptr.ParseSentence( \
            (<PDependencyInstance>dependency_instance).thisptr)

cdef class PTurboSemanticParserWorker:
    cdef TurboSemanticParserWorker *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=False):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboSemanticParserWorker()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def load_semantic_parser_model(self, file_model):
        self.thisptr.LoadSemanticParserModel(file_model)

    def parse_semantic_dependencies(self, file_test, file_prediction):
        self.thisptr.ParseSemanticDependencies(file_test, file_prediction)

    def parse_semantic_dependencies_from_sentence(self, semantic_instance):
        self.thisptr.ParseSemanticDependenciesFromSentence( \
            (<PSemanticInstance>semantic_instance).thisptr)

cdef class PTurboCoreferenceResolverWorker:
    cdef TurboCoreferenceResolverWorker *thisptr
    cdef bool allocate
    def __cinit__(self, allocate=False):
        self.allocate = allocate
        if allocate:
            self.thisptr = new TurboCoreferenceResolverWorker()

    def __dealloc__(self):
        if self.allocate:
          del self.thisptr

    def load_coreference_resolver_model(self, file_model):
        self.thisptr.LoadCoreferenceResolverModel(file_model)

    def resolve_coreferences(self, file_test, file_prediction):
        self.thisptr.ResolveCoreferences(file_test, file_prediction)

    def resolve_coreferences_from_document(self, coreference_document):
        self.thisptr.ResolveCoreferencesFromDocument( \
            (<PCoreferenceDocument>coreference_document).thisptr)
