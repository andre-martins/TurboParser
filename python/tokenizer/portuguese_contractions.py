# -*- coding: utf-8 -*-

import regex
from contractions import Contractions


class PortugueseContractions(Contractions):
    def __init__(self):
        # A blacklist of words that should not be confused with contractions.
        # If True, mark consonants removed due to enclitics with symbols # and
        # -CL- for mesoclitics.
        self.mark_enclitics = False
        self.non_contractions = {}
        self.contractions = self._generate_contractions()
        self.clitics, self.clitic_suffixes = self._generate_clitics()

    def _generate_contractions(self):
        """
        Generate contractions for Portuguese, along with the words
        and lemmas that are contracted (e.g. contraction "das" is composed by
        words "de" + "as", with corresponding lemmas "de" + "o".
        Return a dictionary of contractions, each entry containing a list of
        words and a list of lemmas (typically lists of length two).

        :rtype: dictionary of lists.
        """
        contractions = {}

        contractions[u'ao'] = [u'a', u'o'], [u'a', u'o']
        contractions[u'à'] = [u'a', u'a'], [u'a', u'o']
        contractions[u'aos'] = [u'a', u'os'], [u'a', u'o']
        contractions[u'às'] = [u'a', u'as'], [u'a', u'o']
        contractions[u'àquele'] = [u'a', u'aquele'], [u'a', u'aquele']
        contractions[u'àquela'] = [u'a', u'aquela'], [u'a', u'aquele']
        contractions[u'àqueles'] = [u'a', u'aqueles'], [u'a', u'aquele']
        contractions[u'àquelas'] = [u'a', u'aquelas'], [u'a', u'aquele']
        contractions[u'àquilo'] = [u'a', u'aquilo'], [u'a', u'aquilo']
        contractions[u'aonde'] = [u'a', u'onde'], [u'a', u'onde']
        contractions[u'àqueloutro'] = [u'a', u'aqueloutro'], \
                                      [u'a', u'aqueloutro']
        contractions[u'àqueloutra'] = [u'a', u'aqueloutra'], \
                                      [u'a', u'aqueloutro']
        contractions[u'àqueloutros'] = [u'a', u'aqueloutros'], \
                                       [u'a', u'aqueloutro']
        contractions[u'àqueloutras'] = [u'a', u'aqueloutras'], \
                                       [u'a', u'aqueloutro']

        contractions[u'do'] = [u'de', u'o'], [u'de', u'o']
        contractions[u'da'] = [u'de', u'a'], [u'de', u'o']
        contractions[u'dos'] = [u'de', u'os'], [u'de', u'o']
        contractions[u'das'] = [u'de', u'as'], [u'de', u'o']
        contractions[u'desse'] = [u'de', u'esse'], [u'de', u'esse']
        contractions[u'dessa'] = [u'de', u'essa'], [u'de', u'esse']
        contractions[u'desses'] = [u'de', u'esses'], [u'de', u'esse']
        contractions[u'dessas'] = [u'de', u'essas'], [u'de', u'esse']
        contractions[u'disso'] = [u'de', u'isso'], [u'de', u'isso']
        contractions[u'deste'] = [u'de', u'este'], [u'de', u'este']
        contractions[u'desta'] = [u'de', u'esta'], [u'de', u'este']
        contractions[u'destes'] = [u'de', u'estes'], [u'de', u'este']
        contractions[u'destas'] = [u'de', u'estas'], [u'de', u'este']
        contractions[u'disto'] = [u'de', u'isto'], [u'de', u'isto']
        contractions[u'daquele'] = [u'de', u'aquele'], [u'de', u'aquele']
        contractions[u'daquela'] = [u'de', u'aquela'], [u'de', u'aquele']
        contractions[u'daqueles'] = [u'de', u'aqueles'], [u'de', u'aquele']
        contractions[u'daquelas'] = [u'de', u'aquelas'], [u'de', u'aquele']
        contractions[u'daquilo'] = [u'de', u'aquilo'], [u'de', u'aquilo']
        contractions[u'dele'] = [u'de', u'ele'], [u'de', u'ele']
        contractions[u'dela'] = [u'de', u'ela'], [u'de', u'ele']
        contractions[u'deles'] = [u'de', u'eles'], [u'de', u'ele']
        contractions[u'delas'] = [u'de', u'elas'], [u'de', u'ele']
        contractions[u'dum'] = [u'de', u'um'], [u'de', u'um']
        contractions[u'duma'] = [u'de', u'uma'], [u'de', u'um']
        contractions[u'duns'] = [u'de', u'uns'], [u'de', u'um']
        contractions[u'dumas'] = [u'de', u'umas'], [u'de', u'um']
        contractions[u'doutro'] = [u'de', u'outro'], [u'de', u'outro']
        contractions[u'doutra'] = [u'de', u'outra'], [u'de', u'outro']
        contractions[u'doutros'] = [u'de', u'outros'], [u'de', u'outro']
        contractions[u'doutras'] = [u'de', u'outras'], [u'de', u'outro']
        contractions[u'daqueloutro'] = [u'de', u'aqueloutro'], \
                                       [u'de', u'aqueloutro']
        contractions[u'daqueloutra'] = [u'de', u'aqueloutra'], \
                                       [u'de', u'aqueloutro']
        contractions[u'daqueloutros'] = [u'de', u'aqueloutros'], \
                                        [u'de', u'aqueloutro']
        contractions[u'daqueloutras'] = [u'de', u'aqueloutras'], \
                                        [u'de', u'aqueloutro']
        contractions[u'dessoutro'] = [u'de', u'essoutro'], [u'de', u'essoutro']
        contractions[u'dessoutra'] = [u'de', u'essoutra'], [u'de', u'essoutro']
        contractions[u'dessoutros'] = [u'de', u'essoutros'], \
                                      [u'de', u'essoutro']
        contractions[u'dessoutras'] = [u'de', u'essoutras'], \
                                      [u'de', u'essoutro']
        contractions[u'destoutro'] = [u'de', u'estoutro'], [u'de', u'estoutro']
        contractions[u'destoutra'] = [u'de', u'estoutra'], [u'de', u'estoutro']
        contractions[u'destoutros'] = [u'de', u'estoutros'], \
                                      [u'de', u'estoutro']
        contractions[u'destoutras'] = [u'de', u'estoutras'], \
                                      [u'de', u'estoutro']
        contractions[u'dalgum'] = [u'de', u'algum'], [u'de', u'algum']
        contractions[u'dalguma'] = [u'de', u'alguma'], [u'de', u'algum']
        contractions[u'dalguns'] = [u'de', u'alguns'], [u'de', u'algum']
        contractions[u'dalgumas'] = [u'de', u'algumas'], [u'de', u'algum']
        contractions[u'dalguém'] = [u'de', u'alguém'], [u'de', u'alguém']
        contractions[u'dalgo'] = [u'de', u'algo'], [u'de', u'algo']
        contractions[u'dacolá'] = [u'de', u'acolá'], [u'de', u'acolá']
        contractions[u'dalgures'] = [u'de', u'algures'], [u'de', u'algures']
        contractions[u'dalhures'] = [u'de', u'alhures'], [u'de', u'alhures']
        contractions[u'dali'] = [u'de', u'ali'], [u'de', u'ali']
        contractions[u'daqui'] = [u'de', u'aqui'], [u'de', u'aqui']
        contractions[u'dentre'] = [u'de', u'entre'], [u'de', u'entre']
        contractions[u'donde'] = [u'de', u'onde'], [u'de', u'onde']
        contractions[u'doutrem'] = [u'de', u'outrem'], [u'de', u'outrem']
        contractions[u'doutrora'] = [u'de', u'outrora'], [u'de', u'outrora']
        contractions[u"d'el-rei"] = [u'de', u'el-rei'], [u'de', u'el-rei']

        contractions[u'no'] = [u'em', u'o'], [u'em', u'o']
        contractions[u'na'] = [u'em', u'a'], [u'em', u'o']
        contractions[u'nos'] = [u'em', u'os'], [u'em', u'o']
        contractions[u'nas'] = [u'em', u'as'], [u'em', u'o']
        contractions[u'nesse'] = [u'em', u'esse'], [u'em', u'esse']
        contractions[u'nessa'] = [u'em', u'essa'], [u'em', u'esse']
        contractions[u'nesses'] = [u'em', u'esses'], [u'em', u'esse']
        contractions[u'nessas'] = [u'em', u'essas'], [u'em', u'esse']
        contractions[u'nisso'] = [u'em', u'isso'], [u'em', u'isso']
        contractions[u'neste'] = [u'em', u'este'], [u'em', u'este']
        contractions[u'nesta'] = [u'em', u'esta'], [u'em', u'este']
        contractions[u'nestes'] = [u'em', u'estes'], [u'em', u'este']
        contractions[u'nestas'] = [u'em', u'estas'], [u'em', u'este']
        contractions[u'nisto'] = [u'em', u'isto'], [u'em', u'isto']
        contractions[u'naquele'] = [u'em', u'aquele'], [u'em', u'aquele']
        contractions[u'naquela'] = [u'em', u'aquela'], [u'em', u'aquele']
        contractions[u'naqueles'] = [u'em', u'aqueles'], [u'em', u'aquele']
        contractions[u'naquelas'] = [u'em', u'aquelas'], [u'em', u'aquele']
        contractions[u'naquilo'] = [u'em', u'aquilo'], [u'em', u'aquilo']
        contractions[u'nele'] = [u'em', u'ele'], [u'em', u'ele']
        contractions[u'nela'] = [u'em', u'ela'], [u'em', u'ele']
        contractions[u'neles'] = [u'em', u'eles'], [u'em', u'ele']
        contractions[u'nelas'] = [u'em', u'elas'], [u'em', u'ele']
        contractions[u'num'] = [u'em', u'um'], [u'em', u'um']
        contractions[u'numa'] = [u'em', u'uma'], [u'em', u'um']
        contractions[u'nuns'] = [u'em', u'uns'], [u'em', u'um']
        contractions[u'numas'] = [u'em', u'umas'], [u'em', u'um']
        contractions[u'noutro'] = [u'em', u'outro'], [u'em', u'outro']
        contractions[u'noutra'] = [u'em', u'outra'], [u'em', u'outro']
        contractions[u'noutros'] = [u'em', u'outros'], [u'em', u'outro']
        contractions[u'noutras'] = [u'em', u'outras'], [u'em', u'outro']
        contractions[u'naqueloutro'] = [u'em', u'aqueloutro'], \
                                       [u'em', u'aqueloutro']
        contractions[u'naqueloutra'] = [u'em', u'aqueloutra'], \
                                       [u'em', u'aqueloutro']
        contractions[u'naqueloutros'] = [u'em', u'aqueloutros'], \
                                        [u'em', u'aqueloutro']
        contractions[u'naqueloutras'] = [u'em', u'aqueloutras'], \
                                        [u'em', u'aqueloutro']
        contractions[u'nessoutro'] = [u'em', u'essoutro'], [u'em', u'essoutro']
        contractions[u'nessoutra'] = [u'em', u'essoutra'], [u'em', u'essoutro']
        contractions[u'nessoutros'] = [u'em', u'essoutros'], \
                                      [u'em', u'essoutro']
        contractions[u'nessoutras'] = [u'em', u'essoutras'], \
                                      [u'em', u'essoutro']
        contractions[u'nestoutro'] = [u'em', u'estoutro'], [u'em', u'estoutro']
        contractions[u'nestoutra'] = [u'em', u'estoutra'], [u'em', u'estoutro']
        contractions[u'nestoutros'] = [u'em', u'estoutros'], \
                                      [u'em', u'estoutro']
        contractions[u'nestoutras'] = [u'em', u'estoutras'], \
                                      [u'em', u'estoutro']
        contractions[u'nalgum'] = [u'em', u'algum'], [u'em', u'algum']
        contractions[u'nalguma'] = [u'em', u'alguma'], [u'em', u'algum']
        contractions[u'nalguns'] = [u'em', u'alguns'], [u'em', u'algum']
        contractions[u'nalgumas'] = [u'em', u'algumas'], [u'em', u'algum']
        contractions[u'nalguém'] = [u'em', u'alguém'], [u'em', u'alguém']
        contractions[u'nalgo'] = [u'em', u'algo'], [u'em', u'algo']
        contractions[u'noutrem'] = [u'em', u'outrem'], [u'em', u'outrem']

        contractions[u'pelo'] = [u'por', u'o'], [u'por', u'o']
        contractions[u'pela'] = [u'por', u'a'], [u'por', u'o']
        contractions[u'pelos'] = [u'por', u'os'], [u'por', u'o']
        contractions[u'pelas'] = [u'por', u'as'], [u'por', u'o']

        contractions[u'hei-de'] = [u'hei', u'de'], [u'haver', u'de']
        contractions[u'há-de'] = [u'há', u'de'], [u'haver', u'de']
        contractions[u'hás-de'] = [u'hás', u'de'], [u'haver', u'de']

        # Add upper cases.
        contraction_toks = contractions.keys()
        for tok in contraction_toks:
            first_char = tok[0]
            if first_char.islower():
                # NOTE: tok.title() fails when some character is not ASCII
                # (e.g. Dalguém).
                upper_tok = tok[0].upper() + tok[1:]
            else:
                upper_tok = regex.sub(ur'^à', ur'À', tok)
            words, lemmas = contractions[tok]
            upper_words = []
            upper_lemmas = []
            for word, lemma in zip(words, lemmas):
                if len(upper_words) == 0:
                    upper_words.append(word.title())
                else:
                    upper_words.append(word[:])
                upper_lemmas.append(lemma[:])
            contractions[upper_tok] = upper_words, upper_lemmas

        return contractions

    def _generate_clitics(self):
        """
        Generate clitics (e.g. '-se') and verb suffixes (e.g. '-ei') for
        Portuguese.

        :rtype: a list of suffixes and a list of clitics.
        """
        import numpy as np

        suffixes = set()
        clitics = set()

        # Suffixos dos verbos em mesoclíticos.
        suffixes.add(u'-ei')
        suffixes.add(u'-ás')
        suffixes.add(u'-á')
        suffixes.add(u'-emos')
        suffixes.add(u'-eis')
        suffixes.add(u'-ão')
        suffixes.add(u'-ia')
        suffixes.add(u'-ias')
        suffixes.add(u'-íamos')
        suffixes.add(u'-íeis')
        suffixes.add(u'-iam')

        # Nota: com -lo, -la, -los, -las, há sempre um r,s,z que cai.
        # Com -no, -na, -nos, -nas, quase nunca cai ("dizem-nos"), a não ser
        # quando o verbo termina em "mos" ("lavamo-nos").
        clitics.add(u'-lo')
        clitics.add(u'-la')
        clitics.add(u'-los')
        clitics.add(u'-las')

        clitics.add(u'-na')
        clitics.add(u'-nas')
        clitics.add(u'-me')
        clitics.add(u'-te')
        clitics.add(u'-no')
        clitics.add(u'-nos')
        clitics.add(u'-vos')
        clitics.add(u'-o')
        clitics.add(u'-os')
        clitics.add(u'-a')
        clitics.add(u'-as')
        clitics.add(u'-se')
        clitics.add(u'-lhe')
        clitics.add(u'-lhes')
        clitics.add(u'-mo')
        clitics.add(u'-mos')
        clitics.add(u'-ma')
        clitics.add(u'-mas')
        clitics.add(u'-to')
        clitics.add(u'-tos')
        clitics.add(u'-ta')
        clitics.add(u'-tas')

        clitics.add(u'-no-lo')
        clitics.add(u'-no-los')
        clitics.add(u'-no-la')
        clitics.add(u'-no-las')
        clitics.add(u'-vo-lo')
        clitics.add(u'-vo-los')
        clitics.add(u'-vo-la')
        clitics.add(u'-vo-las')
        clitics.add(u'-lho')
        clitics.add(u'-lhos')
        clitics.add(u'-lha')
        clitics.add(u'-lhas')
        clitics.add(u'-se-me')
        clitics.add(u'-se-te')
        clitics.add(u'-se-nos')
        clitics.add(u'-se-vos')
        clitics.add(u'-se-lhe')
        clitics.add(u'-se-lhes')
        clitics.add(u'-se-mo')
        clitics.add(u'-se-mos')
        clitics.add(u'-se-ma')
        clitics.add(u'-se-mas')
        clitics.add(u'-se-to')
        clitics.add(u'-se-tos')
        clitics.add(u'-se-ta')
        clitics.add(u'-se-tas')

        clitics.add(u'-se-no-lo')
        clitics.add(u'-se-no-los')
        clitics.add(u'-se-no-la')
        clitics.add(u'-se-no-las')
        clitics.add(u'-se-vo-lo')
        clitics.add(u'-se-vo-los')
        clitics.add(u'-se-vo-la')
        clitics.add(u'-se-vo-las')
        clitics.add(u'-se-lho')
        clitics.add(u'-se-lhos')
        clitics.add(u'-se-lha')
        clitics.add(u'-se-lhas')

        # Sort lists by length so that, if one suffix/clitic is a suffix of
        # another, we always check the longest one first.
        clitic_list = list(clitics)
        clitic_inds = list(np.argsort([len(clitic) for clitic in clitic_list]))
        clitic_inds.reverse()

        suffix_list = list(suffixes)
        suffix_inds = list(np.argsort([len(suffix) for suffix in suffix_list]))
        suffix_inds.reverse()

        final_clitic_list = [clitic_list[i] for i in clitic_inds]
        final_suffix_list = [suffix_list[i] for i in suffix_inds]

        return final_clitic_list, final_suffix_list

    def split_if_contraction(self, word):
        original_word = word

        # Handle preposition+determiner contractions.
        if word in self.contractions:
            return ' '.join(self.contractions[word][0])

        # Before looking at clitic regexes, check if the word is in a blacklist.
        if word in self.non_contractions:
            return word

        # Handle clitics. The output is consistent with the
        # Portuguese CINTIL corpus (e.g.: "fá-lo-ei" ->  "fá#-CL-ei" + "-lo").
        new_tok = word
        suffix_tok = ''
        for suffix in self.clitic_suffixes:
            if word.endswith(suffix):
                suffix_tok = word[-len(suffix):]
                new_tok = word[:-len(suffix)]
                break
        clitic_tok = ''
        for clitic in self.clitics:
            if new_tok.endswith(clitic):
                clitic_tok = new_tok[-len(clitic):]
                new_tok = new_tok[:-len(clitic)]
                break
        if clitic_tok != '':
            if clitic_tok in [u'-lo', u'-la', u'-los', u'-las']:
                if self.mark_enclitics:
                    new_tok += u'#'
            if suffix_tok != '':
                if self.mark_enclitics:
                    new_tok += u'-CL' + suffix_tok
                elif suffix_tok.startswith(u'-'):
                    new_tok += suffix_tok[1:]
                else:
                    new_tok += suffix_tok
            word = ' '.join([new_tok, clitic_tok])

        return word
