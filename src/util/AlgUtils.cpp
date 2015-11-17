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

#include "AlgUtils.h"
#include "math.h"
#include <algorithm>

// Compute the transitive closure of a graph using the Floyd-Warshall algorithm.
void ComputeTransitiveClosure(vector<vector<bool> > *graph) {
  for (int k = 0; k < graph->size(); ++k) {
    // Get ascestors.
    vector<int> ancestors;
    for (int l = 0; l < graph->size(); ++l) {
      if ((*graph)[l][k]) ancestors.push_back(l);
    }
    // Get descendants.
    vector<int> descendants;
    for (int l = 0; l < graph->size(); ++l) {
      if ((*graph)[k][l]) descendants.push_back(l);
    }
    for (int i = 0; i < ancestors.size(); ++i) {
      for (int j = 0; j < descendants.size(); ++j) {
        (*graph)[ancestors[i]][descendants[j]] = true;
      }
    }
  }
}

// Implement the insertion sort algorithm. This is convenient when we need
// to sort a vector which is a perturbed version of a previously sorted vector.
// Insertion sort is better than other sort algorithms when the vector is
// almost sorted.
void InsertionSort(pair<double, int> arr[], int length) {
  int i, j;
  pair<double, int> tmp;

  for (i = 1; i < length; i++) {
    j = i;
    while (j > 0 && arr[j - 1].first > arr[j].first) {
      tmp = arr[j];
      arr[j] = arr[j - 1];
      arr[j - 1] = tmp;
      j--;
    }
  }
}

// Project d-dimensional vector x onto the set { u >= 0 | sum(u) = r }, which
// is the probability simplex when r = 1. Argument x is overwritten with the
// result of the projection.
// The algorithm has complexity O(d log d) and requires a sort. For more
// information, see the following paper:
//
// Efficient Projections onto the L1-Ball for Learning in High Dimensions
// John Duchi, Shai Shalev-Shwartz, Yoram Singer, and Tushar Chandra
// International Conference on Machine Learning (ICML 2008)
int project_onto_simplex(double* x, int d, double r) {
  int j;
  double s = 0.0;
  double tau;
  vector<double> y(d, 0.0);

  for (j = 0; j < d; j++) {
    s += x[j];
    y[j] = x[j];
  }
  sort(y.begin(), y.end());

  for (j = 0; j < d; j++) {
    tau = (s - r) / ((double)(d - j));
    if (y[j] > tau) break;
    s -= y[j];
  }

  for (j = 0; j < d; j++) {
    if (x[j] < tau) {
      x[j] = 0.0;
    } else {
      x[j] -= tau;
    }
  }

  return 0;
}

// Cached version of the algorithm in project_onto_simplex(...).
// y is a cached version of projection computed in the previous call, sorted,
// so that we have a hint for the ordering.
int project_onto_simplex_cached(double* x,
                                int d,
                                double r,
                                vector<pair<double, int> >& y) {
  int j;
  double s = 0.0;
  double tau;

  // Load x into a reordered y (the reordering is cached).
  if (y.size() != d) {
    y.resize(d);
    for (j = 0; j < d; j++) {
      s += x[j];
      y[j].first = x[j];
      y[j].second = j;
    }
    sort(y.begin(), y.end());
  } else {
    for (j = 0; j < d; j++) {
      s += x[j];
      y[j].first = x[y[j].second];
    }
    // If reordering is cached, use a sorting algorithm
    // which is fast when the vector is almost sorted.
    InsertionSort(&y[0], d);
  }

  for (j = 0; j < d; j++) {
    tau = (s - r) / ((double)(d - j));
    if (y[j].first > tau) break;
    s -= y[j].first;
  }

  for (j = 0; j < d; j++) {
    if (x[j] < tau) {
      x[j] = 0.0;
    } else {
      x[j] -= tau;
    }
  }

  return 0;
}

// Project d-dimensional vector x onto the set
// { u in R^d | u_i <= u_d, for all i < d }.
// Argument x is overwritten with the result of the projection.
// The algorithm has complexity O(d log d) and requires a sort. For more
// information, see Algorithm 10, p. 120, in:
//
// The Geometry of Constrained Structured Prediction: Applications to
// Inference and Learning of Natural Language Syntax.
// Andre Martins.
// PhD Thesis.
//
// This is the cached version of that algorithm.
// y is a cached version of projection computed in the previous call, sorted,
// so that we have a hint for the ordering.
int project_onto_cone_cached(double* x, int d,
                             vector<pair<double, int> >& y) {
  int j;
  double s = 0.0;
  double yav = 0.0;

  if (y.size() != d) {
    y.resize(d);
    for (j = 0; j < d; j++) {
      y[j].first = x[j];
      y[j].second = j;
    }
  } else {
    for (j = 0; j < d; j++) {
      if (y[j].second == d - 1 && j != d - 1) {
        y[j].second = y[d - 1].second;
        y[d - 1].second = d - 1;
      }
      y[j].first = x[y[j].second];
    }
  }
  InsertionSort(&y[0], d - 1);

  for (j = d - 1; j >= 0; j--) {
    s += y[j].first;
    yav = s / ((double)(d - j));
    if (j == 0 || yav >= y[j - 1].first) break;
  }

  for (; j < d; j++) {
    x[y[j].second] = yav;
  }

  return 0;
}
