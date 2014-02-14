import sys

filepath = sys.argv[1]

f = open(filepath)
for line in f:
    line = line.rstrip('\n')
    if line.startswith('#') and line.split('\t')[0] != '#':
        print line
    elif line == '':
        print line
    else:
        fields = line.split('\t')
        fields_out = fields[:4]
        fields_out.extend(fields[6:])
        line_out = '\t'.join(fields_out)
        print line_out

f.close()
