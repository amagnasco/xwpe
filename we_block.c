/* we_block.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"
#include <ctype.h>

extern int e_undo_sw;

/*	delete block */
int e_blck_del(FENSTER *f)
{
 BUFFER *b;
 SCHIRM *s;
 int i,y,len;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  s = f->s;
 if ((s->mark_end.y < s->mark_begin.y) ||
   ((s->mark_begin.y == s->mark_end.y) && (s->mark_end.x <= s->mark_begin.x)))
 {
  return(0);
 }
 if (f->ins == 8)
  return(WPE_ESC);
 if (s->mark_begin.y == s->mark_end.y)
 {
  e_del_nchar(b, s, s->mark_begin.x, s->mark_begin.y,
    s->mark_end.x - s->mark_begin.x);
  b->b.x = s->mark_end.x = s->mark_begin.x;
 }
 else
 {
/***********************/ 
  y=s->mark_begin.y;
  if(s->mark_begin.x>0) y++;
  len=y-s->mark_end.y+1;
/***********************/ 

  e_add_undo('d', b, s->mark_begin.x, s->mark_begin.y, 0);
  f->save = b->cn->maxchg + 1;
  
/***********************/ 
  e_brk_recalc(f,y,len);
/***********************/ 
 }
 sc_txt_1(f);
 e_cursor(f, 1);
 e_schirm(f, 1);
 return(0);
}

/*      dup selected block BD*/
int e_blck_dup(char *dup, FENSTER *f)
{
 BUFFER *b;
 SCHIRM *s;
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 if (f->ins == 8)
  return(0);
 b = f->b;  s = f->s;
 i=s->mark_end.x - s->mark_begin.x;
 /* Brian thinks that 80 characters is enough.  For the time being return
    nothing if longer than that. */
 if ((s->mark_end.y != s->mark_begin.y) || (i <= 0) || (i >= 80))
 {
  return(0);
 }
 strncpy(dup, &b->bf[s->mark_begin.y].s[s->mark_begin.x], i);
 dup[i]=0;
 return(i);
}

int e_blck_clear(BUFFER *b, SCHIRM *s)
{
 int i;
 int len = (s->mark_end.y-s->mark_begin.y-1);

 if (s->mark_end.y == s->mark_begin.y)
 {
  e_del_nchar(b, s, s->mark_begin.x, s->mark_end.y,
    s->mark_end.x - s->mark_begin.x);
  b->b.x = s->mark_end.x = s->mark_begin.x;
  b->b.y = s->mark_end.y = s->mark_begin.y;
 return(0);
 }
 for (i = s->mark_begin.y + 1; i < s->mark_end.y; i++)
  FREE(b->bf[i].s);
 for (i = s->mark_begin.y + 1; i <= b->mxlines-len; i++)
  b->bf[i] = b->bf[i+len];
 (b->mxlines) -= len;
 (s->mark_end.y) -= len;
 e_del_nchar(b, s, 0, s->mark_end.y, s->mark_end.x);
 if (*(b->bf[s->mark_begin.y].s+(len = b->bf[s->mark_begin.y].len)) != '\0')
  len++;
 e_del_nchar(b, s, s->mark_begin.x, s->mark_begin.y, len - s->mark_begin.x);
 b->b.x = s->mark_end.x = s->mark_begin.x;
 b->b.y = s->mark_end.y = s->mark_begin.y;
 return(0);
}

/*   write buffer to screen */
int e_show_clipboard(FENSTER *f)
{
 ECNT *cn = f->ed;
 FENSTER *fo;
 int i, j;

 for (j = 1; j <= cn->mxedt; j++)
  if (cn->f[j] == cn->f[0])
  {
   e_switch_window(cn->edt[j], f);
   return(0);
  }

 if (cn->mxedt > MAXEDT)
 {
  e_error(e_msg[ERR_MAXWINS], 0, cn->fb);
  return(0);
 }
 for (j = 1; j <= MAXEDT; j++)
 {
  for (i = 1; i <= cn->mxedt && cn->edt[i] != j; i++)
   ;
  if ( i > cn->mxedt) break;
 }
 cn->curedt=j;
 (cn->mxedt)++;
 cn->edt[cn->mxedt]=j;

 f = cn->f[cn->mxedt] = cn->f[0];
#ifdef PROG
 if (WpeIsProg())
 {
  for (i = cn->mxedt-1; i > 0; i--)
   ;
  if (i < 1)
  {
   f->a = e_set_pnt(0, 1);
   f->e = e_set_pnt(MAXSCOL-1, 2*MAXSLNS/3);
  }
  else
  {
   f->a = e_set_pnt(cn->f[i]->a.x+1, cn->f[i]->a.y+1);
   f->e = e_set_pnt(cn->f[i]->e.x, cn->f[i]->e.y);
  }
 }
 else
#endif
 {
  if (cn->mxedt < 2)
  {
   f->a = e_set_pnt(0, 1);
   f->e = e_set_pnt(MAXSCOL-1, MAXSLNS-2);
  }
  else
  {
   f->a = e_set_pnt(cn->f[cn->mxedt-1]->a.x+1, cn->f[cn->mxedt-1]->a.y+1);
   f->e = e_set_pnt(cn->f[cn->mxedt-1]->e.x, cn->f[cn->mxedt-1]->e.y);
  }
 }
 f->winnum = cn->curedt;

 if (cn->mxedt > 1)
 {
  fo = cn->f[cn->mxedt-1];
  e_ed_rahmen(fo, 0);
 }
 e_firstl(f, 1);
 e_zlsplt(f);
 e_schirm(f, 1);
 e_cursor(f, 1);
 return(0);
}

