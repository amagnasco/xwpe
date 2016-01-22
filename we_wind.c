/* we_wind.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"

#define MAXSVSTR 20

int e_make_xr_rahmen(int xa, int ya, int xe, int ye, int sw);

/* break string into multiple line to fit into windows 

   REM: Caller has to free returned vector!!! 
*/
char **StringToStringArray(char *str, int *maxLen, int minWidth, int *anzahl)
{
 int i, j, k, anz = 0, mxlen = 0, max = 0.8 * MAXSCOL;
 char **s = MALLOC(sizeof(char*));

 for (k = 0, i = 0; str[i]; i++)
 {
  if((i-k) == max || str[i] == '\n')
  {
   j = i-1;
   if (str[i] != '\n') 
    for (; j > 0 && !isspace(str[j]); j--)
     ;
   if(j > k)
    i = j;
   anz++;
   s = REALLOC(s, anz * sizeof(char *));
   s[anz-1] = MALLOC((i - k + 2) * sizeof(char));
   for (j = k; j <= i; j++)
    s[anz-1][j-k] = str[j];
   if (isspace(str[j-1]))
    j--;
   if (mxlen < (j-k))
    mxlen = j - k;
   s[anz-1][j-k] = '\0';
   k = i+1;
  }
 }
 anz++;
 s = REALLOC(s, anz * sizeof(char *));
 s[anz-1] = MALLOC((i - k + 2) * sizeof(char));
 for (j = k; j <= i; j++)
  s[anz-1][j-k] = str[j];
 if (mxlen < (j-k))
  mxlen = j - k;
 if (mxlen < minWidth)
  mxlen = minWidth;

 *maxLen = mxlen;
 *anzahl = anz;

 return (s);
}

/*
      Print error message        */
int e_error(char *text, int sw, FARBE *f)
{
 PIC *pic = NULL;
 int len, i, xa, xe, ya = 8, ye = 14;
 char *header = NULL;

 fk_cursor(0);
 WpeMouseChangeShape(WpeErrorShape);
 if ((len = strlen((char *)text)) < 20 ) len = 20;
 xa = (80-len)/2 - 2;
 xe = 82 - (80-len)/2;
 if (sw == -1) header = "Message";
 else if (sw == 0) header = "Error";
 else if (sw == 1) header = "Serious Error";
 else if (sw == 2) header = "Fatal Error";

 if (sw < 2) pic = e_std_kst(xa, ya, xe, ye, header, 1, f->nr.fb, f->nt.fb, f->ne.fb);
 if (sw == 2 || pic == NULL)
 {
  pic = e_open_view(xa, ya, xe, ye, 0, 0);
  e_std_rahmen(xa, ya, xe, ye, header, 1, 0, 0);
 }
 if (sw < 2)
 {
  e_pr_str((xe + xa - e_str_len((unsigned char *)text))/2,
    ya + 2, text, f->nt.fb, 0, 0, 0, 0);
  e_pr_str((xe + xa - 4)/2, ya + 4, " OK ", f->nz.fb, 1, -1,
    f->ns.fb, f->nt.fb);
 }
 else
 {
  e_pr_str((xe + xa - e_str_len((unsigned char *)text))/2,
    ya + 2, text, 112, 0, 0, 0, 0);
  e_pr_str((xe + xa - 4)/2, ya + 4, " OK ", 32, 1, -1, 46, 112);
 }
 do
 {
#if  MOUSE
  if ((i = e_toupper(e_getch())) == -1)
   i = e_er_mouse(xa+3, ya,(xe+xa-4)/2, ya+4);
#else
  i = e_toupper(e_getch());
#endif
 } while (i != WPE_ESC && i != WPE_CR && i != 'O');
 WpeMouseRestoreShape();
 if (pic != NULL) e_close_view(pic, 1);
 else e_cls(0, ' ');
 fk_cursor(1);
 if (sw == 1) e_quit(WpeEditor->f[WpeEditor->mxedt]);
 if (sw > 0) WpeExit(sw);
 return(sw);
}

/*   message with selection        */
int e_message(int sw, char *str, FENSTER *f)
{
 int i, ret, mxlen = 0, anz = 0;
 char **s;
 W_OPTSTR *o = e_init_opt_kst(f);

 if (!o)
  return(-1);

 s = StringToStringArray(str, &mxlen, 22, &anz);

 o->ye = MAXSLNS - 6;
 o->ya = o->ye - anz - 5;
 o->xa = (MAXSCOL - mxlen - 6)/2;
 o->xe = o->xa + mxlen + 6;

 o->bgsw = 0;
 o->name = "Message";
 for (i = 0; i < anz; i++)
 {
  e_add_txtstr((o->xe-o->xa-strlen(s[i]))/2, 2+i, s[i], o);
  FREE(s[i]);
 }
 FREE(s);
 if (!sw)
 {
  o->crsw = AltO;
  e_add_bttstr((o->xe-o->xa-4)/2, o->ye-o->ya-2, 0, AltO, "Ok", NULL, o);
 }
 else
 {
  o->crsw = AltY;
  e_add_bttstr(4, o->ye-o->ya-2, 0, AltY, "Yes", NULL, o);
  e_add_bttstr((o->xe-o->xa-2)/2, o->ye-o->ya-2, 0, AltN, "No", NULL, o);
  e_add_bttstr(o->xe-o->xa-9, o->ye-o->ya-2, -1, WPE_ESC, "Cancel", NULL, o);
 }
 ret = e_opt_kst(o);
 freeostr(o);
 return(ret == WPE_ESC ? WPE_ESC : (ret == AltN ? 'N' : 'Y'));
}

/*         First opening of a window                 */
void e_firstl(FENSTER *f, int sw)
{
 f->pic = NULL;
 f->pic = e_ed_kst(f, f->pic, sw);
 if (f->pic == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, f->fb);
}

/*         Writing of the file type    */
int e_pr_filetype(FENSTER *f)
{
 int frb = f->fb->es.fb;

 e_pr_char(f->a.x+2, f->e.y, 'A', frb);
 if (f->ins == 0 || f->ins == 2)
  e_pr_char(f->a.x+16, f->e.y, 'O', frb);
 else if (f->ins == 8)
  e_pr_char(f->a.x+16, f->e.y, 'R', frb);
 else
  e_pr_char(f->a.x+16, f->e.y, 'I', frb);
 if (f->ins > 1)
  e_pr_char(f->a.x+17, f->e.y, 'S', frb);
 else
  e_pr_char(f->a.x+17, f->e.y, 'L', frb);
 return(0);
}

/*   open section of screen and save background  */
PIC *e_open_view(int xa, int ya, int xe, int ye, int col, int sw)
{
 PIC *pic = MALLOC(sizeof(PIC));
 int i, j;

 if (pic == NULL) return(NULL);
 pic->a.x = xa;
 pic->a.y = ya;
#ifndef NEWSTYLE
 if(!WpeIsXwin())
 {
  pic->e.x = xe;
  pic->e.y = ye;
 }
 else
 {
  pic->e.x = xe < MAXSCOL-2 ? xe + 2 : xe < MAXSCOL-1 ? xe + 1 : xe;
  pic->e.y = ye < MAXSLNS-2 ? ye + 1 : ye;
 }
#else
 pic->e.x = xe;
 pic->e.y = ye;
#endif
 if (sw!=0)
 {
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
  pic->p = MALLOC((pic->e.x - pic->a.x + 1) * 3 * (pic->e.y - pic->a.y + 1));
#else
  pic->p = MALLOC((pic->e.x - pic->a.x + 1) * 2 * (pic->e.y - pic->a.y + 1));
#endif
  if (pic->p == NULL) {  FREE(pic);  return(NULL);  }
  for (j = pic->a.y; j <= pic->e.y; ++j)
   for (i = 2*pic->a.x; i <= 2*pic->e.x+1; ++i)
    *( pic->p + (j-pic->a.y)*2*(pic->e.x-pic->a.x+1) + (i-2*pic->a.x) ) = e_gt_byte(i, j);
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
  e_get_pic_xrect(xa, ya, xe, ye, pic);
#endif
 }
 else pic->p = (char *)0;
 if (sw < 2)
 {
  for (j = ya; j <= ye; ++j)
   for (i = xa; i <= xe; ++i)
    e_pr_char(i, j, ' ', col);
 }
#ifndef NO_XWINDOWS
 if (WpeIsXwin()) (*e_u_setlastpic)(pic);
#endif
#ifndef NEWSTYLE
 if (WpeIsXwin())
 {
  if (sw != 0)
  {
   if (xe < MAXSCOL-1) for(i = ya+1; i <= ye+1 && i < MAXSLNS-1; i++)
    e_pt_col(xe+1, i, SHDCOL);
   if (xe < MAXSCOL-2) for(i = ya+1; i <= ye+1 && i < MAXSLNS-1; i++)
    e_pt_col(xe+2, i, SHDCOL);
   if (ye < MAXSLNS-2) for(i = xa+2; i <= xe; i++) e_pt_col(i, ye+1, SHDCOL);
  }
 }
#endif
 return(pic);
}

