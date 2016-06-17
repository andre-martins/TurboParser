# -*- coding: utf-8 -*-

from contractions import Contractions
import os
import sys
import regex

class SpanishContractions(Contractions):
    def __init__(self):
        # A blacklist of words that should not be confused with contractions.
        self.non_contractions = {} #{u'perla', u'perlas', u'arte', u'parte', \
                                 #u'aparte'}
        # A whitelist of frequent words that regexes are not getting but are
        # contractions.
        self.contractions = {}
        verbs = [] #[u'convencer', u'haber', u'hacer', u'meter', u'vender', \
                 #u'poner', u'tener', u'comer', u'mover', u'atender', \
                 #u'responder', u'devolver', u'dar']
        for verb in verbs:
            for suffix in [u'me', u'te', u'nos', u'os']:
                self.contractions[verb + suffix] = [verb, suffix]

        # Load Spanish verbs and their inflections from a lexicon.
        filepath = os.sep.join([os.path.dirname(os.path.realpath(__file__)),
                                'spanish_verbs.txt'])
        f = open(filepath)
        self.verbs = set()
        for line in f:
            fields = line.rstrip('\n').split()
            assert len(fields) == 3
            self.verbs.add(unicode(fields[0].decode('utf8')))
        f.close()

    def split_if_contraction(self, word):
        original_word = word

        # Handle preposition+determiner contractions.
        word = regex.sub(ur'^(A|a)l$', ur'\1 el', word)
        word = regex.sub(ur'^(D|d)el$', ur'\1e el', word)

        if original_word != word:
            return word

        # Before looking at clitic regexes, check if the word is in a blacklist.
        if word in self.non_contractions:
            return word

        # Before looking at clitic regexes, check if the word is in a whitelist.
        if word in self.contractions:
            return ' '.join(self.contractions[word])

        # Right now excludes capitalized words. Might fail if the word is in the
        # beginning of the sentences, but avoids catching a lot of proper nouns,
        # such as "Charles", "Bonaparte", etc.
        #if regex.search(ur'^[^\p{IsLower}]', word) is not None:
        #    return word

        # Handle clitics.
        # Note: removing "os", "me", "te" from this regex, since it catches a
        # lot of false positives which are plural nouns (e.g. "sombreros",
        # "firme", "muerte").
        #
        # Before this was:
        word = regex.sub( \
            ur'(ar|er|ir|ír|ndo)(me|te|se|nos|le|lo|la|les|los|las)$', \
            ur'\1 \2', word)
        #
        # Now we split this into 2 regexes, one more permissive and another
        # just for -er infinitives.
        # "nos" is excluded from "er" since it catches things like
        # 'internos', 'modernos', 'paternos', 'maternos', 'externos', ...
        #word = regex.sub( \
        #    ur'(ar|ir|ír|ndo)(me|te|se|nos|le|lo|la|les|los|las)$', \
        #    ur'\1 \2', word)
        #word = regex.sub( \
        #    ur'(er)(se|le|lo|la|les|los|las)$', \
        #    ur'\1 \2', word)
        word = regex.sub( \
            ur'(ár|ér|ír|ndo)(se)(me|te|nos|os|le|lo|la|les|los|las)$', \
            ur'\1 \2 \3', word)
        word = regex.sub(ur'(ár|ér|ír)(os)(le|lo|la|les|los|las)$', \
                         ur'\1 \2 \3', word)

        # If the first token is not a verb in the lexicon, put back the
        # original word. This is to avoid a lot of false positives that
        # are caught with the regexes above.
        if original_word != word:
            verb = word.split(' ')[0]
            verb = regex.sub(ur'á(ndo)$', ur'a\1', verb)
            verb = regex.sub(ur'é(ndo)$', ur'e\1', verb)
            verb = regex.sub(ur'í(ndo)$', ur'i\1', verb)
            if verb not in self.verbs:
                word = original_word
            elif original_word in self.verbs:
                word = original_word

        return word

class SpanishAncoraContractions(SpanishContractions):
    def __init__(self):
        SpanishContractions.__init__(self)

    def split_if_contraction(self, word):
        # Handle preposition+determiner contractions.
        word = regex.sub(ur'^(A|a)l$', ur'a el', word)
        word = regex.sub(ur'^(D|d)el$', ur'de el', word)

        # Before looking at clitic regexes, check if the word is in a blacklist.
        if word in self.non_contractions:
            return word

        # Before looking at clitic regexes, check if the word is in a whitelist.
        if word in self.contractions:
            return ' '.join(self.contractions[word])

        # Right now excludes capitalized words. Might fail if the word is in the
        # beginning of the sentences, but avoids catching a lot of proper nouns,
        # such as "Charles", "Bonaparte", etc.
        if regex.search(ur'^[^\p{IsLower}]', word) is not None:
            return word

        # Handle clitics.
        word = regex.sub( \
            ur'(ar|ir|ír)(me|te|se|nos|le|lo|la|les|los|las)$', \
            ur'\1 \2', word)
        word = regex.sub( \
            ur'(er)(se|le|lo|la|les|los|las)$', \
            ur'\1 \2', word)
        word = regex.sub( \
            ur'á(ndo)(me|te|se|nos|os|le|lo|la|les|los|las)$', \
            ur'a\1 \2', word)
        word = regex.sub( \
            ur'é(ndo)(me|te|se|nos|os|le|lo|la|les|los|las)$', \
                          ur'e\1 \2', word)
        word = regex.sub(ur'í(ndo)(me|te|se|nos|os|le|lo|la|les|los|las)$', \
                         ur'i\1 \2', word)
        word = regex.sub(ur'á(r|ndo)(se)(me|te|nos|os|le|lo|la|les|los|las)$', \
                         ur'a\1 \2 \3', word)
        word = regex.sub(ur'é(r|ndo)(se)(me|te|nos|os|le|lo|la|les|los|las)$', \
                         ur'e\1 \2 \3', word)
        word = regex.sub(ur'í(r|ndo)(se)(me|te|nos|os|le|lo|la|les|los|las)$', \
                         ur'i\1 \2 \3', word)
        word = regex.sub(ur'á(r)(os)(le|lo|la|les|los|las)$', \
                         ur'a\1 \2 \3', word)
        word = regex.sub(ur'é(r)(os)(le|lo|la|les|los|las)$', \
                         ur'e\1 \2 \3', word)
        word = regex.sub(ur'í(r)(os)(le|lo|la|les|los|las)$', \
                         ur'i\1 \2 \3', word)

        # In AnCora, all contractions have two words only.
        word = ' '.join(word.split(' ')[:2])
        return word
