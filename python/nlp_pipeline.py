import nltk
import tokenizers.portuguese.word_tokenizer as tokenizer_PT
import lemmatizer
import turboparser as tp
import os
import pdb

class NLPPipelineWorker:
    def __init__(self, pipeline, language):
        self.tagger = None
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
        f_tagging = open('tagging.tmp', 'w')
        for token in tokenized_sentence:
            f_tagging.write(token + '\t_\n')
        f_tagging.close()
        worker.tagger.tag('tagging.tmp', 'tagging.tmp.pred')
        f_tagging_pred = open('tagging.tmp.pred')
        tags = []
        for line in f_tagging_pred:
            line = line.rstrip('\n')
            if line == '':
                continue
            fields = line.split('\t')
            tag = fields[1]
            tags.append(tag)
        f_tagging_pred.close()
        if worker.lemmatizer != None:
            lemmas = worker.lemmatizer.lemmatize_sentence(tokenized_sentence,
                                                          tags)
        else:
            lemmas = ['_' for token in tokenized_sentence]
        return tags, lemmas

    def recognize_entities(self, tokenized_sentence, tags, language):
        worker = self.get_worker(language)
        f_ner = open('ner.tmp', 'w')
        for i, token in enumerate(tokenized_sentence):
            tag = tags[i]
            f_ner.write(token + '\t' + tag + '\t' + '\t_\n')
        f_ner.close()
        worker.entity_recognizer.tag('ner.tmp', 'ner.tmp.pred')
        f_ner_pred = open('ner.tmp.pred')
        entity_tags = []
        for line in f_ner_pred:
            line = line.rstrip('\n')
            if line == '':
                continue
            fields = line.split('\t')
            entity_tag = fields[2]
            entity_tags.append(entity_tag)
        f_ner_pred.close()
        return entity_tags

    def parse(self, tokenized_sentence, tags, lemmas, language):
        worker = self.get_worker(language)
        f_conll = open('conll.tmp', 'w')
        for i, token in enumerate(tokenized_sentence):
            tag = tags[i]
            lemma = lemmas[i]
            f_conll.write(str(i+1) + '\t' + token + '\t' + lemma + '\t' +
                          tag + '\t' + tag + '\t_\t_\t_\n')
        f_conll.close()
        worker.parser.parse('conll.tmp', 'conll.tmp.pred')
        f_conll_pred = open('conll.tmp.pred')
        tags = []
        heads = []
        deprels = []
        for line in f_conll_pred:
            line = line.rstrip('\n')
            if line == '':
                continue
            fields = line.split('\t')
            lemma = fields[2]
            head = int(fields[6])
            deprel = fields[7]
            heads.append(head)
            deprels.append(deprel)
        f_conll_pred.close()
        return heads, deprels

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
        f_conll = open('conll2008.tmp', 'w')
        for i, token in enumerate(tokenized_sentence):
            tag = tags[i]
            lemma = lemmas[i]
            head = heads[i]
            deprel = deprels[i]
            f_conll.write(str(i+1) + '\t_\t_\t_\t_\t' + token + '\t' + lemma + \
                              '\t' + tag + '\t' + str(head) + '\t' + deprel + \
                              '\t_\n')
        f_conll.close()
        worker.semantic_parser.parse_semantic_dependencies('conll2008.tmp',
                                                           'conll2008.tmp.pred')
        f_conll_pred = open('conll2008.tmp.pred')
        predicates = []
        argument_lists = []
        for line in f_conll_pred:
            line = line.rstrip('\n')
            line = line.rstrip('\t')
            if line == '':
                continue
            fields = line.split('\t')
            predicate = fields[10]
            argument_list = fields[11:]
            predicates.append(predicate)
            argument_lists.append(argument_list)
        f_conll_pred.close()
        return predicates, argument_lists

    def resolve_coreferences(self, all_tokenized_sentences, all_tags,
                             all_lemmas, all_heads, all_deprels,
                             all_entity_tags, language):
        worker = self.get_worker(language)
        f_conll = open('conll.coref.tmp', 'w')
        f_conll.write('#begin document (_); part 000\n')
        for j, tokenized_sentence in enumerate(all_tokenized_sentences):
            tags = all_tags[j]
            lemmas = all_lemmas[j]
            heads = all_heads[j]
            deprels = all_deprels[j]
            # For now, don't use this (must be coded as spans).
            entity_tags = all_entity_tags[j]
            for i, token in enumerate(tokenized_sentence):
                tag = tags[i]
                lemma = lemmas[i]
                head = heads[i]
                deprel = deprels[i]
                f_conll.write('_\t0\t' + str(i+1) + '\t' + token + '\t' + \
                                  tag + '\t*\t' + str(head) + '\t' + deprel + \
                                  '\t-\t-\t-\t-\t*\t_\n')
            f_conll.write('\n')
        f_conll.write('#end document\n')
        f_conll.close()
        worker.coreference_resolver.resolve_coreferences('conll.coref.tmp',
                                                         'conll.coref.tmp.pred')
        f_conll_pred = open('conll.coref.tmp.pred')
        all_coref_info = [[] for j in xrange(len(all_tokenized_sentences))]
        j = 0
        for line in f_conll_pred:
            line = line.rstrip('\n')
            line = line.rstrip('\t')
            if line.startswith('#begin') or line.startswith('#end'):
                continue
            if line == '':
                j += 1
                continue
            fields = line.split('\t')
            coref = fields[-1]
            all_coref_info[j].append(coref)
        f_conll_pred.close()
        return all_coref_info

    def parse_conll(self, text, language):
        sentences = self.split_sentences(text, language)
        conll_str = ''
        for sentence in sentences:
            tokenized_sentence = self.tokenize(sentence, language)
            tags, lemmas = self.tag(tokenized_sentence, language)
            heads, deprels = self.parse(tokenized_sentence, tags, lemmas,
                                        language)
            for i, token in enumerate(tokenized_sentence):
                conll_str += str(i+1) + '\t' + token + '\t' + lemmas[i] + '\t' + tags[i] + '\t' + tags[i] + '\t_\t' + str(heads[i]) + '\t' + deprels[i] + '\n'
            conll_str += '\n'
        return conll_str
    
        
