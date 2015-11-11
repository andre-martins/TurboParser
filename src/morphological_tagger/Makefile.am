UTIL = ../util
CLASSIFIER = ../classifier
SEQUENCE = ../sequence
ENTITY_RECOGNIZER = ../entity_recognizer
PARSER = ../parser

TurboMorphologicalTaggerprgdir = ../..
TurboMorphologicalTaggerprg_PROGRAMS = TurboMorphologicalTagger
TurboMorphologicalTagger_SOURCES = MorphologicalDictionary.cpp \
MorphologicalDictionary.h \
MorphologicalFeatures.cpp \
MorphologicalFeatures.h \
MorphologicalFeatureTemplates.h \
MorphologicalInstance.cpp \
MorphologicalInstance.h \
MorphologicalInstanceNumeric.cpp \
MorphologicalInstanceNumeric.h \
MorphologicalOptions.cpp \
MorphologicalOptions.h \
MorphologicalPipe.cpp \
MorphologicalPipe.h \
MorphologicalReader.cpp \
MorphologicalReader.h \
MorphologicalWriter.cpp \
MorphologicalWriter.h \
TurboMorphologicalTagger.cpp \
$(SEQUENCE)/SequenceFeatures.h \
$(SEQUENCE)/SequencePipe.cpp  $(SEQUENCE)/SequencePipe.h \
$(SEQUENCE)/SequenceReader.cpp $(SEQUENCE)/SequenceReader.h \
$(SEQUENCE)/SequenceWriter.cpp $(SEQUENCE)/SequenceWriter.h \
$(SEQUENCE)/SequencePart.cpp $(SEQUENCE)/SequencePart.h \
$(SEQUENCE)/SequenceDecoder.cpp $(SEQUENCE)/SequenceDecoder.h \ 
$(SEQUENCE)/SequenceOptions.cpp  $(SEQUENCE)/SequenceOptions.h \
$(SEQUENCE)/TokenDictionary.cpp $(SEQUENCE)/TokenDictionary.h \  
$(SEQUENCE)/SequenceInstance.cpp  $(SEQUENCE)/SequenceInstance.h \
$(SEQUENCE)/SequenceInstanceNumeric.cpp $(SEQUENCE)/SequenceInstanceNumeric.h \
$(SEQUENCE)/SequenceDictionary.cpp $(SEQUENCE)/SequenceDictionary.h \
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

