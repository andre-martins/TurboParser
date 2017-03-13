// Copyright (c) 2012-2015 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.3.
//
// TurboParser 2.3 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.3 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.3.  If not, see <http://www.gnu.org/licenses/>.

#include "Dictionary.h"
#include "EntityDecoder.h"
#include "SequencePart.h"
#include "EntityPipe.h"
#include <iostream> // Remove this.

DEFINE_double(ner_train_cost_false_positives, 0.5,
              "Cost for 'false positives' -- penalises recall and favours precision in BIO tagging.");
DEFINE_double(ner_train_cost_false_negatives, 0.5,
              "Cost for 'false negatives' -- penalises precision and favours recall in BIO tagging.");

void EntityDecoder::DecodeCostAugmented(Instance *instance, Parts *parts,
                                          const vector<double> &scores,
                                          const vector<double> &gold_output,
                                          vector<double> *predicted_output,
                                          double *cost,
                                          double *loss) {
  
  SequenceParts *sequence_parts = static_cast<SequenceParts*>(parts);
  int offset_unigrams, num_unigrams;
  
  sequence_parts->GetOffsetUnigram(&offset_unigrams, &num_unigrams);
  
  ////////////////////////////////////////////////////
  // F1: a = 0.5, b = 0.5.
  // Recall: a = 0, b = 1.
  // In general:
  // p = a - (a+b)*z0
  // q = b*sum(z0)
  // p'*z + q = a*sum(z) - (a+b)*z0'*z + b*sum(z0)
  //          = a*(1-z0)'*z + b*(1-z)'*z0.
  ////////////////////////////////////////////////////

  // Penalty for predicting 1 when it is 0 (FP).
  double a = FLAGS_ner_train_cost_false_positives;
  // Penalty for predicting 0 when it is 1 (FN).
  double b = FLAGS_ner_train_cost_false_negatives;
  //double b = 1 - a;

  // p = 0.5-z0, q = 0.5'*z0, loss = p'*z + q
  double q = 0.0;
  vector<double> p(num_unigrams, 0.0);

  vector<double> scores_cost = scores;
  
  for (int r = 0; r < num_unigrams; ++r) {
    
    SequenceDictionary *dictionary;
    dictionary = static_cast<SequenceDictionary*>(pipe_->GetSequenceDictionary());
    SequencePartUnigram *unigram_part = (static_cast<SequencePartUnigram*>((*sequence_parts)[r]));
    int tag = unigram_part->tag();
    const std::string & tag_name = dictionary->GetTagName(tag);
    EntityOptions *entity_options = static_cast<EntityPipe*>(pipe_)->GetEntityOptions();
    if (tag_name[0] != 'O'){ // if inside (not outside)
      p[r] = a - (a+b)*gold_output[offset_unigrams + r];
      q += b*gold_output[offset_unigrams + r];
    } else {
      p[r] = 0;
    }
    scores_cost[offset_unigrams + r] += p[r];    
  }

  Decode(instance, parts, scores_cost, predicted_output);

  *cost = q;
  for (int r = 0; r < num_unigrams; ++r) {
    *cost += p[r] * (*predicted_output)[offset_unigrams + r];
  }

  *loss = *cost;
  for (int r = 0; r < parts->size(); ++r) {
    *loss += scores[r] * ((*predicted_output)[r] - gold_output[r]);
  }

}
