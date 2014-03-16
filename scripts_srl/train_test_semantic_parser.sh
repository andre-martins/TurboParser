#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english".
train_algorithm=svm_mira # Training algorithm.
train_algorithm_pruner=crf_mira # Training algorithm for pruner.
num_epochs=10 # Number of training epochs.
num_epochs_pruner=10 # Number of training epochs for the pruner.
regularization_parameter=$2 #0.001 # The C parameter in MIRA.
regularization_parameter_pruner=1e12 # Same for the pruner.
train=true
test=true
prune=true # This will revert to false if model_type=basic.
prune_labels=true
prune_distances=true
train_external_pruner=false # If true, the pruner is trained separately.
trained_external_pruner=true #false # If true, loads the external pruner.
posterior_threshold=0.0001 # Posterior probability threshold for the pruner.
pruner_max_arguments=10 #20 # Maximum number of candidate heads allowed by the pruner.
labeled=true # Output semantic labels.
deterministic_labels=true # Find and impose deterministic labels.
use_dependency_syntactic_features=true # Must set to false for the SemEval 2014 closed track.
case_sensitive=false # Distinguish word upper/lower case.
model_type=af+cs #af+as+gp+cp # Parts used in the model (subset of "af+cs+gp+as+hb+np+dp").
                    # Some shortcuts are: "standard" (means "af+cs+gp");
                    # "basic" (means "af"); and "full" (means "af+cs+gp+as+hb").
                    # Currently, flags np+dp are not recommended because they
                    # make the parser a lot slower.
train_cost_false_positives=$3
train_cost_false_negatives=$4
file_format=sdp # conll

if [ "${file_format}" == "sdp" ]
then
    allow_self_loops=false
    allow_root_predicate=true
    allow_unseen_predicates=false # This should be irrelevant.
    use_predicate_senses=false #true
    formalism=$5 #pcedt #pas #dm
    subfolder=sdp/${formalism}
else
    allow_self_loops=true
    allow_root_predicate=false
    allow_unseen_predicates=false
    use_predicate_senses=true
    formalism=conll2008
    subfolder=srl
fi

if [ "${model_type}" == "basic" ]
then
    #echo "Reverting prune to false..."
    prune=false
    #echo "Stupid pruning..."
fi

#suffix=parser_pruned-${prune}_model-${model_type}
suffix=semantic_parser_${formalism}_pruned-${prune}_model-${model_type}_syntax-${use_dependency_syntactic_features}_C-${regularization_parameter}
suffix_pruner=semantic_parser_${formalism}_pruner_syntax-${use_dependency_syntactic_features}_C-${regularization_parameter_pruner}

# Set path folders.
path_bin=${root_folder} # Folder containing the binary.
path_data=${root_folder}/${subfolder}/data/${language} # Folder with the data.
path_models=${root_folder}/${subfolder}/models/${language} # Folder where models are stored.
path_results=${root_folder}/${subfolder}/results/${language} # Folder for the results.

# Create folders if they don't exist.
mkdir -p ${path_data}
mkdir -p ${path_models}
mkdir -p ${path_results}

# Set file paths. Allow multiple test files.
file_model=${path_models}/${language}_${suffix}.model
file_pruner_model=${path_models}/${language}_${suffix_pruner}.model
file_results=${path_results}/${language}_${suffix}.txt
file_pruner_results=${path_results}/${language}_${suffix_pruner}.txt

if [ "$language" == "english" ]
then
    if [ "$file_format" == "sdp" ]
    then
        file_train=${path_data}/${formalism}_augmented_train.sdp
        #file_train=${path_data}/${formalism}_augmented_dev.sdp
        files_test[0]=${path_data}/${formalism}_augmented_dev.sdp
        files_test[1]=${path_data}/${formalism}_augmented_test.sdp
        files_test[2]=${path_data}/${formalism}_augmented_train.sdp
    else
        file_train=${path_data}/${language}_train.conll2008
        files_test[0]=${path_data}/${language}_test.conll2008
        files_test[1]=${path_data}/${language}_devel.conll2008
        files_test[2]=${path_data}/${language}_test.conll2008.MST
        files_test[3]=${path_data}/${language}_devel.conll2008.MST
    fi
else
    file_train=${path_data}/${language}_train.conll2008
    file_test=${path_data}/${language}_test.conll2008
    files_test[0]=${file_test}
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_prediction=${file_test}.${suffix}.pred
    file_pruner_prediction=${file_test}.${suffix_pruner}.pred
    files_prediction[$i]=${file_prediction}
    files_pruner_prediction[$i]=${file_pruner_prediction}
done

################################################
# Train the pruner model.
################################################

