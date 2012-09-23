DIR=$(cd $(dirname "$0"); pwd)
tmp_folder=${DIR}/tmp

if [ $# == 0 ]
then
    read text
    echo $text | ${DIR}/tokenizer.sed > ${tmp_folder}/tmp.tokenized 
else
    ${DIR}/tokenizer.sed $1 > ${tmp_folder}/tmp.tokenized
fi

cd ${DIR}

${DIR}/create_conll_corpus_from_text.sh ${tmp_folder}/tmp.tokenized > ${tmp_folder}/tmp.conll
${DIR}/create_tagging_corpus.sh ${tmp_folder}/tmp.conll # Creates tmp.conll.tagging.
${DIR}/run_tagger.sh ${tmp_folder}/tmp.conll.tagging # Creates tmp.conll.tagging.pred.
${DIR}/create_conll_predicted_tags_corpus.sh ${tmp_folder}/tmp.conll ${tmp_folder}/tmp.conll.tagging.pred # Creates tmp.conll.predpos
${DIR}/run_parser.sh ${tmp_folder}/tmp.conll.predpos # Creates tmp.conll.predpos.pred.
cat ${tmp_folder}/tmp.conll.predpos.pred
