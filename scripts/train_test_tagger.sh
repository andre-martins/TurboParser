#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english_proj".
train_algorithm=svm_mira # Training algorithm.
num_epochs=10 # Number of training epochs.
regularization_parameter=1e12 # The C parameter in MIRA.
train=true
test=true
model_type=2 # Second-order model (trigrams).
suffix=tagger

# Set path folders.
path_bin=${root_folder} # Folder containing the binary.
path_data=${root_folder}/data/${language} # Folder with the data.
path_models=${root_folder}/models/${language} # Folder where models are stored.
path_results=${root_folder}/results/${language} # Folder for the results.

# Create folders if they don't exist.
mkdir -p ${path_data}
mkdir -p ${path_models}
mkdir -p ${path_results}

# Set file paths. Allow multiple test files.
file_model=${path_models}/${language}_${suffix}.model
file_train=${path_data}/${language}_train.conll.tagging
if [ "$language" == "english_proj" ]
then
    files_test[0]=${path_data}/${language}_test.conll.tagging
    files_test[1]=${path_data}/${language}_dev.conll.tagging
else
    files_test[0]=${path_data}/${language}_test.conll.tagging
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_prediction=${file_test}.pred
    files_prediction[$i]=${file_prediction}
done

################################################
# Train the tagger.
################################################

if $train
then
    echo "Training..."
    ${path_bin}/TurboTagger \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --tagger_model_type=${model_type} \
        --form_cutoff=1 \
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
        ${path_bin}/TurboTagger \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --logtostderr

        echo ""
        echo "Evaluating..."
        perl eval_predpos.pl ${file_prediction} ${file_test}
    done
fi
