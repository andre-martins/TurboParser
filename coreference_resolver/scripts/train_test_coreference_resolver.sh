#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ../..;pwd`"
task_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english_proj".
train_algorithm=crf_margin_sgd # Training algorithm.
num_epochs=20 # Number of training epochs.
regularization_parameter=10 #0.01 #$2 #1e12 # The C parameter in MIRA.
train_initial_learning_rate=0.1
train_learning_rate_schedule=invsqrt
train=true
test=true
false_anaphor_cost=0.0 #0.1
false_new_cost=3.0
false_wrong_link_cost=1.0
#model_type=2 # Second-order model (trigrams).
form_cutoff=20 #1 # Word cutoff. Only words which occur more than these times won't be considered unknown.
#tagging_scheme=bilou # bio
#file_gazetteer= # Empty gazetteer file by default.
suffix=coreference_resolver

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
coreference_file_mention_tags=${path_data}/${language}_mention_tags.txt
coreference_file_pronouns=${path_data}/${language}_pronouns.txt
coreference_file_gender_number_statistics=${path_data}/gender.data

if [ "$language" == "english_ontonotes_wsj" ] || [ "$language" == "english_ontonotes" ]
then
    file_train_orig=${path_data}/${language}_train.conll.coref
    files_test_orig[0]=${path_data}/${language}_test_blind.conll.coref
    files_test_orig[1]=${path_data}/${language}_dev.conll.coref
    files_test_orig[2]=${path_data}/${language}_train.conll.coref
    files_gold_orig[0]=${path_data}/${language}_test.conll.coref
    files_gold_orig[1]=${path_data}/${language}_dev.conll.coref
    files_gold_orig[2]=${path_data}/${language}_train.conll.coref

    file_train=${file_train_orig}_with_heads
    files_test[0]=${files_test_orig[0]}_with_heads
    files_test[1]=${files_test_orig[1]}_with_heads
    files_test[2]=${files_test_orig[2]}_with_heads
    files_gold[0]=${files_gold_orig[0]}_with_heads
    files_gold[1]=${files_gold_orig[1]}_with_heads
    files_gold[2]=${files_gold_orig[2]}_with_heads

    echo "Applying English head finder..."
    python ${path_scripts}/head_finder/english_head_finder.py ${file_train_orig} 3 4 5 > ${file_train}
    python ${path_scripts}/head_finder/english_head_finder.py ${files_test_orig[0]} 3 4 5 > ${files_test[0]}
    python ${path_scripts}/head_finder/english_head_finder.py ${files_test_orig[1]} 3 4 5 > ${files_test[1]}
    python ${path_scripts}/head_finder/english_head_finder.py ${files_test_orig[2]} 3 4 5 > ${files_test[2]}
    python ${path_scripts}/head_finder/english_head_finder.py ${files_gold_orig[0]} 3 4 5 > ${files_gold[0]}
    python ${path_scripts}/head_finder/english_head_finder.py ${files_gold_orig[1]} 3 4 5 > ${files_gold[1]}
    python ${path_scripts}/head_finder/english_head_finder.py ${files_gold_orig[2]} 3 4 5 > ${files_gold[2]}
else
    file_train=${path_data}/${language}_train.conll.coref
    files_test[0]=${path_data}/${language}_test.conll.coref
    files_test[1]=${path_data}/${language}_dev.conll.coref
    files_gold[0]=${path_data}/${language}_test.conll.coref
    files_gold[1]=${path_data}/${language}_test.conll.coref
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
    ${path_bin}/TurboCoreferenceResolver \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --coreference_file_mention_tags=${coreference_file_mention_tags} \
        --coreference_file_pronouns=${coreference_file_pronouns} \
        --coreference_file_gender_number_statistics=${coreference_file_gender_number_statistics} \
        --train_algorithm=${train_algorithm} \
        --train_initial_learning_rate=${train_initial_learning_rate} \
        --train_learning_rate_schedule=${train_learning_rate_schedule} \
        --train_regularization_constant=${regularization_parameter} \
        --form_cutoff=${form_cutoff} \
        --false_anaphor_cost=${false_anaphor_cost} \
        --false_new_cost=${false_new_cost} \
        --false_wrong_link_cost=${false_wrong_link_cost} \
        --logtostderr
fi

################################################
# Test the coreference resolver.
################################################

if $test
then

    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_gold=${files_gold[$i]}
        file_test=${files_test[$i]}
        file_prediction=${files_prediction[$i]}

        echo ""
        echo "Testing on ${file_test}..."
        ${path_bin}/TurboCoreferenceResolver \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --logtostderr

        echo ""
        echo "Evaluating..."

        ${path_scripts}/scorer/v8.01/scorer.pl muc \
            ${file_gold} ${file_prediction} | tail -5
        ${path_scripts}/scorer/v8.01/scorer.pl bcub \
            ${file_gold} ${file_prediction} | tail -5
        ${path_scripts}/scorer/v8.01/scorer.pl ceafe \
            ${file_gold} ${file_prediction} | tail -5
    done
fi
