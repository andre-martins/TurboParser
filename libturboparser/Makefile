UTIL = ../src/util
CLASSIFIER = ../src/classifier
SEQUENCE = ../src/sequence
TAGGER = ../src/tagger
ENTITYRECOGNIZER = ../src/entity_recognizer
PARSER = ../src/parser
SEMANTICPARSER = ../src/semantic_parser
COREFERENCERESOLVER = ../src/coreference_resolver
MORPHOLOGICALTAGGER = ../src/morphological_tagger
AUXLIBS = ../deps/local/lib
AUXINCLUDES = ../deps/local/include

OBJS = TurboParserInterface.o CoreferenceDecoder.o CoreferenceDictionary.o CoreferenceFeatures.o CoreferenceDocumentNumeric.o CoreferenceSentenceNumeric.o Mention.o CoreferenceDocument.o CoreferenceSentence.o CoreferenceOptions.o CoreferencePart.o CoreferencePipe.o CoreferenceReader.o CoreferenceWriter.o SemanticDecoder.o SemanticDictionary.o SemanticFeatures.o SemanticInstanceNumeric.o SemanticInstance.o SemanticOptions.o SemanticPart.o SemanticPipe.o SemanticReader.o SemanticWriter.o DependencyDecoder.o DependencyDictionary.o DependencyFeatures.o DependencyInstance.o DependencyInstanceNumeric.o DependencyOptions.o DependencyPart.o DependencyPipe.o DependencyReader.o DependencyWriter.o TaggerDictionary.o TaggerFeatures.o TaggerOptions.o TaggerPipe.o MorphologicalDictionary.o MorphologicalFeatures.o MorphologicalInstance.o MorphologicalInstanceNumeric.o MorphologicalOptions.o MorphologicalPipe.o MorphologicalReader.o MorphologicalWriter.o EntityDictionary.o EntityFeatures.o EntityInstance.o EntityInstanceNumeric.o EntityOptions.o EntityPipe.o EntityReader.o EntityWriter.o EntityDecoder.o SequenceDecoder.o SequenceDictionary.o SequenceInstance.o SequenceInstanceNumeric.o SequenceOptions.o SequencePart.o SequencePipe.o SequenceReader.o SequenceWriter.o TokenDictionary.o Alphabet.o Dictionary.o Options.o Parameters.o Pipe.o Reader.o Writer.o AlgUtils.o SerializationUtils.o StringUtils.o TimeUtils.o
CC = g++
DEBUG = -g
INCLUDES = -I$(UTIL)/ -I$(CLASSIFIER) -I$(SEQUENCE) -I$(TAGGER) -I$(ENTITYRECOGNIZER) -I$(PARSER) -I$(SEMANTICPARSER) -I$(COREFERENCERESOLVER)  -I$(MORPHOLOGICALTAGGER) -I$(AUXINCLUDES)
LIBS = -L/usr/local/lib/ -L$(AUXLIBS)
CFLAGS = -std=gnu++11 -std=c++11 -O3 -Wall -Wno-sign-compare -c -fmessage-length=0 -fPIC $(INCLUDES)
LDFLAGS = -shared
LFLAGS = $(LIBS) -Wl,-whole-archive -lad3 -Wl,-no-whole-archive -lgflags -lglog

all : libturboparser.a libturboparser.so

libturboparser.a : $(OBJS)
	ar rcs libturboparser.a $(OBJS)

libturboparser.so : $(OBJS)
	$(CC) -o libturboparser.so $(OBJS) $(LDFLAGS) $(LFLAGS)

TurboParserInterface.o: TurboParserInterface.h TurboParserInterface.cpp $(TAGGER)/TaggerPipe.h $(ENTITYRECOGNIZER)/EntityPipe.h $(PARSER)/DependencyPipe.h $(SEMANTICPARSER)/SemanticPipe.h $(COREFERENCERESOLVER)/CoreferencePipe.h $(MORPHOLOGICALTAGGER)/MorphologicalPipe.h $(UTIL)/Utils.h
	$(CC) $(CFLAGS) TurboParserInterface.cpp