if ${train_external_pruner}
then
    echo "Training pruner..."
    ${path_bin}/TurboSemanticParser \
        --train \
        --train_epochs=${num_epochs_pruner} \
        --file_model=${file_pruner_model} \
        --file_train=${file_train} \
        --model_type=basic \
        --labeled=false \
        --deterministic_labels=false \
        --use_dependency_syntactic_features=${use_dependency_syntactic_features} \
        --prune_labels=${prune_labels} \
        --prune_distances=${prune_distances} \
        --prune_basic=false \
        --only_supported_features \
        --form_case_sensitive=${case_sensitive} \
        --train_algorithm=${train_algorithm_pruner} \
        --train_regularization_constant=${regularization_parameter_pruner} \
        --train_cost_false_positives=${train_cost_false_positives} \
        --train_cost_false_negatives=${train_cost_false_negatives} \
        --allow_self_loops=${allow_self_loops} \
        --allow_root_predicate=${allow_root_predicate} \
        --allow_unseen_predicates=${allow_unseen_predicates} \
        --use_predicate_senses=${use_predicate_senses} \
        --file_format=${file_format} \
        --logtostderr

    rm -f ${file_pruner_results}
    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test=${files_test[$i]}
        file_pruner_prediction=${files_pruner_prediction[$i]}

        echo ""
        echo "Testing pruner on ${file_test}..."
        ${path_bin}/TurboSemanticParser \
            --test \
            --evaluate \
            --file_model=${file_pruner_model} \
            --file_test=${file_test} \
            --file_prediction=${file_pruner_prediction} \
            --file_format=${file_format} \
            --logtostderr

        echo ""
        echo "Evaluating pruner..."
        touch ${file_pruner_results}

        if [ "$file_format" == "sdp" ]
        then
            python remove_augmented.py ${file_pruner_prediction} > ${file_pruner_prediction}.unaugmented
            sh evaluator/run.sh Evaluator ${file_test}.unaugmented ${file_pruner_prediction}.unaugmented \
                >> ${file_pruner_results}
            cat ${file_pruner_results}
        else
            perl eval08.pl -q -g ${file_test} -s ${file_pruner_prediction} | grep -A7 'SEMANTIC SCORES:' \
                >> ${file_pruner_results}
            cat ${file_pruner_results}
        fi
    done
fi

################################################
# Train the parser.
################################################

if $train
then
    if $trained_external_pruner
    then
        # The pruner was already trained. Just set the external pruner
        # to the model that was obtained and train the parser.
        echo "Training..."
        ${path_bin}/TurboSemanticParser \
            --train \
            --train_epochs=${num_epochs} \
            --file_model=${file_model} \
            --file_train=${file_train} \
            --labeled=${labeled} \
            --deterministic_labels=${deterministic_labels} \
            --use_dependency_syntactic_features=${use_dependency_syntactic_features} \
            --prune_labels=${prune_labels} \
            --prune_distances=${prune_distances} \
            --prune_basic=${prune} \
            --pruner_posterior_threshold=${posterior_threshold} \
            --pruner_max_arguments=${pruner_max_arguments} \
            --use_pretrained_pruner \
            --file_pruner_model=${file_pruner_model} \
            --form_case_sensitive=${case_sensitive} \
            --train_algorithm=${train_algorithm} \
            --train_regularization_constant=${regularization_parameter} \
            --train_cost_false_positives=${train_cost_false_positives} \
            --train_cost_false_negatives=${train_cost_false_negatives} \
            --model_type=${model_type} \
            --allow_self_loops=${allow_self_loops} \
            --allow_root_predicate=${allow_root_predicate} \
            --allow_unseen_predicates=${allow_unseen_predicates} \
            --use_predicate_senses=${use_predicate_senses} \
            --file_format=${file_format} \
            --logtostderr
    else
        # Train a pruner along with the parser.
        ${path_bin}/TurboSemanticParser \
            --train \
            --train_epochs=${num_epochs} \
            --file_model=${file_model} \
            --file_train=${file_train} \
            --labeled=${labeled} \
            --deterministic_labels=${deterministic_labels} \
            --use_dependency_syntactic_features=${use_dependency_syntactic_features} \
            --form_case_sensitive=${case_sensitive} \
            --train_algorithm=${train_algorithm} \
            --train_regularization_constant=${regularization_parameter} \
            --train_cost_false_positives=${train_cost_false_positives} \
            --train_cost_false_negatives=${train_cost_false_negatives} \
            --model_type=${model_type} \
            --prune_labels=${prune_labels} \
            --prune_distances=${prune_distances} \
            --prune_basic=${prune} \
            --pruner_posterior_threshold=${posterior_threshold} \
            --pruner_max_arguments=${pruner_max_arguments} \
            --pruner_train_epochs=${num_epochs_pruner} \
            --pruner_train_algorithm=${train_algorithm_pruner} \
            --pruner_train_regularization_constant=${regularization_parameter_pruner} \
            --allow_self_loops=${allow_self_loops} \
            --allow_root_predicate=${allow_root_predicate} \
            --allow_unseen_predicates=${allow_unseen_predicates} \
            --use_predicate_senses=${use_predicate_senses} \
            --file_format=${file_format} \
            --logtostderr
    fi
fi

# move above:
#            --pruner_train_cost_false_positives=${train_cost_false_positives_pruner} \
#            --pruner_train_cost_false_negatives=${train_cost_false_negatives_pruner} \


################################################
# Test the parser.
################################################

if $test
then

    rm -f ${file_results}
    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test=${files_test[$i]}
        file_prediction=${files_prediction[$i]}

        echo ""
        echo "Testing on ${file_test}..."
        ${path_bin}/TurboSemanticParser \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --file_format=${file_format} \
            --logtostderr

        echo ""
        echo "Evaluating..."
        touch ${file_results}

        if [ "$file_format" == "sdp" ]
        then
            python remove_augmented.py ${file_prediction} > ${file_prediction}.unaugmented
            sh evaluator/run.sh Evaluator ${file_test}.unaugmented ${file_prediction}.unaugmented \
                >> ${file_results}
            cat ${file_results}
        else
            perl eval08.pl -q -g ${file_test} -s ${file_prediction} | grep -A7 'SEMANTIC SCORES:' \
                >> ${file_results}
            cat ${file_results}
        fi
    done
fi
