/*-------------------------------------------------------------------------*\
  <WeString.c> -- Some string routines for xwpe

  Date      Programmer  Description
  04/27/97  Dennis      Created based on functions from "we_hfkt.c".
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Original header of "we_hfkt.c"
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* we_hfkt.c                                              */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include "Xwpe.h"
#include "WeString.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


int WpeStrnccmp(const char *s1, const char *s2, int n)
{
 /* Added a check for end of string. */
 for (; (n > 0) && (*s1) && (toupper(*s1) == toupper(*s2));
      n--, s1++, s2++) ;
 return(n > 0 ? toupper(*s1) - toupper(*s2) : 0);
}

int WpeStrccmp(const char *s1, const char *s2)
{
 for (; (*s1) && (toupper(*s1) == toupper(*s2)); s1++, s2++) ;
 return(toupper(*s1) - toupper(*s2));
}

char *WpeStrcstr(char *str, const char *substr)
{
 const int len = strlen(substr);

 for (; *str; str++)
 {
  if (WpeStrnccmp(str, substr, len) == 0)
  {
   return(str);
  }
 }
 return(NULL);
}

char *WpeStrdup(const char *str)
{
 char *newstr;
 
 newstr = WpeMalloc((strlen(str)+1)*sizeof(char));
 if (newstr != NULL)
 {
  strcpy(newstr, str);
 }
 return(newstr);
}

int WpeNumberOfPlaces(int n)
{
 int i;

 if (n == 0)
 {
  return(1);
 }
 else if (n < 0)
 {
  n = -n;
 }
 for (i = 0; n > 0; n /= 10, i++) ;
 return(i);
}

char *WpeNumberToString(int n, int len, char *s)
{
 int nlen;

 nlen = WpeNumberOfPlaces(n);
 if (nlen > len)
 {
  nlen = len;
 }
 else
 {
  memset(s, ' ', len - nlen);
 }
 s[len] = '\0';
 for (; nlen > 0 ; n /= 10, nlen--)
 {
  len--;
  s[len] = (n % 10) + '0';
 }
 return(s);
}

int WpeStringToNumber(const char *s)
{
 /* This checking of blanks is probably not needed. */
 while ((*s) == ' ')
 {
  s++;
 }
 return(atoi(s));
}

char *WpeStringToUpper(char *s)
{
 char *s2;

 for (s2 = s; *s2; s2++)
 {
  *s2 = toupper(*s2);
 }
 return (s);
}

char *WpeStringBlank(char *s, int len)
{
 memset(s, ' ', len);
 s[len] = '\0';
 return(s);
}

char *WpeStringCutChar(char *s, char c)
{
 char *tmp;

 tmp = strrchr(s, c);
 if (tmp != NULL)
 {
  *tmp = '\0';
 }
 return(s);
}

