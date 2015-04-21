import sys
import pdb

keep_document_names = True
has_sense = True # True for SemEval 2015, False for SemEval 2014.
trim_first_line = True # True for SemEval 2015, False for SemEval 2014.

filepath = sys.argv[1]
if len(sys.argv) > 2:
    filepath_companion = sys.argv[2]
else:
    filepath_companion = ''

f = open(filepath)
if filepath_companion == '':
    f_companion = None
else:
    f_companion = open(filepath_companion)

if trim_first_line:
    line = f.readline()
    line = line.rstrip('\n')
    print line

for line in f:
    line = line.rstrip('\n')
    if f_companion != None:
        line_companion = f_companion.readline()
        line_companion = line_companion.rstrip('\n')

    if line.startswith('#') and line.split('\t')[0] != '#':
        if keep_document_names: print line
    elif line == '':
        print line
    else:
        fields = line.split("\t")
        word_index = fields[0]
        word = fields[1]
        lemma = fields[2]
        pos = fields[3]
        if len(fields) == 4:
            top = '-'
            pred = '-'
            if has_sense:
                sense = '-'
        else:
            top = fields[4]
            pred = fields[5]
            if has_sense:
                sense = fields[6]
        if has_sense:
            args = fields[7:]
        else:
            args = fields[6:]

        if f_companion != None:
            fields_companion = line_companion.split("\t")
            predicted_pos = fields_companion[0]
            head = fields_companion[1]
            deprel = fields_companion[2]
        else:
            predicted_pos = '_'
            head = '_'
            deprel = '_'

        fields_output = [word_index,
                         word,
                         lemma,
                         pos, #predicted_pos
                         head,
                         deprel,
                         top,
                         pred]
        if has_sense:
            fields_output.append(sense);
        fields_output.extend(args);
        line_output = '\t'.join(fields_output)
        print line_output

f.close()
if f_companion != None:
    f_companion.close()
