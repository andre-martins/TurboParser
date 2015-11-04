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

#ifndef MORPHINSTANCE_H_
#define MORPHINSTANCE_H_

#include "SequenceInstance.h"

class MorphInstance : public SequenceInstance {
public:
  MorphInstance() {}
  virtual ~MorphInstance() {}

  Instance* Copy() {
    MorphInstance* instance = new MorphInstance();
    instance->Initialize(forms_, 
                        lemmas_, 
                        cpostags_, 
                        tags_); //feats_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const std::vector<std::string> &forms,
                  const std::vector<std::string> &lemmas,
                  const std::vector<std::string> &cpostags,
                  const std::vector<std::string> &tags); //&feats);

  const std::string &GetLemma(int i) const { return lemmas_[i]; }
  const std::string &GetCoarsePosTag(int i) const { return cpostags_[i]; }
  //const std::string &GetMorphFeat(int i) const { return feats_[i]; }


protected:
  //from sequenceInstance: std::vector<std::string> forms_;
  std::vector<std::string> lemmas_;
  std::vector<std::string> cpostags_;
  //std::vector<std::string> feats_; //using sequenceInstance: std::vector<std::string> tags_
  //assuming following terminology: morphs = morph_feats = feats = tags in this case
};

#endif /* MORPHINSTANCE_H_*/