/*   move block to buffer */
int e_edt_del(FENSTER *f)
{
 e_edt_copy(f);
 e_blck_del(f);
 return(0);
}

/* copy block to buffer */
int e_edt_copy(FENSTER *f)
{
 BUFFER *b;
 BUFFER *b0 = f->ed->f[0]->b;
 int i, save;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  save = f->save;
 if ((f->s->mark_end.y < f->s->mark_begin.y) ||
   ((f->s->mark_begin.y == f->s->mark_end.y) &&
     (f->s->mark_end.x <= f->s->mark_begin.x)))
 {
  return(0);
 }
 for (i = 1; i < b0->mxlines; i++)
  FREE(b0->bf[i].s);
 b0->mxlines = 1;
 *(b0->bf[0].s) = WPE_WR;
 *(b0->bf[0].s+1) = '\0';
 b0->bf[0].len = 0;
 e_copy_block(0, 0, b, b0, f);
 f->save = save;
 return(0);
}

/*            Copy block buffer into window  */
int e_edt_einf(FENSTER *f)
{
 BUFFER *b;
 BUFFER *b0 = f->ed->f[0]->b;
 int i,y,len;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 if (f->ins == 8)
  return(0);
 e_undo_sw = 1;
 
/**********************/ 
 y=b->b.y;
 if(b->b.x>0) y++;
/**********************/ 

 e_copy_block(b->b.x, b->b.y, b0, b, f);
 
/**********************/ 
 len=b0->f->s->mark_end.y-b0->f->s->mark_begin.y;
 e_brk_recalc(f,y,len);
/**********************/ 

 e_undo_sw = 0;
 e_add_undo('c', b, b->b.x, b->b.y, 0);
 sc_txt_2(f);
 f->save = b->cn->maxchg + 1;
 return(0);
}

/*   move block within window */
int e_blck_move(FENSTER *f)
{
 BUFFER *b;
 int i;
 POINT ka;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 ka.x = f->ed->f[f->ed->mxedt]->s->mark_begin.x;
 if (b->b.y > f->ed->f[f->ed->mxedt]->s->mark_begin.y)
  ka.y = f->ed->f[f->ed->mxedt]->s->mark_begin.y;
 else
 {
  ka.y = f->ed->f[f->ed->mxedt]->s->mark_end.y;
  if (b->b.y == f->ed->f[f->ed->mxedt]->s->mark_begin.y &&
    b->b.x < f->ed->f[f->ed->mxedt]->s->mark_begin.x)
   ka.x = f->ed->f[f->ed->mxedt]->s->mark_end.x +
     f->ed->f[f->ed->mxedt]->s->mark_begin.x - b->b.x;
 }
 e_undo_sw = 1;
 e_move_block(b->b.x, b->b.y, b, b, f);
 e_undo_sw = 0;
 e_add_undo('v', b, ka.x, ka.y, 0);
 f->save = b->cn->maxchg + 1;
 return(0);
}

