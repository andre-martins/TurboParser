# -*- coding: utf-8 -*-
"""
Copyright (c) 2012-2013 Andre Martins <atm@priberam.pt> 
All Rights Reserved.

This file is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.
"""

import re

def split_contractions(sentence, contractions, use_cintil_format=True):
    """
    Split clitics from the verb form (in Portuguese).
    If use_cintil_format is True, the output is consistent with the 
    Portuguese CINTIL corpus (e.g.: "dos" ->  "de_" + "os").

    :rtype: string with tokens separated by whitespaces.
    """
    toks = sentence.split(' ')
    new_toks = []
    for tok in toks:
        if tok in contractions:
            words, _ = contractions[tok]
            words = words[:]
            if use_cintil_format:
                words[0] += '_'
            for word in words:
                new_toks.append(word)
        else:
            new_toks.append(tok)
    return ' '.join(new_toks)


def generate_contractions():
    """
    Generate contractions for Portuguese, along with the words
    and lemmas that are contracted (e.g. contraction "das" is composed by 
    words "de" + "as", with corresponding lemmas "de" + "o".
    Return a dictionary of contractions, each entry containing a list of 
    words and a list of lemmas (typically lists of length two).
    
    :rtype: dictionary of lists.
    """
    contractions = {}
    
    contractions['ao'] = ['a', 'o'], ['a', 'o']
    contractions['à'] = ['a', 'a'], ['a', 'o']
    contractions['aos'] = ['a', 'os'], ['a', 'o']
    contractions['às'] = ['a', 'as'], ['a', 'o']
    contractions['àquele'] = ['a', 'aquele'], ['a', 'aquele']
    contractions['àquela'] = ['a', 'aquela'], ['a', 'aquele']
    contractions['àqueles'] = ['a', 'aqueles'], ['a', 'aquele']
    contractions['àquelas'] = ['a', 'aquelas'], ['a', 'aquele']
    contractions['àquilo'] = ['a', 'aquilo'], ['a', 'aquilo']
    contractions['aonde'] = ['a', 'onde'], ['a', 'onde']
    contractions['àqueloutro'] = ['a', 'aqueloutro'], ['a', 'aqueloutro']
    contractions['àqueloutra'] = ['a', 'aqueloutra'], ['a', 'aqueloutro']
    contractions['àqueloutros'] = ['a', 'aqueloutros'], ['a', 'aqueloutro']
    contractions['àqueloutras'] = ['a', 'aqueloutras'], ['a', 'aqueloutro']

    contractions['do'] = ['de', 'o'], ['de', 'o']
    contractions['da'] = ['de', 'a'], ['de', 'o']
    contractions['dos'] = ['de', 'os'], ['de', 'o']
    contractions['das'] = ['de', 'as'], ['de', 'o']
    contractions['desse'] = ['de', 'esse'], ['de', 'esse']
    contractions['dessa'] = ['de', 'essa'], ['de', 'esse']
    contractions['desses'] = ['de', 'esses'], ['de', 'esse']
    contractions['dessas'] = ['de', 'essas'], ['de', 'esse']
    contractions['disso'] = ['de', 'isso'], ['de', 'isso']
    contractions['deste'] = ['de', 'este'], ['de', 'este']
    contractions['desta'] = ['de', 'esta'], ['de', 'este']
    contractions['destes'] = ['de', 'estes'], ['de', 'este']
    contractions['destas'] = ['de', 'estas'], ['de', 'este']
    contractions['disto'] = ['de', 'isto'], ['de', 'isto']
    contractions['daquele'] = ['de', 'aquele'], ['de', 'aquele'] 
    contractions['daquela'] = ['de', 'aquela'], ['de', 'aquele']
    contractions['daqueles'] = ['de', 'aqueles'], ['de', 'aquele']
    contractions['daquelas'] = ['de', 'aquelas'], ['de', 'aquele']    
    contractions['daquilo'] = ['de', 'aquilo'], ['de', 'aquilo']    
    contractions['dele'] = ['de', 'ele'],['de', 'ele']
    contractions['dela'] = ['de', 'ela'], ['de', 'ele']
    contractions['deles'] = ['de', 'eles'], ['de', 'ele']
    contractions['delas'] = ['de', 'elas'], ['de', 'ele']
    contractions['dum'] = ['de', 'um'], ['de', 'um']
    contractions['duma'] = ['de', 'uma'], ['de', 'um']
    contractions['duns'] = ['de', 'uns'], ['de', 'um']
    contractions['dumas'] = ['de', 'umas'], ['de', 'um']    
    contractions['doutro'] = ['de', 'outro'], ['de', 'outro']
    contractions['doutra'] = ['de', 'outra'], ['de', 'outro']
    contractions['doutros'] = ['de', 'outros'], ['de', 'outro']
    contractions['doutras'] = ['de', 'outras'], ['de', 'outro']  
    contractions['daqueloutro'] = ['de', 'aqueloutro'], ['de', 'aqueloutro'] 
    contractions['daqueloutra'] = ['de', 'aqueloutra'], ['de', 'aqueloutro']
    contractions['daqueloutros'] = ['de', 'aqueloutros'], ['de', 'aqueloutro']
    contractions['daqueloutras'] = ['de', 'aqueloutras'], ['de', 'aqueloutro']    
    contractions['dessoutro'] = ['de', 'essoutro'], ['de', 'essoutro']
    contractions['dessoutra'] = ['de', 'essoutra'], ['de', 'essoutro']
    contractions['dessoutros'] = ['de', 'essoutros'], ['de', 'essoutro']
    contractions['dessoutras'] = ['de', 'essoutras'], ['de', 'essoutro']    
    contractions['destoutro'] = ['de', 'estoutro'], ['de', 'estoutro']
    contractions['destoutra'] = ['de', 'estoutra'], ['de', 'estoutro']
    contractions['destoutros'] = ['de', 'estoutros'], ['de', 'estoutro']
    contractions['destoutras'] = ['de', 'estoutras'], ['de', 'estoutro']    
    contractions['dalgum'] = ['de', 'algum'], ['de', 'algum']
    contractions['dalguma'] = ['de', 'alguma'], ['de', 'algum']
    contractions['dalguns'] = ['de', 'alguns'], ['de', 'algum']
    contractions['dalgumas'] = ['de', 'algumas'], ['de', 'algum']
    contractions['dalguém'] = ['de', 'alguém'], ['de', 'alguém']
    contractions['dalgo'] = ['de', 'algo'], ['de', 'algo']
    contractions['dacolá'] = ['de', 'acolá'], ['de', 'acolá']
    contractions['dalgures'] = ['de', 'algures'], ['de', 'algures']
    contractions['dalhures'] = ['de', 'alhures'], ['de', 'alhures']
    contractions['dali'] = ['de', 'ali'], ['de', 'ali']
    contractions['daqui'] = ['de', 'aqui'], ['de', 'aqui']
    contractions['dentre'] = ['de', 'entre'], ['de', 'entre']
    contractions['donde'] = ['de', 'onde'], ['de', 'onde']
    contractions['doutrem'] = ['de', 'outrem'], ['de', 'outrem']
    contractions['doutrora'] = ['de', 'outrora'], ['de', 'outrora']
    contractions["d'el-rei"] = ['de', 'el-rei'], ['de', 'el-rei']

    contractions['no'] = ['em', 'o'], ['em', 'o']
    contractions['na'] = ['em', 'a'], ['em', 'o']
    contractions['nos'] = ['em', 'os'], ['em', 'o']
    contractions['nas'] = ['em', 'as'], ['em', 'o']
    contractions['nesse'] = ['em', 'esse'], ['em', 'esse']
    contractions['nessa'] = ['em', 'essa'], ['em', 'esse']
    contractions['nesses'] = ['em', 'esses'], ['em', 'esse']
    contractions['nessas'] = ['em', 'essas'], ['em', 'esse']
    contractions['nisso'] = ['em', 'isso'], ['em', 'isso']
    contractions['neste'] = ['em', 'este'], ['em', 'este']
    contractions['nesta'] = ['em', 'esta'], ['em', 'este']
    contractions['nestes'] = ['em', 'estes'], ['em', 'este']
    contractions['nestas'] = ['em', 'estas'], ['em', 'este']
    contractions['nisto'] = ['em', 'isto'], ['em', 'isto']
    contractions['naquele'] = ['em', 'aquele'], ['em', 'aquele']
    contractions['naquela'] = ['em', 'aquela'], ['em', 'aquele']
    contractions['naqueles'] = ['em', 'aqueles'], ['em', 'aquele']
    contractions['naquelas'] = ['em', 'aquelas'], ['em', 'aquele']
    contractions['naquilo'] = ['em', 'aquilo'], ['em', 'aquilo']
    contractions['nele'] = ['em', 'ele'], ['em', 'ele']
    contractions['nela'] = ['em', 'ela'], ['em', 'ele']
    contractions['neles'] = ['em', 'eles'], ['em', 'ele']
    contractions['nelas'] = ['em', 'elas'], ['em', 'ele']
    contractions['num'] = ['em', 'um'], ['em', 'um']
    contractions['numa'] = ['em', 'uma'], ['em', 'um']
    contractions['nuns'] = ['em', 'uns'], ['em', 'um']
    contractions['numas'] = ['em', 'umas'], ['em', 'um']    
    contractions['noutro'] = ['em', 'outro'], ['em', 'outro']
    contractions['noutra'] = ['em', 'outra'], ['em', 'outro']
    contractions['noutros'] = ['em', 'outros'], ['em', 'outro']
    contractions['noutras'] = ['em', 'outras'], ['em', 'outro']    
    contractions['naqueloutro'] = ['em', 'aqueloutro'], ['em', 'aqueloutro']
    contractions['naqueloutra'] = ['em', 'aqueloutra'], ['em', 'aqueloutro']
    contractions['naqueloutros'] = ['em', 'aqueloutros'], ['em', 'aqueloutro']
    contractions['naqueloutras'] = ['em', 'aqueloutras'], ['em', 'aqueloutro']    
    contractions['nessoutro'] = ['em', 'essoutro'], ['em', 'essoutro']
    contractions['nessoutra'] = ['em', 'essoutra'], ['em', 'essoutro']
    contractions['nessoutros'] = ['em', 'essoutros'], ['em', 'essoutro']
    contractions['nessoutras'] = ['em', 'essoutras'], ['em', 'essoutro']    
    contractions['nestoutro'] = ['em', 'estoutro'], ['em', 'estoutro']
    contractions['nestoutra'] = ['em', 'estoutra'], ['em', 'estoutro']
    contractions['nestoutros'] = ['em', 'estoutros'], ['em', 'estoutro']
    contractions['nestoutras'] = ['em', 'estoutras'], ['em', 'estoutro']    
    contractions['nalgum'] = ['em', 'algum'], ['em', 'algum']
    contractions['nalguma'] = ['em', 'alguma'], ['em', 'algum']
    contractions['nalguns'] = ['em', 'alguns'], ['em', 'algum']
    contractions['nalgumas'] = ['em', 'algumas'], ['em', 'algum']
    contractions['nalguém'] = ['em', 'alguém'], ['em', 'alguém']
    contractions['nalgo'] = ['em', 'algo'], ['em', 'algo']
    contractions['noutrem'] = ['em', 'outrem'], ['em', 'outrem']

    contractions['pelo'] = ['por', 'o'], ['por', 'o']
    contractions['pela'] = ['por', 'a'], ['por', 'o']
    contractions['pelos'] = ['por', 'os'], ['por', 'o']
    contractions['pelas'] = ['por', 'as'], ['por', 'o']
    
    contractions['hei-de'] = ['hei', 'de'], ['haver', 'de']
    contractions['há-de'] = ['há', 'de'], ['haver', 'de']
    contractions['hás-de'] = ['hás', 'de'], ['haver', 'de']    
    
    # Add upper cases.
    contraction_toks = contractions.keys()
    for tok in contraction_toks:
        first_char = tok[0]
        if first_char.islower():
            # NOTE: tok.title() fails when some character is not ASCII (e.g. DalguéM)
            upper_tok = tok[0].upper() + tok[1:] 
        else:
            upper_tok = re.sub('^à', 'À', tok)
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
