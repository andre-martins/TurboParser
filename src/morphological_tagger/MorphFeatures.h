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

#ifndef MORPHFEATURES_H_
#define MORPHFEATURES_H_

#include "SequenceFeatures.h"
#include "FeatureEncoder.h"

class MorphFeatures : public SequenceFeatures {
public:
  MorphFeatures(Pipe* pipe) : SequenceFeatures(pipe) {}
  virtual ~MorphFeatures() {};

public:
  void AddUnigramFeatures(SequenceInstanceNumeric *sentence,
    int position);

  void AddBigramFeatures(SequenceInstanceNumeric *sentence,
    int position);

  void AddTrigramFeatures(SequenceInstanceNumeric *sentence,
    int position);

protected:
  void AddFeature(uint64_t fkey, BinaryFeatures* features) {
    features->push_back(fkey);
  }

protected:
  FeatureEncoder encoder_; // Encoder that converts features into a codeword.
};

#endif /* MORPHFEATURES_H_ */
