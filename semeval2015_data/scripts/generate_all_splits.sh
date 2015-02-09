#!/bin/bash

# Folder where the data will be placed.
data_folder="`cd $(dirname $0);cd ..;pwd`"
generate_test_splits=true
blind_test=false #true

for language in english czech chinese
do
    if [ "${language}" == "english" ]
    then
        prefix=en
        formalisms=( "dm" "pas" "psd" )
        use_companion=true
	domains=( "id" "ood" )
    elif [ "${language}" == "czech" ]
    then
        prefix=cs
        formalisms=( "psd" )
        use_companion=false
	domains=( "id" "ood" )
    elif [ "${language}" == "chinese" ]
    then
        prefix=cz
        formalisms=( "pas" )
        use_companion=false
	domains=( "id" )
    fi

    for formalism in "${formalisms[@]}"
    do
        echo "Generating splits for ${language} ${formalism}..."
	if ${use_companion}
	then
	    train_companion=../train/${prefix}.sb.bn.cpn
	else
	    train_companion=""
	fi
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
	    if ${blind_test}
	    then
		extension=tt
	    else
		extension=sdp
	    fi
	    for domain in "${domains[@]}"
	    do
		cp ../test/${prefix}.${domain}.${formalism}.${extension} \
                    ${path_data}/${language}_${domain}_${formalism}_augmented_test.sdp.unaugmented

		if ${use_companion}
		then
		    test_companion=../test/${prefix}.${domain}.sb.bn.cpn
		else
		    test_companion=""
		fi
		python augment_with_companion_data.py \
                    ../test/${prefix}.${domain}.${formalism}.${extension} \
                    ${test_companion} > \
                    ${path_data}/${language}_${domain}_${formalism}_augmented_test.sdp
	    done
        fi
    done
done
