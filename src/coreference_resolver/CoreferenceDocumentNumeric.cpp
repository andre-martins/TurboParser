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

#include "CoreferenceDocumentNumeric.h"
#include "CoreferencePipe.h"

void CoreferenceDocumentNumeric::Initialize(
    const CoreferenceDictionary &dictionary,
    CoreferenceDocument* instance) {
#if 0
  TokenDictionary *token_dictionary = dictionary.GetTokenDictionary();
  DependencyDictionary *dependency_dictionary =
    dictionary.GetDependencyDictionary();
  SemanticDictionary *semantic_dictionary =
    dictionary.GetSemanticDictionary();
  SemanticInstance *semantic_instance =
    static_cast<SemanticInstance*>(instance);
  CoreferenceOptions *options =
    static_cast<CoreferencePipe*>(dictionary.GetPipe())->GetCoreferenceOptions();
#endif

  Clear();

  sentences_.resize(instance->GetNumSentences());
  for (int i = 0; i < instance->GetNumSentences(); ++i) {
    CoreferenceSentence *sentence_instance = instance->GetSentence(i);
    CoreferenceSentenceNumeric *sentence = new CoreferenceSentenceNumeric;
    sentence->Initialize(dictionary, sentence_instance);
    sentences_[i] = sentence;
  }

  ComputeGlobalWordPositions(instance);

  // Compute coreference information.
  Alphabet coreference_labels;
  for (int i = 0; i < instance->GetNumSentences(); ++i) {
    CoreferenceSentence *sentence_instance = instance->GetSentence(i);
    const std::vector<NamedSpan*> &coreference_spans =
      sentence_instance->GetCoreferenceSpans();
    for (int k = 0; k < coreference_spans.size(); ++k) {
      const std::string &label = coreference_spans[k]->name();
      int id = coreference_labels.Insert(label);
      int start = GetDocumentPosition(i, coreference_spans[k]->start());
      int end = GetDocumentPosition(i, coreference_spans[k]->end());
      NumericSpan *span = new NumericSpan(start, end, id);
      coreference_spans_.push_back(span);
    }
  }

#if 0
  LOG(INFO) << "Found " << coreference_spans_.size()
            << " mentions organized into "
            << coreference_labels.size() << " entities.";
#endif

  // GenerateMentions()?
}

void CoreferenceDocumentNumeric::ComputeGlobalWordPositions(
    CoreferenceDocument* instance) {
  sentence_cumulative_lengths_.clear();
  sentence_cumulative_lengths_.resize(instance->GetNumSentences());
  int offset = 0;
  for (int i = 0; i < GetNumSentences(); ++i) {
    CoreferenceSentenceNumeric *sentence = GetSentence(i);
    // Subtract 1 since there is an extra start symbol.
    sentence_cumulative_lengths_[i] = offset + sentence->size() - 1;
  }
}
