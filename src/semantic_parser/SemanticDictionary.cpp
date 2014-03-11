// Copyright (c) 2012-2013 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.1.
//
// TurboParser 2.1 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.1 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.1.  If not, see <http://www.gnu.org/licenses/>.

#include "SemanticDictionary.h"
#include "SemanticPipe.h"

// Special symbols.
const string kPredicateUnknown = "_UNKNOWN_.01"; // Unknown predicate.
const string kPathUnknown = "_UNKNOWN_"; // Unknown path.

// Maximum alphabet sizes.
const unsigned int kMaxPredicateAlphabetSize = 0xffff;
const unsigned int kMaxRoleAlphabetSize = 0xffff;
const unsigned int kMaxRelationPathAlphabetSize = 0xffff;
const unsigned int kMaxPosPathAlphabetSize = 0xffff;

DEFINE_int32(relation_path_cutoff, 0,
             "Ignore relation paths whose frequency is less than this.");
DEFINE_int32(pos_path_cutoff, 0,
             "Ignore relation paths whose frequency is less than this.");

void SemanticDictionary::CreatePredicateRoleDictionaries(SemanticReader *reader) {
  LOG(INFO) << "Creating predicate and role dictionaries...";

  // Initialize lemma predicates.
  int num_lemmas = token_dictionary_->GetNumLemmas();
  lemma_predicates_.resize(num_lemmas);

  vector<int> role_freqs;
  vector<int> predicate_freqs;

  string special_symbols[NUM_SPECIAL_PREDICATES];
  special_symbols[PREDICATE_UNKNOWN] = kPredicateUnknown;
  for (int i = 0; i < NUM_SPECIAL_PREDICATES; ++i) {
    predicate_alphabet_.Insert(special_symbols[i]);

    // Counts of special symbols are set to -1:
    predicate_freqs.push_back(-1);
  }

  // Go through the corpus and build the predicate/roles dictionaries,
  // counting the frequencies.
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  SemanticInstance *instance =
    static_cast<SemanticInstance*>(reader->GetNext());
  int instance_length = instance->size();
  while (instance != NULL) {
    for (int k = 0; k < instance->GetNumPredicates(); ++k) {
      int i = instance->GetPredicateIndex(k);
      const std::string lemma = instance->GetLemma(i);
      const std::string predicate_name = instance->GetPredicateName(k);

      // Get the lemma integer representation.
      int lemma_id = token_dictionary_->GetLemmaId(lemma);

      // If the lemma does not exist, the predicate will not be added.
      SemanticPredicate *predicate = NULL;
      if (lemma_id >= 0) {
        // Add predicate name to alphabet.
        int predicate_id =
          predicate_alphabet_.Insert(predicate_name);
        if (predicate_id >= predicate_freqs.size()) {
          CHECK_EQ(predicate_id, predicate_freqs.size());
          predicate_freqs.push_back(0);
        }
        ++predicate_freqs[predicate_id];

        // Add predicate to the list of lemma predicates.
        std::vector<SemanticPredicate*> *predicates =
          &lemma_predicates_[lemma_id];
        for (int j = 0; j < predicates->size(); ++j) {
          if ((*predicates)[j]->id() == predicate_id) {
            predicate = (*predicates)[j];
          }
        }
        if (!predicate) {
          predicate = new SemanticPredicate(predicate_id);
          predicates->push_back(predicate);
        }
      }

      // Add semantic roles to alphabet.
      for (int l = 0; l < instance->GetNumArgumentsPredicate(k); ++l) {
        int role_id = role_alphabet_.Insert(instance->GetArgumentRole(k, l));
        if (role_id >= role_freqs.size()) {
          CHECK_EQ(role_id, role_freqs.size());
          role_freqs.push_back(0);
        }
        ++role_freqs[role_id];
        // Add this role to the predicate.
        if (predicate && !predicate->HasRole(role_id)) {
          predicate->InsertRole(role_id);
        }
      }
    }
    delete instance;
    instance = static_cast<SemanticInstance*>(reader->GetNext());
  }
  reader->Close();
  role_alphabet_.StopGrowth();

  // Take care of the special "unknown" predicate.
  bool allow_unseen_predicates =
    static_cast<SemanticPipe*>(pipe_)->GetSemanticOptions()->
       allow_unseen_predicates();
  bool use_predicate_senses =
    static_cast<SemanticPipe*>(pipe_)->GetSemanticOptions()->
       use_predicate_senses();
  if (allow_unseen_predicates || !use_predicate_senses) {
    // 1) Add the predicate as the singleton list of lemma predicates for the
    // "unknown" lemma.
    std::vector<SemanticPredicate*> *predicates =
      &lemma_predicates_[TOKEN_UNKNOWN];
    CHECK_EQ(predicates->size(), 0);
    SemanticPredicate *predicate = new SemanticPredicate(PREDICATE_UNKNOWN);
    predicates->push_back(predicate);

    // 2) Add all possible roles to the special "unknown" predicate.
    for (int role_id = 0; role_id < role_alphabet_.size(); ++role_id) {
      if (!predicate->HasRole(role_id)) predicate->InsertRole(role_id);
    }
  }

  predicate_alphabet_.StopGrowth();

  CHECK_LT(predicate_alphabet_.size(), kMaxPredicateAlphabetSize);
  CHECK_LT(role_alphabet_.size(), kMaxRoleAlphabetSize);


  // Prepare alphabets for dependency paths (relations and POS).
  vector<int> relation_path_freqs;
  Alphabet relation_path_alphabet;
  vector<int> pos_path_freqs;
  Alphabet pos_path_alphabet;

  string special_path_symbols[NUM_SPECIAL_PATHS];
  special_path_symbols[PATH_UNKNOWN] = kPathUnknown;
  for (int i = 0; i < NUM_SPECIAL_PATHS; ++i) {
    relation_path_alphabet.Insert(special_path_symbols[i]);
    pos_path_alphabet.Insert(special_path_symbols[i]);

    // Counts of special symbols are set to -1:
    relation_path_freqs.push_back(-1);
    pos_path_freqs.push_back(-1);
  }

  // Go through the corpus and build the existing labels for:
  // - each head-modifier POS pair,
  // - each syntactic path (if available).
  // Keep also the maximum left/right arc lengths for each pair of POS tags.
  existing_roles_.clear();
  existing_roles_.resize(token_dictionary_->GetNumPosTags(),
                         vector<vector<int> >(
                           token_dictionary_->GetNumPosTags()));

  vector<vector<int> > existing_roles_with_relation_path;

  maximum_left_distances_.clear();
  maximum_left_distances_.resize(token_dictionary_->GetNumPosTags(),
                                 vector<int>(
                                   token_dictionary_->GetNumPosTags(), 0));

  maximum_right_distances_.clear();
  maximum_right_distances_.resize(token_dictionary_->GetNumPosTags(),
                                  vector<int>(
                                    token_dictionary_->GetNumPosTags(), 0));

  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  instance = static_cast<SemanticInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int k = 0; k < instance->GetNumPredicates(); ++k) {
      int p = instance->GetPredicateIndex(k);
      const string &predicate_pos = instance->GetPosTag(p);
      int predicate_pos_id = token_dictionary_->GetPosTagId(predicate_pos);
      if (predicate_pos_id < 0) predicate_pos_id = TOKEN_UNKNOWN;

      // Add semantic roles to alphabet.
      for (int l = 0; l < instance->GetNumArgumentsPredicate(k); ++l) {
        int a = instance->GetArgumentIndex(k, l);
        const string &argument_pos = instance->GetPosTag(a);
        int argument_pos_id = token_dictionary_->GetPosTagId(argument_pos);
        if (argument_pos_id < 0) argument_pos_id = TOKEN_UNKNOWN;
        int role_id = role_alphabet_.Lookup(instance->GetArgumentRole(k, l));
        CHECK_GE(role_id, 0);

        // Insert new role in the set of existing labels, if it is not there
        // already. NOTE: this is inefficient, maybe we should be using a
        // different data structure.
        vector<int> &roles = existing_roles_[predicate_pos_id][argument_pos_id];
        int j;
        for (j = 0; j < roles.size(); ++j) {
          if (roles[j] == role_id) break;
        }
        if (j == roles.size()) roles.push_back(role_id);

        // Update the maximum distances if necessary.
        if (p < a) {
          // Right attachment.
          if (a - p >
              maximum_right_distances_[predicate_pos_id][argument_pos_id]) {
            maximum_right_distances_[predicate_pos_id][argument_pos_id] = a - p;
          }
        } else {
          // Left attachment (or self-loop). TODO(atm): treat self-loops differently?
          if (p - a >
              maximum_left_distances_[predicate_pos_id][argument_pos_id]) {
            maximum_left_distances_[predicate_pos_id][argument_pos_id] = p - a;
          }
        }

        // Compute the syntactic path between the predicate and the argument and
        // add it to the dictionary.
        string relation_path;
        string pos_path;
        ComputeDependencyPath(instance, p, a, &relation_path, &pos_path);
        int relation_path_id = relation_path_alphabet.Insert(relation_path);
        if (relation_path_id >= relation_path_freqs.size()) {
          CHECK_EQ(relation_path_id, relation_path_freqs.size());
          relation_path_freqs.push_back(0);
        }
        ++relation_path_freqs[relation_path_id];
        int pos_path_id = pos_path_alphabet.Insert(pos_path);
        if (pos_path_id >= pos_path_freqs.size()) {
          CHECK_EQ(pos_path_id, pos_path_freqs.size());
          pos_path_freqs.push_back(0);
        }
        ++pos_path_freqs[pos_path_id];

        // Insert new role in the set of existing labels with this relation
        // path, if it is not there already. NOTE: this is inefficient, maybe we
        // should be using a different data structure.
        if (relation_path_id >= existing_roles_with_relation_path.size()) {
          existing_roles_with_relation_path.resize(relation_path_id + 1);
        }
        vector<int> &path_roles =
          existing_roles_with_relation_path[relation_path_id];
        for (j = 0; j < path_roles.size(); ++j) {
          if (path_roles[j] == role_id) break;
        }
        if (j == path_roles.size()) path_roles.push_back(role_id);
      }
    }
    delete instance;
    instance = static_cast<SemanticInstance*>(reader->GetNext());
  }
  reader->Close();

  // Now adjust the cutoffs if necessary.
  int relation_path_cutoff = FLAGS_relation_path_cutoff;
  while (true) {
    relation_path_alphabet_.clear();
    existing_roles_with_relation_path_.clear();
    for (int i = 0; i < NUM_SPECIAL_PATHS; ++i) {
      int relation_path_id =
        relation_path_alphabet_.Insert(special_path_symbols[i]);
      vector<int> &roles = existing_roles_with_relation_path[i];
      CHECK_EQ(roles.size(), 0);
      existing_roles_with_relation_path_.push_back(roles);
      //existing_roles_with_relation_path_.push_back(vector<int>(0));
    }
    for (Alphabet::iterator iter = relation_path_alphabet.begin();
         iter != relation_path_alphabet.end();
         ++iter) {
      if (relation_path_freqs[iter->second] > relation_path_cutoff) {
        int relation_path_id = relation_path_alphabet_.Insert(iter->first);
        vector<int> &roles = existing_roles_with_relation_path[iter->second];
        existing_roles_with_relation_path_.push_back(roles);
      }
    }
    CHECK_EQ(relation_path_alphabet_.size(),
             existing_roles_with_relation_path_.size());
    if (relation_path_alphabet_.size() < kMaxRelationPathAlphabetSize) break;
    ++relation_path_cutoff;
    CHECK(false); // For now, disallowed: this would mess up the relation path filter.
    LOG(INFO) << "Incrementing relation path cutoff to "
              << relation_path_cutoff << "...";
  }

  int pos_path_cutoff = FLAGS_pos_path_cutoff;
  while (true) {
    pos_path_alphabet_.clear();
    for (int i = 0; i < NUM_SPECIAL_PATHS; ++i) {
      pos_path_alphabet_.Insert(special_path_symbols[i]);
    }
    for (Alphabet::iterator iter = pos_path_alphabet.begin();
         iter != pos_path_alphabet.end();
         ++iter) {
      if (pos_path_freqs[iter->second] > pos_path_cutoff) {
        pos_path_alphabet_.Insert(iter->first);
      }
    }
    if (pos_path_alphabet_.size() < kMaxPosPathAlphabetSize) break;
    ++pos_path_cutoff;
    LOG(INFO) << "Incrementing pos path cutoff to "
              << pos_path_cutoff << "...";
  }

  relation_path_alphabet_.StopGrowth();
  pos_path_alphabet_.StopGrowth();

  CHECK_LT(relation_path_alphabet_.size(), kMaxRelationPathAlphabetSize);
  CHECK_LT(pos_path_alphabet_.size(), kMaxPosPathAlphabetSize);

