#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Normally use autoreconf or autoreconf --install -v to process this file
# autoreconf calls all necessary autotools including autoconf, autoheader, automake etc.
# if autoreconf runs successfully and without warnings, you can use ./configure to
# finalize the configuration.
#

AC_PREREQ([2.69])
AC_INIT([XWindows Programming Editor], [1.6.0], [github@github.com/amagnasco])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/keys.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile src/Makefile man/Makefile])

# Checks for programs.
AC_PROG_CC			# define CC as the current compiler
AC_PROG_LN_S			# define ln -s || ln || cp -pR
AC_PROG_INSTALL

AC_C_CONST			# define empty const if compiler does not support const

# Check for X11
# It exports variables X_CFLAGS and X_LIBS
AC_PATH_XTRA

# AC_PATH_XTRA does not give an error by default
# but X11 is optional for us, so we set a HAVE_X11 if X is available
# Remark that HAVE_LIBX11 is not an AM defined constant, but defined in config.h
AM_CONDITIONAL([HAVE_X11],[test "X$no_x" != "Xyes"])

# Many code blocks depend on NO_XWINDOWS being defined or not defined
if test "X${no_x}" = "Xyes"; then
	AC_DEFINE([NO_XWINDOWS], [], [X11 library is not available])
fi

# Checks for libraries.

AC_CHECK_LIB([ncurses], [tgetent])
AC_CHECK_LIB([X11], [XQueryPointer])
AC_CHECK_LIB([SM], [main])
AC_CHECK_LIB([ICE], [main])

# Checks for libraries: copied from old configure.in
AC_CHECK_LIB([gpm], [main])
AC_CHECK_LIB([z], [main])

# Define specific variables
AC_DEFINE([PRNTCMD], ["lpr"], [The print command to be used.])

# for we_fl_unix.c
if man -s 1 ls > /dev/null 2> /dev/null; then
 AC_DEFINE([MAN_S_OPT], [], [Is using man -s option possible?])
fi

# Checks for header files.
AC_PATH_X
AC_CHECK_HEADERS([fcntl.h limits.h locale.h stdlib.h string.h sys/ioctl.h termios.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SIZE_T

# Checks for library functions.
AC_FUNC_FORK
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([getcwd memset mkdir mkfifo pathconf regcomp rmdir setlocale strchr strdup strrchr strstr])

#
# Next 2 if-eval statements plus defines are copied from old configure.in
#      Both result in redefinition of functions. The reason is unclear.
#
if eval 'grep -s "[[ 	]]putc[[ 	(]]" /usr/include/*.h > /dev/null'; then
 :;
else
 AC_DEFINE([DEFPGC], [], [copied from old configure.in. Use is unclear.])
fi

if eval 'grep -s "[[ 	]]tputs[[ 	(]]" /usr/include/*.h > /dev/null'; then
 :;
else
 AC_DEFINE([DEFTPUTS], [], [copied from old configure.in. Use is unclear.])
fi

AC_OUTPUT
