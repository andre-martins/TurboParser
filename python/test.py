import nlp_pipeline
pipe = nlp_pipeline.NLPPipeline()

text = 'I solved the problem with statistics.' 
language = 'EN-Nonprojective'
conll_str = pipe.parse_conll(text, language )
print(conll_str)

text = 'Lisbon is the capital and the largest city of Portugal.'
sentences = pipe.split_sentences(text, language)
for sentence in sentences:
    tokenized_sentence = pipe.tokenize(sentence, language)
    tags, lemmas, feats = pipe.tag(tokenized_sentence, language)
    entity_tags = pipe.recognize_entities(tokenized_sentence,tags, language)
    print(entity_tags)