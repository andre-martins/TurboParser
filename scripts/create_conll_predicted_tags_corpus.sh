file_conll=$1
file_tagging=$2

./create_conll_predicted_tags_corpus.pl \
${file_conll} ${file_tagging} > ${file_conll}.predpos
