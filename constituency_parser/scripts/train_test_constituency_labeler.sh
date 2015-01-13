#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ../..;pwd`"
task_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english_proj".
train_algorithm=svm_mira # Training algorithm.
num_epochs=10 #20 # Number of training epochs.
regularization_parameter=$2 #0.001 # The C parameter in MIRA.
train_cost_false_positives=$3
train_cost_false_negatives=$4
ignore_null_labels=false #true
train=true
test=true
case_sensitive=false # Distinguish word upper/lower case.
form_cutoff=0 # Cutoff in word occurrence.
lemma_cutoff=0 # Cutoff in lemma occurrence.

#suffix_parser=parser_pruned-true_model-standard.pred
suffix=constituency_labeler

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
file_model=${path_models}/${language}_${suffix}.model
file_results=${path_results}/${language}_${suffix}.txt

if [ "$language" == "english_ptb" ]
then
    file_train=${path_data}/${language}_train.trees
    files_test[0]=${path_data}/${language}_test.trees
    files_test[1]=${path_data}/${language}_dev.trees
else
    file_train=${path_data}/${language}_train.trees
    files_test[0]=${path_data}/${language}_test.trees
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_prediction=${file_test}.${suffix}.pred
    files_prediction[$i]=${file_prediction}
done


################################################
# Train the parser.
################################################

if $train
then
    ${path_bin}/TurboConstituencyLabeler \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --form_case_sensitive=${case_sensitive} \
        --form_cutoff=${form_cutoff} \
        --lemma_cutoff=${lemma_cutoff} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --constituency_labeler_train_cost_false_positives=${train_cost_false_positives} \
        --constituency_labeler_train_cost_false_negatives=${train_cost_false_negatives} \
        --ignore_null_labels=${ignore_null_labels} \
        --logtostderr
fi


################################################
# Test the parser.
################################################

if $test
then

    rm -f ${file_results}

    # Test first with oracle backbone dependencies.
    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test=${files_test[$i]}
        file_prediction=${files_prediction[$i]}

        echo ""
        echo "Testing on ${file_test}..."
        ${path_bin}/TurboConstituencyLabeler \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --logtostderr

        echo ""
        echo "Evaluating..."
        touch ${file_results}
        EVALB/evalb -p EVALB/COLLINS_new.prm ${files_test[$i]} ${files_prediction[$i]} | grep Bracketing | head -3 \
            >> ${file_results}
        cat ${file_results}
    done
fi
