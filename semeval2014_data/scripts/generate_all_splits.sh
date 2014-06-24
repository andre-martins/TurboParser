#!/bin/bash

# Folder where the data will be placed.
data_folder="`cd $(dirname $0);cd ..;pwd`"
language=english

for formalism in dm pas pcedt
do
    echo "Generating splits for ${formalism}..."
    python augment_with_companion_data.py \
        ../train/${formalism}.sdp \
        ../train/sb.bn.cpn > \
        ../train/${formalism}_augmented.sdp

    path_data=${data_folder}/${formalism}/data/${language}
    mkdir -p ${path_data}

    python split_data.py ../train/${formalism}.sdp train+dev_ids > \
        ${path_data}/${language}_${formalism}_augmented_train+dev.sdp.unaugmented
    python split_data.py ../train/${formalism}.sdp train_ids > \
        ${path_data}/${language}_${formalism}_augmented_train.sdp.unaugmented
    python split_data.py ../train/${formalism}.sdp dev_ids > \
        ${path_data}/${language}_${formalism}_augmented_dev.sdp.unaugmented

    python split_data.py ../train/${formalism}_augmented.sdp train+dev_ids > \
        ${path_data}/${language}_${formalism}_augmented_train+dev.sdp
    python split_data.py ../train/${formalism}_augmented.sdp train_ids > \
        ${path_data}/${language}_${formalism}_augmented_train.sdp
    python split_data.py ../train/${formalism}_augmented.sdp dev_ids > \
        ${path_data}/${language}_${formalism}_augmented_dev.sdp

    cp ../test/${formalism}.sdp \
        ${path_data}/${language}_${formalism}_augmented_test.sdp.unaugmented

    python augment_with_companion_data.py \
        ../test/${formalism}.sdp \
        ../test/sb.bn.cpn > \
        ${path_data}/${language}_${formalism}_augmented_test.sdp
done
