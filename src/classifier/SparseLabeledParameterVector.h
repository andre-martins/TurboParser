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

#ifndef SPARSELABELEDPARAMETERVECTOR_H_
#define SPARSELABELEDPARAMETERVECTOR_H_

//#define USE_CUSTOMIZED_HASH_TABLE

#ifdef USE_CUSTOMIZED_HASH_TABLE
#include "HashTable.h"
#else
#ifdef _WIN32
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#endif
#include "SerializationUtils.h"

using namespace std;

// Threshold for renormalizing the parameter vector.
const double kLabeledScaleFactorThreshold = 1e-9;
// After more than kNumMaxSparseLabels labels, use a dense representation.
const int kNumMaxSparseLabels = 5;

// This class contains the weights for every label conjoined with a single
// feature. This is a pure virtual class, so that we can derive from it a
// class for sparse label sets, and another for dense label sets.
// When training, features that get conjoined with more than kNumMaxSparseLabels
// use the dense variant, while the others use the sparse variant.
class LabelWeights {
 public:
  LabelWeights() {};
  virtual ~LabelWeights() {};

  // True if set of labels is sparse.
  virtual bool IsSparse() const = 0;

  // Number of allocated labels.
  virtual int Size() const = 0;

  // Get/set/add weight to a labeled feature.
  virtual double GetWeight(int label) const = 0;
  virtual void SetWeight(int label, double weight) = 0;
  virtual void AddWeight(int label, double weight) = 0;
  virtual double SetWeightAndNormalize(int label, double value, double scaling_factor) = 0;
  virtual double AddWeightAndNormalize(int label, double value, double scaling_factor) = 0;

  // Get/set weight querying by position rather than the label.
  virtual void GetLabelWeightByPosition(int position, int *label,
      double *weight) const = 0;
  virtual void SetWeightByPosition(int position, double weight) = 0;
};

// Sparse implementation of LabelWeights.
class SparseLabelWeights : public LabelWeights {
 public:
  SparseLabelWeights() {};
  virtual ~SparseLabelWeights() {};

  bool IsSparse() const { return true; }
  int Size() const { return label_weights_.size(); }

  double GetWeight(int label) const {
    for (int k = 0; k < label_weights_.size(); ++k) {
      if (label == label_weights_[k].first) {
		return label_weights_[k].second;
	  }
    }
    return 0.0;
  }
  void SetWeight(int label, double weight) {
    for (int k = 0; k < label_weights_.size(); ++k) {
      if (label == label_weights_[k].first) {
        label_weights_[k].second = weight;
        return;
      }
    }
    label_weights_.push_back(std::pair<int,double>(label, weight));
  }
  void AddWeight(int label, double weight) {
    for (int k = 0; k < label_weights_.size(); ++k) {
      if (label == label_weights_[k].first) {
        label_weights_[k].second += weight;
        return;
      }
    }
    label_weights_.push_back(std::pair<int, double>(label, weight));
  }
  // Sets new weight, normalize it and returns previous value of label_weights_[k].second, with k such that label == label_weights_[k].first
  double SetWeightAndNormalize(int label, double value, double scaling_factor) {
	  double previous_value;
	  for (int k = 0; k < label_weights_.size(); ++k) {
		  if (label == label_weights_[k].first) {
			  previous_value = label_weights_[k].second * scaling_factor;
			  label_weights_[k].second = value / scaling_factor;
			  return previous_value;
		  }
	  }
	  label_weights_.push_back(std::pair<int, double>(label, value / scaling_factor));
	  return 0.0;
  }
  // Add weight value to current weight, normalize it and returns previous value of label_weights_[k].second, with k such that label == label_weights_[k].first
  double AddWeightAndNormalize(int label, double value, double scaling_factor) {
	  double previous_value;
	  for (int k = 0; k < label_weights_.size(); ++k) {
		  if (label == label_weights_[k].first) {
			  previous_value = label_weights_[k].second * scaling_factor;
			  label_weights_[k].second  +=  value / scaling_factor;
			  return previous_value;
		  }
	  }
	  label_weights_.push_back(std::pair<int, double>(label, value / scaling_factor ) );
	  return 0.0;
  }

