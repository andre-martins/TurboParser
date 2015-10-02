language=$1
model_type=basic
open=true
C=0.01
cost_fp=0.4
cost_fn=0.6
echo "${language} ${C} ${cost_fp} ${cost_fn} ${formalism} ${model_type} ${open}"
./train_test_semantic_parser.sh ${language} ${C} ${cost_fp} ${cost_fn} ${model_type} conll conll2008 ${open} >& out_${language}_open-${open}_deterministic_${C}_${cost_fp}_${cost_fn}_${model_type}