/*   close screen section - refresh background  */
int e_close_view(PIC *pic, int sw)
{
 int i, j;
#ifndef NO_XWINDOWS
 if (WpeIsXwin()) (*e_u_setlastpic)(NULL);
#endif
 if (pic == NULL) return(-1);
 if (sw != 0 && pic->p != NULL)
 {
  for (j = pic->a.y; j <= pic->e.y; ++j)
   for (i = 2*pic->a.x; i <= 2*pic->e.x+1; ++i)
    e_pt_byte(i, j, *(pic->p + (j-pic->a.y)*2*(pic->e.x-pic->a.x+1) + (i-2*pic->a.x)));
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
  e_put_pic_xrect(pic);
#endif
 }
 else if (sw != 0)
 {
  for (j = pic->a.y; j <= pic->e.y; ++j)
   for (i = pic->a.x; i <= pic->e.x; ++i)
    e_pr_char(i, j, ' ', 0);
 }
 if (sw < 2)
 {
  if (pic->p != NULL) FREE(pic->p);
  FREE(pic);
 }
 e_refresh();
 return(sw);
}

/*    Frame for edit window   */
void e_ed_rahmen(FENSTER *f, int sw)
{
 extern char *e_hlp;
 extern int nblst;
 extern WOPT *blst;
 char *header = NULL;

 if (!DTMD_ISTEXT(f->dtmd))
 {
  if (f->datnam[0]) header = f->datnam;
  if (f->dtmd == DTMD_FILEDROPDOWN)
   e_std_rahmen(f->a.x, f->a.y, f->e.x, f->e.y, header, sw, f->fb->er.fb,
     f->fb->es.fb);
  else
   e_std_rahmen(f->a.x, f->a.y, f->e.x, f->e.y, header, sw, f->fb->nr.fb,
     f->fb->ne.fb);
  if (f->winnum < 10 && f->winnum >= 0)
   e_pr_char(f->e.x-6, f->a.y, '0' + f->winnum, f->fb->nr.fb);
  else if (f->winnum >= 0)
   e_pr_char(f->e.x-6, f->a.y, 'A' - 10 + f->winnum, f->fb->nr.fb);
  if (sw > 0 && (f->dtmd == DTMD_FILEMANAGER || f->dtmd == DTMD_DATA))
  {
   if (f->zoom == 0)
    e_pr_char(f->e.x-3, f->a.y, WZN, f->fb->ne.fb);
   else
    e_pr_char(f->e.x-3, f->a.y, WZY, f->fb->ne.fb);
#ifdef NEWSTYLE
   if (!WpeIsXwin())
   {
#endif
    e_pr_char(f->e.x-4, f->a.y, '[', f->fb->nr.fb);
    e_pr_char(f->e.x-2, f->a.y, ']', f->fb->nr.fb);
#ifdef NEWSTYLE
   }
   else
    e_make_xrect(f->e.x-4, f->a.y, f->e.x-2, f->a.y, 0);
#endif
   blst = f->blst;
   nblst = f->nblst;
   e_hlp = f->hlp_str;
   e_pr_uul(f->fb);
  }
#ifdef NEWSTYLE
  if (WpeIsXwin())
   e_make_xr_rahmen(f->a.x, f->a.y, f->e.x, f->e.y, sw);
#endif
  return;
 }
 if (f->datnam[0])
 {
  if (strcmp(f->dirct, f->ed->dirct) == 0 ||
    f->dtmd == DTMD_HELP || strcmp(f->datnam, BUFFER_NAME) == 0 ||
    NUM_COLS_ON_SCREEN < 40)
  {
   header = (char *)WpeMalloc(strlen(f->datnam) + 1);
   strcpy(header, f->datnam);
  }
  else
  {
   header = (char *)WpeMalloc(strlen(f->dirct) + strlen(f->datnam) + 1);
   strcpy(header, f->dirct);
   strcat(header, f->datnam);
  }
 }
 e_std_rahmen(f->a.x, f->a.y, f->e.x, f->e.y, header, sw, f->fb->er.fb,
   f->fb->es.fb);
 if (header)
  WpeFree(header);
 if (sw > 0)
 {
  e_mouse_bar(f->e.x, f->a.y+1, NUM_LINES_ON_SCREEN - 1, 0, f->fb->em.fb);
  e_mouse_bar(f->a.x+19, f->e.y, NUM_COLS_ON_SCREEN - 20, 1, f->fb->em.fb);
  if (f->zoom == 0)
   e_pr_char(f->e.x-3, f->a.y, WZN, f->fb->es.fb);
  else
   e_pr_char(f->e.x-3, f->a.y, WZY, f->fb->es.fb);
#ifdef NEWSTYLE
  if (!WpeIsXwin())
  {
#endif
   e_pr_char(f->e.x-4, f->a.y, '[', f->fb->er.fb);
   e_pr_char(f->e.x-2, f->a.y, ']', f->fb->er.fb);
#ifdef NEWSTYLE
  }
  else
   e_make_xrect(f->e.x-4, f->a.y, f->e.x-2, f->a.y, 0);
#endif
  e_pr_filetype(f);
  if (WpeIsXwin() && NUM_LINES_ON_SCREEN > 8)
  {
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
   e_pr_char(f->a.x, f->a.y + 2, 'F', f->fb->em.fb);
   e_make_xrect(f->a.x, f->a.y+2, f->a.x, f->a.y+2, 0);
   e_pr_char(f->a.x, f->a.y + 4, 'R', f->fb->em.fb);
   e_make_xrect(f->a.x, f->a.y+4, f->a.x, f->a.y+4, 0);
   e_pr_char(f->a.x, f->a.y + 6, 'A', f->fb->em.fb);
   e_make_xrect(f->a.x, f->a.y+6, f->a.x, f->a.y+6, 0);
   if (f->ins != 8)
   {
    e_pr_char(f->a.x, f->a.y + 8, 'S', f->fb->em.fb);
    e_make_xrect(f->a.x, f->a.y+8, f->a.x, f->a.y+8, 0);
   }
#else
   e_pr_char(f->a.x, f->a.y + 2, 'F', f->fb->em.fb);
   e_pr_char(f->a.x, f->a.y + 3, MCI, f->fb->em.fb);
   e_pr_char(f->a.x, f->a.y + 4, 'R', f->fb->em.fb);
   e_pr_char(f->a.x, f->a.y + 5, MCI, f->fb->em.fb);
   e_pr_char(f->a.x, f->a.y + 6, 'A', f->fb->em.fb);
   if (f->ins != 8)
   {
    e_pr_char(f->a.x, f->a.y + 7, MCI, f->fb->em.fb);
    e_pr_char(f->a.x, f->a.y + 8, 'S', f->fb->em.fb);
   }
#endif
  }
  e_zlsplt(f);
  blst = f->blst;
  nblst = f->nblst;
  e_hlp = f->hlp_str;
  e_pr_uul(f->fb);
 }
 if (f->winnum < 10 && f->winnum >= 0)
  e_pr_char(f->e.x-6, f->a.y, '0' + f->winnum, f->fb->er.fb);
 else if (f->winnum >= 0)
  e_pr_char(f->e.x-6, f->a.y, 'A' - 10 + f->winnum, f->fb->er.fb);
#ifdef NEWSTYLE
 if (WpeIsXwin())
  e_make_xr_rahmen(f->a.x, f->a.y, f->e.x, f->e.y, sw);
#endif
}

