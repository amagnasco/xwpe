/* we_xterm.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#ifndef NO_XWINDOWS

#include "edit.h"

/* partial conversion in place */
#include "WeXterm.h"

#ifndef XWPE_DLL
#define WpeDllInit WpeXtermInit
#endif

int e_X_sw_color(void);
int fk_show_cursor(void);
int e_ini_size(void);
int e_x_getch(void);
int fk_x_mouse(int *g);
int e_x_refresh(void);
int fk_x_locate(int x, int y);
int fk_x_cursor(int x);
int e_x_sys_ini(void);
int e_x_sys_end(void);
int fk_x_putchar(int c);
int x_bioskey(void);
int e_x_system(const char *exe);
int e_x_cp_X_to_buffer(FENSTER *f);
int e_x_copy_X_buffer(FENSTER *f);
int e_x_paste_X_buffer(FENSTER *f);
int e_x_change(PIC *pic);
int e_x_repaint_desk(FENSTER *f);
void e_setlastpic(PIC *pic);
int e_make_xr_rahmen(int xa, int ya, int xe, int ye, int sw);
int e_x_kbhit(void);

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>

#include <X11/keysym.h>
#include <X11/cursorfont.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>


#ifndef NEWSTYLE
#define NOXCACHE
#endif

#define BUFSIZE 80

extern char *e_tmp_dir;

/*  global constants  */

/*  for TextSchirm (text screen)   */

extern char *altschirm;
#ifdef NEWSTYLE
extern char *extbyte, *altextbyte;
#endif
int old_cursor_x = 0, old_cursor_y = 0, cur_on = 1;
extern PIC *e_X_l_pic;

extern struct mouse e_mouse;

int WpeDllInit(int *argc, char **argv)
{
 e_s_u_clr = e_s_x_clr;
 e_n_u_clr = e_n_x_clr;
 e_frb_u_menue = e_frb_x_menue;
 e_pr_u_col_kasten = e_pr_x_col_kasten;
 fk_u_cursor = fk_x_cursor;
 fk_u_locate = fk_x_locate;
 e_u_refresh = e_x_refresh;
 e_u_getch = e_x_getch;
 u_bioskey = x_bioskey;
 e_u_sys_ini = e_x_sys_ini;
 e_u_sys_end = e_x_sys_end;
 e_u_system = e_x_system;
 fk_u_putchar = fk_x_putchar;
 fk_mouse = fk_x_mouse;
 e_u_cp_X_to_buffer = e_x_cp_X_to_buffer;
 e_u_copy_X_buffer = e_x_copy_X_buffer;
 e_u_paste_X_buffer = e_x_paste_X_buffer;
 e_u_kbhit = e_x_kbhit;
 e_u_change = e_x_change;
 e_u_ini_size = e_ini_size;
 e_u_setlastpic = e_setlastpic;
 WpeMouseChangeShape = (void (*)(WpeMouseShape))WpeNullFunction;
 WpeMouseRestoreShape = (void (*)(WpeMouseShape))WpeNullFunction;
/* WpeMouseChangeShape = WpeXMouseChangeShape;
 WpeMouseRestoreShape = WpeXMouseRestoreShape;*/
 WpeDisplayEnd = WpeNullFunction;
 e_u_switch_screen = WpeZeroFunction;
 e_u_d_switch_out = (int (*)(int sw))WpeZeroFunction;
 MCI = 2;
 MCA = 1;
#ifdef NEWSTYLE
 RD1 = ' ';
 RD2 = ' ';
 RD3 = ' ';
 RD4 = ' ';
 RD5 = ' ';
 RD6 = ' ';
 RE1 = ' ';
 RE2 = ' ';
 RE3 = ' ';
 RE4 = ' ';
 RE5 = ' ';
 RE6 = ' ';
#else
 RD1 = 13;
 RD2 = 12;
 RD3 = 14;
 RD4 = 11;
 RD5 = 18;
 RD6 = 25;
 RE1 = '.';
 RE2 = '.';
 RE3 = '.';
 RE4 = '.';
 RE5 = '.';
 RE6 = ':';
#endif
 WBT = 1;
 ctree[0] = "\016\022\030";
 ctree[1] = "\016\022\022";
 ctree[2] = "\016\030\022";
 ctree[3] = "\025\022\022";
 ctree[4] = "\016\022\022";
 WpeXInit(argc, argv);
 return 0;
}

#ifdef NEWSTYLE
#ifdef NOXCACHE

