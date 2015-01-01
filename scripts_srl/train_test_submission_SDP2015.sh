model_type=basic #af+as+cs+gp+cp+ccp
open=true

language=english
formalism=dm
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} sdp >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=english
formalism=pas
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} sdp >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=english
formalism=psd
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} sdp >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=chinese
formalism=pas
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} sdp >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=czech
formalism=psd
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} sdp >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}
