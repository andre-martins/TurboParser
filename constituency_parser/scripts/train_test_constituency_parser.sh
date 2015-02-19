#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ../..;pwd`"
task_folder="`cd $(dirname $0);cd ..;pwd`"

language=$1 # Example: "english_ptb".
discontinuous=$2
word_cutoff=$3 #0
C_parser=$4 #0.001
C_labeler=$5 #0.01
C_unary_predictor=$6 #1.0
delta_encoding=$7 #true #false
parser_model_type=$8 #full

path_data=${task_folder}/data/${language} # Folder with the data.
path_models=${task_folder}/models/${language} # Folder where models are stored.
path_results=${task_folder}/results/${language} # Folder for the results.

if ${discontinuous}
then
    projective=false
else
    projective=true
fi

if [ "$language" == "english_ptb" ]
then
    # Constituency files (gold tags).
    file_train_trees=${path_data}/${language}_train.trees
    files_test_trees[0]=${path_data}/${language}_test.trees
    files_test_trees[1]=${path_data}/${language}_dev.trees

    # Dependency files (predicted tags).
    file_train_conll=${path_data}/${language}_ftags_train.conll.predpos
    files_test_conll[0]=${path_data}/${language}_ftags_test.conll.predpos
    files_test_conll[1]=${path_data}/${language}_ftags_dev.conll.predpos
else
    # Example: swedish.
    # Constituency files (gold tags).
    file_train_trees=${path_data}/${language}_train.trees
    files_test_trees[0]=${path_data}/${language}_test.trees
    files_test_trees[1]=${path_data}/${language}_dev.trees

    # Dependency files (predicted tags).
    file_train_conll=${path_data}/${language}_ftags_train.conll
    files_test_conll[0]=${path_data}/${language}_ftags_test.conll
    files_test_conll[1]=${path_data}/${language}_ftags_dev.conll
fi

./train_test_parser.sh ${language} ${projective} ${C_parser} ${parser_model_type} ${word_cutoff}

if ! ${discontinuous}
then
    # Convert constituency trees to dependency trees using an existing dependency
    # file as a guide.
    file_train_trees_conll=${file_train_trees}.conll
    rm -f file_train_trees_conll
    java -jar -Dfile.encoding=utf-8 converter.jar conv-auto \
	${file_train_trees} \
	${file_train_trees_conll} \
	true \
	${file_train_conll}
    for (( i=0; i<${#files_test_trees[*]}; i++ ))
    do
	file_test_trees=${files_test_trees[$i]}
	file_test_conll=${files_test_conll[$i]}
	file_test_trees_conll=${file_test_trees}.conll
	rm -f file_test_trees_conll
	java -jar -Dfile.encoding=utf-8 converter.jar conv-auto \
            ${file_test_trees} \
            ${file_test_trees_conll} \
            true \
            ${file_test_conll}
    done
fi

if [ "${parser_model_type}" == "basic" ]
then
    parser_pruned=false
else
    parser_pruned=true
fi
suffix_parser=parser_pruned-${parser_pruned}_model-${parser_model_type}_cutoff-${word_cutoff}.pred
suffix_indexer=${suffix_parser}.labeler.pred.conv.trees

./train_test_dependency_labeler.sh ${language} ${suffix_parser} ${C_labeler} ${delta_encoding}
./train_test_constituency_labeler.sh ${language} ${suffix_indexer} 10 ${C_unary_predictor} 0.5 0.5 true

