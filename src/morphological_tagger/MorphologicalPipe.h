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

#ifndef MORPHOLOGICALPIPE_H_
#define MORPHOLOGICALPIPE_H_

#include "SequencePipe.h"
#include "MorphologicalOptions.h"
#include "MorphologicalReader.h"
#include "MorphologicalDictionary.h"
#include "MorphologicalInstanceNumeric.h"
#include "MorphologicalWriter.h"
#include "MorphologicalFeatures.h"

class MorphologicalPipe : public SequencePipe {
public:
  MorphologicalPipe(Options* options) : SequencePipe(options) {}
  virtual ~MorphologicalPipe() {}

  MorphologicalReader *GetMorphologicalReader() {
    return static_cast<MorphologicalReader*>(reader_);
  };
  MorphologicalDictionary *GetMorphologicalDictionary() {
    return static_cast<MorphologicalDictionary*>(dictionary_);
  };
  MorphologicalOptions *GetMorphologicalOptions() {
    return static_cast<MorphologicalOptions*>(options_);
  };

protected:
  void CreateDictionary() {
    dictionary_ = new MorphologicalDictionary(this);
    GetSequenceDictionary()->SetTokenDictionary(token_dictionary_);
  }

  void CreateReader() { reader_ = new MorphologicalReader(options_); }
  void CreateWriter() { writer_ = new MorphologicalWriter; }
  Features *CreateFeatures() { return new MorphologicalFeatures(this); };

  void PreprocessData();

  Instance *GetFormattedInstance(Instance *instance) {
    MorphologicalInstanceNumeric *instance_numeric =
      new MorphologicalInstanceNumeric;
    instance_numeric->Initialize(*GetMorphologicalDictionary(),
                                 static_cast<MorphologicalInstance*>(instance));
    return instance_numeric;
  }

  void GetAllowedTags(Instance *instance, int i, vector<int> *allowed_tags) {
    // Make word-tag dictionary pruning.
    allowed_tags->clear();
    bool prune_tags = GetMorphologicalOptions()->prune_tags();
    if (!prune_tags) return;

    MorphologicalInstanceNumeric *sentence =
      static_cast<MorphologicalInstanceNumeric*>(instance);
    MorphologicalDictionary *morph_dictionary = GetMorphologicalDictionary();

    int cpostag_id = sentence->GetCPosTagId(i);
    *allowed_tags = morph_dictionary->GetAllowedMorphologicalTags(cpostag_id);
  }

};

#endif /* MORPHOLOGICALPIPE_H_ */

