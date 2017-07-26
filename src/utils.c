/**
 * utils.c
 *
 * Copyright Fred Kruse, Guus Bonnema.
 *
 * User License GPL v 2 (see file COPYING)
 *
 * modification 2017: renamed from makro.h to utils.h and utils.c
 * 			replaced macros isalnum1 and isalpha1 by functions.
 *
 */

#include <ctype.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "utils.h"

int
isalnum1 (int x)
{
    return (isalnum (x) || x == '_');
}

int
isalpha1 (int x)
{
    return (isalpha (x) || x == '_');
}

/**
 * function print_stacktrace()
 *
 * Prints the backtrace to standard output. It uses backtrace() and backtrace_symbols.
 *
 */
void
print_stacktrace ()
{
    char **strings;
    const size_t max_nr_pointers = 100;
    void *buff[max_nr_pointers];

#ifndef HAVE_BACKTRACE
    printf ("backtrace() not available on this system.\n");
    return;
#endif
    int nptrs = backtrace (buff, max_nr_pointers);
    printf ("backtrace() returned %d addresses.\n", nptrs);

    strings = backtrace_symbols (buff, nptrs);
    if (strings == NULL)
    {
        perror
        ("backtrace_symbols had a problem returning the stacktrace symbols.");
        exit (EXIT_FAILURE);
    }

    for (int i = 0; i < nptrs; i++)
    {
        printf ("%s\n", strings[i]);
    }

    // Remark: only free the toplevel strings, not the string below (see man page).
    free (strings);
    return;
}
