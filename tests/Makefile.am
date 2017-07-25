## Process this file with automake to produce Makefile.in
## Normally, using `autoreconf -iv` from the project directory will automatically
## call automake.

TESTS = check_search
check_PROGRAMS = check_search
AM_LDFLAGS = -rdynamic
lib_LTLIBRARIES = libxwpe_tests.la
libxwpe_tests_la_SOURCES = check_search.c
libxwpe_tests_la_LDFLAGS = -version-info 0:0:0
check_search_SOURCES = main.c
check_search_LDADD = $(top_builddir)/src/libxwpe-term.la
check_search_LDADD += $(top_builddir)/src/libxwpe-x11.la 
check_search_LDADD += $(top_builddir)/src/libxwpe.la
check_search_LDADD += $(top_builddir)/tests/libxwpe_tests.la
