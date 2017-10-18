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
// Along with TurboParser 2.3.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TAGGERPIPE_H_
#define TAGGERPIPE_H_

#include "SequencePipe.h"
#include "TaggerOptions.h"
#include "TaggerDictionary.h"
#include "TaggerFeatures.h"

class TaggerPipe : public SequencePipe {
public:
  TaggerPipe(Options* options) : SequencePipe(options) {}
  virtual ~TaggerPipe() {}

  TaggerDictionary *GetTaggerDictionary() {
    return static_cast<TaggerDictionary*>(dictionary_);
  };
  TaggerOptions *GetTaggerOptions() {
    return static_cast<TaggerOptions*>(options_);
  };

protected:
  void CreateDictionary() {
    dictionary_ = new TaggerDictionary(this);
    GetSequenceDictionary()->SetTokenDictionary(token_dictionary_);
  }
  Features *CreateFeatures() { return new TaggerFeatures(this); };


protected:
  void GetAllowedTags(Instance *instance, int i, vector<int> *allowed_tags) {
    // Make word-tag dictionary pruning.
    allowed_tags->clear();
    bool prune_tags = GetTaggerOptions()->prune_tags();
    if (!prune_tags) return;

    SequenceInstanceNumeric *sentence =
      static_cast<SequenceInstanceNumeric*>(instance);
    TaggerDictionary *tagger_dictionary = GetTaggerDictionary();

    int word_id = sentence->GetFormId(i);
    int lexicon_word_id = sentence->GetLexiconWordId(i);
    tagger_dictionary->GetWordTags(word_id, lexicon_word_id, allowed_tags);
  }
};

#endif /* TAGGERPIPE_H_ */