#define e_print_xrect(x, y, n)                                               \
  if(extbyte[n])							     \
  {									     \
   XSetForeground(WpeXInfo.display, WpeXInfo.gc, 						     \
			!(extbyte[n] & 16) ? WpeXInfo.colors[0] : WpeXInfo.colors[15]);\
   if(extbyte[n] & 2)							     \
	XDrawLine(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*((x)+1)-1, 		     \
		WpeXinfo.font_height*(y), WpeXInfo.font_width*((x)+1)-1, 			     \
					   WpeXInfo.font_height*((y)+1)-1);	     \
   if(extbyte[n] & 4)							     \
   	XDrawLine(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*(x), 			     \
		WpeXInfo.font_height*((y)+1)-1, 					     \
		WpeXInfo.font_width*((x)+1)-1, WpeXInfo.font_height*((y)+1)-1);		     \
									     \
   XSetForeground(WpeXInfo.display, WpeXInfo.gc, 						     \
		!(extbyte[n] & 16) ? WpeXInfo.colors[15] : WpeXInfo.colors[0]);	     \
   if(extbyte[n] & 8)							     \
	XDrawLine(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*(x), 			     \
		WpeXInfo.font_height*(y), WpeXInfo.font_width*(x), 				     \
		WpeXInfo.font_height*((y)+1)-1);					     \
   if(extbyte[n] & 1)							     \
	XDrawLine(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*(x), 			     \
		WpeXInfo.font_height*(y), WpeXInfo.font_width*((x)+1)-1, 			     \
		WpeXInfo.font_height*(y));					     \
  }


#else				/* cached  a.r. */