/*    move Block    */
void e_move_block(int x, int y, BUFFER *bv, BUFFER *bz, FENSTER *f)
{
 SCHIRM *s = f->ed->f[f->ed->mxedt]->s;
 SCHIRM *sv = bv->f->s;
 SCHIRM *sz = bz->f->s;
 int sw = (y < s->mark_begin.y) ? 0 : 1, i, n = s->mark_end.y - s->mark_begin.y - 1;
 int kax = s->mark_begin.x, kay = s->mark_begin.y, kex = s->mark_end.x, key = s->mark_end.y;
 STRING *str, *tmp;
 unsigned char *cstr;

 if (key < kay || (kay == key && kex <= kax)) return;
 if (f->ins == 8)
 {
  return;
 }
 if (bv == bz && y >= kay && y <= key && x >= kax && x <= kex)
 {
  return;
 }
 if (kay == key)
 {
  n = kex - kax;
  bz->b.x = x; bz->b.y = y;
  if ((cstr = MALLOC(f->ed->maxcol * sizeof(char))) == NULL)
  {
   e_error(e_msg[ERR_LOWMEM], 0, bz->fb);
   return;
  }
  for (i = 0; i < n; i++)
   cstr[i] = bv->bf[key].s[kax+i];
  e_ins_nchar(bz, sz, cstr, x, y, n);
  bv->b.x = kax;  bv->b.y = kay+bz->b.y-y;
  e_del_nchar(bv, sv, sv->mark_begin.x, sv->mark_begin.y, n);
  if (bv == bz && kay == y && x > kex)
   x -= n;
  sz->mark_begin.x = x;   sz->mark_end.x = bz->b.x = x + n;
  sz->mark_begin.y = sz->mark_end.y = bz->b.y = y;
  e_cursor(f, 1);
  e_schirm(f, 1);
  FREE(cstr);
  return;
 }

 if (bv == bz && y == kay && x < kax )
 {
  n = kax - x;
  bv->b.x = x; bv->b.y = y;
  if ((cstr = MALLOC(f->ed->maxcol * sizeof(char))) == NULL)
  {
   e_error(e_msg[ERR_LOWMEM], 0, bz->fb);
   return;
  }
  for (i = 0; i < n; i++)
   cstr[i] = bv->bf[y].s[x+i];
  e_del_nchar(bv, sv, x, y, n);
  bz->b.x = kex; bz->b.y = key;
  e_ins_nchar(bz, sz, cstr, kex, key, n);
  bv->b.x = sv->mark_begin.x = x;  bv->b.y = sv->mark_begin.y = y;
  sv->mark_end.x = kex;  sv->mark_end.y = key;
  e_cursor(f, 1);
  e_schirm(f, 1);
  FREE(cstr);
  return;
 }

 if (bv == bz && y == key && x > kex )
 {
  n = x - kex;
  bv->b.x = kex; bv->b.y = y;
  if ((cstr = MALLOC(f->ed->maxcol * sizeof(char))) == NULL)
  {
   e_error(e_msg[ERR_LOWMEM], 0, bz->fb);
   return;
  }
  for (i = 0; i < n; i++)
   cstr[i] = bv->bf[y].s[kex+i];
  e_del_nchar(bv, sv, kex, y, n);
  bz->b.x = kex; bz->b.y = key;
  e_ins_nchar(bz, sz, cstr, kax, kay, n);

  bv->b.x = sv->mark_end.x;  bv->b.y = sv->mark_end.y;
  e_cursor(f, 1);
  e_schirm(f, 1);
  FREE(cstr);
  return;
 }

 while (bz->mxlines+n > bz->mx.y-2)
 {
  bz->mx.y += MAXLINES;
  if ((tmp = REALLOC(bz->bf, bz->mx.y * sizeof(STRING))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, bz->fb);
  else
   bz->bf = tmp;
  if (bz->f->c_sw)
   bz->f->c_sw = REALLOC(bz->f->c_sw, bz->mx.y * sizeof(int));
 }
 if ((str = MALLOC((n+2) * sizeof(STRING))) == NULL)
 {
  e_error(e_msg[ERR_LOWMEM], 0, bz->fb);
  return;
 }
 for (i = kay; i <= key; i++)
  str[i-kay] = bv->bf[i];
 e_new_line(y+1, bz);
 if (*(bz->bf[y].s+bz->bf[y].len) != '\0')
  (bz->bf[y].len)++;
 for (i = x;  i <= bz->bf[y].len; i++)
  *(bz->bf[y+1].s+i-x) = *(bz->bf[y].s+i);
 *(bz->bf[y].s+x) = '\0';
 bz->bf[y].len = bz->bf[y].nrc = x;
 bz->bf[y+1].len = e_str_len(bz->bf[y+1].s);
 bz->bf[y+1].nrc = e_str_nrc(bz->bf[y+1].s);
 for (i = bz->mxlines; i > y; i--)
  bz->bf[i+n] = bz->bf[i];
 (bz->mxlines) += n;
 for (i = 1; i <= n; i++)
  bz->bf[y+i] = str[i];

 if (bz == bv)
 {
  if (y < kay)
  {  kay += (n+1);  key +=(n+1);  }
  else if (y > kay)
   y -= n;
 }

 for (i = kay + 1; i <= bv->mxlines-n; i++)
  bv->bf[i] = bv->bf[i+n];
 (bv->mxlines) -= n;

 if (*(str[0].s+(n = str[0].len)) != '\0')
  n++;
 e_ins_nchar(bz, sz, (str[key-kay].s), 0, y+key-kay, kex);
 e_ins_nchar(bz, sz, (str[0].s+kax), x, y, n-kax);
 e_del_nchar(bv, sv, 0, kay+1, kex);
 e_del_nchar(bv, sv, kax, kay, n - kax);

 if (bz == bv && sw != 0)
  y--;

 sv->mark_begin.x = sv->mark_end.x = bv->b.x = kax;
 sv->mark_begin.y = sv->mark_end.y = bv->b.y = kay;

 sz->mark_begin.x = x;
 sz->mark_begin.y = y;
 bz->b.x = sz->mark_end.x = kex;
 bz->b.y = sz->mark_end.y = key-kay+y;

 f->save = f->ed->maxchg + 1;
 sc_txt_1(bv->f);
 sc_txt_1(bz->f);
 e_cursor(f, 1);
 e_schirm(f, 1);
 FREE(str);
}

/*   copy block within window   */
int e_blck_copy(FENSTER *f)
{
 BUFFER *b;
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 if (f->ins == 8) return(0);
 f->save = 1;
 e_undo_sw = 1;
 e_copy_block(b->b.x, b->b.y, b, b, f);
 e_undo_sw = 0;
 e_add_undo('c', b, b->b.x, b->b.y, 0);
 sc_txt_2(f);
 f->save = b->cn->maxchg + 1;
 return(0);
}

/*   copy block  */
void e_copy_block(int x, int y, BUFFER *buffer_src, BUFFER *buffer_dst,
  FENSTER *f)
{
 BUFFER *b = f->ed->f[f->ed->mxedt]->b;
 SCHIRM *s_src = buffer_src->f->s;
 SCHIRM *s_dst = buffer_dst->f->s;
 int i, j, n = s_src->mark_end.y - s_src->mark_begin.y - 1;
 int kax = s_src->mark_begin.x, kay = s_src->mark_begin.y, kex = s_src->mark_end.x, key = s_src->mark_end.y;
 int kse = key, ksa = kay;
 STRING **str, *tmp;
 unsigned char *cstr;

 if (key < kay || (kay == key && kex <= kax)) return;
 if ((cstr = MALLOC(buffer_src->mx.x + 1)) == NULL)
 {
  e_error(e_msg[ERR_LOWMEM], 0, buffer_dst->fb);
  return;
 }
 if (kay == key)
 {
  if (buffer_src == buffer_dst && y == kay && x >= kax && x < kex)
  {
   FREE(cstr);
   return;
  }
  n = kex - kax;
  buffer_dst->b.x = x; buffer_dst->b.y = y;
  for (i = 0; i < n; i++)
   cstr[i] = buffer_src->bf[key].s[kax+i];
  e_ins_nchar(buffer_dst, s_dst, cstr, x, y, n);
  s_dst->mark_begin.x = x;
  s_dst->mark_end.x = buffer_dst->b.x = x + n;
  s_dst->mark_begin.y = s_dst->mark_end.y = buffer_dst->b.y = y;
  FREE(cstr);
  e_cursor(f, 1);
  e_schirm(f, 1);
  return;
 }
   
 if (buffer_src == buffer_dst && ( (y > kay && y < key) ||
   ( y == kay && x >= kax ) || ( y == key && x < kex) ))
 {
  FREE(cstr);
  return;
 }
 while (buffer_dst->mxlines+n > buffer_dst->mx.y-2)
 {
  buffer_dst->mx.y += MAXLINES;
  if ((tmp = REALLOC(buffer_dst->bf, buffer_dst->mx.y * sizeof(STRING))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, buffer_dst->fb);
  else
   buffer_dst->bf = tmp;
  if (buffer_dst->f->c_sw)
   buffer_dst->f->c_sw = REALLOC(buffer_dst->f->c_sw , buffer_dst->mx.y * sizeof(int));
 }
 if ((str = MALLOC((n+2) * sizeof(STRING *))) == NULL)
 {
  e_error(e_msg[ERR_LOWMEM], 0, buffer_dst->fb);
  FREE(cstr);
  return;
 }
 e_new_line(y+1, buffer_dst);
 if (buffer_dst == buffer_src && y < ksa)
 {
  kse += (n+1);
  ksa += (n+1);
 }
 if (*(buffer_dst->bf[y].s+buffer_dst->bf[y].len) != '\0')
  (buffer_dst->bf[y].len)++;
 for (i = x;  i <= buffer_dst->bf[y].len; i++)
  *(buffer_dst->bf[y+1].s+i-x) = *(buffer_dst->bf[y].s+i);
 *(buffer_dst->bf[y].s+x) = '\0';
 buffer_dst->bf[y].len = buffer_dst->bf[y].nrc = x;
 buffer_dst->bf[y+1].len = e_str_len(buffer_dst->bf[y+1].s);
 buffer_dst->bf[y+1].nrc = e_str_nrc(buffer_dst->bf[y+1].s);
 for (i = buffer_dst->mxlines; i > y; i--)
  buffer_dst->bf[i+n] = buffer_dst->bf[i];
 (buffer_dst->mxlines) += n;
 for (i = ksa; i <= kse; i++)
  str[i-ksa] = &(buffer_src->bf[i]);
 for (i = 1; i <= n; i++)
  buffer_dst->bf[i+y].s = NULL;
 for (i = 1; i <= n; i++)
 {
  if ((buffer_dst->bf[i+y].s = MALLOC(buffer_dst->mx.x+1)) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, b->fb);
  for (j = 0; j <= str[i]->len; j++)
   *(buffer_dst->bf[i+y].s+j) = *(str[i]->s+j);
  if (*(str[i]->s+str[i]->len) != '\0')
   *(buffer_dst->bf[i+y].s+j) = '\0';
  buffer_dst->bf[i+y].len = str[i]->len;
  buffer_dst->bf[i+y].nrc = str[i]->nrc;
 }

 for (i = 0; i < kex; i++)
  cstr[i] = str[key-kay]->s[i];
 e_ins_nchar(buffer_dst, s_dst, cstr, 0, y+key-kay, kex);
 if (*(str[0]->s+(n = str[0]->len)) != '\0')
  n++;
 for (i = 0; i < n - kax; i++)
  cstr[i] = str[0]->s[i+kax];
 cstr[n-kax] = '\0';
 e_ins_nchar(buffer_dst, s_dst, cstr, x, y, n-kax);
 s_dst->mark_begin.x = x;
 s_dst->mark_begin.y = y;
 buffer_dst->b.x = s_dst->mark_end.x = kex;
 buffer_dst->b.y = s_dst->mark_end.y = key-kay+y;

 e_cursor(f, 1);
 e_schirm(f, 1);
 FREE(cstr);
 FREE(str);
}

/*   delete block marks   */
int e_blck_hide(FENSTER *f)
{
 BUFFER *b;
 SCHIRM *s;
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 s = f->s;
 s->mark_begin = e_set_pnt(0, 0);
 s->mark_end = e_set_pnt(0, 0);
 e_schirm(f, 1);
 return(0);
}

/*   mark begin of block   */
int e_blck_begin(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0) return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 f->s->mark_begin = f->b->b;
 e_schirm(f, 1);
 return(0);
}

/*           Set end of block   */
int e_blck_end(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 f->s->mark_end = f->b->b;
 e_schirm(f, 1);
 return(0);
}

/* goto begin of block   */
int e_blck_gt_beg(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 f->b->b = f->s->mark_begin;
 e_schirm(f, 1);
 return(0);
}

/*   goto end of block   */
int e_blck_gt_end(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 f->b->b = f->s->mark_end;
 e_schirm(f, 1);
 return(0);
}

/*   mark text line in block   */
int e_blck_mrk_all(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 f->s->mark_begin.x = 0;
 f->s->mark_begin.y = 0;
 f->s->mark_end.y = f->b->mxlines-1;
 f->s->mark_end.x = f->b->bf[f->b->mxlines-1].len;
 e_schirm(f, 1);
 return(0);
}

/*   mark text line in block   */
int e_blck_mrk_line(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 f->s->mark_begin.x = 0;
 f->s->mark_begin.y = f->b->b.y;
 if(f->b->b.y < f->b->mxlines-1)
 {
  f->s->mark_end.x = 0;
  f->s->mark_end.y = f->b->b.y+1;
 }
 else
 {
  f->s->mark_end.x = f->b->bf[f->b->b.y].len;
  f->s->mark_end.y = f->b->b.y;
 }
 e_schirm(f, 1);
 return(0);
}

/*	block: up- or downcase
mode=0 every character downcase
mode=1 every character upcase
mode=2 first character in each word upcase
mode=3 first character in each line upcase   */
int e_blck_changecase(FENSTER *f, int mode)
{
 BUFFER *b;
 SCHIRM *screen;
 int i,x,y,x_begin,x_end;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0) return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 screen = f->s;

 for (y = screen->mark_begin.y; y <= screen->mark_end.y; y++)
 {
  if (y == screen->mark_begin.y)
   x_begin = screen->mark_begin.x;
  else x_begin = 0;
  if (y == screen->mark_end.y)
   x_end = screen->mark_end.x;
  else x_end = b->bf[y].len;
  for (x = x_begin; x < x_end; x++)
   if (mode == 0)
    b->bf[y].s[x]=tolower(b->bf[y].s[x]);
   else if (mode == 1)
    b->bf[y].s[x]=toupper(b->bf[y].s[x]);
   else if ((mode == 2) && ((x == 0) || (b->bf[y].s[x-1] == ' ') || (b->bf[y].s[x-1] == '\t')))
    b->bf[y].s[x]=toupper(b->bf[y].s[x]);
   else if ((mode == 3) && (x == 0))
    b->bf[y].s[x]=toupper(b->bf[y].s[x]);
 }
 f->save++;
 e_schirm(f, 1);
 return 0;
}

