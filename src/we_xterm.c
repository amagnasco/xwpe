/** \file we_xterm.c                                       */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"

#ifndef NO_XWINDOWS

#include <ctype.h>
#include "keys.h"
#include "model.h"
#include "we_control.h"
#include "edit.h"
#include "we_screen.h"
#include "we_unix.h"
#include "WeString.h"
#include "we_term.h"
#include "we_xterm.h"

/* partial conversion in place */
#include "WeXterm.h"

//#ifndef TERMCAP
#if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES
#include <curses.h>
#endif

/** global field filled in we_main.c and used in we_fl_unix.c */
char *user_shell;

int fk_show_cursor (void);
int e_x_getch (void);
int fk_x_mouse (int *g);
int e_x_refresh (void);
int fk_x_locate (int x, int y);
int fk_x_cursor (int x);
int e_x_sys_ini (void);
int e_x_sys_end (void);
int fk_x_putchar (int c);
int x_bioskey (void);
int e_x_system (const char *exe);
int e_x_cp_X_to_buffer (we_window_t * window);
int e_x_copy_X_buffer (we_window_t * window);
int e_x_paste_X_buffer (we_window_t * window);
int e_x_change (we_view_t * view);
int e_x_repaint_desk (we_window_t * window);
void e_setlastpic (we_view_t * view);
int e_x_kbhit (void);
x_selection_t e_x_get_X_selection();
void e_x_empty_buffer(we_buffer_t *buffer);
void e_x_free_X_selection(x_selection_t xsel);
void e_x_paste_X_selection(we_buffer_t *buffer, x_selection_t xsel);
void e_x_reinit_marked_area(we_screen_t *screen, we_buffer_t *buffer);

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

int old_cursor_x = 0, old_cursor_y = 0;

static int cur_on = 1;

extern we_view_t *e_X_l_pic;

extern struct mouse e_mouse;