#define WPE_MAXSEG 1000
int nseg[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
XSegment seg[8][WPE_MAXSEG];
int scol[8] = { 0, 0, 15, 15, 15, 15, 0, 0 };

int e_XLookupString(XKeyEvent *event, char *buffer_return, int buffer_size,
		    KeySym *keysym_return, XComposeStatus *status)
{
    static int first = 1;
    static XIC xic;
    static XIM xim;

    if (first) {
	if (!XSetLocaleModifiers(""))
	    XSetLocaleModifiers("@im=none");
	xim = XOpenIM(event->display, NULL, NULL, NULL);
	xic = XCreateIC(xim,
			XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
			XNClientWindow, WpeXInfo.window, NULL);
	first = 0;
    }
    if (xic) {
	if (XFilterEvent((XEvent*)event, WpeXInfo.window))
	    return (0);

	return (XmbLookupString(xic, event, buffer_return, buffer_size,
				keysym_return, NULL));
    }

    return (XLookupString(event, buffer_return, buffer_size,
			  keysym_return, status));
}

void e_flush_xrect()
{
 int i;

 for (i = 0; i < 8; i++)
  if (nseg[i])
  {
   XSetForeground(WpeXInfo.display, WpeXInfo.gc, WpeXInfo.colors[scol[i]]);
   XDrawSegments(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, seg[i], nseg[i]);
   nseg[i] = 0;
  }
}

void e_print_xrect(int x, int y, int n)
{
 int c = extbyte[n] & 16 ? 4 : 0;

 if (extbyte[n])
 {
  if ((nseg[0] > WPE_MAXSEG) || (nseg[1] > WPE_MAXSEG) || (nseg[2] > WPE_MAXSEG) ||
    (nseg[3] > WPE_MAXSEG) || (nseg[4] > WPE_MAXSEG) || (nseg[5] > WPE_MAXSEG) ||
    (nseg[6] > WPE_MAXSEG) || (nseg[7] > WPE_MAXSEG))
   e_flush_xrect();
  if (extbyte[n] & 2)
  {
   seg[c][nseg[c]].x1 = WpeXInfo.font_width*((x)+1)-1;
   seg[c][nseg[c]].y1 = WpeXInfo.font_height*(y);
   seg[c][nseg[c]].x2 = WpeXInfo.font_width*((x)+1)-1;
   seg[c][nseg[c]].y2 = WpeXInfo.font_height*((y)+1)-1;
   nseg[c]++;
  }
  if (extbyte[n] & 4)
  {
   seg[c+1][nseg[c+1]].x1 = WpeXInfo.font_width*(x);
   seg[c+1][nseg[c+1]].y1 = WpeXInfo.font_height*((y)+1)-1;
   seg[c+1][nseg[c+1]].x2 = WpeXInfo.font_width*((x)+1)-1;
   seg[c+1][nseg[c+1]].y2 = WpeXInfo.font_height*((y)+1)-1;
   nseg[c+1]++;
  }
  if (extbyte[n] & 8)
  {
   seg[c+2][nseg[c+2]].x1 = WpeXInfo.font_width*(x);
   seg[c+2][nseg[c+2]].y1 = WpeXInfo.font_height*(y);
   seg[c+2][nseg[c+2]].x2 = WpeXInfo.font_width*(x);
   seg[c+2][nseg[c+2]].y2 = WpeXInfo.font_height*((y)+1)-1;
   nseg[c+2]++;
  }
  if (extbyte[n] & 1)
  {
   seg[c+3][nseg[c+3]].x1 = WpeXInfo.font_width*(x);
   seg[c+3][nseg[c+3]].y1 = WpeXInfo.font_height*(y);
   seg[c+3][nseg[c+3]].x2 = WpeXInfo.font_width*((x)+1)-1;
   seg[c+3][nseg[c+3]].y2 = WpeXInfo.font_height*(y);
   nseg[c+3]++;
  }
 }
}
#endif

#endif

int fk_show_cursor()
{
 int x;

 if (!cur_on)
  return(0);
 x = 2 * old_cursor_x + 2 * MAXSCOL * old_cursor_y;
 if (x > 0)
 {
  XSetForeground(WpeXInfo.display, WpeXInfo.gc,
    WpeXInfo.colors[schirm[x + 1] % 16]);
  XSetBackground(WpeXInfo.display, WpeXInfo.gc,
    WpeXInfo.colors[schirm[x + 1] / 16]);
  XDrawImageString(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
    WpeXInfo.font_width*old_cursor_x,
    WpeXInfo.font_height*(old_cursor_y+1) - WpeXInfo.font->max_bounds.descent,
    schirm + x, 1);
#ifdef NEWSTYLE
  e_print_xrect(old_cursor_x, old_cursor_y, x/2);
#ifndef NOXCACHE
  e_flush_xrect();
#endif
#endif
 }
 x = 2 * cur_x + 2 * MAXSCOL * cur_y;

 XSetForeground(WpeXInfo.display, WpeXInfo.gc,
   WpeXInfo.colors[schirm[x + 1] / 16]);
 XSetBackground(WpeXInfo.display, WpeXInfo.gc,
   WpeXInfo.colors[schirm[x + 1] % 16]);
 XDrawImageString(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
   WpeXInfo.font_width * cur_x,
   WpeXInfo.font_height * (cur_y + 1) - WpeXInfo.font->max_bounds.descent,
   schirm + x, 1);
 old_cursor_x = cur_x;
 old_cursor_y = cur_y;
 return(cur_on);
}

int e_ini_size()
{
 old_cursor_x = cur_x;
 old_cursor_y = cur_y;

 if (schirm)
  FREE(schirm);
 if(altschirm)
  FREE(altschirm);
 schirm = MALLOC(2 * MAXSCOL * MAXSLNS);
 altschirm = MALLOC(2 * MAXSCOL * MAXSLNS);
#ifdef NEWSTYLE
 if (extbyte)
  FREE(extbyte);
 if (altextbyte)
  FREE(altextbyte);
 extbyte = MALLOC(MAXSCOL * MAXSLNS);
 altextbyte = MALLOC(MAXSCOL * MAXSLNS);
 if (!schirm || !altschirm || !extbyte || !altextbyte)
  return(-1);
#else
 if(!schirm || !altschirm)
  return(-1);
#endif
 return(0);
}

#define A_Normal 	16
#define A_Reverse 	1
#define A_Standout	1
#define A_Underline	1
#define A_Bold		16

int e_X_sw_color()
{
 FARBE *fb = WpeEditor->fb;
 fb->er = e_n_clr(A_Normal);
 fb->et = e_n_clr(A_Normal);
 fb->ez = e_n_clr(A_Reverse);
 fb->es = e_n_clr(A_Normal);
 fb->em = e_n_clr(A_Standout);
 fb->ek = e_n_clr(A_Underline);
 fb->nr = e_n_clr(A_Standout);
 fb->nt = e_n_clr(A_Reverse);
 fb->nz = e_n_clr(A_Normal);
 fb->ns = e_n_clr(A_Bold);
 fb->mr = e_n_clr(A_Standout);
 fb->mt = e_n_clr(A_Standout);
 fb->mz = e_n_clr(A_Normal);
 fb->ms = e_n_clr(A_Normal);
 fb->fr = e_n_clr(A_Normal);
 fb->ft = e_n_clr(A_Normal);
 fb->fz = e_n_clr(A_Standout);
 fb->fs = e_n_clr(A_Standout);
 fb->of = e_n_clr(A_Standout);
 fb->df = e_n_clr(A_Normal);
 fb->dc = 0x02;
#ifdef DEBUGGER
 fb->db = e_n_clr(A_Standout);
 fb->dy = e_n_clr(A_Standout);
#endif
 return(0);
}

int e_x_refresh()
{
#ifndef NOXCACHE				/* a.r. */
#define STRBUFSIZE 1024
   unsigned long oldback = 0, oldfore = 0;
   static char stringbuf[STRBUFSIZE];
   int stringcount = 0, oldI = 0, oldX = 0, oldY = 0, oldJ = 0;
#endif
   int i, j, x, y, cur_tmp = cur_on;
   fk_cursor(0);
   for(i = 0; i < MAXSLNS; i++)
   for(j = 0; j < MAXSCOL; j++)
   {  y = j + MAXSCOL*i;
      x = 2*y;
#ifdef NEWSTYLE
      if(schirm[x] != altschirm[x] || schirm[x+1] != altschirm[x+1]
		|| extbyte[y] != altextbyte[y])
#else
      if(schirm[x] != altschirm[x] || schirm[x+1] != altschirm[x+1])
#endif
      {
#ifdef NOXCACHE
	 XSetForeground(WpeXInfo.display, WpeXInfo.gc, WpeXInfo.colors[schirm[x+1] % 16]);
	 XSetBackground(WpeXInfo.display, WpeXInfo.gc, WpeXInfo.colors[schirm[x+1] / 16]);
	 XDrawImageString(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*j,
    		WpeXInfo.font_height*(i+1) - WpeXInfo.font->max_bounds.descent,
							schirm + x, 1);
#else
	 if (   oldback != WpeXInfo.colors[schirm[x+1] / 16]  	/* a.r. */
	     || oldfore != WpeXInfo.colors[schirm[x+1] % 16]
	     || i != oldI
	     || j > oldJ+1	/* is there a more elegant solution? */
	     || stringcount >= STRBUFSIZE
            )
	   {
	        XDrawImageString(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
		    		 oldX, oldY, stringbuf, stringcount);
	        oldback = WpeXInfo.colors[schirm[x+1] / 16];
	        oldfore = WpeXInfo.colors[schirm[x+1] % 16];
	 	XSetForeground(WpeXInfo.display, WpeXInfo.gc, oldfore );
	 	XSetBackground(WpeXInfo.display, WpeXInfo.gc, oldback );
		oldX = WpeXInfo.font_width*j;
    		oldY = WpeXInfo.font_height*(i+1) - WpeXInfo.font->max_bounds.descent;
		oldI = i;
		stringcount = 0;
		stringbuf[stringcount++] = schirm[x];
	   }
	 else
		stringbuf[stringcount++] = schirm[x];
#endif
#ifndef NEWSTYLE
	 if(schirm[x] == 16)
	 {  XFillRectangle(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*j,
    		WpeXInfo.font_height*(i), WpeXInfo.font_width,
			(WpeXInfo.font_height + WpeXInfo.font->max_bounds.descent)/2);
	 }
	 else if(schirm[x] == 20)
	 {  XFillRectangle(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, WpeXInfo.font_width*j,
    		(int)(WpeXInfo.font_height*(i+1./2)), WpeXInfo.font_width,
			(WpeXInfo.font_height + WpeXInfo.font->max_bounds.descent)/2);
	 }
#endif
	 altschirm[x] = schirm[x];
	 altschirm[x+1] = schirm[x+1];
#ifdef NEWSTYLE
	 e_print_xrect(j, i, y);
	 altextbyte[y] = extbyte[y];
#endif
#ifndef NOXCACHE
	 oldJ = j;
#endif
      }
   }
#ifndef NOXCACHE
   XDrawImageString(WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
		    oldX,
    		    oldY,
		    stringbuf,
		    stringcount);
#ifdef NEWSTYLE
   e_flush_xrect();
#endif
#endif
   fk_cursor(cur_tmp);
   fk_show_cursor();
   XFlush(WpeXInfo.display);
   return(0);
}

int e_x_change(PIC *pic)
{
 XEvent report;
 XExposeEvent *expose_report;
 KeySym keysym;
 unsigned char buffer[BUFSIZE];
 int charcount;
 unsigned int key_b;
 XSizeHints size_hints;

 expose_report = (XExposeEvent *)&report;
 while (XCheckMaskEvent(WpeXInfo.display, KeyPressMask | ButtonPressMask |
		ExposureMask | StructureNotifyMask, &report) == True)
 {
  switch(report.type)
  {
   case Expose:
    /* Reason for +2 : Assumes extra character on either side. */
    e_refresh_area(expose_report->x/WpeXInfo.font_width,
                           expose_report->y/WpeXInfo.font_height,
                           expose_report->width/WpeXInfo.font_width+2,
                           expose_report->height/WpeXInfo.font_height+2);
/*	    e_abs_refr();*/
    e_refresh();
    break;
   case ConfigureNotify:
    size_hints.width = (report.xconfigure.width / WpeXInfo.font_width) * WpeXInfo.font_width;
    size_hints.height = (report.xconfigure.height / WpeXInfo.font_height) * WpeXInfo.font_height;
    if (size_hints.width != MAXSCOL * WpeXInfo.font_width ||
      size_hints.height != MAXSLNS * WpeXInfo.font_height)
    {
     MAXSCOL = size_hints.width / WpeXInfo.font_width;
     MAXSLNS = size_hints.height / WpeXInfo.font_height;
     e_x_repaint_desk(WpeEditor->f[WpeEditor->mxedt]);
    }
    break;
   case KeyPress:
    charcount = e_XLookupString(&report.xkey, buffer, BUFSIZE, &keysym, NULL);
    if (charcount == 1 && *buffer == CtrlC) return(CtrlC);
    break;
   case ButtonPress:
    if (!pic)
     break;
    key_b = report.xbutton.state;
    if (report.xbutton.button == 1)
    {
     e_mouse.k = (key_b & ShiftMask) ? 3 : 0 +
		         (key_b & ControlMask) ? 4 : 0 +
		         (key_b & WpeXInfo.altmask) ? 8 : 0;
     e_mouse.x = report.xbutton.x/WpeXInfo.font_width;
     e_mouse.y = report.xbutton.y/WpeXInfo.font_height;
     if (e_mouse.x > (pic->e.x + pic->a.x - 10)/2 &&
       e_mouse.x < (pic->e.x + pic->a.x + 6)/2 )
      return(CtrlC);
    }
    break;
  }
 }
 return(0);
}

int e_x_getch()
{
 Window tmp_win, tmp_root;
 XEvent report;
 XSelectionEvent se;
 KeySym keysym;
 int charcount;
 unsigned char buffer[BUFSIZE];
 int c, root_x, root_y, x, y;
 unsigned int key_b;
 XSizeHints size_hints;

 e_refresh();

 XQueryPointer(WpeXInfo.display, WpeXInfo.window, &tmp_root, &tmp_win,
   &root_x, &root_y, &x, &y, &key_b);
 if (key_b & (Button1Mask | Button2Mask | Button3Mask))
 {
  e_mouse.x = x / WpeXInfo.font_width;
  e_mouse.y = y / WpeXInfo.font_height;
  c = 0;
  if (key_b & Button1Mask) c |= 1;
  if (key_b & Button2Mask) c |= 4;
  if (key_b & Button3Mask) c |= 2;
  return(-c);
 }

 while (1)
 {
  XNextEvent(WpeXInfo.display, &report);

  switch (report.type)
  {
   case Expose:
    do
    {
     /* Reason for +2 : Assumes extra character on either side. */
     e_refresh_area(report.xexpose.x / WpeXInfo.font_width,
       report.xexpose.y / WpeXInfo.font_height,
       report.xexpose.width / WpeXInfo.font_width + 2,
       report.xexpose.height / WpeXInfo.font_height + 2);
    } while (XCheckMaskEvent(WpeXInfo.display, ExposureMask, &report) == True);
    e_refresh();
    break;
   case ConfigureNotify:
    size_hints.width = (report.xconfigure.width / WpeXInfo.font_width) * WpeXInfo.font_width;
    size_hints.height = (report.xconfigure.height / WpeXInfo.font_height) * WpeXInfo.font_height;
    if (size_hints.width != MAXSCOL * WpeXInfo.font_width ||
      size_hints.height != MAXSLNS * WpeXInfo.font_height)
    {
     MAXSCOL = size_hints.width / WpeXInfo.font_width;
     MAXSLNS = size_hints.height / WpeXInfo.font_height;
     e_x_repaint_desk(WpeEditor->f[WpeEditor->mxedt]);
    }
    break;
   case ClientMessage:
    if (report.xclient.message_type == WpeXInfo.protocol_atom &&
      ((report.xclient.format == 8 &&
        report.xclient.data.b[0] == WpeXInfo.delete_atom) ||
      (report.xclient.format == 16 &&
        report.xclient.data.s[0] == WpeXInfo.delete_atom) ||
      (report.xclient.format == 32 &&
        report.xclient.data.l[0] == WpeXInfo.delete_atom)))
    {
     e_quit(WpeEditor->f[WpeEditor->mxedt]);
    }
    break;
   case KeyPress:
    charcount = e_XLookupString(&report.xkey, buffer, BUFSIZE, &keysym,
      NULL);
    key_b = report.xkey.state;
    if (charcount == 1)
    {
     if (*buffer == 127)
     {
      if (key_b & ControlMask)
       return(CENTF);
      else if (key_b & ShiftMask)
       return(ShiftDel);
      else if (key_b & WpeXInfo.altmask)
       return(AltDel);
      else
       return(ENTF);
     }
     if ((key_b & ShiftMask) && (*buffer == '\t'))
      return(WPE_BTAB);

     if (key_b & WpeXInfo.altmask)
      c = e_tast_sim(key_b & ShiftMask ? toupper(*buffer) : *buffer);
     else
      return(*buffer);
    }
    else
    {
     c = 0;
     if (key_b & ControlMask)
     {
      if (keysym == XK_Left) c = CCLE;
      else if (keysym == XK_Right) c = CCRI;
      else if (keysym == XK_Home) c = CPS1;
      else if (keysym == XK_End) c = CEND;
      else if (keysym == XK_Insert) c = CEINFG;
      else if (keysym == XK_Delete) c = CENTF;
      else if (keysym == XK_Prior) c = CBUP;
      else if (keysym == XK_Next) c = CBDO;
      else if (keysym == XK_F1) c = CF1;
      else if (keysym == XK_F2) c = CF2;
      else if (keysym == XK_F3) c = CF3;
      else if (keysym == XK_F4) c = CF4;
      else if (keysym == XK_F5) c = CF5;
      else if (keysym == XK_F6) c = CF6;
      else if (keysym == XK_F7) c = CF7;
      else if (keysym == XK_F8) c = CF8;
      else if (keysym == XK_F9) c = CF9;
      else if (keysym == XK_F10) c = CF10;
     }
     else if (key_b & WpeXInfo.altmask)
     {
      if (keysym == XK_F1) c = AF1;
      else if (keysym == XK_F2) c = AF2;
      else if (keysym == XK_F3) c = AF3;
      else if (keysym == XK_F4) c = AF4;
      else if (keysym == XK_F5) c = AF5;
      else if (keysym == XK_F6) c = AF6;
      else if (keysym == XK_F7) c = AF7;
      else if (keysym == XK_F8) c = AF8;
      else if (keysym == XK_F9) c = AF9;
      else if (keysym == XK_F10) c = AF10;
      else if (keysym == XK_Insert) c = AltEin;
      else if (keysym == XK_Delete) c = AltDel;
     }
     else
     {
      if (keysym == XK_Left) c = CLE;
      else if (keysym == XK_Right) c = CRI;
      else if (keysym == XK_Up) c = CUP;
      else if (keysym == XK_Down) c = CDO;
      else if (keysym == XK_Home) c = POS1;
      else if (keysym == XK_End) c = ENDE;
      else if (keysym == XK_Insert) c = EINFG;
      else if (keysym == XK_Delete) c = ENTF;
      else if (keysym == XK_BackSpace) c = CtrlH;
      else if (keysym == XK_Prior) c = BUP;
      else if (keysym == XK_Next) c = BDO;
      else if (keysym == XK_F1) c = F1;
      else if (keysym == XK_F2) c = F2;
      else if (keysym == XK_F3) c = F3;
      else if (keysym == XK_F4) c = F4;
      else if (keysym == XK_F5) c = F5;
      else if (keysym == XK_F6) c = F6;
      else if (keysym == XK_F7) c = F7;
      else if (keysym == XK_F8) c = F8;
      else if (keysym == XK_F9) c = F9;
      else if (keysym == XK_F10) c = F10;
      else if (keysym == XK_L1) c = STOP;
      else if (keysym == XK_L2) c = AGAIN;
      else if (keysym == XK_L3) c = PROPS;
      else if (keysym == XK_L4) c = UNDO;
      else if (keysym == XK_L5) c = FRONT;
      else if (keysym == XK_L6) c = COPY;
      else if (keysym == XK_L7) c = OPEN;
      else if (keysym == XK_L8) c = PASTE;
      else if (keysym == XK_L9) c = FID;
      else if (keysym == XK_L10) c = CUT;
      else if (keysym == XK_Help) c = HELP;
     }
    }
    if (c != 0)
    {
     if (key_b & ShiftMask)
      c = c + 512;
     return(c);
    }
    break;
   case ButtonPress:
    key_b = report.xbutton.state;
    e_mouse.k = (key_b & ShiftMask) ? 3 : 0 +
      (key_b & ControlMask) ? 4 : 0 +
      (key_b & WpeXInfo.altmask) ? 8 : 0;
    e_mouse.x = report.xbutton.x/WpeXInfo.font_width;
    e_mouse.y = report.xbutton.y/WpeXInfo.font_height;
    c = 0;
    if (report.xbutton.button == 1) c |= 1;
    if (report.xbutton.button == 2) c |= 2;
    if (report.xbutton.button == 3) c |= 4;
    return(-c);
   case SelectionRequest:
    if (WpeXInfo.selection)
    {
     se.type = SelectionNotify;
     se.display = report.xselectionrequest.display;
     se.requestor = report.xselectionrequest.requestor;
     se.selection = report.xselectionrequest.selection;
     se.time = report.xselectionrequest.time;
     se.target = report.xselectionrequest.target;
     if (report.xselectionrequest.property == None)
      report.xselectionrequest.property = report.xselectionrequest.target;
     /* Xt asks for TARGETS.  Should probably support that. */
     if (report.xselectionrequest.target == WpeXInfo.text_atom)
     {
      se.property = report.xselectionrequest.property;
      XChangeProperty(se.display, se.requestor, se.property, se.target, 8,
        PropModeReplace, WpeXInfo.selection, strlen(WpeXInfo.selection));
     }
     else
      se.property = None;
     XSendEvent(WpeXInfo.display, se.requestor, False, 0, (XEvent *)&se);
    }
    break;
   case SelectionClear:
    if (WpeXInfo.selection)
    {
     WpeFree(WpeXInfo.selection);
     WpeXInfo.selection = NULL;
    }
    break;
   default:
    break;
  }
 }
 return(0);
}

int e_x_kbhit()
{
 XEvent report;
 KeySym keysym;
 int charcount;
 unsigned char buffer[BUFSIZE];
 int c;
 unsigned int key_b;

 e_refresh();

 if (XCheckMaskEvent(WpeXInfo.display, ButtonPressMask | KeyPressMask, &report) == False)
  return(0);

 if (report.type == ButtonPress)
 {
  key_b = report.xbutton.state;
  e_mouse.k = (key_b & ShiftMask) ? 3 : 0;
  e_mouse.x = report.xbutton.x/WpeXInfo.font_width;
  e_mouse.y = report.xbutton.y/WpeXInfo.font_height;
  c = 0;
  if(report.xbutton.button == 1) c |= 1;
  if(report.xbutton.button == 2) c |= 2;
  if(report.xbutton.button == 3) c |= 4;
  return(-c);
 }
 else
 {
  charcount = e_XLookupString(&report.xkey, buffer, BUFSIZE,
						&keysym, NULL);
  if(charcount == 1) return(*buffer);
  else return(0);
 }
}

void e_setlastpic(PIC *pic)
{
 extern PIC *e_X_l_pic;

 e_X_l_pic = pic;
}

int fk_x_locate(int x, int y)
{
 cur_x = x;
 return(cur_y = y);
}

int fk_x_cursor(int x)
{
 return(cur_on = x);
}

int e_x_sys_ini()
{
 return(0);
}

int e_x_sys_end()
{
 return(0);
}

int fk_x_putchar(int c)
{
 return(fputc(c, stdout));
}

int x_bioskey()
{
 return(e_mouse.k);
}

int e_x_system(const char *exe)
{
 FILE *fp;
 int ret;
 char file[80];
 char *string;

 sprintf(file, "%s/we_sys_tmp", e_tmp_dir);
 string = MALLOC(strlen(XTERM_CMD) + strlen(exe) + strlen(file) +
   strlen(user_shell) + 40);
 if (!(fp = fopen(file, "w+")))
 {
  FREE(string);
  return(-1);
 }
 fputs("$*\necho type \\<Return\\> to continue\nread i\n", fp);
 fclose(fp);
 chmod(file, 0700);
 if (exe[0] == '/')
  sprintf(string, "%s -geometry 80x25-0-0 +sb -e %s %s %s", XTERM_CMD,
    user_shell, file, exe);
 else
  sprintf(string, "%s -geometry 80x25-0-0 +sb -e %s %s ./%s", XTERM_CMD,
    user_shell, file, exe);
 ret = system(string);
 remove(file);
 FREE(string);
 return(ret);
}

int e_x_repaint_desk(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i, g[4];
 extern PIC *e_X_l_pic;
 PIC *sv_pic = NULL, *nw_pic = NULL;

 if (e_X_l_pic && e_X_l_pic != cn->f[cn->mxedt]->pic)
 {
  sv_pic = e_X_l_pic;
  nw_pic = e_open_view(e_X_l_pic->a.x, e_X_l_pic->a.y, e_X_l_pic->e.x,
    e_X_l_pic->e.y, 0, 2);
 }
 e_ini_size();
 if (cn->mxedt < 1)
 {
  e_cls(f->fb->df.fb, f->fb->dc);
  e_ini_desk(f->ed);
  if (nw_pic)
  {
   e_close_view(nw_pic, 1);
   e_X_l_pic = sv_pic;
  }
  return(0);
 }
 ini_repaint(cn);
 e_abs_refr();
 for (i = cn->mxedt; i >= 1; i--)
 {
  FREE(cn->f[i]->pic->p);
  FREE(cn->f[i]->pic);
 }
 for (i = 0; i <= cn->mxedt; i++)
 {
  if (cn->f[i]->e.x >= MAXSCOL) cn->f[i]->e.x = MAXSCOL-1;
  if (cn->f[i]->e.y >= MAXSLNS-1) cn->f[i]->e.y = MAXSLNS-2;
  if (cn->f[i]->e.x - cn->f[i]->a.x < 26)
   cn->f[i]->a.x = cn->f[i]->e.x - 26;
  if (!DTMD_ISTEXT(cn->f[i]->dtmd) && cn->f[i]->e.y - cn->f[i]->a.y < 9)
   cn->f[i]->a.y = cn->f[i]->e.y - 9;
  else if (DTMD_ISTEXT(cn->f[i]->dtmd) && cn->f[i]->e.y - cn->f[i]->a.y < 3)
   cn->f[i]->a.y = cn->f[i]->e.y - 3;
 }
 for (i = 1; i < cn->mxedt; i++)
 {
  e_firstl(cn->f[i], 0);
  e_schirm(cn->f[i], 0);
 }
 e_firstl(cn->f[i], 1);
 e_schirm(cn->f[i], 1);
 if (nw_pic)
 {
  e_close_view(nw_pic, 1);
  e_X_l_pic = sv_pic;
 }
 g[0] = 2; fk_mouse(g);
 end_repaint();
 e_cursor(cn->f[i], 1);
 g[0] = 0; fk_mouse(g);
 g[0] = 1; fk_mouse(g);
 return(0);
}


int fk_x_mouse(int *g)
{
 Window tmp_win, tmp_root;
 int root_x, root_y, x, y;
 unsigned int key_b;

 if (!XQueryPointer(WpeXInfo.display, WpeXInfo.window, &tmp_root, &tmp_win,
   &root_x, &root_y, &x, &y, &key_b))
 {
  g[2] = e_mouse.x * 8;
  g[3] = e_mouse.y * 8;
  g[0] = g[1] = 0;
  return(0);
 }
 g[0] = 0;
 if (key_b & Button1Mask)
  g[0] |= 1;
 if(key_b & Button2Mask)
  g[0] |= 4;
 if(key_b & Button3Mask)
  g[0] |= 2;
 g[1] = g[0];
 g[2] = x/WpeXInfo.font_width * 8;
 g[3] = y/WpeXInfo.font_height * 8;
 return(g[1]);
}

int e_x_cp_X_to_buffer(FENSTER *f)
{
 BUFFER *b0 = f->ed->f[0]->b;
 SCHIRM *s0 = f->ed->f[0]->s;
 int i, j, k, n;
 unsigned char *str;
 XEvent report;
 Atom type;
 int format;
 long nitems, bytes_left;

 for (i = 1; i < b0->mxlines; i++)
  FREE(b0->bf[i].s);
 b0->mxlines = 1;
 *(b0->bf[0].s) = WPE_WR;
 *(b0->bf[0].s+1) = '\0';
 b0->bf[0].len = 0;
#if SELECTION
 if (WpeXInfo.selection)
 {
  str = WpeStrdup(WpeXInfo.selection);
  n = strlen(str);
 }
 else
 {
  /* Should check for errors especially failure to send SelectionNotify */
  XConvertSelection(WpeXInfo.display, WpeXInfo.selection_atom,
    WpeXInfo.text_atom, WpeXInfo.property_atom, WpeXInfo.window, CurrentTime);
  n = 0;
  while (!XCheckTypedEvent(WpeXInfo.display, SelectionNotify, &report))
  {
   /* Should probably have a better timeout period than this. */
   sleep(0);
   n++;
   if (n > 1000)
    return 0;
  }
  if (WpeXInfo.property_atom == None)
   return 0;
  XGetWindowProperty(WpeXInfo.display, WpeXInfo.window, WpeXInfo.property_atom,
    0, 1000000, FALSE, WpeXInfo.text_atom, &type, &format, &nitems, &bytes_left,
    &str);
  if (type == None)
  {
   /* Specified property does not exit*/
   return 0;
  }
  n = strlen(str);
 }
#else
 str = XFetchBytes(WpeXInfo.display, &n);
#endif
 for (i = k = 0; i < n; i++, k++)
 {
  for (j = 0; i < n && str[i] != '\n' && j < b0->mx.x-1; j++, i++)
   b0->bf[k].s[j] = str[i];
  if (i < n)
  {
   e_new_line(k+1, b0);
   if (str[i] == '\n')
   {
    b0->bf[k].s[j] = WPE_WR;
    b0->bf[k].nrc = j+1;
   }
   else
    b0->bf[k].nrc = j;
   b0->bf[k].s[j+1] = '\0';
   b0->bf[k].len = j;
  }
  else
  {
   b0->bf[k].s[j] = '\0';
   b0->bf[k].nrc = b0->bf[k].len = j;
  }
 }
 s0->mark_begin.x = s0->mark_begin.y = 0;
 s0->mark_end.y = b0->mxlines-1;
 s0->mark_end.x = b0->bf[b0->mxlines-1].len;
#if SELECTION
 if (WpeXInfo.selection)
  WpeFree(str);
 else
#endif
  XFree(str);
 return 0;
}

int e_x_copy_X_buffer(FENSTER *f)
{
 e_cp_X_to_buffer(f);
 e_edt_einf(f);
 return(0);
}

int e_x_paste_X_buffer(FENSTER *f)
{
 BUFFER *b0 = f->ed->f[0]->b;
 SCHIRM *s0 = f->ed->f[0]->s;
 int i, j, n;

 e_edt_copy(f);
#if SELECTION
 if (WpeXInfo.selection)
 {
  WpeFree(WpeXInfo.selection);
  WpeXInfo.selection = NULL;
 }
#endif
 if ((s0->mark_end.y == 0 && s0->mark_end.x == 0) ||
   s0->mark_end.y < s0->mark_begin.y)
  return(0);
 if (s0->mark_end.y == s0->mark_begin.y)
 {
  if (s0->mark_end.x < s0->mark_begin.x)
   return(0);
  n = s0->mark_end.x - s0->mark_begin.x;
#if SELECTION
  WpeXInfo.selection = WpeMalloc(n + 1);
  strncpy(WpeXInfo.selection, b0->bf[s0->mark_begin.y].s+s0->mark_begin.x,
    n);
  WpeXInfo.selection[n] = 0;
  XSetSelectionOwner(WpeXInfo.display, WpeXInfo.selection_atom,
    WpeXInfo.window, CurrentTime);
#else
  XStoreBytes(WpeXInfo.display, b0->bf[s0->mark_begin.y].s+s0->mark_begin.x,
    n);
#endif
  return(0);
 }
 WpeXInfo.selection = WpeMalloc(b0->bf[s0->mark_begin.y].nrc * sizeof(char));
 for (n = 0, j = s0->mark_begin.x; j < b0->bf[s0->mark_begin.y].nrc; j++, n++)
  WpeXInfo.selection[n] = b0->bf[s0->mark_begin.y].s[j];
 for (i = s0->mark_begin.y+1; i < s0->mark_end.y; i++)
 {
  WpeXInfo.selection = WpeRealloc(WpeXInfo.selection, (n + b0->bf[i].nrc)*sizeof(char));
  for (j = 0; j < b0->bf[i].nrc; j++, n++)
   WpeXInfo.selection[n] = b0->bf[i].s[j];
 }
 WpeXInfo.selection = WpeRealloc(WpeXInfo.selection, (n + s0->mark_end.x + 1)*sizeof(char));
 for (j = 0; j < s0->mark_end.x; j++, n++)
  WpeXInfo.selection[n] = b0->bf[i].s[j];
 WpeXInfo.selection[n] = 0;
#if SELECTION
 XSetSelectionOwner(WpeXInfo.display, WpeXInfo.selection_atom,
   WpeXInfo.window, CurrentTime);
#else
 XStoreBytes(WpeXInfo.display, WpeXInfo.selection, n);
 WpeFree(WpeXInfo.selection);
 WpeXInfo.selection = NULL;
#endif
 return(0);
}

#endif