int e_changecase_dialog(FENSTER *f)
{
 static int b_sw = 0;
 int ret;
 W_OPTSTR *o = e_init_opt_kst(f);
 if (!o) return(-1);
 o->xa = 21;
 o->xe = 52;
 o->ya = 4;
 o->ye = 15;

 o->bgsw = AltO;
 o->name = "Change Case";
 o->crsw = AltO;
 e_add_txtstr(4, 2, "Which mode:", o);

 e_add_pswstr(1, 5, 3, 0, AltD, 0, "Downcase          ", o);
 e_add_pswstr(0, 5, 4, 0, AltU, b_sw & 1, "Upcase            ", o);
 e_add_pswstr(0, 5, 5, 0, AltH, b_sw & 2, "Headline          ", o);
 e_add_pswstr(0, 5, 6, 0, AltE, b_sw & 3, "First character   ", o);

 e_add_bttstr(7, 9, 1, AltO, " Ok ", NULL, o);
 e_add_bttstr(18, 9, -1, WPE_ESC, "Cancel", NULL, o);
 ret = e_opt_kst(o);

 if (ret != WPE_ESC)
  e_blck_changecase(f,o->pstr[0]->num);
 freeostr(o);
 return 0;
}

/*   unindent block   */
int e_blck_to_left(FENSTER *f)
{
 BUFFER *b;
 SCHIRM *s;
 int n = f->ed->tabn/2, i, j, k, l, m, nn;
 char *tstr = MALLOC((n+2)*sizeof(char));

 for(i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--);
 if(i <= 0) return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 s = f->s;
 for(i = 0; i <= n; i++) tstr[i] = ' ';
 tstr[n] = '\0';
 for (i = (!s->mark_begin.x) ? s->mark_begin.y : s->mark_begin.y + 1;
   i < s->mark_end.y || (i == s->mark_end.y && s->mark_end.x > 0); i++)
 {
  for(j = 0; j < b->bf[i].len && isspace(b->bf[i].s[j]); j++);
  for(l = j - 1, k = 0; l >= 0 && k < n; l--)
  {
   if(b->bf[i].s[l] == ' ') k++;
   else if(b->bf[i].s[l] == '\t')
   {
    for(nn = m = 0; m < l; m++)
    {
     if(b->bf[i].s[m] == ' ') nn++;
     else if(b->bf[i].s[m] == '\t')
     nn += f->ed->tabn - (nn % f->ed->tabn);
    }
    k += f->ed->tabn - (nn % f->ed->tabn);
   }
  }
  l = j - l - 1;
  if(l > 0)
  {
   nn = s->mark_begin.x;
   e_del_nchar(b, s, j - l, i, l);
   if (k > n) e_ins_nchar(b, s, tstr, j-l, i, k-n);
   s->mark_begin.x = nn;
  }
 }
 FREE(tstr);
 e_schirm(f, 1);
 return(0);
}