  void GetLabelWeightByPosition(int position, int *label,
                                double *weight) const {
    *label = label_weights_[position].first;
    *weight = label_weights_[position].second;
    CHECK_GE(*label, 0);
  }

  void SetWeightByPosition(int position, double weight) {
    label_weights_[position].second = weight;
  }

 protected:
  std::vector<std::pair<int, double> > label_weights_;
};

// Dense implementation of LabelWeights.
class DenseLabelWeights : public LabelWeights {
 public:
  DenseLabelWeights() {};
  DenseLabelWeights(LabelWeights *label_weights) {
    CHECK(label_weights->IsSparse());
    for (int k = 0; k < label_weights->Size(); ++k) {
      int label;
      double weight;
      label_weights->GetLabelWeightByPosition(k, &label, &weight);
      CHECK_GE(label, 0);
      SetWeight(label, weight);
    }
  }
  virtual ~DenseLabelWeights() {};

  bool IsSparse() const { return false; }
  int Size() const { return weights_.size(); }

  double GetWeight(int label) const {
    if (label >= weights_.size()) return 0.0;
    return weights_[label];
  }
  void SetWeight(int label, double weight) {
    CHECK_GE(label, 0);
    if (label >= weights_.size()) {
      weights_.resize(label + 1, 0.0);
    }
    weights_[label] = weight;
  }
  void AddWeight(int label, double weight) {
    CHECK_GE(label, 0);
    if (label >= weights_.size()) {
      weights_.resize(label + 1, 0.0);
    }
    weights_[label] += weight;
  }
  // Sets new weight, normalize it and returns previous value of weights_[label]
  double SetWeightAndNormalize(int label, double value, double scaling_factor) {
	  CHECK_GE(label, 0);
	  if (label >= weights_.size()) {
		  weights_.resize(label + 1, 0.0);
	  }
	  double previous_value = weights_[label] * scaling_factor;
	  weights_[label] = value / scaling_factor;
	  return previous_value;
  }
  // Add weight value to current weight, normalize it and returns previous value of weights_[label]
  double AddWeightAndNormalize(int label, double value, double scaling_factor) {
	  CHECK_GE(label, 0);
	  if (label >= weights_.size()) {
		  weights_.resize(label + 1, 0.0);
	  }
	  double previous_value = weights_[label] * scaling_factor;
	  weights_[label] += value / scaling_factor;
	  return previous_value;
  }

  void GetLabelWeightByPosition(int position, int *label,
                                double *weight) const {
    CHECK_GE(position, 0);
    *label = position;
    *weight = weights_[position];
  }

  void SetWeightByPosition(int position, double weight) {
    weights_[position] = weight;
  }

 protected:
  vector<double> weights_;
};

// A labeled parameter map maps from feature keys ("labeled" features) to
// LabelWeights, which contain the weights of several labels conjoined with
// that feature.
#ifdef USE_CUSTOMIZED_HASH_TABLE
typedef HashTable<uint64_t, LabelWeights*> LabeledParameterMap;
#else
typedef std::tr1::unordered_map <uint64_t, LabelWeights*> LabeledParameterMap;
#endif

// This class implements a sparse parameter vector, which contains weights for
// the labels conjoined with each feature key. For fast lookup, this is
// implemented using an hash table.
// We represent a weight vector as a triple
// (values_, scale_factor_ , squared_norm_), where values_ contains the
// weights up to a scale, scale_factor_ is a factor such that
// weights[k] = scale_factor_ * values_[k], and the squared norm is cached.
// This way we can scale the weight vector in constant time (this operation is
// necessary in some training algorithms such as SGD), and manipulating a few
// elements is still fast. Plus, we can obtain the norm in constant time.
class SparseLabeledParameterVector {
 public:
  SparseLabeledParameterVector() { growth_stopped_ = false; }
  virtual ~SparseLabeledParameterVector() { Clear(); }

