#ifndef WE_FIND_H
#define WE_FIND_H
/**
 *  \file we_find.h
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

#include "config.h"
#include <string.h>

/* Struct definitions */

/**
 * The FIND structure is in use for searching.
 */
typedef struct FND
{
    char search[80], replace[80];
    char file[80];		/* filename or pattern to search/open */
    char *dirct;
    size_t sn;			/* length of the search string */
    size_t rn;			/* length of the replace string */
    /** sw is a binary field where each bit has a logical value
     *
     * bit  | test     | zero              | one                  |
     * val  |          | off               | on                   |
     * ------------------------------------------------------------
     *  2^0 | sw & 1   | (not replace)     | replace (best guess) |
     *  2^1 | sw & 2   | from cursor       | entire scope         |
     *  2^2 | sw & 4   | search forward    | search backward      |
     *  2^3 | sw & 8   | global scope      | selection            |
     *  2^4 | sw & 16  |                   | confirm replace      |
     *  2^5 | sw & 32  |                   | regular expression   |
     *  2^6 | sw & 64  |                   | word boundary        |
     *  2^7 | sw & 128 | ignore case       | case sensitive       |
     *  2^8 | sw & 256 |
     *  2^9 | sw & 512 |
     *  2^10| sw & 1024|
     *
     * The name of the corresponding functions that test correctly
     * you can find by prefixing the description with "find_" and
     * replace space by "_". bit 3 (sw & 8) you can test with
     * find_global_scope(find->sw). The other values are reserved for now.
     *
     * You can find these values defined in we_block.c: e_find and e_replace
     *
     */
    unsigned int sw;
} FIND;


/* Function prototypes */

_Bool find_replace (unsigned int sw);
_Bool find_from_cursor (unsigned int sw);
_Bool find_entire_scope (unsigned int sw);
_Bool find_search_forward (unsigned int sw);
_Bool find_search_backward (unsigned int sw);
_Bool find_global_scope (unsigned int sw);
_Bool find_selection (unsigned int sw);
_Bool find_confirm_replace (unsigned int sw);
_Bool find_regular_expression (unsigned int sw);
_Bool find_word_boundary (unsigned int sw);
_Bool find_ignore_case (unsigned int sw);
_Bool find_case_sensitive (unsigned int sw);

/**
 * find string in text line and return position if successful. Optionally search
 * in a case sensitive manner. Reversing start_offset and end_offset triggers a backward search.
 *
 * @param start_offset			int representing the starting offset from within the search_string
 * @param end_offset			int representing the ending offset from within the search string
 * @param search_string			string containing the string to be searched (a.k.a. haystack)
 * @param search_expression		string containing the search expression (a.k.a. needle)
 * @param case_sensitive		boolean true if we want a case sensitive search
 *
 * @return -1 if not found, position starting from 0 if found
 *
 */
int
e_strstr (int start_offset, int end_offset,
          unsigned char *search_string, unsigned char *search_expression,
          size_t *end_match_return, _Bool case_sensitive);
/**
 *  search a string with a regular expression. Optionally search
 *  in a case sensitive manner. Reversing start_offset and
 *  end_offset triggers a backward search.
 *
 * @param start_offset			int representing the starting offset from within the search_string
 * @param end_offset			int representing the ending offset from within the search string
 * @param search_string			string containing the string to be searched (a.k.a. haystack)
 * @param regular_expression		string containing the search expression (a.k.a. needle)
 * @param end_match_str			pointer to size_t integer. function will return end position in
 *							    case of a match.
 * @param case_sensitive		boolean true if we want a case sensitive search
 *
 * @return -1 if not found, position starting from 0 if found
 *
 */
int e_rstrstr (const size_t start_offset,
               const size_t end_offset,
               const unsigned char *search_string,
               const unsigned char *regular_expression,
               size_t * end_match_str, const _Bool case_sensitive);

#endif
