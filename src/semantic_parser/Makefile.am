UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
PARSER = ../parser

TurboSemanticParserprgdir = ../..
TurboSemanticParserprg_PROGRAMS = TurboSemanticParser
TurboSemanticParser_SOURCES = SemanticDecoder.cpp SemanticFeatures.h \
SemanticInstanceNumeric.h SemanticPipe.cpp SemanticWriter.h \
SemanticDecoder.h SemanticOptions.cpp \
SemanticPipe.h \
TurboSemanticParser.cpp \
SemanticDictionary.cpp SemanticInstance.cpp SemanticOptions.h \
SemanticReader.cpp SemanticDictionary.h \
SemanticPredicate.h \
SemanticInstance.h SemanticPart.cpp SemanticReader.h \
SemanticFeatures.cpp SemanticInstanceNumeric.cpp SemanticPart.h \
SemanticWriter.cpp \
SemanticFeatureTemplates.h \
FactorArgumentAutomaton.h \
FactorPredicateAutomaton.h \
FactorSemanticGraph.h \
$(PARSER)/DependencyInstanceNumeric.cpp \
$(PARSER)/DependencyInstanceNumeric.h \
$(PARSER)/DependencyWriter.cpp \
$(PARSER)/DependencyWriter.h \
$(PARSER)/DependencyDictionary.cpp \
$(PARSER)/DependencyDictionary.h \
$(PARSER)/DependencyInstance.cpp \
$(PARSER)/DependencyInstance.h \
$(PARSER)/DependencyReader.cpp \
$(PARSER)/DependencyReader.h \
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

AM_CPPFLAGS = -I$(UTIL) -I$(CLASSIFIER) -I$(SEQUENCE) -I$(PARSER) $(CPPFLAGS)
LDADD = $(LFLAGS)

