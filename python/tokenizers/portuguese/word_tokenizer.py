# -*- coding: utf-8 -*-
# Natural Language Toolkit: Tokenizers
#
# Copyright (C) 2001-2012 NLTK Project
# Author: André Martins <atm@priberam.pt> 
#         
# URL: <http://nltk.sourceforge.net>
# For license information, see LICENSE.TXT

r"""

Portuguese Word Tokenizer

The Portuguese word tokenizer uses regular expressions to tokenize text as in
the Portuguese Cintil or Floresta Treebanks.
"""

import re
from nltk.tokenize.api import TokenizerI
import contractions as contr
import clitics as clit

class PortugueseWordTokenizer(TokenizerI):
    """
    This is a version of the Treebank tokenizer
    (nltk.tokenize.TreebankWordTokenizer) for Portuguese which tries to
    match the tokenization decisions of the Portuguese Cintil Treebank 
    (http://cintil.ul.pt/pt/manual.pdf).

    PortugueseCintilWordTokenizer assumes that the
    text has already been segmented into sentences, e.g. using 
    ``sent_tokenize()``. It assumes ISO-8859-15 encoding.

    This tokenizer performs the following steps:

    - split standard contractions, e.g. ``dos`` -> ``de_ os`` (see 
    contractions.py for a full list of contractions)
    - treat most punctuation characters as separate tokens
    - split off commas and single quotes, when followed by whitespace
    - if flag "replace_quotes" is True, replace all quotes by a single quote.
    - if flag "replace_parenthesis" is True, replace all parenthesis by a 
    single quote.
    - separate periods that appear at the end of line

        >>> import nltk
        >>> import tokenizers.portuguese.cintil_tokenizer as tokenizer_PT
        >>> sent_tokenizer = nltk.data.load('tokenizers/punkt/portuguese.pickle')
        >>> word_tokenizer = tokenizer_PT.PortugueseCintilWordTokenizer()
        >>> s = '''Um pastel de Belém custa 1,05 EUR em Lisboa. Queria dois pastéis desses, com açúcar e canela. Obrigado!''' 
        >>> sentences = sent_tokenizer.tokenize(s)
        >>> for sentence in sentences: print word_tokenizer.tokenize(sentence)
        ['Um', 'pastel', 'de', 'Bel\xc3\xa9m', 'custa', '1,05', 'EUR', 'em', 'Lisboa', '.']
        ['Queria', 'dois', 'past\xc3\xa9is', 'de_', 'esses', ',', 'com', 'a\xc3\xa7\xc3\xbacar', 'e', 'canela', '.']
        ['Obrigado', '!']

    NB. this tokenizer assumes that the text is presented as one sentence per line,
    where each line is delimited with a newline character.
    The only periods to be treated as separate tokens are those appearing
    at the end of a line.
    """
    def __init__(self, replace_quotes=False, replace_parenthesis=False):
        # Tokenization style.
        self.tokenization_style = ''

        # If True, replace all quotes by a single quote.
        self.replace_quotes = replace_quotes

        # If True, replace all parenthesis by a single quote.
        self.replace_parenthesis = replace_parenthesis

        # Generate list of contractions.
        self.contractions = contr.generate_contractions()

        # Generate list of clitics.
        self.clitics, self.suffixes = clit.generate_clitics()


    def tokenize(self, text):
        """
        Return a tokenized copy of *s*.

        :rtype: list of str
        """
        # Replace non-breaking spaces by spaces.
        # Note: the Portuguese sentence tokenizer should also do this!!
        text = re.sub('\xc2\xa0', ' ', text)

        if self.replace_parenthesis:
            # Replace all parenthesis by single quotes.
            # This looks a really terrible idea. However, since there are
            # no sentences with parenthesis in the CINTIL corpus (I don't
            # know why), further processing units in the pipeline (such as
            # a POS tagger or a parser) trained on that corpora would get
            # confused and so stupid things. Pretending everything is a 
            # single quote seems to be the least of all evils.
            text = re.sub(r'\(|\)', '\'', text)

        if self.replace_quotes:
            # Replace all quotes by single quotes.
            # This looks a terrible idea, but necessary for consistency with
            # the CINTIL corpus.
            text = re.sub(r'"|«|»|``|“|”|\'|`', '\' ', text)
        else:
            #starting quotes
            text = re.sub('«', r'``', text) # These are non-ASCII starting quotes
            text = re.sub('»', r'"', text) # These are non-ASCII ending quotes
            text = re.sub('“', r'``', text) # These are non-ASCII starting quotes
            text = re.sub('”', r'"', text) # These are non-ASCII ending quotes
            text = re.sub(r'^\"', r'``', text)
            text = re.sub(r'(``)', r' \1 ', text)
            text = re.sub(r'([ (\[{<])"', r'\1 `` ', text)

        #punctuation
        text = re.sub(r'([:,])([^\d])', r' \1 \2', text)
        text = re.sub(r'\.\.\.', r' ... ', text)
        text = re.sub(r'[;@#$%&]', r' \g<0> ', text)
        text = re.sub(r'([^\.])(\.)([\]\)}>"\']*)\s*$', r'\1 \2\3 ', text)
        text = re.sub(r'[?!]', r' \g<0> ', text)

        text = re.sub(r"([^'])' ", r"\1 ' ", text)

        #parens, brackets, etc.
        text = re.sub(r'[\]\[\(\)\{\}\<\>]', r' \g<0> ', text)
        text = re.sub(r'[^-](---)[^-]', r' -- ', text)
        text = re.sub(r'[^-](--)[^-]', r' -- ', text)

        #add extra space to make things easier
        text = " " + text + " "

        #ending quotes
        text = re.sub(r'"', " '' ", text)
        text = re.sub(r'(\S)(\'\')', r'\1 \2 ', text)

        # Split on contractions and clitics.
        if self.tokenization_style == 'cintil':
            text = contr.split_contractions(text, self.contractions,
                                            use_cintil_format=True)
            text = clit.split_clitics(text, self.clitics, self.suffixes,
                                      use_cintil_format=True)
        else:
            text = contr.split_contractions(text, self.contractions,
                                            use_cintil_format=False)
            text = clit.split_clitics(text, self.clitics, self.suffixes,
                                      use_cintil_format=False)

        text = re.sub(" +", " ", text)
        text = text.strip()

        #add space at end to match up with MacIntyre's output (for debugging)
        if text != "":
            text += " "

        return text.split()