/*   Output - screen content */
int e_schirm(FENSTER *f, int sw)
{
 int j;

 if (f->dtmd == DTMD_FILEMANAGER)
  return(WpeDrawFileManager(f));
 else if (f->dtmd == DTMD_DATA)
  return(e_data_schirm(f));
 else if (f->dtmd == DTMD_FILEDROPDOWN)
  return(e_pr_file_window((FLWND*)f->b, 1, sw, f->fb->er.fb, f->fb->ez.fb,
    f->fb->frft.fb));
 if (NUM_LINES_OFF_SCREEN_TOP < 0)
  NUM_LINES_OFF_SCREEN_TOP = 0;

#ifdef PROG
 if (f->c_sw)
  for (j = NUM_LINES_OFF_SCREEN_TOP; j < f->b->mxlines && j < LINE_NUM_ON_SCREEN_BOTTOM ; j++ )
   e_pr_c_line(j, f);
 else
#endif
  for (j = NUM_LINES_OFF_SCREEN_TOP; j < f->b->mxlines && j < LINE_NUM_ON_SCREEN_BOTTOM ; j++ )
   e_pr_line(j, f);
 for (; j < LINE_NUM_ON_SCREEN_BOTTOM ; j++ )
  e_blk((NUM_COLS_ON_SCREEN - 1), f->a.x + 1, j - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, f->fb->et.fb);
 return(j);
}

/*   Move and modify window */
int e_size_move(FENSTER *f)
{
 int xa = f->a.x, ya = f->a.y, xe = f->e.x, ye = f->e.y;
 int c = 0, xmin = 26, ymin = 3;

 e_ed_rahmen(f, 0);
 if (f->dtmd == DTMD_FILEDROPDOWN)
  xmin = 15;
 else if (!DTMD_ISTEXT(f->dtmd))
  ymin = 9;
 while ((c = e_getch()) != WPE_ESC && c != WPE_CR)
 {
  switch (c)
  {
   case CLE:
    if (xa > 0) {  xa--; xe--;  }
    break;
   case CRI:
    if (xe < MAXSCOL-1) {  xa++; xe++;  }
    break;
   case CUP:
    if (ya > 1) {  ya--; ye--;  }
    break;
   case CDO:
    if (ye < MAXSLNS-2) {  ya++; ye++;  }
    break;
   case SCLE:
   case CCLE:
    if ((xe - xa) > xmin) xe--;
    break;
   case SCRI:
   case CCRI:
    if (xe < MAXSCOL-1) xe++;
    break;
   case SCUP:
   case BUP:
    if ((ye - ya) > ymin) ye--;
    break;
   case SCDO:
   case BDO:
    if (ye < MAXSLNS-2) ye++;
    break;
  }
  if (xa != f->a.x || ya != f->a.y || xe != f->e.x || ye != f->e.y)
  {
   f->a.x = xa;
   f->a.y = ya;
   f->e.x = xe;
   f->e.y = ye;
   f->pic = e_ed_kst(f, f->pic, 0);
   if (f->pic == NULL)  e_error(e_msg[ERR_LOWMEM], 1, f->fb);
   if (f->dtmd == DTMD_FILEDROPDOWN)
   {
    FLWND *fw = (FLWND*) f->b;
    fw->xa = f->a.x+1; fw->xe = f->e.x;
    fw->ya = f->a.y+1; fw->ye = f->e.y;
   }
   e_cursor(f, 0);
   e_schirm(f, 0);
  }
 }
 e_ed_rahmen(f, 1);
 return(c);
}

/*       Standard Box                                  */
PIC *e_std_kst(int xa, int ya, int xe, int ye, char *name, int sw, int fr,
  int ft, int fes)
{
 PIC *pic = e_open_view(xa, ya, xe, ye, ft, 1);
 if (pic == NULL) return (NULL);
 e_std_rahmen(xa, ya, xe, ye, name, sw, fr, fes);
 return(pic);
}

PIC *e_change_pic(int xa, int ya, int xe, int ye, PIC *pic, int sw, int frb)
{
   int i, j;
   int box = 2, ax, ay, ex, ey;
   PIC *newpic;
   if(sw<0) {  sw = -sw; box = 1;  }
   if (pic == NULL)
   {  newpic = e_open_view(xa, ya, xe, ye, frb, box);
      if (newpic == NULL) return (NULL);
   }
   else if (xa > pic->e.x || xe < pic->a.x ||
                                          ya > pic->e.y || ye < pic->a.y)
   {  e_close_view(pic, box);
      newpic = e_open_view(xa, ya, xe, ye, frb, box);
      if (newpic == NULL) return (NULL);
   }
   else
   {
      newpic = MALLOC(sizeof(PIC));
      if(newpic == NULL)  return(NULL);
      newpic->a.x = xa;
      newpic->a.y = ya;
#ifndef NEWSTYLE
      if(!WpeIsXwin())
      {  newpic->e.x = xe;
	 newpic->e.y = ye;
      }
      else
      {  newpic->e.x = xe < MAXSCOL-2 ? xe + 2 : xe < MAXSCOL-1 ? xe + 1 : xe;
	 newpic->e.y = ye < MAXSLNS-2 ? ye + 1 : ye;
      }
#else
      newpic->e.x = xe;
      newpic->e.y = ye;
#endif
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
      newpic->p = MALLOC( (newpic->e.x - newpic->a.x + 1) * 3
				* (newpic->e.y - newpic->a.y + 1) );
#else
      newpic->p = MALLOC( (newpic->e.x - newpic->a.x + 1) * 2
				* (newpic->e.y - newpic->a.y + 1) );
#endif
      if (newpic->p == NULL) {  FREE(newpic);  return(NULL);  }
      ax = pic->a.x > newpic->a.x ? pic->a.x : newpic->a.x;
      ay = pic->a.y > newpic->a.y ? pic->a.y : newpic->a.y;
      ex = pic->e.x < newpic->e.x ? pic->e.x : newpic->e.x;
      ey = pic->e.y < newpic->e.y ? pic->e.y : newpic->e.y;
      for (j = ay; j <= ey; ++j)
      for (i = 2*ax; i <= 2*ex+1; ++i)
      {  *(newpic->p + 2*(newpic->e.x-newpic->a.x+1)*(j-newpic->a.y)
				+ (i-2*newpic->a.x))
                   = *(pic->p + 2*(pic->e.x-pic->a.x+1)*(j-pic->a.y)
                                 + (i - 2*pic->a.x) );
      }
      
      for (j = newpic->a.y; j < ay; ++j)
      for (i = 2*newpic->a.x; i <= 2*newpic->e.x+1; ++i)
      {  *(newpic->p + 2*(newpic->e.x-newpic->a.x+1)*(j-newpic->a.y)
		+ (i-2*newpic->a.x)) = e_gt_byte(i, j);
      }
      
      for (j = newpic->e.y; j > ey; --j)
      for (i = 2*newpic->a.x; i <= 2*newpic->e.x+1; ++i)
      {  *(newpic->p + 2*(newpic->e.x-newpic->a.x+1)*(j-newpic->a.y)
		+ (i-2*newpic->a.x)) = e_gt_byte(i, j);
      }
      
      for (j = newpic->a.y; j <= newpic->e.y; ++j)
      for (i = 2*newpic->a.x; i < 2*ax; ++i)
      {  *(newpic->p + 2*(newpic->e.x-newpic->a.x+1)*(j-newpic->a.y)
		+ (i-2*newpic->a.x)) = e_gt_byte(i, j);
      }
      
      for (j = newpic->a.y; j <= newpic->e.y; ++j)
      for (i = 2*(ex+1); i <= 2*newpic->e.x + 1; ++i)
      {  *(newpic->p + 2*(newpic->e.x-newpic->a.x+1)*(j-newpic->a.y)
		+ (i-2*newpic->a.x)) = e_gt_byte(i, j);
      }
      for (j = pic->a.y; j < ya; ++j)
      for (i = 2*pic->a.x; i <= 2*pic->e.x+1; ++i)
      e_pt_byte(i, j, *( pic->p + (j-pic->a.y)*2
                         *(pic->e.x-pic->a.x+1) + (i-2*pic->a.x) ) );
      for (j = pic->a.y; j <= pic->e.y; ++j)
      for (i = 2*pic->a.x; i < 2*xa; i=i+1)
      e_pt_byte(i, j, *( pic->p + (j-pic->a.y)*2
                       *(pic->e.x-pic->a.x+1) + (i - 2*pic->a.x) ) );
      for (j = pic->e.y; j > ye; --j)
      for (i = 2*pic->a.x; i <= 2*pic->e.x+1; ++i)
      e_pt_byte(i, j, *( pic->p + (j-pic->a.y)*2
                       *(pic->e.x-pic->a.x+1) + (i-2*pic->a.x) ) );
      for (j = pic->e.y; j >= pic->a.y; --j)
      for (i = 2*pic->e.x + 1; i > 2*xe; --i)
      e_pt_byte(i, j, *( pic->p + (j-pic->a.y)*2
                       *(pic->e.x-pic->a.x+1) + (i-2*pic->a.x) ) );
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
      e_put_pic_xrect(pic);
      e_get_pic_xrect(xa, ya, xe, ye, newpic);
#endif
#ifndef NEWSTYLE
      if (WpeIsXwin())
      {  if(xe < MAXSCOL-1) for(i = ya+1; i <= ye+1 && i < MAXSLNS-1; i++) 
						e_pt_col(xe+1, i, SHDCOL);
         if(xe < MAXSCOL-2) for(i = ya+1; i <= ye+1 && i < MAXSLNS-1; i++)
						e_pt_col(xe+2, i, SHDCOL);
         if(ye < MAXSLNS-2) for(i = xa+2; i <= xe; i++)
      						e_pt_col(i, ye+1, SHDCOL);
      }
#endif
      FREE(pic->p);
      FREE(pic);
   }
#ifndef NO_XWINDOWS
   if (WpeIsXwin()) (*e_u_setlastpic)(newpic);
#endif
   return(newpic);
}

