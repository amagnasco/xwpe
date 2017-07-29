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

	for ( ; i <= i_end; i++) {
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
 * with forward or backward search.
 * 
 * This match ignores submatches when using parentheses.
 * 
 */
int
e_rstrstr (size_t start_offset,
           size_t end_offset,
           unsigned char *search_string,
           unsigned char *regular_expression,
           size_t * end_match, _Bool case_sensitive)
{
	// TODO: start > end means backward search
	// Adapt regex search to this phenomenon
	_Bool search_forward = start_offset <= end_offset;
    regex_t *preg = malloc(sizeof(regex_t));
    regmatch_t *matches;  // is set directly after compile
    size_t start, end, len_search_str;
    size_t nr_matches;
    int start_match;

    // copy input variables
    len_search_str = strlen ((const char *) search_string);
    start = start_offset;
    end = end_offset;

    unsigned char str[len_search_str + 1];
    /* copy excluding null byte */
    strncpy ((char *) str, (const char *) search_string, len_search_str);
    str[len_search_str] = '\0';

    // Compile regular expression, return -1 if compile fails.
	int errcode = compile_regex(preg, regular_expression, case_sensitive);
	if (errcode != 0)
		return -1;

    // Note the number of submatches the compile has found and add 1 for the entire match.
	// Allocate matches. Submatches will be ignored ... for now.
    nr_matches = preg->re_nsub + 1;
    matches = malloc (nr_matches * sizeof (regmatch_t));

    unsigned char old = search_string[end];	/* Save char */
    str[end] = '\0';
	errcode = search_regex(preg, &str[start], nr_matches, matches);
    str[end] = old;		/* Restore char */
	if (errcode != 0)	// no match found or an error occurred.
	{
		regfree(preg);
		free(matches);
		return -1;
	}

	// Now process the match found relative to the start position of searching
	start_match = matches[0].rm_so+start;
	*end_match = matches[0].rm_eo+start;

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