  // Lock/unlock the parameter vector. If the vector is locked, no new features
  // can be inserted.
  void StopGrowth () { growth_stopped_ = true; }
  void AllowGrowth () { growth_stopped_ = false; }
  bool growth_stopped () const { return growth_stopped_; }

  // Clear the parameter vector.
  void Clear() {
    for (LabeledParameterMap::iterator iterator = values_.begin();
         iterator != values_.end();
         ++iterator) {
      delete iterator->second;
    }
    values_.clear();
  }

  // Save/load the parameters to/from a file.
  void Save(FILE *fs) const {
    bool success;
    success = WriteInteger(fs, Size());
    CHECK(success);
    for (LabeledParameterMap::const_iterator iterator = values_.begin();
         iterator != values_.end();
         ++iterator) {
      success = WriteUINT64(fs, iterator->first);
      CHECK(success);
      const LabelWeights *label_weights = iterator->second;
      int length = label_weights->Size();
      success = WriteInteger(fs, length);
      CHECK(success);
      int label;
      double value;
      for (int k = 0; k < length; ++k) {
        label_weights->GetLabelWeightByPosition(k, &label, &value);
        CHECK_GE(label, 0);
        success = WriteInteger(fs, label);
        CHECK(success);
        success = WriteDouble(fs, value);
        CHECK(success);
      }
    }
  }
  void Load(FILE *fs) {
    bool success;
    int num_features;

    Initialize();
    success = ReadInteger(fs, &num_features);
    CHECK(success);
    for (int i = 0; i < num_features; ++i) {
      uint64_t key;
      success = ReadUINT64(fs, &key);
      CHECK(success);
      int length;
      success = ReadInteger(fs, &length);
      CHECK(success);
      int label;
      double value;
      for (int k = 0; k < length; ++k) {
        success = ReadInteger(fs, &label);
        CHECK(success);
        success = ReadDouble(fs, &value);
        CHECK(success);
        Set(key, label, value);
      }
    }

#define PRINT_STATISTICS
#ifdef PRINT_STATISTICS
    // Print some statistics:
    int num_sparse = 0;
    int num_total = 0;
    int num_labels_sparse = 0;
    for (LabeledParameterMap::iterator iterator = values_.begin();
         iterator != values_.end();
         ++iterator) {
      LabelWeights *label_weights = iterator->second;
      if (label_weights->IsSparse()) {
        ++num_sparse;
        int length = label_weights->Size();
        num_labels_sparse += length;
      }
      ++num_total;
    }
    LOG(INFO) << "Statistics for labeled parameter vector:";
    LOG(INFO) << "Features with sparse labels: " << num_sparse
              << " Total: " << num_total
              << " Sparse labels: " << num_labels_sparse;
#endif
  }

  // Initialize to all-zeros.
  void Initialize() {
    Clear();
    scale_factor_ = 1.0;
    squared_norm_ = 0.0;
  }

  // Get the number of instantiated features.
  // This is the number of parameters up to different labels.
  int Size() const { return values_.size(); }

  // True if this feature key is already instantiated.
  bool Exists(uint64_t key) const {
    LabeledParameterMap::const_iterator iterator = values_.find(key);
    if (iterator == values_.end()) return false;
    return true;
  }

  // Get the weights for the specified labels. Returns false if no key was
  // found, in which case weights becomes empty.
  bool Get(uint64_t key, const vector<int> &labels,
           vector<double> *weights) const {
    LabeledParameterMap::const_iterator iterator = values_.find(key);
    if (iterator == values_.end()) {
      weights->clear();
      return false;
    }
    GetValues(iterator, labels, weights);
    return true;
  }

  // Get squared norm of the parameter vector.
  double GetSquaredNorm() const { return squared_norm_; }

  // Scale the weight vector by a factor scale_factor.
  // w_k' = w_k * c_k
  void Scale(double scale_factor) {
    scale_factor_ *= scale_factor;
    squared_norm_ *= scale_factor * scale_factor;
    RenormalizeIfNecessary();
  }

