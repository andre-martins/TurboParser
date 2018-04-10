from __future__ import print_function

import sys

import numpy as np


def split_corpus(filename, num_partitions):
    sentences = []
    sentence = ''
    f = open(filename)
    for line in f:
        if line.rstrip() == '':
            sentences.append(sentence)
            sentence = ''
        else:
            sentence += line
    f.close()

    partition_length = int(np.ceil(float(len(sentences)) / num_partitions))
    print('Number of sentences: ', len(sentences))
    print('Number of partitions: ', num_partitions)
    print('Partition size: ', partition_length)

    offset = 0
    for k in range(num_partitions):
        split_train_suffix = 'all-splits-except-' + str(k)
        split_test_suffix = 'split-' + str(k)
        filename_train_split = filename + '_' + split_train_suffix
        filename_test_split = filename + '_' + split_test_suffix
        f_train = open(filename_train_split, 'w')
        f_test = open(filename_test_split, 'w')
        for i, sentence in enumerate(sentences):
            sentence = sentences[i]
            if offset <= i < offset + partition_length:
                f_test.write(sentence + '\n')
            else:
                f_train.write(sentence + '\n')
        offset += partition_length
        f_train.close()
        f_test.close()


if __name__ == "__main__":
    filename = sys.argv[1]
    num_partitions = int(sys.argv[2])
    split_corpus(filename, num_partitions)
