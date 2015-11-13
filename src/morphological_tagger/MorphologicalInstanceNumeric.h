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

#ifndef MORPHOLOGICALINSTANCENUMERIC_H_
#define MORPHOLOGICALINSTANCENUMERIC_H_

#include "SequenceInstanceNumeric.h"
#include "MorphologicalInstance.h"
#include "MorphologicalDictionary.h"

class MorphologicalInstanceNumeric : public SequenceInstanceNumeric {
public:
  MorphologicalInstanceNumeric() {};
  virtual ~MorphologicalInstanceNumeric() { Clear(); };

  void Clear() {
    SequenceInstanceNumeric::Clear();
    lemmas_ids_.clear();
    cpostags_ids_.clear();
  }

  void Initialize(const MorphologicalDictionary &dictionary,
                  MorphologicalInstance *instance);

  const std::vector<int> &GetCPosTagIds() const { return cpostags_ids_; }
  const std::vector<int> &GetLemmaIds(int i) const { return lemmas_ids_; }

  int GetCPosTagId(int i) { return cpostags_ids_[i]; }
  int GetLemmaId(int i) { return lemmas_ids_[i]; }


private:
  std::vector<int> lemmas_ids_;   //Lemma or stem 
  std::vector<int> cpostags_ids_; //Coarse-grained part-of-speech tag
};

#endif /* MORPHOLOGICALINSTANCENUMERIC_H_ */
