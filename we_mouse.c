/* we_mouse.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"

#if  MOUSE

#include "makro.h"

int e_mouse_cursor();

/*   mouse button pressed (?)     */
int e_mshit()
{
 extern struct mouse e_mouse;
 int g[4]; /* = { 3, 0, 0, 0 };  */
 g[0] = 3;  g[1] = 0;

 fk_mouse(g);
 e_mouse.x = g[2]/8;
 e_mouse.y = g[3]/8;
 return(g[1]);
}

/*     mouse main menu control */
int e_m1_mouse()
{
 extern struct mouse e_mouse;
 extern OPT opt[];
 extern int e_mn_men;
 int c, n;

 if (e_mouse.y == MAXSLNS-1) c = e_m3_mouse();
 else if (e_mouse.y != 0) c = WPE_ESC;
 else
 {
  for (n = 1; n < MENOPT; n++)
   if (e_mouse.x < opt[n].x-e_mn_men) break;
  c = opt[n-1].as;
 }
 return(c);
}

/*   mouse options box control */
int e_m2_mouse(int xa, int ya, int xe, int ye, OPTK *fopt)
{
 extern struct mouse e_mouse;
 int c;

 if (e_mouse.y == MAXSLNS) c = e_m3_mouse();
 else if (e_mouse.y == 0) return(e_m1_mouse());
 else if (e_mouse.x <= xa || e_mouse.x >= xe || e_mouse.y <= ya ||
   e_mouse.y >= ye)
  c = WPE_ESC;
 else  c = fopt[e_mouse.y-ya-1].o;
 while (e_mshit())
  ;
 return(c);
}

/*      Mouse assistant bar control       */
int e_m3_mouse()
{
 extern WOPT *blst;
 extern struct mouse e_mouse;
 extern int nblst;
 int i;

 if (e_mouse.y != MAXSLNS-1) return(WPE_ESC);
 while (e_mshit())
  ;
 for(i = 1; i < nblst; i++)
  if(e_mouse.x < blst[i].x) return(blst[i-1].as);
 return(blst[nblst-1].as);
}

/*   mouse errors box control */
int e_er_mouse(int x, int y, int xx, int yy)
{
 extern struct mouse e_mouse;

 while (e_mshit() != 0)
  ;
 if (y == e_mouse.y && x == e_mouse.x) return (WPE_CR);
 if (yy == e_mouse.y && xx-1 <= e_mouse.x && xx+4 >= e_mouse.x)
  return (WPE_CR);
 return(0);
}

/*   mouse messages box control */
int e_msg_mouse(int x, int y, int x1, int x2, int yy)
{
 extern struct mouse e_mouse;

 while (e_mshit() != 0)
  ;
 if (y == e_mouse.y && x == e_mouse.x) return (WPE_CR);
 if (yy == e_mouse.y)
 {
  if (x1-1 <= e_mouse.x && x1+5 >= e_mouse.x) return ('Y');
  if (x2-1 <= e_mouse.x && x2+5 >= e_mouse.x) return (WPE_ESC);
  if ((x1=(x1+x2)/2 - 1) <= e_mouse.x && x1+5 >= e_mouse.x) return ('N');
 }
 return(0);
}

int e_rahmen_mouse(FENSTER *f)
{
 extern struct mouse e_mouse;
 int c = 1;

 if (e_mouse.x == f->a.x+3 && e_mouse.y == f->a.y) c = WPE_ESC;
 else if (e_mouse.x == f->e.x-3 && e_mouse.y == f->a.y) e_ed_zoom(f);
 else if (e_mouse.x == f->a.x && e_mouse.y == f->a.y) e_eck_mouse(f, 1);
 else if (e_mouse.x == f->e.x && e_mouse.y == f->a.y) e_eck_mouse(f, 2);
 else if (e_mouse.x == f->e.x && e_mouse.y == f->e.y) e_eck_mouse(f, 3);
 else if (e_mouse.x == f->a.x && e_mouse.y == f->e.y) e_eck_mouse(f, 4);
 else if (e_mouse.y == f->a.y && e_mouse.x > f->a.x && e_mouse.x < f->e.x)
  e_eck_mouse(f, 0);
 else c = 0;
 while (e_mshit() != 0)
  ;
 return(c);
}

