UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
ENTITY_RECOGNIZER = ../entity_recognizer
PARSER = ../parser

TurboTaggerprgdir = ../..
TurboTaggerprg_PROGRAMS = TurboTagger
TurboTagger_SOURCES = TaggerFeatures.cpp TaggerFeatures.h \
TaggerFeatureTemplates.h TaggerOptions.cpp TaggerOptions.h \
TaggerDictionary.cpp TaggerDictionary.h \
TaggerPipe.cpp TaggerPipe.h \
$(SEQUENCE)/SequenceInstanceNumeric.cpp \
$(SEQUENCE)/SequencePart.h $(SEQUENCE)/SequenceWriter.cpp \
$(SEQUENCE)/SequenceDecoder.cpp $(SEQUENCE)/SequenceFeatures.h \
$(SEQUENCE)/SequenceInstanceNumeric.h $(SEQUENCE)/SequencePipe.cpp \
$(SEQUENCE)/SequenceWriter.h $(SEQUENCE)/SequenceDecoder.h \
$(SEQUENCE)/SequenceOptions.cpp $(SEQUENCE)/SequencePipe.h \
$(SEQUENCE)/TokenDictionary.cpp $(SEQUENCE)/SequenceDictionary.cpp \
$(SEQUENCE)/SequenceInstance.cpp $(SEQUENCE)/SequenceOptions.h \
$(SEQUENCE)/SequenceReader.cpp $(SEQUENCE)/SequenceDictionary.h \
$(SEQUENCE)/SequenceInstance.h $(SEQUENCE)/SequencePart.cpp \
$(SEQUENCE)/SequenceReader.h $(SEQUENCE)/TokenDictionary.h \
$(SEQUENCE)/Lexicon.h \
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
$(UTIL)/Utils.h \
TurboTagger.cpp

AM_CPPFLAGS = -I$(UTIL) -I$(CLASSIFIER) -I$(SEQUENCE) -I$(ENTITY_RECOGNIZER) -I$(PARSER) $(CPPFLAGS)
LDADD = $(LFLAGS)

