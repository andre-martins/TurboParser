# -*- coding: utf-8 -*-

import regex
from contractions import Contractions


class EnglishContractions(Contractions):
    def __init__(self):
        # List of contractions adapted from Robert MacIntyre's tokenizer.
        # These were in turn collected from the TreebankWordTokenizer in NLTK.
        self.CONTRACTIONS = [regex.compile(r"([^' ])('[sS]|'[mM]|'[dD]|')\b"),
                             regex.compile(
                                 r"([^' ])('ll|'LL|'re|'RE|'ve|'VE|n't|N'T)\b")]
        self.CONTRACTIONS2 = [regex.compile(r"(?i)\b(can)(not)\b"),
                              regex.compile(r"(?i)\b(d)('ye)\b"),
                              regex.compile(r"(?i)\b(gim)(me)\b"),
                              regex.compile(r"(?i)\b(gon)(na)\b"),
                              regex.compile(r"(?i)\b(got)(ta)\b"),
                              regex.compile(r"(?i)\b(lem)(me)\b"),
                              regex.compile(r"(?i)\b(mor)('n)\b"),
                              regex.compile(r"(?i)\b(wan)(na) ")]
        self.CONTRACTIONS3 = [regex.compile(r"(?i) ('t)(is)\b"),
                              regex.compile(r"(?i) ('t)(was)\b")]
        self.CONTRACTIONS4 = [regex.compile(r"(?i)\b(whad)(dd)(ya)\b"),
                              regex.compile(r"(?i)\b(wha)(t)(cha)\b")]

    def split_if_contraction(self, word):
        for regexp in self.CONTRACTIONS:
            word = regexp.sub(r'\1 \2', word)
        for regexp in self.CONTRACTIONS2:
            word = regexp.sub(r'\1 \2', word)
        for regexp in self.CONTRACTIONS3:
            word = regexp.sub(r'\1 \2', word)

        # We are not using CONTRACTIONS4 since
        # they are also commented out in the SED scripts
        # for regexp in self.CONTRACTIONS4:
        #     word = regexp.sub(r'\1 \2 \3', word)

        return word
