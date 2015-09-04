import re
import sys
import parse_tree as pt
import mod_collins_head_finder as mchf
import warnings
import pdb

# Adapted from the Berkeley coreference system.
def extract_dependency_structure(num_words, parse_tree, head_finder):
    constituents = parse_tree.get_constituents()
    subtree_heads = dict()
    trees = parse_tree.get_post_order_traversal()
    heads = [-1] * num_words
    for tree in trees:
        if tree.is_leaf():
            pass
        elif tree.is_preterminal():
            constituent = constituents[tree]
            subtree_heads[tree] = constituent.start
        else:
            children = tree.get_children()
            head = head_finder.determine_head(tree)
            if head == None:
                warnings.warn('No head found: ' + str(tree))
                head = children[0]
            head_index = subtree_heads[head]
            for child in children:
                if child == head:
                    subtree_heads[tree] = head_index
                else:
                    heads[subtree_heads[child]] = head_index
    return heads


def parse_conll_file(filename, tag_column, word_column, parse_column):
    f = open(filename)

    # Obtain dependencies from the parse tree using Collins' head rules.
    head_finder = mchf.ModCollinsHeadFinder()

    all_fields = []
    parse_tree_desc = ''
    for line in f:
        line = line.rstrip('\n')
        if line == '':
            # Load the syntactic parse tree from its string description.
            parse_tree = pt.ParseTree()
            parse_tree.load(parse_tree_desc)
            num_words = len(all_fields)
            heads = extract_dependency_structure(num_words, parse_tree, head_finder)
            for i, fields in enumerate(all_fields):
                deprel = '_'
                new_fields = fields[:(parse_column+1)] + [str(heads[i]+1), deprel] + fields[(parse_column+1):]
                new_line = '\t'.join(new_fields)
                print new_line
            print

            all_fields = []
            parse_tree_desc = ''
        elif line.startswith('#begin') or line.startswith('#end'):
            print line
        else:
            line = re.sub('[ ]+', '\t', line)
            fields = line.split('\t')
            all_fields.append(fields)
            assert len(fields) >= 3, pdb.set_trace()
            word = fields[word_column]
            tag = fields[tag_column]
            parse_span = fields[parse_column]
            parse_tree_bit = re.sub('\*', '(' + tag + ' ' + word + ')', parse_span)
            parse_tree_desc += parse_tree_bit

    f.close()


if __name__ == '__main__':
    filename = sys.argv[1]
    word_column = int(sys.argv[2])
    tag_column = int(sys.argv[3])
    parse_column = int(sys.argv[4])

    parse_conll_file(filename, tag_column, word_column, parse_column)
