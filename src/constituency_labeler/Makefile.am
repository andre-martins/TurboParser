UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
ENTITY_RECOGNIZER = ../entity_recognizer
PARSER = ../parser

TurboConstituencyLabelerprgdir = ../..
TurboConstituencyLabelerprg_PROGRAMS = TurboConstituencyLabeler
TurboConstituencyLabeler_SOURCES = TurboConstituencyLabeler.cpp \
ConstituencyLabelerDecoder.cpp \
ConstituencyLabelerDecoder.h \
ConstituencyLabelerFeatures.cpp \
ConstituencyLabelerFeatures.h \
ConstituencyLabelerFeatureTemplates.h \
ConstituencyLabelerPipe.cpp \
ConstituencyLabelerPipe.h \
ConstituencyLabelerOptions.cpp \
ConstituencyLabelerOptions.h \
ConstituencyLabelerDictionary.cpp \
ConstituencyLabelerDictionary.h \
ConstituencyLabelerPart.cpp \
ConstituencyLabelerPart.h \
ConstituencyLabelerReader.cpp \
ConstituencyLabelerReader.h \
ConstituencyLabelerWriter.cpp \
ConstituencyLabelerWriter.h \
ConstituencyLabelerInstance.cpp \
ConstituencyLabelerInstance.h \
ConstituencyLabelerInstanceNumeric.cpp \
ConstituencyLabelerInstanceNumeric.h \
ConstituencyDictionary.cpp \
ConstituencyDictionary.h \
ConstituencyReader.cpp \
ConstituencyReader.h \
ConstituencyWriter.cpp \
ConstituencyWriter.h \
ConstituencyInstance.cpp \
ConstituencyInstance.h \
ConstituencyInstanceNumeric.cpp \
ConstituencyInstanceNumeric.h \
ParseTree.cpp \
ParseTree.h \
Span.h \
$(SEQUENCE)/TokenDictionary.cpp $(SEQUENCE)/TokenDictionary.h \
$(SEQUENCE)/SequenceDictionary.cpp $(SEQUENCE)/SequenceDictionary.h \
$(SEQUENCE)/SequenceInstanceNumeric.cpp $(SEQUENCE)/SequenceInstanceNumeric.h \
$(SEQUENCE)/SequenceInstance.cpp $(SEQUENCE)/SequenceInstance.h \
$(SEQUENCE)/SequenceReader.cpp $(SEQUENCE)/SequenceReader.h \
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