#if 0
  // Go again through the corpus to build the existing labels for:
  // - each syntactic path (if available).
  reader->Open(pipe_->GetOptions()->GetTrainingFilePath());
  instance = static_cast<SemanticInstance*>(reader->GetNext());
  while (instance != NULL) {
    int instance_length = instance->size();
    for (int k = 0; k < instance->GetNumPredicates(); ++k) {
      int p = instance->GetPredicateIndex(k);
      const string &predicate_pos = instance->GetPosTag(p);
      int predicate_pos_id = token_dictionary_->GetPosTagId(predicate_pos);
      if (predicate_pos_id < 0) predicate_pos_id = TOKEN_UNKNOWN;

      // Add semantic roles to alphabet.
      for (int l = 0; l < instance->GetNumArgumentsPredicate(k); ++l) {
        int a = instance->GetArgumentIndex(k, l);
        const string &argument_pos = instance->GetPosTag(a);
        int argument_pos_id = token_dictionary_->GetPosTagId(argument_pos);
        if (argument_pos_id < 0) argument_pos_id = TOKEN_UNKNOWN;
        int role_id = role_alphabet_.Lookup(instance->GetArgumentRole(k, l));
        CHECK_GE(role_id, 0);

        // Compute the syntactic path between the predicate and the argument and
        // add it to the dictionary.
        string relation_path;
        ComputeDependencyPath(instance, p, a, &relation_path, &pos_path);
        int relation_path_id = relation_path_alphabet_.Get(relation_path);
        CHECK_GE(relation_path_id, 0);

        // Insert new role in the set of existing labels with this relation
        // path, if it is not there already. NOTE: this is inefficient, maybe we
        // should be using a different data structure.
        if (relation_path_id >= existing_roles_with_relation_path_.size()) {
          existing_roles_with_relation_path_.resize(relation_path_id + 1);
        }
        vector<int> &path_roles = existing_roles_with_relation_path_[relation_path_id];
        for (j = 0; j < path_roles.size(); ++j) {
          if (path_roles[j] == role_id) break;
        }
        if (j == path_roles.size()) path_roles.push_back(role_id);
      }
    }
    delete instance;
    instance = static_cast<SemanticInstance*>(reader->GetNext());
  }
  reader->Close();
