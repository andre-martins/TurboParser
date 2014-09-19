UTIL = ../src/util
CLASSIFIER = ../src/classifier
TAGGER = ../src/tagger
PARSER = ../src/parser
SEMANTICPARSER = ../src/semantic_parser
AUXLIBS = ../deps/local/lib
AUXINCLUDES = ../deps/local/include

OBJS = TurboParserInterface.o SemanticDecoder.o SemanticDictionary.o SemanticFeatures.o SemanticInstanceNumeric.o SemanticInstance.o SemanticOptions.o SemanticPart.o SemanticPipe.o SemanticReader.o SemanticWriter.o DependencyDecoder.o DependencyDictionary.o DependencyFeatures.o DependencyInstance.o DependencyInstanceNumeric.o DependencyOptions.o DependencyPart.o DependencyPipe.o DependencyReader.o DependencyWriter.o SequenceDecoder.o SequenceDictionary.o SequenceFeatures.o SequenceInstance.o SequenceInstanceNumeric.o SequenceOptions.o SequencePart.o SequencePipe.o SequenceReader.o SequenceWriter.o TokenDictionary.o Alphabet.o Dictionary.o Options.o Parameters.o Pipe.o Reader.o Writer.o AlgUtils.o SerializationUtils.o StringUtils.o TimeUtils.o
CC = g++
DEBUG = -g
INCLUDES = -I$(UTIL)/ -I$(CLASSIFIER) -I$(TAGGER) -I$(PARSER) -I$(SEMANTICPARSER) -I$(AUXINCLUDES)
LIBS = -L/usr/local/lib/ -L$(AUXLIBS)
CFLAGS = -O3 -Wall -Wno-sign-compare -c -fmessage-length=0 -fPIC $(INCLUDES)
LFLAGS = $(LIBS) -lad3 -lgflags -lglog

all : libturboparser.a

libturboparser.a : $(OBJS)
	ar rcs libturboparser.a $(OBJS)

TurboParserInterface.o: TurboParserInterface.h TurboParserInterface.cpp $(TAGGER)/SequencePipe.h $(PARSER)/DependencyPipe.h $(SEMANTICPARSER)/SemanticPipe.h $(UTIL)/Utils.h
	$(CC) $(CFLAGS) TurboParserInterface.cpp

#####################

SemanticDecoder.o: $(SEMANTICPARSER)/SemanticDecoder.h $(SEMANTICPARSER)/SemanticDecoder.cpp $(SEMANTICPARSER)/SemanticPart.h $(SEMANTICPARSER)/SemanticPipe.h $(PARSER)/FactorTree.h $(SEMANTICPARSER)/FactorPredicateAutomaton.h $(SEMANTICPARSER)/FactorArgumentAutomaton.h $(UTIL)/AlgUtils.h $(UTIL)/logval.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticDecoder.cpp

SemanticDictionary.o: $(SEMANTICPARSER)/SemanticDictionary.h $(SEMANTICPARSER)/SemanticDictionary.cpp $(SEMANTICPARSER)/SemanticPipe.h $(CLASSIFIER)/Dictionary.h $(TAGGER)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticDictionary.cpp

SemanticFeatures.o: $(SEMANTICPARSER)/SemanticFeatures.h $(SEMANTICPARSER)/SemanticFeatures.cpp $(SEMANTICPARSER)/SemanticPipe.h $(SEMANTICPARSER)/SemanticPart.h $(SEMANTICPARSER)/SemanticFeatureTemplates.h $(CLASSIFIER)/Features.h $(SEMANTICPARSER)/SemanticInstanceNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticFeatures.cpp

SemanticInstance.o: $(SEMANTICPARSER)/SemanticInstance.h $(SEMANTICPARSER)/SemanticInstance.cpp $(CLASSIFIER)/Instance.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticInstance.cpp

SemanticInstanceNumeric.o: $(SEMANTICPARSER)/SemanticInstanceNumeric.h $(SEMANTICPARSER)/SemanticInstanceNumeric.cpp $(SEMANTICPARSER)/SemanticInstance.h  $(SEMANTICPARSER)/SemanticDictionary.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticInstanceNumeric.cpp

SemanticOptions.o: $(SEMANTICPARSER)/SemanticOptions.h $(SEMANTICPARSER)/SemanticOptions.cpp $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Options.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticOptions.cpp

SemanticPart.o: $(SEMANTICPARSER)/SemanticPart.h $(SEMANTICPARSER)/SemanticPart.cpp $(CLASSIFIER)/Part.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticPart.cpp

SemanticPipe.o: $(SEMANTICPARSER)/SemanticPipe.h $(SEMANTICPARSER)/SemanticPipe.cpp $(CLASSIFIER)/Pipe.h $(SEMANTICPARSER)/SemanticOptions.h $(SEMANTICPARSER)/SemanticReader.h $(SEMANTICPARSER)/SemanticDictionary.h $(TAGGER)/TokenDictionary.h $(SEMANTICPARSER)/SemanticInstanceNumeric.h $(SEMANTICPARSER)/SemanticWriter.h $(SEMANTICPARSER)/SemanticPart.h $(SEMANTICPARSER)/SemanticFeatures.h $(SEMANTICPARSER)/SemanticDecoder.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticPipe.cpp

