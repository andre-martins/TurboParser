import sys
import operator
import pdb

filepath = sys.argv[1] # Input file (CONLL format).
filepath_out = sys.argv[2] # Output file (most frequent lemma for each word).
f = open(filepath)
f_out = open(filepath_out, 'w')

lemmas = {}
frequency = {}

for line in f:
    line = line.rstrip('\n')
    if line == '':
        continue
    elif line.startswith('#begin document'):
        continue
    elif line.startswith('#end document'):
        continue

    fields = line.split()
    index = int(fields[0])
    word = fields[1]
    lemma = fields[2]
    tag = fields[4]

    if lemma == '_':
        continue

    if (word, tag) not in lemmas:
        lemmas[(word, tag)] = {}
        frequency[(word, tag)] = 1
    else:
        frequency[(word, tag)] += 1
    if lemma not in lemmas[(word, tag)]:
        lemmas[(word, tag)][lemma] = 1
    else:
        lemmas[(word, tag)][lemma] += 1


sorted_words = sorted(frequency.iteritems(), key=operator.itemgetter(1))
for (word, tag), word_frequency in reversed(sorted_words):
    #print word, word_frequency
    sorted_lemmas = sorted(lemmas[(word, tag)].iteritems(), key=operator.itemgetter(1))
    #for lemma, lemma_frequency in reversed(sorted_lemmas):
    #    print word, tag, word_frequency, lemma, lemma_frequency
    most_frequent_lemma = sorted_lemmas[-1][0]
    f_out.write('%s\t%s\t%s\n' % (word, tag, most_frequent_lemma))

f_out.close()