PIC *e_ed_kst(FENSTER *f, PIC *pic, int sw)
{
   PIC *newpic = e_change_pic(f->a.x, f->a.y, f->e.x,
				f->e.y, pic, sw, f->fb->er.fb);
   e_ed_rahmen(f, sw);
   return(newpic);
}

/*    delete buffer     */
int e_close_buffer(BUFFER *b)
{
 int i;

 if (b != NULL)
 {
  e_remove_undo(b->ud, b->cn->numundo + 1);
  if (b->bf != NULL)
  {
   for (i = 0; i < b->mxlines; i++)
   {
    if (b->bf[i].s != NULL)
     FREE( b->bf[i].s );
    b->bf[i].s = NULL;
   }
   FREE(b->bf);
  }
  FREE(b);
 }
 return(0);
}

/*    close window */
int e_close_window(FENSTER *f)
{
 ECNT *cn = f->ed;
 FENSTER *f0 = f->ed->f[0];
 int c = 0;
 long maxname;
 char text[256];

 f = cn->f[cn->mxedt];
 if (f->dtmd == DTMD_FILEMANAGER)
 {
  FLBFFR *b = (FLBFFR *)f->b;

  FREE(f->dirct);
  FREE(b->rdfile);
  freedf(b->df);  freedf(b->fw->df);
  freedf(b->dd);  freedf(b->cd);  freedf(b->dw->df);
  FREE(b->fw);
  FREE(b->dw);
  FREE(b);
  (cn->mxedt)--;
  cn->curedt = cn->edt[cn->mxedt];
  e_close_view(f->pic, 1);
  if (f != f0 && f != NULL)
  {
   e_free_find(&f->fd);
   FREE(f);
  }
  if (cn->mxedt > 0)
  {
   f = cn->f[cn->mxedt];
   e_ed_rahmen(f, 1);
  }
  return(0);
 }
 if (f->dtmd == DTMD_DATA)
 {
  FLWND *fw = (FLWND *)f->b;
  int swt = f->ins;

#ifdef PROG
  if (swt == 4 && f->save)
   e_p_update_prj_fl(f);
#endif
  if (f->dirct)
   FREE(f->dirct);
  if (swt == 7)
   freedf(fw->df);
  FREE(fw);
  (cn->mxedt)--;
  cn->curedt = cn->edt[cn->mxedt];
  e_close_view(f->pic, 1);
  if (f != f0 && f != NULL)
  {
   e_free_find(&f->fd);
   FREE(f);
  }
  if (cn->mxedt > 0 && (swt < 5 || swt == 7))
  {
   f = cn->f[cn->mxedt];
   e_ed_rahmen(f, 1);
  }
  return(0);
 }
 if (f == NULL || f->ed->mxedt <= 0)
  return(0);
 if (f != f0)
 {
  if (f->save != 0 && f->ins != 8)
  {
   sprintf(text, "File %s NOT saved!\nDo you want to save File ?", f->datnam);
   c = e_message(1, text, f);
   if (c == WPE_ESC)
    return(c);
   else if (c == 'Y')
    e_save(f);
  }
  /* Check if file system could have an autosave or emergency save file
     >12 check is to eliminate dos file systems */
  if ((maxname = pathconf(f->dirct, _PC_NAME_MAX) >= strlen(f->datnam) + 4) &&
    (maxname > 12))
  {
   remove(e_make_postf(text, f->datnam, ".ASV"));
   remove(e_make_postf(text, f->datnam, ".ESV"));
  }
  if (strcmp(f->datnam, "Messages") && strcmp(f->datnam, "Watches"))
   e_close_buffer(f->b);
  if (f->dtmd == DTMD_HELP && f->ins == 8)
   e_help_free(f);
  if (f->datnam != NULL)
   FREE(f->datnam);
  if (f->dirct != NULL)
   FREE(f->dirct);
  if (f && f->s != NULL)
   FREE(f->s);
 }
 (cn->mxedt)--;
 cn->curedt = cn->edt[cn->mxedt];
 e_close_view(f->pic, 1);
 if (f != f0 && f != NULL)
 {
  e_free_find(&f->fd);
  FREE(f);
 }
 if (cn->mxedt > 0)
 {
  f = cn->f[cn->mxedt];
  e_ed_rahmen(f, 1);
 }
 return(c);
}

/*    Toggle among windows  */
int e_rep_win_tree(ECNT *cn)
{
 int i;

 if (cn->mxedt <= 0) return(0);
 ini_repaint(cn);
 for ( i = 1; i < cn->mxedt; i++)
 {
  e_firstl(cn->f[i], 0);
  e_schirm(cn->f[i], 0);
 }
 e_firstl(cn->f[i], 1);
 e_schirm(cn->f[i], 1);
 e_cursor(cn->f[i], 1);
 end_repaint();
 return(0);
}

void e_switch_window(int num, FENSTER *f)
{
 ECNT *cn = f->ed;
 FENSTER *ft;
 int n, i, te;

 for (n = 1; cn->edt[n] != num && n < cn->mxedt; n++)
  ;
 if (n >= cn->mxedt) return;
 for (i = cn->mxedt; i >= 1; i--)
 {
  FREE(cn->f[i]->pic->p);
  FREE(cn->f[i]->pic);
 }
 ft = cn->f[n];
 te = cn->edt[n];
 for ( i = n; i < cn->mxedt; i++)
 {
  cn->edt[i] = cn->edt[i+1];
  cn->f[i] = cn->f[i+1];
 }
 cn->f[i] = ft;
 cn->edt[i] = te;
 cn->curedt = num;
 e_rep_win_tree(cn);
}

/*    zoom windows   */
int e_ed_zoom(FENSTER *f)
{
 if (f->ed->mxedt > 0)
 {
  if(f->zoom == 0)
  {
   f->sa = e_set_pnt(f->a.x, f->a.y);
   f->se = e_set_pnt(f->e.x, f->e.y);
   f->a = e_set_pnt(0, 1);
   f->e = e_set_pnt(MAXSCOL-1, MAXSLNS-2);
   f->zoom = 1;
  }
  else
  {
   f->a = e_set_pnt(f->sa.x, f->sa.y);
   f->e = e_set_pnt(f->se.x, f->se.y);
   f->zoom = 0;
  }
  f->pic = e_ed_kst(f, f->pic, 1);
  if(f->pic == NULL)  e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  e_cursor(f, 1);
  e_schirm(f, 1);
 }
 return(WPE_ESC);
}

