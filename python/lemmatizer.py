class BasicLemmatizer:
    def __init__(self):
        self.lemmas = {}

    def load_lemmatizer_model(self, file_model):
        self.lemmas = {}
        with open(file_model) as f:
            for line in f:
                line = line.rstrip('\n')
                fields = line.split('\t')
                self.lemmas[(fields[0], fields[1])] = fields[2]

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
        with open(file_test) as f, open(file_prediction, 'w') as f_out:
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
