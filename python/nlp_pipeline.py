import nltk
import tokenizers.portuguese.word_tokenizer as tokenizer_PT
import lemmatizer
import turboparser as tp
import pdb

class NLPPipelineWorker:
    def __init__(self, pipeline, language):
        self.tagger = pipeline.turbo_interface.create_tagger()
        self.parser = pipeline.turbo_interface.create_parser()
        self.lemmatizer = None
        if language == 'PT':
            self.sent_tokenizer = nltk.data.load('tokenizers/punkt/portuguese.pickle')
            self.word_tokenizer = tokenizer_PT.PortugueseFlorestaWordTokenizer()
            self.tagger.load_tagger_model('/home/atm/workspace/CPP/TurboParser/models/portuguese_floresta_v2.0_nomwe_auto/portuguese_floresta_v2.0_nomwe_auto_tagger.model')
            self.parser.load_parser_model('/home/atm/workspace/CPP/TurboParser/models/portuguese_floresta_v2.0_nomwe_auto/portuguese_floresta_v2.0_nomwe_auto_parser_pruned-true_model-standard.model')
            self.lemmatizer = lemmatizer.BasicLemmatizer()
            self.lemmatizer.load_lemmatizer_model('/home/atm/workspace/CPP/TurboParser/models/portuguese_floresta_v2.0_nomwe_auto/portuguese_floresta_v2.0_nomwe_auto_lemmatizer.model')
        elif language == 'PT-Cintil':
            self.sent_tokenizer = nltk.data.load('tokenizers/punkt/portuguese.pickle')
            self.word_tokenizer = tokenizer_PT.PortugueseCintilWordTokenizer()
            self.tagger.load_tagger_model('/home/atm/workspace/CPP/TurboParser/models/portuguese_cetem-depbank/portuguese_cetem-depbank_tagger.model')
            self.parser.load_parser_model('/home/atm/workspace/CPP/TurboParser/models/portuguese_cetem-depbank/portuguese_cetem-depbank_parser_pruned-true_model-standard.model')
        elif language == 'ES':
            self.sent_tokenizer = nltk.data.load('tokenizers/punkt/spanish.pickle')
            self.word_tokenizer = nltk.TreebankWordTokenizer() # For now...
            self.tagger.load_tagger_model('/home/atm/workspace/CPP/TurboParser/models/spanish_conll2009_v2.0_nomwe_auto/spanish_conll2009_v2.0_nomwe_auto_tagger.model')
            self.parser.load_parser_model('/home/atm/workspace/CPP/TurboParser/models/spanish_conll2009_v2.0_nomwe_auto/spanish_conll2009_v2.0_nomwe_auto_parser_pruned-true_model-standard.model')
            self.lemmatizer = lemmatizer.BasicLemmatizer()
            self.lemmatizer.load_lemmatizer_model('/home/atm/workspace/CPP/TurboParser/models/spanish_conll2009_v2.0_nomwe_auto/spanish_conll2009_v2.0_nomwe_auto_lemmatizer.model')
        elif language == 'EN':
            self.sent_tokenizer = nltk.data.load('tokenizers/punkt/english.pickle')
            self.word_tokenizer = nltk.TreebankWordTokenizer()
            self.tagger.load_tagger_model('/home/atm/workspace/CPP/TurboParser/models/english_proj/english_proj_tagger.model')
            self.parser.load_parser_model('/home/atm/workspace/CPP/TurboParser/models/english_proj/english_proj_parser_pruned-true_model-standard.model')
        else:
            raise NotImplementedError
        
class NLPPipeline:
    def __init__(self):
        self.turbo_interface = tp.PTurboParser()
        self.workers = {}
            
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
    
        
