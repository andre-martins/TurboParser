// Copyright (c) 2012 Andre Martins
// All Rights Reserved.
//
// This file is part of TurboParser 2.0.
//
// TurboParser 2.0 is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// TurboParser 2.0 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with TurboParser 2.0.  If not, see <http://www.gnu.org/licenses/>.

#ifndef SPARSEPARAMETERVECTOR_H_
#define SPARSEPARAMETERVECTOR_H_

#ifdef _WIN32
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include "SerializationUtils.h"

using namespace std;

// A parameter map is a hash-table that maps from feature keys (64-bit words)
// to weights (double-precision floats).
typedef std::tr1::unordered_map <uint64_t, double> ParameterMap;

// A threshold beyond which we need to renormalize the parameter vector.
const double kScaleFactorThreshold = 1e-9;

// This class implements a sparse parameter vector, which contains a weight for
// each feature key. For fast lookup, this is implemented using an hash table.
// We represent a weight vector as a triple
// (values_, scale_factor_ , squared_norm_), where values_ contains the
// weights up to a scale, scale_factor_ is a factor such that
// weights[k] = scale_factor_ * values_[k], and the squared norm is cached.
// This way we can scale the weight vector in constant time (this operation is
// necessary in some training algorithms such as SGD), and manipulating a few
// elements is still fast. Plus, we can obtain the norm in constant time.
class SparseParameterVector {
 public:
  SparseParameterVector() { growth_stopped_ = false; };
  virtual ~SparseParameterVector() {};

  // Lock/unlock the parameter vector. If the vector is locked, no new features
  // can be inserted.
  void StopGrowth () { growth_stopped_ = true; }
  void AllowGrowth () { growth_stopped_ = false; }
  bool growth_stopped () const { return growth_stopped_; }

  // Save/load the parameters to/from a file.
  void Save(FILE *fs) {
    bool success;
    success = WriteInteger(fs, Size());
    CHECK(success);
    for (ParameterMap::const_iterator iterator = values_.begin();
         iterator != values_.end();
         ++iterator) {
      success = WriteUINT64(fs, iterator->first);
      CHECK(success);
      double value = GetValue(iterator);
      success = WriteDouble(fs, value);
      CHECK(success);
    }
  }
  void Load(FILE *fs) {
    Initialize();

    bool success;
    int length;
    success = ReadInteger(fs, &length);
    CHECK(success);
    for (int i = 0; i < length; ++i) {
      uint64_t key;
      double value;
      success = ReadUINT64(fs, &key);
      CHECK(success);
      success = ReadDouble(fs, &value);
      CHECK(success);
      Set(key, value);
    }
  }

  // Initialize to all-zeros.
  void Initialize() {
    scale_factor_ = 1.0;
    squared_norm_ = 0.0;
  }

  // Get the number of instantiated features.
  int Size() const { return values_.size(); }

  // True if this feature key is already instantiated.
  bool Exists(uint64_t key) const {
    ParameterMap::const_iterator iterator = values_.find(key);
    if (iterator == values_.end()) return false;
    return true;
  }

  // Get the weight of this feature key.
  double Get(uint64_t key) const {
    ParameterMap::const_iterator iterator = values_.find(key);
    if (iterator == values_.end()) return 0.0;
    return iterator->second * scale_factor_;
  }

  // Get the squared norm of the parameter vector.
  double GetSquaredNorm() const { return squared_norm_; }

  // Scale the parameter vector by a factor.
  // w_k' = w_k * c_k
  void Scale(double scale_factor) {
    scale_factor_ *= scale_factor;
    squared_norm_ *= scale_factor * scale_factor;
    RenormalizeIfNecessary();
  }

  // Set the parameter value of a feature pointed by an iterator.
  void SetValue(ParameterMap::iterator iterator, double value) {
    double current_value = GetValue(iterator);
    squared_norm_ += value * value - current_value * current_value;
    iterator->second = value / scale_factor_;

    // This prevents numerical issues:
    if (squared_norm_ < 0.0) squared_norm_ = 0.0;
  }

  // Get the parameter value of a feature pointed by an iterator.
  double GetValue(ParameterMap::const_iterator iterator) const {
    return iterator->second * scale_factor_;
  }

  // Obtain the iterator pointing to a feature key. If the key does not exist
  // and the parameters are not locked, inserts the key and returns the
  // corresponding iterator.
  ParameterMap::iterator FindOrInsert(uint64_t key) {
    ParameterMap::iterator iterator = values_.find(key);
    if (iterator != values_.end() || growth_stopped()) return iterator;
    pair<ParameterMap::iterator, bool> result =
      values_.insert(pair<uint64_t, double>(key, 0.0));
    CHECK(result.second);
    return result.first;
  }

  // Set the weight of this feature key to "value". Return false if the feature
  // is not instantiated and cannot be inserted.
  // w'[id] = val.
  bool Set(uint64_t key, double value) {
    ParameterMap::iterator iterator = FindOrInsert(key);
    if (iterator != values_.end()) {
      SetValue(iterator, value);
      return true;
    } else {
      return false;
    }
  }

  // Increment the weight of this feature key by an amount of "value". Return
  // false if the feature is not instantiated and cannot be inserted.
  // w'[id] = w[id] + val.
  bool Add(uint64_t key, double value) {
    ParameterMap::iterator iterator = FindOrInsert(key);
    if (iterator != values_.end()) {
      SetValue(iterator, iterator->second * scale_factor_ + value);
      return true;
    } else {
      return false;
    }
  }

  // Increments the weights of several features.
  // NOTE: Silently bypasses the ones that could not be inserted, if any.
  // w'[id] = w[id] + val.
  void Add(const vector<uint64_t> &keys, const vector<double> &values) {
    for (int i = 0; i < keys.size(); ++i) {
      Add(keys[i], values[i]);
    }
  }

  // Adds two parameter vectors. This has the effect of incrementing the weights
  // of several features.
  // NOTE: Silently bypasses the ones that could not be inserted, if any.
  // w'[id] = w[id] + val.
  void Add(const SparseParameterVector &parameters) {
    for (ParameterMap::const_iterator iterator = parameters.values_.begin();
         iterator != parameters.values_.end();
         ++iterator) {
      uint64_t key = iterator->first;
      double value = parameters.GetValue(iterator);
      CHECK_EQ(value, value);
      Add(key, value);
    }
  }

 protected:
  // If the scale factor is too small, renormalize the entire parameter map.
  void RenormalizeIfNecessary() {
    if (scale_factor_ > -kScaleFactorThreshold && 
        scale_factor_ < kScaleFactorThreshold) {
      Renormalize();
    }
  }

  // Renormalize the entire parameter map (an expensive operation).
  void Renormalize() {
    LOG(INFO) << "Renormalizing the parameter map...";
    for (ParameterMap::iterator iterator = values_.begin();
         iterator != values_.end();
         ++iterator) {
      iterator->second *= scale_factor_;
    }
    scale_factor_ = 1.0;
  }

 protected:
  ParameterMap values_; // Weight values, up to a scale.
  double scale_factor_; // The scale factor, such that w = values * scale.
  double squared_norm_; // The squared norm of the parameter vector.
  bool growth_stopped_; // True if parameters are locked.
};

#endif /*SPARSEPARAMETERVECTOR_H_*/
