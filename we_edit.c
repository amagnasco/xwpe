/* we_edit.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"
#include "makro.h"

#include<sys/types.h> /*  included for digital station  */
#include<sys/stat.h>

int e_undo_sw = 0, e_redo_sw = 0;

char *e_make_postf();
int e_del_a_ind();
int e_tab_a_ind();
int e_help_next();

#ifdef PROG
BUFFER *e_p_m_buffer = NULL;
#ifdef DEBUGGER
BUFFER *e_p_w_buffer = NULL;
#endif
#endif

/* open edit window */
int e_edit(ECNT *cn, char *filename)
{
 extern char *e_hlp_str[];
 extern WOPT *eblst, *hblst, *mblst, *dblst;
 FILE *fp = NULL;
 FENSTER *f, *fo;
 char *complete_fname, *path, *file;
 int ftype = 0, i, j, st = 0;
 struct stat buf[1];

 /* Allows project files to be open automatically */
 j = strlen(filename);
 if ((WpeIsProg()) && (j > 4) && (!strcmp(&filename[j - 4], ".prj")))
 {
  if (!e_prog.project)
  {
   e_prog.project = MALLOC(1);
   e_prog.project[0] = '\0';
  }
  else
  {
   for (i = cn->mxedt; i > 0 &&
     (cn->f[i]->dtmd != DTMD_DATA || cn->f[i]->ins != 4); i--)
    ;
   if (i > 0)
   {
    e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
    e_close_window(cn->f[cn->mxedt]);
   }
  }
  e_prog.project = REALLOC(e_prog.project, j + 1);
  strcpy(e_prog.project, filename);
  e_make_prj_opt(cn->f[cn->mxedt]);
/************************************/  
  e_rel_brkwtch(cn->f[cn->mxedt]);
/************************************/  
  e_prj_ob_file(cn->f[cn->mxedt]);
  return 0;
 }

 /* Check to see if the file is already opened BD */
 WpeFilenameToPathFile(filename, &path, &file);
 /* Should check for error here */
 for (i = cn->mxedt; i >= 0; i--)
 {
  if ((strcmp(cn->f[i]->datnam, file) == 0) &&
    (strcmp(cn->f[i]->dirct, path) == 0))
  {
   e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
   WpeFree(path);
   WpeFree(file);
   return(0);
  }
 }

 if (cn->mxedt >= MAXEDT)
 {
  e_error(e_msg[ERR_MAXWINS], 0, cn->fb);
  return(-1);
 }
 if (stat(filename, buf) == 0)
 {
  if (!S_ISREG(buf->st_mode))
  {
   /* error message should go here */
   return(-1);
  }
 }
 for (j = 1; j <= MAXEDT; j++)
 {
  for (i = 1; i <= cn->mxedt && cn->edt[i] != j; i++);
  if( i > cn->mxedt) break;
 }
 cn->curedt=j;
 (cn->mxedt)++;
 cn->edt[cn->mxedt]=j;
   
 if ((f = (FENSTER *)MALLOC(sizeof(FENSTER))) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
   
 f->fb = cn->fb;
 cn->f[cn->mxedt] = f;

 if ((f->b = (BUFFER *)MALLOC(sizeof(BUFFER))) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, f->fb);
 if ((f->s = (SCHIRM*)MALLOC(sizeof(SCHIRM))) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, f->fb);
 if ((f->b->bf = (STRING *) MALLOC(MAXLINES*sizeof(STRING))) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, f->fb);
#ifdef PROG
 for (i = cn->mxedt-1;
   i > 0 && (!strcmp(cn->f[i]->datnam, "Messages") || !DTMD_ISTEXT(cn->f[i]->dtmd) ||
     !strcmp(cn->f[i]->datnam, "Watches") ||
     !strcmp(cn->f[i]->datnam, "Stack")); i--)
  ;
 for (j = cn->mxedt-1; j > 0 && !st; j--)
  if (!strcmp(cn->f[j]->datnam, "Stack"))
   st = 1;
#else
 for(i = cn->mxedt-1; i > 0 && !DTMD_ISTEXT(cn->f[i]->dtmd); i--)
  ;
#endif
#ifdef PROG
 if (WpeIsProg())
 {
  if ((e_we_sw & 8) || !strcmp(filename, "Messages") ||
    !strcmp(filename, "Watches"))
  {
   f->a = e_set_pnt(0, 2*MAXSLNS/3 + 1);
   f->e = e_set_pnt(MAXSCOL-1, MAXSLNS-2);
  }
  else if (!strcmp(filename, "Stack"))
  {
   f->a = e_set_pnt(2*MAXSCOL/3, 1);
   f->e = e_set_pnt(MAXSCOL-1, 2*MAXSLNS/3);
  }
  else
  {
   if (i < 1)
   {
    f->a = e_set_pnt(0, 1);
    f->e = e_set_pnt(st ? 2*MAXSCOL/3-1 : MAXSCOL-1, 2*MAXSLNS/3);
   }
   else
   {
    f->a = e_set_pnt(cn->f[i]->a.x+1, cn->f[i]->a.y+1);
    f->e = e_set_pnt(st ? 2*MAXSCOL/3-1 : cn->f[i]->e.x, cn->f[i]->e.y);
   }
  }
 }
 else
#endif
 {
  if (i < 1)
  {
   f->a = e_set_pnt(0, 1);
   f->e = e_set_pnt(MAXSCOL-1, MAXSLNS-2);
  }
  else
  {
   f->a = e_set_pnt(cn->f[i]->a.x + 1, cn->f[i]->a.y + 1);
   f->e = e_set_pnt(cn->f[i]->e.x, cn->f[i]->e.y);
  }
 }
 if (NUM_COLS_ON_SCREEN < 26)
  f->a.x = f->e.x - 26;
 if(NUM_LINES_ON_SCREEN < 3)
  f->a.y = f->e.y - 3;
 f->winnum = cn->curedt;
 f->dtmd = cn->dtmd;
 f->ins = 1;
 f->save = 0;
 f->zoom = 0;
 f->ed = cn;
 f->pic = NULL;
 f->hlp_str = e_hlp_str[0];
 f->blst = eblst;
 f->nblst = 7;
 f->b->f = f;
 f->b->b = e_set_pnt(0, 0);
 f->b->cl = f->b->clsv = 0;
 f->b->mx = e_set_pnt(cn->maxcol, MAXLINES);
 f->b->mxlines = 0;
 f->b->fb = f->fb;
 f->b->cn = cn;
 f->b->ud = NULL;
 f->b->rd = NULL;
 f->fd.dirct = NULL;
 if (WpeIsProg())
  e_add_synt_tl(filename, f);
 else
 {
  f->c_st = NULL;
  f->c_sw = NULL;
 }
 if ((f->ed->edopt & ED_ALWAYS_AUTO_INDENT) ||
   ((f->ed->edopt & ED_SOURCE_AUTO_INDENT) && f->c_st))
 {
  f->flg = 1;
 }
 else
 {
  f->flg = 0;
 }
 f->s->c = e_set_pnt(0, 0);
 f->s->ks = e_set_pnt(0, 0);
 f->s->mark_begin = e_set_pnt(0, 0);
 f->s->mark_end = e_set_pnt(0, 0);
 f->s->fa = e_set_pnt(0, 0);
 f->s->fe = e_set_pnt(0, 0);
 f->s->fb = f->fb;
#ifdef DEBUGGER
 f->s->brp = MALLOC(sizeof(int));
 f->s->brp[0] = 0;
 f->s->da.y = -1;
#endif
 f->dirct = path;
 for (i = 0; i < 9; i++)
  f->s->pt[i] = e_set_pnt(-1, -1);
 if (cn->mxedt == 0)    /*  Clipboard  */
 {
  cn->curedt=0;
  cn->edt[cn->mxedt]=0;
  WpeFree(file);
  file = f->datnam = WpeStrdup(BUFFER_NAME);
#ifdef UNIX
  f->filemode = 0600;
#endif
  e_new_line(0,f->b);
  *(f->b->bf[0].s) = WPE_WR;
  *(f->b->bf[0].s+1) = '\0';
  f->b->bf[0].len = 0;
  f->b->bf[0].nrc = 1;
  return(0);
 }
 if (strcmp(file,"") == 0)
 {
  WpeFree(file);
  file = f->datnam = WpeStrdup("Noname");
 }
 else
 {
  f->datnam = file;
 }
 if (strcmp(filename, "Help") == 0)
 {
  complete_fname = e_mkfilename(LIBRARY_DIR, HELP_FILE);
  f->dtmd = DTMD_HELP;
  f->ins = 8;
  f->hlp_str = e_hlp_str[25];
  f->nblst = 7;
  f->blst = hblst;
  ftype = 1;
 }
 else
  complete_fname = e_mkfilename(f->dirct, f->datnam);
#ifdef PROG
 if (WpeIsProg())
 {
  if (!strcmp(filename, "Messages"))
  {
   f->ins = 8;
   f->hlp_str = e_hlp_str[3];
   f->nblst = 8;
   f->blst = mblst;
   ftype = 2;
  }
  else if (!strcmp(filename, "Watches"))
  {
   f->ins = 8;
   f->hlp_str = e_hlp_str[1];
   f->blst = dblst;
   ftype = 3;
  }
  else if (!strcmp(filename, "Stack"))
  {
   f->ins = 8;
   f->hlp_str = e_hlp_str[2];
   f->blst = dblst;
   ftype = 4;
  }
 }
#endif
 if (ftype != 1)
  fp = fopen(complete_fname, "rb");
 if (fp != NULL && access(complete_fname, 2) != 0) f->ins = 8;
#ifdef UNIX
 if (fp != NULL)
 {
  stat(complete_fname, buf);
  f->filemode = buf->st_mode;
 }
 else
 {
  umask(i = umask(077));
  f->filemode = 0666 & ~i;
 }
#endif
 FREE(complete_fname);

 if (fp != NULL && ftype != 1)
 {
  e_readin(0, 0, fp, f->b, &f->dtmd);
  if (fclose(fp) != 0)
   e_error(e_msg[ERR_FCLOSE], 0, cn->fb);
  if (cn->dtmd == DTMD_HELP)
   cn->dtmd = DTMD_NORMAL;
#ifdef PROG
  if (WpeIsProg())
  {
   if (e_we_sw & 8)
   {
    strcpy(f->datnam, "Messages");
    e_we_sw &= ~8;
   }
   if (!strcmp(f->datnam, "Messages"))
   {
    e_make_error_list(f);
    f->ins = 8;
   }
  }
#endif
 }
 else
 {
  e_new_line(0,f->b);
  *(f->b->bf[0].s) = WPE_WR;
  *(f->b->bf[0].s+1) = '\0';
  f->b->bf[0].len = 0;
  f->b->bf[0].nrc = 1;
 }
#ifdef PROG
 if (ftype == 2)
 {
  if(e_p_m_buffer != NULL)
  {
   e_close_buffer(f->b);
   f->b = e_p_m_buffer;
   f->b->f = f;
  }
  else
  {
   e_p_m_buffer = f->b;
   FREE(f->b->bf[0].s);
   f->b->mxlines = 0;
  }
 }
#ifdef DEBUGGER
 if (ftype == 3)
 {
  if (e_p_w_buffer != NULL)
  {
   e_close_buffer(f->b);
   f->b = e_p_w_buffer;
   f->b->f = f;
  }
  else
  {
   e_p_w_buffer = f->b;
/*  e_ins_nchar(f->b, f->s, "No Watches", 0, 0, 10);*/
  }
 }
#endif
#endif
 sc_txt_1(f);
 if (cn->mxedt > 1)
 {
  fo = cn->f[cn->mxedt-1];
  e_ed_rahmen(fo, 0);
 }
 e_firstl(f, 1);
 e_zlsplt(f);
 e_brk_schirm(f);
 e_schirm(f, 1);
 e_cursor(f, 1);
 return(0);
}

/*   keyboard output routine */
int e_eingabe(ECNT *e)
{
 BUFFER *b = e->f[e->mxedt]->b;
 SCHIRM *s = e->f[e->mxedt]->s;
 FENSTER *f = e->f[e->mxedt];
 int ret, c = 0;
 unsigned char cc;

 fk_cursor(1);
 while (c != WPE_ESC)
 {
  if (e->mxedt < 1) c = WpeHandleMainmenu(-1, f);
  else if (!DTMD_ISTEXT(f->dtmd)) return(0);
  else
  {
   if (f->save > f->ed->maxchg) e_autosave(f);
#if  MOUSE
   if ((c = e_getch()) < 0) cc = c = e_edt_mouse(c, f);
   else cc = c;
#else
   cc = c = e_getch();
#endif
  }
  if ((c > 31 || (c == WPE_TAB && !(f->flg & 1)) ||
    (f->ins > 1 && f->ins != 8)) && c < 255)
  {
   if (f->ins == 8) continue;
   if (f->ins == 0 || f->ins == 2)  e_put_char(c, b, s);
   else e_ins_nchar(b, s, &cc, b->b.x, b->b.y, 1);
   e_schirm(f, 1);
  }
  else if (c == WPE_DC)
  {
   if (f->ins == 8)
   {
    if (f->dtmd == DTMD_HELP) e_help_last(f);
    continue;
   }
   if (b->b.y > 0 || b->b.x > 0)
   {
    if (b->bf[b->b.y].len == 0)
    {
     e_del_line(b->b.y, b, s);
     b->b.y--;
     b->b.x = b->bf[b->b.y].len;
     if (*(b->bf[b->b.y].s+b->b.x) == '\0') b->b.x--;
    }
    else
    {
     if (b->b.x > 0) b->b.x--;
     else
     {
      b->b.y--;
      b->b.x = b->bf[b->b.y].len;
      if (*(b->bf[b->b.y].s+b->b.x) == '\0') b->b.x--;
     }
     if (f->flg & 1) e_del_a_ind(b,  s);
     else e_del_nchar(b, s, b->b.x, b->b.y, 1);
    }
    e_schirm(f, 1);
   }
  }
  else if (c == ENTF || c == 4)
  {
   if (f->ins == 8)
   {
#ifdef DEBUGGER
    if (WpeIsProg()) e_d_is_watch(c, f);
#endif
    continue;
   }
   if (*(b->bf[b->b.y].s + b->b.x) != '\0' &&
     (b->b.y < b->mxlines-1 || *(b->bf[b->b.y].s + b->b.x) != WPE_WR))
   {
    e_del_nchar(b, s, b->b.x, b->b.y, 1);
    e_schirm(f, 1);
   }
  }
  else if (c == WPE_CR)
  {
#ifdef PROG
   if (f->ins == 8)
   {
    if (f->dtmd == DTMD_HELP) {  e_help_ret(f);  goto weiter;  }
    if (WpeIsProg())  {  e_d_car_ret(f);  goto weiter;  }
    else continue;
   }
#else
   if (f->ins == 8) continue;
#endif
   e_car_ret(b, s);
   e_schirm(f, 1);
  }
  else if (c == WPE_TAB)
  {
   e_tab_a_ind(b, s);
   e_schirm(f, 1);
  }
/*
          else if(c == WPE_TAB)
          {    if(f->ins == 8) continue;
               if (f->ins == 0 || f->ins == 2)  
                 for(c = f->ed->tabn - b->b.x % f->ed->tabn; c > 0 
                                              && b->b.x < b->mx.x; c--) 
              		{   e_put_char(' ', b, s);  b->b.x++;  }
               else  e_ins_nchar(b, s, f->ed->tabs, b->b.x, b->b.y, 
                                       f->ed->tabn - b->b.x % f->ed->tabn);
               e_schirm(b, s, f, 1);
          }
*/
  else
  {
   ret = e_tst_cur(c, e); /*up/down arrows go this way*/
   if (ret != 0) ret = e_tst_fkt(c, e);
  }
weiter:
  f = e->f[e->mxedt];
  if (e->mxedt > 0 && DTMD_ISTEXT(f->dtmd))
  {
   b = f->b;
   s = f->s;
   s->ks.x = b->b.x;  s->ks.y = b->b.y;
   e_cursor(f, 1);
   if ((c & 511) != CUP && (c & 511) != CDO) b->clsv = b->cl;
   e_zlsplt(f);
  }
 }
 return(c);
}

/*              Interpretation of cursor keys
                     ( Primitive editor )                    */
int e_tst_cur(int c, ECNT *e)
{
   FENSTER *f = e->f[e->mxedt];
   BUFFER *b = f->b;
   SCHIRM *s = f->s;
   
   switch (c)
   {  case CtrlP:
      case CUP:
      case CUP+512:
	 if(b->b.y > 0) (b->b.y)--;
	 b->b.x = e_chr_sp(b->clsv, b, f);
	 break;
      case CtrlN:
      case CDO:
      case CDO+512:
	 if(b->b.y < b->mxlines - 1) (b->b.y)++;
	 b->b.x = e_chr_sp(b->clsv, b, f);
	 break;
      case CtrlB:
      case CLE:
      case CLE+512:
	 (b->b.x)--;
	 if(b->b.x < 0)
	 {  if(b->b.y > 0)
	    {  (b->b.y)--; b->b.x = b->bf[b->b.y].len;  }
	    else {  b->b.x = 0;  }
	 }
	 break;
      case CtrlF:
      case CRI:
      case CRI+512:
	 (b->b.x)++;
	 if(b->b.x > b->bf[b->b.y].len)
	 {  if(b->b.y < b->mxlines - 1)  {  (b->b.y)++; b->b.x = 0;  }
	    else {  b->b.x = b->bf[b->b.y].len;  }
	 }
	 break;
	 
      case CCLE:
      case CCLE+512:
	 if(b->b.x <= 0 && b->b.y > 0)
	 {  b->b.y--;
	    b->b.x = b->bf[b->b.y].len;
	 }
	 else if( b->b.x > 0 )
	 b->b.x = e_su_rblk(b->b.x - 1, b->bf[b->b.y].s);
	 break;
      case CCRI:
      case CCRI+512:
	 if(b->b.x >= b->bf[b->b.y].len && b->b.y < b->mxlines)
	 {  b->b.x = 0; b->b.y++;  }
	 else if( b->b.x < b->bf[b->b.y].len )
	 b->b.x = e_su_lblk(b->b.x, b->bf[b->b.y].s);
	 break;
      case BDO:
      case BDO+512:
	 b->b.y = b->b.y + NUM_LINES_ON_SCREEN - 2;
	 if(b->b.y > b->mxlines - 1) b->b.y = b->mxlines - 1;
	 e_schirm(f, 1);
	 e_cursor(f, 1);
	 break;
      case BUP:
      case BUP+512:
	 b->b.y = b->b.y - f->e.y + f->a.y + 2;
	 if(b->b.y < 0) b->b.y = 0;
	 e_schirm(f, 1);
	 e_cursor(f, 1);
	 break;
      case CBDO:
      case CBDO+512:
	 b->b.y = b->mxlines - 1;
	 b->b.x = b->bf[b->mxlines - 1].len;
	 e_schirm(f, 1);
	 break;
      case CBUP:
      case CBUP+512:
	 if(b->b.y != 0)
	 {  b->b.x = 0;
	    b->b.y = 0;
	    e_schirm(f, 1);  }
	 break;
      case CEND:
      case CEND+512:
	 if(LINE_NUM_ON_SCREEN_BOTTOM < b->mxlines)
	 b->b.y = LINE_NUM_ON_SCREEN_BOTTOM - 1;
	 else
	 b->b.y = b->mxlines - 1;
	 b->b.x = b->bf[b->b.y].len;
	 break;
      case CPS1:
      case CPS1+512:
	 b->b.x = 0;
	 b->b.y = s->c.y;
	 break;
      case AltI:
      case EINFG:
	 if (f->ins == 8) {
#ifdef DEBUGGER
	    if (WpeIsProg()) e_d_is_watch(c, f);
#endif
	    break;  }
	 if(f->ins & 1) f->ins &= ~1;
	 else f->ins |= 1;
#ifdef NEWSTYLE
	 e_ed_rahmen(f, 1);
#else
	 e_pr_filetype(f);
#endif
	 break;
      case AltJ:
	 if (f->ins == 8) break;
	 if(f->ins & 2) f->ins &= ~2;
	 else f->ins |= 2;
#ifdef NEWSTYLE
	 e_ed_rahmen(f, 1);
#else
	 e_pr_filetype(f);
#endif
	 break;
      case CtrlA:
      case POS1:
      case POS1+512:
	 b->b.x = 0;
	 break;
      case CtrlE:
      case ENDE:
      case ENDE+512:
	 b->b.x = b->bf[b->b.y].len;
	 break;
      case CtrlT:
	 if(f->ins == 8) break;
	 if( b->b.x < b->bf[b->b.y].len )
	 {  c = e_su_lblk(b->b.x, b->bf[b->b.y].s);
	    e_del_nchar(b, s, b->b.x, b->b.y, c-b->b.x);
	 }
	 else if(*(b->bf[b->b.y].s+b->b.x) == WPE_WR)
	 e_del_nchar(b, s, b->b.x, b->b.y, 1);
	 else if(b->b.x >= b->bf[b->b.y].len && b->b.y < b->mxlines)
	 {  b->b.x = 0; (b->b.y)++;  }
	 e_schirm(f, 1);
	 break;
      case CtrlZ:
	 if(f->ins == 8) break;
	 e_del_nchar(b, s, b->b.x, b->b.y, b->bf[b->b.y].len-b->b.x);
	 e_schirm(f, 1);
	 break;
      case DGZ:
	 if(f->ins == 8) break;
	 e_del_line(b->b.y, b, s);
	 if(b->b.y > b->mxlines - 1) (b->b.y)--;
	 e_schirm(f, 1);
	 break;
      case AF7:
      case AltV:
	 if(f->dtmd != DTMD_HELP || f->ins != 8) return(c);
         e_help_next(f, 0);
	 break;
      case AF8:
      case AltT:
	 if(f->dtmd != DTMD_HELP || f->ins != 8) return(c);
         e_help_next(f, 1);
	 break;
      default:
	 return(c);
   }
   if(c >= 512 )
   {  if(s->ks.y == s->mark_begin.y && s->ks.x == s->mark_begin.x &&
		(s->mark_begin.y != s->mark_end.y || s->mark_begin.x != s->mark_end.x))
      {  s->mark_begin.x = b->b.x;  s->mark_begin.y = b->b.y;  }
      else if(s->ks.y == s->mark_end.y && s->ks.x == s->mark_end.x &&
		(s->mark_begin.y != s->mark_end.y || s->mark_begin.x != s->mark_end.x))
      {  s->mark_end.x = b->b.x;  s->mark_end.y = b->b.y;  }
      else if(s->ks.y < b->b.y || ( s->ks.y == b->b.y && s->ks.x < b->b.x))
      {  s->mark_begin.x = s->ks.x;  s->mark_begin.y = s->ks.y;
	 s->mark_end.x = b->b.x;  s->mark_end.y = b->b.y;
      }
      else
      {  s->mark_end.x = s->ks.x;  s->mark_end.y = s->ks.y;
	 s->mark_begin.x = b->b.x;  s->mark_begin.y = b->b.y;
      }
      e_schirm(f, 1);
   }
   return(0);
}

/*   function key (F1-F12) evaluation, editor only */
int e_tst_fkt(int c, ECNT *e)
{
 extern OPT opt[];
 int i;
 FENSTER *f = e->f[e->mxedt];

#ifdef PROG
 if (e_tst_dfkt(f, c) == 0 || ((WpeIsProg()) && e_prog_switch(f, c) == 0))
#else
 if (e_tst_dfkt(f, c) == 0)
#endif
 {
  f = e->f[e->mxedt];
  fk_cursor(1);
  if (e->mxedt > 0)
   e_cursor(f, 1);
  return(0);
 }

 for (i = 0; i < MENOPT; i++)
  if (c == opt[i].as)
   WpeHandleMainmenu(i, f);

 switch (c)
 {
  case CtrlK:
   e_ctrl_k(f);    /*  ctrl k  */
   break;
  case CtrlO:      /*  ctrl o  */
   e_ctrl_o(f);
   break;
  case AltG:
   e_goto_line(f);
   break;
  case CF10:
  case CtrlW:
   e_show_clipboard(f);
   break;
  case CtrlDel:
   e_blck_del(f);
   break;
  case UNDO:
  case AltBS:
  case CtrlU:
   e_make_undo(f);
   break;
  case AGAIN:
  case SABS:
  case CtrlR:
   e_make_redo(f);
   break;
  case CUT:
  case ShiftDel:    /*  shift Del  :  Delete to Clipboard  */
/*               case 402:     */
  case CtrlX:
   e_edt_del(f);
   break;
  case PASTE:
  case ShiftEin:    /*  shift Einf  */
  case CtrlV:
   e_edt_einf(f);
   break;
#if !defined(NO_XWINDOWS)
  case AltEin:
   e_copy_X_buffer(f);
   break;
  case AltDel:
   e_paste_X_buffer(f);
   break;
#endif
  case COPY:
  case CtrlEin:    /*  ctrl Einf  */
/*               case 401:    */
  case CtrlC:
   e_edt_copy(f);
   break;
  default:
   if (f->ed->edopt & ED_CUA_STYLE)
   {
    switch (c)
    {
     case AF2:
      e_m_save(f);
      break;
     case FID:
     case AF3:
      e_find(f);
      break;
     case SF3:
      e_replace(f);
      break;
     case F3:
      e_rep_search(f);
      break;
     default:
      return(c);
    }
   }
   else
   {
    switch(c)
    {
     case F2:
      e_m_save(f);
      break;
     case FID:
     case F4:
      e_find(f);
      break;
     case AF4:
      e_replace(f);
      break;
     case CtrlL:
     case CF4:
      e_rep_search(f);
      break;
     default:
      return(c);
    }
   }
   break;
 }
 fk_cursor(1);
 return(0);
}

int e_ctrl_k(FENSTER *f)
{
 BUFFER *b = f->ed->f[f->ed->mxedt]->b;
 SCHIRM *s = f->ed->f[f->ed->mxedt]->s;
 int c;

 c = e_toupper(e_getch());
 if (c < 32)
  c = c + 'A' - 1;
 switch (c)
 {
  case 'A':
   b->b = s->mark_begin;
   e_schirm(f, 1);
   break;
  case 'B':
   s->mark_begin = e_set_pnt(b->b.x, b->b.y);
   e_schirm(f, 1);
   break;
  case 'C':
   e_blck_copy(f);
   break;
  case 'D':
   e_changecase_dialog(f);
   break;
  case 'F':
   e_mk_beauty(1, 3, f);
   break;
  case 'H':
   e_blck_hide(f);
   break;
  case 'I':
   e_blck_to_right(f);
   break;
  case 'K':
   s->mark_end = e_set_pnt(b->b.x, b->b.y);
   e_schirm(f, 1);
   break;
  case 'L':
   f->s->mark_begin.x = 0;
   f->s->mark_begin.y = f->b->b.y;
   if (f->b->b.y < f->b->mxlines-1)
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
   break;
  case 'R':
   e_blck_read(f);
   break;
  case 'U':
   e_blck_to_left(f);
   break;
  case 'V':
   e_blck_move(f);
   break;
  case 'W':
   e_blck_write(f);
   break;
  case 'X':
   s->mark_begin.x = 0;
   s->mark_begin.y = 0;
   s->mark_end.y = b->mxlines-1;
   s->mark_end.x = b->bf[b->mxlines-1].len;
   e_schirm(f, 1);
   break;
  case 'Y':
   e_blck_del(f);
   break;
  case 'Z':
   b->b = s->mark_end;
   e_schirm(f, 1);
   break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
   s->pt[c-'0'] = e_set_pnt(b->b.x, b->b.y);
   s->fa.y = b->b.y;  s->fa.x = b->b.x;  s->fe.x = b->b.x+1;
   e_schirm(f, 1);
   break;
 }
 return 0;
}

/*   Ctrl - O - Dispatcher     */
int e_ctrl_o(FENSTER *f)
{
 BUFFER *b = f->ed->f[f->ed->mxedt]->b;
 SCHIRM *s = f->ed->f[f->ed->mxedt]->s;
 int i, c;
 unsigned char cc;

 c = e_toupper(e_getch());
 if (c < 32) c = c + 'A' - 1;
 switch(c)
 {
  case 'Y':     /*  delete end of line    */
   if (f->ins == 8) break;
   e_del_nchar(b, s, b->b.x, b->b.y, b->bf[b->b.y].len-b->b.x);
   e_schirm(f, 1);
   e_cursor(f, 1);
   break;
  case 'T':     /*  delete up to beginning of next word    */
   if (f->ins == 8) break;
   if (b->b.x <= 0 && b->b.y > 0)
   {
    b->b.y--;
    b->b.x = b->bf[b->b.y].len;
   }
   else if (b->b.x > 0)
   {
    c = b->b.x;
    b->b.x = e_su_rblk(b->b.x - 1, b->bf[b->b.y].s);
    e_del_nchar(b, s, b->b.x, b->b.y, c-b->b.x);
   }
   e_schirm(f, 1);
   break;
  case 'F':     /*  find string    */
   e_find(f);
   break;
  case 'A':     /*  replace string    */
   e_replace(f);
   break;
#ifdef PROG
  case 'S':     /*  find declaration    */
   e_sh_def(f);
   break;
  case 'N':     /*  ...next...  */
   e_sh_nxt_def(f);
   break;
  case 'K':     /*  next bracket  */
   e_nxt_brk(f);
   break;
  case 'B':     /*  beautify text  */
   e_p_beautify(f);
   break;
#endif
  case 'U':     /*   for help file: create button */
   if (s->mark_begin.y == s->mark_end.y && s->mark_begin.y >= 0 && s->mark_begin.x < s->mark_end.x)
   {
    cc = HED;
    e_ins_nchar(b, s, &cc, s->mark_end.x, s->mark_end.y, 1);
    cc = HBG;
    e_ins_nchar(b, s, &cc, s->mark_begin.x, s->mark_end.y, 1);
    e_schirm(f, 1);
   }
   break;
  case 'M':     /*   for help file: Mark-Line  */
   if (s->mark_begin.y == s->mark_end.y && s->mark_begin.y >= 0 && s->mark_begin.x < s->mark_end.x)
   {
    cc = HED;
    e_ins_nchar(b, s, &cc, s->mark_end.x, s->mark_end.y, 1);
    cc = HBB;
    e_ins_nchar(b, s, &cc, s->mark_begin.x, s->mark_end.y, 1);
    e_schirm(f, 1);
   }
   break;
  case 'H':     /*   for help file: create Header  */
   if (s->mark_begin.y == s->mark_end.y && s->mark_begin.y >= 0 && s->mark_begin.x < s->mark_end.x)
   {
    cc = HED;
    e_ins_nchar(b, s, &cc, s->mark_end.x, s->mark_end.y, 1);
    cc = HHD;
    e_ins_nchar(b, s, &cc, s->mark_begin.x, s->mark_end.y, 1);
    e_schirm(f, 1);
   }
   break;
  case 'E':     /*   for help file: create end mark  */
   e_new_line(b->b.y, b);
   cc = HFE;
   e_ins_nchar(b, s, &cc, 0, b->b.y, 1);
   e_schirm(f, 1);
   break;
  case 'L':     /*   for help file: delete "help" special chars  */
   for (i = 0; i < b->bf[b->b.y].len; i++)
    if (b->bf[b->b.y].s[i] == HBG || b->bf[b->b.y].s[i] == HED ||
      b->bf[b->b.y].s[i] == HHD || b->bf[b->b.y].s[i] == HBB ||
      b->bf[b->b.y].s[i] == HFE || b->bf[b->b.y].s[i] == HFB ||
      b->bf[b->b.y].s[i] == HFE)
    {
     e_del_nchar(b, s, i, b->b.y, 1);
     i--;
    }
   e_schirm(f, 1);
   break;
  case 'C':     /*   for help file: check help file   */
   e_help_comp(f);
   break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
   b->b.x = s->pt[c-'0'].x;  b->b.y = s->pt[c-'0'].y;
   s->fa.y = b->b.y;  s->fa.x = b->b.x;  s->fe.x = b->b.x+1;
   e_cursor(f, 1);
   break;
 }
 return(0);
}

/* general function key dispatcher 
   basically, every time when it returns with zero
   something has happened */
int e_tst_dfkt(FENSTER *f, int c)
{
 if (c >= Alt1 && c <= Alt9)
 {
  e_switch_window(c - Alt1 + 1, f);
  return(0);
 }
 if (c >= 1024 && c <= 1049)
 {
  e_switch_window(c - 1014, f);
  return(0);
 }
 switch (c)
 {
  case F1:
  case HELP:
   e_help_loc(f, 0);
   break;
#if defined(PROG)
  case CF1:
   if (WpeIsProg())
    e_topic_search(f);
   break;
  case AF1:
   if (WpeIsProg())
    e_funct_in(f);
   break;
#endif
  case WPE_ESC:
  case F10:
  case AltBl:
   WpeHandleMainmenu(-1, f);
   break;
  case Alt0:
   e_list_all_win(f);
   break;
  case AF5:
   e_deb_out(f);
   break;
  default:
   if (f->ed->edopt & ED_CUA_STYLE)
   {
    switch (c)
    {
     case CtrlL:
      e_size_move(f);
      break;
     case OPEN:
     case F2:
      WpeManager(f);
      break;
     case SF2:
      WpeManagerFirst(f);
      break;
     case CF4:
     case AltX:
      e_close_window(f);
      break;
     case FRONT:
     case AltZ:
     case SF4:
      e_ed_cascade(f);
      break;
     case SF5:
      e_ed_tile(f);
      break;
     case SF6:
      e_ed_zoom(f);
      break;
     case CF6:
     case AltN:
      e_ed_next(f);
      break;
     case STOP:
     case AF4:
      e_quit(f);
      break;
     default:
      return(c);
    }
   }
   else
   {
    switch(c)
    {
     case CF5:
     case AF2:
      e_size_move(f);
      break;
     case OPEN:
     case F3:
      WpeManager(f);
      break;
     case CF3:
      WpeManagerFirst(f);
      break;
     case AF3:
      e_close_window(f);
      break;
     case FRONT:
     case AltZ:
     case F5:
      e_ed_zoom(f);
      break;
     case F6:
     case AltN:
      e_ed_next(f);
      break;
     case STOP:
     case AltX:
      e_quit(f);
      break;
     default:
      return(c);
    }
   } 
 }
 return(0);
}

int e_chr_sp(int x, BUFFER *b, FENSTER *f)
{
 int i, j;

 for (i = j = 0; i + j < x && i < b->bf[b->b.y].len; i++)
 {
  if (*(b->bf[b->b.y].s + i) == WPE_TAB)
   j += (f->ed->tabn - ((j + i) % f->ed->tabn) - 1);
#ifdef UNIX
  else if (!WpeIsXwin() && ((unsigned char) *(b->bf[b->b.y].s + i)) > 126)
  {
   j++;
   if (((unsigned char) *(b->bf[b->b.y].s + i)) < 128 + ' ')
    j++;
  }
  else if (*(b->bf[b->b.y].s + i) < ' ')
   j++;
  if (f->dtmd == DTMD_HELP)
  {
   if (b->bf[b->b.y].s[i] == HBG || b->bf[b->b.y].s[i] == HFB ||
     b->bf[b->b.y].s[i] == HED || b->bf[b->b.y].s[i] == HHD ||
     b->bf[b->b.y].s[i] == HFE || b->bf[b->b.y].s[i] == HBB ||
     b->bf[b->b.y].s[i] == HNF)
    j -= 2;
  }
#endif
 }
 return(i);
}

/****************************************/
int FindFirstNospaceChar(BUFFER *b, int line)
/* returns char number in the line, -1 otherwise */
{
  int k;
  /* advance x while char[k] == space() */
  for(k = 0; k < b->bf[line].len; k++) {
    if(!isspace(b->bf[line].s[k])) {
      return k;
    }
  }
  return -1;
}

/****************************************/
int GetXOfCharNum(BUFFER *b, int line, int char_num)
{
   int x, k;
   
   for(x = 0, k = 0; k < b->bf[line].len; k++) {
     if(k == char_num) {
       return x;
     }
     x = ((b->bf[line].s[k] == '\t') ?
          (x / b->cn->tabn + 1) * b->cn->tabn : x + 1);
   }
   /* return 0 to secure result interpretation */
   return 0;
}

/****************************************/
int GetCharNumOfX(BUFFER *b, int line, int char_x)
{
   int x, k, next_x;
   
   for(x = 0, k = 0; k < b->bf[line].len; k++) {
     next_x = ((b->bf[line].s[k] == '\t') ?
          (x / b->cn->tabn + 1) * b->cn->tabn : x + 1);
     if(next_x > char_x) {
       return x;
     }
     x = next_x;
   }
   /* return 0 to secure result interpretation */
   return 0;
}
/****************************************/

/* New version of auto-indent */
int e_tab_a_ind(BUFFER *b, SCHIRM *s)
{
 int a_indent = b->cn->autoindent;
 int line, x, k, char_to_ins;
 char *str;
 int do_auto_indent = 0;
 int first_nospace_k;

 /* If at the right of current pos there are only space chars, tab up to
    first nospace position (auto-indent pos) of previous lines. 
    Tail spaces of current line are erased.
    If left current pos there are only spaces and even if there
    are nospaces at the right, tab up to auto-indent pos.
    Otherwise, insert TAB char.
 */

 line = b->b.y; /* current line */

 first_nospace_k = FindFirstNospaceChar(b, line);
 if (first_nospace_k > -1)
 {
  /* there is a nospace char in the line */
  /* get its x */
  if(b->b.x > GetXOfCharNum(b, line, first_nospace_k))
  {
   /* nospace char before curr pos */

   /* are there nospace chars under and after cursor pos */
   do_auto_indent = 1;
   for (k = GetCharNumOfX(b, line, b->b.x); k < b->bf[line].len; k++)
   {
    if (!isspace(b->bf[line].s[k]))
    {
     /* yes, there are */
     do_auto_indent = 0;
     break;
    }
   }
   if (do_auto_indent)
   {
    /* erase all tail spaces */
    e_del_nchar(b, s, b->b.x, line, b->bf[line].len - b->b.x);
   }
  }
  else
  {
   /* nospace char only at or after curr pos */
   do_auto_indent = 1;
  }
 }
 else
 {
  /* there are only space chars in the line */
  do_auto_indent = 1;
  if (b->bf[line].len > 0)
  {
   /* erase all tail spaces */
   e_del_nchar(b, s, b->b.x, line, b->bf[line].len - b->b.x);
  }
 }

 if (!do_auto_indent)
 {
  /* insert TAB char */
  str = MALLOC(sizeof(char));
  str[0] = '\t';
  char_to_ins = 1;
 }
 else
 {
  /* auto-indent */ 

  /* find x of the first nospace char of lines from above */
  x = 0;
  for (line = b->b.y - 1; (line > 0); line--)
  {
   k = FindFirstNospaceChar(b, line);
   if (k > -1)
   {
    x = GetXOfCharNum(b, line, k);
    break;
   }
  }

  if (b->b.x < x)
  {
   /* indent to x with spaces */
   /* insert chars */
   k = x - b->b.x;
   str = MALLOC(k * sizeof(char));
   for (x = 0; x < k; x++)
    str[x] = ' ';
   char_to_ins = k;
  }
  else if (b->b.x < (x + a_indent))
  {
   /* indent to x + a_indent with spaces */
   /* insert chars */
   k = x + a_indent - b->b.x;
   str = MALLOC(k * sizeof(char));
   for (x = 0; x < k; x++)
    str[x] = ' ';
   char_to_ins = k;
  }
  else
  {
   /* insert TAB char */
   str = MALLOC(sizeof(char));
   str[0] = '\t';
   char_to_ins = 1;
  }
 }

 e_ins_nchar(b, s, str, b->b.x, b->b.y, char_to_ins);
 FREE(str);
 return(b->b.x);
}

int e_del_a_ind(BUFFER *b, SCHIRM *s)
{
 int i = 1, j = -1, k;

 if (b->b.y > 0)
 {
  for (i = 0;
    i < b->bf[b->b.y].len && i < b->b.x && isspace(b->bf[b->b.y].s[i]); i++)
   ;
  if (i == b->b.x)
  {
   for (j = 0, i = 0; j <= b->b.x; j++)
   {
    i = b->bf[b->b.y].s[j] == '\t' ? (i / b->cn->tabn + 1) * b->cn->tabn :
      i + 1;
   }
   if (i != j)
   {
    char *str = MALLOC(i * sizeof(char));
    e_del_nchar(b, s, 0, b->b.y, j);
    for (j = 0; j < i; j++)
     str[j] = ' ';
    e_ins_nchar(b, s, str, 0, b->b.y, i);
    b->b.x = i - 1;
    FREE(str);
   }
   for (j = b->b.y-1; j >= 0; j--)
   {
    for (i = 0, k = 0; k < b->bf[j].len && isspace(b->bf[j].s[k]); k++)
     i = b->bf[j].s[k] == '\t' ? (i / b->cn->tabn + 1) * b->cn->tabn : i + 1;
    if (k < b->bf[j].len && b->bf[j].s[k] != '#' && i <= b->b.x)
    {
     i = b->b.x - i + 1;
     b->b.x -= i - 1;
     break;
    }
   }
   if (j < 0)
   {
    i = b->b.x;
    b->b.x = 0;
   }
  }
 }
 if (j < 0)
  i = 1;
 e_del_nchar(b, s, b->b.x, b->b.y, i);
 return(i);
}

int e_car_a_ind(BUFFER *b, SCHIRM *s)
{
 int i, j, k;
 unsigned char *str;

 if (b->b.y == 0)
  return(0);
 j = b->b.y;
 do
 {
  j--;
  for (i = 0, k = 0;
    k < b->bf[j].len && (isspace(b->bf[j].s[k]) || b->bf[j].s[i] == '{'); k++)
   i = b->bf[j].s[k] == '\t' ? (i / b->cn->tabn + 1) * b->cn->tabn : i + 1;
 } while (j > 0 && b->bf[j].s[k] == '#');
 if (k == b->bf[j].len && k > 0 && b->bf[j].s[k-1] == '{')
  i--;
 if (i > 0)
 {
  str = MALLOC(i * sizeof(char));
  for (j = 0; j < i; j++)
   str[j] = ' ';
  e_ins_nchar(b, s, str, 0, b->b.y, i);
  b->b.x = i;
  FREE(str);
 }
 return(i);
}

/*   write n blanks */
int e_blk(int anz, int xa, int ya, int col)
{
 for (anz--; anz >= 0 ; anz--)
  e_pr_char(xa+anz, ya, ' ', col);
 return(anz);
}

/*       insert Carriage Return     */
int e_car_ret(BUFFER *b, SCHIRM *s)
{
   int len, i;
   len = b->bf[b->b.y].len;
   e_add_undo('a', b, b->b.x, b->b.y, 1);
   (b->f->save)++;
   if(b->b.x != len || *(b->bf[b->b.y].s+len) != '\0')
   {  e_new_line(b->b.y+1, b);
      for(i=0; i <= len - b->b.x; i++)
              *(b->bf[b->b.y+1].s + i) = *(b->bf[b->b.y].s+b->b.x+i);
      *(b->bf[b->b.y+1].s+i)='\0';
      b->bf[b->b.y+1].len = e_str_len(b->bf[b->b.y+1].s);
      b->bf[b->b.y+1].nrc = e_str_nrc(b->bf[b->b.y+1].s);
      if(s->mark_begin.y > b->b.y) (s->mark_begin.y)++;
      else if(s->mark_begin.y == b->b.y && s->mark_begin.x > b->b.x)
      {  (s->mark_begin.y)++;  (s->mark_begin.x) -= (b->b.x);  }
      if(s->mark_end.y > b->b.y) (s->mark_end.y)++;
      else if(s->mark_end.y == b->b.y && s->mark_end.x > b->b.x)
      {  (s->mark_end.y)++;  (s->mark_end.x) -= (b->b.x);  }
   }
   *(b->bf[b->b.y].s+b->b.x) = WPE_WR;
   *(b->bf[b->b.y].s+b->b.x+1) = '\0';
   b->bf[b->b.y].len = e_str_len(b->bf[b->b.y].s);
   b->bf[b->b.y].nrc = e_str_nrc(b->bf[b->b.y].s);
   sc_txt_3(b->b.y, b, 1);
/***************************/   
   if(b->b.x>0) e_brk_recalc(b->f,b->b.y+1,1);
   else e_brk_recalc(b->f,b->b.y,1);
/***************************/   
   if (b->b.y < b->mxlines - 1)
   {  (b->b.y)++;
      b->b.x = 0;
   }
   if(b->f->flg & 1) e_car_a_ind(b, s);
   return(b->b.y);
}

/*   cursor placement */
void e_cursor(FENSTER *f, int sw)
{
 BUFFER *b = f->b;
 SCHIRM *s = f->s;
 static int iold = 0, jold = 0;
 int i, j;

 if (!DTMD_ISTEXT(f->dtmd)) return;
 if (b->b.y > b->mxlines-1) b->b.y = b->mxlines-1;
 if (b->b.y < 0) b->b.y = 0;
 if (b->b.x < 0) b->b.x = 0;
 if (b->mxlines==0) b->b.x=0; /* the else branch needs b->b.y < b->mxlines, which is not true for b->mxlines==0 */
 else if (b->b.x > b->bf[b->b.y].len) b->b.x = b->bf[b->b.y].len;
 for (i = j = 0; i < b->b.x; i++)
 {
  if (*(b->bf[b->b.y].s + i) == WPE_TAB)
   j += (f->ed->tabn - ((j + i) % f->ed->tabn) - 1);
  else if (!WpeIsXwin() && ((unsigned char) *(b->bf[b->b.y].s + i)) > 126)
  {
   j++;
   if (((unsigned char) *(b->bf[b->b.y].s + i)) < 128 + ' ') j++;
  }
  else if (*(b->bf[b->b.y].s + i) < ' ') j++;
  if (f->dtmd == DTMD_HELP)
  {
   if (b->bf[b->b.y].s[i] == HBG || b->bf[b->b.y].s[i] == HED ||
     b->bf[b->b.y].s[i] == HHD || b->bf[b->b.y].s[i] == HFE ||
     b->bf[b->b.y].s[i] == HFB || b->bf[b->b.y].s[i] == HBB ||
     b->bf[b->b.y].s[i] == HNF)
    j -= 2;
  }
 }
 if (b->b.y - s->c.y < 0 || b->b.y - s->c.y >= NUM_LINES_ON_SCREEN - 1 ||
   s->c.y < 0 || s->c.y >= b->mxlines ||
   b->b.x + j - s->c.x < 0 ||
   b->b.x + j- s->c.x >= NUM_COLS_ON_SCREEN - 1 )
 {
#if defined(UNIX)
      /*if(b->b.y - s->c.y < 0) s->c.y = b->b.y - (f->e.y - f->a.y)/2;
      else if(b->b.y - s->c.y >= f->e.y - f->a.y -1)
      s->c.y = b->b.y - (f->e.y - f->a.y)/2;*/
  if (b->b.y - s->c.y < -1)
  {
   s->c.y = b->b.y - (NUM_LINES_ON_SCREEN)/2;
  }
  else if (b->b.y - s->c.y == -1)
  {
   s->c.y -= 1;
  }
  else if (b->b.y - s->c.y > NUM_LINES_ON_SCREEN -1)
  {
   s->c.y = b->b.y - (NUM_LINES_ON_SCREEN)/2;
  }
  else if (b->b.y - s->c.y == NUM_LINES_ON_SCREEN -1)
  {
   s->c.y += 1;
  }
#else
  if (b->b.y - s->c.y < 0) s->c.y = b->b.y;
  else if (b->b.y - s->c.y >= NUM_LINES_ON_SCREEN -1)
   (s->c.y) = b->b.y - f->e.y + f->a.y + 2;
#endif
  if (s->c.y >= b->mxlines - 1) s->c.y = b->mxlines - 2 ;
  if (s->c.y < 0) s->c.y = 0 ;

  if (b->b.x + j - s->c.x < 0)
   (s->c.x) = b->b.x + j - (NUM_COLS_ON_SCREEN)/2 ;
  else if (b->b.x + j - s->c.x >= NUM_COLS_ON_SCREEN - 1)
   (s->c.x) = b->b.x + j - (NUM_COLS_ON_SCREEN)/2 ;
  if (s->c.x < 0) s->c.x = 0 ;
  else if (s->c.x >= b->bf[b->b.y].len + j) s->c.x = b->bf[b->b.y].len + j;
  e_schirm(f, sw);
 }
 if (s->fa.y == -1) {  e_schirm(f, sw);  s->fa.y--;  }
 else if (s->fa.y > -1) s->fa.y = -1;
 if (sw != 0)
 {
  iold = e_lst_zeichen(f->e.x, f->a.y+1, f->e.y-f->a.y-1, 0,
    f->fb->em.fb, b->mxlines, iold, b->b.y);
  jold = e_lst_zeichen(f->a.x+19, f->e.y, f->e.x-f->a.x-20, 1,
    f->fb->em.fb, b->mx.x, jold, b->b.x);
 }
 b->cl = b->b.x+j;
 fk_locate(f->a.x+b->b.x-s->c.x+j+1, f->a.y+b->b.y-s->c.y+1);
}

/*   delete one line */
int e_del_line(int yd, BUFFER *b, SCHIRM *s)
{
 int i;

 if (b->mxlines == 1)
 {
  *(b->bf[0].s) = '\0';
  b->bf[0].nrc = b->bf[0].len = 0;
 }
 else
 {
  e_add_undo('l', b, 0, yd, 1);
  (b->mxlines)--;
  for (i = yd; i < b->mxlines; i++)
   b->bf[i] = b->bf[i+1];
  if (s->mark_begin.y > yd)
   (s->mark_begin.y)--;
  else if (s->mark_begin.y == yd)
   (s->mark_begin.x) = 0;
  if (s->mark_end.y > yd)
   (s->mark_end.y)--;
  else if (s->mark_end.y == yd)
  {
   (s->mark_end.y)--;
   s->mark_end.x = b->bf[yd-1].len;
  }
 }
 sc_txt_3(yd, b, -1);
 if (b->f)
  (b->f->save) += 10;
/*******************************/   
 e_brk_recalc(b->f,yd,-1);
/*******************************/   
 return(b->mxlines);
}

/*   delete N chars from buffer */
int e_del_nchar(BUFFER *b, SCHIRM *s, int x, int y, int n)
{
 FENSTER *f = WpeEditor->f[WpeEditor->mxedt];
 int len, i, j;

 (f->save) += n;
 e_add_undo('r', b, x, y, n);
 e_undo_sw++;
 len = b->bf[y].len;
 if (*(b->bf[y].s+len) == WPE_WR) len++;
 for (j = x; j <= len - n; ++j) *(b->bf[y].s+j) = *(b->bf[y].s+j+n)
  ;
 if (s->mark_begin.y == y && s->mark_begin.x > x)
  s->mark_begin.x = (s->mark_begin.x-n > x) ? s->mark_begin.x-n : x;
 if (s->mark_end.y == y && s->mark_end.x > x)
  s->mark_end.x = (s->mark_end.x-n > x) ? s->mark_end.x-n : x;

 if (len <= n) e_del_line(y, b, s);
 else if (y < b->mxlines - 1 && *(b->bf[y].s+len-n-1) != WPE_WR )
 {
  if (b->mx.x+n-len > b->bf[y+1].len)
   i = b->bf[y+1].len;
  else
   i = e_su_rblk(b->mx.x+n-len, b->bf[y+1].s);
  if (b->bf[y+1].s[i] == WPE_WR) i++;
  if (i > 0)
  {
   for (j = 0; j < i; ++j)
    *(b->bf[y].s+len-n+j) = *(b->bf[y+1].s+j);
   *(b->bf[y].s+len-n+i) = '\0';
   if (s->mark_begin.y == y+1 && s->mark_begin.x <= i)
   {  s->mark_begin.y--;  s->mark_begin.x += (len-n);  }
   if (s->mark_end.y == y+1 && s->mark_end.x <= i)
   {  s->mark_end.y--;  s->mark_end.x += (len-n);  }
   e_del_nchar(b, s, 0, y+1, i);
  }
 }
 if (y < b->mxlines)
 {
  b->bf[y].len = e_str_len(b->bf[y].s);
  b->bf[y].nrc = e_str_nrc(b->bf[y].s);
 }
 e_undo_sw--;
 sc_txt_4(y, b, 0);
 return(x+n);
}

/*   insert N chars in buffer */
int e_ins_nchar(BUFFER *b, SCHIRM *sch, unsigned char *s, int xa, int ya,
  int n)
{
 FENSTER *f = WpeEditor->f[WpeEditor->mxedt];
 int i, j;

 (f->save) += n;
 e_add_undo('a', b, xa, ya, n);
 e_undo_sw++;
 if (b->bf[ya].len+n >= b->mx.x-1)
 {
  if (xa < b->bf[ya].len)
  {
   i = b->mx.x-n-1;
   if (i >= b->bf[ya].len-1)
    i = b->bf[ya].len-1;
  }
  else
  {
   for (; xa < b->mx.x-1; xa++, s++, n--)
   {
    *(b->bf[ya].s + xa + 1) = *(b->bf[ya].s + xa);
    *(b->bf[ya].s + xa) = *s;
   }
   *(b->bf[ya].s + xa + 1) = '\0';
   i = b->mx.x;
   b->bf[ya].len = e_str_len(b->bf[ya].s);
   b->bf[ya].nrc = e_str_nrc(b->bf[ya].s);
  }
  for (; i > 0 && *(b->bf[ya].s+i) != ' ' && *(b->bf[ya].s+i) != '-'; i--);
  if (i == 0)
  {
   if (s[n-1] != ' ' && s[n-1] != '-')
    i = b->bf[ya].len - 2;
   else
    i--;
  }
  if (*(b->bf[ya].s+b->bf[ya].len) == WPE_WR || ya == b->mxlines)
  {
   e_new_line(ya+1, b);
   if (sch->mark_begin.y > ya)
    (sch->mark_begin.y)++;
   else if(sch->mark_begin.y == ya)
   {
    if (sch->mark_begin.x > i)
    {
     (sch->mark_begin.y)++;
     (sch->mark_begin.x) -= (i+1);
    }
    else if (sch->mark_begin.x >= xa)
     sch->mark_begin.x += n;
   }
   if (sch->mark_end.y > ya)
    (sch->mark_end.y)++;
   else if(sch->mark_end.y == ya)
   {
    if(sch->mark_end.x > i)
    {
     (sch->mark_end.y)++;
     (sch->mark_end.x) -= (i+1);
    }
    else if(sch->mark_end.x >= xa)
     sch->mark_end.x += n;
   }
   for (j = i+1; *(b->bf[ya].s+j) != WPE_WR && *(b->bf[ya].s+j) != '\0'; j++)
    *(b->bf[ya+1].s+j-i-1) = *(b->bf[ya].s+j);
   *(b->bf[ya+1].s+j-i-1) = WPE_WR;
   b->bf[ya+1].len = e_str_len(b->bf[ya+1].s);
   b->bf[ya+1].nrc = e_str_nrc(b->bf[ya+1].s);
   sc_txt_4(ya, b, 1);
  }
  else
  {
   e_ins_nchar(b, sch, b->bf[ya].s+i+1, 0, ya+1, b->bf[ya].len-i-1);
   if (sch->mark_begin.y == ya)
   {
    if (sch->mark_begin.x > i)
    {
     (sch->mark_begin.y)++;
     (sch->mark_begin.x) -= (i+1);
    }
    else if (sch->mark_begin.x >= xa)
     sch->mark_begin.x += n;
   }
   if (sch->mark_end.y == ya)
   {
    if (sch->mark_end.x > i)
    {
     (sch->mark_end.y)++;
     (sch->mark_end.x) -= (i+1);
    }
    else if (sch->mark_end.x >= xa)
     sch->mark_end.x += n;
   }
  }
/*	 if(*(b->bf[ya].s+i) == ' ') *(b->bf[ya].s+i) = '\0';
         else
*/
  *(b->bf[ya].s+i+1) = '\0';
  b->bf[ya].len = e_str_len(b->bf[ya].s);
  b->bf[ya].nrc = e_str_nrc(b->bf[ya].s);
  if (xa > b->bf[ya].len)
  {
   xa -= (b->bf[ya].len);
   ya++;
   b->bf[ya].len = e_str_len(b->bf[ya].s);
   b->bf[ya].nrc = e_str_nrc(b->bf[ya].s);
   if (sch->mark_begin.y == ya && sch->mark_begin.x >= xa)
    sch->mark_begin.x += n;
   if (sch->mark_end.y == ya && sch->mark_end.x >= xa)
    sch->mark_end.x += n;
  }
 }
 else
 {
  if (sch->mark_begin.y == ya && sch->mark_begin.x >= xa)
   sch->mark_begin.x += n;
  if(sch->mark_end.y == ya && sch->mark_end.x >= xa)
   sch->mark_end.x += n;
 }
 for (j = b->bf[ya].len; j >= xa; --j)
  *(b->bf[ya].s+j+n) = *(b->bf[ya].s+j);
 for (j = 0; j < n; ++j)
  *(b->bf[ya].s+xa+j) = *(s+j);
 if (b->bf[ya].s[b->bf[ya].len] == WPE_WR)
  b->bf[ya].s[b->bf[ya].len+1] = '\0';
 b->b.x = xa + n;
 b->b.y = ya;
 b->bf[ya].len = e_str_len(b->bf[ya].s);
 b->bf[ya].nrc = e_str_nrc(b->bf[ya].s);
 e_undo_sw--;
 sc_txt_4(ya, b, 0);
 return(xa+n);
}

/*   insert new line */
int e_new_line(int yd, BUFFER *b)
{
 int i;

 if (b->mxlines > b->mx.y-2)
 {
  b->mx.y += MAXLINES;
  if ((b->bf = REALLOC(b->bf, b->mx.y * sizeof(STRING))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, b->fb);
  if (b->f->c_sw)
   b->f->c_sw = REALLOC(b->f->c_sw , b->mx.y * sizeof(int));
 }
 for (i = b->mxlines-1; i >= yd; i--)
 {
  b->bf[i+1] = b->bf[i];
 }
 (b->mxlines)++;
 b->bf[yd].s = MALLOC(b->mx.x+1);
 if (b->bf[yd].s == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, b->fb);
 *(b->bf[yd].s) = '\0';
 b->bf[yd].len = 0;
 b->bf[yd].nrc = 0;
 return(b->mxlines);
}

/*     Overwriting of a character       */
int e_put_char(int c, BUFFER *b, SCHIRM *s)
{
 unsigned char cc = c;

 if (b->b.x == b->bf[b->b.y].len)
  e_ins_nchar(b, s, &cc, b->b.x, b->b.y, 1);
 else
 {
  e_add_undo('p', b, b->b.x, b->b.y, 1);
  (b->f->save)++;
  *(b->bf[b->b.y].s+b->b.x) = c;
 }
 (b->b.x)++;
 return(b->b.x);
}

/*   search right (left end of word) */
int e_su_lblk(int xa, char *s)
{
 int len = strlen(s);

 if (xa >= len)
  xa = len - 1;
 for (; xa < len  && isalnum1(s[xa]); xa++)
  ;
 for (; xa < len && !isalnum1(s[xa]); xa++)
  ;
 return(xa);
}

/*     Search left (left end of word)     */
int e_su_rblk(int xa, char *s)
{
 int len = strlen(s);

 if (xa <= 0)
  return(xa);
 if (xa > len)
  xa = len;
 for (xa--; xa > 0 && !isalnum1(s[xa]); xa--)
  ;
 if (!xa)
  return(xa);
 for (; xa > 0 && isalnum1(s[xa]); xa--)
  ;
 return(!xa ? xa : xa + 1);
}

/* Prints out the line number and column of cursor */
void e_zlsplt(FENSTER *f)
{
 char str[20];

 if (!DTMD_ISTEXT(f->dtmd))
  return;
 sprintf(str,"%5d:%-4d", f->b->b.y + 1, f->b->cl + 1);
 e_puts(str,f->a.x+5,f->e.y,f->fb->er.fb);
 if (f->save)
  e_pr_char(f->a.x+3, f->e.y, '*', f->fb->er.fb);
 else
  e_pr_char(f->a.x+3, f->e.y, ' ', f->fb->er.fb);
#ifdef NEWSTYLE
 if (WpeIsXwin())
 {
  e_make_xrect(f->a.x + 1, f->e.y, f->e.x - 1, f->e.y, 0);
  e_make_xrect(f->a.x + 5, f->e.y, f->a.x + 14, f->e.y, 0);
 }
#endif
}

void WpeFilenameToPathFile(char *filename, char **path, char **file)
{
 char *tmp;
 char *cur_dir;
 int len;

 *path = NULL;
 tmp = strrchr(filename, DIRC);
 if (tmp)
 {
  *file = WpeStrdup(tmp + 1);
 }
 else
 {
  *file = WpeStrdup(filename);
 }
 if ((!tmp) || ((filename + 1 == tmp) && (*filename == '.')))
 {
  *path = WpeGetCurrentDir(WpeEditor);
 }
 else
 {
  if (*filename != DIRC)
  {
   if ((cur_dir = WpeGetCurrentDir(WpeEditor)) == NULL)
   {
    WpeFree(*file);
    *file = NULL;
    return ;
   }
   /* Delete the slash at the end. */
   len = strlen(cur_dir);
   cur_dir[len - 1] = 0;
   len = tmp - filename + 1;
   while (strncmp(filename, "../", 3) == 0)
   {
    tmp = strrchr(cur_dir, DIRC);
    if (tmp == cur_dir)
    {
     cur_dir[1] = 0;
    }
    else
    {
     *tmp = 0;
    }
    len -= 3;
    filename += 3;
   }
   *path = WpeMalloc((strlen(cur_dir) + len + 2) * sizeof(char));
   strcpy(*path, cur_dir);
   strcat(*path, DIRS);
   strncat(*path, filename, len);
   if ((*path)[strlen(cur_dir) + len] != DIRC)
   {
    (*path)[strlen(cur_dir) + len + 1] = 0;
   }
   else
   {
    (*path)[strlen(cur_dir) + len] = DIRC;
    (*path)[strlen(cur_dir) + len + 1] = 0;
   }
   WpeFree(cur_dir);
  }
  else
  {
   len = tmp - filename + 1;
   *path = WpeMalloc(len + 1 * sizeof(char));
   strncpy(*path, filename, len);
   (*path)[len] = 0;
  }
 }
}

/*   draw mouse slider cursor */
int e_lst_zeichen(int x, int y, int n, int sw, int frb, int max, int iold,
  int new)
{
 int inew;
 double d = max ? 1./(float)max : 0;

 if (n < 3)
  return(1);

 if ((inew = (int) (new*(n-2)*d + 1.5)) > n-2)
  inew = n-2;
 if (iold < 1)
  iold = 1;
 if (inew < 1)
  inew = 1;

 if (sw == 0)
 {
  if (iold < n-1)
   e_pr_char(x, y+iold, MCI, frb);
  e_pr_char(x, y+inew, MCA, frb);
  e_make_xrect(x, y+1, x, y+n-2, 0);
  e_make_xrect(x, y+inew, x, y+inew, 0);
 }
 else
 {
  if (iold < n-1) e_pr_char(x+iold, y, MCI, frb);
   e_pr_char(x+inew, y, MCA, frb);
  e_make_xrect(x+1, y, x+n-2, y, 0);
  e_make_xrect(x+inew, y, x+inew, y, 0);
 }
 return(inew);
}

/*   draw mouse slider bar */
void e_mouse_bar(int x, int y, int n, int sw, int frb)
{
 int sv = n;

 if (sw == 0)
 {
  e_pr_char(x, y, MCU, frb);
  e_pr_char(x, y+n-1, MCD, frb);
  for (; n > 2; n--)
   e_pr_char(x, y+n-2, MCI, frb);
  e_make_xrect(x, y+1, x, y+sv-2, 0);
  e_make_xrect(x, y, x, y, 0);
  e_make_xrect(x, y+sv-1, x, y+sv-1, 0);
 }
 else
 {
  e_pr_char(x, y, MCL, frb);
  e_pr_char(x+n-1, y, MCR, frb);
  for (; n > 2; n--)
   e_pr_char(x+n-2, y, MCI, frb);
  e_make_xrect(x+1, y, x+sv-2, y, 0);
  e_make_xrect(x, y, x, y, 0);
  e_make_xrect(x+sv-1, y, x+sv-1, y, 0);
 }
}

int e_autosave(FENSTER *f)
{
 char *tmp, *str;
 long maxname;

 f->save = 1;
 if (!(f->ed->autosv & 2)) return(0);
 /* Check if file system could have an autosave or emergency save file
    >12 check is to eliminate dos file systems */
 if (((maxname = pathconf(f->dirct, _PC_NAME_MAX)) >= strlen(f->datnam) + 4) &&
   (maxname > 12))
 {
  str = MALLOC(strlen(f->datnam) + 5);
  str = e_make_postf(str, f->datnam, ".ASV");
  tmp = f->datnam;
  f->datnam = str;
  /*e_save(f);*/
  WpeMouseChangeShape(WpeWorkingShape);
  e_write(0, 0, f->b->mx.x, f->b->mxlines-1, f, WPE_NOBACKUP);
  WpeMouseRestoreShape();
  f->datnam = tmp;
  f->save = 1;
  FREE(str);
 }
 return(0);
}

Undo *e_remove_undo(Undo *ud, int sw)
{
 if (ud == NULL)
  return(ud);
 ud->next = e_remove_undo(ud->next, sw + 1);
 if (sw > WpeEditor->numundo)
 {
  if(ud->type == 'l')
   FREE(ud->u.pt);
  else if (ud->type == 'd')
  {
   BUFFER *b = (BUFFER*) ud->u.pt;
   int i;

   FREE(b->f->s);
   FREE(b->f);
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
  FREE(ud);
  ud = NULL;
 }
 return(ud);
}

int e_add_undo(int sw, BUFFER *b, int x, int y, int n)
{
 Undo *next;

 if (e_undo_sw) return(0);
 if (!e_redo_sw && b->rd)
  b->rd = e_remove_undo(b->rd, WpeEditor->numundo+1);
 if ((next = MALLOC(sizeof(Undo))) == NULL)
 {
  e_error(e_msg[ERR_LOWMEM], 0, b->fb);
  return(-1);
 }
 next->type = sw;
 next->b.x = x;
 next->b.y = y;
 next->a.x = n;
 if (e_redo_sw == 1) next->next = b->rd;
 else next->next = b->ud;
 if (sw == 'a') ;
 else if (sw == 'p') next->u.c = b->bf[y].s[x];
 else if (sw == 'r' || sw == 's')
 {
  char *str = MALLOC(n);
  int i;

  if (str == NULL)
  {
   e_error(e_msg[ERR_LOWMEM], 0, b->fb);
   FREE(next);
   return(-1);
  }
  for (i = 0; i < n; i++)
   str[i] = b->bf[y].s[x+i];
  next->u.pt = str;

/* !!! obsolete !!!
  next->a.y = b->cn->fd.rn;
*/
  next->a.y = b->f->fd.rn;

 }
 else if (sw == 'l') next->u.pt = b->bf[y].s;
 else if (sw == 'c' || sw == 'v')
 {
  SCHIRM *s = b->cn->f[b->cn->mxedt]->s;

  next->a = s->mark_begin;
  next->e = s->mark_end;
 }
 else if (sw == 'd')
 {
  BUFFER *bn = MALLOC(sizeof(BUFFER));
  SCHIRM *sn = MALLOC(sizeof(SCHIRM));
  FENSTER *fn = MALLOC(sizeof(FENSTER));
  FENSTER *f = b->cn->f[b->cn->mxedt];

  bn->bf = (STRING *) MALLOC(MAXLINES*sizeof(STRING));
  if (bn == NULL || sn == 0 || bn->bf == NULL)
   return(e_error(e_msg[ERR_LOWMEM], 0, b->fb));
  fn->b = bn;
  fn->c_sw = NULL;
  fn->c_st = NULL;
  fn->ed = NULL; /* Quick fix so that breakpoints aren't recalculated */
  fn->fd.dirct = NULL;
  bn->f = fn;
  bn->f->s = sn;
  bn->b = e_set_pnt(0, 0);
  bn->mx = e_set_pnt(b->cn->maxcol, MAXLINES);
  bn->mxlines = 0;
  sn->fb = bn->fb = b->fb;
  bn->cn = b->cn;
  bn->ud = NULL;
  bn->rd = NULL;
  sn->c = sn->ks = sn->mark_begin = sn->mark_end = sn->fa = sn->fe = e_set_pnt(0, 0);
  e_new_line(0 ,bn);
  *(bn->bf[0].s) = WPE_WR;
  *(bn->bf[0].s+1) = '\0';
  bn->bf[0].len = 0;
  bn->bf[0].nrc = 1;
  next->u.pt = bn;
  e_undo_sw = 1;
  e_move_block(0, 0, b, bn, f);
  e_undo_sw = 0;
 }
 if (e_redo_sw == 1) b->rd = next;
 else
 {
  next->next = e_remove_undo(b->ud, 1);
  b->ud = next;
 }
 return(0);
}

int e_make_undo(FENSTER *f)
{
 return(e_make_rudo(f, 0));
}

int e_make_redo(FENSTER *f)
{
 return(e_make_rudo(f, 1));
}

int e_make_rudo(FENSTER *f, int sw)
{
 BUFFER *b;
 SCHIRM *s;
 Undo *ud;
 int i;

 for (i = f->ed->mxedt; i > 0 && !DTMD_ISTEXT(f->ed->f[i]->dtmd); i--);
 if (i <= 0) return(0);
 e_switch_window(f->ed->edt[i], f);
 f = f->ed->f[f->ed->mxedt];
 b = f->b;  s = f->s;
 if (!sw) ud = b->ud;
 else ud = b->rd;
 if (ud == NULL)
 {
  e_error((!sw ? e_msg[ERR_UNDO] : e_msg[ERR_REDO]), 0, b->fb);
  return(-1);
 }
 f = f->ed->f[f->ed->mxedt];
 if (!sw) e_redo_sw = 1;
 else e_redo_sw = 2;
 b->b = ud->b;
 if (ud->type == 'r' || ud->type == 's')
 {
  if (ud->type == 's')
  {
   e_add_undo('s', b, ud->b.x, ud->b.y, ud->a.y);
   e_undo_sw = 1;
   e_del_nchar(b, s, ud->b.x, ud->b.y, ud->a.y);
  }
  if (*((char*)ud->u.pt) == '\n' && ud->a.x == 1) e_car_ret(b, s);
  else if (*((char*)ud->u.pt + ud->a.x - 1) == '\n')
  {
   e_ins_nchar(b, s, ((char *)ud->u.pt), ud->b.x, ud->b.y, ud->a.x-1);
   e_car_ret(b, s);
  }
  else e_ins_nchar(b, s, ((char *)ud->u.pt), ud->b.x, ud->b.y, ud->a.x);
  e_undo_sw = 0;
  s->mark_begin = ud->b;
  s->mark_end.y = ud->b.y;
  s->mark_end.x = ud->b.x + ud->a.x;
  FREE(ud->u.pt);
 }
 else if (ud->type == 'l')
 {
  for(i = b->mxlines; i > ud->b.y; i--) b->bf[i] = b->bf[i-1];
  (b->mxlines)++;
  b->bf[b->b.y].s = ud->u.pt;
  b->bf[b->b.y].len = e_str_len(b->bf[b->b.y].s);
  b->bf[b->b.y].nrc = e_str_nrc(b->bf[b->b.y].s);
  s->mark_begin = ud->b;
  s->mark_end.y = ud->b.y + 1;
  s->mark_end.x = 0;
  e_add_undo('y', b, 0, b->b.y, 0);
 }
 else if (ud->type == 'y')
  e_del_line(b->b.y, b, s);
 else if (ud->type == 'a')
  e_del_nchar(b, s, ud->b.x, ud->b.y, ud->a.x);
 else if (ud->type == 'p')
  b->bf[ud->b.y].s[ud->b.x] = ud->u.c;
 else if (ud->type == 'c')
 {
  b->b = s->mark_begin = ud->a;
  s->mark_end = ud->e;
/*	e_blck_clear(b, s);   */
  e_blck_del(f);
 }
 else if (ud->type == 'v')
 {
  b->b = ud->b;
  s->mark_begin = ud->a;
  s->mark_end = ud->e;
  e_blck_move(f);
 }
 else if (ud->type == 'd')
 {
  BUFFER *bn = (BUFFER*)ud->u.pt;
  e_undo_sw = 1;
  s->mark_begin = bn->f->s->mark_begin;
  s->mark_end = bn->f->s->mark_end;
  e_move_block(ud->b.x, ud->b.y, bn, b, f);
  e_undo_sw = 0;
  FREE(bn->f->s);
  FREE(bn->f);
  FREE(bn->bf[0].s);
  FREE(bn->bf);
  FREE(ud->u.pt);
  e_add_undo('c', b, ud->b.x, ud->b.y, 0);
 }
 if (!sw) b->ud = ud->next;
 else b->rd = ud->next;
 e_redo_sw = 0;
 FREE(ud);
 e_schirm(f, 1);
 e_cursor(f, 1);
 return(0);
}

char *e_make_postf(char *out, char *name, char *pf)
{
 strcpy(out, name);
 strcat(out, pf);
 return(out);
}