/*   cascade windows   */
int e_ed_cascade(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 if (cn->mxedt < 1)
  return 0; /* no windows open */
 for (i = cn->mxedt; i >= 1; i--)
 {
  FREE(cn->f[i]->pic->p);
  FREE(cn->f[i]->pic);
  cn->f[i]->a = e_set_pnt(i-1, i);
  cn->f[i]->e = e_set_pnt(MAXSCOL-1-cn->mxedt+i, MAXSLNS-2-cn->mxedt+i);
 }
 ini_repaint(cn);
 for ( i = 1; i < cn->mxedt; i++)
 {
  e_firstl(cn->f[i], 0);
  e_schirm(cn->f[i], 0);
 }
 e_firstl(cn->f[i], 1);
 e_schirm(cn->f[i], 1);
 e_cursor(cn->f[i], 1);
 end_repaint();
 return(0);
}

/*   Tile windows   */
int e_ed_tile(FENSTER *f)
{
 ECNT *cn = f->ed;
 POINT atmp[MAXEDT+1];
 POINT etmp[MAXEDT+1];
 int i, j, ni, nj;
 int editwin = 0; /* number of editor windows */
 int editorwin[MAXEDT + 1];
 int maxlines = MAXSLNS;

 for (i = cn->mxedt; i >= 1; i--)
 {
  if ((!(cn->edopt & ED_OLD_TILE_METHOD)) && (!DTMD_ISTEXT(cn->f[i]->dtmd) ||
    ((WpeIsProg()) && ((strcmp(cn->f[i]->datnam, "Messages") == 0) ||
    (strcmp(cn->f[i]->datnam, "Watches") == 0)))))
  {
   editorwin[i] = 0;
  }
  else
  {
   editwin++;
   editorwin[i] = 1;
  }
 }
 if (editwin < 1)
  return(0);
 if ((!(cn->edopt & ED_OLD_TILE_METHOD)) && (WpeIsProg()))
 {
  maxlines -= MAXSLNS / 3 - 1;
 }
 for (i = cn->mxedt; i >= 1; i--)
 {
  FREE(cn->f[i]->pic->p);
  FREE(cn->f[i]->pic);
 }
 for (ni = editwin, nj = 1; ni > 1; ni--)
 {
  nj = editwin / ni;
  if (editwin % ni)
   nj++;
  if (nj >= ni)
   break;
 }
 if (nj*ni < editwin)
  nj++;
 for (j = 0; j < nj; j++)
 {
  for (i = 0; i < ni; i++)
  {
   if (j == 0)
   {
    if (i == 0)
    {
     atmp[j*ni+i].x = i * MAXSCOL / ni;
     etmp[j*ni+i].x = (i + 1) * MAXSCOL / ni - 1;
     if (etmp[j*ni+i].x - atmp[j*ni+i].x < 26)
      etmp[j*ni+i].x = atmp[j*ni+i].x + 26;
    }
    else
    {
     etmp[j*ni+i].x = (i + 1) * MAXSCOL / ni - 1;
     atmp[j*ni+i].x = etmp[j*ni+i-1].x + 1;
     if (etmp[j*ni+i].x - atmp[j*ni+i].x < 26)
      etmp[j*ni+i].x = atmp[j*ni+i].x + 26;
     if (etmp[j*ni+i].x >= MAXSCOL)
     {
      etmp[j*ni+i].x = MAXSCOL - 1;
      atmp[j*ni+i].x = etmp[j*ni+i].x - 26;
     }
    }
   }
   else
   {
    atmp[j*ni+i].x = atmp[(j-1)*ni+i].x;
    etmp[j*ni+i].x = etmp[(j-1)*ni+i].x;
    /* make the last window full width */
    if ((j * ni + i) == (editwin - 1))
     etmp[j * ni + i].x = MAXSCOL - 1;
   }
  }
 }
 for (i = 0; i < ni; i++)
 {
  for (j = 0; j < nj; j++)
  {
   if (i == 0)
   {
    if (j == 0)
    {
     atmp[j*ni+i].y = j * (maxlines-2) / nj + 1;
     etmp[j*ni+i].y = (j + 1) * (maxlines-2) / nj;
     if (etmp[j*ni+i].y - atmp[j*ni+i].y < 3)
      etmp[j*ni+i].y = atmp[j*ni+i].y + 3;
    }
    else
    {
     etmp[j*ni+i].y = (j + 1) * (maxlines-2) / nj;
     atmp[j*ni+i].y = etmp[(j-1)*ni+i].y + 1;
     if (etmp[j*ni+i].y - atmp[j*ni+i].y < 3)
      etmp[j*ni+i].y = atmp[j*ni+i].y + 3;
     if (etmp[j*ni+i].y > maxlines - 2)
     {
      etmp[j*ni+i].y = maxlines - 2;
      atmp[j*ni+i].y = etmp[j*ni+i].y - 3;
     }
    }
   }
   else
   {
    atmp[j*ni+i].y = atmp[j*ni+i-1].y;
    etmp[j*ni+i].y = etmp[j*ni+i-1].y;
   }
  }
 }
 for (i = 0, j = 1; i < editwin; i++, j++)
 {
  while (!editorwin[j]) j++;
  cn->f[j]->a = e_set_pnt(atmp[i].x, atmp[i].y);
  cn->f[j]->e = e_set_pnt(etmp[i].x, etmp[i].y);
  cn->f[j]->zoom = 0; /* Make sure zoom is off */
 }
 ini_repaint(cn);
 for ( i = 1; i < cn->mxedt; i++)
 {
  e_firstl(cn->f[i], 0);
  e_schirm(cn->f[i], 0);
 }
 e_firstl(cn->f[i], 1);
 e_schirm(cn->f[i], 1);
 e_cursor(cn->f[i], 1);
 end_repaint();
 return(0);
}

/*   call next window   */
int e_ed_next(FENSTER *f)
{
 if (f->ed->mxedt > 0) e_switch_window(f->ed->edt[1], f);
 return(0);
}