#####################

CoreferenceDecoder.o: $(COREFERENCERESOLVER)/CoreferenceDecoder.h $(COREFERENCERESOLVER)/CoreferenceDecoder.cpp $(COREFERENCERESOLVER)/CoreferencePart.h $(COREFERENCERESOLVER)/CoreferencePipe.h $(UTIL)/AlgUtils.h $(UTIL)/logval.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceDecoder.cpp

CoreferenceDictionary.o: $(COREFERENCERESOLVER)/CoreferenceDictionary.h $(COREFERENCERESOLVER)/CoreferenceDictionary.cpp $(COREFERENCERESOLVER)/CoreferencePipe.h $(CLASSIFIER)/Dictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceDictionary.cpp

CoreferenceFeatures.o: $(COREFERENCERESOLVER)/CoreferenceFeatures.h $(COREFERENCERESOLVER)/CoreferenceFeatures.cpp $(COREFERENCERESOLVER)/CoreferencePipe.h $(COREFERENCERESOLVER)/CoreferencePart.h $(COREFERENCERESOLVER)/CoreferenceFeatureTemplates.h $(CLASSIFIER)/Features.h $(COREFERENCERESOLVER)/CoreferenceDocumentNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceFeatures.cpp

CoreferenceDocument.o: $(COREFERENCERESOLVER)/CoreferenceDocument.h $(COREFERENCERESOLVER)/CoreferenceDocument.cpp $(COREFERENCERESOLVER)/CoreferenceSentence.h $(CLASSIFIER)/Instance.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceDocument.cpp

CoreferenceSentence.o: $(COREFERENCERESOLVER)/CoreferenceSentence.h $(COREFERENCERESOLVER)/CoreferenceSentence.cpp $(CLASSIFIER)/Instance.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceSentence.cpp

CoreferenceDocumentNumeric.o: $(COREFERENCERESOLVER)/CoreferenceDocumentNumeric.h $(COREFERENCERESOLVER)/CoreferenceDocumentNumeric.cpp $(COREFERENCERESOLVER)/CoreferenceSentenceNumeric.h $(COREFERENCERESOLVER)/CoreferenceDocument.h  $(COREFERENCERESOLVER)/CoreferenceDictionary.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceDocumentNumeric.cpp

CoreferenceSentenceNumeric.o: $(COREFERENCERESOLVER)/CoreferenceSentenceNumeric.h $(COREFERENCERESOLVER)/CoreferenceSentenceNumeric.cpp $(COREFERENCERESOLVER)/CoreferenceSentence.h $(COREFERENCERESOLVER)/Mention.h $(COREFERENCERESOLVER)/CoreferenceDictionary.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceSentenceNumeric.cpp

Mention.o: $(COREFERENCERESOLVER)/Mention.h $(COREFERENCERESOLVER)/Mention.cpp $(COREFERENCERESOLVER)/CoreferenceDictionary.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/Mention.cpp

CoreferenceOptions.o: $(COREFERENCERESOLVER)/CoreferenceOptions.h $(COREFERENCERESOLVER)/CoreferenceOptions.cpp $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Options.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceOptions.cpp

CoreferencePart.o: $(COREFERENCERESOLVER)/CoreferencePart.h $(COREFERENCERESOLVER)/CoreferencePart.cpp $(CLASSIFIER)/Part.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferencePart.cpp