class PortugueseCintilWordTokenizer(PortugueseWordTokenizer):
    """
    This is a version of the Treebank tokenizer
    (nltk.tokenize.TreebankWordTokenizer) for Portuguese which tries to
    match the tokenization decisions of the Portuguese Cintil Treebank 
    (http://cintil.ul.pt/pt/manual.pdf).

    PortugueseCintilWordTokenizer assumes that the
    text has already been segmented into sentences, e.g. using 
    ``sent_tokenize()``. It assumes ISO-8859-15 encoding.

    This tokenizer performs the following steps:

    - split standard contractions, e.g. ``dos`` -> ``de_ os`` (see 
    contractions.py for a full list of contractions)
    - treat most punctuation characters as separate tokens
    - split off commas and single quotes, when followed by whitespace
    - replace all quotes by a single quote.
    - replace all parenthesis by a single quote.
    - separate periods that appear at the end of line

        >>> import nltk
        >>> import tokenizers.portuguese.word_tokenizer as tokenizer_PT
        >>> sent_tokenizer = nltk.data.load('tokenizers/punkt/portuguese.pickle')
        >>> word_tokenizer = tokenizer_PT.PortugueseCintilWordTokenizer()
        >>> s = '''Um pastel de Belém custa 1,05 EUR em Lisboa. Queria dois pastéis desses, com açúcar e canela. Obrigado!''' 
        >>> sentences = sent_tokenizer.tokenize(s)
        >>> for sentence in sentences: print word_tokenizer.tokenize(sentence)
        ['Um', 'pastel', 'de', 'Bel\xc3\xa9m', 'custa', '1,05', 'EUR', 'em', 'Lisboa', '.']
        ['Queria', 'dois', 'past\xc3\xa9is', 'de_', 'esses', ',', 'com', 'a\xc3\xa7\xc3\xbacar', 'e', 'canela', '.']
        ['Obrigado', '!']

    NB. this tokenizer assumes that the text is presented as one sentence per line,
    where each line is delimited with a newline character.
    The only periods to be treated as separate tokens are those appearing
    at the end of a line.
    """
    def __init__(self):
        # Tokenization style.
        self.tokenization_style = 'cintil'

        # Call generic initializer.
        PortugueseWordTokenizer.__init__(self,
                                         replace_quotes = True,
                                         replace_parenthesis = True)



class PortugueseFlorestaWordTokenizer(PortugueseWordTokenizer):
    """
    This is a version of the Treebank tokenizer
    (nltk.tokenize.TreebankWordTokenizer) for Portuguese which tries to
    match the tokenization decisions of the Portuguese Floresta Sintá(c)tica
    Treebank (http://www.linguateca.pt/floresta/).

    PortugueseFlorestaWordTokenizer assumes that the
    text has already been segmented into sentences, e.g. using
    ``sent_tokenize()``. It assumes ISO-8859-15 encoding.

    This tokenizer performs the following steps:

    - split standard contractions, e.g. ``dos`` -> ``de_ os`` (see
    contractions.py for a full list of contractions)
    - treat most punctuation characters as separate tokens
    - split off commas and single quotes, when followed by whitespace
    - separate periods that appear at the end of line

        >>> import nltk
        >>> import tokenizers.portuguese.word_tokenizer as tokenizer_PT
        >>> sent_tokenizer = nltk.data.load('tokenizers/punkt/portuguese.pickle')
        >>> word_tokenizer = tokenizer_PT.PortugueseFlorestaWordTokenizer()
        >>> s = '''Um pastel de Belém custa 1,05 EUR em Lisboa. Queria dois pastéis desses, com açúcar e canela. Obrigado!''' 
        >>> sentences = sent_tokenizer.tokenize(s)
        >>> for sentence in sentences: print word_tokenizer.tokenize(sentence)
        ['Um', 'pastel', 'de', 'Bel\xc3\xa9m', 'custa', '1,05', 'EUR', 'em', 'Lisboa', '.']
        ['Queria', 'dois', 'past\xc3\xa9is', 'de_', 'esses', ',', 'com', 'a\xc3\xa7\xc3\xbacar', 'e', 'canela', '.']
        ['Obrigado', '!']

    NB. this tokenizer assumes that the text is presented as one sentence per line,
    where each line is delimited with a newline character.
    The only periods to be treated as separate tokens are those appearing
    at the end of a line.
    """


    def __init__(self):
        # Tokenization style.
        self.tokenization_style = 'floresta'

        # Call generic initializer.
        PortugueseWordTokenizer.__init__(self,
                                         replace_quotes = False,
                                         replace_parenthesis = False)
