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

import numpy as np

def split_clitics(sentence, clitics, suffixes, use_cintil_format=True):
    """
    Split clitics from the verb form (in Portuguese).
    If use_cintil_format is True, the output is consistent with the
    Portuguese CINTIL corpus (e.g.: "fá-lo-ei" ->  "fá#-CL-ei" + "-lo").

    :rtype: string with tokens separated by whitespaces.
    """
    toks = sentence.split(' ')
    new_toks = []
    for tok in toks:
        new_tok = tok[:]
        suffix_tok = ''
        for suffix in suffixes:
            if tok.endswith(suffix):
                suffix_tok = tok[-len(suffix):]
                new_tok = tok[:-len(suffix)]
                break

        clitic_tok = ''
        for clitic in clitics:
            if new_tok.endswith(clitic):
                clitic_tok = new_tok[-len(clitic):]
                new_tok = new_tok[:-len(clitic)]
                break

        if clitic_tok != '':
            if clitic_tok in ['-lo', '-la', '-los', '-las']:
                new_tok += '#'
            if suffix_tok != '':
                new_tok += '-CL' + suffix_tok
                
            new_toks.append(new_tok)
            new_toks.append(clitic_tok)
        else:
            new_toks.append(tok)
            
    return ' '.join(new_toks)


def generate_clitics():
    """
    Generate clitics (e.g. '-se') and verb suffixes (e.g. '-ei') for
    Portuguese.
    
    :rtype: a list of suffixes and a list of clitics.
    """                
    suffixes = set()
    clitics = set()
    
    # Suffixos dos verbos em mesoclíticos.
    suffixes.add('-ei')
    suffixes.add('-ás')
    suffixes.add('-á')
    suffixes.add('-emos')
    suffixes.add('-eis')
    suffixes.add('-ão')
    suffixes.add('-ia')
    suffixes.add('-ias')
    suffixes.add('-íamos')
    suffixes.add('-íeis')
    suffixes.add('-iam')
    
    # Nota: com -lo, -la, -los, -las, há sempre um r,s,z que cai.
    # Com -no, -na, -nos, -nas, quase nunca cai ("dizem-nos"), a não ser quando 
    # o verbo termina em "mos" ("lavamo-nos").    
    clitics.add('-lo')
    clitics.add('-la')
    clitics.add('-los')
    clitics.add('-las')
    
    clitics.add('-na')
    clitics.add('-nas')
    clitics.add('-me')
    clitics.add('-te')
    clitics.add('-no')
    clitics.add('-nos')
    clitics.add('-vos')
    clitics.add('-o')
    clitics.add('-os')
    clitics.add('-a')
    clitics.add('-as')
    clitics.add('-se')
    clitics.add('-lhe')
    clitics.add('-lhes')
    clitics.add('-mo')
    clitics.add('-mos')
    clitics.add('-ma')
    clitics.add('-mas')
    clitics.add('-to')
    clitics.add('-tos')
    clitics.add('-ta')
    clitics.add('-tas')
    
    clitics.add('-no-lo')
    clitics.add('-no-los')
    clitics.add('-no-la')
    clitics.add('-no-las')
    clitics.add('-vo-lo')
    clitics.add('-vo-los')
    clitics.add('-vo-la')
    clitics.add('-vo-las')
    clitics.add('-lho')
    clitics.add('-lhos')
    clitics.add('-lha')
    clitics.add('-lhas')
    clitics.add('-se-me')
    clitics.add('-se-te')
    clitics.add('-se-nos')
    clitics.add('-se-vos')
    clitics.add('-se-lhe')
    clitics.add('-se-lhes')
    clitics.add('-se-mo')
    clitics.add('-se-mos')
    clitics.add('-se-ma')
    clitics.add('-se-mas')
    clitics.add('-se-to')
    clitics.add('-se-tos')
    clitics.add('-se-ta')
    clitics.add('-se-tas')
    
    clitics.add('-se-no-lo')
    clitics.add('-se-no-los')
    clitics.add('-se-no-la')
    clitics.add('-se-no-las')
    clitics.add('-se-vo-lo')
    clitics.add('-se-vo-los')
    clitics.add('-se-vo-la')
    clitics.add('-se-vo-las')
    clitics.add('-se-lho')
    clitics.add('-se-lhos')
    clitics.add('-se-lha')
    clitics.add('-se-lhas')
    
    # Sort lists by length so that, if one suffix/clitic is a suffix of another,
    # we always check the longest one first.
    clitic_list = list(clitics)
    clitic_inds = list(np.argsort([len(clitic) for clitic in clitic_list]))
    clitic_inds.reverse()

    suffix_list = list(suffixes)
    suffix_inds = list(np.argsort([len(suffix) for suffix in suffix_list]))
    suffix_inds.reverse()
    
    final_clitic_list = [clitic_list[i] for i in clitic_inds]
    final_suffix_list = [suffix_list[i] for i in suffix_inds]
    

    return final_clitic_list, final_suffix_list
    
    
if __name__ == "__main__":
    print generate_clitics()
    