CoreferencePipe.o: $(COREFERENCERESOLVER)/CoreferencePipe.h $(COREFERENCERESOLVER)/CoreferencePipe.cpp $(CLASSIFIER)/Pipe.h $(COREFERENCERESOLVER)/CoreferenceOptions.h $(COREFERENCERESOLVER)/CoreferenceReader.h $(COREFERENCERESOLVER)/CoreferenceDictionary.h $(SEQUENCE)/TokenDictionary.h $(COREFERENCERESOLVER)/CoreferenceDocumentNumeric.h $(COREFERENCERESOLVER)/CoreferenceWriter.h $(COREFERENCERESOLVER)/CoreferencePart.h $(COREFERENCERESOLVER)/CoreferenceFeatures.h $(COREFERENCERESOLVER)/CoreferenceDecoder.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferencePipe.cpp

CoreferenceReader.o: $(COREFERENCERESOLVER)/CoreferenceReader.h $(COREFERENCERESOLVER)/CoreferenceReader.cpp $(COREFERENCERESOLVER)/CoreferenceDocument.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceReader.cpp

CoreferenceWriter.o: $(COREFERENCERESOLVER)/CoreferenceWriter.h $(COREFERENCERESOLVER)/CoreferenceWriter.cpp $(COREFERENCERESOLVER)/CoreferenceDocument.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(COREFERENCERESOLVER)/CoreferenceWriter.cpp

#####################

SemanticDecoder.o: $(SEMANTICPARSER)/SemanticDecoder.h $(SEMANTICPARSER)/SemanticDecoder.cpp $(SEMANTICPARSER)/SemanticPart.h $(SEMANTICPARSER)/SemanticPipe.h $(PARSER)/FactorTree.h $(SEMANTICPARSER)/FactorPredicateAutomaton.h $(SEMANTICPARSER)/FactorArgumentAutomaton.h $(UTIL)/AlgUtils.h $(UTIL)/logval.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticDecoder.cpp

SemanticDictionary.o: $(SEMANTICPARSER)/SemanticDictionary.h $(SEMANTICPARSER)/SemanticDictionary.cpp $(SEMANTICPARSER)/SemanticPipe.h $(CLASSIFIER)/Dictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
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

SemanticPipe.o: $(SEMANTICPARSER)/SemanticPipe.h $(SEMANTICPARSER)/SemanticPipe.cpp $(CLASSIFIER)/Pipe.h $(SEMANTICPARSER)/SemanticOptions.h $(SEMANTICPARSER)/SemanticReader.h $(SEMANTICPARSER)/SemanticDictionary.h $(SEQUENCE)/TokenDictionary.h $(SEMANTICPARSER)/SemanticInstanceNumeric.h $(SEMANTICPARSER)/SemanticWriter.h $(SEMANTICPARSER)/SemanticPart.h $(SEMANTICPARSER)/SemanticFeatures.h $(SEMANTICPARSER)/SemanticDecoder.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticPipe.cpp

SemanticReader.o: $(SEMANTICPARSER)/SemanticReader.h $(SEMANTICPARSER)/SemanticReader.cpp $(SEMANTICPARSER)/SemanticInstance.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticReader.cpp

SemanticWriter.o: $(SEMANTICPARSER)/SemanticWriter.h $(SEMANTICPARSER)/SemanticWriter.cpp $(SEMANTICPARSER)/SemanticInstance.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(SEMANTICPARSER)/SemanticWriter.cpp

#####################

DependencyDecoder.o: $(PARSER)/DependencyDecoder.h $(PARSER)/DependencyDecoder.cpp $(PARSER)/DependencyPart.h $(PARSER)/DependencyPipe.h $(PARSER)/FactorTree.h $(PARSER)/FactorHeadAutomaton.h $(PARSER)/FactorGrandparentHeadAutomaton.h $(PARSER)/FactorTrigramHeadAutomaton.h $(PARSER)/FactorSequence.h $(UTIL)/AlgUtils.h $(UTIL)/logval.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyDecoder.cpp

DependencyDictionary.o: $(PARSER)/DependencyDictionary.h $(PARSER)/DependencyDictionary.cpp $(PARSER)/DependencyPipe.h $(CLASSIFIER)/Dictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
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

