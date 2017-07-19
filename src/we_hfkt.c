/* we_hfkt.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include <ctype.h>
#include <string.h>
#include <regex.h>
#include "config.h"
#include "keys.h"
#include "model.h"
#include "edit.h"
#include "WeString.h"
#include "we_hfkt.h"
#include "utils.h"

/*        find string in text line    */
int
e_strstr (int start_offset, int end_offset, unsigned char *search_string,
	  unsigned char *search_expression)
{
  int i, j, len_search_exp = strlen ((const char *) search_expression);

  if (start_offset > end_offset)
    {
      for (i = start_offset - len_search_exp; i >= end_offset; i--)
	{
	  for (j = 0; j < len_search_exp; j++)
	    if (search_string[i + j] != search_expression[j])
	      break;
	  if (j == len_search_exp)
	    return (i);
	}
    }
  else
    {
      for (i = start_offset >= 0 ? start_offset : 0;
	   i <= end_offset - len_search_exp
	   /* && search_string[i] != '\0' && search_string[i] != WR */ ;
	   i++)
	{
	  for (j = 0; j < len_search_exp; j++)
	    if (search_string[i + j] != search_expression[j])
	      break;
	  if (j == len_search_exp)
	    return (i);
	}
    }
  return (-1);
}

/*        Find string in line (ignoring case)   */
int
e_ustrstr (int start_offset, int end_offset, unsigned char *search_string,
	   unsigned char *search_expression)
{
  int i, j, len_search_exp = strlen ((const char *) search_expression);

  if (start_offset > end_offset)
    {
      for (i = start_offset - len_search_exp; i >= end_offset; i--)
	{
	  for (j = 0; j < len_search_exp; j++)
	    if (toupper (search_string[i + j]) !=
		toupper (search_expression[j]))
	      break;
	  if (j == len_search_exp)
	    return (i);
	}
    }
  else
    {
      for (i = start_offset < 0 ? 0 : start_offset;
	   i <= end_offset - len_search_exp; i++)
	{
	  for (j = 0; j < len_search_exp; j++)
	    if (toupper (search_string[i + j]) !=
		toupper (search_expression[j]))
	      break;
	  if (j == len_search_exp)
	    return (i);
	}
    }
  return (-1);
}

/*   find string in text line (including control chars), case insensitive */
int
e_urstrstr (int start_offset, int end_offset, unsigned char *search_string,
	    unsigned char *regular_expression, size_t * end_match)
{
  int i;
  unsigned char *str;
  unsigned char *ft =
    malloc ((strlen ((const char *) regular_expression) +
	     1) * sizeof (unsigned char));

  if (start_offset <= end_offset)
    {
      str = malloc ((end_offset + 1) * sizeof (unsigned char));
      for (i = 0; i < end_offset; i++)
	str[i] = toupper (search_string[i]);
      str[end_offset] = '\0';
    }
  else
    {
      str = malloc ((start_offset + 1) * sizeof (unsigned char));
      for (i = 0; i < start_offset; i++)
	str[i] = toupper (search_string[i]);
      str[start_offset] = '\0';
    }
  for (i = 0; (ft[i] = toupper (regular_expression[i])) != '\0'; i++)
    ;

  int result =
    e_rstrstr (start_offset, end_offset, str, ft, end_match, 0 /* case_sensitive = false */);
  free (str);
  free (ft);
  return (result);
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
	  ? REG_EXTENDED | REG_NEWLINE 
	  : REG_EXTENDED | REG_NEWLINE | REG_ICASE;
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
    regexec (regz, (const char *) &search_string[start], nr_matches, matches,
	     REG_NOTBOL | REG_NOTEOL);
  str[end] = old;	  /* Restore char */
  if (search_result == REG_NOMATCH)
  {
	  regfree(regz);		/* Obliged internal free */
	  free(matches);
	  return -1;
  }
  if (search_result != 0)
    {
      char err_buff[1024];
      regerror(search_result, regz, err_buff, 1024);
      printf("[regex search] error message: %s\n", err_buff);
      regfree(regz);		/* Obliged internal free */
      free (matches);
      return -1;		/* Error other than not found */
    }
  start_match = (matches[0].rm_so < (signed long) start) ? matches[0].rm_so : (signed long) start;
  *end_match = (matches[0].rm_eo > (signed long) end) ? matches[0].rm_eo : (signed long) end;

  regfree (regz);		/* Obliged internal free */
  free (matches);
  return start_match;
}

/*   numbers box (numbers input/edit)     */
int
e_num_kst (char *s, int num, int max, we_window * f, int n, int sw)
{
  int ret, nz = WpeNumberOfPlaces (max);
  char *tmp = malloc ((strlen (s) + 2) * sizeof (char));
  W_OPTSTR *o = e_init_opt_kst (f);

  if (!o || !tmp)
    return (-1);
  o->xa = 20;
  o->ya = 4;
  o->xe = 52;
  o->ye = 10;
  o->bgsw = 0;
  o->name = s;
  o->crsw = AltO;
  sprintf (tmp, "%s:", s);
  e_add_numstr (3, 2, 29 - nz, 2, nz, max, n, sw, tmp, num, o);
  free (tmp);
  e_add_bttstr (6, 4, 1, AltO, " Ok ", NULL, o);
  e_add_bttstr (21, 4, -1, WPE_ESC, "Cancel", NULL, o);
  ret = e_opt_kst (o);
  if (ret != WPE_ESC)
    num = o->nstr[0]->num;
  freeostr (o);
  return (num);
}

/*   determine string length */
int
e_str_len (unsigned char *s)
{
  int i;

  for (i = 0; *(s + i) != '\0' && *(s + i) != WPE_WR; i++)
    ;
  return (i);
}

/*           COLOR - fill struct with constants           */
COLOR
e_s_x_clr (int f, int b)
{
  COLOR c;

  c.f = f;
  c.b = b;
  c.fb = 16 * b + f;
  return (c);
}

COLOR
e_n_x_clr (int fb)
{
  COLOR f;

  f.fb = fb;
  f.b = fb / 16;
  f.f = fb % 16;
  return (f);
}

COLOR
e_s_t_clr (int f, int b)
{
  COLOR c;

  c.f = f;
  c.b = b;
  c.fb = f;
  return (c);
}

COLOR
e_n_t_clr (int fb)
{
  COLOR f;

  f.fb = fb;
  f.b = fb;
  f.f = fb;
  return (f);
}

/*            POINT - fill struct with constants            */
POINT
e_set_pnt (int x, int y)
{
  POINT p;

  p.x = x;
  p.y = y;
  return (p);
}

int
e_pr_uul (we_colorset * fb)
{
  extern WOPT *blst;
  extern int nblst;
  int i;

  e_blk (MAXSCOL, 0, MAXSLNS - 1, fb->mt.fb);
  for (i = 0; i < nblst && blst[i].x < MAXSCOL; ++i)
    e_pr_str_scan (blst[i].x + 1, MAXSLNS - 1, blst[i].t, fb->mt.fb,
		   blst[i].s, blst[i].n, fb->ms.fb, blst[i].x,
		   i == nblst - 1 ? MAXSCOL - 1 : blst[i + 1].x - 1);
  return (i);
}
