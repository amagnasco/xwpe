#ifndef WE_FINE_H
#define WE_FINE_H
/**
 *  we_find.h
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

/* Struct definitions */

/**
 * The search functions use this struct to store the 
 * results of the search
 *
 */
typedef struct
{
  int nr_hits;			/* -1 if failed, 0 if failed, 1 if success */
  size_t start_match;		/* offset of matched substring within string */
  size_t end_match;		/* offset of last matched char within string */
} Search_result;

/**
 * The FIND structure is in use for searching. 
 *
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
   * bit  | test     | zero              | one                |
   * val  |          | off               | on                 |
   * ------------------------------------------------------  
   *  2^0 | sw & 1   |                   | successful         |
   *  2^1 | sw & 2   | from cursor       | entire scope       |
   *  2^2 | sw & 4   | search forward    | search backward    |
   *  2^3 | sw & 8   | global scope      | selection          |
   *  2^4 | sw & 16  |                   | confirm replace    |
   *  2^5 | sw & 32  |                   | regular expression |
   *  2^6 | sw & 64  |                   | word boundary      |
   *  2^7 | sw & 128 | ignore case       | case sensitive     |
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

_Bool find_successful (unsigned int sw);
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

int e_strstr (int x, int n, unsigned char *s, unsigned char *f);
int e_ustrstr (int x, int n, unsigned char *s, unsigned char *f);
int e_rstrstr (size_t start_offset, size_t end_offset,
	       unsigned char *search_string,
	       unsigned char *regular_expression,
	       size_t * end_match_str, _Bool case_sensitive);

#endif