/*   indent block   */
int e_blck_to_right(FENSTER *f)
{
 BUFFER *b;
 SCHIRM *s;
 int n = f->ed->tabn/2, i, j;
 char *tstr = MALLOC((n+1)*sizeof(char));

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;
 s = f->s;
 for (i = 0; i < n; i++)
  tstr[i] = ' ';
 tstr[n] = '\0';
 for (i = (!s->mark_begin.x) ? s->mark_begin.y : s->mark_begin.y + 1;
   i < s->mark_end.y || (i == s->mark_end.y && s->mark_end.x > 0); i++)
 {
  for (j = 0; b->bf[i].len && isspace(b->bf[i].s[j]); j++)
   ;
  e_ins_nchar(b, s, tstr, j, i, n);
  if (i == s->mark_begin.y)
   s->mark_begin.x = 0;
 }
 FREE(tstr);
 e_schirm(f, 1);
 return(0);
}

/*            Read block from file   */
int e_blck_read(FENSTER *f)
{
 if (f->ins == 8)
  return(WPE_ESC);
 WpeCreateFileManager(1, f->ed, "");
 f->save = f->ed->maxchg + 1;
 return(0);
}

/*   write block to file   */
int e_blck_write(FENSTER *f)
{
 WpeCreateFileManager(2, f->ed, "");
 return(0);
}

