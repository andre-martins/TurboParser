import sys
import re

fields_to_convert = [1,3,4]

filepath = sys.argv[1]
f = open(filepath)
for line in f:
    line = line.rstrip('\n')
    if line == '':
        print
    else:
        fields = line.split('\t')
        for i in fields_to_convert:
            field = fields[i]
            field = re.sub('\(', '-LRB-', field)
            field = re.sub('\)', '-RRB-', field)
            field = re.sub('\[', '-LSB-', field)
            field = re.sub('\]', '-RSB-', field)
            field = re.sub('\{', '-LCB-', field)
            field = re.sub('\}', '-RCB-', field)
            fields[i] = field
        print '\t'.join(fields)

f.close()