  // Set the weight of this feature key conjoined with this label to "value".
  // Return false if the feature is not instantiated and cannot be inserted.
  // w'[id] = val
  bool Set(uint64_t key, int label, double value) {
    CHECK_GE(label, 0);
    LabeledParameterMap::iterator iterator = FindOrInsert(key);
    if (iterator != values_.end()) {
      SetValue(iterator, label, value);
      return true;
    } else {
      return false;
    }
  }

  // Increment the weight of this feature key conjoined with this label by an
  // amount of "value".
  // Return false if the feature is not instantiated and cannot be inserted.
  // w'[id] = w[id] + val
  bool Add(uint64_t key, int label, double value) {
    LabeledParameterMap::iterator iterator = FindOrInsert(key);
    if (iterator != values_.end()) {
      AddValue(iterator, label, value);
      return true;
    } else {
      return false;
    }
  }

  // Increment the weights of this feature key conjoined with these labels by an
  // amount of "value".
  // Return false if the feature is not instantiated and cannot be inserted.
  // w'[id] = w[id] + val
  void Add(uint64_t key, const vector<int> &labels,
           const vector<double> &values) {
    LabeledParameterMap::iterator iterator = FindOrInsert(key);
    for (int k = 0; k < labels.size(); ++k) {
      AddValue(iterator, labels[k], values[k]);
    }
  }
  // Increment the weights of these feature keys paired with these labels by an
  // amount of "value".
  // NOTE: Silently bypasses the ones that could not be inserted, if any.
  // w'[id] = w[id] + val
  void Add(const vector<uint64_t> &keys, const vector<int> &labels,
           const vector<double> &values) {
    for (int i = 0; i < keys.size(); ++i) {
      Add(keys[i], labels[i], values[i]);
    }
  }

  // Adds two parameter vectors. This has the effect of incrementing the weights
  // of several features.
  // NOTE: Silently bypasses the ones that could not be inserted, if any.
  void Add(const SparseLabeledParameterVector &parameters) {
    for (LabeledParameterMap::const_iterator iterator =
          parameters.values_.begin();
         iterator != parameters.values_.end();
         ++iterator) {
      uint64_t key = iterator->first;
      LabelWeights *label_weights = iterator->second;
      int label;
      double value;
      for (int k = 0; k < label_weights->Size(); ++k) {
        label_weights->GetLabelWeightByPosition(k, &label, &value);
        value *= parameters.scale_factor_;
        CHECK_EQ(value, value);
        //LOG(INFO) << value;
        Add(key, label, value);
      }
    }
  }

 protected:
  // Get the weights for the specified labels.
  void GetValues(LabeledParameterMap::const_iterator iterator,
                 const vector<int> &labels,
                 vector<double> *values) const {
    values->resize(labels.size());
    LabelWeights *label_weights = iterator->second;
    for (int i = 0; i < labels.size(); ++i) {
      (*values)[i] = label_weights->GetWeight(labels[i]) * scale_factor_;
    }
  }

  // Get the weight for the specified label.
  // Two versions of this function: one using a const_iterator,
  // another using an iterator.
  double GetValue(LabeledParameterMap::const_iterator iterator,
                  int label) const {
    LabelWeights *label_weights = iterator->second;
    return label_weights->GetWeight(label) * scale_factor_;
  }
  double GetValue(LabeledParameterMap::iterator iterator,
                  int label) const {
    LabelWeights *label_weights = iterator->second;
    return label_weights->GetWeight(label) * scale_factor_;
  }

