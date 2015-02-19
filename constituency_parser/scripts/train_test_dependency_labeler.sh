#!/bin/bash

# Root folder where TurboParser is installed.
root_folder="`cd $(dirname $0);cd ../..;pwd`"
task_folder="`cd $(dirname $0);cd ..;pwd`"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:${root_folder}/deps/local/lib"

# Set options.
language=$1 # Example: "slovene" or "english_proj".
train_algorithm=svm_mira # Training algorithm.
num_epochs=20 #10 #20 # Number of training epochs.
regularization_parameter=$3 #0.001 # The C parameter in MIRA.
train=true
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
delta_encoding=$4
dependency_to_constituency=true

suffix_parser=$2 #parser_pruned-true_model-full.pred
suffix=labeler

# Set path folders.
path_bin=${root_folder} # Folder containing the binary.
path_scripts_parser=${root_folder}/scripts # Folder containing scripts for the parser.
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
file_results=${path_results}/${language}_${suffix}.txt

if [ "$language" == "english_ptb" ]
then
    discontinuous=false
    evalb_bin=${path_scripts}/EVALB/evalb
    evalb_parameter_file=${path_scripts}/EVALB/COLLINS_new.prm

    file_train=${path_data}/${language}_train.trees.conll
    files_test[0]=${path_data}/${language}_test.trees.conll
    files_test[1]=${path_data}/${language}_dev.trees.conll
    files_test_parsed[0]=${path_data}/${language}_ftags_test.conll.predpos.${suffix_parser}
    files_test_parsed[1]=${path_data}/${language}_ftags_dev.conll.predpos.${suffix_parser}
    if ${delta_encoding}
    then
        file_train_transformed=${path_data}/${language}_delta_train.trees.conll
        files_test_transformed[0]=${path_data}/${language}_delta_test.trees.conll
        files_test_transformed[1]=${path_data}/${language}_delta_dev.trees.conll
        files_test_parsed_transformed[0]=${path_data}/${language}_ftags_delta_test.conll.predpos.${suffix_parser}
        files_test_parsed_transformed[1]=${path_data}/${language}_ftags_delta_dev.conll.predpos.${suffix_parser}
    fi
else
    if [ "$language" == "tgpred" ]
    then
	discontinuous=true
	discontinuous_kind=TIGER
	evalb_bin=${path_scripts}/disco-dop/discodop/eval.py
	evalb_parameter_file=${path_scripts}/evalb_spmrl2013/spmrl.prm	
	use_dev=true
    elif [ "$language" == "hnpred" ]
    then
	discontinuous=true
	discontinuous_kind=TIGER
	evalb_bin=${path_scripts}/disco-dop/discodop/eval.py
	evalb_parameter_file=${path_scripts}/disco-dop/discodop/proper.prm	
	use_dev=false
    elif [ "$language" == "ngpred" ]
    then
	discontinuous=true
	discontinuous_kind=NEGRA
	evalb_bin=${path_scripts}/disco-dop/discodop/eval.py
	evalb_parameter_file=${path_scripts}/disco-dop/discodop/proper.prm
	use_dev=false
    else
	discontinuous=false
	evalb_bin=${path_scripts}/evalb_spmrl2013/evalb_spmrl
	evalb_parameter_file=${path_scripts}/evalb_spmrl2013/spmrl.prm
	use_dev=true
    fi

    if ${use_dev}
    then
	file_train=${path_data}/${language}_train.trees.conll
	files_test[0]=${path_data}/${language}_test.trees.conll
	files_test[1]=${path_data}/${language}_dev.trees.conll
	files_test_parsed[0]=${path_data}/${language}_ftags_test.conll.${suffix_parser}
	files_test_parsed[1]=${path_data}/${language}_ftags_dev.conll.${suffix_parser}
	if ${delta_encoding}
	then
            file_train_transformed=${path_data}/${language}_delta_train.trees.conll
            files_test_transformed[0]=${path_data}/${language}_delta_test.trees.conll
            files_test_transformed[1]=${path_data}/${language}_delta_dev.trees.conll
            files_test_parsed_transformed[0]=${path_data}/${language}_ftags_delta_test.conll.${suffix_parser}
            files_test_parsed_transformed[1]=${path_data}/${language}_ftags_delta_dev.conll.${suffix_parser}
	fi
    else
	file_train=${path_data}/${language}_train.trees.conll
	files_test[0]=${path_data}/${language}_test.trees.conll
	files_test_parsed[0]=${path_data}/${language}_ftags_test.conll.${suffix_parser}
	if ${delta_encoding}
	then
            file_train_transformed=${path_data}/${language}_delta_train.trees.conll
            files_test_transformed[0]=${path_data}/${language}_delta_test.trees.conll
            files_test_parsed_transformed[0]=${path_data}/${language}_ftags_delta_test.conll.${suffix_parser}
	fi
    fi
