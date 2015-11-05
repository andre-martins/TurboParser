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

#ifndef MORPHPIPE_H_
#define MORPHPIPE_H_

#include "SequencePipe.h"
#include "MorphOptions.h"
#include "MorphReader.h"
#include "MorphDictionary.h"
#include "MorphInstanceNumeric.h"
#include "MorphWriter.h"
#include "MorphFeatures.h"

class MorphPipe : public SequencePipe {
public:
  MorphPipe(Options* options) : SequencePipe(options) {}
  virtual ~MorphPipe() {}

  MorphReader *GetMorphReader() {
    return static_cast<MorphReader*>(reader_);
  };
  MorphDictionary *GetMorphDictionary() {
    return static_cast<MorphDictionary*>(dictionary_);
  };
  MorphOptions *GetMorphOptions() {
    return static_cast<MorphOptions*>(options_);
  };

 protected:
  void CreateDictionary() {
    dictionary_ = new MorphDictionary(this);
    GetSequenceDictionary()->SetTokenDictionary(token_dictionary_);
  }

  void CreateReader() { reader_ = new MorphReader(options_); }
  //void CreateWriter() { writer_ = new MorphWriter; }
  Features *CreateFeatures() { return new MorphFeatures(this); };

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    MorphInstanceNumeric *instance_numeric = new MorphInstanceNumeric;
    instance_numeric->Initialize(*GetMorphDictionary(), static_cast<MorphInstance*>(instance));
    return instance_numeric;
  }
  
protected:
  //void SaveModel(FILE* fs);
  //void LoadModel(FILE* fs);

  void GetAllowedTags(Instance *instance, int i, vector<int> *allowed_tags) {
    // Make word-tag dictionary pruning.
    allowed_tags->clear();
    bool prune_tags = GetMorphOptions()->prune_tags();
    if (!prune_tags) return;

    MorphInstanceNumeric *sentence =
      static_cast<MorphInstanceNumeric*>(instance);
    MorphDictionary *morph_dictionary = GetMorphDictionary();

    int cpostag_id = sentence->GetCPosTagId(i);
    *allowed_tags = morph_dictionary->GetCpostagMorphtags(cpostag_id);
  }

};

#endif /* MORPHPIPE_H_ */

