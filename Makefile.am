############################################
# Temporary solution to enable debug builds.
# See: http://stackoverflow.com/questions/4553735/gnu-autotools-debug-release-targets
############################################
if DEBUG
AM_CFLAGS = -g3 -O0
AM_CXXFLAGS = -g3 -O0
else
AM_CFLAGS = -O2
AM_CXXFLAGS = -O2
endif
############################################

SUBDIRS=src/tagger src/parser src/semantic_parser src/entity_recognizer \
src/coreference_resolver src/dependency_labeler src/constituency_labeler
EXTRA_DIST=scripts/create_conll_corpus_from_text.pl \
scripts/create_conll_predicted_tags_corpus.pl \
scripts/create_conll_predicted_tags_corpus.sh \
scripts/create_tagging_corpus.sh \
scripts/eval.pl \
scripts/eval_predpos.pl \
scripts/parse.sh \
scripts/run_parser.sh \
scripts/run_tagger.sh \
scripts/tokenizer.sed \
scripts/train_test_parser.sh \
scripts/train_test_tagger.sh \
scripts_ner/conlleval.txt \
scripts_ner/train_test_entity_recognizer.sh \
scripts_ner/convert_ontonotes_to_NER_corpus.py \
scripts_ner/create_gazetteer_file.py \
scripts_ner/prepare_NER_corpus_for_POS_tagging.sh \
scripts_ner/create_NER_corpus_from_POS_tagged_corpus.sh \
scripts_srl/eval08.pl \
scripts_srl/remove_augmented.py \
scripts_srl/train_test_semantic_parser.sh \
scripts_srl/train_test_submission_SDP2015.sh \
scripts_srl/train_test_submission_closed.sh \
scripts_srl/train_test_submission_open.sh \
data/sample/sample_train.conll \
data/sample/sample_test.conll \
semeval2014_data/README \
semeval2014_data/scripts/augment_with_companion_data.py \
semeval2014_data/scripts/dev_ids \
semeval2014_data/scripts/generate_all_splits.sh \
semeval2014_data/scripts/split_data.py \
semeval2014_data/scripts/train+dev_ids \
semeval2014_data/scripts/train_ids \
semeval2015_data/README \
semeval2015_data/scripts/augment_with_companion_data.py \
semeval2015_data/scripts/generate_all_splits.sh \
semeval2015_data/scripts/split_data.py \
semeval2015_data/scripts/en_train_ids \
semeval2015_data/scripts/en_dev_ids \
semeval2015_data/scripts/en_train+dev_ids \
semeval2015_data/scripts/cs_train_ids \
semeval2015_data/scripts/cs_dev_ids \
semeval2015_data/scripts/cs_train+dev_ids \
semeval2015_data/scripts/cz_train_ids \
semeval2015_data/scripts/cz_dev_ids \
semeval2015_data/scripts/cz_train+dev_ids \
libturboparser/Makefile \
libturboparser/TurboParserInterface.cpp \
libturboparser/TurboParserInterface.h \
python/install_wrapper.sh \
python/lemmatizer.py \
python/nlp_pipeline.py \
python/README \
python/setup.py \
python/turbo_parser.pyx \
python/tokenizers/__init__.py \
python/tokenizers/portuguese/__init__.py \
python/tokenizers/portuguese/clitics.py \
python/tokenizers/portuguese/contractions.py \
python/tokenizers/portuguese/word_tokenizer.py \
install_deps.sh \
deps/AD3-2.0.2.tar.gz \
deps/gflags-2.0-no-svn-files.tar.gz \
deps/glog-0.3.2.tar.gz

