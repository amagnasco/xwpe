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

#include "config.h"
#include <ctype.h>
#include <execinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
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

int compile_regex(regex_t *preg, const unsigned char *regular_expression,
                  const _Bool case_sensitive)
{
    if (!preg)
        return -1;

    int cflags = REG_EXTENDED | REG_NEWLINE;
    cflags = case_sensitive ? cflags : cflags | REG_ICASE;

    int errcode = regcomp(preg, (const char *) regular_expression, cflags);
    if (errcode != 0)
    {
        const size_t buff_size = 1024;
        char *err_buff;
        err_buff = malloc(buff_size * sizeof(char));
        regerror (errcode, preg, err_buff, buff_size);
        printf ("[regex search] error message: %s\n", err_buff);
        free(err_buff);
        regfree (preg);		/* Obliged internal free */
        return -1;
    }
    return 0;
}

int search_regex(regex_t *preg, const unsigned char * search_string,
                 int nr_matches, regmatch_t * matches)
{
    int errcode =
        regexec (preg, (const char *)search_string, nr_matches, matches, REG_NOTBOL | REG_NOTEOL);
    if (errcode == REG_NOMATCH)
    {
        return -1;
    }
    else if (errcode != 0)
    {
        print_regerror(errcode, preg);
        return -1;
    }
    return 0;
}

void print_regerror(int errcode, regex_t *preg)
{
    const size_t buff_size = 1024;
    char *err_buff;
    err_buff = malloc(buff_size * sizeof(char));
    regerror (errcode, preg, err_buff, buff_size);
    printf ("[regex search] error message: %s\n", err_buff);
    free(err_buff);
    return;
}

#ifndef __USE_MISC
int
strncasecmp (const char *s1, const char *s2, size_t n)
{
    if (n == 0)
        return 0;

    while (n-- != 0 && tolower(*s1) == tolower(*s2))
    {
        if (n == 0 || *s1 == '\0' || *s2 == '\0')
            break;
        s1++;
        s2++;
    }

    return tolower(*(unsigned char *) s1) - tolower(*(unsigned char *) s2);
}
#endif
