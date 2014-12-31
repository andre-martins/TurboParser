#!/bin/bash

# Folder where the data will be placed.
data_folder="`cd $(dirname $0);cd ..;pwd`"
generate_test_splits=false

for language in english czech chinese
do
    if [ "${language}" == "english" ]
    then
        prefix=en
        formalisms=( "dm" "pas" "psd" )
        train_companion=../train/${prefix}.sb.bn.cpn
        test_companion=../test/${prefix}.sb.bn.cpn
    elif [ "${language}" == "czech" ]
    then
        prefix=cs
        formalisms=( "psd" )
        train_companion=""
        test_companion=""
    elif [ "${language}" == "chinese" ]
    then
        prefix=cz
        formalisms=( "pas" )
        train_companion=""
        test_companion=""
    fi

    for formalism in "${formalisms[@]}"
    do
        echo "Generating splits for ${language} ${formalism}..."
        python augment_with_companion_data.py \
            ../train/${prefix}.${formalism}.sdp \
            ${train_companion} > \
            ../train/${prefix}.${formalism}_augmented.sdp

        path_data=${data_folder}/${formalism}/data/${language}
        mkdir -p ${path_data}

        python split_data.py ../train/${prefix}.${formalism}.sdp ${prefix}_train+dev_ids > \
            ${path_data}/${language}_${formalism}_augmented_train+dev.sdp.unaugmented
        python split_data.py ../train/${prefix}.${formalism}.sdp ${prefix}_train_ids > \
            ${path_data}/${language}_${formalism}_augmented_train.sdp.unaugmented
        python split_data.py ../train/${prefix}.${formalism}.sdp ${prefix}_dev_ids > \
            ${path_data}/${language}_${formalism}_augmented_dev.sdp.unaugmented

        python split_data.py ../train/${prefix}.${formalism}_augmented.sdp ${prefix}_train+dev_ids > \
            ${path_data}/${language}_${formalism}_augmented_train+dev.sdp
        python split_data.py ../train/${prefix}.${formalism}_augmented.sdp ${prefix}_train_ids > \
            ${path_data}/${language}_${formalism}_augmented_train.sdp
        python split_data.py ../train/${prefix}.${formalism}_augmented.sdp ${prefix}_dev_ids > \
            ${path_data}/${language}_${formalism}_augmented_dev.sdp

        if ${generate_test_splits}
        then
            cp ../test/${prefix}.${formalism}.sdp \
                ${path_data}/${language}_${formalism}_augmented_test.sdp.unaugmented

            python augment_with_companion_data.py \
                ../test/${prefix}.${formalism}.sdp \
                ${test_companion} > \
                ${path_data}/${language}_${formalism}_augmented_test.sdp
        fi
    done
done
