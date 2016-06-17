#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ../..;pwd`"
task_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "UD_Danish".
language_prefix=$2 # Example: "da-ud"
train_algorithm=svm_mira # Training algorithm.
num_epochs=20 # Number of training epochs.
regularization_parameter=0.01 # The C parameter in MIRA.
train=true
test=true
model_type=0 # 0 for unigrams, 1 for bigrams, 2 for trigrams.
form_cutoff=1 # Word cutoff. Only words which occur more than these times won't be considered unknown.
prefix_length=0
suffix_length=3
suffix=morphological_tagger
use_fine_tags=false

# Set path folders.
path_bin=${root_folder} # Folder containing the binary.
path_scripts=${task_folder}/scripts # Folder containing scripts.
path_data=${task_folder}/data/${language} # Folder with the data.
path_models=${task_folder}/models/${language} # Folder where models are stored.
path_results=${task_folder}/results/${language} # Folder for the results.

# Create folders if they don't exist.
mkdir -p ${path_data}
mkdir -p ${path_models}
mkdir -p ${path_results}

# Set file paths. Allow multiple test files.
file_model=${path_models}/${language_prefix}_${suffix}.model
if ${use_fine_tags}
then
    file_train_orig=${path_data}/${language_prefix}-train.conllu
    files_test_orig[0]=${path_data}/${language_prefix}-test.conllu
    files_test_orig[1]=${path_data}/${language_prefix}-dev.conllu

    file_train=${path_data}/${language_prefix}_ftags-train.conllu
    files_test[0]=${path_data}/${language_prefix}_ftags-test.conllu
    files_test[1]=${path_data}/${language_prefix}_ftags-dev.conllu

    rm -f file_train
    awk 'NF>0{OFS="\t";NF=10;$4=$5;$5=$5;print}NF==0{print}' ${file_train_orig} \
        > ${file_train}

    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test_orig=${files_test_orig[$i]}
        file_test=${files_test[$i]}
        rm -f file_test
        awk 'NF>0{OFS="\t";NF=10;$4=$5;$5=$5;print}NF==0{print}' ${file_test_orig} \
            > ${file_test}
    done
else
    file_train=${path_data}/${language_prefix}-train.conllu
    files_test[0]=${path_data}/${language_prefix}-test.conllu
    files_test[1]=${path_data}/${language_prefix}-dev.conllu
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_prediction=${file_test}.pred
    files_prediction[$i]=${file_prediction}
done

################################################
# Train the morphological tagger.
################################################

if $train
then
    echo "Training..."
    ${path_bin}/TurboMorphologicalTagger \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --sequence_model_type=${model_type} \
        --form_cutoff=${form_cutoff} \
        --prefix_length=${prefix_length} \
        --suffix_length=${suffix_length} \
        --logtostderr
fi

################################################
# Test the tagger.
################################################

if $test
then

    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test=${files_test[$i]}
        file_prediction=${files_prediction[$i]}

        echo ""
        echo "Testing on ${file_test}..."
        ${path_bin}/TurboMorphologicalTagger \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --logtostderr

        echo ""
        #echo "Evaluating..."
        #perl ${path_scripts}/eval_predpos.pl ${file_prediction} ${file_test}
    done
fi
