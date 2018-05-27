# -*- coding: utf-8 -*-

import regex
from contractions import Contractions


class FrenchContractions(Contractions):
    def __init__(self):
        pass

    def split_if_contraction(self, word):
        # Handle preposition+determiner contractions.
        word = regex.sub(ur'^(A|a)u$', ur'à le', word)
        word = regex.sub(ur'^(A|a)uquel$', ur'à lequel', word)
        word = regex.sub(ur'^(A|a)ux$', ur'à les', word)
        word = regex.sub(ur'^(A|a)uxquels$', ur'à lesquels', word)
        word = regex.sub(ur'^(A|a)uxquelles$', ur'à lesquelles', word)
        word = regex.sub(ur'^(D|d)u$', ur'de le', word)
        word = regex.sub(ur'^(D|d)uquel$', ur'de lequel', word)
        word = regex.sub(ur'^(D|d)es$', ur'de les', word)
        word = regex.sub(ur'^(D|d)esquels$', ur'de lesquels', word)
        word = regex.sub(ur'^(D|d)esquelles$', ur'de lesquelles', word)

        return word
