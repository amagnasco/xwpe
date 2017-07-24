## Process this file with automake to produce Makefile.in
## Normally, using `autoreconf -iv` from the project directory will automatically
## call automake.

TESTS = check_search
check_PROGRAMS = check_search
check_search_SOURCES = check_search.c
##check_search_HEADERS = check_search.h
##check_search_CFLAGS = @CHECK_CFLAGS@
check_search_LDADD = $(top_builddir)/src/libxwpe-term.la
check_search_LDADD += $(top_builddir)/src/libxwpe-x11.la 
check_search_LDADD += $(top_builddir)/src/libxwpe.la
