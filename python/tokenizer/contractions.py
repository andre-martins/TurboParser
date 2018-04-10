# -*- coding: utf-8 -*-


class Contractions(object):
    def __init__(self):
        pass

    def split_if_contraction(self, word):
        raise NotImplementedError

    def split_contractions(self, tokenized_sentence):
        words = tokenized_sentence.split(' ')
        output_words = []
        for word in words:
            split_words = self.split_if_contraction(word).split(' ')
            output_words += split_words
        return ' '.join(output_words)
