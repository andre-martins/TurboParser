import os
import sys

if __name__ == "__main__":
    path = sys.argv[1] # Path to the folder where the gazetteers are.
    destination_filepath = sys.argv[2] # Path to the destination file.
    f_out = open(destination_filepath, 'w')
    gazetteers_list = [name for name in os.listdir(path) if
                       not os.path.isdir(path + os.sep + name)]
    gazetteers_list.sort()
    for name in gazetteers_list:
        print 'Adding ', name, '...'
        filepath = path + os.sep + name
        f = open(filepath)
        for line in f:
            line = line.rstrip('\r\n')
            f_out.write('%s\t%s\n' % (name, line))
    f_out.close()
