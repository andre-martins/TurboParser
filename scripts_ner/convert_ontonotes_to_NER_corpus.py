import sys
import pdb

filepath = sys.argv[1]
f = open(filepath)

entity_type = 'O'
for line in f:
    line = line.rstrip('\n')
    if line == '':
        entity_type = 'O'
        print line
    elif line.startswith('#begin') or line.startswith('#end'):
        entity_type = 'O'
        continue
    else:
        fields = line.split()
        entity_info = fields[10]
        if entity_info == '*':
            if entity_type != 'O':
                prefixed_entity = 'I-%s' % entity_type
            else:
                prefixed_entity = 'O'
        elif entity_info.startswith('('):
            if entity_info.endswith(')'):
                entity_type = entity_info[1:-1]
                prefixed_entity = 'B-%s' % entity_type
                entity_type = 'O'
            else:
                assert entity_info[-1] == '*', pdb.set_trace()
                entity_type = entity_info[1:-1]
                prefixed_entity = 'B-%s' % entity_type
        else:
            assert entity_info == '*)', pdb.set_trace()
            assert entity_type != 'O', pdb.set_trace()
            prefixed_entity = 'I-%s' % entity_type
            entity_type = 'O'
        print '%s %s %s' % (fields[3], fields[4], prefixed_entity)

f.close()

