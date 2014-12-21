#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english_proj".
train_algorithm=svm_mira # Training algorithm.
num_epochs=10 # Number of training epochs.
regularization_parameter=0.001 # The C parameter in MIRA.
train=false #true
test=true
case_sensitive=false # Distinguish word upper/lower case.
form_cutoff=0 # Cutoff in word occurrence.
lemma_cutoff=0 # Cutoff in lemma occurrence.
#projective=true #false # If true, force single-rooted projective trees.
#model_type=standard # Parts used in the model (subset of "af+cs+gp+as+hb+np+dp+gs+ts").
                    # Some shortcuts are: "standard" (means "af+cs+gp");
                    # "basic" (means "af"); and "full" (means "af+cs+gp+as+hb+gs+ts").
                    # Currently, flags np+dp are not recommended because they
                    # make the parser a lot slower.

suffix_parser=parser_pruned-true_model-standard.pred
suffix=labeler

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
file_results=${path_results}/${language}_${suffix}.txt

if [ "$language" == "english_proj" ] || [ "$language" == "english_proj_stanford" ]
then
    file_train_orig=${path_data}/${language}_train.conll.predpos
    files_test_orig[0]=${path_data}/${language}_test.conll
    files_test_orig[1]=${path_data}/${language}_dev.conll
    files_test_orig[2]=${path_data}/${language}_test.conll.predpos
    files_test_orig[3]=${path_data}/${language}_dev.conll.predpos

    file_train=${path_data}/${language}_ftags_train.conll.predpos
    files_test[0]=${path_data}/${language}_ftags_test.conll
    files_test[1]=${path_data}/${language}_ftags_dev.conll
    files_test[2]=${path_data}/${language}_ftags_test.conll.predpos
    files_test[3]=${path_data}/${language}_ftags_dev.conll.predpos

    rm -f file_train
    awk 'NF>0{OFS="\t";NF=10;$4=$5;$5=$5;print}NF==0{print}' ${file_train_orig} \
        > ${file_train}

    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test_orig=${files_test_orig[$i]}
        file_test=${files_test[$i]}
        rm -f file_test
        awk 'NF>0{OFS="\t";NF=10;$4=$5;$5=$5;print}NF==0{print}' ${file_test_orig} \
            > ${file_test}
    done
elif [ "$language" == "english" ]
then
    file_train=${path_data}/${language}_train.conll
    files_test[0]=${path_data}/${language}_test.conll
    files_test[1]=${path_data}/${language}_dev.conll
elif [ "$language" == "dutch" ]
then
    file_train=${path_data}/${language}_train.conll
    files_test[0]=${path_data}/${language}_test.conll
else
    # For all languages except english and dutch,
    # replace coarse tags by fine tags.
    file_train_orig=${path_data}/${language}_train.conll
    file_test_orig=${path_data}/${language}_test.conll
    file_train=${path_data}/${language}_ftags_train.conll
    file_test=${path_data}/${language}_ftags_test.conll
    rm -f file_train file_test
    awk 'NF>0{OFS="\t";NF=10;$4=$5;$5=$5;print}NF==0{print}' ${file_train_orig} \
        > ${file_train}
    awk 'NF>0{OFS="\t";NF=10;$4=$5;$5=$5;print}NF==0{print}' ${file_test_orig} \
        > ${file_test}
    files_test[0]=${file_test}
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_prediction=${file_test}.${suffix}.pred
    files_prediction[$i]=${file_prediction}
done


################################################
# Train the parser.
################################################

if $train
then
    ${path_bin}/TurboDependencyLabeler \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train} \
        --form_case_sensitive=${case_sensitive} \
        --form_cutoff=${form_cutoff} \
        --lemma_cutoff=${lemma_cutoff} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --logtostderr
fi


################################################
# Test the parser.
################################################

if $test
then

    rm -f ${file_results}
    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        file_test=${files_test[$i]}
        file_test_parsed=${file_test}.${suffix_parser}
        file_prediction=${files_prediction[$i]}

        echo ""
        echo "Testing on ${file_test}..."
        ${path_bin}/TurboDependencyLabeler \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test_parsed} \
            --file_prediction=${file_prediction} \
            --logtostderr

        echo ""
        echo "Evaluating..."
        touch ${file_results}
#        perl ${path_scripts}/eval.pl -b -q -g ${file_test} -s ${file_test_parsed} | tail -5 \
        perl ${path_scripts}/eval.pl -b -q -g ${file_test} -s ${file_prediction} | tail -5 \
            >> ${file_results}
        cat ${file_results}
    done
fi
