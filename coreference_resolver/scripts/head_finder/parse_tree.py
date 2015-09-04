from span import *
import re

class ParseTreeConstituent(Span):
    def __init__(self, start, end, name='', tree=None):
        Span.__init__(self, start, end, name)
        self.tree = tree
        
        
class ParseTree:
    def __init__(self):
        self.label = ''
        self.children = list() # This is a list of ParseTree objects.
        
    def is_leaf(self):
        return len(self.children) == 0
        
    def is_preterminal(self):
        return len(self.children) == 1 and self.children[0].is_leaf()
        
    def get_label(self):
        return self.label
        
    def get_children(self):
        return self.children

    def add_child(self, tree):
        self.children.append(tree)

    def get_post_order_traversal(self):
        trees = []
        for kid in self.children:
            trees.extend(kid.get_post_order_traversal())
        trees.append(self)
        return trees

    def add_constituents_recursively(self, constituents, index):
        if self.is_leaf():
            constituent = ParseTreeConstituent(index, index, self.get_label(), self)
            constituents[self] = constituent
            return 1 # Length of leaf constituent.
        else:
            next_index = index
            for kid in self.get_children():
                next_index += kid.add_constituents_recursively(constituents, next_index)
            constituent = ParseTreeConstituent(index, next_index-1, self.get_label(), self)
            constituents[self] = constituent
            return next_index - index

    def get_constituents(self):
        constituents = dict()
        self.add_constituents_recursively(constituents, 0)
        return constituents

    def __str__(self):
        desc = ''
        if not self.is_leaf():
            desc += '('
        desc += self.get_label()
        if not self.is_leaf():
            for i, child in enumerate(self.get_children()):
                if i > 0 or self.get_label() != '':
                    desc += ' '
                desc += child.__str__()
            desc += ')'
        return desc


    def load(self, desc):
        left_bracket = '('
        right_bracket = ')'
        whitespace = ' '
        name = ''
        tree_stack = []
        line = re.sub('[\n\r\t\f]', ' ', desc)
        line = re.sub('\)', ') ', line)        
        line = re.sub('\(', ' (', line)
        while True:
            line_orig = line
            line = re.sub('\( \(', '((', line)
            line = re.sub('\) \)', '))', line)
            if line == line_orig:
                break
        line = re.sub('[ ]+', ' ', line)
        line = line.strip(' ')
#        print line
#        pdb.set_trace()
        for j, ch in enumerate(line):
            if ch == left_bracket:
                name = ''
                if len(tree_stack) > 0:
                    tree = ParseTree()
                    tree_stack[-1].add_child(tree)
                else:
                    tree = self
                tree_stack.append(tree)
            elif ch == right_bracket:
                tree = tree_stack.pop()
            elif ch == whitespace:
                pass
            else:
                name += ch
                assert j+1 < len(line) and line[j+1] != left_bracket, pdb.set_trace()
                if line[j+1] == right_bracket:
                    # Finished a terminal node.
                    tree = ParseTree()
                    tree.label = name
                    tree_stack[-1].add_child(tree)
                elif line[j+1] == whitespace:
                    # Just started a non-terminal or a pre-terminal node.
                    tree_stack[-1].label = name
                    name = ''
            
        assert len(tree_stack) == 0, pdb.set_trace()
        assert line == str(self), pdb.set_trace()