/*   write a line (screen content)     */
void e_pr_line(int y, FENSTER *f)
{
 BUFFER *b = f->b;
 SCHIRM *s = f->s;
 int i, j, k, frb;
#ifdef DEBUGGER
 int fsw = 0;
#endif

 for (i = j = 0; j < NUM_COLS_OFF_SCREEN_LEFT; j++, i++)
 {
  if (*(b->bf[y].s + i) == WPE_TAB)
   j += (f->ed->tabn - j % f->ed->tabn - 1);
  else if(((unsigned char) *(b->bf[y].s + i)) > 126)
  {
   j++;
   if (((unsigned char) *(b->bf[y].s + i)) < 128 + ' ')
    j++;
  }
  else if (*(b->bf[y].s + i) < ' ') j++;
 }
 if (j > NUM_COLS_OFF_SCREEN_LEFT) i--;
#ifdef DEBUGGER
 for (j = 1; j <= s->brp[0]; j++)
  if (s->brp[j] == y) {  fsw = 1;  break;  }
 for (j = NUM_COLS_OFF_SCREEN_LEFT; i < b->bf[y].len && j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
 {
  if (y == s->da.y && i >= s->da.x && i < s->de.x )
   frb = s->fb->dy.fb;
  else if (fsw) frb = s->fb->db.fb;
/*	else if( (i == s->pt[0].x && y == s->pt[0].y) || (i == s->pt[1].x && y == s->pt[1].y)  */
  else if (y == s->fa.y && i >= s->fa.x && i < s->fe.x )
   frb = s->fb->ek.fb;
#else
 for (j = NUM_COLS_OFF_SCREEN_LEFT; i < b->bf[y].len && j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
 {
  if (y == s->fa.y && i >= s->fa.x && i < s->fe.x )
   frb = s->fb->ek.fb;
#endif
/*	if( (i == s->pt[0].x && y == s->pt[0].y) || (i == s->pt[1].x && y == s->pt[1].y)  
         || (i == s->pt[2].x && y == s->pt[2].y) || (i == s->pt[3].x && y == s->pt[3].y)  
         || (i == s->pt[4].x && y == s->pt[4].y) || (i == s->pt[5].x && y == s->pt[5].y)  
         || (i == s->pt[6].x && y == s->pt[6].y) || (i == s->pt[7].x && y == s->pt[7].y)  
         || (i == s->pt[8].x && y == s->pt[8].y) || (i == s->pt[9].x && y == s->pt[9].y))
            frb = s->fb->ek.fb;
*/
  else if ((y < s->mark_end.y && ( y > s->mark_begin.y ||
    (y == s->mark_begin.y && i >= s->mark_begin.x) ) ) ||
    (y == s->mark_end.y && i < s->mark_end.x && ( y > s->mark_begin.y ||
    (y == s->mark_begin.y && i >= s->mark_begin.x) ) ) )
   frb = s->fb->ez.fb;
  else
   frb = s->fb->et.fb;

  if (f->dtmd == DTMD_HELP)
  {
   if (*(b->bf[y].s + i) == HBG || *(b->bf[y].s + i) == HFB ||
     *(b->bf[y].s + i) == HHD || *(b->bf[y].s + i) == HBB)
   {
    if (*(b->bf[y].s + i) == HHD) frb = s->fb->hh.fb;
    else if(*(b->bf[y].s + i) == HBB) frb = s->fb->hm.fb;
    else frb = s->fb->hb.fb;
#ifdef NEWSTYLE
    if (*(b->bf[y].s + i) != HBB) k = j;
    else k = -1;
#endif
    for (i++; b->bf[y].s[i] != HED && i < b->bf[y].len &&
      j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
     e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1,
       y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, *(b->bf[y].s + i), frb);
    j--;
#ifdef NEWSTYLE
    if (WpeIsXwin() && k >= 0)
     e_make_xrect(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + k + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
       f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, 0);
#endif
    continue;
   }
   else if (*(b->bf[y].s + i) == HFE)
   {
    for (; j < COL_NUM_ON_SCREEN_RIGHT; j++)
     e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
       ' ', s->fb->hh.fb);
    return;
   }
   else if (*(b->bf[y].s + i) == HNF)
   {
    for (k = j, i++; b->bf[y].s[i] != ':' && i < b->bf[y].len &&
      j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
     e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1,
       y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, *(b->bf[y].s + i), s->fb->hb.fb);
#ifdef NEWSTYLE
    if (WpeIsXwin())
     e_make_xrect(f->a.x-NUM_COLS_OFF_SCREEN_LEFT+k+1, y-NUM_LINES_OFF_SCREEN_TOP+f->a.y+1,
       f->a.x-NUM_COLS_OFF_SCREEN_LEFT+j, y-NUM_LINES_OFF_SCREEN_TOP+f->a.y+1, 0);
#endif
    for (; b->bf[y].s[i] != HED && i < b->bf[y].len &&
      j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
     e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1,
       y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, *(b->bf[y].s + i), frb);
    for (i++; b->bf[y].s[i] != HED && i < b->bf[y].len &&
      j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
     e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1,
       y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, ' ', frb);
    for (; b->bf[y].s[i] != '.' && i < b->bf[y].len &&
      j < COL_NUM_ON_SCREEN_RIGHT; i++, j++)
     e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1,
       y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1, ' ', frb);
    j--;
    continue;
   }
   else if (*(b->bf[y].s + i) == HED) {  j--;  continue;  }
  }
  if (*(b->bf[y].s + i) == WPE_TAB)
   for (k = f->ed->tabn - j % f->ed->tabn; k > 1 &&
     j < NUM_COLS_ON_SCREEN + NUM_COLS_OFF_SCREEN_LEFT - 2; k--, j++)
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      ' ', frb);
  else if (!WpeIsXwin() && ((unsigned char)*(b->bf[y].s + i)) > 126)
  {
   e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j +1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
     '@', frb);
   if (++j >= COL_NUM_ON_SCREEN_RIGHT) return;
   if (((unsigned char)*(b->bf[y].s + i)) < 128 + ' ' &&
     j < COL_NUM_ON_SCREEN_RIGHT)
   {
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j +1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      '^', frb);
    if (++j >= COL_NUM_ON_SCREEN_RIGHT) return;
   }
  }
  else if (*(b->bf[y].s + i) < ' ')
  {
   e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j +1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
     '^', frb);
   if (++j >= COL_NUM_ON_SCREEN_RIGHT) return;
  }
  if (*(b->bf[y].s + i) == WPE_TAB)
   e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,                  ' ', frb);
  else if (!WpeIsXwin() && ((unsigned char)*(b->bf[y].s + i)) > 126 &&
    j < COL_NUM_ON_SCREEN_RIGHT)
  {
   if (((unsigned char)*(b->bf[y].s + i)) < 128 + ' ')
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      ((unsigned char) *(b->bf[y].s + i)) + 'A' - 129, frb);
   else
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      ((unsigned char) *(b->bf[y].s + i)) - 128, frb);
  }
  else if (*(b->bf[y].s + i) < ' ' && j < COL_NUM_ON_SCREEN_RIGHT)
   e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
     *(b->bf[y].s + i) + 'A' - 1, frb);
  else
   e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
     *(b->bf[y].s + i), frb);
 }

 if ((i == b->bf[y].len) && (f->ed->edopt & ED_SHOW_ENDMARKS) &&
   (DTMD_ISMARKABLE(f->dtmd)) && (j < COL_NUM_ON_SCREEN_RIGHT))
 {
  if ((y < s->mark_end.y && ( y > s->mark_begin.y ||
    (y == s->mark_begin.y && i >= s->mark_begin.x) ) ) ||
    (y == s->mark_end.y && i < s->mark_end.x && ( y > s->mark_begin.y ||
    (y == s->mark_begin.y && i >= s->mark_begin.x) ) ) )
  {
   if (*(b->bf[y].s + i) == WPE_WR)
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      PWR, s->fb->ez.fb);
   else
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      PNL, s->fb->ez.fb);
  }
  else
  {
   if (*(b->bf[y].s + i) == WPE_WR)
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      PWR, s->fb->et.fb);
   else
    e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
      PNL, s->fb->et.fb);
  }
  j++;
 }

 for (; j < COL_NUM_ON_SCREEN_RIGHT; j++)
  e_pr_char(f->a.x - NUM_COLS_OFF_SCREEN_LEFT + j + 1, y - NUM_LINES_OFF_SCREEN_TOP + f->a.y + 1,
    ' ', s->fb->et.fb);
}

/*   draw standard-box frame  */
void e_std_rahmen(int xa, int ya, int xe, int ye, char *name, int sw, int frb,
  int fes)
{
 int i;
 char rhm[2][6];
 char *short_name;
   
 rhm[0][0] = RE1; rhm[0][1] = RE2; rhm[0][2] = RE3; rhm[0][3] = RE4;
 rhm[0][4] = RE5; rhm[0][5] = RE6; rhm[1][0] = RD1; rhm[1][1] = RD2;
 rhm[1][2] = RD3; rhm[1][3] = RD4; rhm[1][4] = RD5; rhm[1][5] = RD6;
 for (i = xa + 1; i < xe; i++)
 {
  e_pr_char(i, ya, rhm[sw][4], frb);
  e_pr_char(i, ye, rhm[sw][4], frb);
 }
 for (i = ya + 1; i < ye; i++)
 {
  e_pr_char(xa, i, rhm[sw][5], frb);
  e_pr_char(xe, i, rhm[sw][5], frb);
 }
 e_pr_char(xa, ya, rhm[sw][0], frb);
 e_pr_char(xa, ye, rhm[sw][2], frb);
 e_pr_char(xe, ya, rhm[sw][1], frb);
 e_pr_char(xe, ye, rhm[sw][3], frb);

 if (name)
 {
  if (strlen(name) < xe - xa - 14)
   e_pr_str((xa+xe-strlen(name))/2, ya, name, frb, 0, 0, 0, 0);
  else
  {
   short_name = strdup(name);
   strcpy(short_name + xe - xa - 17, "...");
   e_pr_str(xa + 7, ya, short_name, frb, 0, 0, 0, 0);
   free(short_name);
  }
 }
 if (sw != 0)
 {
  e_pr_char(xa+3, ya, WBT, fes);
#ifdef NEWSTYLE
  if (!WpeIsXwin())
#endif
  {
   e_pr_char(xa+2, ya, '[', frb);
   e_pr_char(xa+4, ya, ']', frb);
  }
#ifdef NEWSTYLE
  else e_make_xrect(xa+2, ya, xa+4, ya, 0);
 }
 if (WpeIsXwin()) e_make_xr_rahmen(xa, ya, xe, ye, sw);
#else
 }
