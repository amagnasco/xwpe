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

/* non exported prototypes */
/**
 * Allocates memory for the specified number of Match occurrences.
 * if the pointer ptr is NULL, new memory is allocated, otherwise
 * the memory is reallocated. If the pointer ptr is non-NULL and size
 * is zero, the effect is of a free.
 *
 *
 *
 * @param ptr pointer to Match array. may be NULL
 * @param size size_t number of occurrences of Match, may be zero
 * @param err_msg string to be printed on error, may be NULL.
 *
 * @return pointer to Match. If reallocation is successful, this pointer
 * is the same as the param ptr. If reallocation is not successful, this
 * pointer is NULL and ptr is unchanged.
 *
 */
Match *allocate_match (Match * ptr, size_t size, const char *err_msg);
/**
 * Initialize a new Search_result.
 *
 */
Search_result new_search_result ();
/**
 * Does the search request being submitted contain sane values?
 *
 * Returns true if start and end values are sane and haystack and needle are non-overlapping.
 *
 */
_Bool search_request_ok(Search_request *request);

Search_result
e_search_line (Search_request * request)
{
    size_t size_result = 10;

    // Precreate Search_result
    Search_result result = new_search_result ();

    // Check input
    if (!search_request_ok(request))
    {
        result.match_result = error;
        strcpy(result.error_msg, "Search request contained non sane values.\n");
        return result;
    }

    // TODO: combine errormessage with messages in e_msg, e_p_msg etc (we_control.c and messages.h)
    char *alloc_err_msg_str = "Request to look for '%s' failed due to memory problems.\n";
    char alloc_err_msg[128];
    sprintf (alloc_err_msg, alloc_err_msg_str, request->needle);

    if (!allocate_match (result.matches, size_result, alloc_err_msg))
    {
        result.match_result = error;
        strncpy(result.error_msg, alloc_err_msg, strlen(alloc_err_msg));
        return result;
    }
    result.nr_hits = 0;
    result.match_result = match_not_found;

    int (*compare)(const char *s1, const char *s2, size_t len);
    compare = request->case_sensitive ? strncmp : strncasecmp;

    const char *haystack = (const char *) request->haystack;
    const char *needle = (const char *) request->needle;
    const size_t len = request->end_offset - request->start_offset + 1;
    for (size_t i=0; i < len; i++, haystack++)
    {
        if (0 == compare(haystack, needle, len))
        {
            result.nr_hits++;
            result.match_result = match_found;
            if (result.nr_hits > size_result)
            {
                size_result *= 2;	/* double the size */
                void *ptr = allocate_match (result.matches, size_result, alloc_err_msg);
                if (!ptr)
                {
                    result.match_result = error;
                    return result;
                }
            }
            size_t start_match = (const char *)request->haystack - haystack;
            result.matches[result.nr_hits - 1].start_match = start_match;
            size_t end_match = start_match + strlen((const char *)request->needle) -1;
            result.matches[result.nr_hits - 1].end_match = end_match;
        }
    }
    return result;
}

Search_result
new_search_result ()
{
    Search_result result;
    result.match_result = 0;
    strcpy (result.error_msg, "");
    result.nr_hits = 0;
    result.matches = NULL;
    return result;
}

_Bool
search_request_ok(Search_request *request)
{

    /* non-null request pointers */
    if (!request || !request->haystack || !request->needle)
        return (_Bool) 0;

    size_t start = request->start_offset;
    size_t end = request->end_offset;
    unsigned char *haystack = request->haystack;
    unsigned char *needle = request->needle;

    _Bool result = 1;

    /* sane start and end offset */
    result = result && start <= end;
    /* non-overlapping pointers */
    size_t len = strlen((char *)request->haystack);
    result = result &&
             (needle > haystack + len + 1 || needle < haystack);
    /* sane end offset */
    result = result && end < len;

    return result;
}

Match *
allocate_match (Match * ptr, size_t size, const char *err_msg)
{
    void *return_ptr = realloc (ptr, size * sizeof (Match));
    if (!return_ptr)
    {
        printf ("Reallocation of search memory failed.\n");
        printf ("size of the request was %ld.\n", size);
        if (err_msg)
            printf ("provided error msg: %s\n", err_msg);
        printf
        ("This should never happen unless memory is really at a minimum.\n");
        printf ("Breaking off the search algorithm.");
        return NULL;
    }
    return return_ptr;
}

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
	 *  for forward and backward we search from -1 * start to -1 * end for backward searches.
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

