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

#ifndef ENTITYINSTANCENUMERIC_H_
#define ENTITYINSTANCENUMERIC_H_

#include "SequenceInstanceNumeric.h"
#include "EntityInstance.h"
#include "EntityDictionary.h"

class EntityInstanceNumeric : public SequenceInstanceNumeric {
public:
  EntityInstanceNumeric() {};
  virtual ~EntityInstanceNumeric() { Clear(); };

  void Clear() {
    SequenceInstanceNumeric::Clear();
    pos_ids_.clear();
    gazetteer_ids_.clear();
  }

  void Initialize(const EntityDictionary &dictionary,
                  EntityInstance *instance);

  const std::vector<int> &GetPosIds() const { return pos_ids_; }

  int GetPosId(int i) { return pos_ids_[i]; }

  const std::vector<int> &GetGazetteerIds(int i) {
    return gazetteer_ids_[i];
  }

private:
  std::vector<int> pos_ids_;
  std::vector<vector<int> > gazetteer_ids_;
};

#endif /* ENTITYINSTANCENUMERIC_H_ */