DependencyPipe.o: $(PARSER)/DependencyPipe.h $(PARSER)/DependencyPipe.cpp $(CLASSIFIER)/Pipe.h $(PARSER)/DependencyOptions.h $(PARSER)/DependencyReader.h $(PARSER)/DependencyDictionary.h $(SEQUENCE)/TokenDictionary.h $(PARSER)/DependencyInstanceNumeric.h $(PARSER)/DependencyWriter.h $(PARSER)/DependencyPart.h $(PARSER)/DependencyFeatures.h $(PARSER)/DependencyDecoder.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyPipe.cpp

DependencyReader.o: $(PARSER)/DependencyReader.h $(PARSER)/DependencyReader.cpp $(PARSER)/DependencyInstance.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyReader.cpp

DependencyWriter.o: $(PARSER)/DependencyWriter.h $(PARSER)/DependencyWriter.cpp $(PARSER)/DependencyInstance.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(PARSER)/DependencyWriter.cpp

#####################

TaggerDictionary.o: $(TAGGER)/TaggerDictionary.h $(TAGGER)/TaggerDictionary.cpp $(TAGGER)/TaggerPipe.h $(SEQUENCE)/SequenceDictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(TAGGER)/TaggerDictionary.cpp

TaggerFeatures.o: $(TAGGER)/TaggerFeatures.h $(TAGGER)/TaggerFeatures.cpp $(TAGGER)/TaggerPipe.h $(TAGGER)/TaggerFeatureTemplates.h $(SEQUENCE)/SequenceFeatures.h $(SEQUENCE)/SequenceInstanceNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(TAGGER)/TaggerFeatures.cpp

TaggerOptions.o: $(TAGGER)/TaggerOptions.h $(TAGGER)/TaggerOptions.cpp $(UTIL)/SerializationUtils.h $(SEQUENCE)/SequenceOptions.h
	$(CC) $(CFLAGS) $(TAGGER)/TaggerOptions.cpp

TaggerPipe.o: $(TAGGER)/TaggerPipe.h $(TAGGER)/TaggerPipe.cpp $(SEQUENCE)/SequencePipe.h $(TAGGER)/TaggerOptions.h $(TAGGER)/TaggerDictionary.h
	$(CC) $(CFLAGS) $(TAGGER)/TaggerPipe.cpp

#####################

MorphologicalDictionary.o: $(MORPHOLOGICALTAGGER)/MorphologicalDictionary.h $(MORPHOLOGICALTAGGER)/MorphologicalDictionary.cpp $(MORPHOLOGICALTAGGER)/MorphologicalPipe.h $(SEQUENCE)/SequenceDictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalDictionary.cpp

MorphologicalFeatures.o: $(MORPHOLOGICALTAGGER)/MorphologicalFeatures.h $(MORPHOLOGICALTAGGER)/MorphologicalFeatures.cpp $(MORPHOLOGICALTAGGER)/MorphologicalPipe.h $(MORPHOLOGICALTAGGER)/MorphologicalFeatureTemplates.h $(SEQUENCE)/SequenceFeatures.h $(SEQUENCE)/SequenceInstanceNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalFeatures.cpp

MorphologicalInstance.o: $(MORPHOLOGICALTAGGER)/MorphologicalInstance.h $(MORPHOLOGICALTAGGER)/MorphologicalInstance.cpp $(SEQUENCE)/SequenceInstance.h 
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalInstance.cpp

MorphologicalInstanceNumeric.o: $(MORPHOLOGICALTAGGER)/MorphologicalInstanceNumeric.h $(MORPHOLOGICALTAGGER)/MorphologicalInstanceNumeric.cpp $(MORPHOLOGICALTAGGER)/MorphologicalInstance.h $(SEQUENCE)/SequenceInstanceNumeric.h $(MORPHOLOGICALTAGGER)/MorphologicalDictionary.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalInstanceNumeric.cpp

