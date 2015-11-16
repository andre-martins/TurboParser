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

#ifndef SEMANTICDICTIONARY_H_
#define SEMANTICDICTIONARY_H_

#include "Dictionary.h"
#include "TokenDictionary.h"
#include "DependencyDictionary.h"
#include "SerializationUtils.h"
#include "SemanticPredicate.h"
#include "SemanticReader.h"

class Pipe;

enum SpecialPredicates {
  PREDICATE_UNKNOWN = 0,
  NUM_SPECIAL_PREDICATES
};

enum SpecialDependencyPaths {
  PATH_UNKNOWN = 0,
  NUM_SPECIAL_PATHS
};

class SemanticDictionary : public Dictionary {
public:
  SemanticDictionary() { token_dictionary_ = NULL; }
  SemanticDictionary(Pipe* pipe) : pipe_(pipe) {}
  virtual ~SemanticDictionary() {
    Clear();
  }

  void CreatePredicateRoleDictionaries(SemanticReader *reader);

  void Clear() {
    // Don't clear token_dictionary, since this class does not own it.
    for (int i = 0; i < lemma_predicates_.size(); ++i) {
      for (int j = 0; j < lemma_predicates_[i].size(); ++j) {
        delete lemma_predicates_[i][j];
      }
      lemma_predicates_[i].clear();
    }
    lemma_predicates_.clear();
    predicate_alphabet_.clear();
    role_alphabet_.clear();
    relation_path_alphabet_.clear();
    pos_path_alphabet_.clear();
    existing_roles_.clear();
    existing_roles_with_relation_path_.clear();
    maximum_left_distances_.clear();
    maximum_right_distances_.clear();
  }

  void BuildPredicateRoleNames() {
    predicate_alphabet_.BuildNames();
    role_alphabet_.BuildNames();
    relation_path_alphabet_.BuildNames();
    pos_path_alphabet_.BuildNames();
  }

  const vector<SemanticPredicate*> &GetLemmaPredicates(int lemma) const {
    return lemma_predicates_[lemma];
  }

  const string &GetPredicateName(int predicate) const {
    return predicate_alphabet_.GetName(predicate);
  }

  const string &GetRoleName(int role) const {
    return role_alphabet_.GetName(role);
  }

  int GetRoleBigramLabel(int first_role, int second_role) const {
    CHECK_GE(first_role, 0);
    CHECK_GE(second_role, 0);
    return first_role * role_alphabet_.size() + second_role;
  }

  int GetNumRoleBigramLabels() const {
    return role_alphabet_.size()*role_alphabet_.size();
  }

  int GetNumRoles() const {
    return role_alphabet_.size();
  }

  bool IsFrequentRolePair(int first_role, int second_role) const {
    int label_bigram = GetRoleBigramLabel(first_role, second_role);
    return frequent_role_pairs_.find(label_bigram) !=
      frequent_role_pairs_.end();
  }

  bool IsRoleDeterministic(int role) const {
    return deterministic_roles_[role];
  }

  const string &GetRelationPathName(int path) const {
    return relation_path_alphabet_.GetName(path);
  }

  const string &GetPosPathName(int path) const {
    return pos_path_alphabet_.GetName(path);
  }

  // TODO(atm): check if we should allow/stop growth of the other dictionaries
  // as well.
  void AllowGrowth() { token_dictionary_->AllowGrowth(); }
  void StopGrowth() { token_dictionary_->StopGrowth(); }

