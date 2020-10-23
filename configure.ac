#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([moba-display], [0.1.0-0000], [pappi-@gmx.de])
AC_CONFIG_AUX_DIR([.])

AM_INIT_AUTOMAKE([foreign])
AC_CONFIG_SRCDIR([src])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC

AC_CONFIG_SUBDIRS([modules/lib-msghandling])


# Checks for libraries.
# FIXME: Replace `main' with a function in `-lMoBaCommon':
AC_CHECK_LIB([mobacommon], [main])
# FIXME: Replace `main' with a function in `-lboost_filesystem':
AC_CHECK_LIB([boost_filesystem], [main])
# FIXME: Replace `main' with a function in `-lboost_thread':
AC_CHECK_LIB([boost_thread], [main])
# FIXME: Replace `main' with a function in `-lboost_regex':
AC_CHECK_LIB([boost_regex], [main])
# FIXME: Replace `main' with a function in `-lboost_system':
AC_CHECK_LIB([boost_system], [main])
# FIXME: Replace `main' with a function in `-lcrypto':
AC_CHECK_LIB([crypto], [main])
# FIXME: Replace `main' with a function in `-lssl':
AC_CHECK_LIB([ssl], [main])

# Checks for header files.
AC_CHECK_HEADERS([fenv.h float.h inttypes.h limits.h locale.h stddef.h stdint.h stdlib.h string.h wchar.h wctype.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_CHECK_HEADER_STDBOOL
AC_TYPE_SIZE_T

# Checks for library functions.
AC_CHECK_FUNCS([socket])

AX_CXX_COMPILE_STDCXX_17()

AC_CONFIG_FILES([
    Makefile
    src/Makefile
])
AC_OUTPUT
