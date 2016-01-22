/* we_fl_unix.c                                           */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"

#ifdef UNIX

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

struct dirfile *e_make_win_list(FENSTER * f);
extern char    *e_tmp_dir;

#ifdef NOSYMLINKS
#define readlink(x, y, z) -1
#define WpeRenameLink(x, y, z, f) 0
#define WpeLinkFile(x, y, sw, f) link(x, y)
#define lstat(x,y)  stat(x,y)
#undef S_ISLNK
#define S_ISLNK(x)  0
#else
#include <unistd.h>
#endif

#define WPE_PATHMAX 2048

/* buffer size for copying */
#define E_C_BUFFERSIZE 524288        /*   1/2  Mega Byte   */

#ifdef DEBUG
int SpecialError(char *text, int sw, FARBE *f, char *file, int line)
{
  fprintf(stderr, "\nFile \"%s\" line %d\n", file, line);
  return e_error(text, sw, f);
}

#define e_error(text, sw, f) SpecialError(text, sw, f, __FILE__, __LINE__)
#endif


/* setup the file-manager structures 
   'dirct' or the current dir. is used to 
   search for directories and files */
/* sw | file manager type
   ------------------------
   0  | open/new file
   1  | read block of text from file
   2  | write block of text to file
   3  | execute file
   4  | save as
   5  | add file to project
   6  | wastebasket

*/
int WpeCreateFileManager(int sw, ECNT *cn, char *dirct)
{
  extern char    *e_hlp_str[];
  extern WOPT    *fblst, *rblst, *wblst, *xblst, *sblst, *ablst;
  FENSTER        *f;
  int             i, j;
  FLBFFR         *b;
  int             allocate_size;  /* inital memory size for allocation */
  char           *sfile;

  /* check whether we reached the maximum number of windows */
  if(cn->mxedt >= MAXEDT)
  {
    e_error(e_msg[ERR_MAXWINS], 0, cn->fb);
    return(-1);
  }

  /* search for a not used window ID number (j) */
  for(j = 1; j <= MAXEDT; j++)
  {
    for(i = 1; i <= cn->mxedt && cn->edt[i] != j; i++)
      ;
    if(i > cn->mxedt)
      break;
  }

  /* change the shape of the mouse :-) */
  WpeMouseChangeShape(WpeWorkingShape);

  /* currently active window, 
     one more window in the system, 
     its number is j */
  cn->curedt = j;
  (cn->mxedt)++;
  cn->edt[cn->mxedt] = j;

  /* allocate window structure */
  if((f = (FENSTER *)MALLOC(sizeof(FENSTER))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  /* allocate buffer related to the window (NOT proper type, later casted) */
  if((b = (FLBFFR *) MALLOC(sizeof(FLBFFR))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  f->fb = cn->fb;
  cn->f[cn->mxedt] = f;        /* store the window structure at appropriate place */
  f->a = e_set_pnt(11, 2);     /* beginning of the box */
  f->e = e_set_pnt(f->a.x + 55, f->a.y + 20); /* other coord. of the box */
  f->winnum = cn->curedt;
  f->dtmd = DTMD_FILEMANAGER;
  f->ins = 1;
  f->save = 0;
  f->zoom = 0;
  f->ed = cn;
  f->c_sw = NULL;
  f->c_st = NULL;
  f->pic = NULL;
  if(sw == 6)
  {
    sw = 0;
    f->datnam = "Wastebasket";
    f->save = 1;
  }
  else
    f->datnam = "File-Manager";  /* window header text */

  /* status line text for different mode */
  if(sw == 0)
  {
    f->blst = fblst;
    f->nblst = 8;
  }
  else if(sw == 1)
  {
    f->blst = rblst;
    f->nblst = 4;
  }
  else if(sw == 2)
  {
    f->blst = wblst;
    f->nblst = 4;
  }
  else if(sw == 3)
  {
    f->blst = xblst;
    f->nblst = 4;
  }
  else if(sw == 4)
  {
    f->blst = sblst;
    f->nblst = 5;
  }
  else if(sw == 5)
  {
    f->blst = ablst;
    f->nblst = 4;
  }

  if(sw == 3)
    f->hlp_str = e_hlp_str[5];
  else
    f->hlp_str = e_hlp_str[4];

  if(!dirct || dirct[0] == '\0')
  {
    /* no working directory has been given */
    f->dirct = WpeGetCurrentDir(cn);
  }
  else
  {
    /* working directory is given, copy it over */
    allocate_size = strlen(dirct);
    if((f->dirct = MALLOC(allocate_size+1)) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

    strcpy(f->dirct, dirct);
  }

  strcpy(f->fd.search, "");
  strcpy(f->fd.replace, "");
  strcpy(f->fd.file, SUDIR);
  f->fd.dirct = WpeStrdup(f->dirct);

  f->fd.sw = 16;
  f->fd.sn = 0;
  f->fd.rn = 0;

  f->b = (BUFFER *)b;
  /* the find pattern can only be 79 see FIND structure */
  if((b->rdfile = MALLOC(80)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
  strcpy(b->rdfile, f->fd.file);               /* find file pattern */

  b->sw = sw;

  /* window for files */
  if((b->fw = (FLWND *) MALLOC(sizeof(FLWND))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  /* window for directory */
  if((b->dw = (FLWND *) MALLOC(sizeof(FLWND))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);


  if((sfile = MALLOC(strlen(f->dirct)+strlen(b->rdfile)+2)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  /* determine current directory */
  b->cd = WpeCreateWorkingDirTree(f->save, cn);
  /* it is necessary to do this, because the file manager may not be
     in the appropriate directory here */
  sprintf(sfile, "%s/%s", f->dirct, SUDIR);
  /* find all other directories in the current */
  b->dd = e_find_dir(sfile, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
  /* setup the drawing in the dir tree window */
  b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);

  i = f->ed->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0;
  if(sw == 3)
    i |= 2;

  /* finds all files matching the pattern */
  sprintf(sfile, "%s/%s", f->dirct, b->rdfile);
  b->df = e_find_files(sfile, i);

  FREE(sfile);

  /* setup the drawing in the file list window */
  b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);

  /* file box - geometry and vertical slider settings */
  b->fw->mxa = f->a.x;
  b->fw->mxe = f->e.x;
  b->fw->mya = f->a.y;
  b->fw->mye = f->e.y;
  b->fw->xa = f->e.x - 33;
  b->fw->xe = f->e.x - 17;
  b->fw->ya = f->a.y + 6;
  b->fw->ye = f->a.y + 17;
  b->fw->f = f;
  b->fw->ia = b->fw->nf = b->fw->nxfo = b->fw->nyfo = 0;
  b->fw->srcha = b->fw->ja = 12;

  /* directory box - geometry and vertical slider settings */
  b->dw->mxa = f->a.x;
  b->dw->mxe = f->e.x;
  b->dw->mya = f->a.y;
  b->dw->mye = f->e.y;
  b->dw->xa = f->a.x + 3;
  b->dw->xe = f->a.x + 28;
  b->dw->ya = f->a.y + 6;
  b->dw->ye = f->a.y + 17;
  b->dw->f = f;
  b->dw->ia = b->dw->ja = b->dw->nxfo = 0;
  b->dw->srcha = -1;
  b->dw->nf = b->dw->nyfo = b->cd->anz - 1;

  if(cn->mxedt > 1)
    e_ed_rahmen(cn->f[cn->mxedt - 1], 0);

  e_firstl(f, 1);

  /* basically it draws the window out */
  WpeDrawFileManager(f);

  /* restore the shape of the mouse */
  WpeMouseRestoreShape();
  return(0);
}

/* drawing out the file-manager, 
   first buttons, than the dir tree and file list */
int WpeDrawFileManager(FENSTER * f)
{
  FLBFFR         *b = (FLBFFR *)f->b;
  int             i, j;
  int             bx1 = 1, bx2 = 1, bx3 = 1, by = 4;

  for(j = f->a.y + 1; j < f->e.y; j++)
    for(i = f->a.x + 1; i < f->e.x; i++)
      e_pr_char(i, j, ' ', f->fb->nt.fb);

  if(NUM_LINES_ON_SCREEN <= 17)
    by = -1;
  else if(b->sw != 0 || NUM_LINES_ON_SCREEN <= 19)
    by = 2;

  if(NUM_LINES_ON_SCREEN > 17)
  {
    e_pr_str((f->a.x + 4), f->e.y - by, "Cancel", f->fb->nz.fb, -1, -1,
             f->fb->ns.fb, f->fb->nt.fb);
    e_pr_str((f->a.x + 14), f->e.y - by, "Change Dir", f->fb->nz.fb, 0, -1,
             f->fb->ns.fb, f->fb->nt.fb);
    if(b->sw == 1 && NUM_COLS_ON_SCREEN >= 34)
      e_pr_str((f->a.x + 28), f->e.y - by, "Read", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
    else if(b->sw == 2 && NUM_COLS_ON_SCREEN >= 35)
      e_pr_str((f->a.x + 28), f->e.y - by, "Write", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
    else if(b->sw == 4)
    {
      if(NUM_COLS_ON_SCREEN >= 34)
        e_pr_str((f->a.x + 28), f->e.y - by, "Save", f->fb->nz.fb, 0, -1,
                 f->fb->ns.fb, f->fb->nt.fb);
    }
    else if(b->sw == 3 && NUM_COLS_ON_SCREEN >= 37)
      e_pr_str((f->a.x + 28), f->e.y - by, "Execute", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
    else if(b->sw == 5 && NUM_COLS_ON_SCREEN >= 33)
      e_pr_str((f->a.x + 28), f->e.y - by, "Add", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
    else if(b->sw == 0)
    {
      if(NUM_COLS_ON_SCREEN >= 35)
        e_pr_str((f->a.x + 28), f->e.y - by, "MKdir", f->fb->nz.fb, 1, -1,
                 f->fb->ns.fb, f->fb->nt.fb);
      if(NUM_COLS_ON_SCREEN >= 49)
        e_pr_str((f->a.x + 37), f->e.y - by, "Attributes", f->fb->nz.fb, 0, -1,
                 f->fb->ns.fb, f->fb->nt.fb);
    }
  }
  if(b->sw == 0 && NUM_LINES_ON_SCREEN > 19)
  {
    e_pr_str((f->a.x + 4), f->e.y - 2, "Move", f->fb->nz.fb, 0, -1,
             f->fb->ns.fb, f->fb->nt.fb);

    if(NUM_COLS_ON_SCREEN >= 21)
      e_pr_str((f->a.x + 13), f->e.y - 2, "Remove", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);

    if(NUM_COLS_ON_SCREEN >= 30)
      e_pr_str((f->a.x + 24), f->e.y - 2, "Link", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);

    if(NUM_COLS_ON_SCREEN >= 39)
      e_pr_str((f->a.x + 33), f->e.y - 2, "COpy", f->fb->nz.fb, 1, -1,
               f->fb->ns.fb, f->fb->nt.fb);

    if(NUM_COLS_ON_SCREEN >= 48)
      e_pr_str((f->a.x + 42), f->e.y - 2, "Edit", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);

  }
  if(NUM_COLS_ON_SCREEN < 45)
    bx3 = 0;
  if(NUM_COLS_ON_SCREEN < 44)
    bx2 = 0;
  if(NUM_COLS_ON_SCREEN < 43)
    bx1 = 0;
  b->xfd = (NUM_COLS_ON_SCREEN - bx1 - bx2 - bx3 - 6) / 2;
  b->xdd = NUM_COLS_ON_SCREEN - bx1 - bx2 - bx3 - b->xfd - 6;
  b->xda = 2 + bx1;
  b->xfa = 4 + bx1 + bx2 + b->xdd;

  e_pr_str((f->a.x + b->xfa), f->a.y + 2, "Name:", f->fb->nt.fb, 0, 1,
           f->fb->nsnt.fb, f->fb->nt.fb);
/*    e_schr_nchar(b->rdfile, f->a.x+b->xfa, f->a.y+3, 0, b->xfd+1, f->fb->fr.fb);   */
  e_schr_nchar_wsv(b->rdfile, f->a.x + b->xfa, f->a.y + 3, 0, b->xfd + 1,
                   f->fb->fr.fb, f->fb->fz.fb);
  e_pr_str((f->a.x + b->xfa), f->a.y + 5, "Files:", f->fb->nt.fb, 0, 1,
           f->fb->nsnt.fb, f->fb->nt.fb);

  e_pr_str((f->a.x + b->xda), f->a.y + 2, "Directory:", f->fb->nt.fb, 0, 1,
           f->fb->nsnt.fb, f->fb->nt.fb);
/*    e_schr_nchar(f->dirct, f->a.x+b->xda, f->a.y+3, 0, b->xdd+1, f->fb->fr.fb);   */
  e_schr_nchar_wsv(f->dirct, f->a.x + b->xda, f->a.y + 3, 0, b->xdd + 1,
                   f->fb->fr.fb, f->fb->fz.fb);
  e_pr_str((f->a.x + b->xda), f->a.y + 5, "DirTree:", f->fb->nt.fb, 3, 1,
           f->fb->nsnt.fb, f->fb->nt.fb);

  b->fw->mxa = f->a.x;
  b->fw->mxe = f->e.x;
  b->fw->mya = f->a.y;
  b->fw->mye = f->e.y;
  b->fw->xa = f->a.x + b->xfa;
  b->fw->xe = b->fw->xa + b->xfd;
  b->fw->ya = f->a.y + 6;
  b->fw->ye = f->e.y - 2 - by;
  b->dw->mxa = f->a.x;
  b->dw->mxe = f->e.x;
  b->dw->mya = f->a.y;
  b->dw->mye = f->e.y;
  b->dw->xa = f->a.x + b->xda;
  b->dw->xe = b->dw->xa + b->xdd;
  b->dw->ya = f->a.y + 6;
  b->dw->ye = f->e.y - 2 - by;

  /* slider bars for file list */
  e_mouse_bar(b->fw->xe, b->fw->ya, b->fw->ye - b->fw->ya, 0, b->fw->f->fb->em.fb);
  e_mouse_bar(b->fw->xa, b->fw->ye, b->fw->xe - b->fw->xa, 1, b->fw->f->fb->em.fb);
  /* file list window */
  e_pr_file_window(b->fw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);

  /* slide bars for directory window */
  e_mouse_bar(b->dw->xe, b->dw->ya, b->dw->ye - b->dw->ya, 0, b->dw->f->fb->em.fb);
  e_mouse_bar(b->dw->xa, b->dw->ye, b->dw->xe - b->dw->xa, 1, b->dw->f->fb->em.fb);
  /* directory window */
  e_pr_file_window(b->dw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);
  return(0);
}

/* tries to find the required style file manager */
int WpeCallFileManager(int sw, FENSTER * f)
{
  int             i, ret;
  FLBFFR         *b;

  for(i = f->ed->mxedt; i > 0; i--)
    if(f->ed->f[i]->dtmd == DTMD_FILEMANAGER)     /* check only file manager windows */
    {
      b = (FLBFFR *)f->ed->f[i]->b;
      /* open/new file manager and it is not in save mode */
      if(sw == 0 && b->sw == sw && f->ed->f[i]->save != 1)
        break;
      /* wastebasket mode required, 
         the window is "open/new" file manager and save mode turned on */
      else if(sw == 6 && b->sw == 0 && f->ed->f[i]->save == 1)
        break;
      /* not open/new or wastebasket filemanager and it is the required style */
      else if(sw != 0 && sw != 6 && b->sw == sw)
        break;
    }

  if(i <= 0) /* we did not find the required style file-manager */
  {
    if(sw == 6) /* wastebasket mode */
    {
      char *tmp;

      if ((tmp = WpeGetWastefile("")))
      {
        ret = WpeCreateFileManager(sw, f->ed, tmp);
        FREE(tmp);
        return(ret);
      }
      else
      {
        e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
        return 0;                /* Error -- no wastebasket */
      }
    }
    /* create the required style file manager */
    return(WpeCreateFileManager(sw, f->ed, ""));
  }
  /* switch to the found file manager */
  e_switch_window(f->ed->edt[i], f);
  return(0);
}

/* It will always create a new file manager */
int WpeManagerFirst(FENSTER * f)
{
  return(WpeCreateFileManager(0, f->ed, ""));
}

/* try to find an "open/new" style file manager or create one */
int WpeManager(FENSTER * f)
{
  return(WpeCallFileManager(0, f));
}

/* try to find an "execute" style file manager or create one */
int WpeExecuteManager(FENSTER * f)
{
  return(WpeCallFileManager(3, f));
}

/* try to find a "save as" style file manager or create one */
int WpeSaveAsManager(FENSTER * f)
{
  return(WpeCreateFileManager(4, f->ed, ""));
}


/* File Manager Handler */
int WpeHandleFileManager(ECNT * cn)
{
  FENSTER        *f = cn->f[cn->mxedt], *fe = NULL;
  FLBFFR         *b = (FLBFFR *) f->b;
  BUFFER         *be = NULL;
  SCHIRM         *se = NULL;
  int             c = AltC, i, j, t;
  int             winnum = 0, nco, svmode = -1, fmode, len, start;
  int             g[4], cold = AltN;
  char            filen[128], *ftmp, *dtp = NULL, *ftp = NULL, *svdir = NULL;
  char           *dirtmp = NULL;
  PIC            *outp = NULL;
  FILE           *fp;
  struct stat     buf;
  char            dtmd;

  /* check whether we really get a file-manager window here */
  if(f->dtmd != DTMD_FILEMANAGER)
    return(0);

  if(f->save == 1)
  {
    svmode = f->ed->flopt;
    f->ed->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
      FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
  }

  /* if it is project management or saving mode 
     save the current directory to return to it */
  if(f->save == 1 || b->sw == 5)
  {
    if((svdir = MALLOC(strlen(f->ed->dirct) + 1)) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
    strcpy(svdir, f->ed->dirct);
  }

  nco = b->cd->anz - 1;
  /* when searching among files, search hidden ones as well */
  fmode = f->ed->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0;
  /* in execution mode show hidden dirs as well */
  if(b->sw == 3)
    fmode |= 2;

  /* searching for the last edited/touched file on the desktop */
  for(i = cn->mxedt; i > 0; i--)
  {
    if (DTMD_ISTEXT(cn->f[i]->dtmd))
    {
      fe = cn->f[i];
      be = fe->b;
      se = fe->s;
      winnum = cn->edt[i];
      break;
    }
  }
  strcpy(f->fd.file, b->rdfile);

  /* go until quit */
  while(c != WPE_ESC)
  {
    /* draw out dir tree and file list windows */
    e_pr_file_window(b->fw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);
    e_pr_file_window(b->dw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);

    switch(c)
    {
      /* filename entry box activation */
      case AltN:
        cold = c;
        fk_cursor(1);
        /* get some answer from the name entry box, 
           result file copied into b->rdfile, max 79 char + '\0' */
        c = e_schr_lst_wsv(b->rdfile, f->a.x + b->xfa, f->a.y + 3, b->xfd + 1, 79,
                           f->fb->fr.fb, f->fb->fz.fb, &f->ed->fdf, f);

        /* determine the entered filename, going backward */
        for(i = strlen(b->rdfile); i >= 0 && b->rdfile[i] != DIRC; i--)
          ;
        strcpy(f->fd.file, b->rdfile + 1 + i);

        /* there is some directory structure in the filename */
        if(i >= 0)
        {
          if(i == 0)
            i++;
          b->rdfile[i] = '\0';
          /* change the working directory */
          FREE(f->dirct);
          if((f->dirct = MALLOC(strlen(b->rdfile) + 1)) == NULL)
            e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
          strcpy(f->dirct, b->rdfile);

          /* restore original filename */
          strcpy(b->rdfile, f->fd.file);
          c = AltC;
        }
#if  MOUSE
        /* if mouse was used (the reason it returned) get appropriate key interpretation */
        if(c == -1)
          c = WpeMngMouseInFileManager(f);
#endif
        if((c >= Alt1 && c <= Alt9) || (c >= 1024 && c <= 1049))
        {
          /* window changing, make the entry unhighlighted */
          e_schr_nchar_wsv(b->rdfile, f->a.x + b->xfa, f->a.y + 3, 0, b->xfd + 1,
                           f->fb->fr.fb, f->fb->fz.fb);
          break;
        }
        if(c == CLE || c == CCLE) /* goto dir name window */
          c = AltD;
        else if(c == CDO || c == BDO || c == WPE_TAB) /* goto file list window */
          c = AltF;
        else if(c == WPE_BTAB) /* goto dir tree window */
          c = AltT;
        else if(    (   c == WPE_CR 
                     || (b->sw == 0 && c == AltE)
                     || (b->sw == 1 && c == AltR) 
                     || (b->sw == 2 && c == AltW) 
                     || (b->sw == 3 && c == AltE) 
                     || (b->sw == 5 && c == AltA) 
                     || (b->sw == 4 && (c == AltS || c == AltY)) ) 
                 && (   strstr(b->rdfile, "*") 
                     || strstr(b->rdfile, "?")
                     || strstr(b->rdfile, "[") ) )
        {
          WpeMouseChangeShape(WpeWorkingShape);
          /* free up existing structures */
          freedf(b->df);
          freedf(b->fw->df);
          /* find files according to the new pattern */
          b->df = e_find_files(b->rdfile, fmode);
          /* setup the drawing in the dir tree window */
          b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);
          b->fw->ia = b->fw->nf = 0;
          b->fw->ja = b->fw->srcha;
          /* jump to file list window */
          c = AltF;
          WpeMouseRestoreShape();
        }
        else
        {
          strcpy(filen, b->rdfile);   /* !!! alloc for filen ??? */
          if(c == WPE_CR)
          {
            if(b->sw == 1)
              c = AltR;
            else if(b->sw == 2)
              c = AltW;
            else if(b->sw == 4)
              c = AltS;
            else if(b->sw == 5)
              c = AltA;
            else
              c = AltE;
          }
        }
        /* entry window is left, make the entry unhighlighted */
        if(c != AltN)
          e_schr_nchar_wsv(b->rdfile, f->a.x + b->xfa, f->a.y + 3, 0, b->xfd + 1,
                           f->fb->fr.fb, f->fb->fz.fb);
        fk_cursor(0);
        break;

      /* directory name entry box activation */
      case AltD:
        cold = c;
        fk_cursor(1);

        /* get the directory name */
        if ((dirtmp = WpeMalloc(WPE_PATHMAX)) == NULL)        /* dirct mat not have enough memory */
          e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

        if (strlen(f->dirct) >= WPE_PATHMAX)
        {
          strncpy(dirtmp, f->dirct, WPE_PATHMAX - 1);
          dirtmp[WPE_PATHMAX - 1] = '\0';
        }
        else
          strcpy(dirtmp, f->dirct);

        c = e_schr_lst_wsv(dirtmp, f->a.x + b->xda, f->a.y + 3, b->xdd + 1, WPE_PATHMAX,
                           f->fb->fr.fb, f->fb->fz.fb, &f->ed->ddf, f);
        FREE(f->dirct);
        f->dirct = WpeStrdup(dirtmp);
        WpeFree(dirtmp);

#if  MOUSE
        if(c == -1)
          c = WpeMngMouseInFileManager(f);
#endif
        if(c == CRI || c == CCRI) /* goto name entry windwow */
          c = AltN;
        else if(c == CDO || c == BDO || c == WPE_TAB) /* goto dir tree window */
          c = AltT;
        else if(c == WPE_BTAB) /* goto file list window */
          c = AltF;
        else if(c == WPE_CR) /* change dir */
          c = AltC;
        /* window left, make the entry unhighlighted */
        if(c != AltD)
          e_schr_nchar_wsv(f->dirct, f->a.x + b->xda, f->a.y + 3, 0, b->xdd + 1,
                           f->fb->fr.fb, f->fb->fz.fb);
        fk_cursor(0);
        break;

      /* directory tree list window activation */
      case AltT:
        cold = c;
        c = e_file_window(1, b->dw, f->fb->ft.fb, f->fb->fz.fb);
#if  MOUSE
        if(c == MBKEY)         /* handle mouse actions in the window */
          c = WpeMngMouseInFileManager(f);
        else if(c < 0)
          c = WpeMouseInFileDirList(c, 1, f);
#endif
        if(c == CCRI)
          c = AltF;
        else if(c == BUP)
          c = AltD;
        else if(c == WPE_TAB)
          c = AltN;
        else if(c == WPE_BTAB)
          c = AltD;
        else if(c == AltC || (c == WPE_CR && b->dw->nf != nco))
        {
          if ((dirtmp = WpeAssemblePath(f->dirct, b->cd, b->dd, b->dw->nf, f)))
          {
            FREE(f->dirct);
            f->dirct = dirtmp;
            e_schr_nchar_wsv(f->dirct, f->a.x + b->xda, f->a.y + 3, 0, b->xdd + 1,
                             f->fb->fr.fb, f->fb->fz.fb);
            f->ed->ddf = e_add_df(f->dirct, f->ed->ddf);
            c = AltC;
          }
          /* there is only one case when it cannot assemble the path,
             that it cannot access wastebasket, then quit the file manager */
          else
            c = WPE_ESC;
        }
        else if(c == WPE_CR)
          c = AltT;
        break;

      /* file list window activation */
      case AltF:
        if(b->df->anz < 1)
        {
          c = cold;
          break;
        }
        cold = c;
        c = e_file_window(1, b->fw, f->fb->ft.fb, f->fb->fz.fb);
#if  MOUSE
        if(c == MBKEY)
          c = WpeMngMouseInFileManager(f);
        else if(c < 0)
          c = WpeMouseInFileDirList(c, 0, f);
#endif
        if(c == BUP)  /* goto file name entry window */
          c = AltN;
        else if(c == CCLE) /* goto dir tree window */
          c = AltT;
        else if(c == WPE_TAB) /* goto dir entry window */
          c = AltD;
        else if(c == WPE_BTAB) /* goto file name entry window */
          c = AltN;
        else if(c == WPE_CR)  /* action selected */
        {
          if(b->sw == 1)
            c = AltR;
          else if(b->sw == 2)
            c = AltW;
          else if(b->sw == 4)
            c = AltS;
          else if(b->sw == 5)
            c = AltA;
          else
            c = AltE;
        }
        if(   (b->sw == 1 && c == AltR)  /* in case of action store the filename */
           || (b->sw == 2 && c == AltW)
           || (b->sw == 3 && c == AltE)
           || (b->sw == 0 && c == AltE) 
           || (b->sw == 4 && (c == AltS || c == AltY)) )
        {
          strcpy(filen, *(b->df->name + b->fw->nf));   /* !!! alloc for filen ??? */
        }
        break;

      /* change dir button activation */
      case AltC:
        c = cold;
        /* if the current dir is equal to the "newly" entered dir, break */
        if(!strcmp(f->ed->dirct, f->dirct))
          break;

        /* in wastebasket mode, we do not allow to go out of it through
           a soft link */
        if((b->sw == 0) && (f->save == 1))
        {
          if(lstat(f->dirct, &buf))
          {
            e_error(e_msg[ERR_ACCFILE], 0, f->ed->fb);
            break;
          }
          if(S_ISLNK(buf.st_mode))
          {
            /* cannot go out through a softlink, restore dir name */
            if((f->dirct = REALLOC(f->dirct, strlen(f->ed->dirct) + 1)) == NULL)
              e_error(e_msg[ERR_LOWMEM], 1, f->fb);
            else
              strcpy(f->dirct, f->ed->dirct);
            break;
          }
        }

        /* change to the desired dir with system error checking */
        if(chdir(f->dirct))
        {
          e_error(e_msg[ERR_WORKDIRACCESS], 0, f->fb);

          /* we cannot determine where we are, try the home */
          if((dirtmp = getenv("HOME")) == NULL)
            e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
          if(chdir(dirtmp))
            e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
        }

        /* get current directory */
        dirtmp = WpeGetCurrentDir(f->ed);

        /* change the shape of the mouse */
        WpeMouseChangeShape(WpeWorkingShape);

        FREE(f->dirct);
        f->dirct = dirtmp;

        /* free up all relevant structures */
        freedf(b->df);
        freedf(b->fw->df);
        freedf(b->cd);
        freedf(b->dw->df);
        freedf(b->dd);

        /* reset the current dir path in the control structure */
        if((f->ed->dirct = REALLOC(f->ed->dirct, strlen(f->dirct) + 1)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->fb);
        else
          strcpy(f->ed->dirct, f->dirct);

        /* setup current directory structure */
        b->cd = WpeCreateWorkingDirTree(f->save, cn);
        /* find all other directories in the current dir */
        b->dd = e_find_dir(SUDIR, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
        /* setup the drawing in the dir tree window */
        b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);

        nco = b->dw->nf = b->cd->anz - 1;
        b->dw->ia = b->dw->ja = 0;

        /* finds all files matching the pattern */
        b->df = e_find_files(b->rdfile, fmode);
        /* setup the drawing in the file list window */
        b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);
        b->fw->nf = b->fw->ia = 0;
        b->fw->ja = 12;

        /* change the shape of the mouse back */
        WpeMouseRestoreShape();
        break;

      /* link file activation */
      case AltL:
      /* copy file activation */
      case AltO:
      /* move file activation */
      case AltM:
        /* moving/copying a file is valid only in mode 0 (open/new file) */
        if(b->sw != 0)
        {
          c = cold;
          break;
        }
        j = c;

        /* we are coming from files */
        if(cold == AltF)
        {
          if((ftmp = MALLOC(129)) == NULL)
            e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

          if(strlen(*(b->df->name + b->fw->nf)) > 128)
          {
            strncpy(ftmp, *(b->df->name + b->fw->nf), 128);
            ftmp[128] = '\0';
          }
          else
            strcpy(ftmp, *(b->df->name + b->fw->nf));

          /* make the file name editable */
          c = e_schreib_leiste(ftmp, b->fw->xa, b->fw->ya + b->fw->nf - b->fw->ia,
                               b->fw->xe - b->fw->xa, 128, f->fb->fr.fb, f->fb->fz.fb);
          if(c == WPE_CR)
          {
            if(j == AltM)
              e_rename(*(b->df->name + b->fw->nf), ftmp, f); /* move */
            else if(j == AltL)
              WpeLinkFile(*(b->df->name + b->fw->nf), ftmp, /* link */
                          f->ed->flopt & FM_TRY_HARDLINK, f);
            else if(j == AltO)
              e_copy(*(b->df->name + b->fw->nf), ftmp, f); /* copy */

            /* after copying/moving/linking, free up the old structures */
            freedf(b->df);
            freedf(b->fw->df);

            /* generate the new file list */
            b->df = e_find_files(b->rdfile, fmode);
            /* setup the drawing of it */
            b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);
            b->fw->ia = b->fw->nf = 0;
            b->fw->ja = b->fw->srcha;
          }
          FREE(ftmp);
        }
        /* we are coming from dirs */
        else if(cold == AltT && b->dw->nf >= b->cd->anz)
        {
          if((ftmp = MALLOC(129)) == NULL)
            e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

          /* selected dir */
          t = b->dw->nf - b->cd->anz;

          if(strlen(*(b->dd->name + t)) > 128)
          {
            strncpy(ftmp, *(b->dd->name + t), 128);
            ftmp[128] = '\0';
          }
          else
            strcpy(ftmp, *(b->dd->name + t));


          /* separate the dir name from other drawings in the line */
          for(i = 0; *(b->dw->df->name[b->dw->nf] + i) &&
              (*(b->dw->df->name[b->dw->nf] + i) <= 32 ||
               *(b->dw->df->name[b->dw->nf] + i) >= 127); i++)
            ;

          if(!WpeIsXwin())
            i += 3;
          b->dw->ja = i;
          e_pr_file_window(b->dw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);
          /* make the name editable */
          c = e_schreib_leiste(ftmp, b->dw->xa, b->dw->ya + b->dw->nf - b->dw->ia, 
                               b->dw->xe - b->dw->xa, 128, f->fb->fr.fb, f->fb->fz.fb);
          if(c == WPE_CR)
          {
            if(j == AltM)
              e_rename(*(b->dd->name + t), ftmp, f); /* move */
            else if(j == AltL)
              e_link(*(b->dd->name + t), ftmp, f); /* link */
            else if(j == AltO)
              e_copy(*(b->dd->name + t), ftmp, f); /* copy */

            /* free up structures */
            freedf(b->cd);
            freedf(b->dw->df);
            freedf(b->dd);

            /* determine current directory */
            b->cd = WpeCreateWorkingDirTree(f->save, cn);
            /* find all other directories in the current */
            b->dd = e_find_dir(SUDIR, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
            /* setup drawing */
            b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);
            nco = b->dw->nf = b->cd->anz - 1;
            b->dw->ia = b->dw->ja = 0;
          }
          FREE(ftmp);
        }
        c = cold;
        cold = AltN; /* go back to name entry */
        break;

      /* remove button activation */
      case ENTF:
        if(b->sw != 0)
        {
          c = cold;
          break;
        }
      /* remove button activation */
      case AltR:
        if(b->sw == 0)
        {
          /* coming from file list */
          if(cold == AltF)
          {
            WpeRemove(*(b->df->name + b->fw->nf), f); /* remove the file */

            /* free up structures */
            freedf(b->df);
            freedf(b->fw->df);

            /* find files according to the pattern */
            b->df = e_find_files(b->rdfile, fmode);

            /* setup drawing */
            b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);
            b->fw->ia = b->fw->nf = 0;
            b->fw->ja = b->fw->srcha;
          }
          /* coming from the dir tree list and the selected dir is a subdir of
             the current directory */
          else if(cold == AltT && b->dw->nf >= b->cd->anz)
          {
            t = b->dw->nf - b->cd->anz;
            WpeRemove(*(b->dd->name + t), f);  /* remove the dir */

            /* free up structures */
            freedf(b->cd);
            freedf(b->dw->df);
            freedf(b->dd);

            /* determine current directory */
            b->cd = WpeCreateWorkingDirTree(f->save, cn);
            /* find all other directories in the current */
            b->dd = e_find_dir(SUDIR, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
            /* setup drawing */
            b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);
            nco = b->dw->nf = b->cd->anz - 1;
            b->dw->ia = b->dw->ja = 0;
          }
          c = cold;
          cold = AltN; /* go back to name entry */
          break;
        }

      /* edit/execute button activation */
      case AltE:
        if(   (c == AltE && b->sw != 0 && b->sw != 3) 
           || (c == AltR && b->sw != 1) )
        {
          c = cold;
          break;
        }
        if(b->sw == 3) /* file-manager in execution mode */
        {
          if(cold == AltF)
            strcpy(filen, *(b->df->name + b->fw->nf));     /* !!! alloc filen ??? */
          if(!WpeIsXwin())
          {
            outp = e_open_view(0, 0, MAXSCOL - 1, MAXSLNS - 1, f->fb->ws, 1);
            fk_locate(0, 0);
            fk_cursor(1);
#if  MOUSE
            g[0] = 2;
            fk_mouse(g);
#endif
            e_sys_ini();
            printf(e_msg[ERR_EXEC], filen);
            fflush(stdout);
          }
          if ((*e_u_system)(filen))
          {
            if(!WpeIsXwin())
              e_sys_end();
            e_error(e_msg[ERR_COMMAND], 0, f->fb);
          }
          else if(!WpeIsXwin())
          {
            printf(e_msg[ERR_HITCR]);
            fflush(stderr);
            fflush(stdout);
            fk_getch();
          }
          if(!WpeIsXwin())
          {
            e_sys_end();
            e_close_view(outp, 1);
            fk_cursor(0);
#if MOUSE
            g[0] = 1;
            fk_mouse(g);
#endif
          }
          c = cold;
          break;
        }
        else
        {
          /* if there is only a pattern get back to file selection */
          if(strstr(filen, "*") || strstr(filen, "?"))
          {
            c = AltF;
            break;
          }
          /* there is no open ??? file */
          if(b->sw == 0 || !fe)
          {
            /* close on open request */
            if(f->ed->flopt & FM_CLOSE_WINDOW)
            {
              e_close_window(f);
            }
            /* editing the file */
            e_edit(cn, filen);
          }
          else
          {
            /* try to open the file, no success return */
            if((fp = fopen(filen, "rb")) == NULL)
            {
              e_error(e_msg[ERR_ACCFILE], 0, f->fb);
              c = cold;
              break;
            }
            if(access(filen, 2) != 0)
              f->ins = 8;
            e_close_window(f);
            e_switch_window(winnum, fe);
            fe = cn->f[cn->mxedt];
            be = fe->b;
            se = fe->s;
            f = cn->f[cn->mxedt];
            if(be->b.x != 0)
            {
              e_new_line(be->b.y + 1, be);
              if(*(be->bf[be->b.y].s + be->bf[be->b.y].len) != '\0')
                (be->bf[be->b.y].len)++;
              for(i = be->b.x; i <= be->bf[be->b.y].len; i++)
                *(be->bf[be->b.y + 1].s + i - be->b.x) = *(be->bf[be->b.y].s + i);
              *(be->bf[be->b.y].s + be->b.x) = '\0';
              be->bf[be->b.y].len = be->b.x;
              be->bf[be->b.y + 1].len = e_str_len(be->bf[be->b.y + 1].s);
              be->bf[be->b.y + 1].nrc = e_str_nrc(be->bf[be->b.y + 1].s);
            }
            se->mark_begin.x = be->b.x;
            start = se->mark_begin.y = be->b.y;
            dtmd = fe->dtmd;
            se->mark_end = e_readin(be->b.x, be->b.y, fp, be, &dtmd);
            fclose(fp);

            if(se->mark_begin.x > 0)
              start++;
            len = se->mark_end.y - start;
            e_brk_recalc(f, start, len);

            e_schirm(fe, 1);
          }

          /* if there was no error */
          dirtmp = WpeGetCurrentDir(cn);
          FREE(cn->dirct);
          cn->dirct = dirtmp;

          if (svmode >= 0)
            cn->flopt = svmode;

          if (svdir != NULL)
          {
            /* go back to the saved directory */
            if (chdir(svdir))
            {
              e_error(e_msg[ERR_WORKDIRACCESS], 0, cn->fb);

              /* we cannot determine where we are, try the home */
              if((dirtmp = getenv("HOME")) == NULL)
                e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
              if(chdir(dirtmp))
                e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
            }

            /* determine current dir */
            dirtmp = WpeGetCurrentDir(cn);

            FREE(cn->dirct);
            cn->dirct = dirtmp;

            FREE(svdir);
            svdir = NULL;
          }

          return(0);
        }

      case AltW: /* write activation */
      case AltS: /* save activation */
        if(   (c == AltW && b->sw != 2) 
           || (c == AltS && b->sw != 4)
           || !fe
           || fe->ins == 8 )
        {
          c = cold;
          break;
        }
        /* only file pattern, return */
        if(strstr(filen, "*") || strstr(filen, "?"))
        {
          c = AltF;
          break;
        }
        /* check whether the file exist */
        if(!access(filen, F_OK))
        {
          if((ftmp = MALLOC(strlen(filen) + 42)) == NULL)
            e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

          sprintf(ftmp, "File %s exist\nDo you want to overwrite it ?", filen);
          i = e_message(1, ftmp, f);
          FREE(ftmp);

          if(i == WPE_ESC)
          {
            c = WPE_ESC;
            break;
          }
          else if(i == 'N')
          {
            c = AltF;
            break;
          }
        }
        if(b->sw != 4)
        {
          dtp = fe->dirct;
          ftp = fe->datnam;
        }
        else /* save as mode, current dir and window header will/may change */
        {
          FREE(fe->dirct);
          FREE(fe->datnam);
        }

        WpeFilenameToPathFile(filen, &fe->dirct, &fe->datnam);
        if(b->sw == 4) /* save as mode */
          e_save(fe);
        else
        {
          e_write(se->mark_begin.x, se->mark_begin.y, se->mark_end.x, se->mark_end.y, fe, WPE_BACKUP);
          FREE(fe->dirct);  /* restore current dir window header */
          FREE(fe->datnam);
          fe->dirct = dtp;
          fe->datnam = ftp;
        }

        if(b->sw == 4 && (f->ed->edopt & ED_SYNTAX_HIGHLIGHT))
        {
          if(fe->c_sw)
            FREE(fe->c_sw);
          if(WpeIsProg())
            e_add_synt_tl(fe->datnam, fe);
          if(fe->c_st)
          {
            if(fe->c_sw)
              FREE(fe->c_sw);
            fe->c_sw = e_sc_txt(NULL, fe->b);
          }
          e_rep_win_tree(f->ed);
        }
        if(svmode >= 0)
          f->ed->flopt = svmode;

        if(svdir != NULL)
        {
          /* go back to the saved directory */
          if(chdir(svdir))
          {
            e_error(e_msg[ERR_WORKDIRACCESS], 0, cn->fb);

            /* we cannot determine where we are, try the home */
            if((dirtmp = getenv("HOME")) == NULL)
              e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
            if(chdir(dirtmp))
              e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
          }

          /* determine current dir */
          dirtmp = WpeGetCurrentDir(cn);

          FREE(cn->dirct);
          cn->dirct = dirtmp;

          FREE(svdir);
          svdir = NULL;
        }

        e_close_window(f);
        return(0);

      /* make dir button activation */
      case EINFG:
      case AltK:
        if(b->sw != 0)
        {
          c = cold;
          break;
        }
        /* create new directory */
        if(WpeMakeNewDir(f) != 0)
        {
          c = cold;
          break;
        }

        /* free up old structures */
        freedf(b->dd);
        freedf(b->dw->df);

        /* create new directory structure */
        b->dd = e_find_dir(SUDIR, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
        b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);
        /* go to the line where the new dir is */
        for(i = 0; i < b->dd->anz && strcmp(b->dd->name[i], "new.dir"); i++)
          ;
        /* set the slidebar variables */
        if((b->dw->nf = b->cd->anz + i) >= b->dw->df->anz)
          b->dw->nf = b->cd->anz - 1;
        if(b->dw->nf - b->dw->ia >= b->dw->ye - b->dw->ya)
          b->dw->ia = b->dw->nf + b->dw->ya - b->dw->ye + 1;
        else if(b->dw->nf - b->dw->ia < 0)
          b->dw->ia = b->dw->nf;
        cold = AltT;
        /* let the user modify the newly created dir */
        c = AltM;
        break;

      /* attribute/add file button activation */
      case AltA:
        /* not valid mode */
        if(b->sw != 0 && b->sw != 5)
        {
          c = cold;
          break;
        }
        /* attribute button */
        if(b->sw == 0)
        {
          if(cold == AltF) /* coming from file list */
          {
            strcpy(filen, *(b->df->name + b->fw->nf));  /* alloc for filen ??? */
            /* change the file attributes */
            WpeFileDirAttributes(filen, f);

            /* free up old file list structures */
            freedf(b->df);
            freedf(b->fw->df);

            /* create new file list */
            b->df = e_find_files(b->rdfile, fmode);
            /* setup drawing */
            b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);
          }
          else if(cold == AltT && b->dw->nf >= b->cd->anz) /* coming from dir tree */
          {
            t = b->dw->nf - b->cd->anz;

            if((ftmp = MALLOC(strlen(*(b->dd->name + t)) + 1)) == NULL)
              e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

            strcpy(ftmp, *(b->dd->name + t));
            /* change the dir attributes */
            WpeFileDirAttributes(ftmp, f);

            FREE(ftmp);

            /* free up old file list structures */
            freedf(b->dd);
            freedf(b->dw->df);
            /* create new dir list */
            b->dd = e_find_dir(SUDIR, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
            /* setup drawing */
            b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);
          }
          c = cold;
        }
        else if(b->sw == 5) /* it is in project management */
        {
          FLWND *fw = (FLWND *)cn->f[cn->mxedt - 1]->b;
          if (cold != AltN)
            strcpy(filen, *(b->df->name + b->fw->nf));
          dirtmp = cn->f[cn->mxedt - 1]->dirct;
          ftmp = MALLOC(strlen(f->dirct) + strlen(filen) + 2);
          len = strlen(dirtmp);
          if (strncmp(dirtmp, f->dirct, len) == 0)
          {
           /* Make path relative to project directory */
           sprintf(ftmp, "%s%s", f->dirct + len, filen);
          }
          else
          {
           /* Full path */
           sprintf(ftmp, "%s%s", f->dirct, filen);
          }
          fw->df->anz++;
          fw->df->name = REALLOC(fw->df->name, fw->df->anz * sizeof(char *));
          for(i = fw->df->anz - 1; i > fw->nf; i--)
            fw->df->name[i] = fw->df->name[i - 1];
          fw->df->name[i] = ftmp;
/* Don't bother notifying the user for each file added to project
   sprintf(ftmp, "File added to Project:\n%s",
   fw->df->name[i]);
   e_message(0, ftmp, f); */
          fw->nf++;
          if(fw->nf - fw->ia >= fw->ye - fw->ya)
            fw->ia = fw->nf + fw->ya - fw->ye + 1;
          c = AltN;
        }
        break;

      /* like save */
      case F2:
        c = AltS;
        break;
      /* leave file manager */
      case AltBl:
        c = WPE_ESC;

      default:
        /* not project management */
        if(b->sw != 5)
        {
          if(svmode >= 0)
          {
            /* restore options */
            f->ed->flopt = svmode;
          }

          if(svdir != NULL)
          {
            /* go back to the saved directory */
            if(chdir(svdir))
            {
              e_error(e_msg[ERR_WORKDIRACCESS], 0, cn->fb);

              /* we cannot determine where we are, try the home */
              if((dirtmp = getenv("HOME")) == NULL)
                e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
              if(chdir(dirtmp))
                e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
            }

            /* determine current dir */
            dirtmp = WpeGetCurrentDir(f->ed);
            FREE(f->ed->dirct);
            f->ed->dirct = dirtmp;
          }


          /* the key dispatcher returns zero when something has happened
             and this means that the file-manager lost focus, just return */
#ifdef PROG
          if(!e_tst_dfkt(f, c) || (WpeIsProg() && !e_prog_switch(f, c)))
#else
          if(!e_tst_dfkt(f, c))
#endif
          {
            if(svdir != NULL)
            {
              FREE(svdir);
              svdir = NULL;
            }

            return(0);
          }
          /* file manager is still active */
          if(svmode >= 0)
            f->ed->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
              FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
        }
        else if(   c == WPE_ESC
                || (!(f->ed->edopt & ED_CUA_STYLE) && c == AF3)
                || (f->ed->edopt & ED_CUA_STYLE && c == CF4))
        {
          if(svdir != NULL)
          {
            /* go back to the saved directory */
            if(chdir(svdir))
            {
              e_error(e_msg[ERR_WORKDIRACCESS], 0, cn->fb);

              /* we cannot determine where we are, try the home */
              if((dirtmp = getenv("HOME")) == NULL)
                e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
              if(chdir(dirtmp))
                e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
            }

            /* determine current dir */
            dirtmp = WpeGetCurrentDir(cn);

            FREE(cn->dirct);
            cn->dirct = dirtmp;

            FREE(svdir);
            svdir = NULL;
          }

          /* close file manager */
          e_close_window(f);

          /* restore options */
          if(svmode >= 0)
            cn->flopt = svmode;

          return(WPE_ESC);
        }
        c = cold;
        break;
    }
  }

  /* change to saved directory and lets hope it works */
  if(svdir != NULL)
  {
    /* go back to the saved directory */
    if(chdir(svdir))
    {
      e_error(e_msg[ERR_WORKDIRACCESS], 0, cn->fb);

      /* we cannot determine where we are, try the home */
      if((dirtmp = getenv("HOME")) == NULL)
        e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
      if(chdir(dirtmp))
        e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
    }

    /* determine current dir */
    dirtmp = WpeGetCurrentDir(cn);

    FREE(cn->dirct);
    cn->dirct = dirtmp;

    FREE(svdir);
    svdir = NULL;
  }

  /* restore options */
  if(svmode >= 0)
    cn->flopt = svmode;

  /* if in save mode or project management */
  if(f->save == 1 || b->sw == 5)
  {
    e_close_window(f);
    return(WPE_ESC);
  }
  else
  {
    e_close_window(f);
    return(0);
  }
}


int WpeGrepFile(char *file, char *string, int sw)
{
  FILE           *fp;
  char            str[256];
  int             ret, nn;

  if((fp = fopen(file, "r")) == NULL)
    return(0);

  nn = strlen(string);
  while(fgets(str, 256, fp))
  {
    if((sw & 32) == 0)
    {
      if((sw & 128) != 0)
        ret = e_strstr(0, strlen(str), str, string);
      else
        ret = e_ustrstr(0, strlen(str), str, string);
    }
    else
    {
      if((sw & 128) != 0)
        ret = e_rstrstr(0, strlen(str), str, string, &nn);
      else
        ret = e_urstrstr(0, strlen(str), str, string, &nn);
    }
    if(   ret >= 0 
       && (   !(sw & 64) 
           || (   isalnum(str[ret + nn]) == 0
               && (ret == 0 || isalnum(str[ret - 1]) == 0))))
    {
      fclose(fp);
      return(1);
    }
  }
  fclose(fp);
  return(0);
}

int WpeMakeNewDir(FENSTER *f)
{
  char           *dirct;
  int             msk, mode, ret;
  struct stat     buf;

  umask(msk = umask(077));
  mode = 0777 & ~msk;

  if((dirct = MALLOC(strlen(f->dirct) + 9)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

  if(f->dirct[strlen(f->dirct) - 1] != DIRC)
    sprintf(dirct, "%s/new.dir", f->dirct);
  else
    sprintf(dirct, "%snew.dir", f->dirct);

  /* check existence and status (whether it is directory) */
  if(stat(dirct, &buf))
  {
    if((ret = mkdir(dirct, mode)) != 0)
      e_error(e_msg[ERR_NONEWDIR], 0, f->ed->fb);
  }
  else
  {
    /* check whether the existing file is a directory */
    if(!(buf.st_mode & S_IFDIR))
    {
      e_error(e_msg[ERR_NEWDIREXIST], 0, f->ed->fb);
      ret = 1;
    }
    else
      ret = 0;
  }

  FREE(dirct);
  return(ret);
}

int WpeFileDirAttributes(char *filen, FENSTER * f)
{
  struct stat     buf[1];
  int             mode, ret;
  W_OPTSTR       *o = e_init_opt_kst(f);

  /* if cannot access or error */
  if(stat(filen, buf))
  {
    e_error(e_msg[ERR_ACCFILE], 0, f->ed->fb);
    return 1;
  }
  mode = buf->st_mode;
  if(o == NULL)
    return(1);
  o->xa = 14;
  o->ya = 4;
  o->xe = 62;
  o->ye = 13;
  o->bgsw = AltO;
  o->name = "Attributes";
  o->crsw = AltO;
  e_add_txtstr(3, 2, "User:", o);
  e_add_txtstr(33, 2, "Other:", o);
  e_add_txtstr(18, 2, "Group:", o);
  e_add_sswstr(4, 3, 0, AltR, mode & 256 ? 1 : 0, "Read   ", o);
  e_add_sswstr(4, 4, 0, AltW, mode & 128 ? 1 : 0, "Write  ", o);
  e_add_sswstr(4, 5, 1, AltX, mode & 64 ? 1 : 0, "EXecute", o);
  e_add_sswstr(19, 3, 2, AltA, mode & 32 ? 1 : 0, "ReAd   ", o);
  e_add_sswstr(19, 4, 2, AltI, mode & 16 ? 1 : 0, "WrIte  ", o);
  e_add_sswstr(19, 5, 0, AltE, mode & 8 ? 1 : 0, "Execute", o);
  e_add_sswstr(34, 3, 3, AltD, mode & 4 ? 1 : 0, "ReaD   ", o);
  e_add_sswstr(34, 4, 3, AltT, mode & 2 ? 1 : 0, "WriTe  ", o);
  e_add_sswstr(34, 5, 4, AltU, mode & 1, "ExecUte", o);
  e_add_bttstr(12, 7, 1, AltO, " Ok ", NULL, o);
  e_add_bttstr(29, 7, -1, WPE_ESC, "Cancel", NULL, o);
  ret = e_opt_kst(o);
  if(ret != WPE_ESC)
  {
    mode = (o->sstr[0]->num << 8) + (o->sstr[1]->num << 7) +
      (o->sstr[2]->num << 6) + (o->sstr[3]->num << 5) +
      (o->sstr[4]->num << 4) + (o->sstr[5]->num << 3) +
      (o->sstr[6]->num << 2) + (o->sstr[7]->num << 1) +
      o->sstr[8]->num;
    if(chmod(filen, mode))
    {
      e_error(e_msg[ERR_CHGPERM], 0, f->ed->fb);
      freeostr(o);
      return(1);
    }
  }
  freeostr(o);
  return(0);
}

/* determines the current working directory,
   it returns the string
   in case of an error - if the directory cannot be accessed try to access HOME,
                         if it is impossible, give it up and exit with "system error"
                       - otherwise exit with "system error"
 */
char *WpeGetCurrentDir(ECNT *cn)
{
  int allocate_size;
  char *current_dir = NULL, *check_dir, *dirtmp;
  short home;

  home = 0;
  allocate_size = 256;
  if ((current_dir = (char *)WpeMalloc(allocate_size + 1)) == NULL)
  {
   e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
   return NULL;
  }

  do
  {
    /* allocate space for the directory name */
    if ((current_dir = (char *)WpeRealloc(current_dir, allocate_size + 1)) == NULL)
    {
     e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
     return NULL;
    }

    check_dir = getcwd(current_dir, allocate_size);
    if(check_dir == NULL)
    {
      switch(errno)
      {
        case EACCES:  /* directory cannot be read */
          if (home == 0)
          {
            e_error(e_msg[ERR_WORKDIRACCESS], 0, cn->fb);

            /* we cannot determine where we are, try the home */
            if ((dirtmp = getenv("HOME")) == NULL)
              e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);
            if (chdir(dirtmp))
              e_error(e_msg[ERR_HOMEDIRACCESS], 1, cn->fb);

            home = 1;
          }
          else
            e_error(e_msg[ERR_SYSTEM], 1, cn->fb); /* will not return */
          break;
        case EINVAL:  /* size is equal to 0 */
          allocate_size = 256; /* impossible !!! */
          break;
        case ERANGE:  /* not enough space for the pathname */
          allocate_size <<= 1;
          break;
        default:      /* System error */
          e_error(e_msg[ERR_SYSTEM], 1, cn->fb); /* will not return */
          break;
      }
    }
  } while (check_dir == NULL);

  if (current_dir[strlen(current_dir) - 1] != DIRC)
   strcat(current_dir, DIRS);
  return(current_dir);
}

/* It always returns the assembled file path, except
   when it is in wastebasket mode and there is an error, return NULL */
char *WpeAssemblePath(char *pth, struct dirfile *cd, struct dirfile *dd, int n, 
                      FENSTER *f)
{
  int             i = 0, k = 0, j = 0, t;
  char           *adir= NULL;  /* assembled directory */
  int             totall = 0;

#ifdef UNIX
  if (!strcmp(cd->name[0], "Wastebasket"))
  {
    if ((adir = WpeGetWastefile("")))
    {
      totall = strlen(adir) + 16;
      if ((adir = REALLOC(adir, totall)) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, f->fb);

      strcat(adir, DIRS);
      i = strlen(adir);
      k++;
    }
    else
    {
      /* Error failed to find wastebasket */
      e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
      return NULL;
    }
  }
#endif
  for(; k <= n && k < cd->anz; i++, j++)
  {
    if(i >= totall-1)
    {
      totall += 16;
      if((adir = REALLOC(adir, totall)) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, f->fb);
    }
    *(adir + i) = *(*(cd->name + k) + j);

    if(*(adir + i) == '\0' || *(adir + i) == DIRC)
    {
      *(adir + i) = DIRC;
      k++;
      j = -1;
    }
  }
  if(n >= k)
  {
    t = n - cd->anz;
    j = 0;

    do
    {
      if(i >= totall-2)
      {
        totall += 16;
        if((adir = REALLOC(adir, totall)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->fb);
      }
      *(adir + i) = *(*(dd->name + t) + j);
      i++;
      j++;
    }
    while(*(adir + i - 1) != '\0');

/*
    for(j = 0; (*(pth + i) = *(*(dd->name + t) + j)) != '\0'; i++, j++)
      ;
*/

  }
  if(n == 0 || n >= k)
    i++;
  *(adir + i - 1) = '\0';
  return(adir);
}


/* create tree structure up to working directory */
struct dirfile *WpeCreateWorkingDirTree(int sw, ECNT *cn)
{
  struct dirfile *df;
  char           *buf;
  char           *tmp, *tmp2;
  char          **dftmp;
  int             buflen = 256;
  int             maxd = 10;    /* inital number of directory levels */
  int             i, j, k;

  buf = WpeGetCurrentDir(cn);

  buflen = strlen(buf);
  if((tmp = MALLOC(buflen+1)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  /* initialise directory list */
  if(   ((df = MALLOC(sizeof(struct dirfile))) == NULL)
     || ((df->name = MALLOC(sizeof(char *) * maxd)) == NULL) )
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  df->anz = 0;

  if(sw == 1)                        /* file-manager open to wastebasket */
  {
    /* instead of saving the real wastebasket dir into the structure
       the "Wastebasket" name will appear */
    if((tmp2 = WpeGetWastefile("")) == NULL)
    {
      e_error(e_msg[ERR_NOWASTE], 0, cn->fb);    /* more error check ??? */
      i = 0;
    }
    else
    {
      i = strlen(tmp2);
      /* increase the level in the dir tree */
      df->anz = 1;
      /* save the name into first level */
      if((*(df->name) = MALLOC(12 * sizeof(char))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
      strcpy(*(df->name), "Wastebasket");

      if(!strncmp(tmp2, buf, i) && buf[i])
      {
        FREE(tmp2);
        i++;
      }
      else
      {
        FREE(tmp2);
        FREE(buf);
        FREE(tmp);
        return(df);
      }
    }
  }
  else
  {
    i = 0;
  }


  for(j = 0; i <= buflen; i++, j++)
  {
    tmp[j] = buf[i];
    /* if directory separator or end of string */
    if(tmp[j] == DIRC || tmp[j] == '\0')
    {
      if(buf[i] == '\0' && j == 0)
        return(df);
      if(df->anz == 0)  /* for the very first time save the '/' sign */
        j++;
      tmp[j] = '\0';

      /* if we need more space for the directories */
      if(df->anz >= maxd)
      {
        maxd += 10;
        dftmp = df->name;
        if((df->name = MALLOC(sizeof(char *) * maxd)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
        for(k = 0; k < maxd - 10; k++)
          *(df->name + k) = *(dftmp + k);
        FREE(dftmp);
      }
      /* save the current directory */
      if((*(df->name + df->anz) = MALLOC((strlen(tmp) + 1) * sizeof(char))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
      strcpy(*(df->name + df->anz), tmp);
      df->anz++;
      j = -1;
    }
  }
  FREE(buf);
  FREE(tmp);
  return(df);
}

/* This function creates a wastebasket directory if needed, then returns a
   string made up of the last filename in the string pointed to by 'file' and
   the wastebasket path.  It returns NULL if there is a memory allocation
   error or the pointer to 'path' otherwise.  The original function did no
   error checking on wastebasket path creation.  The modified version does --
   it also returns NULL on an error in the chdir function or mkdir
   function. */
char  *WpeGetWastefile(char *file)
{
  static char    *wastebasket = NULL;
  int             i, lw;
  char           *tmp;             /* storage for wastebasket path */
  char           *tmp2;

  if(!wastebasket)  /* setup and test access to wastebasket directory */
  {
    if((tmp2 = getenv("HOME")) == NULL)
      return NULL;

    lw = strlen(tmp2) + 1 + strlen(WASTEBASKET) + 1;
    /*    HOME          /     WASTEBASKET       \0    */
    if((tmp = (char *)MALLOC(lw)) == NULL)
      return NULL;

    sprintf(tmp, "%s/%s", tmp2, WASTEBASKET);

    /* if wastebasket dir does not exist, create it with error checking */
    if(access(tmp, F_OK))
    {
      if(mkdir(tmp, 0700))
      {
        FREE(tmp);
        return NULL;
      }
    }
    /* check for wastebasket's permissions */
    if(access(tmp, R_OK | W_OK | X_OK))
    {
      FREE(tmp);
      return NULL;
    }
    wastebasket = tmp;
  }

  /* verify that wastebasket directory still exists before proceeding */
  if(access(wastebasket, F_OK | R_OK | W_OK | X_OK))
  {
    /* try to recreate it */
    if(mkdir(wastebasket, 0700))
      return NULL;
  }

  /* return wastebasket directory path if no filename in 'file' */
  if(file[0] == '\0')
  {
    if((tmp2 = MALLOC(strlen(wastebasket)+1)) == NULL)
      return NULL;
    strcpy(tmp2, wastebasket);
    return(tmp2);
  }

  /* else get filename from end of 'file' string, 
     append to wastebasket sting */
  for(i = strlen(file) - 1; i >= 0 && file[i] != DIRC; i--)
    ;
  if((tmp2 = MALLOC(strlen(wastebasket) + strlen(file + 1 + i) + 2)) == NULL)
    return NULL;

  sprintf(tmp2, "%s/%s", wastebasket, file + 1 + i);
  return(tmp2);
}

struct dirfile *WpeGraphicalFileList(struct dirfile *df, int sw, ECNT *cn)
{
 struct dirfile *edf;
 char          **name, **ename, *stmp, str[256];
 int             ntmp, n, i, *num;

 /* allocate the same structure as the argument */
 if ((edf = MALLOC(sizeof(struct dirfile))) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

 edf->anz = df->anz;
 edf->name = NULL;

/* OSF and AIX fix, for malloc(0) they return NULL */
 if (df->anz)
 {
  if ((edf->name = MALLOC(df->anz * sizeof(char *))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  if ((num = MALLOC(df->anz * sizeof(int))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  for (i = 0; i < df->anz; i++)
  {
   e_file_info(*(df->name + i), str, num + i, sw);
   if ((*(edf->name + i) = MALLOC(strlen(str) + 1)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
   strcpy(*(edf->name + i), str);
  }

  /* sort by time or size mode */
  if (sw & 3)
  {
   if ((ename = MALLOC(df->anz * sizeof(char *))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

   if ((name = MALLOC(df->anz * sizeof(char *))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

   for (i = 0; i < df->anz; i++)
   {
    for (ntmp = num[i], n = i; n > 0 && ntmp > num[n - 1]; n--)
    {
     *(ename + n) = *(ename + n - 1);
     *(name + n) = *(name + n - 1);
     num[n] = num[n - 1];
    }
    *(ename + n) = *(edf->name + i);
    *(name + n) = *(df->name + i);
    num[n] = ntmp;
   }
   FREE(edf->name);
   FREE(df->name);
   edf->name = ename;
   df->name = name;
  }

  FREE(num);

  /* reverse order */
  if (sw & 4)
  {
   for (i = 0; i < (df->anz) / 2; i++)
   {
    stmp = edf->name[i];
    edf->name[i] = edf->name[edf->anz - i - 1];
    edf->name[edf->anz - i - 1] = stmp;
    stmp = df->name[i];
    df->name[i] = df->name[df->anz - i - 1];
    df->name[df->anz - i - 1] = stmp;
   }
  }
 }

 return(edf);
}

struct dirfile *WpeGraphicalDirTree(struct dirfile *cd, struct dirfile *dd, ECNT *cn)
{
  extern char    *ctree[5];
  struct dirfile *edf;
  char            str[256];
  int             i = 0, j;

  if((edf = MALLOC(sizeof(struct dirfile))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

  /* for the OSF and AIX this should never be zero, we are always somewhere */
  if(cd->anz + dd->anz > 0)
  {
    if((edf->name = MALLOC((cd->anz + dd->anz) * sizeof(char *))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, cn->fb);

    for(i = 0; i < cd->anz; i++)
    {
      if(!i)
#ifndef DJGPP
      {
        if(*cd->name[0] == DIRC && *(cd->name[0] + 1) == '\0')
          strcpy(str, "Root");
        else
          strcpy(str, *(cd->name + i));
#else
      {
        strcpy(str, *(cd->name + i));
#endif
      }
      else
      {
        for(str[0] = '\0', j = 0; j < i - 1; j++)
          strcat(str, "  ");
        if(i == cd->anz - 1 && dd->anz < 1)
          strcat(str, ctree[1]);
        else if(i == cd->anz - 1)
          strcat(str, ctree[2]);
        else
          strcat(str, ctree[0]);
        strcat(str, *(cd->name + i));
      }
      if((*(edf->name + i) = MALLOC((strlen(str) + 1) * sizeof(char))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
      strcpy(*(edf->name + i), str);
    }

    for(; i < cd->anz + dd->anz; i++)
    {
      for(str[0] = '\0', j = 0; j < cd->anz - 2; j++)
        strcat(str, "  ");
      strcat(str, " ");
      if(i == cd->anz + dd->anz - 1)
        strcat(str, ctree[4]);
      else
        strcat(str, ctree[3]);
      {
        int             tttt = i - cd->anz;
        strcat(str, *(dd->name + tttt));
      }
      if((*(edf->name + i) = MALLOC((strlen(str) + 1) * sizeof(char))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
      strcpy(*(edf->name + i), str);
    }
  }

  edf->anz = i;
  return(edf);
}

int WpeDelWastebasket(FENSTER *f)
{
  char            *tmp;
  int             ret, mode = f->ed->flopt;

  WpeMouseChangeShape(WpeWorkingShape);
  f->ed->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                 FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
  if ((tmp = WpeGetWastefile("")))
  {
    ret = WpeRemoveDir(tmp, "*", f, 0);
    FREE(tmp);

    /* Unfortunately there is this racing condition, so
       this is necessary to get back the deleted wastebasket. */
    if ((tmp = WpeGetWastefile("")))
    {
      FREE(tmp);
      ret = 0;
    }
    else
    {
      e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
      ret = 1;
    }
  }
  else
  {
    /* Error failed to find wastebasket */
    e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
    ret = 1;
  }
  f->ed->flopt = mode;
  WpeMouseRestoreShape();
  return(ret);
}

int WpeShowWastebasket(FENSTER *f)
{
  return(WpeCallFileManager(6, f));
}

int WpeQuitWastebasket(FENSTER * f)
{
  char            *tmp;
  int             ret = 0, mode = f->ed->flopt;

  if(mode & FM_PROMPT_DELETE)
    f->ed->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                   FM_REMOVE_PROMPT | FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
  else if(mode & FM_DELETE_AT_EXIT)
    f->ed->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                   FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;

  if((mode & FM_PROMPT_DELETE) || (mode & FM_DELETE_AT_EXIT))
  {
    WpeMouseChangeShape(WpeWorkingShape);
    if ((tmp = WpeGetWastefile("")))
    {
      ret = WpeRemoveDir(tmp, "*", f, 0);
      FREE(tmp);
    }
    else
    {
      /* Error failed to find wastebasket */
      e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
      ret = 0;
    }
    WpeMouseRestoreShape();
  }

  f->ed->flopt = mode;
  return(ret);
}


int WpeRemoveDir(char *dirct, char *file, FENSTER * f, int rec)
{
  PIC            *pic = NULL;
  char           *tmp;
  int             i, ret, svmode = f->ed->flopt;
  struct dirfile *dd;

  if(rec > MAXREC)
    return(0);

  /* only copy it to the wastebasket */
  if(f->ed->flopt & FM_REMOVE_INTO_WB)
  {
    if ((tmp = WpeGetWastefile(dirct)))
    {
      i = strlen(tmp);
      /* we should not use the fact, that the wasbasket is always something
         meaningful !!! */
      if(strncmp(tmp, dirct, i))
      {
        ret = WpeRenameCopyDir(dirct, file, tmp, f, 0, 0);
        FREE(tmp);
        return(ret);
      }
      FREE(tmp);
    }
    else
    {
      e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
      return 1;
    }
  }

  if((tmp = MALLOC(strlen(dirct) + strlen(file)+2)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

  /* search for files in the directory */
  sprintf(tmp, "%s%c%s", dirct, DIRC, file);
  dd = e_find_files(tmp, f->ed->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0);

  /* it is called for the first time and the user should be asked about
     the deletion */
  if(!rec && (f->ed->flopt & FM_REMOVE_PROMPT) && dd->anz > 0)
  {
    if((ret = WpeDirDelOptions(f)) < 0)
    {
      freedf(dd);
      FREE(tmp);
      return(ret == WPE_ESC ? 1 : 0);
    }
    if(ret)
      f->ed->flopt |= FM_REMOVE_PROMPT;
    else
      f->ed->flopt &= ~FM_REMOVE_PROMPT;
    rec = -1;
  }
  FREE(tmp);

  /* cleans up the files in the directory */
  for(i = 0; i < dd->anz; i++)
  {
    if((tmp = MALLOC(strlen(dirct) + strlen(dd->name[i])+15)) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    sprintf(tmp, "Remove File:\n%s%c%s", dirct, DIRC, dd->name[i]);
    if(f->ed->flopt & FM_REMOVE_PROMPT)
    {
      ret = e_message(1, tmp, f);
    }
    else
      ret = 'Y';
    if(ret == WPE_ESC)
    {
      freedf(dd);
      FREE(tmp);
      f->ed->flopt = svmode;
      return(1);
    }
    else if(ret == 'Y')
    {
      /* this should definitely fit in */
      sprintf(tmp, "%s%c%s", dirct, DIRC, dd->name[i]);

      /* put message out */
      if(e_mess_win("Remove", tmp, &pic, f))
      {
        FREE(tmp);
        break;
      }

      if(pic)
      {
        e_close_view(pic, 1);
        pic = NULL;
      }

      /* try to remove it */
      if(remove(tmp))
      {
        e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);
        freedf(dd);
        FREE(tmp);
        f->ed->flopt = svmode;
        return(1);
      }
    }
    FREE(tmp);
  }

  if(pic)
    e_close_view(pic, 1);
  freedf(dd);

  /* if recursive action is specified clean up the 
     subdirectories as well */
  if(f->ed->flopt & FM_REKURSIVE_ACTIONS)
  {
    if((tmp = MALLOC(strlen(dirct) + strlen(SUDIR)+2)) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    /* search for subdirectories */
    sprintf(tmp, "%s%c%s", dirct, DIRC, SUDIR);
    dd = e_find_dir(tmp, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);

    /* should the user be asked about deletion ? */
    if(!rec && (f->ed->flopt & FM_REMOVE_PROMPT) && dd->anz > 0)
    {
      if((ret = WpeDirDelOptions(f)) < 0)
      {
        freedf(dd);
        FREE(tmp);
        return(ret == WPE_ESC ? 1 : 0);
      }
      if(ret)
        f->ed->flopt |= FM_REMOVE_PROMPT;
      else
        f->ed->flopt &= ~FM_REMOVE_PROMPT;
    }
    else if(rec < 0)
      rec = 0;

    FREE(tmp);

    /* call recursively itself to delete the subdirectories */
    for(rec++, i = 0; i < dd->anz; i++)
    {
      if((tmp = MALLOC(strlen(dirct) + strlen(dd->name[i])+2)) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

      sprintf(tmp, "%s%c%s", dirct, DIRC, dd->name[i]);
      if(WpeRemoveDir(tmp, file, f, rec))
      {
        freedf(dd);
        FREE(tmp);
        f->ed->flopt = svmode;
        return(1);
      }
      FREE(tmp);
    }
    freedf(dd);
  }

  f->ed->flopt = svmode;

  /* remove finally the directory itself */
  if(rmdir(dirct))
  {
    e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);
    return(1);
  }
  else
    return(0);
}

int WpeRemove(char *file, FENSTER * f)
{
  struct stat     buf;
  struct stat     lbuf;
  char           *tmp2;
  int             ret;

  if(lstat(file, &lbuf))
  {
    e_error(e_msg[ERR_ACCFILE], 0, f->ed->fb);
    return 1;
  }

  WpeMouseChangeShape(WpeWorkingShape);

  /* this is important, first we check whether it is a file,
     this check works even when it is a pointer, if it is not
     a file, then it can be an "invalid" symbolic link, check for that */
  if(((stat(file, &buf) == 0) && S_ISREG(buf.st_mode)) || S_ISLNK(lbuf.st_mode))
  {
    if((f->ed->flopt & FM_REMOVE_INTO_WB))
    {
      if ((tmp2 = WpeGetWastefile(file)))
      {
        ret = strlen(tmp2);
        if(strncmp(tmp2, file, ret))
        {
          e_rename(file, tmp2, f);
        }
        FREE(tmp2);
        ret = 0;
      }
      else
      {
        e_error(e_msg[ERR_NOWASTE], 0, f->ed->fb);
        ret = 1;
      }
    }
    else
    {
      if(f->ed->flopt & FM_REMOVE_PROMPT)
      {
        if((tmp2 = MALLOC(strlen(file) + 14)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

        sprintf(tmp2, "Remove File:\n%s", file);
        ret = e_message(1, tmp2, f);
        FREE(tmp2);
      }
      else
        ret = 'Y';

      if(ret == 'Y')
      {
        if(remove(file))
        {
          e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);
          ret = 1;
        }
        else
          ret = 0;
      }
      else
        ret = 0;
    }
  }
  else
  {
    ret = WpeRemoveDir(file, f->fd.file, f, 0);
  }

  WpeMouseRestoreShape();
  return(ret);
}

/* sw = 0  -> renames the directory
   sw = 1  -> copies the directory
   sw = 2  -> links directory */
int WpeRenameCopyDir(char *dirct, char *file, char *newname, FENSTER *f,
                     int rec, int sw)
{
  char            *tmp, *ntmp, *mtmp;
  int             i, ret, mode;
  struct dirfile *dd;
  struct stat     buf;
  PIC            *pic = NULL;

  if(rec > MAXREC)
    return(0);

  /* check whether the dir already exist */
  ret = access(newname, F_OK);
  /* rename mode and dir does not exist */
  if (sw == 0 && ret && file[0] == '*' && file[1] == '\0')
  {
    if ((ret = rename(dirct, newname)))
      e_error(e_msg[ERR_RENFILE], 0, f->ed->fb);
    return(ret);
  }

  /* directory does not exist */
  if (ret != 0)
  {
    /* get the permissions */
    if (stat(dirct, &buf))
    {
      e_error(e_msg[ERR_ACCFILE], 0, f->ed->fb);
      return(1);
    }
    /* with that permission create the new dir */
    if(mkdir(newname, buf.st_mode))
    {
      e_error(e_msg[ERR_NONEWDIR], 0, f->ed->fb);
      return(1);
    }
  }

  if(f->ed->flopt & FM_REKURSIVE_ACTIONS)
  {
    if((tmp = MALLOC(strlen(dirct) + 2 + strlen(SUDIR))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    sprintf(tmp, "%s%c%s", dirct, DIRC, SUDIR);
    dd = e_find_dir(tmp, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
    FREE(tmp);

    for(rec++, i = 0; i < dd->anz; i++)
    {

      if((tmp = MALLOC(strlen(dirct) + 2 + strlen(dd->name[i]))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

      if((ntmp = MALLOC(strlen(newname) + 2 + strlen(dd->name[i]))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

      sprintf(tmp, "%s%c%s", dirct, DIRC, dd->name[i]);
      sprintf(ntmp, "%s%c%s", newname, DIRC, dd->name[i]);

      if(WpeRenameCopyDir(tmp, file, ntmp, f, rec, sw))
      {
        FREE(tmp);
        FREE(ntmp);
        freedf(dd);
        return(1);
      }
      FREE(tmp);
      FREE(ntmp);
    }
    freedf(dd);
  }

  if((tmp = MALLOC(strlen(dirct) + 2 + strlen(file))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);
  sprintf(tmp, "%s%c%s", dirct, DIRC, file);
  dd = e_find_files(tmp, f->ed->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0);
  FREE(tmp);

  mode = f->ed->flopt;
  f->ed->flopt &= ~FM_REMOVE_PROMPT;

  for(i = 0; i < dd->anz; i++)
  {
    if((ntmp = MALLOC(strlen(newname) + 2 + strlen(dd->name[i]))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    sprintf(ntmp, "%s%c%s", newname, DIRC, dd->name[i]);
    ret = 'Y';

    if(access(ntmp, 0) == 0)
    {
      if(f->ed->flopt & FM_MOVE_PROMPT)
      {
        if((tmp = MALLOC(strlen(ntmp) + 31)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

        sprintf(tmp, "File %s exist !\nOverwrite File ?", ntmp);
        if(pic)
        {
          e_close_view(pic, 1);
          pic = NULL;
        }
        ret = e_message(1, tmp, f);
        FREE(tmp);

        if(ret == 'Y')
        {
          if(WpeRemove(ntmp, f))
          {
            FREE(ntmp);
            freedf(dd);
            return(1);
          }
        }
        else if(ret == WPE_ESC)
        {
          FREE(ntmp);
          freedf(dd);
          return(1);
        }
      }
      else if(f->ed->flopt & FM_MOVE_OVERWRITE)
      {
        if(WpeRemove(ntmp, f))
        {
          FREE(ntmp);
          freedf(dd);
          return(1);
        }
      }
    }

    if((tmp = MALLOC(strlen(dirct) + 2 + strlen(dd->name[i]))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    sprintf(tmp, "%s%c%s", dirct, DIRC, dd->name[i]);

    if(ret == 'Y')
    {
      if((mtmp = MALLOC(strlen(tmp) + 2 + strlen(ntmp))) == NULL)
        e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

      sprintf(mtmp, "%s %s", tmp, ntmp);
      if(e_mess_win(!sw ? "Rename" : "Copy", mtmp, &pic, f))
      {
        FREE(tmp);
        FREE(ntmp);
        FREE(mtmp);
        break;
      }
      FREE(mtmp);

      if (sw == 0)
      {
        if ((ret = rename(tmp, ntmp)))
        {
          e_error(e_msg[ERR_RENFILE], 0, f->ed->fb);
          FREE(tmp);
          FREE(ntmp);
          freedf(dd);
          return(1);
        }
      }
      else if (sw == 1)
      {
        if (WpeCopyFileCont(tmp, ntmp, f))
        {
          FREE(tmp);
          FREE(ntmp);
          freedf(dd);
          return(1);
        }
      }
      else if(sw == 2)
      {
        if(WpeLinkFile(tmp, ntmp, f->ed->flopt & FM_TRY_HARDLINK, f))
        {
          FREE(tmp);
          FREE(ntmp);
          freedf(dd);
          return(1);
        }
      }
    }

    FREE(tmp);
    FREE(ntmp);
  }

  if(pic)
    e_close_view(pic, 1);

  f->ed->flopt = mode;

  if(sw == 0)
  {
    if(rmdir(dirct))
      e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);
  }

  freedf(dd);
  return(0);
}


int WpeRenameCopy(char *file, char *newname, FENSTER *f, int sw)
{
 struct stat     buf;
 struct stat     lbuf;
 char           *tmp, *tmpl;
 int             ln = -1, ret = 'Y';
 int             allocate_size, retl = 0;

 WpeMouseChangeShape(WpeWorkingShape);

 /* in copy mode check whether it is a link */
 if (sw == 0)
 {
  if (lstat(file, &lbuf))
  {
   e_error(e_msg[ERR_ACCFILE], 0, f->ed->fb);
   return 1;
  }

  if (S_ISLNK(lbuf.st_mode))
   ln = 1;
 }

 if ((stat(file, &buf) == 0) && S_ISDIR(buf.st_mode) && ln < 0)
  retl = WpeRenameCopyDir(file, f->fd.file, newname, f, 0, sw);
 else
 {
  /* check whether file exist */
  if (access(newname, 0) == 0)
  {
   if (f->ed->flopt & FM_MOVE_OVERWRITE)
   {
    if (WpeRemove(newname, f))
    {
     WpeMouseRestoreShape();
     return(1);
    }
   }
   else if (f->ed->flopt & FM_MOVE_PROMPT)
   {
    if ((tmp = MALLOC(strlen(newname) + 26)) == NULL)
     e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);
    sprintf(tmp, "File %s exist\nRemove File ?", newname);
    ret = e_message(1, tmp, f);
    FREE(tmp);
    if (ret == 'Y')
    {
     if (WpeRemove(newname, f))
     {
      WpeMouseRestoreShape();
      return(1);
     }
    }
   }
  }

  if (ret == 'Y')
  {
   if (sw == 1)
   {
    retl = WpeCopyFileCont(file, newname, f);
   }
   else if (sw == 2)
   {
    retl = WpeLinkFile(file, newname, f->ed->flopt & FM_TRY_HARDLINK, f);
   }
   else if (sw == 0 && ln < 0) /* rename mode, no softlink */
   {
    if ((retl = rename(file, newname)) == -1)
    {
     if (errno == EXDEV)
     {
      if ((retl = WpeCopyFileCont(file, newname, f)) == 0)
      {
       if ((retl = remove(file)))
        e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);
      }
     }
     else
     {
      e_error(e_msg[ERR_RENFILE], 0, f->ed->fb);
      retl = 1;
     }
    }
   }
   else if (sw == 0)
   {
    allocate_size = 2;
    tmp = NULL;
    do
    {
     if (tmp)
      FREE(tmp);
     allocate_size += 4;
     if ((tmp = MALLOC(allocate_size)) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

      ln = readlink(file, tmp, allocate_size-1);
    } while (!(ln < allocate_size-1));
    tmp[ln] = '\0';

    for (; ln >= 0 && tmp[ln] != DIRC; ln--)
     ;
    if (ln < 0)
    {
     if ((tmpl = MALLOC(strlen(f->dirct) + 2 + strlen(tmp))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

     sprintf(tmpl, "%s%c%s", f->dirct, DIRC, tmp);
     retl = WpeRenameLink(file, newname, tmpl, f);
     FREE(tmpl);
    }
    else
    {
     retl = WpeRenameLink(file, newname, tmp, f);
    }
   }
  }
 }
 WpeMouseRestoreShape();
 return(retl);
}

int WpeCopyFileCont(char *oldfile, char *newfile, FENSTER *f)
{
  struct stat     buf;
  int             ret;
  char           *buffer;
  FILE           *fpo, *fpn;

  /* get the status of the file */
  if(stat(oldfile, &buf))
  {
    e_error(e_msg[ERR_ACCFILE], 0, f->ed->fb);
    return(1);
  }

  /* open files for copying */
  if((fpo = fopen(oldfile, "rb")) == NULL)
  {
    e_error(e_msg[ERR_OREADFILE], 0, f->ed->fb);
    return(1);
  }
  if((fpn = fopen(newfile, "wb")) == NULL)
  {
    e_error(e_msg[ERR_OWRITEFILE], 0, f->ed->fb);
    return(1);
  }

  /* allocate buffer for copying */
  if((buffer = malloc(E_C_BUFFERSIZE)) == NULL)
  {
    e_error(e_msg[ERR_ALLOC_CBUF], 0, f->ed->fb);
    return(1);
  }

  /* copy until end of file */
  do
  {
    ret = fread(buffer, 1, E_C_BUFFERSIZE, fpo);
    /* we should be able to write the same amount of info */
    if (fwrite(buffer, 1, ret, fpn) != ret)
    {
      fclose(fpo);
      fclose(fpn);
      free(buffer);
      e_error(e_msg[ERR_INCONSCOPY], 0, f->ed->fb);
      return(1);
    }
  } while(!feof(fpo));

  fclose(fpo);
  fclose(fpn);
  free(buffer);

  /* Well, we just created the file, so theoretically this 
     should succed. Of course this is a racing condition !!! */
  if(chmod(newfile, buf.st_mode))
    e_error(e_msg[ERR_CHGPERM], 0, f->ed->fb);
  return(0);
}

#ifndef NOSYMLINKS
/* Link a file according to the required mode
   sw != 0 -> symbolic link
   sw == 0 -> hard link
   When hard linking does not work, it tries to do
   symbolic linking
*/
int WpeLinkFile(char *fl, char *ln, int sw, FENSTER *f)
{
  int ret;

  if(sw || link(fl, ln))
  {
    ret = symlink(fl, ln);
    if (ret)
    {
      e_error(e_msg[ERR_LINKFILE], 0, f->ed->fb);
    }
    return(ret);
  }
  return(0);
}

int WpeRenameLink(char *old, char *ln, char *fl, FENSTER *f)
{
  int ret;

  ret = symlink(fl, ln);
  if(ret)
  {
    e_error(e_msg[ERR_LINKFILE], 0, f->ed->fb);
    return(1);
  }
  else
  {
    if(remove(old))
    {
      e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);
      return(1);
    }
    else
      return(0);
  }
}
#endif

int e_rename(char *file, char *newname, FENSTER * f)
{
  return(WpeRenameCopy(file, newname, f, 0));
}

int e_rename_dir(char *dirct, char *file, char *newname, FENSTER * f, int rec)
{
  return(WpeRenameCopyDir(dirct, file, newname, f, rec, 0));
}


int e_link(char *file, char *newname, FENSTER *f)
{
  return(WpeRenameCopy(file, newname, f, 2));
}


int e_copy(char *file, char *newname, FENSTER * f)
{
  return(WpeRenameCopy(file, newname, f, 1));
}


int WpeFileManagerOptions(FENSTER * f)
{
  int             ret;
  W_OPTSTR       *o = e_init_opt_kst(f);

  if(o == NULL)
    return(1);

  o->xa = 8;
  o->ya = 1;
  o->xe = 69;
  o->ye = 22;
  o->bgsw = AltO;
  o->name = "File-Manager-Options";
  o->crsw = AltO;
  e_add_txtstr(4, 2, "Directories:", o);
  e_add_txtstr(35, 13, "Wastebasket:", o);
  e_add_txtstr(4, 7, "Move/Copy:", o);
  e_add_txtstr(35, 8, "Remove:", o);
  e_add_txtstr(35, 2, "Sort Files by:", o);
  e_add_txtstr(4, 12, "Links on Files:", o);
  e_add_txtstr(4, 16, "On Open:", o);

  e_add_sswstr(5, 3, 12, AltF, 
               f->ed->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0, 
               "Show Hidden Files      ", o);
  e_add_sswstr(5, 4, 12, AltD, 
               f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0, 
               "Show Hidden Directories", o);
  e_add_sswstr(5, 5, 2, AltK, 
               f->ed->flopt & FM_REKURSIVE_ACTIONS ? 1 : 0, 
               "ReKursive Actions      ", o);
  e_add_sswstr(5, 17, 0, AltC, 
               f->ed->flopt & FM_CLOSE_WINDOW ? 1 : 0, 
               "Close File Manager     ", o);

  e_add_sswstr(36, 6, 0, AltR, 
               f->ed->flopt & FM_REVERSE_ORDER ? 1 : 0, 
               "Reverse Order    ", o);

  e_add_pswstr(0, 36, 14, 0, AltP, 0, "Prompt for Delete", o);
  e_add_pswstr(0, 36, 15, 10, AltE, 0, "Delete at Exit   ", o);
  e_add_pswstr(0, 36, 16, 8, AltL, f->ed->flopt & FM_PROMPT_DELETE ? 0 :
               (f->ed->flopt & FM_DELETE_AT_EXIT ? 1 : 2), "Don't DeLete     ", o);
  e_add_pswstr(1, 36, 3, 0, AltN, 0, "Name             ", o);
  e_add_pswstr(1, 36, 4, 1, AltI, 0, "TIme             ", o);
  e_add_pswstr(1, 36, 5, 0, AltB, f->ed->flopt & FM_SORT_NAME ? 1 :
               (f->ed->flopt & FM_SORT_TIME ? 2 : 0), "Bytes            ", o);
  e_add_pswstr(2, 5, 8, 12, AltQ, 0, "Prompt for eQual Files ", o);
  e_add_pswstr(2, 5, 9, 1, AltV, 0, "OVerwrite equal Files  ", o);
  e_add_pswstr(2, 5, 10, 4, AltT, f->ed->flopt & FM_MOVE_PROMPT ? 0 :
               (f->ed->flopt & FM_MOVE_OVERWRITE ? 1 : 2), "Don'T overwrite        ", o);
  e_add_pswstr(3, 5, 13, 4, AltH, 0, "Try Hardlink           ", o);
  e_add_pswstr(3, 5, 14, 7, AltS, 
               f->ed->flopt & FM_TRY_HARDLINK ? 1 : 0,
               "Always Symbolic Link   ", o);
  e_add_pswstr(4, 36, 9, 5, AltW, 0, "into Wastebasket ", o);
  e_add_pswstr(4, 36, 10, 0, AltA, 0, "Absolute (Prompt)", o);
  e_add_pswstr(4, 36, 11, 6, AltM, f->ed->flopt & FM_REMOVE_INTO_WB ? 0 :
               (f->ed->flopt & FM_REMOVE_PROMPT ? 1 : 2), "No ProMpt        ", o);
  e_add_bttstr(16, 19, 1, AltO, " Ok ", NULL, o);
  e_add_bttstr(38, 19, -1, WPE_ESC, "Cancel", NULL, o);

  ret = e_opt_kst(o);
  if(ret != WPE_ESC)
  {
    f->ed->flopt = o->sstr[0]->num +
      (o->sstr[1]->num << 1) +
      (o->sstr[2]->num ? FM_REKURSIVE_ACTIONS : 0) +
      (o->sstr[3]->num ? FM_CLOSE_WINDOW : 0) +
      (o->sstr[4]->num ? FM_REVERSE_ORDER : 0) +
      (o->pstr[0]->num ? (o->pstr[0]->num == 1 ? 4 : 0) : 8) +
      (o->pstr[2]->num ? (o->pstr[2]->num == 1 ? 16 : 0) : 32) +
      (o->pstr[4]->num ? (o->pstr[4]->num == 1 ? 128 : 0) : 64) +
      (o->pstr[1]->num ? (o->pstr[1]->num == 1 ? 01000 : 02000) : 0) +
      (o->pstr[3]->num ? FM_TRY_HARDLINK : 0);
  }

  freeostr(o);
  return(0);
}

int WpeDirDelOptions(FENSTER * f)
{
  int             ret;
  W_OPTSTR       *o = e_init_opt_kst(f);

  if(o == NULL)
    return(1);

  o->xa = 19;
  o->ya = 11;
  o->xe = 53;
  o->ye = 19;
  o->bgsw = AltO;
  o->name = "Message";
  o->crsw = AltO;
  e_add_txtstr(4, 2, "Delete Directory:", o);
  e_add_pswstr(0, 5, 3, 0, AltD, 0, "Delete without Prompt", o);
  e_add_pswstr(0, 5, 4, 0, AltP, 1, "Prompt for Files     ", o);
  e_add_bttstr(7, 6, 1, AltO, " Ok ", NULL, o);
  e_add_bttstr(22, 6, -1, WPE_ESC, "Cancel", NULL, o);

  ret = e_opt_kst(o);

  ret = (ret == WPE_ESC) ? -1 : o->pstr[0]->num;

  freeostr(o);
  return(ret);
}

int WpeShell(FENSTER * f)
{
  PIC            *outp = NULL;
  int             g[4];

  if (!WpeIsXwin())
  {
    outp = e_open_view(0, 0, MAXSCOL - 1, MAXSLNS - 1, f->fb->ws, 1);
    fk_locate(0, 0);
    fk_cursor(1);
#if  MOUSE
    g[0] = 2;
    fk_mouse(g);
#endif
    (*e_u_s_sys_ini) ();
  }
  (*e_u_system) (user_shell);
  if(!WpeIsXwin())
  {
    (*e_u_s_sys_end) ();
    e_close_view(outp, 1);
    fk_cursor(0);
#if  MOUSE
    g[0] = 1;
    fk_mouse(g);
#endif
  }
  return(0);
}

/*   print file */
#ifndef NOPRINTER
int WpePrintFile(FENSTER *f)
{
 char           *str, *dp;
 int             c, sins = f->ins;

 for (c = f->ed->mxedt; c > 0 && !DTMD_ISTEXT(f->ed->f[c]->dtmd); c--)
  ;
 if (c <= 0)
   return(0);
 f = f->ed->f[c];

 if (strcmp(f->ed->print_cmd, "") == 0)
 {
  return(e_error(e_msg[ERR_NOPRINT], 0, f->fb));
 }
 if ((str = MALLOC(strlen(f->datnam) + 32 )) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

 sprintf(str, "File: %s\nDo you want to print it?", f->datnam);
 c = e_message(1, str, f);
 FREE(str);
 if (c != 'Y')
  return(0);

 dp = f->dirct;
 f->dirct = e_tmp_dir;
 f->ins = 0;

 e_save(f);

 f->dirct = dp;
 f->ins = sins;

 if ((str = MALLOC(strlen(e_tmp_dir) + 7 +
                   strlen(f->ed->print_cmd) + strlen(f->datnam) )) == NULL)
  e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

 sprintf(str, "cd %s; %s %s", e_tmp_dir, f->ed->print_cmd, f->datnam);
 if (system(str))
  e_error(e_msg[ERR_NOTINSTALL], 0, f->fb);

 sprintf(str, "%s/%s", e_tmp_dir, f->datnam);
 if (remove(str))
  e_error(e_msg[ERR_DELFILE], 0, f->ed->fb);

 FREE(str);
 return(0);
}
#else
int WpePrintFile(FENSTER * f)
{
  return(e_error(e_msg[ERR_NOPRINT], 0, f->fb));
}
#endif

struct dirfile *WpeSearchFiles(FENSTER *f, char *dirct, char *file, char *string,
                               struct dirfile *df, int sw)
{
  struct dirfile *dd;
  char          **tname, *tp, *tmp, *tmp2;
  int             i;
  static int      rec = 0;

  if(rec > MAXREC)
    return(df);

  /* if not absolute path is given */
  if(*dirct != DIRC)
  {
    tmp2 = WpeGetCurrentDir(f->ed);

    tmp = REALLOC(tmp2, strlen(tmp2) + strlen(dirct) + 4);
    if(tmp == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    tmp2 = tmp;
    if(tmp2[strlen(tmp2) - 1] != DIRC)
      sprintf(tmp2 + strlen(tmp2), "%c%s%c", DIRC, dirct, DIRC);
    else
      sprintf(tmp2 + strlen(tmp2), "%s%c", dirct, DIRC);
  }
  else
  {
    if((tmp2 = MALLOC(strlen(dirct) + 2)) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    if(dirct[strlen(dirct) - 1] != DIRC)
      sprintf(tmp2, "%s%c", dirct, DIRC);
    else
      sprintf(tmp2, "%s", dirct);
  }

  /* assemble total path, dir + filename */
  if((tmp = MALLOC(strlen(tmp2) + strlen(file) + 2)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);
  sprintf(tmp, "%s%s", tmp2, file);

  /* initialise structure, only once */
  if(df == NULL)
  {
    if((df = MALLOC(sizeof(struct dirfile))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

    df->anz = 0;
    if((df->name = MALLOC(sizeof(char *))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);
  }

  /* search all matching file */
  dd = e_find_files(tmp, 0);

  /* if we found something */
  if(dd && dd->anz > 0)
  {

    /* file find */
    if(!(sw & 1024))
    {
      for(i = 0; i < dd->anz; i++)
      {
        if((tp = MALLOC(strlen(tmp2) + strlen(dd->name[i]) + 2)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

        sprintf(tp, "%s%s", tmp2, dd->name[i]);

        df->anz++;

        if((tname = REALLOC(df->name, df->anz * sizeof(char *))) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

        df->name = tname;
        df->name[df->anz - 1] = tp;
      }
    }
    /* file grep */
    else
    {
      for(i = 0; i < dd->anz; i++)
      {
        if((tp = MALLOC(strlen(tmp2) + strlen(dd->name[i]) + 2)) == NULL)
          e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

        sprintf(tp, "%s%s", tmp2, dd->name[i]);

        if(WpeGrepFile(tp, string, sw))
        {
          df->anz++;
          if((tname = REALLOC(df->name, df->anz * sizeof(char *))) == NULL)
            e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

          df->name = tname;
          df->name[df->anz - 1] = tp;
        }
        else
          FREE(tp);
      }
    }
  }

  freedf(dd);
  FREE(tmp2);
  FREE(tmp);

  /* whether recursive action */
  if(!(sw & 512))
    return(df);

  if((tmp = MALLOC(strlen(dirct) + strlen(SUDIR) + 2)) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->ed->fb);

  if(dirct[strlen(dirct) - 1] != DIRC)
    sprintf(tmp, "%s%c%s", dirct, DIRC, SUDIR);
  else
    sprintf(tmp, "%s%s", dirct, SUDIR);

  /* find directories */
  dd = e_find_dir(tmp, 0);

  FREE(tmp);

  if(!dd)
    return(df);

  rec++;
  for(i = 0; i < dd->anz; i++)
  {
    if((tmp = MALLOC(strlen(dirct) + strlen(dd->name[i]) + 3)) == NULL)
    {
      e_error(e_msg[ERR_LOWMEM], 0, f->ed->fb);
      rec--;
      freedf(dd);
      return(df);
    }

    if(dirct[strlen(dirct) - 1] != DIRC)
      sprintf(tmp, "%s%c%s", dirct, DIRC, dd->name[i]);
    else
      sprintf(tmp, "%s%s", dirct, dd->name[i]);

    df = WpeSearchFiles(f, tmp, file, string, df, sw);
    FREE(tmp);
  }

  freedf(dd);
  rec--;
  return(df);

}


int WpeGrepWindow(FENSTER * f)
{
  FIND           *fd = &(f->ed->fd);
  int             ret;
  char            strTemp[80];
  W_OPTSTR       *o = e_init_opt_kst(f);

  if(!o)
    return(-1);
  if(e_blck_dup(strTemp, f))
  {
    strcpy(fd->search, strTemp);
    fd->sn = strlen(fd->search);
  }
  o->xa = 7;
  o->ya = 3;
  o->xe = 63;
  o->ye = 19;
  o->bgsw = 0;
  o->name = "Grep";
  o->crsw = AltO;
  e_add_txtstr(4, 4, "Options:", o);
  e_add_wrstr(4, 2, 18, 2, 35, 128, 0, AltT, "Text to Find:", fd->search, &f->ed->sdf, o);
  e_add_wrstr(4, 10, 17, 10, 36, 128, 0, AltF, "File:", fd->file, &f->ed->fdf, o);
  e_add_wrstr(4, 12, 17, 12, 36, WPE_PATHMAX, 0, AltD, "Directory:", fd->dirct, &f->ed->ddf, o);
  e_add_sswstr(5, 5, 0, AltC, fd->sw & 128 ? 1 : 0, "Case sensitive    ", o);
  e_add_sswstr(5, 6, 0, AltW, fd->sw & 64 ? 1 : 0, "Whole words only  ", o);
  e_add_sswstr(5, 7, 0, AltR, fd->sw & 32 ? 1 : 0, "Regular expression", o);
  e_add_sswstr(5, 8, 0, AltS, 0, "Search Recursive  ", o);
  e_add_bttstr(16, 14, 1, AltO, " Ok ", NULL, o);
  e_add_bttstr(34, 14, -1, WPE_ESC, "Cancel", NULL, o);
  ret = e_opt_kst(o);
  if(ret != WPE_ESC)
  {
    fd->sw = 1024 + (o->sstr[0]->num << 7) + (o->sstr[1]->num << 6) +
      (o->sstr[2]->num << 5) + (o->sstr[3]->num << 9);
    strcpy(fd->search, o->wstr[0]->txt);
    fd->sn = strlen(fd->search);
    strcpy(fd->file, o->wstr[1]->txt);
    if (fd->dirct)
    {
     WpeFree(fd->dirct);
    }
    fd->dirct = WpeStrdup(o->wstr[2]->txt);
  }
  freeostr(o);
  if(ret != WPE_ESC)
    ret = e_data_first(2, f->ed, fd->dirct);
  return(ret);
}


int WpeFindWindow(FENSTER * f)
{
  FIND           *fd = &(f->ed->fd);
  int             ret;
  W_OPTSTR       *o = e_init_opt_kst(f);

  if(!o)
    return(-1);
  o->xa = 7;
  o->ya = 3;
  o->xe = 61;
  o->ye = 14;
  o->bgsw = 0;
  o->name = "Find File";
  o->crsw = AltO;
  e_add_txtstr(4, 6, "Options:", o);
  e_add_wrstr(4, 2, 15, 2, 36, 128, 0, AltF, "File:", fd->file, &f->ed->fdf, o);
  e_add_wrstr(4, 4, 15, 4, 36, WPE_PATHMAX, 0, AltD, "Directory:", fd->dirct, &f->ed->ddf, o);
  e_add_sswstr(5, 7, 0, AltS, 1, "Search Recursive  ", o);
  e_add_bttstr(13, 9, 1, AltO, " Ok ", NULL, o);
  e_add_bttstr(33, 9, -1, WPE_ESC, "Cancel", NULL, o);
  ret = e_opt_kst(o);
  if(ret != WPE_ESC)
  {
    fd->sw = (o->sstr[0]->num << 9);
    strcpy(fd->file, o->wstr[0]->txt);
    if (fd->dirct)
    {
     WpeFree(fd->dirct);
    }
    fd->dirct = WpeStrdup(o->wstr[1]->txt);
  }
  freeostr(o);
  if(ret != WPE_ESC)
    ret = e_data_first(3, f->ed, fd->dirct);
  return(ret);
}






















int e_ed_man(char *str, FENSTER * f)
{
  char            command[256], tstr[_POSIX_PATH_MAX];
  char            cc, hstr[80], nstr[10];
  int             mdsv = f->ed->dtmd, bg, i, j = 0;
  BUFFER         *b;

  if(!str)
    return(0);
  while(isspace(*str++));
  if(!*--str)
    return(0);
  for(i = f->ed->mxedt; i >= 0; i--)
  {
    if(!strcmp(f->ed->f[i]->datnam, str))
    {
      e_switch_window(f->ed->edt[i], f);
      return(0);
    }
  }
  WpeMouseChangeShape(WpeWorkingShape);
  sprintf(tstr, "%s/%s", e_tmp_dir, str);
  for(i = 0; (hstr[i] = str[i]) && str[i] != '(' && str[i] != ')'; i++);
  hstr[i] = '\0';
  if(str[i] == '(')
  {
    for(++i; (nstr[j] = str[i]) && str[i] != ')' && str[i] != '('; i++, j++);

    /* Some SEE ALSO's are list as "foobar(3X)" but are in section 3 not 3X.
       This is a quick hack to fix the problem.  -- Dennis */
    if(isdigit(nstr[0]))
      j = 1;
  }
  nstr[j] = '\0';

  while(1)
  {
#ifdef MAN_S_OPT
    if(!nstr[0])
      sprintf(command, " man %s > \'%s\' 2> /dev/null", hstr, tstr);
    else
      sprintf(command, " man -s %s %s > \'%s\' 2> /dev/null", nstr, hstr, tstr);
#else
    sprintf(command, " man %s %s > \'%s\' 2> /dev/null", nstr, hstr, tstr);
#endif
    system(command);
    chmod(tstr, 0400);
    f->ed->dtmd = DTMD_HELP;
    e_edit(f->ed, tstr);
    f->ed->dtmd = mdsv;
    f = f->ed->f[f->ed->mxedt];
    b = f->b;
    if(b->mxlines > 1 || !nstr[1])
      break;
    nstr[1] = '\0';
    chmod(tstr, 0600);
    remove(tstr);
    e_close_window(f);
  }
  if(b->mxlines == 1 && b->bf[0].len == 0)
  {
    e_ins_nchar(f->b, f->s, "No manual entry for ", 0, 0, 20);
    e_ins_nchar(f->b, f->s, hstr, b->b.x, b->b.y, strlen(hstr));
    e_ins_nchar(f->b, f->s, ".", b->b.x, b->b.y, 1);
  }
  for(i = 0; i < b->mxlines; i++)
    if(b->bf[i].len == 0 && (i == 0 || b->bf[i - 1].len == 0))
    {
      e_del_line(i, b, f->s);
      i--;
    }
  for(bg = 0; bg < b->bf[0].len && isspace(b->bf[0].s[bg]); bg++);
  if(bg == b->bf[0].len)
    bg = 0;
  for(i = 0;
      i < b->mxlines &&
      WpeStrnccmp(b->bf[i].s + bg, "\017SEE\005 \017ALSO\005", 12) &&
      WpeStrnccmp(b->bf[i].s + bg, "SEE ALSO", 8);
      i++);
  if(i < b->mxlines)
    for(bg = 0, i++; i < b->mxlines && b->bf[i].len > 0 && bg >= 0; i++)
    {
      bg = 0;
      while(b->bf[i].s[bg])
      {
        for(; isspace(b->bf[i].s[bg]); bg++);
        if(!b->bf[i].s[bg])
          continue;
        for(j = bg + 1;
            b->bf[i].s[j] && b->bf[i].s[j] != ',' && b->bf[i].s[j] != '.' &&
            b->bf[i].s[j] != ' ' && b->bf[i].s[j] != '(';
            j++);
        if(b->bf[i].s[j] != '(')
        {
          bg = -1;
          break;
        }
        if(b->bf[i].s[j - 1] == 5)
          e_del_nchar(b, f->s, j - 1, i, 1);
        for(j++; b->bf[i].s[j] && b->bf[i].s[j] != ',' && b->bf[i].s[j] != '.';
            j++);
        if(b->bf[i].s[bg] == 15)
          b->bf[i].s[bg] = HFB;
        else
        {
          cc = HFB;
          e_ins_nchar(b, f->s, &cc, bg, i, 1);
          j++;
        }
        cc = HED;
        e_ins_nchar(b, f->s, &cc, j, i, 1);
        j++;
        if(b->bf[i].s[j])
          j++;
        bg = j;
      }
    }
  b->b.x = b->b.y = 0;
  chmod(tstr, 0600);
  remove(tstr);
  WpeMouseRestoreShape();
  e_schirm(f, 1);
  return(0);
}

int e_funct(FENSTER * f)
{
 char            str[80];

 if (f->ed->hdf && f->ed->hdf->anz > 0)
  strcpy(str, f->ed->hdf->name[0]);
 else
  str[0] = '\0';
 if (e_add_arguments(str, "Function", f, 0, AltF, &f->ed->hdf))
 {
  f->ed->hdf = e_add_df(str, f->ed->hdf);
  e_ed_man(str, f);
 }
 return(0);
}

struct dirfile *e_make_funct(char *man)
{
 struct dirfile *df = NULL, *dout = MALLOC(sizeof(struct dirfile));
 char *sustr, *subpath, *manpath;
 int ret = 0, n, i = 0, j, k, l = 0;

 manpath = NULL;
 WpeMouseChangeShape(WpeWorkingShape);
 dout->anz = 0;
 dout->name = NULL;
 if (getenv("MANPATH"))
 {
  manpath = strdup(getenv("MANPATH"));
 }
 if ((!manpath) || (manpath[0] == '\0'))
 {
  manpath = strdup("/usr/man:/usr/share/man:/usr/X11R6/man:/usr/local/man");
 }
 /* Allocate the maximum possible rather than continually realloc. */
 sustr = malloc(strlen(manpath) + 10);
 while (manpath[i])
 {
  subpath = manpath + i;
  for (n = 0; (manpath[i]) && (manpath[i] != PTHD); i++, n++);
  if (manpath[i] == PTHD)
  {
   manpath[i] = '\0';
   i++;
  }
  sprintf(sustr, "%s/man%s/*", subpath, man);
  df = e_find_files(sustr, 0);
  if (!df->anz)
  {
   freedf(df);
   continue;
  }
  for (j = 0; j < df->anz; j++)
  {
   k = strlen(df->name[j]) - 1;

   /* If the file is gzipped strip the .gz ending. */
   if ((k > 2) && (strcmp(df->name[j] + k - 2, ".gz") == 0))
   {
    k -= 3;
    *(df->name[j] + k + 1) = '\0';
   }
   else if ((k > 3) && (strcmp(df->name[j] + k - 3, ".bz2") == 0))
   {
    k -= 4;
    *(df->name[j] + k + 1) = '\0';
   }
   else if ((k > 1) && (strcmp(df->name[j] + k - 1, ".Z") == 0))
   {
    k -= 2;
    *(df->name[j] + k + 1) = '\0';
   }

   for (k = strlen(df->name[j]) - 1; k >= 0 && *(df->name[j] + k) != '.'; k--)
    ;
   if (k >= 0 /**(df->name[j]+k)*/ )
   {
    df->name[j] = REALLOC(df->name[j],
      (l = strlen(df->name[j]) + 2) * sizeof(char));
    *(df->name[j] + k) = '(';
    *(df->name[j] + l - 2) = ')';
    *(df->name[j] + l - 1) = '\0';
   }
  }
  if (!dout->name)
   dout->name = MALLOC(df->anz * sizeof(char *));
  else
   dout->name = REALLOC(dout->name, (df->anz + dout->anz) * sizeof(char *));
  for (j = 0; j < df->anz; j++)
  {
   for (k = 0; k < dout->anz; k++)
   {
    if (!(ret = strcmp(df->name[j], dout->name[k])))
    {
     FREE(df->name[j]);
     break;
    }
    else if (ret < 0)
     break;
   }
   if (!ret && dout->anz)
    continue;
   for (l = dout->anz; l > k; l--)
    dout->name[l] = dout->name[l - 1];
   dout->name[k] = df->name[j];
   dout->anz++;
  }
  FREE(df);
 }
 free(manpath);
 free(sustr);
 WpeMouseRestoreShape();
 return(dout);
}


#ifdef PROG
extern struct dirfile **e_p_df;
#endif

int e_data_first(int sw, ECNT *cn, char *nstr)
{
  extern char    *e_hlp_str[];
  extern WOPT    *gblst, *oblst;
  FENSTER        *f;
  int             i, j;
  struct dirfile *df = NULL;
  FLWND          *fw;

  if(cn->mxedt >= MAXEDT)
  {
    e_error(e_msg[ERR_MAXWINS], 0, cn->fb);
    return(-1);
  }
  for(j = 1; j <= MAXEDT; j++)
  {
    for(i = 1; i <= cn->mxedt && cn->edt[i] != j; i++);
    if(i > cn->mxedt)
      break;
  }
  cn->curedt = j;
  (cn->mxedt)++;
  cn->edt[cn->mxedt] = j;

  if((f = (FENSTER *) MALLOC(sizeof(FENSTER))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
  if((fw = (FLWND *) MALLOC(sizeof(FLWND))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
  f->fb = cn->fb;
  cn->f[cn->mxedt] = f;
  f->a = e_set_pnt(22, 3);
  f->e = e_set_pnt(f->a.x + 35, f->a.y + 18);
  f->winnum = cn->curedt;
  f->dtmd = DTMD_DATA;
  f->ins = sw;
  f->save = 0;
  f->zoom = 0;
  f->ed = cn;
  f->c_sw = NULL;
  f->c_st = NULL;
  f->pic = NULL;
  f->fd.dirct = NULL;

  if(!nstr)
    f->dirct = NULL; /* how about a free up ??? */
  else
  {
    f->dirct = MALLOC(strlen(nstr) + 1);
    strcpy(f->dirct, nstr);
  }
  WpeMouseChangeShape(WpeWorkingShape);
  if(sw == 1)
  {
    f->datnam = "Function-Index";
    df = e_make_funct(nstr);
  }
  else if(sw == 2)
  {
    f->datnam = "Grep";
    df = WpeSearchFiles(f, nstr, cn->fd.file, 
                        cn->fd.search, NULL,
                        cn->fd.sw);
  }
  else if(sw == 3)
  {
    f->datnam = "Find";
    df = WpeSearchFiles(f, nstr, cn->fd.file, 
                        cn->fd.search, NULL,
                        cn->fd.sw);
  }
  else if(sw == 7)
  {
    f->datnam = "Windows";
    df = e_make_win_list(f);
  }
#ifdef PROG
  else if(sw == 4)
  {
    f->datnam = "Project";
    df = e_p_df[0];
  }
  else if(sw == 5)
  {
    f->datnam = "Variables";
    df = e_p_df[1];
  }
  else if(sw == 6)
  {
    f->datnam = "Install";
    df = e_p_df[2];
  }
#endif
  f->hlp_str = e_hlp_str[16 + sw];
  if(sw < 4)
  {
    f->blst = gblst;
    f->nblst = 4;
  }
  else
  {
    f->blst = oblst;
    f->nblst = 4;
  }
  WpeMouseRestoreShape();
  f->b = (BUFFER *) fw;
  fw->df = df;

  fw->mxa = f->a.x;
  fw->mxe = f->e.x;
  fw->mya = f->a.y;
  fw->mye = f->e.y;
  fw->xa = f->a.x + 3;
  fw->xe = f->e.x - 13;
  fw->ya = f->a.y + 3;
  fw->ye = f->e.y - 1;
  fw->f = f;
  fw->ia = fw->nf = fw->nxfo = fw->nyfo = 0;
  fw->srcha = fw->ja = 0;

  if(cn->mxedt > 1 && (f->ins < 5 || f->ins == 7))
    e_ed_rahmen(cn->f[cn->mxedt - 1], 0);
  e_firstl(f, 1);
  e_data_schirm(f);
  return(0);
}

int e_data_schirm(FENSTER * f)
{
  int             i, j;
  FLWND          *fw = (FLWND *) f->b;

  for(j = f->a.y + 1; j < f->e.y; j++)
    for(i = f->a.x + 1; i < f->e.x; i++)
      e_pr_char(i, j, ' ', f->fb->nt.fb);

  if(NUM_COLS_ON_SCREEN > 25)
  {
    if(f->ins < 4 || f->ins == 7)
      e_pr_str((f->e.x - 9), f->e.y - 4, "Show", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
    else if(f->ins > 3)
    {
      e_pr_str((f->e.x - 9), f->e.y - 8, "Add", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
      e_pr_str((f->e.x - 9), f->e.y - 6, "Edit", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
      e_pr_str((f->e.x - 9), f->e.y - 4, "Delete", f->fb->nz.fb, 0, -1,
               f->fb->ns.fb, f->fb->nt.fb);
      if(f->ins == 4 && f->a.y < f->e.y - 10)
      {
        e_pr_str((f->e.x - 9), f->e.y - 10, "Options", f->fb->nz.fb,
                 0, -1, f->fb->ns.fb, f->fb->nt.fb);
      }
    }
    e_pr_str((f->e.x - 9), f->e.y - 2, "Cancel", f->fb->nz.fb, -1, -1,
             f->fb->ns.fb, f->fb->nt.fb);
  }

  if(NUM_COLS_ON_SCREEN > 25)
  {
    fw->xa = f->a.x + 3;
    fw->xe = f->e.x - 13;
  }
  else
  {
    fw->xa = f->a.x + 3;
    fw->xe = f->e.x - 2;
  }
  fw->mxa = f->a.x;
  fw->mxe = f->e.x;
  fw->mya = f->a.y;
  fw->mye = f->e.y;
  fw->xa = f->a.x + 3;
  fw->ya = f->a.y + 3;
  fw->ye = f->e.y - 1;
#ifdef PROG
  if(f->ins == 4)
    fw->df = e_p_df[0];
#endif
  if(f->ins == 1)
    e_pr_str(fw->xa, f->a.y + 2, "Functions:", f->fb->nt.fb, 0, 1,
             f->fb->nsnt.fb, f->fb->nt.fb);
  else if(f->ins == 3)
    e_pr_str(fw->xa, f->a.y + 2, "Directories:", f->fb->nt.fb, 0, 1,
             f->fb->nsnt.fb, f->fb->nt.fb);
  e_mouse_bar(fw->xe, fw->ya, fw->ye - fw->ya, 0, fw->f->fb->em.fb);
  e_mouse_bar(fw->xa, fw->ye, fw->xe - fw->xa, 1, fw->f->fb->em.fb);
  e_pr_file_window(fw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);
  return(0);
}

int e_data_eingabe(ECNT * cn)
{
  FENSTER        *f = cn->f[cn->mxedt];
  FLWND          *fw = (FLWND *) f->b;
  int             c = AltF;

  fk_cursor(0);
  if(f->ins == 7)
  {
    freedf(fw->df);
    fw->df = e_make_win_list(f);
  }
  while(c != WPE_ESC)
  {
    if(f->dtmd != DTMD_DATA)
      return(0);
#ifdef PROG
    if(f->ins == 4)
      fw->df = e_p_df[0];
#endif
    if(c == AltF)
      c = e_file_window(0, fw, f->fb->ft.fb, f->fb->fz.fb);
#if  MOUSE
    if(c == MBKEY)
      c = e_data_ein_mouse(f);
#endif
    if(((c == WPE_CR || c == AltS) && (f->ins < 4 || f->ins == 7)) ||
       ((c == AltA || c == EINFG) && (f->ins > 3 && f->ins < 7)))
    {
      if(f->ins == 1)
        e_ed_man(fw->df->name[fw->nf], f);
      else if(f->ins == 2)
      {
        e_edit(f->ed, fw->df->name[fw->nf]);
        e_rep_search(f->ed->f[f->ed->mxedt]);
      }
      else if(f->ins == 3)
      {
        e_edit(f->ed, fw->df->name[fw->nf]);

/*        WpeCreateFileManager(0, f->ed, fw->df->name[fw->nf]); */
      }
      else if(f->ins == 7)
        e_switch_window(f->ed->edt[fw->df->anz - fw->nf], f);
#ifdef PROG
      else if(f->ins == 4)
      {
        WpeCreateFileManager(5, f->ed, NULL);
        while(WpeHandleFileManager(f->ed) != WPE_ESC);
        e_p_df[f->ins - 4] = fw->df;
        c = AltF;
        f->save = 1;
      }
      else if(f->ins > 4 && f->ins < 7)
      {
        e_p_add_df(fw, f->ins);
        e_p_df[f->ins - 4] = fw->df;
        c = AltF;
      }
#endif
      if(f->ins < 4 || f->ins == 7)
        return(0);
    }
#ifdef PROG
    else if(f->ins > 3 && f->ins < 7 && (c == AltD || c == ENTF))
    {
      e_p_del_df(fw, f->ins);
      c = AltF;
      f->save = 1;
    }
    else if(f->ins > 4 && f->ins < 7 && (c == AltE || c == WPE_CR))
    {
      e_p_edit_df(fw, f->ins);
      c = AltF;
      f->save = 1;
    }
    else if(f->ins == 4 && (c == AltE || c == WPE_CR))
    {
      e_edit(f->ed, fw->df->name[fw->nf]);
      c = WPE_ESC;
    }
    else if(f->ins == 4 && c == AltO)
    {
      e_project_options(f);
      c = AltF;
    }
#endif
    else if(c != AltF)
    {
      if(c == AltBl)
        c = WPE_ESC;
      else if(c == WPE_ESC)
        c = (f->ed->edopt & ED_CUA_STYLE) ? CF4 : AF3;
      if(f->ins == 7 && ((!(f->ed->edopt & ED_CUA_STYLE) && c == AF3)
                         || ((f->ed->edopt & ED_CUA_STYLE) && c == CF4)))
        e_close_window(f);
      if(f->ins == 4 && ((!(f->ed->edopt & ED_CUA_STYLE) && c == AF3)
                         || ((f->ed->edopt & ED_CUA_STYLE) && c == CF4)))
      {
        FLWND          *fw = (FLWND *) f->ed->f[f->ed->mxedt]->b;
        fw->df = NULL;
        e_close_window(f->ed->f[f->ed->mxedt]);
        return(0);
      }
      if(f->ins == 4 && (!e_tst_dfkt(f, c) || !e_prog_switch(f, c)))
        return(0);
      if(f->ins > 4 && ((!(f->ed->edopt & ED_CUA_STYLE) && c == AF3)
                        || ((f->ed->edopt & ED_CUA_STYLE) && c == CF4)))
        return(c);
      else if((f->ins < 4 || f->ins == 7) && !e_tst_dfkt(f, c))
        return(0);
      else
        c = AltF;
    }
  }
  return((f->ed->edopt & ED_CUA_STYLE) ? CF4 : AF3);
}

int e_get_funct_in(char *nstr, FENSTER * f)
{
  return(e_data_first(1, f->ed, nstr));
}

int e_funct_in(FENSTER * f)
{
  int             n, xa = 37, ya = 2, num = 8;
  OPTK           *opt = MALLOC(num * sizeof(OPTK));
  char            nstr[2];

  opt[0].t = "User Commands";
  opt[0].x = 0;
  opt[0].o = 'U';
  opt[1].t = "System Calls";
  opt[1].x = 0;
  opt[1].o = 'S';
  opt[2].t = "C-Lib.-Functions";
  opt[2].x = 0;
  opt[2].o = 'C';
  opt[3].t = "Devices & Netw. I.";
  opt[3].x = 0;
  opt[3].o = 'D';
  opt[4].t = "File Formats";
  opt[4].x = 0;
  opt[4].o = 'F';
  opt[5].t = "Games & Demos";
  opt[5].x = 0;
  opt[5].o = 'G';
  opt[6].t = "Environment, ...";
  opt[6].x = 0;
  opt[6].o = 'E';
  opt[7].t = "Maintenance Com.";
  opt[7].x = 0;
  opt[7].o = 'M';

  n = e_opt_sec_box(xa, ya, num, opt, f, 1);

  FREE(opt);
  if(n < 0)
    return(WPE_ESC);

  nstr[0] = '1' + n;
  nstr[1] = '\0';
  return(e_get_funct_in(nstr, f));
}

#endif
