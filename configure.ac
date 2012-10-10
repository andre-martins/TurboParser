#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([TurboParser], [2.0.1], [afm@cs.cmu.edu], [TurboParser],
                       [http://www.ark.cs.cmu.edu/TurboParser/])

AM_INIT_AUTOMAKE([1.10 -Wall no-define])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AUX_LIBS=../../deps/local/lib
AUX_INCLUDES=../../deps/local/include
LFLAGS="-L${AUX_LIBS} -lad3 -lgflags -lglog"
CPPFLAGS="-I${AUX_INCLUDES} -I${INCLUDES}/ad3"
AC_SUBST(LFLAGS)
AC_SUBST(CPPFLAGS)

# Macros
# This will make ILOG use STL libs
#AC_DEFINE([IL_STD], [], [This is necessary to make ILOG use STL libs])

# Checks for header files.
AC_CHECK_HEADERS([stddef.h string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday])

AC_CONFIG_FILES([Makefile
                 src/tagger/Makefile
                 src/parser/Makefile])
AC_OUTPUT
