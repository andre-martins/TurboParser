from parse_tree import *
import pdb

#/**
# * A base class for Head Finders similar to the one described in
# * Michael Collins' 1999 thesis.  For a given constituent we perform
# * <p/>
# * for categoryList in categoryLists
# * for index = 1 to n [or n to 1 if R->L]
# * for category in categoryList
# * if category equals daughter[index] choose it.
# * <p/>
# * with a final default that goes with the direction (L->R or R->L)
# * For most constituents, there will be only one category in the list,
# * the exception being, in Collins' original version, NP.
# * <p/>
# * It is up to the overriding base class to initialize the map
# * from constituent type to categoryLists, "nonTerminalInfo", in its constructor.
# * Entries are presumed to be of type String[][].  Each String[] is a list of
# * categories, except for the first entry, which specifies direction of
# * traversal and must be one of "right", "left" or "rightdis" or "leftdis".
# * <p/>
# * "left" means search left-to-right by category and then by position
# * "leftdis" means search left-to-right by position and then by category
# * "right" means search right-to-left by category and then by position
# * "rightdis" means search right-to-left by position and then by category
# * <p/>
# * <p/>
# * 10/28/2002 -- Category label identity checking now uses the
# * equals() method instead of ==, so not interning category labels
# * shouldn't break things anymore.  (Roger Levy) <br>
# * 2003/02/10 -- Changed to use TreebankLanguagePack and to cut on
# * characters that set off annotations, so this should work even if
# * functional tags are still on nodes. <br>
# * 2004/03/30 -- Made abstract base class and subclasses for CollinsHeadFinder,
# * ModCollinsHeadFinder, SemanticHeadFinder, ChineseHeadFinder
# * (and trees.icegb.ICEGBHeadFinder, trees.international.negra.NegraHeadFinder,
# * and movetrees.EnglishPennMaxProjectionHeadFinder)
# *
# * @author Christopher Manning
# * @author Galen Andrew
# */

def is_punctuation_tag(tag):
    return tag in set(["''", "``", "-LRB-", "-RRB-", ".", ":", ","])

def transform_label(tree):
    transformed_label = tree.get_label()
    return transform_label_string(tree.is_leaf(), transformed_label)

def transform_label_string(is_leaf, transformed_label):
    cut_index = transformed_label.find('-')
    cut_index2 = transformed_label.find('=')
    cut_index3 = transformed_label.find('^')
    if (cut_index3 > 0 and (cut_index3 < cut_index2 or cut_index2 == -1)):
        cut_index2 = cut_index3
    if (cut_index2 > 0 and (cut_index2 < cut_index or cut_index <= 0)):
        cut_index = cut_index2
    if (cut_index > 0 and not is_leaf):
        transformed_label = transformed_label[:cut_index]
    return transformed_label


    

