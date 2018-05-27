# -*- coding: utf-8 -*-
import regex
from contractions import Contractions


class GermanContractions(Contractions):
    def __init__(self):
        pass

    def split_if_contraction(self, word):
        # From the CONLLu corpus
        word = regex.sub(ur'^(A|a)m$', ur'\1n dem', word)
        word = regex.sub(ur'^(A|a)ns$', ur'\1n das', word)
        word = regex.sub(ur'^(A|a)ufs$', ur'\1uf das', word)
        word = regex.sub(ur'^(B|b)eim$', ur'\1ei dem', word)
        word = regex.sub(ur'^(D|d)urchs$', ur'\1urch das', word)  # Not found in the CONLLu corpus.
        word = regex.sub(ur'^(F|f)ürs$', ur'\1ür das', word)
        word = regex.sub(ur'^(H|h)interm$', ur'\1inter dem', word)  # Not found in the CONLLu corpus.
        word = regex.sub(ur'^(I|i)m$', ur'\1n dem', word)
        word = regex.sub(ur'^(I|i)ns$', ur'\1n das', word)
        word = regex.sub(ur'^(Ü|ü)bers$', ur'\1ber das', word)
        word = regex.sub(ur'^(U|u)ms$', ur'\1m das', word)
        word = regex.sub(ur'^(U|u)nters$', ur'\1nter das', word)  # Not found in the CONLLu corpus.
        word = regex.sub(ur'^(U|u)nterm$', ur'\1nter dem', word)  # Not found in the CONLLu corpus.
        word = regex.sub(ur'^(V|v)om$', ur'\1on dem', word)
        word = regex.sub(ur'^(V|v)ors$', ur'\1or das', word)  # Not found in the CONLLu corpus.
        word = regex.sub(ur'^(V|v)orm$', ur'\1or dem', word)  # Not found in the CONLLu corpus.
        word = regex.sub(ur'^(Z|z)um$', ur'\1u dem', word)
        word = regex.sub(ur'^(Z|z)ur$', ur'\1u der', word)

        return word
