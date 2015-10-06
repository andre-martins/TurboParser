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

#ifndef TIMEUTILS_H
#define TIMEUTILS_H

#ifdef _WIN32
#include <time.h>
#else
#include <sys/time.h>
#endif

#ifdef _WIN32
//#include <windows.h> //I've ommited this line.
#ifndef _WINSOCKAPI_
struct timeval {
	long    tv_sec;         /* seconds */
	long    tv_usec;        /* and microseconds */
};
#endif
extern int gettimeofday(struct timeval *tv, struct timezone *tz);
#endif
using namespace std;

extern int diff_ms(timeval t1, timeval t2);

extern int diff_us(timeval t1, timeval t2);

#endif // TIME_UTILS_H
