import nlp_pipeline
pipe = nlp_pipeline.NLPPipeline()

conll_str = pipe.parse_conll('I solved the problem with statistics.', 'EN-Nonprojective')
print(conll_str)