/*   find again   */
int e_rep_search(FENSTER *f)
{
 SCHIRM *s;
 BUFFER *b;
 FIND *fd = &(f->ed->fd);
 int i, ret, j, iend, jend, end;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  s = f->s;
 ret = b->b.x;
 j = b->b.y;
 if ((fd->sw & 8) == 0)
 {
  jend = (fd->sw & 4) ? 0 : b->mxlines-1;
  iend = (fd->sw & 4) ? 0 : b->bf[b->mxlines-1].len;
 }
 else
 {
  jend = (fd->sw & 4) ? s->mark_begin.y : s->mark_end.y;
  iend = (fd->sw & 4) ? s->mark_begin.x : s->mark_end.x;
 }

 for (; ; ret = -1)
 {
  if ((fd->sw & 4) == 0 && j > jend)
   break;
  else if ((fd->sw & 4) != 0 && j < jend)
   break;
  do
  {
   if (j == jend)
    end = iend;
   else if (!(fd->sw & 4))
    end = b->bf[j].len;
   else
    end = 0;
   if (fd->sw & 4 && ret == -1)
    ret = b->bf[j].len;
   if ((fd->sw & 32) == 0)
   {
    if ((fd->sw & 128) != 0)
     ret = e_strstr(ret, end, b->bf[j].s, fd->search);
    else
     ret = e_ustrstr(ret, end, b->bf[j].s, fd->search);
   }
   else
   {
    if ((fd->sw & 128) != 0)
     ret = e_rstrstr(ret, end, b->bf[j].s, fd->search, &(fd->sn));
    else
     ret = e_urstrstr(ret, end, b->bf[j].s, fd->search, &(fd->sn));
   }
   if ((fd->sw & 4) == 0 && j == jend && ret > iend)
    break;
   else if ((fd->sw & 4) != 0 && j == jend && ret < iend)
    break;
   if (ret >= 0 && ((fd->sw & 64) == 0 ||
     (isalnum(*(b->bf[j].s+ret+fd->sn)) == 0 &&
     (ret == 0 || isalnum(*(b->bf[j].s+ret-1)) == 0))))
   {
    s->fa.x = ret;
    b->b.y = s->fa.y = s->fe.y = j;
    s->fe.x = ret + fd->sn;
    b->b.x = ret = !(fd->sw & 4) ? s->fe.x : ret;
    if (!(fd->sw & 1))
    {
     e_cursor(f, 1);
     s->fa.y = j;
     e_schirm(f, 1);
    }
    return(1);
   }
   else if (ret >= 0 && (fd->sw & 64) != 0)
    ret++;
  } while(ret >= 0);
  if ((fd->sw & 4) == 0 && j > jend)
   break;
  else if ((fd->sw & 4) != 0 && j < jend)
   break;
  j = ((fd->sw & 4) == 0) ? j+1 : j-1;
 }
 if (!(fd->sw & 1))
  e_message(0, e_msg[ERR_GETSTRING], f);
 return(0);
}

