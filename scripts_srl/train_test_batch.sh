cost_fp=0.5
cost_fn=0.5
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}

cost_fp=0.8
cost_fn=0.2
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}

cost_fp=0.2
cost_fn=0.8
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}

cost_fp=0.4
cost_fn=0.6
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}

cost_fp=0.6
cost_fn=0.4
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}

cost_fp=1.0
cost_fn=1.0
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}

cost_fp=0.1
cost_fn=0.1
./train_test_semantic_parser.sh english ${cost_fp} ${cost_fn} >& out_${cost_fp}_${cost_fn}