int
WpeXtermInit (int *argc, char **argv)
{
    e_s_u_clr = e_s_x_clr;
    e_n_u_clr = e_n_x_clr;
    e_frb_u_menue = e_frb_x_menue;
    e_pr_u_colorsets = e_pr_x_colorsets;
    fk_u_cursor = fk_x_cursor;
    fk_u_locate = fk_x_locate;
    e_u_refresh = e_x_refresh;
    e_u_getch = e_x_getch;
    u_bioskey = x_bioskey;
    e_u_sys_ini = e_x_sys_ini;
    e_u_sys_end = e_x_sys_end;
    e_u_system = e_x_system;
    fk_u_putchar = fk_x_putchar;
    fk_u_mouse = fk_x_mouse;
    e_u_cp_X_to_buffer = e_x_cp_X_to_buffer;
    e_u_copy_X_buffer = e_x_copy_X_buffer;
    e_u_paste_X_buffer = e_x_paste_X_buffer;
    e_u_kbhit = e_x_kbhit;
    e_u_change = e_x_change;
    e_u_setlastpic = e_setlastpic;
    WpeMouseChangeShape = (void (*)(WpeMouseShape)) WpeNullFunction;
    WpeMouseRestoreShape = (void (*)(void)) WpeNullFunction;
    /* WpeMouseChangeShape = WpeXMouseChangeShape;
     WpeMouseRestoreShape = WpeXMouseRestoreShape;*/
    WpeDisplayEnd = WpeNullFunction;
    e_u_switch_screen = WpeZeroFunction;
    e_u_d_switch_out = (int (*)(int sw)) WpeZeroFunction;
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
    WpeXInit (argc, argv);
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


#else /* cached  a.r. */

#define WPE_MAXSEG 1000
int nseg[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

XSegment seg[8][WPE_MAXSEG];
int scol[8] = { 0, 0, 15, 15, 15, 15, 0, 0 };

int
e_XLookupString (XKeyEvent * event, char *buffer_return, int buffer_size,
                 KeySym * keysym_return, XComposeStatus * status)
{
    static int first = 1;
    static XIC xic;
    static XIM xim;

    if (first) {
        if (!XSetLocaleModifiers ("")) {
            XSetLocaleModifiers ("@im=none");
        }
        xim = XOpenIM (event->display, NULL, NULL, NULL);
        xic = XCreateIC (xim,
                         XNInputStyle, XIMPreeditNothing | XIMStatusNothing,
                         XNClientWindow, WpeXInfo.window, NULL);
        first = 0;
    }
    if (xic) {
        if (XFilterEvent ((XEvent *) event, WpeXInfo.window)) {
            return (0);
        }

        return (XmbLookupString (xic, event, buffer_return, buffer_size,
                                 keysym_return, NULL));
    }

    return (XLookupString (event, buffer_return, buffer_size,
                           keysym_return, status));
}

void
e_flush_xrect ()
{
    int i;

    for (i = 0; i < 8; i++)
        if (nseg[i]) {
            XSetForeground (WpeXInfo.display, WpeXInfo.gc,
                            WpeXInfo.colors[scol[i]]);
            XDrawSegments (WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc, seg[i],
                           nseg[i]);
            nseg[i] = 0;
        }
}

void
e_print_xrect (int x, int y, int n)
{
    int c = extbyte[n] & 16 ? 4 : 0;

    if (extbyte[n]) {
        if ((nseg[0] > WPE_MAXSEG) || (nseg[1] > WPE_MAXSEG)
                || (nseg[2] > WPE_MAXSEG) || (nseg[3] > WPE_MAXSEG)
                || (nseg[4] > WPE_MAXSEG) || (nseg[5] > WPE_MAXSEG)
                || (nseg[6] > WPE_MAXSEG) || (nseg[7] > WPE_MAXSEG)) {
            e_flush_xrect ();
        }
        if (extbyte[n] & 2) {
            seg[c][nseg[c]].x1 = WpeXInfo.font_width * ((x) + 1) - 1;
            seg[c][nseg[c]].y1 = WpeXInfo.font_height * (y);
            seg[c][nseg[c]].x2 = WpeXInfo.font_width * ((x) + 1) - 1;
            seg[c][nseg[c]].y2 = WpeXInfo.font_height * ((y) + 1) - 1;
            nseg[c]++;
        }
        if (extbyte[n] & 4) {
            seg[c + 1][nseg[c + 1]].x1 = WpeXInfo.font_width * (x);
            seg[c + 1][nseg[c + 1]].y1 = WpeXInfo.font_height * ((y) + 1) - 1;
            seg[c + 1][nseg[c + 1]].x2 = WpeXInfo.font_width * ((x) + 1) - 1;
            seg[c + 1][nseg[c + 1]].y2 = WpeXInfo.font_height * ((y) + 1) - 1;
            nseg[c + 1]++;
        }
        if (extbyte[n] & 8) {
            seg[c + 2][nseg[c + 2]].x1 = WpeXInfo.font_width * (x);
            seg[c + 2][nseg[c + 2]].y1 = WpeXInfo.font_height * (y);
            seg[c + 2][nseg[c + 2]].x2 = WpeXInfo.font_width * (x);
            seg[c + 2][nseg[c + 2]].y2 = WpeXInfo.font_height * ((y) + 1) - 1;
            nseg[c + 2]++;
        }
        if (extbyte[n] & 1) {
            seg[c + 3][nseg[c + 3]].x1 = WpeXInfo.font_width * (x);
            seg[c + 3][nseg[c + 3]].y1 = WpeXInfo.font_height * (y);
            seg[c + 3][nseg[c + 3]].x2 = WpeXInfo.font_width * ((x) + 1) - 1;
            seg[c + 3][nseg[c + 3]].y2 = WpeXInfo.font_height * (y);
            nseg[c + 3]++;
        }
    }
}
#endif

#endif

int
fk_show_cursor ()
{
    int x;

    if (!cur_on) {
        return (0);
    }
    x = 2 * old_cursor_x + 2 * max_screen_cols() * old_cursor_y;
    if (x > 0) {
        XSetForeground (WpeXInfo.display, WpeXInfo.gc,
                        WpeXInfo.colors[global_screen[x + 1] % 16]);
        XSetBackground (WpeXInfo.display, WpeXInfo.gc,
                        WpeXInfo.colors[global_screen[x + 1] / 16]);
        XDrawImageString (WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
                          WpeXInfo.font_width * old_cursor_x,
                          WpeXInfo.font_height * (old_cursor_y + 1) -
                          WpeXInfo.font->max_bounds.descent, global_screen + x, 1);
#ifdef NEWSTYLE
        e_print_xrect (old_cursor_x, old_cursor_y, x / 2);
#ifndef NOXCACHE
        e_flush_xrect ();
#endif
#endif
    }
    x = 2 * cur_x + 2 * max_screen_cols() * cur_y;

    XSetForeground (WpeXInfo.display, WpeXInfo.gc,
                    WpeXInfo.colors[global_screen[x + 1] / 16]);
    XSetBackground (WpeXInfo.display, WpeXInfo.gc,
                    WpeXInfo.colors[global_screen[x + 1] % 16]);
    XDrawImageString (WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
                      WpeXInfo.font_width * cur_x,
                      WpeXInfo.font_height * (cur_y + 1) -
                      WpeXInfo.font->max_bounds.descent, global_screen + x, 1);
    old_cursor_x = cur_x;
    old_cursor_y = cur_y;
    return (cur_on);
}

#define A_Normal 	16
#define A_Reverse 	1
#define A_Standout	1
#define A_Underline	1
#define A_Bold		16

int
e_X_sw_color ()
{
    we_colorset_t *fb = global_editor_control->colorset;
    fb->er = e_n_u_clr (A_Normal);
    fb->et = e_n_u_clr (A_Normal);
    fb->ez = e_n_u_clr (A_Reverse);
    fb->es = e_n_u_clr (A_Normal);
    fb->em = e_n_u_clr (A_Standout);
    fb->ek = e_n_u_clr (A_Underline);
    fb->nr = e_n_u_clr (A_Standout);
    fb->nt = e_n_u_clr (A_Reverse);
    fb->nz = e_n_u_clr (A_Normal);
    fb->ns = e_n_u_clr (A_Bold);
    fb->mr = e_n_u_clr (A_Standout);
    fb->mt = e_n_u_clr (A_Standout);
    fb->mz = e_n_u_clr (A_Normal);
    fb->ms = e_n_u_clr (A_Normal);
    fb->fr = e_n_u_clr (A_Normal);
    fb->ft = e_n_u_clr (A_Normal);
    fb->fz = e_n_u_clr (A_Standout);
    fb->fs = e_n_u_clr (A_Standout);
    fb->of = e_n_u_clr (A_Standout);
    fb->df = e_n_u_clr (A_Normal);
    fb->dc = 0x02;
#ifdef DEBUGGER
    fb->db = e_n_u_clr (A_Standout);
    fb->dy = e_n_u_clr (A_Standout);
#endif
    return (0);
}

int
e_x_refresh ()
{
#ifndef NOXCACHE		/* a.r. */
#define STRBUFSIZE 1024
    unsigned long oldback = 0, oldfore = 0;
    static char stringbuf[STRBUFSIZE];
    int stringcount = 0, oldI = 0, oldX = 0, oldY = 0, oldJ = 0;
#endif
    int i, j, x, y, cur_tmp = cur_on;
    fk_u_cursor (0);
    for (i = 0; i < max_screen_lines(); i++)
        for (j = 0; j < max_screen_cols(); j++) {
            y = j + max_screen_cols() * i;
            x = 2 * y;
#ifdef NEWSTYLE
            if (global_screen[x] != global_alt_screen[x] || global_screen[x + 1] != global_alt_screen[x + 1]
                    || extbyte[y] != altextbyte[y])
#else
            if (global_screen[x] != global_alt_screen[x] || global_screen[x + 1] != global_alt_screen[x + 1])
#endif
            {
#ifdef NOXCACHE
                XSetForeground (WpeXInfo.display, WpeXInfo.gc,
                                WpeXInfo.colors[global_screen[x + 1] % 16]);
                XSetBackground (WpeXInfo.display, WpeXInfo.gc,
                                WpeXInfo.colors[global_screen[x + 1] / 16]);
                XDrawImageString (WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
                                  WpeXInfo.font_width * j,
                                  WpeXInfo.font_height * (i + 1) -
                                  WpeXInfo.font->max_bounds.descent, global_screen + x,
                                  1);
#else
                if (oldback != (unsigned) WpeXInfo.colors[global_screen[x + 1] / 16]	/* a.r. */
                        || oldfore != (unsigned) WpeXInfo.colors[global_screen[x + 1] % 16] || i != oldI || j > oldJ + 1	/* is there a more elegant solution? */
                        || stringcount >= STRBUFSIZE) {
                    XDrawImageString (WpeXInfo.display, WpeXInfo.window,
                                      WpeXInfo.gc, oldX, oldY, stringbuf,
                                      stringcount);
                    oldback = WpeXInfo.colors[global_screen[x + 1] / 16];
                    oldfore = WpeXInfo.colors[global_screen[x + 1] % 16];
                    XSetForeground (WpeXInfo.display, WpeXInfo.gc, oldfore);
                    XSetBackground (WpeXInfo.display, WpeXInfo.gc, oldback);
                    oldX = WpeXInfo.font_width * j;
                    oldY =
                        WpeXInfo.font_height * (i + 1) -
                        WpeXInfo.font->max_bounds.descent;
                    oldI = i;
                    stringcount = 0;
                    stringbuf[stringcount++] = global_screen[x];
                } else {
                    stringbuf[stringcount++] = global_screen[x];
                }
#endif
#ifndef NEWSTYLE
                if (global_screen[x] == 16) {
                    XFillRectangle (WpeXInfo.display, WpeXInfo.window,
                                    WpeXInfo.gc, WpeXInfo.font_width * j,
                                    WpeXInfo.font_height * (i),
                                    WpeXInfo.font_width,
                                    (WpeXInfo.font_height +
                                     WpeXInfo.font->max_bounds.descent) / 2);
                } else if (global_screen[x] == 20) {
                    XFillRectangle (WpeXInfo.display, WpeXInfo.window,
                                    WpeXInfo.gc, WpeXInfo.font_width * j,
                                    (int) (WpeXInfo.font_height * (i + 1. / 2)),
                                    WpeXInfo.font_width,
                                    (WpeXInfo.font_height +
                                     WpeXInfo.font->max_bounds.descent) / 2);
                }
#endif
                global_alt_screen[x] = global_screen[x];
                global_alt_screen[x + 1] = global_screen[x + 1];
#ifdef NEWSTYLE
                e_print_xrect (j, i, y);
                altextbyte[y] = extbyte[y];
#endif
#ifndef NOXCACHE
                oldJ = j;
#endif
            }
        }
#ifndef NOXCACHE
    XDrawImageString (WpeXInfo.display, WpeXInfo.window, WpeXInfo.gc,
                      oldX, oldY, stringbuf, stringcount);
#ifdef NEWSTYLE
    e_flush_xrect ();
#endif
#endif
    fk_u_cursor (cur_tmp);
    fk_show_cursor ();
    XFlush (WpeXInfo.display);
    return (0);
}

int
e_x_change (we_view_t * view)
{
    XEvent report;
    XExposeEvent *expose_report;
    KeySym keysym;
    char buffer[BUFSIZE];
    int charcount;
    unsigned int key_b;
    XSizeHints size_hints;

    expose_report = (XExposeEvent *) & report;
    while (XCheckMaskEvent (WpeXInfo.display, KeyPressMask | ButtonPressMask |
                            ExposureMask | StructureNotifyMask,
                            &report) == True) {
        switch (report.type) {
        case Expose:
            /* Reason for +2 : Assumes extra character on either side. */
            e_refresh_area (expose_report->x / WpeXInfo.font_width,
                            expose_report->y / WpeXInfo.font_height,
                            expose_report->width / WpeXInfo.font_width + 2,
                            expose_report->height / WpeXInfo.font_height + 2);
            /*	    e_abs_refr();*/
            e_u_refresh ();
            break;
        case ConfigureNotify:
            size_hints.width =
                (report.xconfigure.width / WpeXInfo.font_width) *
                WpeXInfo.font_width;
            size_hints.height =
                (report.xconfigure.height / WpeXInfo.font_height) *
                WpeXInfo.font_height;
            if (size_hints.width != max_screen_cols() * WpeXInfo.font_width
                    || size_hints.height != max_screen_lines() * WpeXInfo.font_height) {
                int cols = size_hints.width / WpeXInfo.font_width;
                set_max_screen_cols(cols);
                int lns = size_hints.height / WpeXInfo.font_height;
                set_max_screen_lines(lns);
                e_x_repaint_desk (global_editor_control->window[global_editor_control->mxedt]);
            }
            break;
        case KeyPress:
            charcount =
                e_XLookupString (&report.xkey, buffer, BUFSIZE, &keysym, NULL);
            if (charcount == 1 && *buffer == CtrlC) {
                return (CtrlC);
            }
            break;
        case ButtonPress:
            if (!view) {
                break;
            }
            key_b = report.xbutton.state;
            if (report.xbutton.button == 1) {
                e_mouse.k = (key_b & ShiftMask) ? 3 : 0 +
                            (key_b & ControlMask) ? 4 : 0 +
                            (key_b & WpeXInfo.altmask) ? 8 : 0;
                e_mouse.x = report.xbutton.x / WpeXInfo.font_width;
                e_mouse.y = report.xbutton.y / WpeXInfo.font_height;
                if (e_mouse.x > (view->e.x + view->a.x - 10) / 2 &&
                        e_mouse.x < (view->e.x + view->a.x + 6) / 2) {
                    return (CtrlC);
                }
            }
            break;
        }
    }
    return (0);
}

int
e_x_getch ()
{
    Window tmp_win, tmp_root;
    XEvent report;
    XSelectionEvent se;
    KeySym keysym;
    int charcount;
    char buffer[BUFSIZE];
    int c, root_x, root_y, x, y;
    unsigned int key_b;
    XSizeHints size_hints;

    e_u_refresh ();

    XQueryPointer (WpeXInfo.display, WpeXInfo.window, &tmp_root, &tmp_win,
                   &root_x, &root_y, &x, &y, &key_b);
    if (key_b & (Button1Mask | Button2Mask | Button3Mask)) {
        e_mouse.x = x / WpeXInfo.font_width;
        e_mouse.y = y / WpeXInfo.font_height;
        c = 0;
        if (key_b & Button1Mask) {
            c |= 1;
        }
        if (key_b & Button2Mask) {
            c |= 4;
        }
        if (key_b & Button3Mask) {
            c |= 2;
        }
        return (-c);
    }

    while (1) {
        XNextEvent (WpeXInfo.display, &report);

        switch (report.type) {
        case Expose:
            do {
                /* Reason for +2 : Assumes extra character on either side. */
                e_refresh_area (report.xexpose.x / WpeXInfo.font_width,
                                report.xexpose.y / WpeXInfo.font_height,
                                report.xexpose.width / WpeXInfo.font_width + 2,
                                report.xexpose.height / WpeXInfo.font_height +
                                2);
            } while (XCheckMaskEvent (WpeXInfo.display, ExposureMask, &report) ==
                     True);
            e_u_refresh ();
            break;
        case ConfigureNotify:
            size_hints.width =
                (report.xconfigure.width / WpeXInfo.font_width) *
                WpeXInfo.font_width;
            size_hints.height =
                (report.xconfigure.height / WpeXInfo.font_height) *
                WpeXInfo.font_height;
            if (size_hints.width != max_screen_cols() * WpeXInfo.font_width
                    || size_hints.height != max_screen_lines() * WpeXInfo.font_height) {
                int cols = size_hints.width / WpeXInfo.font_width;
                set_max_screen_cols(cols);
                int lns = size_hints.height / WpeXInfo.font_height;
                set_max_screen_lines(lns);
                e_x_repaint_desk (global_editor_control->window[global_editor_control->mxedt]);
            }
            break;
        case ClientMessage:
            // Atoms are defined as unsigned long, hence a cast to unsigned
            if (report.xclient.message_type == WpeXInfo.protocol_atom &&
                    ((report.xclient.format == 8 &&
                      (unsigned) report.xclient.data.b[0] == WpeXInfo.delete_atom)
                     || (report.xclient.format == 16
                         && (unsigned) report.xclient.data.s[0] ==
                         WpeXInfo.delete_atom) || (report.xclient.format == 32
                                                   && (unsigned) report.xclient.
                                                   data.l[0] ==
                                                   WpeXInfo.delete_atom))) {
                e_quit (global_editor_control->window[global_editor_control->mxedt]);
            }
            break;
        case KeyPress:
            charcount = e_XLookupString (&report.xkey, buffer, BUFSIZE, &keysym,
                                         NULL);
            key_b = report.xkey.state;
            if (charcount == 1) {
                if (*buffer == 127) {
                    if (key_b & ControlMask) {
                        return (CENTF);
                    } else if (key_b & ShiftMask) {
                        return (ShiftDel);
                    } else if (key_b & WpeXInfo.altmask) {
                        return (AltDel);
                    } else {
                        return (ENTF);
                    }
                }
                if ((key_b & ShiftMask) && (*buffer == '\t')) {
                    return (WPE_BTAB);
                }

                if (key_b & WpeXInfo.altmask)
                    c =
                        e_tast_sim (key_b & ShiftMask ? toupper (*buffer) :
                                    *buffer);
                else {
                    return (*buffer);
                }
            } else {
                c = 0;
                if (key_b & ControlMask) {
                    if (keysym == XK_Left) {
                        c = CCLE;
                    } else if (keysym == XK_Right) {
                        c = CCRI;
                    } else if (keysym == XK_Home) {
                        c = CPS1;
                    } else if (keysym == XK_End) {
                        c = CEND;
                    } else if (keysym == XK_Insert) {
                        c = CEINFG;
                    } else if (keysym == XK_Delete) {
                        c = CENTF;
                    } else if (keysym == XK_Prior) {
                        c = CBUP;
                    } else if (keysym == XK_Next) {
                        c = CBDO;
                    } else if (keysym == XK_F1) {
                        c = CF1;
                    } else if (keysym == XK_F2) {
                        c = CF2;
                    } else if (keysym == XK_F3) {
                        c = CF3;
                    } else if (keysym == XK_F4) {
                        c = CF4;
                    } else if (keysym == XK_F5) {
                        c = CF5;
                    } else if (keysym == XK_F6) {
                        c = CF6;
                    } else if (keysym == XK_F7) {
                        c = CF7;
                    } else if (keysym == XK_F8) {
                        c = CF8;
                    } else if (keysym == XK_F9) {
                        c = CF9;
                    } else if (keysym == XK_F10) {
                        c = CF10;
                    }
                } else if (key_b & WpeXInfo.altmask) {
                    if (keysym == XK_F1) {
                        c = AF1;
                    } else if (keysym == XK_F2) {
                        c = AF2;
                    } else if (keysym == XK_F3) {
                        c = AF3;
                    } else if (keysym == XK_F4) {
                        c = AF4;
                    } else if (keysym == XK_F5) {
                        c = AF5;
                    } else if (keysym == XK_F6) {
                        c = AF6;
                    } else if (keysym == XK_F7) {
                        c = AF7;
                    } else if (keysym == XK_F8) {
                        c = AF8;
                    } else if (keysym == XK_F9) {
                        c = AF9;
                    } else if (keysym == XK_F10) {
                        c = AF10;
                    } else if (keysym == XK_Insert) {
                        c = AltEin;
                    } else if (keysym == XK_Delete) {
                        c = AltDel;
                    }
                } else {
                    if (keysym == XK_Left) {
                        c = CLE;
                    } else if (keysym == XK_Right) {
                        c = CRI;
                    } else if (keysym == XK_Up) {
                        c = CUP;
                    } else if (keysym == XK_Down) {
                        c = CDO;
                    } else if (keysym == XK_Home) {
                        c = POS1;
                    } else if (keysym == XK_End) {
                        c = ENDE;
                    } else if (keysym == XK_Insert) {
                        c = EINFG;
                    } else if (keysym == XK_Delete) {
                        c = ENTF;
                    } else if (keysym == XK_BackSpace) {
                        c = CtrlH;
                    } else if (keysym == XK_Prior) {
                        c = BUP;
                    } else if (keysym == XK_Next) {
                        c = BDO;
                    } else if (keysym == XK_F1) {
                        c = F1;
                    } else if (keysym == XK_F2) {
                        c = F2;
                    } else if (keysym == XK_F3) {
                        c = F3;
                    } else if (keysym == XK_F4) {
                        c = F4;
                    } else if (keysym == XK_F5) {
                        c = F5;
                    } else if (keysym == XK_F6) {
                        c = F6;
                    } else if (keysym == XK_F7) {
                        c = F7;
                    } else if (keysym == XK_F8) {
                        c = F8;
                    } else if (keysym == XK_F9) {
                        c = F9;
                    } else if (keysym == XK_F10) {
                        c = F10;
                    } else if (keysym == XK_L1) {
                        c = STOP;
                    } else if (keysym == XK_L2) {
                        c = AGAIN;
                    } else if (keysym == XK_L3) {
                        c = PROPS;
                    } else if (keysym == XK_L4) {
                        c = UNDO;
                    } else if (keysym == XK_L5) {
                        c = FRONT;
                    } else if (keysym == XK_L6) {
                        c = COPY;
                    } else if (keysym == XK_L7) {
                        c = OPEN;
                    } else if (keysym == XK_L8) {
                        c = PASTE;
                    } else if (keysym == XK_L9) {
                        c = FID;
                    } else if (keysym == XK_L10) {
                        c = CUT;
                    } else if (keysym == XK_Help) {
                        c = HELP;
                    }
                }
            }
            if (c != 0) {
                if (key_b & ShiftMask) {
                    c = c + 512;
                }
                return (c);
            }
            break;
        case ButtonPress:
            key_b = report.xbutton.state;
            e_mouse.k = (key_b & ShiftMask) ? 3 : 0 +
                        (key_b & ControlMask) ? 4 : 0 +
                        (key_b & WpeXInfo.altmask) ? 8 : 0;
            e_mouse.x = report.xbutton.x / WpeXInfo.font_width;
            e_mouse.y = report.xbutton.y / WpeXInfo.font_height;
            c = 0;
            if (report.xbutton.button == 1) {
                c |= 1;
            }
            if (report.xbutton.button == 2) {
                c |= 2;
            }
            if (report.xbutton.button == 3) {
                c |= 4;
            }
            return (-c);
        case SelectionRequest:
            if (WpeXInfo.selection) {
                se.type = SelectionNotify;
                se.display = report.xselectionrequest.display;
                se.requestor = report.xselectionrequest.requestor;
                se.selection = report.xselectionrequest.selection;
                se.time = report.xselectionrequest.time;
                se.target = report.xselectionrequest.target;
                if (report.xselectionrequest.property == None)
                    report.xselectionrequest.property =
                        report.xselectionrequest.target;
                /* Xt asks for TARGETS.  Should probably support that. */
                if (report.xselectionrequest.target == WpeXInfo.text_atom) {
                    se.property = report.xselectionrequest.property;
                    XChangeProperty (se.display, se.requestor, se.property,
                                     se.target, 8, PropModeReplace,
                                     WpeXInfo.selection,
                                     strlen ((const char *) WpeXInfo.
                                             selection));
                } else {
                    se.property = None;
                }
                XSendEvent (WpeXInfo.display, se.requestor, False, 0,
                            (XEvent *) & se);
            }
            break;
        case SelectionClear:
            if (WpeXInfo.selection) {
                free (WpeXInfo.selection);
                WpeXInfo.selection = NULL;
            }
            break;
        default:
            break;
        }
    }
    return (0);
}

int
e_x_kbhit ()
{
    XEvent report;
    KeySym keysym;
    int charcount;
    char buffer[BUFSIZE];
    int c;
    unsigned int key_b;

    e_u_refresh ();

    if (XCheckMaskEvent
            (WpeXInfo.display, ButtonPressMask | KeyPressMask, &report) == False) {
        return (0);
    }

    if (report.type == ButtonPress) {
        key_b = report.xbutton.state;
        e_mouse.k = (key_b & ShiftMask) ? 3 : 0;
        e_mouse.x = report.xbutton.x / WpeXInfo.font_width;
        e_mouse.y = report.xbutton.y / WpeXInfo.font_height;
        c = 0;
        if (report.xbutton.button == 1) {
            c |= 1;
        }
        if (report.xbutton.button == 2) {
            c |= 2;
        }
        if (report.xbutton.button == 3) {
            c |= 4;
        }
        return (-c);
    } else {
        charcount = e_XLookupString (&report.xkey, buffer, BUFSIZE,
                                     &keysym, NULL);
        if (charcount == 1) {
            return (*buffer);
        } else {
            return (0);
        }
    }
}

void
e_setlastpic (we_view_t * view)
{
    extern we_view_t *e_X_l_pic;

    e_X_l_pic = view;
}

int
fk_x_locate (int x, int y)
{
    cur_x = x;
    return (cur_y = y);
}

int
fk_x_cursor (int x)
{
    return (cur_on = x);
}

int
e_x_sys_ini ()
{
    return (0);
}

int
e_x_sys_end ()
{
    return (0);
}

int
fk_x_putchar (int c)
{
    return (fputc (c, stdout));
}

int
x_bioskey ()
{
    return (e_mouse.k);
}

int
e_x_system (const char *exe)
{
    FILE *fp;
    int ret;
    char file[80];
    char *string;

    sprintf (file, "%s/we_sys_tmp", e_tmp_dir);
    string = malloc (strlen (XTERM_CMD) + strlen (exe) + strlen (file) +
                     strlen (user_shell) + 40);
    if (!(fp = fopen (file, "w+"))) {
        free (string);
        return (-1);
    }
    fputs ("$*\necho type \\<Return\\> to continue\nread i\n", fp);
    fclose (fp);
    chmod (file, 0700);
    if (exe[0] == '/')
        sprintf (string, "%s -geometry 80x25-0-0 +sb -e %s %s %s", XTERM_CMD,
                 user_shell, file, exe);
    else
        sprintf (string, "%s -geometry 80x25-0-0 +sb -e %s %s ./%s", XTERM_CMD,
                 user_shell, file, exe);
    ret = system (string);
    remove (file);
    free (string);
    return (ret);
}

int
e_x_repaint_desk (we_window_t * window)
{
    we_control_t *control = window->edit_control;
    int i, g[4];
    extern we_view_t *e_X_l_pic;
    we_view_t *sv_pic = NULL, *nw_pic = NULL;

    if (e_X_l_pic && e_X_l_pic != control->window[control->mxedt]->view) {
        sv_pic = e_X_l_pic;
        nw_pic = e_open_view (e_X_l_pic->a.x, e_X_l_pic->a.y, e_X_l_pic->e.x,
                              e_X_l_pic->e.y, 0, 2);
    }
    old_cursor_x = cur_x;
    old_cursor_y = cur_y;
    e_alloc_global_screen ();
    if (control->mxedt < 1) {
        e_cls (window->colorset->df.fg_bg_color, window->colorset->dc);
        e_ini_desk (window->edit_control);
        if (nw_pic) {
            e_close_view (nw_pic, 1);
            e_X_l_pic = sv_pic;
        }
        return (0);
    }
    ini_repaint (control);
    e_abs_refr ();
    for (i = control->mxedt; i >= 1; i--) {
        free (control->window[i]->view->p);
        free (control->window[i]->view);
    }
    for (i = 0; i <= control->mxedt; i++) {
        if (control->window[i]->e.x >= max_screen_cols()) {
            control->window[i]->e.x = max_screen_cols() - 1;
        }
        if (control->window[i]->e.y >= max_screen_lines() - 1) {
            control->window[i]->e.y = max_screen_lines() - 2;
        }
        if (control->window[i]->e.x - control->window[i]->a.x < 26) {
            control->window[i]->a.x = control->window[i]->e.x - 26;
        }
        if (!DTMD_ISTEXT (control->window[i]->dtmd) && control->window[i]->e.y - control->window[i]->a.y < 9) {
            control->window[i]->a.y = control->window[i]->e.y - 9;
        } else if (DTMD_ISTEXT (control->window[i]->dtmd)
                   && control->window[i]->e.y - control->window[i]->a.y < 3) {
            control->window[i]->a.y = control->window[i]->e.y - 3;
        }
    }
    for (i = 1; i < control->mxedt; i++) {
        e_firstl (control->window[i], 0);
        e_write_screen (control->window[i], 0);
    }
    e_firstl (control->window[i], 1);
    e_write_screen (control->window[i], 1);
    if (nw_pic) {
        e_close_view (nw_pic, 1);
        e_X_l_pic = sv_pic;
    }
    g[0] = 2;
    fk_u_mouse (g);
    end_repaint ();
    e_cursor (control->window[i], 1);
    g[0] = 0;
    fk_u_mouse (g);
    g[0] = 1;
    fk_u_mouse (g);
    return (0);
}


int
fk_x_mouse (int *g)
{
    Window tmp_win, tmp_root;
    int root_x, root_y, x, y;
    unsigned int key_b;

    if (!XQueryPointer (WpeXInfo.display, WpeXInfo.window, &tmp_root, &tmp_win,
                        &root_x, &root_y, &x, &y, &key_b)) {
        g[2] = e_mouse.x * 8;
        g[3] = e_mouse.y * 8;
        g[0] = g[1] = 0;
        return (0);
    }
    g[0] = 0;
    if (key_b & Button1Mask) {
        g[0] |= 1;
    }
    if (key_b & Button2Mask) {
        g[0] |= 4;
    }
    if (key_b & Button3Mask) {
        g[0] |= 2;
    }
    g[1] = g[0];
    g[2] = x / WpeXInfo.font_width * 8;
    g[3] = y / WpeXInfo.font_height * 8;
    return (g[1]);
}

int
e_x_cp_X_to_buffer (we_window_t * window)
{

    we_buffer_t *b0 = window->edit_control->window[0]->buffer;
    e_x_empty_buffer(b0);

    x_selection_t xsel = e_x_get_X_selection();
    if (xsel.success) {
        /* paste x selection string into window buffer */
        e_x_paste_X_selection(b0, xsel);
        e_x_free_X_selection(xsel);

        we_screen_t *s0 = window->edit_control->window[0]->screen;
        e_x_reinit_marked_area(s0, b0);
    }

    return 0;
}

int
e_x_copy_X_buffer (we_window_t * window)
{
    e_u_cp_X_to_buffer (window);
    e_edt_einf (window);
    return (0);
}

int
e_x_paste_X_buffer (we_window_t * window)
{
    we_buffer_t *b0 = window->edit_control->window[0]->buffer;
    we_screen_t *s0 = window->edit_control->window[0]->screen;
    int i, n;
    unsigned int j;

    e_edt_copy (window);
#if defined SELECTION
    if (WpeXInfo.selection) {
        free (WpeXInfo.selection);
        WpeXInfo.selection = NULL;
    }
#endif
    if ((s0->mark_end.y == 0 && s0->mark_end.x == 0) ||
            s0->mark_end.y < s0->mark_begin.y) {
        return (0);
    }
    if (s0->mark_end.y == s0->mark_begin.y) {
        if (s0->mark_end.x < s0->mark_begin.x) {
            return (0);
        }
        n = s0->mark_end.x - s0->mark_begin.x;
#if defined SELECTION
        WpeXInfo.selection = malloc (n + 1);
        // TODO: check malloc result
        strncpy ((char *) WpeXInfo.selection,
                 (char *) b0->buflines[s0->mark_begin.y].s + s0->mark_begin.x, n);
        WpeXInfo.selection[n] = 0;
        XSetSelectionOwner (WpeXInfo.display, WpeXInfo.selection_atom,
                            WpeXInfo.window, CurrentTime);
#else
        XStoreBytes (WpeXInfo.display,
                     b0->buflines[s0->mark_begin.y].s + s0->mark_begin.x, n);
#endif
        return (0);
    }
    WpeXInfo.selection = malloc (b0->buflines[s0->mark_begin.y].nrc * sizeof (char));
    for (n = 0, j = s0->mark_begin.x; j < b0->buflines[s0->mark_begin.y].nrc;
            j++, n++) {
        WpeXInfo.selection[n] = b0->buflines[s0->mark_begin.y].s[j];
    }
    for (i = s0->mark_begin.y + 1; i < s0->mark_end.y; i++) {
        WpeXInfo.selection =
            realloc (WpeXInfo.selection, (n + b0->buflines[i].nrc) * sizeof (char));
        for (j = 0; j < b0->buflines[i].nrc; j++, n++) {
            WpeXInfo.selection[n] = b0->buflines[i].s[j];
        }
    }
    WpeXInfo.selection =
        realloc (WpeXInfo.selection, (n + s0->mark_end.x + 1) * sizeof (char));
    for (j = 0; j < (unsigned) s0->mark_end.x; j++, n++) {
        WpeXInfo.selection[n] = b0->buflines[i].s[j];
    }
    WpeXInfo.selection[n] = 0;
#if defined SELECTION
    XSetSelectionOwner (WpeXInfo.display, WpeXInfo.selection_atom,
                        WpeXInfo.window, CurrentTime);
#else
    XStoreBytes (WpeXInfo.display, WpeXInfo.selection, n);
    free (WpeXInfo.selection);
    WpeXInfo.selection = NULL;
#endif
    return (0);
}

/**
 * Retrieves the current selection from the X buffer
 *
 * @return x_selection_t the selection results containing indiction of success, length of
 * the string and the string.
 *
 */
x_selection_t
e_x_get_X_selection()
{
    /** The local length variable being computed */
    int n;
    /** The local string variable for the resulting selection. */
    unsigned char *str;

    /** The selection to be returned, including an indication of success */
    x_selection_t xsel;

    /** Local variable necessary for call to XWindows functions. */
    XEvent report;
    /**
     * Indicates failure if type == None.
     *
     * Local variable necessary for call to XWindows functions.
     */
    Atom type;
    /** Local variable necessary for call to XWindows functions. */
    int format;
    /** Local variable necessary for call to XWindows functions. */
    unsigned long bytes_left;
    /** Local variable necessary for call to XWindows functions. */
    unsigned long nitems;

    xsel.success = false;
    xsel.len = 0;
    xsel.str = NULL;

#if defined SELECTION
    if (WpeXInfo.selection) {
        str = (unsigned char *) WpeStrdup ((const char *) WpeXInfo.selection);
        n = strlen ((const char *) str);
    } else {
        /* Should check for errors especially failure to send SelectionNotify */
        XConvertSelection (WpeXInfo.display, WpeXInfo.selection_atom,
                           WpeXInfo.text_atom, WpeXInfo.property_atom,
                           WpeXInfo.window, CurrentTime);
        n = 0;
        while (!XCheckTypedEvent (WpeXInfo.display, SelectionNotify, &report)) {
            /* Should probably have a better timeout period than this. */
            sleep (0);
            n++;
            if (n > 1000) {
                return xsel;
            }
        }
        if (WpeXInfo.property_atom == None) {
            return xsel;
        }
        XGetWindowProperty (WpeXInfo.display, WpeXInfo.window,
                            WpeXInfo.property_atom, 0, 1000000, FALSE,
                            WpeXInfo.text_atom, &type, &format, &nitems,
                            &bytes_left, &str);
        if (type == None) {
            /* Specified property does not exist */
            return xsel;
        }
        n = strlen ((const char *) str);
    }
#else
    str = XFetchBytes (WpeXInfo.display, &n);
#endif
    xsel.success = true;
    xsel.len = n;
    xsel.str = str;
    return xsel;
}

void e_x_empty_buffer(we_buffer_t *buffer)
{
    for (int i = 1; i < buffer->mxlines; i++) {
        free (buffer->buflines[i].s);
    }
    buffer->mxlines = 1;
    *(buffer->buflines[0].s) = WPE_WR;
    *(buffer->buflines[0].s + 1) = '\0';
    buffer->buflines[0].len = 0;
}

void
e_x_free_X_selection(x_selection_t xsel)
{
#if defined SELECTION
    if (WpeXInfo.selection) {
        free (xsel.str);
    } else
#endif
        XFree (xsel.str);
}

void e_x_paste_X_selection(we_buffer_t *buffer,  x_selection_t xsel)
{
    /* copy input parameters for easier reference */
    unsigned char *str = xsel.str;
    int n = xsel.len;

    int i;	/**< index into string to fetch characters */
    int j;  /**< index into buffer to put characters */
    int k;	/**< index into buffer to put characters into the right line */

    for (i = k = 0; i < n; i++, k++) {
        for (j = 0; i < n && str[i] != '\n' && j < buffer->mx.x - 1; j++, i++) {
            buffer->buflines[k].s[j] = str[i];
        }
        if (i < n) {
            e_new_line (k + 1, buffer);
            if (str[i] == '\n') {
                buffer->buflines[k].s[j] = WPE_WR;
                buffer->buflines[k].nrc = j + 1;
            } else {
                buffer->buflines[k].nrc = j;
            }
            buffer->buflines[k].s[j + 1] = '\0';
            buffer->buflines[k].len = j;
        } else {
            buffer->buflines[k].s[j] = '\0';
            buffer->buflines[k].nrc = buffer->buflines[k].len = j;
        }
    }
}

/**
 * Initializes the marked area on the screen to be the complete buffer.
 *
 */
void e_x_reinit_marked_area(we_screen_t *screen, we_buffer_t *buffer)
{
    screen->mark_begin.x = screen->mark_begin.y = 0;
    screen->mark_end.y = buffer->mxlines - 1;
    screen->mark_end.x = buffer->buflines[buffer->mxlines - 1].len;
}

#endif // ifndef NO_XWINDOWS
