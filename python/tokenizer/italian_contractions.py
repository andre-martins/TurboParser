# -*- coding: utf-8 -*-

import sys
import regex
import os
from contractions import Contractions

class ItalianContractions(Contractions):
    def __init__(self):
        # Load Italian verbs and their inflections from a lexicon.
        filepath = os.sep.join([os.path.dirname(os.path.realpath(__file__)),
                                'italian_verbs.txt'])
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
        word = regex.sub(ur'^([A|a])l$', ur'\1 il', word)
        word = regex.sub(ur'^([A|a])llo$', ur'\1 lo', word)
        word = regex.sub(ur'^([A|a])i$', ur'\1 i', word)
        word = regex.sub(ur'^([A|a])gli$', ur'\1 gli', word)
        word = regex.sub(ur'^([A|a])lla$', ur'\1 la', word)
        word = regex.sub(ur'^([A|a])lle$', ur'\1 le', word)
        word = regex.sub(ur'^([A|a])ll\'$', ur"\1 l'", word)

        word = regex.sub(ur'^([C|c])ol$', ur'\1on il', word)
        word = regex.sub(ur'^([C|c])oi$', ur'\1on i', word)
        word = regex.sub(ur'^([C|c])ogli$', ur'\1on gli', word)
        word = regex.sub(ur'^([C|c])olla$', ur'\1on la', word)
        word = regex.sub(ur'^([C|c])olle$', ur'\1on le', word)
        word = regex.sub(ur'^([C|c])oll\'$', ur"\1on l'", word) # Not very used.

        word = regex.sub(ur'^([D|d])al$', ur'\1a il', word)
        word = regex.sub(ur'^([D|d])allo$', ur'\1a lo', word)
        word = regex.sub(ur'^([D|d])ai$', ur'\1a i', word)
        word = regex.sub(ur'^([D|d])agli$', ur'\1a gli', word)
        word = regex.sub(ur'^([D|d])alla$', ur'\1a la', word)
        word = regex.sub(ur'^([D|d])alle$', ur'\1a le', word)
        word = regex.sub(ur'^([D|d])all\'$', ur"\1a l'", word)

        word = regex.sub(ur'^([D|d])el$', ur'\1i il', word)
        word = regex.sub(ur'^([D|d])ello$', ur'\1i lo', word)
        word = regex.sub(ur'^([D|d])ei$', ur'\1i i', word)
        word = regex.sub(ur'^([D|d])egli$', ur'\1i gli', word)
        word = regex.sub(ur'^([D|d])ella$', ur'\1i la', word)
        word = regex.sub(ur'^([D|d])elle$', ur'\1i le', word)
        word = regex.sub(ur'^([D|d])ell\'$', ur"\1i l'", word)
        word = regex.sub(ur'^([D|d])e\'$', ur'\1i i', word) # Not very used.

        word = regex.sub(ur'^Nel$', ur'In il', word)
        word = regex.sub(ur'^Nello$', ur'In lo', word)
        word = regex.sub(ur'^Nei$', ur'In i', word)
        word = regex.sub(ur'^Negli$', ur'In gli', word)
        word = regex.sub(ur'^Nella$', ur'In la', word)
        word = regex.sub(ur'^Nelle$', ur'In le', word)
        word = regex.sub(ur'^Nell\'$', ur"In l'", word)

        word = regex.sub(ur'^nel$', ur'in il', word)
        word = regex.sub(ur'^nello$', ur'in lo', word)
        word = regex.sub(ur'^nei$', ur'in i', word)
        word = regex.sub(ur'^negli$', ur'in gli', word)
        word = regex.sub(ur'^nella$', ur'in la', word)
        word = regex.sub(ur'^nelle$', ur'in le', word)
        word = regex.sub(ur'^nell\'$', ur"in l'", word)

        word = regex.sub(ur'^([P|p])el$', ur'\1er il', word) # Not used.
        word = regex.sub(ur'^([P|p])ei$', ur'\1er i', word) # Not used.

        word = regex.sub(ur'^([S|s])ul$', ur'\1u il', word)
        word = regex.sub(ur'^([S|s])ullo$', ur'\1u lo', word)
        word = regex.sub(ur'^([S|s])ui$', ur'\1u i', word)
        word = regex.sub(ur'^([S|s])ugli$', ur'\1u gli', word)
        word = regex.sub(ur'^([S|s])ulla$', ur'\1u la', word)
        word = regex.sub(ur'^([S|s])ulle$', ur'\1u le', word)
        word = regex.sub(ur'^([S|s])ull\'$', ur"\1u l'", word)

        if original_word != word:
            return word

        # Handle clitics.
        word = regex.sub(ur'^(ecco)(mi|ti|si|ci|vi|le|lo|la|li|ne|l\')$', \
                         ur'\1 \2', word)
        word = regex.sub(ur'^(glie)(le|lo|la|li|ne|l\')$', \
                         ur'\1 \2', word)
        # Removed "or", "ta", "to", "ti" below because it was catching a lot of
        # nouns (e.g. "aeroporti", "abitati", "cartone", "aristocratici").
        # Was:
        # word = regex.sub( \
        #    ur'(ar|er|ir|or|ur|ndo|ta|to|te|ti)' \
        #      '(mi|ti|si|ci|vi|gli|le|lo|la|li|ne)$', \
        #    ur'\1 \2', word)
        word = regex.sub( \
           ur'(ar|er|ir|ppor|^por|ur|ndo|te)' \
             '(mi|ti|si|ci|vi|gli|le|lo|la|li|ne)$', \
           ur'\1 \2', word)
        word = regex.sub( \
            ur'(ar|er|ir|or|ur|ndo|ta|to|te|ti)(me|te|se|ce|ve)'\
              '(gli|le|lo|la|li|ne)$', \
            ur'\1 \2 \3', word)
        word = regex.sub(ur'(ar|er|ir|or|ur|ndo)(glie)(le|lo|la|li|ne)$', \
                         ur'\1 \2 \3', word)

        # If the first token is not a verb in the lexicon, put back the
        # original word. This is to avoid a lot of false positives that
        # are caught with the regexes above.
        if original_word != word:
            if word.split(' ')[0] not in self.verbs:
                word = original_word
            elif original_word in self.verbs:
                word = original_word

        return word