/*   goto line  */
int e_goto_line(FENSTER *f)
{
 int i, num;
 SCHIRM *s;
 BUFFER *b;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  s = f->s;
 if ((num = e_num_kst("Goto Line Number", b->b.y+1, b->mxlines, f, 0, AltG)) > 0)
  b->b.y = num - 1;
 else if (num == 0)
  b->b.y = num;
 e_cursor(f, 1);
 return(0);
}

int e_find(FENSTER *f)
{
 SCHIRM *s;
 BUFFER *b;
 FIND *fd = &(f->ed->fd);
 int i, ret;
 char strTemp[80];
 W_OPTSTR *o = e_init_opt_kst(f);

 if (!o)
  return(-1);
 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  s = f->s;
 if (e_blck_dup(strTemp, f))
 {
  strcpy(fd->search, strTemp);
  fd->sn = strlen(fd->search);
 }
 o->xa = 7;  o->ya = 3;  o->xe = 63;  o->ye = 18;
 o->bgsw = 0;
 o->crsw = AltO;
 o->name = "Find";
 e_add_txtstr(32, 4, "Direction:", o);
 e_add_txtstr(4, 4, "Options:", o);
 e_add_txtstr(4, 9, "Scope:", o);
 e_add_txtstr(32, 9, "Begin:", o);
 e_add_wrstr(4, 2, 18, 2, 35, 128, 0, AltT, "Text to Find:", fd->search, &f->ed->sdf, o);
 e_add_sswstr(5, 5, 0, AltC, fd->sw & 128 ? 1 : 0, "Case sensitive    ", o);
 e_add_sswstr(5, 6, 0, AltW, fd->sw & 64 ? 1 : 0, "Whole words only  ", o);
 e_add_sswstr(5, 7, 0, AltR, fd->sw & 32 ? 1 : 0, "Regular expression", o);
 e_add_pswstr(0, 33, 5, 6, AltD, 0, "ForwarD        ", o);
 e_add_pswstr(0, 33, 6, 0, AltB, fd->sw & 4 ? 1 : 0, "Backward       ", o);
 e_add_pswstr(1, 5, 10, 0, AltG, 0, "Global            ", o);
 e_add_pswstr(1, 5, 11, 0, AltS, fd->sw & 8 ? 1 : 0, "Selected Text     ", o);
 e_add_pswstr(2, 33, 10, 0, AltF, 0, "From Cursor    ", o);
 e_add_pswstr(2, 33, 11, 0, AltE, fd->sw & 2 ? 1 : 0, "Entire Scope   ", o);
 e_add_bttstr(16, 13, 1, AltO, " Ok ", NULL, o);
 e_add_bttstr(34, 13, -1, WPE_ESC, "Cancel", NULL, o);
 ret = e_opt_kst(o);
 if (ret != WPE_ESC)
 {
  fd->sw = (o->pstr[0]->num << 2) + (o->pstr[1]->num << 3) +
    (o->pstr[2]->num << 1) + (o->sstr[0]->num << 7) + (o->sstr[1]->num << 6) +
    (o->sstr[2]->num << 5);
  strcpy(fd->search, o->wstr[0]->txt);
  fd->sn = strlen(fd->search);
  if ((fd->sw & 2) != 0)
  {
   if ((fd->sw & 4) == 0)
   {
    b->b.x = (fd->sw & 8) == 0 ? 0 : s->mark_begin.x;
    b->b.y = (fd->sw & 8) == 0 ? 0 : s->mark_begin.y;
   }
   else
   {
    b->b.x = (fd->sw & 8) == 0 ? b->bf[b->mxlines-1].len : s->mark_end.x;
    b->b.y = (fd->sw & 8) == 0 ? b->mxlines-1 : s->mark_end.y;
   }
  }
 }
 freeostr(o);
 if (ret != WPE_ESC)
  e_rep_search(f);
 return(0);
}

