#!/bin/bash

this_folder="`cd $(dirname $0);pwd`"

file_conll=$1
file_tagging=$2

${this_folder}/create_conll_predicted_tags_corpus.pl \
${file_conll} ${file_tagging} > ${file_conll}.predpos
