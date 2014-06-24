model_type=af+as+cs+cp+ccp #af+as+cs+gp+cp+ccp
open=true #false

formalism=dm
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh english ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} >& out_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

formalism=pas
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh english ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} >& out_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

formalism=pcedt
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh english ${C} ${cost_fp} ${cost_fn} ${model_type} ${open} ${formalism} >& out_open-${open}_deterministic_${formalism}_${C}_${cost_fp}_${cost_fn}_${model_type}

