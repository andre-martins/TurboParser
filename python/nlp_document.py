import turboparser as tp
from nlp_sentence import NLPSentence

class NLPDocument(dict):
    def __init__(self, sentences):
        self['sentences'] = sentences

    def compute_coreferences(self, worker):
        sentences = self['sentences']
        coreference_sentences = []
        for sentence in sentences:
            sentence['coreference_spans'] = None
            words = sentence['words']
            tags = sentence['tags']
            lemmas = sentence['lemmas']
            # TurboParser assumes 1-based indexing.
            heads = [h+1 for h in sentence['heads']]
            deprels = sentence['dependency_relations']
            # For now, don't use this (must be coded as spans).
            entity_tags = sentence['entity_tags']
            feats = [[] for word in words]
            predicate_names = []
            predicate_indices = []
            argument_roles = []
            argument_indices = []
            speakers = ['-' for word in words]
            # TurboParser requires pre-appending a dummy root symbol.
            words_with_root = ['_root_'] + words
            lemmas_with_root = ['_root_'] + lemmas
            tags_with_root = ['_root_'] + tags
            feats_with_root = [['_root_']] + feats
            deprels_with_root = ['_root_'] + deprels
            heads_with_root = [-1] + heads
            speakers_with_root = ['__'] + speakers
            p_entity_spans = []
            p_constituent_spans = []
            p_coreference_spans = []
            coreference_sentence = tp.PCoreferenceSentence()
            coreference_sentence.initialize('', \
                                            words_with_root, \
                                            lemmas_with_root, \
                                            tags_with_root, \
                                            tags_with_root, \
                                            feats_with_root, \
                                            deprels_with_root, \
                                            heads_with_root, \
                                            predicate_names, \
                                            predicate_indices, \
                                            argument_roles, \
                                            argument_indices, \
                                            speakers_with_root, \
                                            p_entity_spans, \
                                            p_constituent_spans, \
                                            p_coreference_spans)
            coreference_sentences.append(coreference_sentence)

        coreference_document = tp.PCoreferenceDocument()
        coreference_document.initialize('', 0, coreference_sentences)
        worker.coreference_resolver.resolve_coreferences_from_document( \
            coreference_document)

        for i, sentence in enumerate(sentences):
            coreference_sentence = coreference_document.get_sentence(i)
            coreference_spans = coreference_sentence.get_coreference_spans()
            # Convert back to 0-based indexing.
            sentence['coreference_spans'] = \
                [(span.start()-1, span.end()-1, span.name()) \
                 for span in coreference_spans]