class ModCollinsHeadFinder:
    def __init__(self):
        self.DEBUG = False
        self.non_terminal_info = dict()
        # default direction if no rule is found for category
        # subclasses can turn it on if they like.
        self.default_rule = []
        
        # This version from Collins' diss (1999: 236-238)
        self.non_terminal_info["ADJP"] =  [["left", "NNS", "QP", "NN", \
			"$", "ADVP", "JJ", "VBN", "VBG", "ADJP", "JJR", "NP", "JJS", "DT", \
			"FW", "RBR", "RBS", "SBAR", "RB" ]]
        self.non_terminal_info["ADVP"] = [["right", "RB", "RBR", "RBS", \
			"FW", "ADVP", "TO", "CD", "JJR", "JJ", "IN", "NP", "JJS", "NN" ]]
        self.non_terminal_info["CONJP"] = [[ "right", "CC", "RB", "IN" ]]
        self.non_terminal_info["FRAG"] = [[ "right" ]] # crap
        self.non_terminal_info["INTJ"] = [[ "left" ]]
        self.non_terminal_info["LST"] = [[ "right", "LS", ":" ]]
        self.non_terminal_info["NAC"] = [["left", "NN", "NNS", "NNP", \
			"NNPS", "NP", "NAC", "EX", "$", "CD", "QP", "PRP", "VBG", "JJ", "JJS", \
			"JJR", "ADJP", "FW" ]]
        self.non_terminal_info["NX"] = [[ "left" ]] # crap
        self.non_terminal_info["PP"] = [[ "right", "IN", "TO", "VBG", \
			"VBN", "RP", "FW" ]]
        # should prefer JJ? (PP (JJ such) (IN as) (NP (NN crocidolite)))
        self.non_terminal_info["PRN"] = [[ "left" ]]
        self.non_terminal_info["PRT"] = [[ "right", "RP" ]]
        self.non_terminal_info["QP"] = [[ "left", "$", "IN", "NNS", \
		  "NN", "JJ", "RB", "DT", "CD", "NCD", "QP", "JJR", "JJS" ]]
        self.non_terminal_info["RRC"] = [["right", "VP", "NP", "ADVP", \
			"ADJP", "PP" ]]
        self.non_terminal_info["S"] = [[ "left", "TO", "IN", "VP", "S", \
			"SBAR", "ADJP", "UCP", "NP" ]]
        self.non_terminal_info["SBAR"] = [[ "left", "WHNP", "WHPP", \
			"WHADVP", "WHADJP", "IN", "DT", "S", "SQ", "SINV", "SBAR", "FRAG" ]]
        self.non_terminal_info["SBARQ"] = [["left", "SQ", "S", "SINV", \
			"SBARQ", "FRAG" ]]
        self.non_terminal_info["SINV"] = [[ "left", "VBZ", "VBD", "VBP", \
			"VB", "MD", "VP", "S", "SINV", "ADJP", "NP" ]]
        self.non_terminal_info["SQ"] = [[ "left", "VBZ", "VBD", "VBP", \
			"VB", "MD", "VP", "SQ" ]]
        self.non_terminal_info["UCP"] = [[ "right" ]]
        self.non_terminal_info["VP"] = [[ "left", "TO", "VBD", "VBN", \
			"MD", "VBZ", "VB", "VBG", "VBP", "AUX", "AUXG", "VP", "ADJP", "NN",
			"NNS", "NP"]]
        self.non_terminal_info["WHADJP"] = [[ "left", "CC", "WRB", "JJ", \
                "ADJP"]]
        self.non_terminal_info["WHADVP"] = [[ "right", "CC", "WRB" ]]
        self.non_terminal_info["WHNP"] = [[ "left", "WDT", "WP", "WP$", \
		  "WHADJP", "WHPP", "WHNP" ]]
        self.non_terminal_info["WHPP"] = [[ "right", "IN", "TO", "FW" ]]
        self.non_terminal_info["X"] = [[ "right" ]] # crap rule
        self.non_terminal_info["NML"] = [[ "rightdis", "NN", "NNP", "NNPS", "NNS", "NX", "JJR" ], \
            [ "left", "NP" ], [ "rightdis", "$", "ADJP", "PRN" ], \
            [ "right", "CD" ], [ "rightdis", "JJ", "JJS", "RB", "QP" ]]
        self.non_terminal_info["NP"] = [[ "rightdis", "NN", "NNP", "NNPS", "NNS", "NX", "JJR" ], \
		    [ "left", "NP" ], [ "rightdis", "$", "ADJP", "PRN" ],
		    [ "right", "CD" ], [ "rightdis", "JJ", "JJS", "RB", "QP" ]]
        self.non_terminal_info["TYPO"] = [[ "left" ]] # another crap rule, for Switchboard (Roger)
        
        

    def determine_head(self, t):
        '''Determine which daughter of the current parse tree is the head.
       
        @param t The parse tree to examine the daughters of.
                 If this is a leaf, <code>null</code> is returned
        @return The daughter parse tree that is the head of <code>t</code>
        @see Tree#percolateHeads(HeadFinder)
             for a routine to call this and spread heads throughout a tree.'''
   
        if t.is_leaf():
            return None
        kids = t.get_children()
        
        
        # if the node is a unary, then that kid must be the head
        # it used to special case preterminal and ROOT/TOP case
        # but that seemed bad (especially hardcoding string "ROOT")
        if len(kids) == 1:
            return kids[0]

        return self.determine_non_trivial_head(t)


    def determine_non_trivial_head(self, t):
        '''Called by determineHead and may be overridden in subclasses
        if special treatment is necessary for particular categories.'''
        the_head = None
        mother_cat = transform_label(t)
        if self.DEBUG:
            print 'Looking for head of ' + t.get_label()

        # We know we have nonterminals underneath
        # (a bit of a Penn Treebank assumption, but).
        
        if mother_cat not in self.non_terminal_info:
            if self.DEBUG:
                print 'Warning: No rule found for ' + mother_cat
            if len(self.default_rule) != 0:
                return self.traverse_locate(t.get_children(), self.default_rule, True)
            else:
                return None
            
        how = self.non_terminal_info[mother_cat]
        for i in xrange(len(how)):
            deflt = (i == len(how) - 1)
            the_head = self.traverse_locate(t.get_children(), how[i], deflt)
            if the_head != None:
                break

        return the_head


    def traverse_locate(self, daughter_trees, how, deflt):
        '''Attempt to locate head daughter tree from among daughters.
        Go through daughterTrees looking for things from a set found by
        looking up the motherkey specifier in a hash map, and if
        you do not find one, take leftmost or rightmost thing iff
        deflt is true, otherwise return <code>null</code>.'''        
        head_index = 0
        found = False

        if how[0] == 'left':
            for i in xrange(1,len(how)):
                for head_index in xrange(len(daughter_trees)):
                    child_cat = transform_label(daughter_trees[head_index])
                    if how[i] == child_cat:
                        found = True
                        break
                if found:
                    break
            if not found:
                # none found by tag, so return first or null.
                if deflt:
                    head_index = 0
                else:
                    return None
        elif how[0] == 'leftdis':
            for head_index in xrange(len(daughter_trees)):
                child_cat = transform_label(daughter_trees[head_index])
                for i in xrange(1,len(how)):
                    if how[i] == child_cat:
                        found = True
                        break
                if found:
                    break
            if not found:
                # none found by tag, so return first or null.
                if deflt:
                    head_index = 0
                else:
                    return None
        elif how[0] == 'right':
            # from right.
            for i in xrange(1,len(how)):
                for head_index in xrange(len(daughter_trees)-1, -1, -1):
                    child_cat = transform_label(daughter_trees[head_index])
                    if how[i] == child_cat:
                        found = True
                        break
                if found:
                    break
            if not found:
                # none found by tag, so return first or null.
                if deflt:
                    head_index = len(daughter_trees)-1
                else:
                    return None
        elif how[0] == 'rightdis':
            # from right, but search for any, not in turn.
            for head_index in xrange(len(daughter_trees)-1, -1, -1):
                child_cat = transform_label(daughter_trees[head_index])
                for i in xrange(1,len(how)):
                    if how[i] == child_cat:
                        found = True
                        break
                if found:
                    break
            if not found:
                # none found by tag, so return first or null.
                if deflt:
                    head_index = len(daughter_trees)-1
                else:
                    return None
        else:
            raise NotImplementedError
            
        head_index = self.post_operation_fix(head_index, daughter_trees)
        return daughter_trees[head_index]


    def post_operation_fix(self, head_index, daughter_trees):
        '''A way for subclasses to fix any heads under special conditions
        The default does nothing.
       
        @param headIdx       the index of the proposed head
        @param daughterTrees the array of daughter trees
        @return the new headIndex.'''
        if head_index >= 2:
            prev_lab = daughter_trees[head_index-1].get_label()
            if prev_lab == 'CC':
                new_head_index = head_index - 2
                t = daughter_trees[new_head_index]
                while new_head_index >= 0 and t.is_preterminal() and is_punctuation_tag(t.get_label()):
                    new_head_index -= 1
                if new_head_index >= 0:
                    head_index = new_head_index
        return head_index
        

if __name__ == "__main__":
    tree = ParseTree()
    desc = "((S (NP (DT the) (JJ quick) (JJ (AA (BB (CC brown)))) (NN fox)) (VP (VBD jumped) (PP (IN over) (NP (DT the) (JJ lazy) (NN dog)))) (. .)))"
    tree.load(desc)
    assert desc == str(tree), pdb.set_trace()

    head_finder = ModCollinsHeadFinder()
    while not tree.is_leaf():
        head = head_finder.determine_head(tree)
        print 'Head', head
        tree = head
    pdb.set_trace()
