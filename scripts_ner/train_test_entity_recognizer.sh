#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english_proj".
train_algorithm=svm_mira # Training algorithm.
num_epochs=50 #20 # Number of training epochs.
regularization_parameter=0.01 #$2 #1e12 # The C parameter in MIRA.
train=true
test=true
model_type=2 # Second-order model (trigrams).
form_cutoff=0 #1 # Word cutoff. Only words which occur more than these times won't be considered unknown.
tagging_scheme=bio #bilou
file_gazetteer= # Empty gazetteer file by default.
suffix=entity_recognizer

# Set path folders.
path_bin=${root_folder} # Folder containing the binary.
path_scripts=${root_folder}/scripts_ner # Folder containing scripts.
path_data=${root_folder}/ner/data/${language} # Folder with the data.
path_models=${root_folder}/ner/models/${language} # Folder where models are stored.
path_results=${root_folder}/ner/results/${language} # Folder for the results.

# Create folders if they don't exist.
mkdir -p ${path_data}
mkdir -p ${path_models}
mkdir -p ${path_results}

# Set file paths. Allow multiple test files.
file_model=${path_models}/${language}_${suffix}.model
file_train=${path_data}/${language}_train.conll.ner

if [ "$language" == "english" ] || [ "$language" == "english_ontonotes" ]
then
    files_test[0]=${path_data}/${language}_test.conll.ner
    files_test[1]=${path_data}/${language}_dev.conll.ner
    files_test[2]=${path_data}/${language}_train.conll.ner
    file_gazetteer=${path_data}/${language}_all_gazetteers.txt

    echo "Creating gazetteer file..."
    python create_gazetteer_file.py ${path_data}/KnownLists $file_gazetteer
    echo "Done."

    if [ "$language" == "english_ontonotes" ]
    then
        model_type=1 # First-order model (bigrams).
    fi

elif [ "$language" == "spanish" ] || [ "$language" == "dutch" ]
then
    files_test[0]=${path_data}/${language}_test.conll.ner
    files_test[1]=${path_data}/${language}_dev.conll.ner
    files_test[2]=${path_data}/${language}_train.conll.ner
else
    files_test[0]=${path_data}/${language}_test.conll.ner
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_prediction=${file_test}.pred
    files_prediction[$i]=${file_prediction}
done

################################################
# Train the entity recognizer.
################################################

if $train
then
    echo "Training..."
    ${path_bin}/TurboEntityRecognizer \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --sequence_model_type=${model_type} \
        --form_cutoff=${form_cutoff} \
        --entity_tagging_scheme=${tagging_scheme} \
        --entity_file_gazetteer=${file_gazetteer} \
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
        ${path_bin}/TurboEntityRecognizer \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --logtostderr

        echo ""
        echo "Evaluating..."
        #perl ${path_scripts}/eval_predpos.pl ${file_prediction} ${file_test}
        paste ${file_test} ${file_prediction} | awk '{ print $1" "$2" "$3" "$6 }' | perl conlleval.txt
    done
fi