MorphologicalOptions.o: $(MORPHOLOGICALTAGGER)/MorphologicalOptions.h $(MORPHOLOGICALTAGGER)/MorphologicalOptions.cpp $(UTIL)/SerializationUtils.h $(SEQUENCE)/SequenceOptions.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalOptions.cpp

MorphologicalPipe.o: $(MORPHOLOGICALTAGGER)/MorphologicalPipe.h $(MORPHOLOGICALTAGGER)/MorphologicalPipe.cpp $(SEQUENCE)/SequencePipe.h $(MORPHOLOGICALTAGGER)/MorphologicalOptions.h $(MORPHOLOGICALTAGGER)/MorphologicalDictionary.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalPipe.cpp

MorphologicalReader.o: $(MORPHOLOGICALTAGGER)/MorphologicalReader.h $(MORPHOLOGICALTAGGER)/MorphologicalReader.cpp $(MORPHOLOGICALTAGGER)/MorphologicalInstance.h $(SEQUENCE)/SequenceReader.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalReader.cpp

MorphologicalWriter.o: $(MORPHOLOGICALTAGGER)/MorphologicalWriter.h $(MORPHOLOGICALTAGGER)/MorphologicalWriter.cpp $(MORPHOLOGICALTAGGER)/MorphologicalInstance.h $(SEQUENCE)/SequenceWriter.h
	$(CC) $(CFLAGS) $(MORPHOLOGICALTAGGER)/MorphologicalWriter.cpp

#####################

EntityDictionary.o: $(ENTITYRECOGNIZER)/EntityDictionary.h $(ENTITYRECOGNIZER)/EntityDictionary.cpp $(ENTITYRECOGNIZER)/EntityPipe.h $(SEQUENCE)/SequenceDictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityDictionary.cpp

EntityFeatures.o: $(ENTITYRECOGNIZER)/EntityFeatures.h $(ENTITYRECOGNIZER)/EntityFeatures.cpp $(ENTITYRECOGNIZER)/EntityPipe.h $(ENTITYRECOGNIZER)/EntityFeatureTemplates.h $(SEQUENCE)/SequenceFeatures.h $(SEQUENCE)/SequenceInstanceNumeric.h $(CLASSIFIER)/FeatureEncoder.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityFeatures.cpp

EntityOptions.o: $(ENTITYRECOGNIZER)/EntityOptions.h $(ENTITYRECOGNIZER)/EntityOptions.cpp $(UTIL)/SerializationUtils.h $(SEQUENCE)/SequenceOptions.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityOptions.cpp

EntityPipe.o: $(ENTITYRECOGNIZER)/EntityPipe.h $(ENTITYRECOGNIZER)/EntityPipe.cpp $(SEQUENCE)/SequencePipe.h $(ENTITYRECOGNIZER)/EntityOptions.h $(ENTITYRECOGNIZER)/EntityDictionary.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityPipe.cpp

EntityReader.o: $(ENTITYRECOGNIZER)/EntityReader.h $(ENTITYRECOGNIZER)/EntityReader.cpp $(ENTITYRECOGNIZER)/EntityInstance.h $(SEQUENCE)/SequenceReader.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityReader.cpp

EntityWriter.o: $(ENTITYRECOGNIZER)/EntityWriter.h $(ENTITYRECOGNIZER)/EntityWriter.cpp $(ENTITYRECOGNIZER)/EntityInstance.h $(SEQUENCE)/SequenceWriter.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityWriter.cpp

EntityInstance.o: $(ENTITYRECOGNIZER)/EntityInstance.h $(ENTITYRECOGNIZER)/EntityInstance.cpp $(SEQUENCE)/SequenceInstance.h 
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityInstance.cpp

EntityInstanceNumeric.o: $(ENTITYRECOGNIZER)/EntityInstanceNumeric.h $(ENTITYRECOGNIZER)/EntityInstanceNumeric.cpp $(ENTITYRECOGNIZER)/EntityInstance.h $(SEQUENCE)/SequenceInstanceNumeric.h $(ENTITYRECOGNIZER)/EntityDictionary.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityInstanceNumeric.cpp
    
