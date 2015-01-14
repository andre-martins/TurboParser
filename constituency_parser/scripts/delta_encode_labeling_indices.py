import sys
import argparse
import pdb

def delta_encode_labeling_indices(heads, labels):
    num_words = len(heads)
    for i, label in enumerate(labels):
        if label == 'ROOT':
            labels[i] = 'ROOT_1'
    #pdb.set_trace()
    #label_names = [label.split('_')[0] for label in labels]
    #label_indices = [int(label.split('_')[1]) for label in labels]
    label_names = ['_'.join(label.split('_')[:-1]) for label in labels]
    label_indices = [int(label.split('_')[-1].lstrip('#')) for label in labels]
    new_labels = ['_' for i in xrange(num_words)]
    for h in xrange(num_words+1):
        left_modifiers = [m for m in xrange(h-2, -1, -1) if heads[m] == h]
        right_modifiers = [m for m in xrange(h, num_words) if heads[m] == h]
        for k, m in enumerate(left_modifiers):
            if k == 0:
                diff = label_indices[m] - 1
            else:
                diff = label_indices[m] - label_indices[left_modifiers[k-1]]
            new_labels[m] = '_'.join([label_names[m], str(diff)])
        for k, m in enumerate(right_modifiers):
            if k == 0:
                diff = label_indices[m] - 1
            else:
                diff = label_indices[m] - label_indices[right_modifiers[k-1]]
            new_labels[m] = '_'.join([label_names[m], str(diff)])

    for i, label in enumerate(new_labels):
        if label == 'ROOT_0':
            new_labels[i] = 'ROOT'
    return new_labels

def delta_decode_labeling_indices(heads, labels):
    num_words = len(heads)
    for i, label in enumerate(labels):
        if label == 'ROOT':
            labels[i] = 'ROOT_0'
    label_names = [label.split('_')[0] for label in labels]
    label_indices = [int(label.split('_')[1]) for label in labels]
    new_labels = ['_' for i in xrange(num_words)]
    for h in xrange(num_words+1):
        left_modifiers = [m for m in xrange(h-2, -1, -1) if heads[m] == h]
        right_modifiers = [m for m in xrange(h, num_words) if heads[m] == h]
        for k, m in enumerate(left_modifiers):
            if k == 0:
                index = label_indices[m] + 1
            else:
                index = label_indices[m] + label_indices[left_modifiers[k-1]]
            label_indices[m] = index
            new_labels[m] = '_'.join([label_names[m], str(index)])
        for k, m in enumerate(right_modifiers):
            if k == 0:
                index = label_indices[m] + 1
            else:
                index = label_indices[m] + label_indices[right_modifiers[k-1]]
            label_indices[m] = index
            new_labels[m] = '_'.join([label_names[m], str(index)])

    for i, label in enumerate(new_labels):
        if label == 'ROOT_1':
            new_labels[i] = 'ROOT'
    return new_labels

parser = argparse.ArgumentParser(description='Convert dependency-to-phrase labeling indices to/from delta encoding.')
parser.add_argument('--from_delta', type=bool, default=False, help='If True, conversion will be from delta to ordering indices (not the other way around).')
parser.add_argument('filepath', type=str, help='Path to the CoNLL file to be converted.')

args = parser.parse_args()

filepath = args.filepath
from_delta = args.from_delta
f = open(filepath)

sentence_fields = []
for line in f:
    line = line.rstrip('\n')
    if line == '':
        heads = [int(fields[6]) for fields in sentence_fields]
        labels = [fields[7] for fields in sentence_fields]
        if from_delta:
            new_labels = delta_decode_labeling_indices(heads, labels)
        else:
            new_labels = delta_encode_labeling_indices(heads, labels)
        for fields, new_label in zip(sentence_fields, new_labels):
            fields[7] = new_label
            print '\t'.join(fields)
        print
        sentence_fields = []
    else:
        fields = line.split('\t')
        sentence_fields.append(fields)

f.close()
