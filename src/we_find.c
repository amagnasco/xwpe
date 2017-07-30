/**
 *  we_find.c
 *
 *  Copyright (C) 2017 Guus Bonnema
 *
 *  This is free software; You can redistribute it and/or
 *  modify it under the terms of the GNU Public License version 2 or later.
 *  See the file COPYING for license information.
 *
 *  Contents of the search functions is based on a rewritten version of the original
 *  search functions in we_edit.c.
 *
 */

#include <ctype.h>
#include <string.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include "we_find.h"
#include "utils.h"

int
e_strstr (int start_offset, int end_offset,
          unsigned char *search_string, unsigned char *search_expression,
          _Bool case_sensitive)
{
    int (*compare)(const char *s1, const char *s2, size_t len);
    compare = case_sensitive ? strncmp : strncasecmp;

    int len_search_expr = strlen ((const char *) search_expression);

    /**
     *  Search forward or backward.
     *
     *  When searching backward start < end. In order to use the same algorithm
     *  for forward and backward we search from -1 * (start - 1) to -1 * end for backward searches.
     *  To determine the offset we have to use absolute value of the index.
     *
     *  Remark 1:
     *  The caller returns the starting position of the last match as the ending position.
     *  This means, that the next match may end in the ending position (backwards) but cannot
     *  start in the ending position, or it will find the old match and each subsequent would
     *  render the same result. For that reason with backward search we start one position before
     *  the end.
     *
     * */
    int start = start_offset < 0 ? 0 : start_offset;
    int end = end_offset;

    int i = start <= end ? start : -1 * (start - 1);
    int i_end = start <= end ? end - 1 : -1 * end;

    for ( ; i <= i_end; i++)
    {
        if (0 == compare((const char *) search_string + abs(i),
                         (const char *) search_expression, len_search_expr))
            return(abs(i));
    }

    // return -1 if nothing was found
    return (-1);
}

/**
 * e_rstrstr: search for a match using a regular expression.
 *
 * This method searches using a regular expression and optionally case_sensitive
 * with forward or backward search. An empty regular expression will always generate a match.
 * If that is not what you want, then don't call this function with an empty
 * regular expression.
 *
 * This match ignores submatches when using parentheses in the regular expression.
 *
 */
int
e_rstrstr (size_t start_offset,
           size_t end_offset,
           unsigned char *search_string,
           unsigned char *regular_expression,
           size_t * end_match_return, _Bool case_sensitive)
{
    _Bool forward_search = start_offset <= end_offset;

    regex_t *preg = malloc(sizeof(regex_t));

    // Compile regular expression, return -1 if compile fails.
    int errcode = compile_regex(preg, regular_expression, case_sensitive);
    if (errcode != 0)
        return -1;

    // Allocate matches.
    size_t nr_matches = preg->re_nsub + 1;
    regmatch_t *matches = malloc (nr_matches * sizeof (regmatch_t));

    //calculate pointer
    unsigned char *pstr = forward_search ?
                          search_string + start_offset : search_string + end_offset;

    // First search
    errcode = search_regex(preg, pstr, nr_matches, matches);
    if (errcode != 0)
    {
        regfree(preg);
        free(matches);
        return -1;
    }

    // Length of the result depends on the specific match of the reg expr.
    size_t len;
    // for backwards we need to find the last occurrence within the [start, end] range
    if (!forward_search)
    {
        while (errcode == 0)
        {
            len = matches[0].rm_eo - matches[0].rm_so;
            pstr = pstr + matches[0].rm_so;
            pstr++; 				 // one past the last match
            errcode = search_regex(preg, pstr, nr_matches, matches);
        }
        pstr--;					// return to the last match
    }

    // Saving the result of the forward search
    if (errcode == 0)
    {
        // set pointer and calc length
        len = matches[0].rm_eo - matches[0].rm_so;
        pstr = pstr + matches[0].rm_so;
    }

    // Now process the match found relative to the start position of searching and offset for backw.
    size_t start_match = pstr - search_string;
    *end_match_return = pstr + len - search_string;

    regfree (preg);		/* Obliged internal free */
    free (matches);
    return start_match;
}

_Bool
find_replace (unsigned int sw)
{
    return (sw & 1) != 0;
}

_Bool
find_from_cursor (unsigned int sw)
{
    return (sw & 2) == 0;
}

_Bool
find_entire_scope (unsigned int sw)
{
    return (sw & 2) != 0;
}

_Bool
find_search_forward (unsigned int sw)
{
    return (sw & 4) == 0;
}

_Bool
find_search_backward (unsigned int sw)
{
    return (sw & 4) != 0;
}

_Bool
find_global_scope (unsigned int sw)
{
    return (sw & 8) == 0;
}

_Bool
find_selection (unsigned int sw)
{
    return (sw & 8) != 0;
}

_Bool
find_confirm_replace (unsigned int sw)
{
    return (sw & 16) != 0;
}

_Bool
find_regular_expression (unsigned int sw)
{
    return (sw & 32) != 0;
}

_Bool
find_word_boundary (unsigned int sw)
{
    return (sw & 64) != 0;
}

_Bool
find_ignore_case (unsigned int sw)
{
    return (sw & 128) == 0;
}

_Bool
find_case_sensitive (unsigned int sw)
{
    return (sw & 128) != 0;
}
