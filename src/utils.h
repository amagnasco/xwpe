#ifndef MAKRO_H
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
int compile_regex(regex_t *preg, unsigned char *regular_expression, _Bool case_sensitive);
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
int search_regex(regex_t *preg, unsigned char * search_string,
                 int nr_matches, regmatch_t * matches);
/**
 * Do a print to std output of the error and the error description from regerror.
 */
void print_regerror(int errcode, regex_t *preg);

#endif // #ifndef MAKRO_H
