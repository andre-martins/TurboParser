UTIL = ../util
CLASSIFIER = ../classifier
PARSER = ../parser

TurboTaggerprgdir = ../..
TurboTaggerprg_PROGRAMS = TurboTagger
TurboTagger_SOURCES = SequenceFeatures.cpp SequenceInstanceNumeric.cpp \
SequencePart.h SequenceWriter.cpp SequenceDecoder.cpp SequenceFeatures.h \
SequenceInstanceNumeric.h SequencePipe.cpp SequenceWriter.h SequenceDecoder.h \
SequenceFeatureTemplates.h SequenceOptions.cpp SequencePipe.h \
TokenDictionary.cpp SequenceDictionary.cpp SequenceInstance.cpp \
SequenceOptions.h SequenceReader.cpp TokenDictionary.h SequenceDictionary.h \
SequenceInstance.h SequencePart.cpp SequenceReader.h TurboTagger.cpp \
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

AM_CPPFLAGS = -I$(UTIL) -I$(CLASSIFIER) -I$(PARSER) $(CPPFLAGS)
LDADD = $(LFLAGS)

