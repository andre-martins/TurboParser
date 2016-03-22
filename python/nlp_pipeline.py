import nltk
import tokenizers.portuguese.word_tokenizer as tokenizer_PT
import lemmatizer
import turboparser as tp
from nlp_sentence import NLPSentence
from nlp_document import NLPDocument
import nlp_utils
from span import Span
import os
import pdb

class NLPPipelineWorker:
    def __init__(self, pipeline, language):
        self.tagger = None
        self.morphological_tagger = None
        self.entity_recognizer = None
        self.parser = None
        self.semantic_parser = None
        self.lemmatizer = None
        self.coreference_resolver = None

        if language not in pipeline.models:
            print 'Error: no model for language %s.' % language
            raise NotImplementedError

        if 'splitter' in pipeline.models[language]:
            self.sent_tokenizer = nltk.data.load(pipeline.models[language]['splitter'])
        else:
            # If no splitter is specified, use the English model.
            self.sent_tokenizer = nltk.data.load('tokenizers/punkt/english.pickle')

        if language == 'PT':
            self.word_tokenizer = tokenizer_PT.PortugueseFlorestaWordTokenizer()
        elif language == 'PT-Cintil':
            self.word_tokenizer = tokenizer_PT.PortugueseCintilWordTokenizer()
        else:
            self.word_tokenizer = nltk.TreebankWordTokenizer() # For now...

        if 'tagger' in pipeline.models[language]:
            self.tagger = pipeline.turbo_interface.create_tagger()
            self.tagger.load_tagger_model(pipeline.models[language]['tagger'])
        if 'morphological_tagger' in pipeline.models[language]:
            self.morphological_tagger = pipeline.turbo_interface.create_morphological_tagger()
            self.morphological_tagger.load_morphological_tagger_model(pipeline.models[language]['morphological_tagger'])
        if 'entity_recognizer' in pipeline.models[language]:
            self.entity_recognizer = pipeline.turbo_interface.create_entity_recognizer()
            self.entity_recognizer.load_entity_recognizer_model(pipeline.models[language]['entity_recognizer'])
        if 'parser' in pipeline.models[language]:
            self.parser = pipeline.turbo_interface.create_parser()
            self.parser.load_parser_model(pipeline.models[language]['parser'])
        if 'lemmatizer' in pipeline.models[language]:
            self.lemmatizer = lemmatizer.BasicLemmatizer()
            self.lemmatizer.load_lemmatizer_model(pipeline.models[language]['lemmatizer'])
        if 'semantic_parser' in pipeline.models[language]:
            self.semantic_parser = pipeline.turbo_interface.create_semantic_parser()
            self.semantic_parser.load_semantic_parser_model(pipeline.models[language]['semantic_parser'])
        if 'coreference_resolver' in pipeline.models[language]:
            self.coreference_resolver = pipeline.turbo_interface.create_coreference_resolver()
            self.coreference_resolver.load_coreference_resolver_model(pipeline.models[language]['coreference_resolver'])


