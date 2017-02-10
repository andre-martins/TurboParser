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

#ifndef DEPENDENCYINSTANCE_H_
#define DEPENDENCYINSTANCE_H_

#include <string>
#include <vector>
#include "Instance.h"

class DependencyInstance : public Instance {
public:
  DependencyInstance() {};
  virtual ~DependencyInstance() {};

  Instance* Copy() {
    DependencyInstance* instance = new DependencyInstance();
    instance->Initialize(forms_, lemmas_, cpostags_, postags_,
                         feats_, deprels_, heads_);
    return static_cast<Instance*>(instance);
  }

  void Initialize(const vector<string> &forms,
                  const vector<string> &lemmas,
                  const vector<string> &cpos,
                  const vector<string> &pos,
                  const vector<vector<string> > &feats,
                  const vector<string> &deprels,
                  const vector<int> &heads);

  int size() { return (int) forms_.size(); };

  const vector<int> &GetHeads() { return heads_; }
  const vector<string> &GetDependencyRelations() { return deprels_; }

  const string &GetForm(int i) { return forms_[i]; };
  const string &GetLemma(int i) { return lemmas_[i]; };
  const string &GetCoarsePosTag(int i) { return cpostags_[i]; };
  const string &GetPosTag(int i) { return postags_[i]; };
  int GetNumMorphFeatures(int i) { return (int)(feats_[i].size()); };
  const string &GetMorphFeature(int i, int j) { return feats_[i][j]; };
  int GetHead(int i) { return heads_[i]; };
  const string &GetDependencyRelation(int i) { return deprels_[i]; };

  void SetHead(int i, int head) { heads_[i] = head; }
  void SetDependencyRelation(int i, const string &dependency_relation) {
    deprels_[i] = dependency_relation;
  }

protected:
  // FORM: the forms - usually words, like "thought"
  vector<string> forms_;
  // LEMMA: the lemmas, or stems, e.g. "think"
  vector<string> lemmas_;
  // COURSE-POS: the course part-of-speech tags, e.g."V"
  vector<string> cpostags_;
  // FINE-POS: the fine-grained part-of-speech tags, e.g."VBD"
  vector<string> postags_;
  // FEATURES: some features associated with the elements separated by "|", e.g. "PAST|3P"
  vector<vector<string> > feats_;
  // HEAD: the IDs of the heads for each element
  vector<int> heads_;
  // DEPREL: the dependency relations, e.g. "SUBJ"
  vector<string> deprels_;
};

#endif /* DEPENDENCYINSTANCE_H_*/