int WpeMngMouseInFileManager(FENSTER * f)
{
  extern struct mouse e_mouse;
  ECNT           *cn = f->ed;
  FLBFFR         *b = (FLBFFR *) f->b;
  int             i, c = 0, by = 4;

  if(e_mouse.y == 0)
    return(AltBl);
  else if(e_mouse.y == MAXSLNS - 1)
    return(e_m3_mouse());
  else if(e_mouse.x < f->a.x || e_mouse.x > f->e.x
          || e_mouse.y < f->a.y || e_mouse.y > f->e.y)
  {
    for(i = cn->mxedt; i > 0; i--)
    {
      if(e_mouse.x >= cn->f[i]->a.x && e_mouse.x <= cn->f[i]->e.x
         && e_mouse.y >= cn->f[i]->a.y && e_mouse.y <= cn->f[i]->e.y)
      {
        while(e_mshit() != 0);
        return(cn->edt[i] < 10 ? Alt1 - 1 + cn->edt[i] : 1014 + cn->edt[i]);
      }
    }
  }
  else if(e_mouse.x == f->a.x + 3 && e_mouse.y == f->a.y)
    c = WPE_ESC;
  else if(e_mouse.x == f->e.x - 3 && e_mouse.y == f->a.y)
    e_ed_zoom(f);
  else if(e_mouse.x == f->a.x && e_mouse.y == f->a.y)
    e_eck_mouse(f, 1);
  else if(e_mouse.x == f->e.x && e_mouse.y == f->a.y)
    e_eck_mouse(f, 2);
  else if(e_mouse.x == f->e.x && e_mouse.y == f->e.y)
    e_eck_mouse(f, 3);
  else if(e_mouse.x == f->a.x && e_mouse.y == f->e.y)
    e_eck_mouse(f, 4);
  else if(e_mouse.y == f->a.y && e_mouse.x > f->a.x
          && e_mouse.x < f->e.x)
    e_eck_mouse(f, 0);
  else
  {
    if(NUM_LINES_ON_SCREEN <= 17)
      by = -1;
    else if(b->sw != 0 || NUM_LINES_ON_SCREEN <= 19)
      by = 2;

    if(NUM_LINES_ON_SCREEN > 17)
    {
      if(e_mouse.y == f->e.y - by && e_mouse.x >= f->a.x + 3
         && e_mouse.x <= f->a.x + 10)
        c = WPE_ESC;
      else if(e_mouse.y == f->e.y - by && e_mouse.x >= f->a.x + 13
              && e_mouse.x <= f->a.x + 24)
        c = AltC;
      else if(b->sw == 1 && NUM_COLS_ON_SCREEN >= 34 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 27 && e_mouse.x <= f->a.x + 32)
        c = AltR;
      else if(b->sw == 2 && NUM_COLS_ON_SCREEN >= 35 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 27 && e_mouse.x <= f->a.x + 33)
        c = AltW;
      else if(b->sw == 4 && NUM_COLS_ON_SCREEN >= 34 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 27 && e_mouse.x <= f->a.x + 32)
        c = AltS;
      else if(b->sw == 4 && NUM_COLS_ON_SCREEN >= 48 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 35 && e_mouse.x <= f->a.x + 46)
        c = AltY;
      else if(b->sw == 3 && NUM_COLS_ON_SCREEN >= 37 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 27 && e_mouse.x <= f->a.x + 35)
        c = AltE;
      else if(b->sw == 5 && NUM_COLS_ON_SCREEN >= 33 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 27 && e_mouse.x <= f->a.x + 31)
        c = AltA;
      else if(b->sw == 0 && NUM_COLS_ON_SCREEN >= 35 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 27 && e_mouse.x <= f->a.x + 33)
        c = AltK;
      else if(b->sw == 0 && NUM_COLS_ON_SCREEN >= 49 && e_mouse.y == f->e.y - by
              && e_mouse.x >= f->a.x + 36 && e_mouse.x <= f->a.x + 47)
        c = AltA;
    }
    if(b->sw == 0 && NUM_LINES_ON_SCREEN > 19)
    {
      if(e_mouse.y == f->e.y - 2 && e_mouse.x >= f->a.x + 3
         && e_mouse.x <= f->a.x + 8)
        c = AltM;
      else if(e_mouse.y == f->e.y - 2 && NUM_COLS_ON_SCREEN >= 21 &&
              e_mouse.x >= f->a.x + 12 && e_mouse.x <= f->a.x + 19)
        c = AltR;
      else if(e_mouse.y == f->e.y - 2 && NUM_COLS_ON_SCREEN >= 30 &&
              e_mouse.x >= f->a.x + 23 && e_mouse.x <= f->a.x + 28)
        c = AltL;
      else if(e_mouse.y == f->e.y - 2 && NUM_COLS_ON_SCREEN >= 39 &&
              e_mouse.x >= f->a.x + 32 && e_mouse.x <= f->a.x + 37)
        c = AltO;
      else if(e_mouse.y == f->e.y - 2 && NUM_COLS_ON_SCREEN >= 48 &&
              e_mouse.x >= f->a.x + 41 && e_mouse.x <= f->a.x + 46)
        c = AltE;
    }

    if(e_mouse.y == f->a.y + 3 && e_mouse.x >= f->a.x + b->xfa
       && e_mouse.x <= f->a.x + b->xfa + b->xfd)
      c = AltN;
    else if(e_mouse.y == f->a.y + 3 && e_mouse.x >= f->a.x + b->xda
            && e_mouse.x <= f->a.x + b->xda + b->xdd)
      c = AltD;
    else if(e_mouse.y >= b->fw->ya && e_mouse.y <= b->fw->ye
            && e_mouse.x >= b->fw->xa && e_mouse.x <= b->fw->xe)
      c = AltF;
    else if(e_mouse.y >= b->dw->ya && e_mouse.y <= b->dw->ye
            && e_mouse.x >= b->dw->xa && e_mouse.x <= b->dw->xe)
      c = AltT;
  }
  while(e_mshit() != 0);
  return(c);
}