/*        Find string in line (ignoring case)   */
int
e_ustrstr (int start_offset, int end_offset, unsigned char *search_string,
           unsigned char *search_expression)
{
    int len_search_exp = strlen ((const char *) search_expression);
    int (*compare) (const char *s1, const char *s2, size_t n);
    compare = strncasecmp;

    if (start_offset > end_offset)
    {
        for (int i = start_offset - len_search_exp; i >= end_offset; i--)
            if (0 == compare ((const char *) search_string + i,
                              (const char *) search_expression, len_search_exp))
                return (i);
    }
    else
    {
        for (int i = start_offset < 0 ? 0 : start_offset;
                i <= end_offset - len_search_exp; i++)
            if (0 == compare ((const char *) search_string + i,
                              (const char *) search_expression, len_search_exp))
                return (i);
    }
    return (-1);
}

/* function e_rstrstr
 *
 * Matches regular expression to provided string. This function returns only the whole match.
 * If the regular expression contains submatches (parentheses), the function ignores the submatches.
 *
 * @param start_offset		integer containing the startpoint within the provided search string
 * @param end_offset		integer containing the endpoint within the provided search string
 * @param search_string		string containing the search string
 * @param regular_expression	string containing the regular expression
 * @param start_match		the returned start position of the entire match (no submatches)
 * @param end_match		the returned end position of the entire match (no submatches)
 *
 * @return: 0 if result is ok, -1 if an error occurred or no search result.
 *
 *   */
int
e_rstrstr (size_t start_offset,
           size_t end_offset,
           unsigned char *search_string,
           unsigned char *regular_expression,
           size_t * end_match, _Bool case_sensitive)
{
    regex_t regz[1];
    regmatch_t *matches = NULL;
    size_t start, end, nr_chars_search_str;
    size_t nr_matches;
    int start_match;

    // copy and sanity check the input variables
    nr_chars_search_str = strlen ((const char *) search_string);
    start = start_offset;
    end = end_offset;
    if (start > end || start > nr_chars_search_str || end > nr_chars_search_str)
    {
        char *msg =
            "Regex search with: start=%lu, end=%lu, string=\"%s\", reg_exp=\"%s\"\n";
        printf (msg, start, end, search_string, regular_expression);
        print_stacktrace ();
        return -1;
    }
    unsigned char str[nr_chars_search_str + 1];
    /* copy excluding null byte */
    strncpy ((char *) str, (const char *) search_string, nr_chars_search_str);
    str[nr_chars_search_str] = '\0';

    // Compile regular expression, return -1 if compile fails.
    int cflags = case_sensitive
                 ? REG_EXTENDED | REG_NEWLINE : REG_EXTENDED | REG_NEWLINE | REG_ICASE;
    if (regcomp (regz, (const char *) regular_expression, cflags))
    {
        return -1;
    }
    // Note the number of submatches the compile has found and add 1 for the entire match.
    // Use only the entire match.
    nr_matches = regz->re_nsub + 1;
    matches = malloc (nr_matches * sizeof (regmatch_t));
    unsigned char old = search_string[end];	/* Save char */
    str[end] = '\0';
    int search_result =
        regexec (regz, (const char *) &search_string[start], nr_matches,
                 matches,
                 REG_NOTBOL | REG_NOTEOL);
    str[end] = old;		/* Restore char */
    if (search_result == REG_NOMATCH)
    {
        regfree (regz);		/* Obliged internal free */
        free (matches);
        return -1;
    }
    if (search_result != 0)
    {
        char err_buff[1024];
        regerror (search_result, regz, err_buff, 1024);
        printf ("[regex search] error message: %s\n", err_buff);
        regfree (regz);		/* Obliged internal free */
        free (matches);
        return -1;		/* Error other than not found */
    }
    start_match =
        (matches[0].rm_so <
         (signed long) start) ? matches[0].rm_so : (signed long) start;
    *end_match =
        (matches[0].rm_eo >
         (signed long) end) ? matches[0].rm_eo : (signed long) end;

    regfree (regz);		/* Obliged internal free */
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