SemanticReader.o: $(SEMANTICPARSER)/SemanticReader.h $(SEMANTICPARSER)/SemanticReader.cpp $(SEMANTICPARSER)/SemanticInstance.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticReader.cpp

SemanticWriter.o: $(SEMANTICPARSER)/SemanticWriter.h $(SEMANTICPARSER)/SemanticWriter.cpp $(SEMANTICPARSER)/SemanticInstance.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticWriter.cpp

#####################

DependencyDecoder.o: $(PARSER)/DependencyDecoder.h $(PARSER)/DependencyDecoder.cpp $(PARSER)/DependencyPart.h $(PARSER)/DependencyPipe.h $(PARSER)/FactorTree.h $(PARSER)/FactorHeadAutomaton.h $(PARSER)/FactorGrandparentHeadAutomaton.h $(PARSER)/FactorTrigramHeadAutomaton.h $(PARSER)/FactorSequence.h $(UTIL)/AlgUtils.h $(UTIL)/logval.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyDecoder.cpp

DependencyDictionary.o: $(PARSER)/DependencyDictionary.h $(PARSER)/DependencyDictionary.cpp $(PARSER)/DependencyPipe.h $(CLASSIFIER)/Dictionary.h $(TAGGER)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyDictionary.cpp

DependencyFeatures.o: $(PARSER)/DependencyFeatures.h $(PARSER)/DependencyFeatures.cpp $(PARSER)/DependencyPipe.h $(PARSER)/DependencyPart.h $(PARSER)/DependencyFeatureTemplates.h $(CLASSIFIER)/Features.h $(PARSER)/DependencyInstanceNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyFeatures.cpp

DependencyInstance.o: $(PARSER)/DependencyInstance.h $(PARSER)/DependencyInstance.cpp $(CLASSIFIER)/Instance.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyInstance.cpp

DependencyInstanceNumeric.o: $(PARSER)/DependencyInstanceNumeric.h $(PARSER)/DependencyInstanceNumeric.cpp $(PARSER)/DependencyInstance.h  $(PARSER)/DependencyDictionary.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyInstanceNumeric.cpp

DependencyOptions.o: $(PARSER)/DependencyOptions.h $(PARSER)/DependencyOptions.cpp $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Options.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyOptions.cpp

DependencyPart.o: $(PARSER)/DependencyPart.h $(PARSER)/DependencyPart.cpp $(CLASSIFIER)/Part.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyPart.cpp

DependencyPipe.o: $(PARSER)/DependencyPipe.h $(PARSER)/DependencyPipe.cpp $(CLASSIFIER)/Pipe.h $(PARSER)/DependencyOptions.h $(PARSER)/DependencyReader.h $(PARSER)/DependencyDictionary.h $(TAGGER)/TokenDictionary.h $(PARSER)/DependencyInstanceNumeric.h $(PARSER)/DependencyWriter.h $(PARSER)/DependencyPart.h $(PARSER)/DependencyFeatures.h $(PARSER)/DependencyDecoder.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyPipe.cpp

DependencyReader.o: $(PARSER)/DependencyReader.h $(PARSER)/DependencyReader.cpp $(PARSER)/DependencyInstance.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyReader.cpp

DependencyWriter.o: $(PARSER)/DependencyWriter.h $(PARSER)/DependencyWriter.cpp $(PARSER)/DependencyInstance.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyWriter.cpp

#####################

SequenceDecoder.o: $(TAGGER)/SequenceDecoder.h $(TAGGER)/SequenceDecoder.cpp $(TAGGER)/SequencePart.h $(TAGGER)/SequencePipe.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceDecoder.cpp

SequenceDictionary.o: $(TAGGER)/SequenceDictionary.h $(TAGGER)/SequenceDictionary.cpp $(TAGGER)/SequencePipe.h $(CLASSIFIER)/Dictionary.h $(TAGGER)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceDictionary.cpp

SequenceFeatures.o: $(TAGGER)/SequenceFeatures.h $(TAGGER)/SequenceFeatures.cpp $(TAGGER)/SequencePipe.h $(TAGGER)/SequencePart.h $(TAGGER)/SequenceFeatureTemplates.h $(CLASSIFIER)/Features.h $(TAGGER)/SequenceInstanceNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceFeatures.cpp

SequenceInstance.o: $(TAGGER)/SequenceInstance.h $(TAGGER)/SequenceInstance.cpp $(CLASSIFIER)/Instance.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceInstance.cpp

SequenceInstanceNumeric.o: $(TAGGER)/SequenceInstanceNumeric.h $(TAGGER)/SequenceInstanceNumeric.cpp $(TAGGER)/SequenceInstance.h  $(TAGGER)/SequenceDictionary.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceInstanceNumeric.cpp

