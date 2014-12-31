import sys
import pdb

keep_document_names = True
trim_first_line = True # True for SemEval 2015, False for SemEval 2014.

filepath = sys.argv[1]
filepath_ids = sys.argv[2]

docs = dict()
f = open(filepath_ids)
for line in f:
    line = line.rstrip('\n')
    docs[line] = False
f.close()

print_word_indices = False
f = open(filepath)
selected = False
word_index = 0

if trim_first_line:
    f.readline()

for line in f:
    line = line.rstrip('\n')
    if line.startswith('#') and line.split('\t')[0] != '#':
        if line in docs:
            selected = True
            docs[line] = True
            if keep_document_names:
                print line
                #print '\n' + line
            else:
                print ''
                #print '\n'
            word_index = 0
        else:
            selected = False
            #print >> sys.stderr, 'Document not selected: ' + line
    elif selected:
        #assert line != '', pdb.set_trace() # Companion data update by SDP organizers (8/3/14, 11.tgz).
        if line != '':
            if print_word_indices:
                print str(word_index+1) + '\t' + line
            else:
                print line
            word_index += 1
        else:
            print line
            #pass

f.close()

for line in docs:
    if not docs[line]:
        #pdb.set_trace()
        sys.stderr.write('Document not found: ' + line, file=sys.stderr)