  void Save(FILE *fs) {
    if (0 > predicate_alphabet_.Save(fs)) CHECK(false);
    if (0 > role_alphabet_.Save(fs)) CHECK(false);
    if (0 > relation_path_alphabet_.Save(fs)) CHECK(false);
    if (0 > pos_path_alphabet_.Save(fs)) CHECK(false);
    bool success;
    int length = lemma_predicates_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < lemma_predicates_.size(); ++i) {
      length = lemma_predicates_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < lemma_predicates_[i].size(); ++j) {
        lemma_predicates_[i][j]->Save(fs);
      }
    }
    CHECK_EQ(deterministic_roles_.size(), GetNumRoles());
    length = deterministic_roles_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < deterministic_roles_.size(); ++i) {
      bool deterministic = deterministic_roles_[i];
      success = WriteBool(fs, deterministic);
      CHECK(success);
    }
    length = existing_roles_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < existing_roles_.size(); ++i) {
      length = existing_roles_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int j = 0; j < existing_roles_[i].size(); ++j) {
        length = existing_roles_[i][j].size();
        success = WriteInteger(fs, length);
        CHECK(success);
        for (int k = 0; k < existing_roles_[i][j].size(); ++k) {
          int label = existing_roles_[i][j][k];
          success = WriteInteger(fs, label);
          CHECK(success);
        }
        int distance;
        distance = maximum_left_distances_[i][j];
        success = WriteInteger(fs, distance);
        CHECK(success);
        distance = maximum_right_distances_[i][j];
        success = WriteInteger(fs, distance);
        CHECK(success);
      }
    }
    length = existing_roles_with_relation_path_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (int i = 0; i < existing_roles_with_relation_path_.size(); ++i) {
      length = existing_roles_with_relation_path_[i].size();
      success = WriteInteger(fs, length);
      CHECK(success);
      for (int k = 0; k < existing_roles_with_relation_path_[i].size(); ++k) {
        int label = existing_roles_with_relation_path_[i][k];
        success = WriteInteger(fs, label);
        CHECK(success);
      }
    }
    length = frequent_role_pairs_.size();
    success = WriteInteger(fs, length);
    CHECK(success);
    for (set<int>::iterator it = frequent_role_pairs_.begin();
    it != frequent_role_pairs_.end();
      ++it) {
      int label_bigram = *it;
      success = WriteInteger(fs, label_bigram);
      CHECK(success);
    }
  }

  void Load(FILE *fs) {
    if (0 > predicate_alphabet_.Load(fs)) CHECK(false);
    if (0 > role_alphabet_.Load(fs)) CHECK(false);
    if (0 > relation_path_alphabet_.Load(fs)) CHECK(false);
    if (0 > pos_path_alphabet_.Load(fs)) CHECK(false);
    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    lemma_predicates_.resize(length);
    for (int i = 0; i < lemma_predicates_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      lemma_predicates_[i].resize(length);
      for (int j = 0; j < lemma_predicates_[i].size(); ++j) {
        lemma_predicates_[i][j] = new SemanticPredicate();
        lemma_predicates_[i][j]->Load(fs);
      }
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    deterministic_roles_.resize(length);
    CHECK_EQ(deterministic_roles_.size(), GetNumRoles());
    for (int i = 0; i < deterministic_roles_.size(); ++i) {
      bool deterministic;
      success = ReadBool(fs, &deterministic);
      CHECK(success);
      deterministic_roles_[i] = deterministic;
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    existing_roles_.resize(length);
    maximum_left_distances_.resize(length);
    maximum_right_distances_.resize(length);
    for (int i = 0; i < existing_roles_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      existing_roles_[i].resize(length);
      maximum_left_distances_[i].resize(length);
      maximum_right_distances_[i].resize(length);
      for (int j = 0; j < existing_roles_[i].size(); ++j) {
        success = ReadInteger(fs, &length);
        CHECK(success);
        existing_roles_[i][j].resize(length);
        for (int k = 0; k < existing_roles_[i][j].size(); ++k) {
          int label;
          success = ReadInteger(fs, &label);
          CHECK(success);
          existing_roles_[i][j][k] = label;
        }
        int distance;
        success = ReadInteger(fs, &distance);
        CHECK(success);
        maximum_left_distances_[i][j] = distance;
        success = ReadInteger(fs, &distance);
        CHECK(success);
        maximum_right_distances_[i][j] = distance;
      }
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    existing_roles_with_relation_path_.resize(length);
    for (int i = 0; i < existing_roles_with_relation_path_.size(); ++i) {
      success = ReadInteger(fs, &length);
      CHECK(success);
      existing_roles_with_relation_path_[i].resize(length);
      for (int k = 0; k < existing_roles_with_relation_path_[i].size(); ++k) {
        int label;
        success = ReadInteger(fs, &label);
        CHECK(success);
        existing_roles_with_relation_path_[i][k] = label;
      }
    }
    success = ReadInteger(fs, &length);
    CHECK(success);
    frequent_role_pairs_.clear();
    for (int k = 0; k < length; ++k) {
      int label_bigram;
      success = ReadInteger(fs, &label_bigram);
      CHECK(success);
      frequent_role_pairs_.insert(label_bigram);
    }
    BuildPredicateRoleNames();
  }

  Pipe *GetPipe() const { return pipe_; }

  TokenDictionary *GetTokenDictionary() const { return token_dictionary_; }
  void SetTokenDictionary(TokenDictionary *token_dictionary) {
    token_dictionary_ = token_dictionary;
    //CHECK(token_dictionary_ == NULL);
  }

  DependencyDictionary *GetDependencyDictionary() const {
    return dependency_dictionary_;
  }
  void SetDependencyDictionary(DependencyDictionary *dependency_dictionary) {
    dependency_dictionary_ = dependency_dictionary;
    //CHECK(token_dictionary_ == NULL);
  }

  const vector<int> &GetExistingRoles(int predicate_pos_id, int argument_pos_id) {
    return existing_roles_[predicate_pos_id][argument_pos_id];
  }

  const vector<int> &GetExistingRolesWithRelationPath(int relation_path_id) {
    return existing_roles_with_relation_path_[relation_path_id];
  }

  int GetMaximumLeftDistance(int predicate_pos_id, int argument_pos_id) {
    return maximum_left_distances_[predicate_pos_id][argument_pos_id];
  }

  int GetMaximumRightDistance(int predicate_pos_id, int argument_pos_id) {
    return maximum_right_distances_[predicate_pos_id][argument_pos_id];
  }

  const Alphabet &GetPredicateAlphabet() const { return predicate_alphabet_; };
  const Alphabet &GetRoleAlphabet() const { return role_alphabet_; };
  const Alphabet &GetRelationPathAlphabet() const {
    return relation_path_alphabet_;
  };
  const Alphabet &GetPosPathAlphabet() const { return pos_path_alphabet_; };

  void ComputeDependencyPath(SemanticInstance *instance,
                             int p, int a,
                             string *relation_path,
                             string *pos_path) const;

protected:
  int FindLowestCommonAncestor(const vector<int>& heads, int p, int a) const;

protected:
  Pipe *pipe_;
  TokenDictionary *token_dictionary_;
  DependencyDictionary *dependency_dictionary_;
  vector<vector<SemanticPredicate*> > lemma_predicates_;
  Alphabet predicate_alphabet_;
  Alphabet role_alphabet_;
  Alphabet relation_path_alphabet_;
  Alphabet pos_path_alphabet_;
  vector<bool> deterministic_roles_;
  vector<vector<vector<int> > > existing_roles_;
  vector<vector<int> > existing_roles_with_relation_path_;
  vector<vector<int> > maximum_left_distances_;
  vector<vector<int> > maximum_right_distances_;
  set<int> frequent_role_pairs_;
};

#endif /* SEMANTICDICTIONARY_H_ */
