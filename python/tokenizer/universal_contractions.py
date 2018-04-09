# -*- coding: utf-8 -*-

import sys
from contractions import Contractions
from spanish_contractions import SpanishContractions, SpanishAncoraContractions
from italian_contractions import ItalianContractions
from french_contractions import FrenchContractions
from portuguese_contractions import PortugueseContractions
from german_contractions import GermanContractions
from english_contractions import EnglishContractions

class UniversalContractions(object):
    def __init__(self, language):
        if language == 'es':
            contraction_splitter = SpanishContractions()
        elif language == 'es-ancora':
            contraction_splitter = SpanishAncoraContractions()
        elif language == 'it':
            contraction_splitter = ItalianContractions()
        elif language == 'fr':
            contraction_splitter = FrenchContractions()
        elif language == 'pt':
            contraction_splitter = PortugueseContractions()
        elif language == 'de':
            contraction_splitter = GermanContractions()
        elif language == 'en':
            contraction_splitter = EnglishContractions()
        else:
            print >> sys.stderr, \
                'No contraction splitter for language %s.' % language
            contraction_splitter = None
        self.language = language
        self.contraction_splitter = contraction_splitter

    def split_if_contraction(self, word):
        if self.contraction_splitter is None:
            return word
        else:
            return self.contraction_splitter.split_if_contraction(word)
