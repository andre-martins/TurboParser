UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
ENTITY_RECOGNIZER = ../entity_recognizer
PARSER = ../parser

TurboDependencyLabelerprgdir = ../..
TurboDependencyLabelerprg_PROGRAMS = TurboDependencyLabeler
TurboDependencyLabeler_SOURCES = DependencyLabelerDecoder.cpp \
DependencyLabelerFeatures.h \
DependencyLabelerPipe.cpp \
DependencyLabelerDecoder.h DependencyLabelerFeatureTemplates.h \
DependencyLabelerOptions.cpp \
DependencyLabelerPipe.h \
TurboDependencyLabeler.cpp \
DependencyLabelerOptions.h \
DependencyLabelerFeatures.cpp \
DependencyLabelerPart.cpp \
DependencyLabelerPart.h \
$(PARSER)/DependencyReader.cpp $(PARSER)/DependencyDictionary.h \
$(PARSER)/DependencyInstance.h \
$(PARSER)/DependencyReader.h \
$(PARSER)/DependencyDictionary.cpp \
$(PARSER)/DependencyInstance.cpp \
$(PARSER)/DependencyInstanceNumeric.h \
$(PARSER)/DependencyWriter.h \
$(PARSER)/DependencyInstanceNumeric.cpp \
$(PARSER)/DependencyWriter.cpp \
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

AM_CPPFLAGS = -I$(UTIL) -I$(CLASSIFIER) -I$(SEQUENCE) -I$(ENTITY_RECOGNIZER) -I$(PARSER) $(CPPFLAGS)
LDADD = $(LFLAGS)

