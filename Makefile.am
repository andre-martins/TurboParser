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

SUBDIRS=src/tagger src/parser
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
data/sample/sample_train.conll \
data/sample/sample_test.conll \
install_deps.sh \
deps/AD3-2.0.2.tar.gz \
deps/gflags-2.0-no-svn-files.tar.gz \
deps/glog-0.3.2.tar.gz

