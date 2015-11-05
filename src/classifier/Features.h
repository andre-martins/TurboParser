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

#ifndef FEATURES_H_
#define FEATURES_H_

#include "Part.h"
#include "Instance.h"
#ifdef _WIN32
#include <stdint.h>
#endif
#include <string>
#include <unordered_map>
#include <functional>



// A vector of binary features. Each feature is represented as a 64-bit key.
typedef vector<uint64_t> BinaryFeatures;

class Pipe;

// Abstract class for the feature handler. Task-specific handlers should derive
// from this class and implement the pure virtual methods.
class Features {
 public:
  Features() {};
  virtual ~Features() {};

 public:
  // Set a pointer to the pipe that owns this feature handler.
  void SetPipe(Pipe *pipe) { pipe_ = pipe; };

  // Get the binary features corresponding to the r-th part.
  virtual const BinaryFeatures &GetPartFeatures(int r) const = 0;
  // Get the binary features corresponding to the r-th part (mutable).
  virtual BinaryFeatures *GetMutablePartFeatures(int r) const = 0;

 protected:
  Pipe *pipe_; // The pipe that owns this feature handler.
};


//struct  FeatureLabelPair : Structure to define a a feature-label pair
struct FeatureLabelPair {
  uint64_t feature;
  int label;
};

//struct  FeatureLabelPairMapper: Structure to define a hash function and a comparison function for FeatureLabelPair
struct FeatureLabelPairMapper {
  template <typename TSeed>
  inline void HashCombine(TSeed value, TSeed *seed) const {
    *seed ^= value + 0x9e3779b9 + (*seed << 6) + (*seed >> 2);
  }
  //Hash function
  inline size_t operator()(const FeatureLabelPair& p) const {
    size_t hash = std::hash<uint64_t>()(p.feature);
    size_t hash_2 = std::hash<int>()(p.label);

    HashCombine<size_t>(hash_2, &hash);
    return hash;
  }
  //Comparison function
  inline bool operator()(const FeatureLabelPair &p, const FeatureLabelPair &q) const {
    return p.feature == q.feature && p.label == q.label;
  }
};

//Defines a hash-table of FeatureLabelPair keys with values, of double type
typedef std::unordered_map<FeatureLabelPair, double, FeatureLabelPairMapper, FeatureLabelPairMapper > FeatureLabelPairHashMap;

//class  FeatureLabelCache: Hash-table for caching FeatureLabelPair keys with corresponding values
class FeatureLabelCache{

public:
  FeatureLabelCache() {
    hits_ = 0;
    misses_ = 0;
  };
  virtual ~FeatureLabelCache() {};
  
  int hits() const { return hits_; };
  int misses() const { return misses_; };
  int size() const { return cache_.size(); };

  void increment_hits() { hits_ += 1; };
  void increment_misses() { misses_+= 1; };

  //Insert a new pair {key, value} in the hash-table
  void insert(FeatureLabelPair key, double value) {
    cache_.insert({ key, value });
  };

  //Searches for a given key in the hash-table.
  //If found, value is returned in argument 'value'.
  //return: true if found, false otherwise
  bool find(FeatureLabelPair key, double * value) {
    FeatureLabelPairHashMap::const_iterator caching_iterator;
    caching_iterator = cache_.find(key);
    if (caching_iterator != cache_.end()) {
      *value = caching_iterator->second;
      return true;
    };
    return false;
  };

 protected:
  FeatureLabelPairHashMap cache_;
  uint64_t hits_;
  uint64_t misses_;
};


#endif /* FEATURES_H_ */