fi

# Obtain a prediction file path for each test file.
for (( i=0; i<${#files_test[*]}; i++ ))
do
    file_test=${files_test[$i]}
    file_test_parsed=${files_test_parsed[$i]}
    file_prediction=${file_test}.${suffix}.pred
    file_prediction_parsed=${file_test_parsed}.${suffix}.pred
    files_prediction[$i]=${file_prediction}
    files_prediction_parsed[$i]=${file_prediction_parsed}
done


################################################
# Train the dependency labeler.
################################################

if $train
then
    if ${delta_encoding}
    then
        python delta_encode_labeling_indices.py ${file_train} > ${file_train_transformed}
        file_train_actual=${file_train_transformed}
    else
        file_train_actual=${file_train}
    fi

    ${path_bin}/TurboDependencyLabeler \
        --train \
        --train_epochs=${num_epochs} \
        --file_model=${file_model} \
        --file_train=${file_train_actual} \
        --form_case_sensitive=${case_sensitive} \
        --form_cutoff=${form_cutoff} \
        --lemma_cutoff=${lemma_cutoff} \
        --train_algorithm=${train_algorithm} \
        --train_regularization_constant=${regularization_parameter} \
        --logtostderr
fi

################################################
# Test the dependency labeler.
################################################

if $test
then

    rm -f ${file_results}

    # Test first with oracle backbone dependencies.
    for (( i=0; i<${#files_test[*]}; i++ ))
    do

        if ${delta_encoding}
        then
            # Convert gold to delta encoding.
            python delta_encode_labeling_indices.py ${files_test[$i]} > ${files_test_transformed[$i]}
            file_test=${files_test_transformed[$i]}
            file_prediction=${file_test}.${suffix}.pred
        else
            file_test=${files_test[$i]}
            file_prediction=${files_prediction[$i]}
        fi

        echo ""
        echo "Testing on ${file_test}..."
        ${path_bin}/TurboDependencyLabeler \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test} \
            --file_prediction=${file_prediction} \
            --logtostderr

        if ${delta_encoding}
        then
            # Convert predicted back from delta encoding.
            python delta_encode_labeling_indices.py --from_delta=True ${file_prediction} > ${files_prediction[$i]}
        fi

        echo ""
        echo "Evaluating..."
        touch ${file_results}
        perl ${path_scripts_parser}/eval.pl -b -q -g ${files_test[$i]} -s ${files_prediction[$i]} | tail -5 \
            >> ${file_results}
        cat ${file_results}

        if ${dependency_to_constituency}
        then
            # Convert gold standard file to phrases.
            # NOTE: gold standard files should have index dependencies...
            python escape_parenthesis.py ${files_test[$i]} > ${files_test[$i]}.escaped
            if ${discontinuous}
            then
		java -jar -Dfile.encoding=utf-8 disconverter.jar deconv ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp ${files_test[$i]}.conv.trees
		java -jar -Dfile.encoding=utf-8 trees2export.jar ${files_test[$i]}.conv.trees ${discontinuous_kind}
	    else
		java -jar -Dfile.encoding=utf-8 converter.jar deconv ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp 4
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp ${files_test[$i]}.conv.trees
	    fi

            # Convert predicted file to phrases.
            python escape_parenthesis.py ${files_prediction[$i]} > ${files_prediction[$i]}.escaped
            if ${discontinuous}
            then
		java -jar -Dfile.encoding=utf-8 disconverter.jar deconv ${files_prediction[$i]}.escaped ${files_prediction[$i]}.conv.trees.tmp
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_prediction[$i]}.escaped ${files_prediction[$i]}.conv.trees.tmp ${files_prediction[$i]}.conv.trees
		java -jar -Dfile.encoding=utf-8 trees2export.jar ${files_prediction[$i]}.conv.trees ${discontinuous_kind}
	    else
		java -jar -Dfile.encoding=utf-8 converter.jar deconv ${files_prediction[$i]}.escaped ${files_prediction[$i]}.conv.trees.tmp 4
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_prediction[$i]}.escaped ${files_prediction[$i]}.conv.trees.tmp ${files_prediction[$i]}.conv.trees
	    fi

	    if ${discontinuous}
	    then
                # Run EVALB.
		${evalb_bin} ${files_test[$i]}.conv.trees.export ${files_prediction[$i]}.conv.trees.export ${evalb_parameter_file} | grep '^labeled' | head -3 \
                    >> ${file_results}
	    else
                # Run EVALB.
		${evalb_bin} -p ${evalb_parameter_file} ${files_test[$i]}.conv.trees ${files_prediction[$i]}.conv.trees | grep Bracketing | head -3 \
                    >> ${file_results}
	    fi
        fi

        cat ${file_results}
    done

    # Now test with predicted backbone dependencies.
    for (( i=0; i<${#files_test[*]}; i++ ))
    do
        if ${delta_encoding}
        then
            # Convert gold to delta encoding.
            #python delta_encode_labeling_indices.py ${files_test_parsed[$i]} > ${files_test_parsed_transformed[$i]}
            file_test=${files_test_transformed[$i]}
            #file_test_parsed=${files_test_parsed_transformed[$i]}
            #file_prediction_parsed=${file_test_parsed}.${suffix}.pred
            file_test_parsed=${files_test_parsed[$i]}
            file_prediction_parsed=${files_test_parsed_transformed[$i]}.${suffix}.pred
        else
            file_test=${files_test[$i]}
            file_test_parsed=${files_test_parsed[$i]}
            file_prediction_parsed=${files_prediction_parsed[$i]}
        fi

        echo ""
        echo "Testing on ${file_test_parsed}..."
        ${path_bin}/TurboDependencyLabeler \
            --test \
            --evaluate \
            --file_model=${file_model} \
            --file_test=${file_test_parsed} \
            --file_prediction=${file_prediction_parsed} \
            --logtostderr

        if ${delta_encoding}
        then
            # Convert back from delta encoding.
            python delta_encode_labeling_indices.py --from_delta=True ${file_prediction_parsed} > ${files_prediction_parsed[$i]}
        fi

        echo ""
        echo "Evaluating..."
        touch ${file_results}
        perl ${path_scripts_parser}/eval.pl -b -q -g ${files_test[$i]} -s ${files_prediction_parsed[$i]} | tail -5 \
            >> ${file_results}
        cat ${file_results}

        if ${dependency_to_constituency}
        then
            # Convert gold standard file to phrases.
            # TODO: use original .trees files here.
            python escape_parenthesis.py ${files_test[$i]} > ${files_test[$i]}.escaped
            if ${discontinuous}
            then
		java -jar -Dfile.encoding=utf-8 disconverter.jar deconv ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp ${files_test[$i]}.conv.trees
		java -jar -Dfile.encoding=utf-8 trees2export.jar ${files_test[$i]}.conv.trees ${discontinuous_kind}
	    else
		java -jar -Dfile.encoding=utf-8 converter.jar deconv ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp 4
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_test[$i]}.escaped ${files_test[$i]}.conv.trees.tmp ${files_test[$i]}.conv.trees
	    fi

            # Convert predicted file to phrases.
            python escape_parenthesis.py ${files_prediction_parsed[$i]} > ${files_prediction_parsed[$i]}.escaped
            if ${discontinuous}
            then
		java -jar -Dfile.encoding=utf-8 disconverter.jar deconv ${files_prediction_parsed[$i]}.escaped ${files_prediction_parsed[$i]}.conv.trees.tmp
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_prediction_parsed[$i]}.escaped ${files_prediction_parsed[$i]}.conv.trees.tmp ${files_prediction_parsed[$i]}.conv.trees
		java -jar -Dfile.encoding=utf-8 trees2export.jar ${files_prediction_parsed[$i]}.conv.trees ${discontinuous_kind}
	    else
		java -jar -Dfile.encoding=utf-8 converter.jar deconv ${files_prediction_parsed[$i]}.escaped ${files_prediction_parsed[$i]}.conv.trees.tmp 4
		java -jar -Dfile.encoding=utf-8 addInfoTree.jar ${files_prediction_parsed[$i]}.escaped ${files_prediction_parsed[$i]}.conv.trees.tmp ${files_prediction_parsed[$i]}.conv.trees
	    fi

	    if ${discontinuous}
	    then
                # Run EVALB.
		${evalb_bin} ${files_test[$i]}.conv.trees.export ${files_prediction_parsed[$i]}.conv.trees.export ${evalb_parameter_file} | grep '^labeled' | head -3 \
                    >> ${file_results}
	    else
                # Run EVALB.
		${evalb_bin} -p ${evalb_parameter_file} ${files_test[$i]}.conv.trees ${files_prediction_parsed[$i]}.conv.trees | grep Bracketing | head -3 \
                    >> ${file_results}
	    fi

            echo "UNARY SCORE: `tail -1 ${file_results}`"

        fi

        cat ${file_results}
    done
fi
