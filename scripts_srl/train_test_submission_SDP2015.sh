model_type=af+as+cs #basic #af+as+cs+gp+cp+ccp

language=czech
open=false
formalism=psd
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} sdp ${formalism} ${open} >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=chinese
open=false
formalism=pas
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} sdp ${formalism} ${open} >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=english
open=true
formalism=dm
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} sdp ${formalism} ${open} >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=english
open=true
formalism=pas
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} sdp ${formalism} ${open} >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

language=english
open=true
formalism=psd
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} sdp ${formalism} ${open} >& out_SDP2015_${language}_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

