#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
#
# Normally use autoreconf or autoreconf -i -v to process this file
# autoreconf calls all necessary autotools including autoconf, autoheader, automake etc.
# if autoreconf runs successfully, you can use ./configure to finalize the configuration.
# However, mark any warning autoreconf gives. They are not to be ignored.
#

AC_PREREQ([2.68])
AC_INIT([XWindows Programming Editor], [1.6.0], [github@github.com/amagnasco])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/keys.h])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
LT_INIT				# working with libtool
LT_PREREQ([2.4.0])		# At least version 2.4 for libtool
AM_INIT_AUTOMAKE

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
AM_CONDITIONAL([HAVE_X11], [test "X$no_x" != "Xyes"])

# Specific defines
AC_DEFINE([UNUSED(X)], [(void)(X)], [Define for arguments in a header that are not going to be used.])

# Many code blocks depend on NO_XWINDOWS being defined or not defined
if test "X${no_x}" = "Xyes"; then
	AC_DEFINE([NO_XWINDOWS], [], [X11 library is not available])
else
	AC_DEFINE([SELECTION], [], [Use X Selection events instead of XStoreBytes])
fi

# Checks for libraries.
AC_CHECK_LIB([ncurses], [tgetent])
AC_CHECK_LIB([curses], [main])
AC_CHECK_LIB([termlib], [main])
AC_CHECK_LIB([termcap], [main])
AC_CHECK_LIB([tinfo], [main])
AC_CHECK_LIB([X11], [XQueryPointer])
AC_CHECK_LIB([SM], [main])
AC_CHECK_LIB([ICE], [main])
AC_CHECK_LIB([gpm], [main])
AC_CHECK_LIB([z], [main])
AC_CHECK_LIB([check], [check])

# Check printer command: empty if necessary
AC_CHECK_PROG([PRNTCMD], [lpr], [lpr])	# preferred value for printer
AC_CHECK_PROG([PRNTCMD], [lp], [lp])	# fallback value for printer
if test -n "${PRNTCMD}"; then
	AC_DEFINE_UNQUOTED([PRNTCMD], ["${PRNTCMD}"], [Copy print command found to config.h])
else
	AC_DEFINE([PRNTCMD], [""], [Empty print command if no command found])
fi

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
AC_CHECK_FUNCS([symlink tparm mkdtemp tempnam]) 
AC_CHECK_FUNCS([getcwd memset mkdir mkfifo pathconf regcomp rmdir setlocale])
AC_CHECK_FUNCS([strchr strdup strrchr strstr])
AC_CHECK_FUNCS([backtrace])

AC_CONFIG_FILES([Makefile src/Makefile data/Makefile man/Makefile tests/Makefile])
AC_OUTPUT
