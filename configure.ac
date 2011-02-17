#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# we need at least version 2.59 of autoconf
AC_PREREQ(2.59)

# name and version for package
AC_INIT(configure.ac)
AM_INIT_AUTOMAKE(nekostring, 0.1.1)

# make sure we're looking in the right place, for --srcdir
AC_CONFIG_SRCDIR([config.h.in])

# config defines go into this file
AC_CONFIG_HEADER([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET
AC_PROG_AWK
AC_PROG_LIBTOOL

# Checks for libraries.
pkg_modules="gtk+-2.0 >= 2.0.0"
PKG_CHECK_MODULES(PACKAGE, [$pkg_modules])
AC_SUBST(PACKAGE_CFLAGS)
AC_SUBST(PACKAGE_LIBS)

dnl Check for GTK+
with_gtk=no
AM_PATH_GTK_2_0(2.0.0, with_gtk='yes', AC_MSG_WARN([GUI will not be built; GTK+ 2.4 or later needed]))
AM_CONDITIONAL(BUILD_GUI, test "x${with_gtk}" != 'xno')

# FIXME: Replace `main' with a function in `-llo':
AC_CHECK_LIB([lo], [main])
# FIXME: Replace `main' with a function in `-lm':
AC_CHECK_LIB([m], [main])
# FIXME: Replace `main' with a function in `-lpthread':
AC_CHECK_LIB([pthread], [main])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memset pow strdup strstr])

AC_CONFIG_FILES([Makefile
                 gui/Makefile
                 src/Makefile])
AC_OUTPUT
