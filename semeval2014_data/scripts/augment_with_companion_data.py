import sys

keep_document_names = True

filepath = sys.argv[1]
filepath_companion = sys.argv[2]
f = open(filepath)
f_companion = open(filepath_companion)
for line, line_companion in zip(f, f_companion):
    line = line.rstrip('\n')
    line_companion = line_companion.rstrip('\n')
    if line.startswith('#') and line.split('\t')[0] != '#':
        if keep_document_names: print line
    elif line == '':
        print line
    else:
        fields = line.split("\t")
        fields_companion = line_companion.split("\t")
        word_index = fields[0]
        word = fields[1]
        lemma = fields[2]
        pos = fields[3]
        if len(fields) == 4:
            top = '-'
            pred = '-'
        else:
            top = fields[4]
            pred = fields[5]
        args = fields[6:]
        predicted_pos = fields_companion[0]
        head = fields_companion[1]
        deprel = fields_companion[2]

        fields_output = [word_index,
                         word,
                         lemma,
                         pos, #predicted_pos
                         "_", #Morph or Syntatic Features by Corentin Ribeyre
                         head,
                         deprel,
                         top,
                         pred]
        fields_output.extend(args);
        line_output = '\t'.join(fields_output)
        print line_output

f.close()
f_companion.close()
