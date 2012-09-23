#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=english_proj
prune=true
posterior_threshold=0.0001
model_type=standard
labeled=true

# Set path folders.
path_bin=${root_folder}
path_models=${root_folder}/models/${language}

# Set file paths.
file_test=$1
file_prediction=${file_test}.pred
suffix=parser_pruned-${prune}_model-${model_type}
file_model=${path_models}/${language}_${suffix}.model

# Run the parser.
file_test=$1
file_prediction=${file_test}.pred

${path_bin}/TurboParser \
    --test \
    --evaluate \
    --file_model=${file_model} \
    --file_test=${file_test} \
    --file_prediction=${file_prediction}

