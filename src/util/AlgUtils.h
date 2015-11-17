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

#ifndef ALGUTILS_H
#define ALGUTILS_H

#include <vector>

using namespace std;

#define NEARLY_EQ_TOL(a,b,tol) (((a)-(b))*((a)-(b))<=(tol))
#define NEARLY_BINARY(a,tol) (NEARLY_EQ_TOL((a),1.0,(tol)) || NEARLY_EQ_TOL((a),0.0,(tol)))
#define NEARLY_ZERO_TOL(a,tol) (((a)<=(tol)) && ((a)>=(-(tol))))

#define MAX(a,b) (((a)<(b))? (b) : (a))
#define LOG_INFINITY (1000.0)
#define LOG_ZERO (-1000.0)

extern void ComputeTransitiveClosure(vector<vector<bool> > *graph);

extern void InsertionSort(pair<double, int> arr[], int length);

extern int project_onto_simplex_cached(double* x,
                                       int d,
                                       double r,
                                       vector<pair<double, int> >& y);

extern int project_onto_simplex(double* x, int d, double r);

extern int project_onto_cone_cached(double* x, int d,
                                    vector<pair<double, int> >& y);
#endif // ALGUTILS_H
