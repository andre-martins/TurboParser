UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
ENTITY_RECOGNIZER = ../entity_recognizer
PARSER = ../parser
SEMANTIC_PARSER = ../semantic_parser

TurboCoreferenceResolverprgdir = ../..
TurboCoreferenceResolverprg_PROGRAMS = TurboCoreferenceResolver
TurboCoreferenceResolver_SOURCES = TurboCoreferenceResolver.cpp \
CoreferencePipe.cpp \
CoreferenceOptions.cpp \
CoreferenceReader.cpp \
CoreferenceWriter.cpp \
CoreferenceDocument.cpp \
CoreferenceSentence.cpp \
CoreferenceDictionary.cpp \
CoreferenceDocumentNumeric.cpp \
CoreferenceSentenceNumeric.cpp \
CoreferencePart.cpp \
CoreferenceFeatures.cpp \
CoreferenceDecoder.cpp \
Mention.cpp \
CoreferencePipe.h \
CoreferenceOptions.h \
CoreferenceReader.h \
CoreferenceWriter.h \
CoreferenceDocument.h \
CoreferenceSentence.h \
CoreferenceDictionary.h \
CoreferenceDocumentNumeric.h \
CoreferenceSentenceNumeric.h \
CoreferencePart.h \
CoreferenceFeatures.h \
CoreferenceFeatureTemplates.h \
CoreferenceDecoder.h \
Mention.h \
CoreferencePronoun.h \
$(SEMANTIC_PARSER)/SemanticInstance.cpp \
$(SEMANTIC_PARSER)/SemanticInstance.h \
$(SEMANTIC_PARSER)/SemanticReader.cpp \
$(SEMANTIC_PARSER)/SemanticReader.h \
$(SEMANTIC_PARSER)/SemanticDictionary.cpp \
$(SEMANTIC_PARSER)/SemanticDictionary.h \
$(SEMANTIC_PARSER)/SemanticInstanceNumeric.cpp \
$(SEMANTIC_PARSER)/SemanticInstanceNumeric.h \
$(PARSER)/DependencyInstance.cpp \
$(PARSER)/DependencyInstance.h \
$(PARSER)/DependencyReader.cpp \
$(PARSER)/DependencyReader.h \
$(PARSER)/DependencyDictionary.cpp \
$(PARSER)/DependencyDictionary.h \
$(PARSER)/DependencyInstanceNumeric.cpp \
$(PARSER)/DependencyInstanceNumeric.h \
$(SEQUENCE)/TokenDictionary.cpp $(SEQUENCE)/TokenDictionary.h \
$(CLASSIFIER)/Alphabet.cpp $(CLASSIFIER)/Dictionary.cpp \
$(CLASSIFIER)/Features.h $(CLASSIFIER)/Options.h $(CLASSIFIER)/Part.h \
$(CLASSIFIER)/Reader.cpp $(CLASSIFIER)/SparseParameterVector.h \
$(CLASSIFIER)/Alphabet.h $(CLASSIFIER)/Dictionary.h $(CLASSIFIER)/Instance.h \
$(CLASSIFIER)/Parameters.cpp $(CLASSIFIER)/Pipe.cpp $(CLASSIFIER)/Reader.h \
$(CLASSIFIER)/Writer.cpp $(CLASSIFIER)/Decoder.h \
$(CLASSIFIER)/FeatureEncoder.h $(CLASSIFIER)/Options.cpp \
$(CLASSIFIER)/Parameters.h $(CLASSIFIER)/Pipe.h \
$(CLASSIFIER)/SparseLabeledParameterVector.h $(CLASSIFIER)/Writer.h \
$(UTIL)/AlgUtils.cpp $(UTIL)/logval.h $(UTIL)/SerializationUtils.h \
$(UTIL)/StringUtils.h $(UTIL)/TimeUtils.h $(UTIL)/AlgUtils.h \
$(UTIL)/SerializationUtils.cpp $(UTIL)/StringUtils.cpp $(UTIL)/TimeUtils.cpp \
$(UTIL)/Utils.h

AM_CPPFLAGS = -I$(UTIL) -I$(CLASSIFIER) -I$(SEQUENCE) -I$(ENTITY_RECOGNIZER) -I$(PARSER) -I$(SEMANTIC_PARSER) $(CPPFLAGS)
LDADD = $(LFLAGS)