/*
   if(xe < MAXSCOL-1) for(i = ya+1; i <= ye; i++) e_pt_col(xe+1, i, SHDCOL);
   if(ye < MAXSLNS-2) for(i = xa+1; i <= xe+1 && i < MAXSCOL; i++) 
						e_pt_col(i, ye+1, SHDCOL);
*/
#endif
}

struct dirfile *e_add_df(char *str, struct dirfile *df)
{
   int i, n;
   char *tmp;

   if (df == NULL)
   {  df = MALLOC(sizeof(struct dirfile));
      df->anz = 0;
      df->name = MALLOC(sizeof(char*));
   }
   for(n = 0; n < df->anz && *df->name[n] && strcmp(df->name[n], str); n++);
   if(n == df->anz)
   {  if(df->anz == MAXSVSTR - 1) FREE(df->name[df->anz-1]);
      else
      {  df->anz++;
	 df->name = REALLOC(df->name, df->anz * sizeof(char*));
      }
      for(i = df->anz-1; i > 0; i--) df->name[i] = df->name[i-1];
      df->name[0] = MALLOC((strlen(str)+1) * sizeof(char));
      strcpy(df->name[0], str);
   }
   else
   {  tmp = df->name[n];
      for(i = n; i > 0; i--) df->name[i] = df->name[i-1];
      if(!tmp[0])
      {  FREE(tmp);
	 df->name[0] = MALLOC((strlen(str) + 1) * sizeof(char));
	 strcpy(df->name[0], str);
      }
      else df->name[0] = tmp;
   }
   return(df);
}

int e_sv_window(int xa, int ya, int *n, struct dirfile *df, FENSTER *f)
{
 ECNT *cn = f->ed;
 int ret, ye = ya + 6;
 int xe = xa +21;
 FLWND *fw = MALLOC(sizeof(FLWND));

 if ((f = (FENSTER *) MALLOC(sizeof(FENSTER))) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
 if (xe > MAXSCOL-3) {  xe = MAXSCOL - 3;  xa = xe - 21;  }
 if (ye > MAXSLNS-3) {  ye = MAXSLNS - 3;  ya = ye - 6;  }
 f->fb = cn->fb;
 f->a = e_set_pnt(xa, ya);
 f->e = e_set_pnt(xe, ye);
 f->dtmd = DTMD_FILEDROPDOWN;
 f->zoom = 0;
 f->ed = cn;
 f->c_sw = NULL;
 f->c_st = NULL;
 f->fd.dirct = NULL;
 f->winnum = -1;
 f->datnam = "";
 if (!(f->pic = e_ed_kst(f, NULL, 1)))
 {  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(0);  }
 f->b = (BUFFER *)fw;
 fw->mxa = xa; fw->mxe = xe; fw->mya = ya; fw->mye = ye;
 fw->xa = xa+1; fw->xe = xe; fw->ya = ya+1; fw->ye = ye;
 fw->df = df; fw->srcha = 0; fw->f = f;
 fw->nf = fw->ia = fw->ja = 0;
 do
 {
  ret = e_file_window(0, fw, f->fb->er.fb, f->fb->ez.fb);
#if MOUSE
  if (ret < 0) ret = e_rahmen_mouse(f);
#endif
  if ((ret == AF2 && !(f->ed->edopt & ED_CUA_STYLE)) ||
    (f->ed->edopt & ED_CUA_STYLE && ret == CtrlL))
   e_size_move(f);
 } while(ret != WPE_CR && ret != WPE_ESC);
 *n = fw->nf;
 e_close_view(f->pic, 1);
 FREE(fw);
 FREE(f);
 return(ret);
}

int e_schr_lst_wsv(char *str, int xa, int ya, int n, int len, int ft,
  int fz, struct dirfile **df, FENSTER *f)
{
#if MOUSE
   extern struct mouse e_mouse;
#endif
   int ret, num;
   do
   {  *df = e_add_df(str, *df);
#ifndef NEWSTYLE
      ret = e_schreib_leiste(str, xa, ya, n-4, len, ft, fz);
#else
      ret = e_schreib_leiste(str, xa, ya, n-3, len, ft, fz);
#endif
#if MOUSE
      if(ret < 0 && e_mouse.y == ya && e_mouse.x >= xa+n-3
				&& e_mouse.x <= xa+n-1) ret = CDO;
#endif
      if (ret == CDO && e_sv_window(xa+n, ya, &num, *df, f) == WPE_CR)
          strcpy(str, (*df)->name[num]);
   }  while(ret == CDO);
   return(ret);
}

int e_schr_nchar_wsv(char *str, int x, int y, int n, int max, int col,
  int csw)
#ifdef NEWSTYLE
{
 e_pr_char(x+max-3, y, ' ', csw);
 e_pr_char(x+max-2, y, WSW, csw);
 e_pr_char(x+max-1, y, ' ', csw);
 e_make_xrect(x+max-3, y, x+max-1, y, 0);
 return(e_schr_nchar(str, x, y, n, max-3, col));
}
#else
{
#if !defined(NO_XWINDOWS)
 int swcol = (e_gt_col(x+max, y) / 16) * 16;
 if (WpeIsXwin())
 {
  e_pr_char(x+max, y, SCR, swcol);
  e_pr_char(x+max, y+1, SCD, swcol);
  e_pr_char(x+max-1, y+1, SCD, swcol);
  e_pr_char(x+max-2, y+1, SCD, swcol);
 }
#endif
 e_pr_char(x+max-3, y, ' ', csw);
 e_pr_char(x+max-2, y, WSW, csw);
 e_pr_char(x+max-1, y, ' ', csw);
 return(e_schr_nchar(str, x, y, n, max-4, col));
}
#endif

int e_mess_win(char *header, char *str, PIC **pic, FENSTER *f)
{
 ECNT *cn = f->ed;
 extern int (*e_u_kbhit)(void);
#if MOUSE
 extern struct mouse e_mouse;
#endif
 int xa, ya, xe, ye, num, anz = 0, mxlen = 0, i, j;
 char **s;

 s = StringToStringArray(str, &mxlen, strlen(header) + 8, &anz);

 ya = (MAXSLNS - anz - 6)/2;
 ye = ya + anz + 5;
 xa = (MAXSCOL - mxlen - 6)/2;
 xe = xa + mxlen + 6;
 if (ya < 2) ya = 2;
 if (ye > MAXSLNS-3) ye = MAXSLNS - 3;
 num = anz;
 if (num > ye - ya - 5)
 {
  num = ye - ya - 5;
  strcpy(s[num-1], "...");
 }

 if (!(*pic) || (*pic)->e.x != xe || (*pic)->a.x != xa || (*pic)->e.x < xe)
 {
  *pic = e_change_pic(xa, ya, xe, ye, *pic, 1, cn->fb->nt.fb);
  for (i = xa + 1; i < xe; i++)
  {
   e_pr_char(i, ye-2, ' ', f->fb->nt.fb);
   e_pr_char(i, ye-1, ' ', f->fb->nt.fb);
  }
  e_pr_str((xe + xa - 6)/2, ye-2, "Ctrl C", cn->fb->nz.fb, -1, -1,
    cn->fb->ns.fb, cn->fb->nt.fb);
 }
 e_std_rahmen(xa, ya, xe, ye, header, 1, cn->fb->nr.fb, cn->fb->ne.fb);
 for (i = xa + 1; i < xe; i++)
  e_pr_char(i, ya+1, ' ', cn->fb->nr.fb);
 for (j = 0; j < num; j++)
 {
  e_pr_char(xa+1, ya+2+j, ' ', cn->fb->nt.fb);
  e_pr_char(xa+2, ya+2+j, ' ', cn->fb->nt.fb);
  e_pr_str(xa+3, ya+2+j, s[j], cn->fb->nt.fb, 0, 0, 0, 0);
  for (i = xa+strlen(s[j])+3; i < xe; i++)
   e_pr_char(i, ya+2+j, ' ', cn->fb->nt.fb);
 }
 for (j += ya+2; j < ye-2; j++)
  for (i = xa + 1; i < xe; i++)
   e_pr_char(i, j, ' ', cn->fb->nt.fb);
 for (i = 0; i < anz; i++)
  FREE(s[i]);
 FREE(s);
#ifndef NO_XWINDOWS
 if (WpeIsXwin())
 {
  while ((i = (*e_u_kbhit)()))
  {
   if (i == -1 && e_mouse.y == ye-2 && e_mouse.x > (xe + xa - 10)/2 &&
     e_mouse.x < (xe + xa + 6)/2 )
    i = CtrlC;
   if (i == CtrlC) break;
  }
 }
 else
#endif
  while ((i = (*e_u_kbhit)()) && i != CtrlC)
   ;
 return(i == CtrlC ? 1 : 0);
}

int e_opt_sec_box(int xa, int ya, int num, OPTK *opt, FENSTER *f, int sw)
{
   PIC *pic;
   int n, nold, max = 0, i, c = 0, xe, ye = ya + num + 1;
   for(i = 0; i < num; i++)
   if((n = strlen(opt[i].t)) > max) max = n;
   xe = xa + max + 3;
   pic = e_std_kst(xa, ya, xe, ye, NULL, sw, f->fb->nr.fb, f->fb->nt.fb, f->fb->ne.fb);
   if(pic == NULL)  {  e_error(e_msg[ERR_LOWMEM], 0, f->fb); return(-2);  }
   for (i = 0; i < num; i++)
   e_pr_str_wsd(xa+2, ya+i+1, opt[i].t, f->fb->mt.fb, opt[i].x,
		1, f->fb->ms.fb, xa+1, xe-1);
#if  MOUSE
   while (e_mshit() != 0);
#endif
   n = 0; nold = 1;
   while (c != WPE_ESC && c != WPE_CR)
   {  if (nold != n)
      {  e_pr_str_wsd(xa+2, nold+ya+1, opt[nold].t, f->fb->mt.fb,
				opt[nold].x, 1, f->fb->ms.fb, xa+1, xe-1);
	 e_pr_str_wsd(xa+2, n+ya+1, opt[n].t, f->fb->mz.fb,
				opt[n].x, 1, f->fb->mz.fb, xa+1, xe-1);
	 nold = n;
      }
#if  MOUSE
      if( (c = e_toupper(e_getch())) == -1)
      c = e_m2_mouse(xa, ya, xe, ye, opt);
#else
      c = e_toupper(e_getch());
#endif
      for (i = 0; i < ye - ya - 1; i++)
      if( c == opt[i].o) {  c = WPE_CR;  n = i;  break;  }
      if (i > ye - ya) c = WPE_ESC;
      else if ( c == CUP || c == CtrlP ) n = n > 0 ? n-1 : ye - ya - 2 ;
      else if ( c == CDO || c == CtrlN ) n = n < ye-ya-2 ? n+1 : 0 ;
      else if ( c == POS1 || c == CtrlA ) n = 0;
      else if ( c == ENDE || c == CtrlE ) n = ye-ya-2;
   }
   if(sw == 1) e_close_view(pic, 1);
   return(c == WPE_ESC ? -1 : n);
}

struct dirfile *e_make_win_list(FENSTER *f)
{
 int i;
 struct dirfile *df;

 if (!(df = MALLOC(sizeof(struct dirfile)))) return(NULL);
 df->anz = f->ed->mxedt;
 if (!(df->name = MALLOC(df->anz * sizeof(char *))))
 {
  FREE(df);
  return(NULL);
 }
 for (i = 0; i < df->anz; i++)
 {
  if (f->ed->f[df->anz-i]->datnam)
  {
   if (!(df->name[i] =
     MALLOC((strlen(f->ed->f[df->anz-i]->datnam)+1) * sizeof(char))))
   {
    df->anz = i;
    freedf(df);
    return(NULL);
   }
   else strcpy(df->name[i], f->ed->f[df->anz-i]->datnam);
  }
  else
  {
   if (!(df->name[i] = MALLOC(sizeof(char))))
   {
    df->anz = i;
    freedf(df);
    return(NULL);
   }
   else *df->name[i] = '\0';
  }
 }
 return(df);
}

int e_list_all_win(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0; i--)
  if (f->ed->f[i]->dtmd == DTMD_DATA && f->ed->f[i]->ins == 7)
  {
   e_switch_window(f->ed->edt[i], f);
   return(0);
  }
 return(e_data_first(7, f->ed, NULL));
}

