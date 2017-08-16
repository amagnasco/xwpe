/**
 * \file utils.h
 *
Copyright (C) utils.h  Guus Bonnema

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; version 2
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/
#ifndef MAKRO_H

/**
 *  utils.h
 *
 *  Copyright (C) 2017 Guus Bonnema
 *
 *  This is free software; You can redistribute it and/or
 *  modify it under the terms of the GNU Public License version 2 or later.
 *  See the file COPYING for license information.
 *
 */
#define MAKRO_H

#include "config.h"
#include <regex.h>

int isalnum1 (int x);
int isalpha1 (int x);
void print_stacktrace ();

/**
 * Compile a regular expression with default flags and optional case sensitive.
 * If the errorcode is non-zero, then the pointer preg is null.
 *
 * @param preg					compiled regular expression.
 * @param regular_expression	the regular expression as a string.
 * @param case_sensitive		true if the search should be case sensitive, false otherwise.
 *
 * @return zero if compile ok, -1 if not ok.
 */
int compile_regex(regex_t *preg, const unsigned char *regular_expression,
                  const _Bool case_sensitive);
/**
 * Search a string for a regular expression using regexec.
 *
 * On error return from regexec, depending on the error code the function
 * returns -1, if not found or -1 plus prints a description of the error.
 *
 * @param preg a pointer to a regex_t struct (a compiled regular expression.
 * @param search_string a string to be searched by a regex.
 * @param nr_matches an integer for the number of matches allocated in matches (array)
 * @param matches an array containing the result of one search (including submatches).
 *		The first occurrence contains the match of the complete expression.
 *
 * @return int errcode 0 if a match was found, an errorcode if no match was found or
 *			any other error occurred.
 */
int search_regex(regex_t *preg, const unsigned char * search_string,
                 int nr_matches, regmatch_t * matches);
/**
 * Do a print to std output of the error and the error description from regerror.
 */
void print_regerror(int errcode, regex_t *preg);

#ifndef __USE_MISC
/**
 * compares strings ignoring case.
 *
 * This function is only defined if __USE_MISC is not defined through one of the
 * feature macro's _BSD_SOURCE or _SVID_SOURCE. Normally we do not define
 * these features. We use _XOPEN_SOURCE 700 in stead which does not include either feature.
 * So we define strncasecmp in our own implementation copied from
 * https://stackoverflow.com/questions/7299119/source-code-for-strncasecmp-function.
 *
 */
int
strncasecmp (const char *s1, const char *s2, size_t n);
#endif

#endif // #ifndef MAKRO_H
