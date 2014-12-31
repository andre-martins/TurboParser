from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp cimport bool

import pdb

# Get the classes from the c++ headers.

cdef extern from "../libturboparser/TurboParserInterface.h" namespace "TurboParserInterface":
    cdef cppclass TurboTaggerWorker:
        TurboTaggerWorker()
        void LoadTaggerModel(string file_model)
        void Tag(string file_test, string file_prediction)

    cdef cppclass TurboParserWorker:
        TurboParserWorker()
        void LoadParserModel(string file_model)
        void Parse(string file_test, string file_prediction)

    cdef cppclass TurboSemanticParserWorker:
        TurboSemanticParserWorker()
        void LoadSemanticParserModel(string file_model)
        void ParseSemanticDependencies(string file_test, string file_prediction)

    cdef cppclass TurboParserInterface:
        TurboParserInterface()
        TurboTaggerWorker* CreateTagger()
        TurboParserWorker* CreateParser()
        TurboSemanticParserWorker* CreateSemanticParser()


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

    def create_parser(self):
        parser = PTurboParserWorker(allocate=False)
        parser.thisptr = self.thisptr.CreateParser()
        return parser

    def create_semantic_parser(self):
        semantic_parser = PTurboSemanticParserWorker(allocate=False)
        semantic_parser.thisptr = self.thisptr.CreateSemanticParser()
        return semantic_parser

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