#ifdef NEWSTYLE
int e_get_pic_xrect(int xa, int ya, int xe, int ye, PIC *pic)
{
 int i = xa, j, ebbg;

 ebbg = (xe - xa + 1) * 2 * (ye - ya + 1);
 for (j = ya; j <= ye; ++j)
  for (i = xa; i <= xe; ++i)
   *( pic->p + ebbg + (j-ya)*(xe-xa+1) + (i-xa) ) = extbyte[j*MAXSCOL + i];
 return(i);
}

int e_put_pic_xrect(PIC *pic)
{
 int i = 0, j;
 int ebbg = (pic->e.x - pic->a.x + 1) * 2 * (pic->e.y - pic->a.y + 1);

 for (j = pic->a.y; j <= pic->e.y; ++j)
  for (i = pic->a.x; i <= pic->e.x; ++i)
   extbyte[j*MAXSCOL+i] =
     *(pic->p + ebbg + (j-pic->a.y)*(pic->e.x-pic->a.x+1) + (i-pic->a.x));
 return(i);
}

int e_make_xrect_abs(int xa, int ya, int xe, int ye, int sw)
{
 int j;

 for (j = xa; j <= xe; j++)
  *(extbyte+ya*MAXSCOL+j) = *(extbyte+ye*MAXSCOL+j) = 0;
 for (j = ya; j <= ye; j++)
  *(extbyte+j*MAXSCOL+xa) = *(extbyte+j*MAXSCOL+xe) = 0;
 return(e_make_xrect(xa, ya, xe, ye, sw));
}

int e_make_xrect(int xa, int ya, int xe, int ye, int sw)
{
 int j;

 if (sw & 2)
 {
  sw = (sw & 1) ? 16 : 0;
  for (j = xa+1; j < xe; j++)
  {
   *(extbyte+ya*MAXSCOL+j) |= (sw | 4);
   *(extbyte+ye*MAXSCOL+j) |= (sw | 1);
  }
  for (j = ya+1; j < ye; j++)
  {
   *(extbyte+j*MAXSCOL+xa) |= (sw | 2);
   *(extbyte+j*MAXSCOL+xe) |= (sw | 8);
  }
 }
 else
 {
  sw = (sw & 1) ? 16 : 0;
  for (j = xa; j <= xe; j++)
  {
   *(extbyte+ya*MAXSCOL+j) |= (sw | 1);
   *(extbyte+ye*MAXSCOL+j) |= (sw | 4);
  }
  for (j = ya; j <= ye; j++)
  {
   *(extbyte+j*MAXSCOL+xa) |= (sw | 8);
   *(extbyte+j*MAXSCOL+xe) |= (sw | 2);
  }
 }
 return(j);
}

int e_make_xr_rahmen(int xa, int ya, int xe, int ye, int sw)
{
 if (!sw)
 {
  e_make_xrect(xa, ya, xe, ye, 0);
  e_make_xrect(xa, ya, xe, ye, 2);
 }
 else
 {
  e_make_xrect(xa+1, ya, xe-1, ya, 0);
  e_make_xrect(xa+1, ye, xe-1, ye, 0);
  e_make_xrect(xa, ya+1, xa, ye-1, 0);
  e_make_xrect(xe, ya+1, xe, ye-1, 0);
  e_make_xrect(xa, ya, xa, ya, 0);
  e_make_xrect(xe, ya, xe, ya, 0);
  e_make_xrect(xe, ye, xe, ye, 0);
  e_make_xrect(xa, ye, xa, ye, 0);
 }
 return(sw);
}
#endif

