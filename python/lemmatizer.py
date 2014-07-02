import pdb

class BasicLemmatizer:
    def __init__(self):
        self.lemmas = {}

    def load_lemmatizer_model(self, file_model):
        self.lemmas = {}
        f = open(file_model)
        for line in f:
            line = line.rstrip('\n')
            fields = line.split('\t')
            self.lemmas[(fields[0], fields[1])] = fields[2]
        f.close()

    def lemmatize_sentence(self, tokenized_sentence, tags):
        lemmas = []
        for word, tag in zip(tokenized_sentence, tags):
            if (word, tag) in self.lemmas:
                lemma = self.lemmas[(word, tag)]
            else:
                lemma = word
            lemmas.append(lemma)
        return lemmas

    def lemmatize(self, file_test, file_prediction):
        f = open(file_test)
        f_out = open(file_prediction, 'w')
        for line in f:
            line = line.rstrip('\n')
            if line == '':
                f_out.write(line + '\n')
                continue
            elif line.startswith('#begin document'):
                f_out.write(line + '\n')
                continue
            elif line.startswith('#end document'):
                f_out.write(line + '\n')
                continue

            fields = line.split()
            word = fields[1]
            tag = fields[3]
            if (word, tag) in self.lemmas:
                lemma = self.lemmas[(word, tag)]
            else:
                lemma = word
            fields_out = fields
            fields_out[2] = lemma
            f_out.write('\t'.join(fields_out) + '\n')

        f.close()
        f_out.close()
