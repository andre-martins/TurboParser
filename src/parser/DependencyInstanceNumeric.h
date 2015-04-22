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

#ifndef DEPENDENCYINSTANCENUMERIC_H_
#define DEPENDENCYINSTANCENUMERIC_H_

#include <vector>
#include <string>
#include "Dictionary.h"
#include "Instance.h"
#include "DependencyInstance.h"
#include "DependencyDictionary.h"

using namespace std;

class DependencyInstanceNumeric : public Instance {
public:
  DependencyInstanceNumeric() {};
  virtual ~DependencyInstanceNumeric() { Clear(); };

  Instance* Copy() {
    CHECK(false) << "Not implemented.";
    return NULL;
  }

  int size() { return form_ids_.size(); };

  void Clear() {
    form_ids_.clear();
    form_lower_ids_.clear();
    lemma_ids_.clear();
    prefix_ids_.clear();
    suffix_ids_.clear();
    pos_ids_.clear();
    cpos_ids_.clear();
    for (int i = 0; i < feats_ids_.size(); i++) {
      feats_ids_[i].clear();
    }
    //shapes_.clear();
    is_noun_.clear();
    is_verb_.clear();
    is_punc_.clear();
    is_coord_.clear();
    heads_.clear();
  }

  void Initialize(const DependencyDictionary &dictionary,
                 DependencyInstance *instance);

#if 0
  // TODO(atm): this is repeated in other tasks. Should move some of these
  // functions to a common class and use inheritance.
  void GetWordShape(const string &word, string *shape) {
    string type = "";
    char last = '\0';
    for (int i = 0; i < word.size(); ++i) {
      if (word[i] >= 'A' && word[i] <= 'Z') {
        if (last != 'A') {
          type += 'A';
          last = 'A';
        } else if (type[type.size()-1] != '+') {
          type += '+';
        }
      }
      else if (word[i] >= 'a' && word[i] <= 'z') {
        if (last != 'a') {
          type += 'a';
          last = 'a';
        } else if (type[type.size()-1] != '+') {
          type += '+';
        }
      }
      else if (word[i] >= '0' && word[i] <= '9') {
        if (last != '0') {
          type += '0';
          last = '0';
        } else if (type[type.size()-1] != '+') {
          type += '+';
          last = '0';
        }
      } else {
        type += word[i];
      }
    }
    *shape = type;
  }
#endif

  const vector<int> &GetFormIds() const { return form_ids_; }
  const vector<int> &GetFormLowerIds() const { return form_lower_ids_; }
  const vector<int> &GetLemmaIds() const { return lemma_ids_; }
  const vector<int> &GetPosIds() const { return pos_ids_; }
  const vector<int> &GetCoarsePosIds() const { return cpos_ids_; }
  const vector<int> &GetHeads() const { return heads_; }
  const vector<int> &GetRelations() const { return relations_; }

  int GetFormId(int i) { return form_ids_[i]; };
  int GetFormLowerId(int i) { return form_lower_ids_[i]; };
  int GetLemmaId(int i) { return lemma_ids_[i]; };
  int GetPrefixId(int i) { return prefix_ids_[i]; };
  int GetSuffixId(int i) { return suffix_ids_[i]; };
  int GetPosId(int i) { return pos_ids_[i]; };
  int GetCoarsePosId(int i) { return cpos_ids_[i]; };
  int GetNumMorphFeatures(int i) { return feats_ids_[i].size(); };
  int GetMorphFeature(int i, int j) { return feats_ids_[i][j]; };
  bool IsNoun(int i) { return is_noun_[i]; };
  bool IsVerb(int i) { return is_verb_[i]; };
  bool IsPunctuation(int i) { return is_punc_[i]; };
  bool IsCoordination(int i) { return is_coord_[i]; };
  int GetHead(int i) { return heads_[i]; };
  int GetRelationId(int i) { return relations_[i]; };

 protected:
  vector<int> form_ids_;
  vector<int> form_lower_ids_;
  vector<int> lemma_ids_;
  vector<int> prefix_ids_;
  vector<int> suffix_ids_;
  vector<int> pos_ids_;
  vector<int> cpos_ids_;
  vector<vector<int> > feats_ids_;
  //vector<string> shapes_;
  vector<bool> is_noun_;
  vector<bool> is_verb_;
  vector<bool> is_punc_;
  vector<bool> is_coord_;
  vector<int> heads_;
  vector<int> relations_;
};

#endif /* DEPENDENCYINSTANCENUMERIC_H_ */