/* The menu and menuhandling have been rewritten to support only 4 choices
  rather than 3 sets of 2 alternatives.
    1 Forward from Cursor
    2 Back from Cursor
    3 Global replace (forward from start of text)
    4 Selected Text (from start of selection, and only shown if a block _is_
      marked)
  The result window is either 'String NOT Found' or the number of
  occurrences found and the number actually replaced.
*/
int e_replace(FENSTER *f)
{
 SCHIRM *s;
 BUFFER *b;
 FIND *fd = &(f->ed->fd);
 int i, ret, c, ym, rep=0, found=0;
 char strTemp[80];
 W_OPTSTR *o = e_init_opt_kst(f);

 if (!o)
  return(-1);
 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--)
  ;
 if (i <= 0)
  return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  s = f->s;
 if (e_blck_dup(strTemp, f))
 {
  strcpy(fd->search, strTemp);
  fd->sn = strlen(fd->search);
 }
 o->xa = 7;  o->ya = 3;  o->xe = 67;  o->ye = 17;
 o->bgsw = 0;
 o->name = "Replace";
 o->crsw = AltO;
 e_add_txtstr(4, 6, "Options:", o);
 e_add_txtstr(32, 6, "Scope:", o);
 e_add_wrstr(4, 2, 18, 2, 35, 128, 0, AltT, "Text to Find:", fd->search, &f->ed->sdf, o);
 e_add_wrstr(4, 4, 18, 4, 35, 128, 0, AltN, "New Text:", fd->replace, &f->ed->rdf, o);
 e_add_sswstr(5, 7, 0, AltC, fd->sw & 128 ? 1 : 0, "Case sensitive    ", o);
 e_add_sswstr(5, 8, 0, AltW, fd->sw & 64 ? 1 : 0, "Whole words only  ", o);
 e_add_sswstr(5, 9, 0, AltR, fd->sw & 32 ? 1 : 0, "Regular expression", o);
 e_add_sswstr(5, 10, 0, AltP, 1, "Prompt on Replace ", o);
 e_add_pswstr(0, 33, 7, 0, AltF, 0, "Forward from Cursor", o);
 e_add_pswstr(0, 33, 8, 0, AltB, fd->sw & 4 ? 1 : 0, "Back from Cursor   ", o);
 e_add_pswstr(0, 33, 9, 0, AltG, fd->sw & 2 ? 1 : 0, "Global Replace     ", o);
 if (s->mark_end.y)
  e_add_pswstr(0, 33, 10, 0, AltS, fd->sw & 10 ? 1 : 0, "Selected Text      ", o);
 e_add_bttstr(10, 12, 1, AltO, " Ok ", NULL, o);
 e_add_bttstr(41, 12, -1, WPE_ESC, "Cancel", NULL, o);
 e_add_bttstr(22, 12, 7, AltA, "Change All", NULL, o);
 ret = e_opt_kst(o);
 if (ret != WPE_ESC)
 {
  strcpy(fd->search, o->wstr[0]->txt);
  fd->sn = strlen(fd->search);
  strcpy(fd->replace, o->wstr[1]->txt);
  fd->rn = strlen(fd->replace);
  fd->sw = 1 + (o->sstr[0]->num << 7)+ (o->sstr[1]->num << 6)
    + (o->sstr[2]->num << 5) + (o->sstr[3]->num << 4);
  switch (o->pstr[0]->num)
  {
   case 2:
    fd->sw |= 2;
    b->b.x = b->b.y = 0;
    break;
   case 1:
    fd->sw |= 4;
    b->b.x = b->bf[b->mxlines-1].len;
    b->b.y = b->mxlines-1;
    break;
   case 3:
    fd->sw |= 10;
    b->b.x = s->mark_begin.x;
    b->b.y = s->mark_begin.y;
   default:
    break;
  }
 }
 freeostr(o);
 if (ret != WPE_ESC)
 {
  while (e_rep_search(f) && ((ret == AltA) || (!found)))
  {
   found++;
   if (f->a.y < 11)
   {
    s->c.y = b->b.y - 1; ym = 15;
   }
   else
   {
    ym = 1;
   }
   e_schirm(f, 1);
   c = 'Y';
   if (fd->sw & 16)
    c = e_message(1, "String found:\nReplace this occurrence ?",f);
   if (c == WPE_ESC)
    break;
   if (c == 'Y')
   {
    rep++;
    e_add_undo('s', b, s->fa.x, b->b.y, fd->sn);
    e_undo_sw = 1;
    e_del_nchar(b, s, s->fa.x, b->b.y, fd->sn);
    e_ins_nchar(b, s, (unsigned char *)fd->replace, s->fa.x, b->b.y, fd->rn);
    e_undo_sw = 0;
    s->fe.x = s->fa.x + fd->rn;
    b->b.x = !(fd->sw & 4) ? s->fe.x : s->fa.x;
    e_schirm(f, 1);
   }
  }
  if (found)
  {
   sprintf(strTemp, "Found %d\nReplaced %d", found, rep);
   e_message(0, strTemp, f);
  }
  else
   e_message(0, e_msg[ERR_GETSTRING], f);
 }
 return(0);
}