  // Set the weight for the specified label.
  void SetValue(LabeledParameterMap::iterator iterator, int label,
                double value) {

	#if USE_N_OPTIMIZATIONS==0
		// TODO: Make this more efficient, avoiding two lookups in LabelWeights.
		double current_value = GetValue(iterator, label);
		squared_norm_ += value * value - current_value * current_value;
		LabelWeights *label_weights = iterator->second;
		label_weights->SetWeight(label, value / scale_factor_);
	#else
		LabelWeights *label_weights = iterator->second;
		double previous_value = label_weights->SetWeightAndNormalize(label, value, scale_factor_);
		squared_norm_ += value * value - previous_value * previous_value;
	#endif


    // If the number of labels is growing large, make this into dense 
    // label weights.
    if (label_weights->Size() > kNumMaxSparseLabels &&
        label_weights->IsSparse()) {
      DenseLabelWeights *dense_label_weights =
          new DenseLabelWeights(label_weights);
      delete label_weights;
      iterator->second = dense_label_weights;
    }

    // This prevents numerical issues:
    if (squared_norm_ < 0.0) squared_norm_ = 0.0;
  }

  // Add weight for the specified label.
  void AddValue(LabeledParameterMap::iterator iterator, int label,
                double value) {
	#if USE_N_OPTIMIZATIONS==0
    // TODO: Make this more efficient, avoiding two lookups in LabelWeights.
		double current_value = GetValue(iterator, label);
		value += current_value;
		squared_norm_ += value * value - current_value * current_value;
		LabelWeights *label_weights = iterator->second;
		if (!label_weights) 
			label_weights = new SparseLabelWeights;
		label_weights->SetWeight(label, value / scale_factor_);
	#else
		LabelWeights *label_weights = iterator->second;
		double previous_value = label_weights->AddWeightAndNormalize(label, value, scale_factor_);
		//squared_norm_ = (value + previous_value) * (value + previous_value) - previous_value * previous_value;	 //step1 
		//squared_norm_ = value * value + 2 * value * previous_value + ( previous_value * previous_value - previous_value * previous_value );	//step2
		squared_norm_ = value * value + 2 * value * previous_value;		//step3
	#endif


    // If the number of labels is growing large, make this into dense 
    // label weights.
    if (label_weights->Size() > kNumMaxSparseLabels &&
        label_weights->IsSparse()) {
      DenseLabelWeights *dense_label_weights =
          new DenseLabelWeights(label_weights);
      delete label_weights;
      iterator->second = dense_label_weights;
    }

    // This prevents numerical issues:
    if (squared_norm_ < 0.0) squared_norm_ = 0.0;
  }

  // Find a key, or insert it in case it does not exist.
  LabeledParameterMap::iterator FindOrInsert(uint64_t key) {
    LabeledParameterMap::iterator iterator = values_.find(key);
    if (iterator != values_.end() || growth_stopped()) return iterator;
    LabelWeights *label_weights = new SparseLabelWeights;
    pair<LabeledParameterMap::iterator, bool> result =
      values_.insert(pair<uint64_t, LabelWeights*>(key, label_weights));
    CHECK(result.second);
    return result.first;
  }

  // If the scale factor is too small, renormalize the entire parameter map.
  void RenormalizeIfNecessary() {
    if (scale_factor_ > -kLabeledScaleFactorThreshold &&
        scale_factor_ < kLabeledScaleFactorThreshold) {
      Renormalize();
    }
  }
  
  // Renormalize the entire parameter map (an expensive operation).
  void Renormalize() {
    LOG(INFO) << "Renormalizing the parameter map...";
    for (LabeledParameterMap::iterator iterator = values_.begin();
         iterator != values_.end();
         ++iterator) {
      LabelWeights *label_weights = iterator->second;
      int label;
      double value;
      for (int k = 0; k < label_weights->Size(); ++k) {
        label_weights->GetLabelWeightByPosition(k, &label, &value);
        label_weights->SetWeightByPosition(k, value * scale_factor_);
      }
    }
    scale_factor_ = 1.0;
  }

 protected:
  LabeledParameterMap values_; // Weight values, up to a scale.
  double scale_factor_; // The scale factor, such that w = values * scale.
  double squared_norm_; // The squared norm of the parameter vector.
  bool growth_stopped_; // True if parameters are locked.
};

#endif /*SPARSELABELEDPARAMETERVECTOR_H_*/