SequenceOptions.o: $(TAGGER)/SequenceOptions.h $(TAGGER)/SequenceOptions.cpp $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Options.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceOptions.cpp

SequencePart.o: $(TAGGER)/SequencePart.h $(TAGGER)/SequencePart.cpp $(CLASSIFIER)/Part.h
	$(CC) $(CFLAGS) $(TAGGER)/SequencePart.cpp

SequencePipe.o: $(TAGGER)/SequencePipe.h $(TAGGER)/SequencePipe.cpp $(CLASSIFIER)/Pipe.h $(TAGGER)/SequenceOptions.h $(TAGGER)/SequenceReader.h $(TAGGER)/SequenceDictionary.h $(TAGGER)/TokenDictionary.h $(TAGGER)/SequenceInstanceNumeric.h $(TAGGER)/SequenceWriter.h $(TAGGER)/SequencePart.h $(TAGGER)/SequenceFeatures.h $(TAGGER)/SequenceDecoder.h
	$(CC) $(CFLAGS) $(TAGGER)/SequencePipe.cpp

SequenceReader.o: $(TAGGER)/SequenceReader.h $(TAGGER)/SequenceReader.cpp $(TAGGER)/SequenceInstance.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceReader.cpp

SequenceWriter.o: $(TAGGER)/SequenceWriter.h $(TAGGER)/SequenceWriter.cpp $(TAGGER)/SequenceInstance.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(TAGGER)/SequenceWriter.cpp

TokenDictionary.o: $(TAGGER)/TokenDictionary.h $(TAGGER)/TokenDictionary.cpp $(CLASSIFIER)/Pipe.h $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Dictionary.h $(CLASSIFIER)/Alphabet.h $(TAGGER)/SequenceReader.h $(PARSER)/DependencyReader.h
	$(CC) $(CFLAGS) $(TAGGER)/TokenDictionary.cpp

#####################

Alphabet.o: $(CLASSIFIER)/Alphabet.h $(CLASSIFIER)/Alphabet.cpp $(UTIL)/SerializationUtils.h $(UTIL)/Utils.h
	$(CC) $(CFLAGS) $(CLASSIFIER)/Alphabet.cpp

Dictionary.o: $(CLASSIFIER)/Dictionary.h $(CLASSIFIER)/Dictionary.cpp
	$(CC) $(CFLAGS) $(CLASSIFIER)/Dictionary.cpp

Options.o: $(CLASSIFIER)/Options.h $(CLASSIFIER)/Options.cpp
	$(CC) $(CFLAGS) $(CLASSIFIER)/Options.cpp

Parameters.o: $(CLASSIFIER)/Parameters.h $(CLASSIFIER)/Parameters.cpp $(CLASSIFIER)/Features.h $(CLASSIFIER)/SparseParameterVector.h $(CLASSIFIER)/SparseLabeledParameterVector.h $(UTIL)/Utils.h
	$(CC) $(CFLAGS) $(CLASSIFIER)/Parameters.cpp

Pipe.o: $(CLASSIFIER)/Pipe.h $(CLASSIFIER)/Pipe.cpp $(CLASSIFIER)/Dictionary.h $(CLASSIFIER)/Features.h $(CLASSIFIER)/Part.h $(CLASSIFIER)/Reader.h $(CLASSIFIER)/Writer.h $(CLASSIFIER)/Options.h $(CLASSIFIER)/Decoder.h $(CLASSIFIER)/Parameters.h $(UTIL)/AlgUtils.h
	$(CC) $(CFLAGS) $(CLASSIFIER)/Pipe.cpp

Reader.o: $(CLASSIFIER)/Reader.h $(CLASSIFIER)/Reader.cpp $(CLASSIFIER)/Instance.h $(UTIL)/Utils.h
	$(CC) $(CFLAGS) $(CLASSIFIER)/Reader.cpp

Writer.o: $(CLASSIFIER)/Writer.h $(CLASSIFIER)/Writer.cpp $(CLASSIFIER)/Instance.h $(UTIL)/Utils.h
	$(CC) $(CFLAGS) $(CLASSIFIER)/Writer.cpp

#####################

AlgUtils.o: $(UTIL)/AlgUtils.h $(UTIL)/AlgUtils.cpp
	$(CC) $(CFLAGS) $(UTIL)/AlgUtils.cpp

SerializationUtils.o: $(UTIL)/SerializationUtils.h $(UTIL)/SerializationUtils.cpp
	$(CC) $(CFLAGS) $(UTIL)/SerializationUtils.cpp

StringUtils.o: $(UTIL)/StringUtils.h $(UTIL)/StringUtils.cpp
	$(CC) $(CFLAGS) $(UTIL)/StringUtils.cpp

TimeUtils.o: $(UTIL)/TimeUtils.h $(UTIL)/TimeUtils.cpp
	$(CC) $(CFLAGS) $(UTIL)/TimeUtils.cpp

#####################

clean:
	rm -f *.o *~ libturboparser.a