#endif

  // Show corpus statistics.
  LOG(INFO) << "Number of predicates: " << predicate_alphabet_.size();
  LOG(INFO) << "Number of roles: " << role_alphabet_.size();
  LOG(INFO) << "Number of relation paths: " << relation_path_alphabet_.size();
  LOG(INFO) << "Number of POS paths: " << pos_path_alphabet_.size();
}

void SemanticDictionary::ComputeDependencyPath(SemanticInstance *instance,
                                               int p, int a,
                                               string *relation_path,
                                               string *pos_path) const {
  const vector<int>& heads = instance->GetHeads();
  vector<string> relations_up;
  vector<string> relations_down;
  vector<string> pos_up;
  vector<string> pos_down;

  int ancestor = FindLowestCommonAncestor(heads, p, a);
  int h = p;
  while (ancestor != h) {
    relations_up.push_back(instance->GetDependencyRelation(h));
    pos_up.push_back(instance->GetPosTag(h));
    h = heads[h];
  }
  h = a;
  while (ancestor != h) {
    relations_down.push_back(instance->GetDependencyRelation(h));
    pos_down.push_back(instance->GetPosTag(h));
    h = heads[h];
  }

  relation_path->clear();
  pos_path->clear();
  for (int i = 0; i < relations_up.size(); ++i) {
    *relation_path += relations_up[i] + "^";
    *pos_path += pos_up[i] + "^";
  }
  *pos_path += instance->GetPosTag(ancestor);
  for (int i = relations_down.size()-1; i >= 0; --i) {
    *relation_path += relations_down[i] + "!";
    *pos_path += pos_down[i] + "!";
  }
}

int SemanticDictionary::FindLowestCommonAncestor(const vector<int>& heads,
                                                 int p, int a) const {
  vector<bool> is_ancestor(heads.size(), false);
  int h = p;
  // 0 is the root and is a common ancestor.
  while (h != 0) {
    is_ancestor[h] = true;
    h = heads[h];
  }
  h = a;
  while (h != 0) {
    if (is_ancestor[h]) return h;
    h = heads[h];
  }
  return 0;
}
