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
jackknifing=false #true #false # True for performing jackknifing in the training data. Useful for downstream applications.
model_type=2 # Second-order model (trigrams).
form_cutoff=1 # Word cutoff. Only words which occur more than these times won't be considered unknown.
suffix=tagger

# Set path folders.
path_bin=${root_folder} # Folder containing the binary.
path_scripts=${root_folder}/scripts # Folder containing scripts.
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

# Use lexicon file if there is one.
if [ -e "${path_data}/${language}_lexicon.txt" ];
then
    file_lexicon=${path_data}/${language}_lexicon.txt
else
    file_lexicon=
fi

# Create tagging corpus from CoNLL data if it does not yet exist.
if [ -e "${path_data}/${language}_train.conll" ] && [ ! -e "${path_data}/${language}_train.conll.tagging" ]; 
then
    echo "Creating tagging corpus from CoNLL data."

    ${path_scripts}/create_tagging_corpus.sh "${path_data}/${language}_train.conll"
    ${path_scripts}/create_tagging_corpus.sh "${path_data}/${language}_test.conll"

    if [ "$language" == "english_proj" ]
    then
        ${path_scripts}/create_tagging_corpus.sh "${path_data}/${language}_dev.conll"
    fi
fi

if [ "$language" == "english_proj" ]
then
    files_test[0]=${path_data}/${language}_test.conll.tagging
    files_test[1]=${path_data}/${language}_dev.conll.tagging
else
    files_test[0]=${path_data}/${language}_test.conll.tagging
    files_test[1]=${path_data}/${language}_dev.conll.tagging
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

    if ${jackknifing}
    then
	num_jackknifing_partitions=10
	file_train_jackknifed=${file_train}.pred

	echo "Jackknifing with ${num_jackknifing_partitions} partitions..."
	python ${path_scripts}/split_corpus_jackknifing.py ${file_train} \
            ${num_jackknifing_partitions}

	for (( i=0; i<${num_jackknifing_partitions}; i++ ))
	do
	    file_train_jackknifing=${file_train}_all-splits-except-${i}
	    file_test_jackknifing=${file_train}_split-${i}
	    file_model_jackknifing=${file_model}_split-${i}
            file_prediction_jackknifing=${file_test_jackknifing}.pred

            echo ""
	    echo "Training on ${file_train_jackknifing}..."
	    ${path_bin}/TurboTagger \
		--train \
		--train_epochs=${num_epochs} \
		--file_model=${file_model_jackknifing} \
		--file_train=${file_train_jackknifing} \
                --file_lexicon=${file_lexicon} \
		--train_algorithm=${train_algorithm} \
		--train_regularization_constant=${regularization_parameter} \
		--sequence_model_type=${model_type} \
		--form_cutoff=${form_cutoff} \
		--logtostderr
	    
            echo ""
            echo "Testing on ${file_test_jackknifing}..."
            ${path_bin}/TurboTagger \
		--test \
		--evaluate \
		--file_model=${file_model_jackknifing} \
		--file_test=${file_test_jackknifing} \
		--file_prediction=${file_prediction_jackknifing} \
		--logtostderr

            echo ""
            echo "Evaluating..."
            perl ${path_scripts}/eval_predpos.pl ${file_prediction_jackknifing} ${file_test_jackknifing}

            if [ "${i}" == "0" ]
	    then
		cat ${file_prediction_jackknifing} > ${file_train_jackknifed}
            else
		cat ${file_prediction_jackknifing} >> ${file_train_jackknifed}
	    fi
	done
    fi

    echo "Training..."
    ${path_bin}/TurboTagger \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --file_lexicon=${file_lexicon} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --sequence_model_type=${model_type} \
        --form_cutoff=${form_cutoff} \
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
        perl ${path_scripts}/eval_predpos.pl ${file_prediction} ${file_test}
    done
fi
