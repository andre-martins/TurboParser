import turboparser as tp

class NLPSentence(dict):
    def __init__(self):
        pass

    def compute_morphology(self, worker):
        words = self['words']
        if len(words) == 0:
            # It is necessary to address this case here, since empty sentences
            # make TurboParser crash.
            self['tags'] = []
            self['lemmas'] = None
            if worker.lemmatizer is not None:
                self['lemmas'] = []
            self['morphological_tags'] = None
            if worker.morphological_tagger is not None:
                self['morphological_tags'] = []
            return

        # Compute POS tags.
        self['tags'] = None
        tags = ['_' for word in words]
        sequence_instance = tp.PSequenceInstance()
        sequence_instance.initialize(words, tags)
        worker.tagger.tag_sentence(sequence_instance)
        tags = [sequence_instance.get_tag(i) for i in xrange(len(words))]
        self['tags'] = tags

        # Compute lemmas.
        self['lemmas'] = None
        if worker.lemmatizer is not None:
            lemmas = worker.lemmatizer.lemmatize_sentence(words, tags)
            self['lemmas'] = lemmas
        else:
            lemmas = ['_' for word in words]

        # Compute morphological tags.
        self['morphological_tags'] = None
        if worker.morphological_tagger is not None:
            feats = ['_' for word in words]
            morphological_instance = tp.PMorphologicalInstance()
            morphological_instance.initialize(words, lemmas, tags, feats)
            worker.morphological_tagger.tag_sentence(morphological_instance)
            feats = [morphological_instance.get_tag(i)
                     for i in xrange(len(words))]
            self['morphological_tags'] = [feat.split('|') if feat != '_'
                                          else []
                                          for feat in feats]

    def compute_entities(self, worker):
        words = self['words']
        tags = self['tags']
        if len(words) == 0:
            # It is necessary to address this case here, since empty sentences
            # make TurboParser crash.
            self['entity_tags'] = []
            return

        # For now, use entity BIO tags. Later we should move to entity spans.
        self['entity_tags'] = None
        entity_tags = ['_' for word in words]
        entity_instance = tp.PEntityInstance()
        entity_instance.initialize(words, tags, entity_tags)
        worker.entity_recognizer.tag_sentence(entity_instance)
        entity_tags = [entity_instance.get_tag(i) for i in xrange(len(words))]
        self['entity_tags'] = entity_tags

    def compute_syntactic_dependencies(self, worker):
        words = self['words']
        tags = self['tags']
        lemmas = self['lemmas']
        feats = [[] for word in words]
        # TurboParser assumes 1-based indexing.
        heads = [0 for word in words]
        deprels = ['_' for word in words]
        # TurboParser requires pre-appending a dummy root symbol.
        words_with_root = ['_root_'] + words
        lemmas_with_root = ['_root_'] + lemmas
        tags_with_root = ['_root_'] + tags
        feats_with_root = [['_root_']] + feats
        deprels_with_root = ['_root_'] + deprels
        heads_with_root = [-1] + heads
        dependency_instance = tp.PDependencyInstance()
        dependency_instance.initialize(words_with_root, lemmas_with_root,
                                       tags_with_root, tags_with_root,
                                       feats_with_root, deprels_with_root,
                                       heads_with_root)
        worker.parser.parse_sentence(dependency_instance)
        # Convert back to 0-based indexing. Words attached to the root will get
        # head = -1.
        self['heads'] = [dependency_instance.get_head(i+1)-1
                         for i in xrange(len(words))]
        self['dependency_relations'] = \
            [dependency_instance.get_dependency_relation(i+1)
             for i in xrange(len(words))]

    def compute_semantic_dependencies(self, worker):
        words = self['words']
        tags = self['tags']
        lemmas = self['lemmas']
        # TurboParser assumes 1-based indexing.
        heads = [h+1 for h in self['heads']]
        deprels = self['dependency_relations']
        feats = [[] for word in words]
        predicate_names = []
        predicate_indices = []
        argument_roles = []
        argument_indices = []
        # TurboParser requires pre-appending a dummy root symbol.
        words_with_root = ['_root_'] + words
        lemmas_with_root = ['_root_'] + lemmas
        tags_with_root = ['_root_'] + tags
        feats_with_root = [['_root_']] + feats
        deprels_with_root = ['_root_'] + deprels
        heads_with_root = [-1] + heads
        semantic_instance = tp.PSemanticInstance()
        semantic_instance.initialize('', words_with_root, lemmas_with_root,
                                     tags_with_root, tags_with_root,
                                     feats_with_root, deprels_with_root,
                                     heads_with_root, predicate_names,
                                     predicate_indices, argument_roles,
                                     argument_indices)
        worker.semantic_parser.parse_semantic_dependencies_from_sentence(
            semantic_instance)

        num_predicates = semantic_instance.get_num_predicates()
        predicate_names = [semantic_instance.get_predicate_name(k)
                           for k in xrange(num_predicates)]
        # Convert back to 0-based indexing.
        predicate_indices = [semantic_instance.get_predicate_index(k)-1
                             for k in xrange(num_predicates)]
        argument_roles = \
            [[semantic_instance.get_argument_role(k, l)
              for l in xrange(semantic_instance.get_num_arguments_predicate(k))]
             for k in xrange(num_predicates)]
        # Convert to back 0-based indexing.
        argument_indices = \
            [[semantic_instance.get_argument_index(k, l)-1
              for l in xrange(semantic_instance.get_num_arguments_predicate(k))]
             for k in xrange(num_predicates)]

        self['predicate_names'] = predicate_names
        self['predicate_indices'] = predicate_indices
        self['argument_roles'] = argument_roles
        self['argument_indices'] = argument_indices
