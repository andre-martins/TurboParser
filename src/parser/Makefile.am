UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
ENTITY_RECOGNIZER = ../entity_recognizer

TurboParserprgdir = ../..
TurboParserprg_PROGRAMS = TurboParser
TurboParser_SOURCES = DependencyDecoder.cpp DependencyFeatures.h \
DependencyInstanceNumeric.h DependencyPipe.cpp DependencyWriter.h \
DependencyDecoder.h DependencyFeatureTemplates.h DependencyOptions.cpp \
DependencyPipe.h FactorGrandparentHeadAutomaton.h FactorTrigramHeadAutomaton.h \
TurboParser.cpp \
DependencyDictionary.cpp DependencyInstance.cpp DependencyOptions.h \
DependencyReader.cpp FactorHeadAutomaton.h DependencyDictionary.h \
DependencyInstance.h DependencyPart.cpp DependencyReader.h FactorSequence.h \
DependencyFeatures.cpp DependencyInstanceNumeric.cpp DependencyPart.h \
DependencyWriter.cpp FactorTree.h \
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

AM_CPPFLAGS = -I$(UTIL) -I$(CLASSIFIER) -I$(SEQUENCE) -I$(ENTITY_RECOGNIZER) $(CPPFLAGS)
LDADD = $(LFLAGS)