EntityDecoder.o: $(ENTITYRECOGNIZER)/EntityDecoder.h $(ENTITYRECOGNIZER)/EntityDecoder.cpp $(SEQUENCE)/SequencePart.h $(ENTITYRECOGNIZER)/EntityPipe.h $(SEQUENCE)/SequenceDecoder.h
	$(CC) $(CFLAGS) $(ENTITYRECOGNIZER)/EntityDecoder.cpp 
    
#####################

SequenceDecoder.o: $(SEQUENCE)/SequenceDecoder.h $(SEQUENCE)/SequenceDecoder.cpp $(SEQUENCE)/SequencePart.h $(SEQUENCE)/SequencePipe.h $(CLASSIFIER)/Decoder.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceDecoder.cpp

SequenceDictionary.o: $(SEQUENCE)/SequenceDictionary.h $(SEQUENCE)/SequenceDictionary.cpp $(SEQUENCE)/SequencePipe.h $(CLASSIFIER)/Dictionary.h $(SEQUENCE)/TokenDictionary.h $(UTIL)/SerializationUtils.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceDictionary.cpp

SequenceInstance.o: $(SEQUENCE)/SequenceInstance.h $(SEQUENCE)/SequenceInstance.cpp $(CLASSIFIER)/Instance.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceInstance.cpp

SequenceInstanceNumeric.o: $(SEQUENCE)/SequenceInstanceNumeric.h $(SEQUENCE)/SequenceInstanceNumeric.cpp $(SEQUENCE)/SequenceInstance.h  $(SEQUENCE)/SequenceDictionary.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceInstanceNumeric.cpp

SequenceOptions.o: $(SEQUENCE)/SequenceOptions.h $(SEQUENCE)/SequenceOptions.cpp $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Options.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceOptions.cpp

SequencePart.o: $(SEQUENCE)/SequencePart.h $(SEQUENCE)/SequencePart.cpp $(CLASSIFIER)/Part.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequencePart.cpp

SequencePipe.o: $(SEQUENCE)/SequencePipe.h $(SEQUENCE)/SequencePipe.cpp $(CLASSIFIER)/Pipe.h $(SEQUENCE)/SequenceOptions.h $(SEQUENCE)/SequenceReader.h $(SEQUENCE)/SequenceDictionary.h $(SEQUENCE)/TokenDictionary.h $(SEQUENCE)/SequenceInstanceNumeric.h $(SEQUENCE)/SequenceWriter.h $(SEQUENCE)/SequencePart.h $(SEQUENCE)/SequenceFeatures.h $(SEQUENCE)/SequenceDecoder.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequencePipe.cpp

SequenceReader.o: $(SEQUENCE)/SequenceReader.h $(SEQUENCE)/SequenceReader.cpp $(SEQUENCE)/SequenceInstance.h $(CLASSIFIER)/Reader.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceReader.cpp

SequenceWriter.o: $(SEQUENCE)/SequenceWriter.h $(SEQUENCE)/SequenceWriter.cpp $(SEQUENCE)/SequenceInstance.h $(CLASSIFIER)/Writer.h
	$(CC) $(CFLAGS) $(SEQUENCE)/SequenceWriter.cpp

TokenDictionary.o: $(SEQUENCE)/TokenDictionary.h $(SEQUENCE)/TokenDictionary.cpp $(CLASSIFIER)/Pipe.h $(UTIL)/SerializationUtils.h $(CLASSIFIER)/Dictionary.h $(CLASSIFIER)/Alphabet.h $(SEQUENCE)/SequenceReader.h $(PARSER)/DependencyReader.h
	$(CC) $(CFLAGS) $(SEQUENCE)/TokenDictionary.cpp

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
	rm -f *.o *~ libturboparser.a libturboparser.so