class NLPPipeline:
    def __init__(self):
        # Load the initialization file.
        configuration_filepath = os.path.dirname(os.path.realpath(__file__)) + \
            os.sep + 'nlp_pipeline.config'
        self.models = {}
        self.load_configuration_file(configuration_filepath)
        self.turbo_interface = tp.PTurboParser()
        self.workers = {}

    def load_configuration_file(self, filepath):
        f = open(filepath)
        language = ''
        for line in f:
            line = line.rstrip('\r\n')
            if line == '':
                language = ''
                continue
            # Ignore comments.
            index = line.find('#')
            if index >= 0:
                line = line[:index]
            line = line.strip()
            if line == '':
                continue
            if language == '':
                language = line
                print 'Loading information for %s' % language
                self.models[language] = {}
            else:
                pair = line.split('=')
                assert len(pair) == 2, pdb.set_trace()
                name = pair[0]
                value = pair[1].strip('"')
                self.models[language][name] = value
        f.close()

    def get_worker(self, language):
        if language in self.workers:
            return self.workers[language]
        else:
            worker = NLPPipelineWorker(self, language)
            self.workers[language] = worker
            return worker

    def split_sentences(self, text, language):
        worker = self.get_worker(language)
        sentences = worker.sent_tokenizer.tokenize(text)
        return sentences

    def tokenize(self, sentence, language):
        worker = self.get_worker(language)
        tokenized_sentence = worker.word_tokenizer.tokenize(sentence)
        return tokenized_sentence

    def tag(self, tokenized_sentence, language):
        worker = self.get_worker(language)
        sent = NLPSentence()
        sent['words'] = tokenized_sentence
        sent.compute_morphology(worker)
        tags = sent['tags']
        if sent['lemmas'] != None:
            lemmas = sent['lemmas']
        else:
            lemmas = ['_' for token in tokenized_sentence]
        if sent['morphological_tags'] != None:
            feats = ['|'.join(morph) if len(morph) > 0 else '_' \
                     for morph in sent['morphological_tags']]
        else:
            feats = ['_' for token in tokenized_sentence]
        return tags, lemmas, feats

    def recognize_entities(self, tokenized_sentence, tags, language):
        worker = self.get_worker(language)
        sent = NLPSentence()
        sent['words'] = tokenized_sentence
        sent['tags'] = tags
        sent.compute_entities(worker)
        entity_tags = sent['entity_tags']
        return entity_tags

    def parse(self, tokenized_sentence, tags, lemmas, language):
        worker = self.get_worker(language)
        sent = NLPSentence()
        sent['words'] = tokenized_sentence
        sent['tags'] = tags
        sent['lemmas'] = lemmas
        sent.compute_syntactic_dependencies(worker)
        # Convert to 1-based indexing for back-compatibility.
        #heads = [h+1 for h in sent['heads']]
        heads = sent['heads']
        deprels = sent['dependency_relations']
        return heads, deprels

    def has_morphological_tagger(self, language):
        worker = self.get_worker(language)
        return (worker.morphological_tagger != None)

    def has_entity_recognizer(self, language):
        worker = self.get_worker(language)
        return (worker.entity_recognizer != None)

    def has_semantic_parser(self, language):
        worker = self.get_worker(language)
        return (worker.semantic_parser != None)

    def has_coreference_resolver(self, language):
        worker = self.get_worker(language)
        return (worker.coreference_resolver != None)

    def parse_semantic_dependencies(self, tokenized_sentence, tags, lemmas,
                                    heads, deprels, language):
        worker = self.get_worker(language)
        sent = NLPSentence()
        sent['words'] = tokenized_sentence
        sent['tags'] = tags
        sent['lemmas'] = lemmas
        sent['heads'] = heads
        # Convert from 1-based indexing for back-compatibility.
        #sent['heads'] = [h-1 for h in heads]
        sent['dependency_relations'] = deprels
        sent.compute_semantic_dependencies(worker)
        num_predicates = len(sent['predicate_names'])
        predicates = ['_' for token in tokenized_sentence]
        argument_lists = [['_' for k in xrange(num_predicates)] \
                          for token in tokenized_sentence]
        for k in xrange(num_predicates):
            name = sent['predicate_names'][k]
            p = sent['predicate_indices'][k]
            predicates[p] = name
            for l in xrange(len(sent['argument_roles'][k])):
                role = sent['argument_roles'][k][l]
                a = sent['argument_indices'][k][l]
                argument_lists[a][k] = role
        return predicates, argument_lists

    def resolve_coreferences(self, all_tokenized_sentences, all_tags,
                             all_lemmas, all_heads, all_deprels,
                             all_entity_tags, language):
        worker = self.get_worker(language)
        sents = []
        for j, tokenized_sentence in enumerate(all_tokenized_sentences):
            sent = NLPSentence()
            sent['words'] = tokenized_sentence
            sent['tags'] = all_tags[j]
            sent['lemmas'] = all_lemmas[j]
            sent['heads'] = all_heads[j]
            # Convert from 1-based indexing for back-compatibility.
            #sent['heads'] = [h-1 for h in all_heads[j]]
            sent['dependency_relations'] = all_deprels[j]
            # For now, don't use this (must be coded as spans).
            sent['entity_tags'] = all_entity_tags[j]
            sents.append(sent)
        doc = NLPDocument(sents)
        doc.compute_coreferences(worker)

        # Convert from spans to coref info.
        all_coref_info = []
        for sent in doc['sentences']:
            spans = []
            for (start, end, name) in sent['coreference_spans']:
                span = Span(start, end, name)
                spans.append(span)
            coref_info = nlp_utils.construct_coreference_info_from_spans( \
                spans, len(sent['words']))
            all_coref_info.append(coref_info)

        return all_coref_info

    def parse_conll(self, text, language):
        sentences = self.split_sentences(text, language)
        conll_str = ''
        for sentence in sentences:
            tokenized_sentence = self.tokenize(sentence, language)
            tags, lemmas, feats = self.tag(tokenized_sentence, language)
            heads, deprels = self.parse(tokenized_sentence, tags, lemmas,
                                        language)
            for i, token in enumerate(tokenized_sentence):
                conll_str += str(i+1) + '\t' + token + '\t' + lemmas[i] + \
                             '\t' + tags[i] + '\t' + tags[i] + '\t' + \
                             feats[i] + '\t' + str(heads[i]+1) + '\t' + \
                             deprels[i] + '\n'
            conll_str += '\n'
        return conll_str
