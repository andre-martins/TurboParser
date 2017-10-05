# -*- coding: utf-8 -*-
'''This implements an aligned unicode string to which substitution operations
(with regular expressions) can be made while always keeping it aligned with
the original string. It's particularly useful for word tokenization when one
needs to retrieve the positions on each token in the buffer.'''

__author__ = 'andre-martins'

# We need "regex" instead of "re" since it allows more powerful patterns using
# Unicode codepoint properties (such as \p{IsLower} etc.)
import regex

class AlignedString(object):
    '''This class implements an aligned string to which substitution operations
    (with regular expressions) can be made while always keeping it aligned with
    the original string.'''
    def __init__(self, string=''):
        self.original_string = string
        self.string = string
        length = len(self.string)
        self.start_positions = range(length)
        self.end_positions = range(1, length+1)

    def __repr__(self):
        elems = []
        for start, end in zip(self.start_positions, self.end_positions):
            elems.append(self.original_string[start:end])
        return self.string + ' || ' + '|'.join(elems)

    def substitute(self, pattern, replacement, offset=0, all_matches=True, \
                   align_groups=True):
        '''Substitute a regex pattern by a replacement string. May start
        searching at a given offset; may return after the first match or
        perform all matches (in left to right order). If the pattern selects
        groups, it tries by default to align each group with the original
        string. Return a (boolean, int) pair indicating if there was any match
        and the offset position after the last match.'''
        matched = False
        while all_matches or not matched:
            m = regex.search(pattern, self.string[offset:])
            if m is None:
                break
            matched = True
            num_groups = len(m.groups())
            if num_groups:
                # Compute actual replacement string.
                # If align_groups, then provide more informative start and end
                # positions that refer back to the matched group.
                actual_replacement = replacement
                if align_groups:
                    group_start_positions = [-1] * num_groups
                    group_end_positions = [-1] * num_groups
                    group_replacement_positions = []
                for i in xrange(num_groups):
                    start = offset + m.start(i+1)
                    end = offset + m.end(i+1)
                    if align_groups:
                        group_start_positions[i] = \
                            self.start_positions[start:end]
                        group_end_positions[i] = \
                            self.end_positions[start:end]
                        group_offset = 0
                        while True:
                            gm = regex.search(r'\\%d' % (i+1), \
                                              actual_replacement[group_offset:])
                            if gm is None:
                                break
                            group_start = group_offset + gm.start()
                            group_end = group_offset + gm.end()
                            group_offset = group_start + len(m.group(i+1))
                            actual_replacement = \
                                actual_replacement[:group_start] + \
                                m.group(i+1) + \
                                actual_replacement[group_end:]
                            # Every replacement positions that started after
                            # group_end need now be shifted by len(m.group(i+1))
                            # - group_end + group_start. This is necessary
                            # for handling well inversions like "\2 \1".
                            shift_amount = len(m.group(i+1)) - group_end + \
                                           group_start
                            for k, (group, other_group_start) in \
                                enumerate(group_replacement_positions):
                                if group < i and other_group_start >= group_end:
                                    group_replacement_positions[k] = \
                                        (group, \
                                         other_group_start + shift_amount)
                            # Now append a new replacement position.
                            group_replacement_positions.append((i, group_start))
                    else:
                        actual_replacement = regex.sub(r'\\%d' % (i+1), \
                                                       m.group(i+1), \
                                                       actual_replacement)
            else:
                actual_replacement = replacement
            start = offset + m.start()
            end = offset + m.end()
            # assert self.string[start:end] == m.group()
            self.string = self.string[:start] + actual_replacement + \
                          self.string[end:]
            length = len(actual_replacement)
            self.start_positions[start:end] = \
                [self.start_positions[start]] * length
            self.end_positions[start:end] = \
                [self.end_positions[end-1]] * length
            if align_groups and num_groups:
                for group, group_start in \
                    group_replacement_positions:
                    group_start += start
                    group_end = group_start + len(group_start_positions[group])
                    self.start_positions[group_start:group_end] = \
                            group_start_positions[group]
                    self.end_positions[group_start:group_end] = \
                            group_end_positions[group]
            offset = start + length
        return matched, offset

    def split(self, pattern):
        '''Split the string on a separator matching the provided regex pattern.
        Return the list of tokens (strings) and the list of (start, end)
        positions of each token.'''
        offset = 0
        tokens = []
        positions = []
        while True:
            m = regex.search(pattern, self.string[offset:])
            if m is None:
                break
            assert not len(m.groups())
            start = offset + m.start()
            end = offset + m.end()
            # assert self.string[start:end] == m.group()
            tokens.append(self.string[offset:start])
            positions.append((offset, start))
            offset = end
        start = len(self.string)
        tokens.append(self.string[offset:start])
        positions.append((offset, start))
        return tokens, positions

