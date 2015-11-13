#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.59])
AC_INIT([TurboParser], [2.3.0], [afm@cs.cmu.edu], [TurboParser],
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

# Check for c++0x compile flag.
AX_CHECK_COMPILE_FLAG([-std=c++0x], [CXXFLAGS="$CXXFLAGS -std=c++0x"])

# Macros
# This will make ILOG use STL libs
#AC_DEFINE([IL_STD], [], [This is necessary to make ILOG use STL libs])

############################################
# Temporary solution to enable debug builds.
# See: http://stackoverflow.com/questions/4553735/gnu-autotools-debug-release-targets
############################################
AC_ARG_ENABLE(debug,
AS_HELP_STRING([--enable-debug],
               [enable debugging, default: no]),
[case "${enableval}" in
             yes) debug=true ;;
             no)  debug=false ;;
             *)   AC_MSG_ERROR([bad value ${enableval} for --enable-debug]) ;;
esac],
[debug=false])

AM_CONDITIONAL(DEBUG, test x"$debug" = x"true")
############################################




# Checks for header files.
AC_CHECK_HEADERS([stddef.h string.h sys/time.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE

# Checks for library functions.
AC_CHECK_FUNCS([gettimeofday])

AC_CONFIG_FILES([Makefile
                 src/tagger/Makefile
                 src/parser/Makefile
                 src/semantic_parser/Makefile
                 src/entity_recognizer/Makefile
                 src/coreference_resolver/Makefile
                 src/dependency_labeler/Makefile
                 src/constituency_labeler/Makefile
                 src/morphological_tagger/Makefile])
AC_OUTPUT
