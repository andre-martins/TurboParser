#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=english_proj
suffix=tagger

# Set path folders.
path_bin=${root_folder}
path_models=${root_folder}/models/${language}

# Set file paths. Allow multiple test files.
file_model=${path_models}/${language}_${suffix}.model

# Run the tagger.
file_test=$1
file_prediction=${file_test}.pred

${path_bin}/TurboTagger \
    --test \
    --file_model=${file_model} \
    --file_test=${file_test} \
    --file_prediction=${file_prediction}