int WpeMouseInFileDirList(int k, int sw, FENSTER * f)
{
  extern struct mouse e_mouse;
  ECNT           *cn = f->ed;
  FLBFFR         *b = (FLBFFR *) f->b;
  int             i;
  char            tmp[256];

  if(e_mouse.x >= f->a.x && e_mouse.x <= f->e.x
     && e_mouse.y >= f->a.y && e_mouse.y <= f->e.y)
    return(0);
  for(i = cn->mxedt - 1; i > 0; i--)
  {
    if(e_mouse.x >= cn->f[i]->a.x && e_mouse.x <= cn->f[i]->e.x
       && e_mouse.y >= cn->f[i]->a.y && e_mouse.y <= cn->f[i]->e.y)
      break;
  }
  if(i <= 0)
    return(0);
  if(sw)
  {
    if(cn->f[i]->dirct[strlen(cn->f[i]->dirct) - 1] == DIRC)
      sprintf(tmp, "%s%s", cn->f[i]->dirct, b->dd->name[b->dw->nf - b->cd->anz]);
    else
      sprintf(tmp, "%s/%s", cn->f[i]->dirct, b->dd->name[b->dw->nf - b->cd->anz]);
#ifndef DJGPP
    if(k == -2)
      e_copy(b->dd->name[b->dw->nf - b->cd->anz], tmp, f);
    else if(k == -4)
      e_link(b->dd->name[b->dw->nf - b->cd->anz], tmp, f);
#else
    if(k == -4)
      e_copy(b->dd->name[b->dw->nf - b->cd->anz], tmp, f);
#endif
    else
      e_rename(b->dd->name[b->dw->nf - b->cd->anz], tmp, f);
    freedf(b->cd);
    freedf(b->dw->df);
    freedf(b->dd);
    b->dd = e_find_dir(SUDIR, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
    b->cd = WpeCreateWorkingDirTree(f->save, cn);  /* ??? cn */
    b->dw->df = WpeGraphicalDirTree(b->cd, b->dd, cn);
    b->dw->nf = b->cd->anz - 1;
    b->dw->ia = b->dw->ja = 0;
    e_pr_file_window(b->dw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);
  }
  else
  {
    if(cn->f[i]->dirct[strlen(cn->f[i]->dirct) - 1] == DIRC)
      sprintf(tmp, "%s%s", cn->f[i]->dirct, b->df->name[b->fw->nf]);
    else
      sprintf(tmp, "%s/%s", cn->f[i]->dirct, b->df->name[b->fw->nf]);
#ifndef DJGPP
    if(k == -2)
      e_copy(b->df->name[b->fw->nf], tmp, f);
    else if(k == -4)
      e_link(b->df->name[b->fw->nf], tmp, f);
#else
    if(k == -4)
      e_copy(b->df->name[b->fw->nf], tmp, f);
#endif
    else
      e_rename(b->df->name[b->fw->nf], tmp, f);
    freedf(b->df);
    freedf(b->fw->df);
    b->df = e_find_files(b->rdfile, f->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
    b->fw->df = WpeGraphicalFileList(b->df, f->ed->flopt >> 9, cn);
    b->fw->ia = b->fw->nf = 0;
    b->fw->ja = b->fw->srcha;
    e_pr_file_window(b->fw, 0, 1, f->fb->ft.fb, f->fb->fz.fb, f->fb->frft.fb);
  }
  return(cn->edt[i] < 10 ? Alt1 - 1 + cn->edt[i] : 1014 + cn->edt[i]);
}


/*  File Window  */
char *e_gt_btstr(int x, int y, int n, char *buffer)
{
 int i;

 for (i = 0; i < n; i++) buffer[i] = e_gt_byte(2*x + i, y)
  ;
 return(buffer);
}

char *e_pt_btstr(int x, int y, int n, char *buffer)
{
 int i;

 for(i = 0; i < n; i++) e_pt_byte(2*x + i, y, buffer[i])
  ;
 return(buffer);
}

int fl_wnd_mouse(sw, k, fw)
     int sw;
     int k;
     FLWND *fw;
{
   extern struct mouse e_mouse;
   int MLEN, i, c, xa, ya, xn, yn, xdif;
   char *file, *bgrd;
   if(e_mouse.x == fw->xe && e_mouse.y >= fw->ya && e_mouse.y < fw->ye)
   {  fw->nf = e_lst_mouse(fw->xe, fw->ya, fw->ye-fw->ya, 0,
						fw->df->anz, fw->nf);
      return(0);
   }
   else if(e_mouse.y == fw->ye && e_mouse.x >= fw->xa && e_mouse.x < fw->xe)
   {  fw->ja = e_lst_mouse(fw->xa, fw->ye, fw->xe-fw->xa, 1,
				strlen(*(fw->df->name+fw->nf)),	fw->ja);
      return(0);
   }
   else if(e_mouse.y >= fw->ya && e_mouse.y < fw->ye &&
			 e_mouse.x >= fw->xa && e_mouse.x < fw->xe)
   {  if(fw->nf == e_mouse.y - fw->ya + fw->ia
		 && e_mouse.y - fw->ya + fw->ia < fw->df->anz)
      {  /*  if(k == -2) {  while (e_mshit() != 0);  return(AltU);  }
	 else if(k == -4) {  while (e_mshit() != 0);  return(AltM);  }
	 else   */
	 {  xa = e_mouse.x; ya = e_mouse.y; xdif = e_mouse.x - fw->xa;
	    if(fw->srcha >= 0) c = fw->srcha;
	    else
	    {  for(c = 0; *(fw->df->name[fw->nf]+c)
			&& ( *(fw->df->name[fw->nf]+c) <= 32
			  || *(fw->df->name[fw->nf]+c) >= 127); c++);
	       if(!WpeIsXwin()) c += 3;
	    }
	    for(MLEN = 1; *(fw->df->name[fw->nf]+c+MLEN)
			&& *(fw->df->name[fw->nf]+c+MLEN) != ' '; MLEN++);
	    MLEN *= 2;
	    file = MALLOC(MLEN * sizeof(char));
	    bgrd = MALLOC(MLEN * sizeof(char));
	    while (e_mshit() != 0)
	    if(sw && (e_mouse.x != xa || e_mouse.y != ya))
	    {  xn = e_mouse.x;  yn = e_mouse.y;
	       if(fw->srcha < 0)
	       {  FLBFFR *b = (FLBFFR *)fw->f->b;
		  if(b->cd->anz > fw->nf)
		  {  while (e_mshit() != 0);
		     FREE (file);  FREE (bgrd);  return(WPE_CR);
		  }
		  for(c = 0; *(fw->df->name[fw->nf]+c)
			&& ( *(fw->df->name[fw->nf]+c) <= 32
			  || *(fw->df->name[fw->nf]+c) >= 127); c++);
		  if(!WpeIsXwin()) c += 3;
		  xdif -= (c + 1);
	       }
	       for (i = 0; i < MLEN/2; i++)
	       {  file[2*i] = *(fw->df->name[fw->nf]+c+i);
		  file[2*i+1] = fw->f->fb->fz.fb;
	       }
	       e_gt_btstr(e_mouse.x-xdif, e_mouse.y, MLEN, bgrd);
	       while (e_mshit() != 0)
	       {  e_pt_btstr(xn-xdif, yn, MLEN, bgrd);
		  xn = e_mouse.x;  yn = e_mouse.y;
		  e_gt_btstr(e_mouse.x-xdif, e_mouse.y, MLEN, bgrd);
		  e_pt_btstr(e_mouse.x-xdif, e_mouse.y, MLEN, file);
		  e_refresh();
	       }
	       e_pt_btstr(xn-xdif, yn, MLEN, bgrd);
	       FREE (file);  FREE (bgrd);
	       return(k);
	    }
	    FREE (file);  FREE (bgrd);
	    if(sw && k == -2) return(AltU);
	    else if(sw && k == -4) return(AltM);
	    else return(WPE_CR);
	 }
      }
      else {  while (e_mshit() != 0);
	      fw->nf = e_mouse.y - fw->ya + fw->ia;  return(0);  }
   }
   else return(MBKEY);
}

/*   mouse slider bar control */
int e_lst_mouse(x, y, n, sw, max, nf)
     int x;
     int y;
     int n;
     int sw;
     int max;
     int nf;
{
   extern struct mouse e_mouse;
   int g[4];  /*  = { 1, 0, 0, 0 };  */
   int inew, iold, nret, frb = e_gt_col(x, y);
   double d;
   if(n < 2) return(nf);
   d = ((double)max)/((double)(n-2));
   g[0] = 1;
   if(sw == 0)
   {  if(e_mouse.x != x) return(nf);
      if(e_mouse.y < y || e_mouse.y >= y+n) return(nf);
      if(e_mouse.y == y) nret= (nf < 1) ? nf : nf-1;
      else if(e_mouse.y == y+n-1) nret= (nf >= max-1) ? nf : nf+1;
      else
      {  nret = (int) ((e_mouse.y-y-1)*d);
	 if(e_gt_char(e_mouse.x, e_mouse.y) == MCA )
	 {  iold = e_mouse.y;
#ifdef NEWSTYLE
	    e_make_xrect_abs(x, iold, x, iold, 1);
	    e_refresh();
#endif
	    fk_mouse(g);
	    for(g[1]=1; g[1] != 0; fk_mouse(g), g[0]=3)
	    {  if((inew = g[3]/8) < y+1) inew = y+1;
	       if(inew > y+n-2) inew = y+n-2;
	       if(iold != inew)
	       {  g[0] = 2; fk_mouse(g);
		  e_pr_char(x, iold, MCI, frb);
		  e_pr_char(x, inew, MCA, frb);
#ifdef NEWSTYLE
		  e_make_xrect(x, y+1, x, y+n-2, 0);
		  e_make_xrect_abs(x, inew, x, inew, 1);
#endif
		  e_refresh();
		  iold = inew;
		  g[0] = 1; fk_mouse(g);
	       };
	    }
	    g[0] = 2;
	    fk_mouse(g);
	    e_pr_char(x, inew, MCI, frb);
	    if(inew-y < 2) nret = 0;
	    else if(inew-y > n - 3) nret = max - 1;
	    else nret = (int) ((inew-y-0.5)*d);
	 }
	 else if(nf < nret)
	 {  nret = (nf+n > max) ? max : nf+n-1;
	    while (e_mshit() != 0);
	 }
	 else
	 {  nret = (nf-n < 0) ? 0 : nf-n+1;
	    while (e_mshit() != 0);
	 }
      }
   }
   else
   {  if(e_mouse.y != y) return(nf);
      if(e_mouse.x < x || e_mouse.x >= x+n) return(nf);
      if(e_mouse.x == x) nret= (nf < 1) ? nf : nf-1;
      else if(e_mouse.x == x+n-1) nret= (nf >= max-1) ? nf : nf+1;
      else
      {  nret = (int) ((e_mouse.x-x-1)*d);
	 if( e_gt_char(e_mouse.x, e_mouse.y) == MCA )
	 {  iold = e_mouse.x;
#ifdef NEWSTYLE
	    e_make_xrect_abs(iold, y, iold, y, 1);
	    e_refresh();
#endif
	    fk_mouse(g);
	    for(g[1]=1; g[1] != 0; fk_mouse(g), g[0]=3)
	    {  if((inew = g[2]/8) < x+1) inew = x+1;
	       if(inew > x+n-2) inew = x+n-2;
	       if(iold != inew)
	       {  g[0] = 2; fk_mouse(g);
		  e_pr_char(iold, y, MCI, frb);
		  e_pr_char(inew, y, MCA, frb);
#ifdef NEWSTYLE
		  e_make_xrect(x+1, y, x+n-2, y, 0);
		  e_make_xrect_abs(inew, y, inew, y, 1);
#endif
		  e_refresh();
		  iold = inew;
		  g[0] = 1; fk_mouse(g);
	       };
	    }
	    g[0] = 2;
	    fk_mouse(g);
	    e_pr_char(inew, y, MCI, frb);
	    if(inew-y < 2) nret = 0;
	    else if(inew-y > n - 3) nret = max - 1;
	    else nret = (int) ((inew-y-0.5)*d);
	 }
	 else if(nf < nret)
	 {  nret = (nf+n > max) ? max : nf+n-1;
	    while (e_mshit() != 0);
	 }
	 else
	 {  nret = (nf-n < 0) ? 0 : nf-n+1;
	    while (e_mshit() != 0);
	 }
      }
   }
   return(nret);
}

/*   mouse window resizer control */
void e_eck_mouse(FENSTER *f, int sw)
{
 int g[4];  /*  = { 3, 1, 0, 0 };  */
 int xold, yold, x, y, xa, xmin = 26, ymin = 3;
 POINT fa, fe;

 e_ed_rahmen(f, 0);
 memcpy(&fa, &f->a, sizeof(POINT));
 memcpy(&fe, &f->e, sizeof(POINT));
 g[0] = 3;
 g[1] = 1;
 fk_mouse(g);
 xold = g[2]/8;
 yold = g[3]/8;
 xa = xold - f->a.x;
 if (f->dtmd == DTMD_FILEDROPDOWN)
  xmin = 15;
 else if (!DTMD_ISTEXT(f->dtmd))
  ymin = 9;
 while(g[1] != 0)
 {
  x = g[2]/8;
  y = g[3]/8;
  if (y < 1)
   y = 1;
  else if (y > MAXSLNS-2)
   y = MAXSLNS-2;
  if (x < 0)
   x = 0;
  else if (x > MAXSCOL-1)
   x = MAXSCOL-1;
  if (xold != x || yold != y)
  {
   xold = x;
   yold = y;
   if (sw == 0)
   {
    x -= xa;
    if (x < 0)
     x = 0;
    else if (x + NUM_COLS_ON_SCREEN > MAXSCOL-1)
     x = MAXSCOL - f->e.x + f->a.x - 1;
    if (f->e.y + y - f->a.y > MAXSLNS-2)
     y = MAXSLNS - f->e.y + f->a.y - 2;
    f->e.x = NUM_COLS_ON_SCREEN + x;
    f->a.x = x;
    f->e.y = f->e.y + y - f->a.y;
    f->a.y = y;
   }
   else if (sw == 1)
   {
    if (x > f->e.x - xmin)
     x = f->e.x - xmin;
    if (y > f->e.y - ymin)
     y = f->e.y - ymin;
    f->a.x = x;
    f->a.y = y;
   }
   else if (sw == 2)
   {
    if (x < f->a.x + xmin)
     x = f->a.x + xmin;
    if (y > f->e.y - ymin)
     y = f->e.y - ymin;
    f->e.x = x;
    f->a.y = y;
   }
   else if (sw == 3)
   {
    if (x < f->a.x + xmin)
     x = f->a.x + xmin;
    if (y < f->a.y + ymin)
     y = f->a.y + ymin;
    f->e.x = x;
    f->e.y = y;
   }
   else if (sw == 4)
   {
    if (x > f->e.x - xmin)
     x = f->e.x - xmin;
    if (y < f->a.y + ymin)
     y = f->a.y + ymin;
    f->a.x = x;
    f->e.y = y;
   }
   g[0] = 2;
   fk_mouse(g);
   f->pic = e_ed_kst(f, f->pic, 0);
   if (f->pic == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
   if (f->dtmd == DTMD_FILEDROPDOWN)
   {
    FLWND *fw = (FLWND*) f->b;
    fw->xa = f->a.x+1;
    fw->xe = f->e.x;
    fw->ya = f->a.y+1;
    fw->ye = f->e.y;
   }
   g[0] = 1;
   fk_mouse(g);
   e_cursor(f, 0);
   e_schirm(f, 0);
   e_refresh();
  }
  g[0] = 3;
  fk_mouse(g);
 }
 if ((memcmp(&fa, &f->a, sizeof(POINT))) ||
   (memcmp(&fe, &f->e, sizeof(POINT))))
  f->zoom = 0;
 e_ed_rahmen(f, 1);
}

/*       Mouse edit window control   */
int e_edt_mouse(int c, FENSTER *f)
{
 int i, ret = 0;
 extern struct mouse e_mouse;
 ECNT *cn = f->ed;

 if (e_mouse.y == 0) return(e_m1_mouse());
 else if (e_mouse.y == MAXSLNS-1) return(e_m3_mouse());
 else if (e_mouse.x >= f->a.x && e_mouse.x <= f->e.x &&
   e_mouse.y >= f->a.y && e_mouse.y <= f->e.y)
 {
  if (e_mouse.x == f->a.x+3 && e_mouse.y == f->a.y) ret = f->ed->edopt & ED_CUA_STYLE ? CF4 : AF3;
  else if (e_mouse.x == f->e.x-3 && e_mouse.y == f->a.y) e_ed_zoom(f);
  else if (e_mouse.x == f->a.x && e_mouse.y == f->a.y) e_eck_mouse(f, 1);
  else if (e_mouse.x == f->e.x && e_mouse.y == f->a.y) e_eck_mouse(f, 2);
  else if (e_mouse.x == f->e.x && e_mouse.y == f->e.y) e_eck_mouse(f, 3);
  else if (e_mouse.x == f->a.x && e_mouse.y == f->e.y) e_eck_mouse(f, 4);
  else if (e_mouse.y == f->a.y) e_eck_mouse(f, 0);
  else if (e_mouse.x == f->a.x && e_mouse.y == f->a.y+2) ret = f->ed->edopt & ED_CUA_STYLE ? AF3 : F4;
  else if (e_mouse.x == f->a.x && e_mouse.y == f->a.y+4) ret = f->ed->edopt & ED_CUA_STYLE ? CF3 : AF4;
  else if (e_mouse.x == f->a.x && e_mouse.y == f->a.y+6) ret = f->ed->edopt & ED_CUA_STYLE ? F3 : CF4;
  else if (f->ins != 8 && e_mouse.x == f->a.x &&
    e_mouse.y == f->a.y+8) ret = f->ed->edopt & ED_CUA_STYLE ? AF2 : F2;
  else if (e_mouse.y == f->e.y && e_mouse.x > f->a.x + 4 &&
    e_mouse.x < f->a.x + 14) ret = AltG;
  else if (e_mouse.y == f->e.y && e_mouse.x == f->a.x + 15 && f->ins != 8)
  {
   if (f->ins & 1) f->ins &= ~1;
   else f->ins |= 1;
   e_pr_filetype(f);
  }
  else if (e_mouse.y == f->e.y && e_mouse.x == f->a.x + 16 && f->ins != 8)
  {
   if (f->ins & 2) f->ins &= ~2;
   else f->ins |= 2;
   e_pr_filetype(f);
  }
  else if (e_mouse.x > f->a.x && e_mouse.x < f->e.x &&
    e_mouse.y > f->a.y && e_mouse.y < f->e.y)
  {
   if (c < -1) ret = e_ccp_mouse(c, f);
   else if (f->dtmd == DTMD_HELP && f->ins == 8 &&
     f->b->b.y == e_mouse.y-f->a.y+NUM_LINES_OFF_SCREEN_TOP-1 &&
     ((i = f->b->b.x) == e_mouse_cursor(f->b, f->s, f)))
    ret = WPE_CR;
   else
#ifdef PROG
#ifdef DEBUGGER
   {
    if (WpeIsProg() && (!strcmp(f->datnam, "Watches") ||
      !strcmp(f->datnam, "Messages") ||
      !strcmp(f->datnam, "Stack")))
     ret = e_d_car_mouse(f);
    else
     e_cur_mouse(f);
   }
#else
   {
    if (WpeIsProg() && !strcmp(f->datnam, "Messages"))
     ret = e_d_car_mouse(f);
    else
     e_cur_mouse(f);
   }
#endif
#else
    e_cur_mouse(f);
#endif
  }
  else if (e_mouse.x == f->e.x && e_mouse.y == f->a.y+1)
  {
   /*changed the while()... to a do...while();  :Mark L*/
   do
   {
    f->b->b.y = f->b->b.y > 0 ? f->b->b.y - 1 : 0;
    f->b->b.x = e_chr_sp(f->b->clsv, f->b, f);
    e_cursor(f, 1);
    e_refresh();
   } while(e_mshit());
  }
  else if (e_mouse.x == f->e.x && e_mouse.y == f->e.y-1)
  {
   /*changed the while()... to a do...while();  :Mark L*/
   do
   {
    f->b->b.y = f->b->b.y < f->b->mxlines - 1 ?
      f->b->b.y + 1 : f->b->mxlines - 1;
    f->b->b.x = e_chr_sp(f->b->clsv, f->b, f);
    e_cursor(f, 1);
    e_refresh();
   } while(e_mshit());
  }
  else if (e_mouse.x == f->e.x &&
    e_mouse.y > f->a.y+1 && e_mouse.y < f->e.y-1)
  {
   f->b->b.y = e_lst_mouse(f->e.x, f->a.y+1, f->e.y-f->a.y-1, 0,
     f->b->mxlines, f->b->b.y);
   e_cursor(f, 1);
   e_refresh();
  }
  else if (e_mouse.y == f->e.y && e_mouse.x == f->a.x+19)
  {
   while (e_mshit())
   {
    f->b->b.x = f->b->b.x > 0 ? f->b->b.x - 1 : 0;
    e_cursor(f, 1);
    e_refresh();
   }
  }
  else if (e_mouse.y == f->e.y && e_mouse.x == f->e.x-2)
  {
   while (e_mshit())
   {
    f->b->b.x = f->b->b.x < f->b->bf[f->b->b.y].len ?
      f->b->b.x + 1 : f->b->bf[f->b->b.y].len;
    e_cursor(f, 1);
    e_refresh();
   }
  }
  else if (e_mouse.y == f->e.y &&
    e_mouse.x > f->a.x+19 && e_mouse.x < f->e.x-2)
  {
   f->b->b.x = e_lst_mouse(f->a.x+19, f->e.y, f->e.x-f->a.x-20, 1,
     f->b->mx.x, NUM_COLS_OFF_SCREEN_LEFT);
   e_cursor(f, 1);
   e_refresh();
  }
 }
 else
 {
  for (i = cn->mxedt; i > 0; i--)
  {
   if (e_mouse.x >= cn->f[i]->a.x && e_mouse.x <= cn->f[i]->e.x &&
     e_mouse.y >= cn->f[i]->a.y && e_mouse.y <= cn->f[i]->e.y)
   {
    ret = cn->edt[i] < 10 ? Alt1-1+cn->edt[i] : 1014+cn->edt[i];
    break;
   }
  }
 }
 while (e_mshit()!= 0)
  ;
 return(ret);
}

int e_mouse_cursor(BUFFER *b, SCHIRM *s, FENSTER *f)
{
 extern struct mouse e_mouse;

 b->b.x = e_mouse.x-f->a.x+s->c.x-1;
 b->b.y = e_mouse.y-f->a.y+s->c.y-1;
 if (b->b.y < 0) b->b.y = 0;
 else if (b->b.y >= b->mxlines) b->b.y = b->mxlines - 1;
 return(b->b.x = e_chr_sp(b->b.x, b, f));
}

/*  Copy, Cut and Paste functions    */
int e_ccp_mouse(int c, FENSTER *f)
{
 BUFFER *b = f->ed->f[f->ed->mxedt]->b;
 SCHIRM *s = f->ed->f[f->ed->mxedt]->s;

 while (e_mshit() != 0)
  ;
 if (c == -2)
 {
  e_mouse_cursor(b, s, f);
  return((bioskey() & 8) ? AltEin : ShiftEin);
 }
 else if (c == -4)
 {
  return((bioskey() & 3) ? ShiftDel : ((bioskey() & 8) ? AltDel : CEINFG));
 }
 else return(0);
}

/*       Mouse cursor in edit window control   */
void e_cur_mouse(f)
     FENSTER *f;
{
   BUFFER *b = f->ed->f[f->ed->mxedt]->b;
   SCHIRM *s = f->ed->f[f->ed->mxedt]->s;
   POINT bs;
   bs.x = b->b.x;  bs.y = b->b.y;
   e_mouse_cursor(b, s, f);
   if((bioskey() & 3) == 0)
   {  if(b->b.x == bs.x && b->b.y == bs.y && f->dtmd != DTMD_HELP)
      {  if(s->mark_begin.y == b->b.y && s->mark_end.y == b->b.y
		&& s->mark_begin.x <= b->b.x && s->mark_end.x > b->b.x)
	 {  s->mark_begin.x = 0;  s->mark_end.x = b->bf[b->b.y].len;  }
	 else
	 {  s->mark_begin.y = s->mark_end.y = b->b.y;
	    for(s->mark_begin.x = b->b.x; s->mark_begin.x > 0
			&& isalnum1(b->bf[b->b.y].s[s->mark_begin.x-1]); s->mark_begin.x--);
	    for(s->mark_end.x = b->b.x; s->mark_end.x < b->bf[b->b.y].len
			&& isalnum1(b->bf[b->b.y].s[s->mark_end.x]); s->mark_end.x++);
	 }
	 e_schirm(f, 1);
      }
      s->ks.x = b->b.x;  s->ks.y = b->b.y;
   }
   else
   {  if(s->mark_end.y < b->b.y || ( s->mark_end.y == b->b.y && s->mark_end.x <= b->b.x))
      {  s->mark_end.x = b->b.x;  s->mark_end.y = b->b.y;
	 s->ks.x = s->mark_begin.x;  s->ks.y = s->mark_begin.y;
      }
      else if(s->mark_begin.y > b->b.y || ( s->mark_begin.y == b->b.y && s->mark_begin.x >= b->b.x))
      {  s->mark_begin.x = b->b.x;  s->mark_begin.y = b->b.y;
	 s->ks.x = s->mark_end.x;  s->ks.y = s->mark_end.y;
      }
      else if(s->mark_end.y < bs.y || ( s->mark_end.y == bs.y && s->mark_end.x <= bs.x))
      {  s->mark_begin.x = b->b.x;  s->mark_begin.y = b->b.y;
	 s->ks.x = s->mark_end.x;  s->ks.y = s->mark_end.y;
      }
      else
      {  s->mark_end.x = b->b.x;  s->mark_end.y = b->b.y;
	 s->ks.x = s->mark_begin.x;  s->ks.y = s->mark_begin.y;
      }
   }
   WpeMouseChangeShape(WpeSelectionShape);
   while(e_mshit() != 0)
   {  bs.x = b->b.x;  bs.y = b->b.y;
      e_mouse_cursor(b, s, f);
      if(b->b.x < 0) b->b.x = 0;
      else if(b->b.x > b->bf[b->b.y].len) b->b.x = b->bf[b->b.y].len;
      if(b->b.x != bs.x || b->b.y != bs.y)
      {  if(s->ks.y < b->b.y || ( s->ks.y == b->b.y && s->ks.x <= b->b.x))
	 {  s->mark_end.x = b->b.x;  s->mark_end.y = b->b.y;
	    s->mark_begin.x = s->ks.x;  s->mark_begin.y = s->ks.y;
	 }
	 else
	 {  s->mark_begin.x = b->b.x;  s->mark_begin.y = b->b.y;
	    s->mark_end.x = s->ks.x;  s->mark_end.y = s->ks.y;
	 }
      }
      e_cursor(f, 1);
      e_schirm(f, 1);
      e_refresh();
   }
   s->ks.x = b->b.x;  s->ks.y = b->b.y;
   WpeMouseRestoreShape();
   e_cursor(f, 1);
}

int e_opt_ck_mouse(xa, ya, md)
     int xa;
     int ya;
     int md;
{
   extern struct mouse e_mouse;
   if(e_mouse.x < xa-2 || e_mouse.x > xa+25
			|| e_mouse.y < ya-1 || e_mouse.y > ya+18) return(WPE_CR);
   if(e_mouse.x >= xa && e_mouse.x < xa+24
			&& e_mouse.y > ya && e_mouse.y < ya+17)
   return(1000+(e_mouse.y-ya-1)*16 + (e_mouse.x - xa)/3);
   else
   return(0);
}

int e_opt_cw_mouse(xa, ya, md)
     int xa;
     int ya;
     int md;
{
   extern struct mouse e_mouse;
   if(e_mouse.y == 0 || e_mouse.y == MAXSLNS-1) return(WPE_ESC);
   if(e_mouse.y == 1 && e_mouse.x == 3) return(WPE_ESC);
   if(e_mouse.x >= xa-30 && e_mouse.x <= xa-3
			&& e_mouse.y >= ya && e_mouse.y <= ya+19) return(WPE_CR);
   if(e_mouse.x >= 1 && e_mouse.x <= 33
			&& e_mouse.y >= 2 && e_mouse.y <= 21)
   {  if(md == 1) return(e_opt_bs_mouse_1());
      else if(md == 2) return(e_opt_bs_mouse_2());
      else if(md == 3) return(e_opt_bs_mouse_3());
      else return(e_opt_bs_mouse());
   }
   if(e_mouse.x > xa && e_mouse.x < xa+12
			&& e_mouse.y > ya && e_mouse.y < ya+20)
   return(374+e_mouse.y-ya);
   else
   return(0);
}

int e_opt_bs_mouse_1()
{
   extern struct mouse e_mouse;  /*  return = sw + 375;  */
   int sw = 0;
   if(e_mouse.y < 2 || e_mouse.y > 21 ||
	   e_mouse.x < 2 || e_mouse.x > 33) return(0);
   else if(e_mouse.y == 2 && e_mouse.x == 6) sw = 1;
   else if(e_mouse.y == 2 && e_mouse.x >= 17 && e_mouse.x <= 28) sw = 3;
   else if(e_mouse.y == 21 && e_mouse.x >= 5 && e_mouse.x <= 10) sw = 1;
   else if(e_mouse.y == 2 || e_mouse.y == 21) sw = 2;
   else if((e_mouse.y == 3 || e_mouse.y == 5)
		&& e_mouse.x >= 18 && e_mouse.x <= 27) sw = 0;
   else if(e_mouse.y == 4 && (e_mouse.x == 18 || e_mouse.x == 27)) sw = 0;
   else if(e_mouse.y == 4 && e_mouse.x == 20) sw = 1;
   else if(e_mouse.y == 4 && e_mouse.x > 18 && e_mouse.x < 27) sw = 2;
   else sw = 4;
   return(sw + 375);
}

int e_opt_bs_mouse_2()
{
   extern struct mouse e_mouse;  /*  return = sw + 375;  */
   int sw = 0;
   if(e_mouse.y < 2 || e_mouse.y > 21 ||
	   e_mouse.x < 1 || e_mouse.x > 32) return(0);
   else if(e_mouse.y == 2 && e_mouse.x == 4) sw = 1;
   else if(e_mouse.y == 2 || e_mouse.y == 21 ||
	   	e_mouse.x == 1 || e_mouse.x == 32) sw = 0;
   else if(e_mouse.y == 4 && e_mouse.x == 5) sw = 3;
   else if(e_mouse.y == 5 && e_mouse.x >= 5 && e_mouse.x <= 24 ) sw = 5;
   else if(e_mouse.y == 7 && e_mouse.x == 5) sw = 3;
   else if(e_mouse.y == 8 && e_mouse.x >= 5 && e_mouse.x <= 24 ) sw = 4;
   else if(e_mouse.y == 10 && e_mouse.x == 5) sw = 3;
   else if(e_mouse.y == 11 && e_mouse.x >= 5 && e_mouse.x <= 20 ) sw = 7;
   else if(e_mouse.y == 12 && e_mouse.x >= 5 && e_mouse.x <= 20 ) sw = 8;
   else if(e_mouse.y == 13 && e_mouse.x >= 5 && e_mouse.x <= 20 ) sw = 6;
   else if(e_mouse.y == 15 && e_mouse.x == 5) sw = 3;
   else if(e_mouse.y == 16 && e_mouse.x >= 5 && e_mouse.x <= 24 ) sw = 11;
   else if(e_mouse.y == 17 && e_mouse.x == 10) sw = 10;
   else if(e_mouse.y == 17 && e_mouse.x >= 5 && e_mouse.x <= 24 ) sw = 9;
   else if(e_mouse.y == 19 && e_mouse.x == 7) sw = 13;
   else if(e_mouse.y == 19 && e_mouse.x >= 6 && e_mouse.x <= 13 ) sw = 12;
   else if(e_mouse.y == 19 && e_mouse.x >= 19 && e_mouse.x <= 26 ) sw = 14;
   else sw = 2;
   return(sw + 375);
}

int e_opt_bs_mouse_3()
{
   extern struct mouse e_mouse;  /*  return = sw + 375;  */
   int sw = 0;
   if(e_mouse.y < 2 || e_mouse.y > 21 ||
	   e_mouse.x < 1 || e_mouse.x > 32) return(0);
   else if(e_mouse.y == 5) sw = 3;
   else if(e_mouse.y == 9) sw = 1;
   else if(e_mouse.y == 11) sw = 2;
   else if(e_mouse.y == 13) sw = 4;
   return(sw+375);
}

int e_opt_bs_mouse()
{
   extern struct mouse e_mouse;
   int sw = 0;
   if(e_mouse.y < 2 || e_mouse.y > 21 ||
	   e_mouse.x < 1 || e_mouse.x > 32) return(0);
   else if(e_mouse.y == 2 && (e_mouse.x == 4 || e_mouse.x == 29 )) sw = 1;
   else if(e_mouse.x == 32 && e_mouse.y > 2 && e_mouse.y < 21 ) sw = 5;
   else if(e_mouse.y == 21 && e_mouse.x > 20 && e_mouse.y < 32 ) sw = 5;
   else if(e_mouse.y == 2 || e_mouse.y == 21 ||
	   e_mouse.x == 1 || e_mouse.x == 32) sw = 0;
   else if(e_mouse.y == 6 && e_mouse.x >= 4 && e_mouse.x <= 30 ) sw = 3;
   else if(e_mouse.y == 7 && e_mouse.x >= 14 && e_mouse.x <= 21 ) sw = 4;
   else if(e_mouse.y == 10 && e_mouse.x >= 4 && e_mouse.x <= 16 ) sw = 6;
   else if(e_mouse.y == 11 && e_mouse.x >= 15 && e_mouse.x <= 20 ) sw = 8;
   else if(e_mouse.y == 13 && e_mouse.x >= 4 && e_mouse.x <= 17 ) sw = 7;
   else if(e_mouse.y == 16 && e_mouse.x >= 4 && e_mouse.x <= 25 ) sw = 9;
   else if(e_mouse.y == 17 && e_mouse.x >= 4 && e_mouse.x <= 23 ) sw = 10;
   else sw = 2;
   return(sw+375);
}

int e_data_ein_mouse(f)
     FENSTER *f;
{
   extern struct mouse e_mouse;
   FLWND *fw = (FLWND *)f->b;
   ECNT *cn = f->ed;
   int i, c = 0;
   if(e_mouse.y == 0) return(AltBl);
   else if(e_mouse.y == MAXSLNS-1) return(e_m3_mouse());
   else if(e_mouse.x < f->a.x || e_mouse.x > f->e.x
	   || e_mouse.y < f->a.y || e_mouse.y > f->e.y)
   {  for(i = cn->mxedt; i > 0; i--)
      {  if(e_mouse.x >= cn->f[i]->a.x && e_mouse.x <= cn->f[i]->e.x
	  && e_mouse.y >= cn->f[i]->a.y && e_mouse.y <= cn->f[i]->e.y)
	 return(cn->edt[i] < 10 ? Alt1-1+cn->edt[i] : 1014+cn->edt[i]);
      }
   }
   else if(e_mouse.x == f->a.x+3 && e_mouse.y == f->a.y) c = WPE_ESC;
   else if(e_mouse.x == f->e.x-3 && e_mouse.y == f->a.y) e_ed_zoom(f);
   else if(e_mouse.x == f->a.x && e_mouse.y == f->a.y) e_eck_mouse(f, 1);
   else if(e_mouse.x == f->e.x && e_mouse.y == f->a.y) e_eck_mouse(f, 2);
   else if(e_mouse.x == f->e.x && e_mouse.y == f->e.y) e_eck_mouse(f, 3);
   else if(e_mouse.x == f->a.x && e_mouse.y == f->e.y) e_eck_mouse(f, 4);
   else if(e_mouse.y == f->a.y && e_mouse.x > f->a.x
				&& e_mouse.x < f->e.x) e_eck_mouse(f, 0);
   else if(e_mouse.y >= fw->ya && e_mouse.y <= fw->ye &&
	e_mouse.x >= fw->xa && e_mouse.x <= fw->xe)  c = AltF;
   else if(e_mouse.y == f->e.y-2 && e_mouse.x >= f->e.x-9
			&& e_mouse.x <= f->e.x-3) c = WPE_ESC;
   else if((f->ins < 4 || f->ins == 7) && e_mouse.y == f->e.y-4
	    && e_mouse.x >= f->e.x-9 && e_mouse.x <= f->e.x-3) c = AltS;
   else if(f->ins > 3 && e_mouse.y == f->e.y-8 && e_mouse.x >= f->e.x-9
			&& e_mouse.x <= f->e.x-3) c = AltA;
   else if(f->ins > 3 && e_mouse.y == f->e.y-6 && e_mouse.x >= f->e.x-9
			&& e_mouse.x <= f->e.x-3) c = AltE;
   else if(f->ins > 3 && e_mouse.y == f->e.y-4 && e_mouse.x >= f->e.x-9
			&& e_mouse.x <= f->e.x-3) c = AltD;
   else if(f->ins == 4 && e_mouse.y == f->e.y-10 && e_mouse.x >= f->e.x-9
			&& e_mouse.x <= f->e.x-3) c = AltO;
   else c = AltF;
   while (e_mshit() != 0);
   return(c);
}

void e_opt_eck_mouse(o)
     W_OPTSTR *o;
{
   int g[4];
   int xold, yold, x, y, xa;
   PIC *pic;
   e_std_rahmen(o->xa, o->ya, o->xe, o->ye, o->name, 0, o->frt, o->frs);
#ifndef NEWSTYLE
#if !defined(DJGPP)
   if(!WpeIsXwin()) pic = e_open_view(o->xa, o->ya, o->xe, o->ye, 0, 2);
   else 
#endif
	pic = e_open_view(o->xa, o->ya, o->xe-2, o->ye-1, 0, 2);
#else
   pic = e_open_view(o->xa, o->ya, o->xe, o->ye, 0, 2);
#endif
   g[0] = 3;  g[1] = 1;
   fk_mouse(g);
   xold = g[2]/8;
   yold = g[3]/8;
   xa = xold - o->xa;
   while(g[1] != 0)
   {  x = g[2]/8;
      y = g[3]/8;
      if(y < 1) y = 1;
      else if(y > MAXSLNS-2) y = MAXSLNS-2;
      if(x < 0) x = 0;
      else if(x > MAXSCOL-1) x = MAXSCOL-1;
      if(xold != x || yold != y)
      {  xold = x;  yold = y;
	 x -= xa;
	 if(x < 0) x = 0;
	 else if(x + o->xe - o->xa > MAXSCOL-1)
	 x = MAXSCOL - o->xe + o->xa - 1;
	 if(o->ye + y - o->ya > MAXSLNS-2)
	 y = MAXSLNS - o->ye + o->ya - 2;
	 o->xe = o->xe - o->xa + x; o->xa = x;
	 o->ye = o->ye + y - o->ya;  o->ya = y;
	 g[0] = 2;  fk_mouse(g);
	 o->pic = e_change_pic(o->xa, o->ya, o->xe, o->ye, o->pic, 1, o->frt);
	 if(o->pic == NULL)  e_error(e_msg[ERR_LOWMEM], 1, o->f->fb);
	 g[0] = 1;  fk_mouse(g);
	 pic->a.x = o->xa;  pic->a.y = o->ya;
	 pic->e.x = o->xe;  pic->e.y = o->ye;
	 e_close_view(pic, 2);
      }
      g[0] = 3;  fk_mouse(g);
   }
   pic->a.x = o->xa;  pic->a.y = o->ya;
   pic->e.x = o->xe;  pic->e.y = o->ye;
   e_close_view(pic, 1);
   e_std_rahmen(o->xa, o->ya, o->xe, o->ye, o->name, 1, o->frt, o->frs);
}

int e_opt_mouse(W_OPTSTR *o)
{
 extern struct mouse e_mouse;
 int c;

 if (e_mouse.y < o->ya || e_mouse.y >= o->ye ||
   e_mouse.x <= o->xa || e_mouse.x >= o->xe)
  return(-1);
 else if (e_mouse.y == o->ya)
 {
  if (e_mouse.x == o->xa+3)
  {  while(e_mshit());  return(WPE_ESC);  }
  else
  {  e_opt_eck_mouse(o);  return(-1);  }
 }
 while (e_mshit())
  ;
 if ((c = e_get_opt_sw(0, e_mouse.x, e_mouse.y, o))) return(c);
 else return(-1);
}

#endif

