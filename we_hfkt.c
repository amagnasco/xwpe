/* we_hfkt.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "edit.h"
#include <regex.h>

/*        find string in text line    */
int e_strstr(int x, int n, unsigned char *s, unsigned char *f)
{
 int i, j, nf = strlen(f);

 if (x > n)
 {
  for (i = x - nf; i >= n; i--)
  {
   for (j = 0; j < nf; j++)
    if (s[i+j] != f[j])
     break;
   if (j == nf)
    return(i);
  }
 }
 else
 {
  for (i = x >= 0 ? x : 0; i <= n - nf /* && s[i] != '\0' && s[i] != WR */; i++)
  {
   for (j = 0; j < nf; j++)
    if (s[i+j] != f[j])
     break;
   if (j == nf)
    return(i);
  }
 }
 return(-1);
}

/*        Find string in line (ignoring case)   */
int e_ustrstr(int x, int n, unsigned char *s, unsigned char *f)
{
 int i, j, nf = strlen(f);

 if(x > n)
 {
  for (i = x-nf; i >= n; i--)
  {
   for (j = 0; j < nf; j++)
    if (e_toupper(s[i+j]) != e_toupper(f[j]))
     break;
   if (j == nf)
    return(i);
  }
 }
 else
 {
  for (i = x < 0 ? 0 : x; i <= n - nf; i++)
  {
   for (j = 0; j < nf; j++)
    if (e_toupper(s[i+j]) != e_toupper(f[j]))
     break;
   if (j == nf)
    return(i);
  }
 }
 return(-1);
}

/*   find string in text line (including control chars), case insensitive */
int e_urstrstr(int x, int n, unsigned char *s, unsigned char *f, int *nn)
{
 int i;
 unsigned char *str;
 unsigned char *ft = MALLOC((strlen(f)+1)*sizeof(unsigned char));

 if (x <= n)
 {
  str = MALLOC((n+1)*sizeof(unsigned char));
  for (i = 0; i < n; i++)
   str[i] = e_toupper(s[i]);
  str[n] = '\0';
 }
 else
 {
  str = MALLOC((x+1)*sizeof(unsigned char));
  for (i = 0; i < x; i++)
   str[i] = e_toupper(s[i]);
  str[x] = '\0';
 }
 for (i = 0; (ft[i] = e_toupper(f[i])) != '\0'; i++)
  ;

 i = e_rstrstr(x, n, str, ft, nn);
 FREE(str);
 FREE(ft);
 return(i);
}

/*   find string in text line (including control chars) */
int e_rstrstr(int x, int n, unsigned char *s, unsigned char *f, int *nn)
{
 regex_t *regz;
 regmatch_t *matches = NULL;
 int start, end, i, len;
 int res;
 unsigned char old;

 regz = MALLOC(sizeof(regex_t));
 if (regcomp(regz,f,REG_EXTENDED))
 {
  free(regz);
  return (-1);
 }
 len = regz->re_nsub;
 if (len)
  matches = MALLOC(len*sizeof(regmatch_t));
 start = (x < n) ? x : n;
 end = (n > x) ? n : x;
 if (start < 0)
  start = 0;
 old = s[end];/* Save char */
 s[end] = '\0';
 res=regexec(regz,&s[start],len,matches,REG_NOTBOL|REG_NOTEOL);
 s[end] = old;/* Restore char */
 regfree(regz);
 free(regz);
 if (res != 0)
 { 
  free(matches);
  return (-1); /* Can't find any occurences */
 }
 start=strlen(s);
 end=0;

 for (i=0;i<len;i++)
 {
  start = (matches[i].rm_so<start) ? matches[i].rm_so : start;
  end = (matches[i].rm_eo>end) ? matches[i].rm_eo : end;
 }
 if (start > end)/* Whole line matches regex */
 {
  end = start;
  start = 0;
 }
 if (matches)
  free(matches);
 *nn=end;
 return start;
}

/*   numbers box (numbers input/edit)     */
int e_num_kst(char *s, int num, int max, FENSTER *f, int n, int sw)
{
 int ret, nz = WpeNumberOfPlaces(max);
 char *tmp = MALLOC((strlen(s)+2) * sizeof(char));
 W_OPTSTR *o = e_init_opt_kst(f);

 if (!o || !tmp)
  return(-1);
 o->xa = 20;  o->ya = 4;  o->xe = 52;  o->ye = 10;
 o->bgsw = 0;
 o->name = s;
 o->crsw = AltO;
 sprintf(tmp, "%s:", s);
 e_add_numstr(3, 2, 29-nz, 2, nz, max, n, sw, tmp, num, o);
 FREE(tmp);
 e_add_bttstr(6, 4, 1, AltO, " Ok ", NULL, o);
 e_add_bttstr(21, 4, -1, WPE_ESC, "Cancel", NULL, o);
 ret = e_opt_kst(o);
 if (ret != WPE_ESC)
  num = o->nstr[0]->num;
 freeostr(o);
 return(num);
}

/*   determine string length */
int e_str_len(unsigned char *s)
{
 int i;

 for (i = 0; *(s+i) != '\0' && *(s+i) != WPE_WR; i++)
  ;
 return (i);
}

#if 0
/*   determine number of chars in a string */
int e_str_nrc(unsigned char *s)
{
 int i;

 for (i = 0; *(s+i) != '\0'; i++)
  ;
 return (i);
}

/*   capitalize letters (German) */
int e_toupper(int c)
{
 if (c >= 'a' && c <= 'z')
  c = c - 'a' + 'A';
 else if (c >= 0xe0 && c <= 0xfe)
  c = c - 0x20;
 return (c);
}
#endif

/*           COLOR - fill struct with constants           */
COLOR e_s_x_clr(int f, int b)
{
 COLOR c;

 c.f = f;
 c.b = b;
 c.fb = 16*b + f;
 return(c);
}

COLOR e_n_x_clr(int fb)
{
 COLOR f;

 f.fb = fb;
 f.b = fb / 16;
 f.f = fb % 16;
 return(f);
}

COLOR e_s_t_clr(int f, int b)
{
 COLOR c;

 c.f = f;
 c.b = b;
 c.fb = f;
 return(c);
}

COLOR e_n_t_clr(int fb)
{
 COLOR f;

 f.fb = fb;
 f.b = fb;
 f.f = fb;
 return(f);
}

/*            POINT - fill struct with constants            */
POINT e_set_pnt(int x, int y)
{
 POINT p;

 p.x = x;
 p.y = y;
 return(p);
}

int e_pr_uul(FARBE *fb)
{
 extern WOPT *blst;
 extern int nblst;
 int i;

 e_blk(MAXSCOL, 0, MAXSLNS-1, fb->mt.fb);
 for (i = 0; i < nblst && blst[i].x < MAXSCOL; ++i)
  e_pr_str_scan(blst[i].x+1, MAXSLNS-1, blst[i].t, fb->mt.fb,
			blst[i].s, blst[i].n, fb->ms.fb, blst[i].x,
			i == nblst-1 ? MAXSCOL-1 : blst[i+1].x-1);
 return(i);
}

