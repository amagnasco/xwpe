/** \file we_opt.c                                         */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <string.h>
#include "keys.h"
#include "messages.h"
#include "options.h"
#include "we_mouse.h"
#include "we_screen.h"
#include "we_term.h"
#include "we_xterm.h"
#include "model.h"
#include "edit.h"
#include "we_opt.h"
#include "WeExpArr.h"
#include "we_prog.h"
#include "WeString.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int WpeReadGeneral (we_control_t * control, char *section, char *option, char *value);
int WpeWriteGeneral (we_control_t * control, char *section, FILE * opt_file);
int WpeReadColor (we_control_t * control, char *section, char *option, char *value);
int WpeWriteColor (we_control_t * control, char *section, FILE * opt_file);
int WpeReadProgramming (we_control_t * control, char *section, char *option, char *value);
int WpeWriteProgramming (we_control_t * control, char *section, FILE * opt_file);
int WpeReadLanguage (we_control_t * control, char *section, char *option, char *value);
int WpeWriteLanguage (we_control_t * control, char *section, FILE * opt_file);

#define E_HLP_NUM 26
char *e_hlp_str[E_HLP_NUM];

extern char *info_file;
#ifdef DEBUGGER
extern int e_deb_type;
#endif
extern we_colorset_t *u_fb, *x_fb;

#define OPTION_SECTIONS 4
#define OPT_SECTION_GENERAL     "General"
#define OPT_SECTION_COLOR       "Color"
#define OPT_SECTION_PROGRAMMING "Programming"
#define OPT_SECTION_LANGUAGE    "Language"

typedef struct wpeOptionSection {
    char* section;
    int (*function)(we_control_t* control, char* section, char* option, char* value);
} WpeOptionSection;

WpeOptionSection WpeSectionRead[] = {
    {OPT_SECTION_GENERAL, WpeReadGeneral},
    {OPT_SECTION_COLOR, WpeReadColor},
    {OPT_SECTION_PROGRAMMING, WpeReadProgramming},
    {OPT_SECTION_LANGUAGE, WpeReadLanguage}
};

/*    About WE      */
int
e_about_WE (we_window_t * window)
{
    we_view_t *view = NULL;
    int xa = 10, ya = 4, xe = xa + 50, ye = ya + 13;
    char tmp[40];

    fk_u_cursor (0);
    view =
        e_std_view (xa, ya, xe, ye, NULL, 1, window->colorset->nr.fg_bg_color, window->colorset->nt.fg_bg_color,
                    window->colorset->ne.fg_bg_color);
    if (view == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
        return (WPE_ESC);
    }

    sprintf (tmp, "          Version %s          ", VERSION);
#ifdef UNIX
    if (WpeIsXwin () && WpeIsProg ()) {
        e_pr_str (xa + 7, ya + 3, "  XWindow Programming Environment  ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 7, ya + 4, "             ( XWPE )              ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
    } else if (WpeIsProg ()) {
        e_pr_str (xa + 7, ya + 3, "   Window Programming Environment  ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 7, ya + 4, "              ( WPE )              ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
    } else if (WpeIsXwin ()) {
        e_pr_str (xa + 7, ya + 3, "          XWindow Editor           ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 7, ya + 4, "              ( XWE )              ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
    } else
#endif
    {
        e_pr_str (xa + 7, ya + 3, "           Window Editor           ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 7, ya + 4, "               ( WE )              ",
                  window->colorset->et.fg_bg_color, 0, 0, 0, 0);
    }
    e_pr_str (xa + 7, ya + 5, tmp, window->colorset->et.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 2, ya + 8, "Copyright (C) 1993 Fred Kruse", window->colorset->nt.fg_bg_color, 0,
              0, 0, 0);
    e_pr_str (xa + 2, ya + 9, "This Sofware comes with ABSOLUTELY NO WARRANTY;",
              window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 2, ya + 10, "This is free software, and you are welcome to",
              window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 2, ya + 11, "redistribute it under certain conditions;",
              window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 2, ya + 12, "See \'Help\\Editor\\GNU Pub...\' for details.",
              window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
#if  MOUSE
    while (e_mshit () != 0)
        ;
    e_u_getch ();
    while (e_mshit () != 0)
        ;
#else
    e_u_getch ();
#endif
    e_close_view (view, 1);
    return (0);
}

/*    delete everything    */
int
e_clear_desk (we_window_t * window)
{
    int i;
    we_control_t *control = window->edit_control;
#if  MOUSE
    int g[4];			/*  = { 2, 0, 0, 0, };  */

    g[0] = 2;
#endif
    fk_u_cursor (0);
    for (i = control->mxedt; i > 0; i--) {
        window = control->window[control->mxedt];
        if (e_close_window (window) == WPE_ESC) {
            return (WPE_ESC);
        }
    }
    control->mxedt = 0;
#if  MOUSE
    fk_u_mouse (g);
#endif
    e_ini_desk (control);
#if  MOUSE
    g[0] = 1;
    fk_u_mouse (g);
#endif
    return (0);
}

/*    redraw everything */
int
e_repaint_desk (we_window_t * window)
{
    /* int j; */
    we_control_t *control = window->edit_control;
    int i;
#if MOUSE
    int g[4];
#endif
#ifndef NO_XWINDOWS
    extern we_view_t *e_X_l_pic;
    we_view_t *sv_pic = NULL, *nw_pic = NULL;

    if (WpeIsXwin ()) {
        if (e_X_l_pic && e_X_l_pic != control->window[control->mxedt]->view) {
            sv_pic = e_X_l_pic;
            nw_pic = e_open_view (e_X_l_pic->a.x, e_X_l_pic->a.y,
                                  e_X_l_pic->e.x, e_X_l_pic->e.y, 0, 2);
        }
        old_cursor_x = cur_x;
        old_cursor_y = cur_y;
        e_alloc_global_screen();
    }
#endif
    if (control->mxedt < 1) {
        e_cls (window->colorset->df.fg_bg_color, window->colorset->dc);
        e_ini_desk (window->edit_control);
#ifndef NO_XWINDOWS
        if ((WpeIsXwin ()) && nw_pic) {
            e_close_view (nw_pic, 1);
            e_X_l_pic = sv_pic;
        }
#endif
        return (0);
    }
    control->curedt = control->mxedt;
    ini_repaint (control);
    e_abs_refr ();
    for (i = 1; i < control->mxedt; i++) {
        e_firstl (control->window[i], 0);
        e_write_screen (control->window[i], 0);
    }
    e_firstl (control->window[i], 1);
    e_write_screen (control->window[i], 1);
#ifndef NO_XWINDOWS
    if (WpeIsXwin () && nw_pic) {
        e_close_view (nw_pic, 1);
        e_X_l_pic = sv_pic;
    }
#endif
#if  MOUSE
    g[0] = 2;
    fk_u_mouse (g);
#endif
    end_repaint ();
    e_cursor (control->window[i], 1);
#if  MOUSE
    g[0] = 0;
    fk_u_mouse (g);
    g[0] = 1;
    fk_u_mouse (g);
#endif
    return (0);
}

/*    write system information   */
int
e_sys_info (we_window_t * window)
{
    we_view_t *view = NULL;
    char tmp[80];
    int xa = 10, ya = 5, xe = xa + 60, ye = ya + 8;

    fk_u_cursor (0);
    view =
        e_std_view (xa, ya, xe, ye, " Information ", 1, window->colorset->nr.fg_bg_color, window->colorset->nt.fg_bg_color,
                    window->colorset->ne.fg_bg_color);
    if (view == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
        return (WPE_ESC);
    }
    e_pr_str (xa + 3, ya + 2, " Current File: ", window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 3, ya + 4, " Current Directory: ", window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 3, ya + 6, " Number of Files: ", window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    if (strcmp (window->datnam, "Clipboard") != 0) {
        if (strcmp (window->dirct, window->edit_control->dirct) == 0) {
            e_pr_str (xa + 23, ya + 2, window->datnam, window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
        } else {
            strcpy (tmp, window->dirct);
            strcat (tmp, DIRS);
            strcat (tmp, window->datnam);
            e_pr_str (xa + 23, ya + 2, tmp, window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
        }
    }
    e_pr_str (xa + 23, ya + 4, window->edit_control->dirct, window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
    e_pr_str (xa + 23, ya + 6,
              WpeNumberToString (window->edit_control->mxedt, WpeNumberOfPlaces (window->edit_control->mxedt),
                                 tmp), window->colorset->nt.fg_bg_color, 0, 0, 0, 0);
#if  MOUSE
    while (e_mshit () != 0);
    e_u_getch ();
    while (e_mshit () != 0);
#else
    e_u_getch ();
#endif
    e_close_view (view, 1);
    return (0);
}

/*   color adjustments  */
int
e_ad_colors (we_window_t * window)
{
    int n, xa = 48, ya = 2, num = 4;
    OPTK *opt = malloc (num * sizeof (OPTK));

    opt[0].t = "Editor Colors";
    opt[0].x = 0;
    opt[0].o = 'E';
    opt[1].t = "Desk Colors";
    opt[1].x = 0;
    opt[1].o = 'D';
    opt[2].t = "Option Colors";
    opt[2].x = 0;
    opt[2].o = 'O';
    opt[3].t = "Progr. Colors";
    opt[3].x = 0;
    opt[3].o = 'P';

    n = e_opt_sec_box (xa, ya, num, opt, window, 1);

    free (opt);
    if (n < 0) {
        return (WPE_ESC);
    }

    return (e_ad_colors_md (window, n));
}

int
e_ad_colors_md (we_window_t * window, int md)
{
    int sw = 0, xa = 0, ya = 1, xe = xa + 79, ye = ya + 22;
    we_view_t *view;

    view = e_std_view (xa, ya, xe, ye, "Adjust Colors", 1, window->colorset->er.fg_bg_color,
                       window->colorset->et.fg_bg_color, window->colorset->es.fg_bg_color);
    if (view == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
        return (WPE_ESC);
    }
    sw = e_dif_colors (sw, xe - 13, ya + 1, window, md);
    e_close_view (view, 1);
    e_repaint_desk (window);
    return (sw);
}

/*   install/execute color adjustments */
int
e_dif_colors (int sw, int xa, int ya, we_window_t * window, int md)
{
    we_color_t *frb = &(window->colorset->er);
    int c = 0, bg, num;

    if (md == 1) {
        bg = 11;
        num = 5;
    } else if (md == 2) {
        bg = 16;
        num = 15;
    } else if (md == 3) {
        bg = 32;
        num = 5;
    } else {
        bg = 0;
        num = 11;
    }
    while (c != WPE_ESC && c > -2) {
        e_pr_dif_colors (sw, xa, ya, window, 1, md);
        e_pr_u_colorsets (xa - 28, ya + 1, frb[sw + bg].fg_color, frb[sw + bg].bg_color, window, 0);
        e_pr_ed_beispiel (1, 2, window, sw, md);
#if  MOUSE
        if ((c = e_u_getch ()) == -1) {
            c = e_opt_cw_mouse (xa, ya, md);
        }
#else
        c = e_u_getch ();
#endif
        if (c >= 375 && c <= 393) {
            sw = c - 375;
        } else if (c == 326) {
            sw = 0;
        } else if (c == 334) {
            sw = num - 1;
        } else if (c == 327) {
            sw = (sw == 0) ? num - 1 : sw - 1;
        } else if (c == 335) {
            sw = (sw == num - 1) ? 0 : sw + 1;
        } else if (c == WPE_CR || c == 285 || c == 330) {
            e_pr_dif_colors (sw, xa, ya, window, 0, md);
            *(frb + sw + bg) =
                e_n_u_clr (e_frb_u_menue (sw, xa - 28, ya + 1, window, md));
        }
    }
    return (c);
}

char *text[] = { "Border", "Bord. Bt.", "Text", "Txt Mrk.1", "Txt Mrk.2",
                 "Scrollbar", "Help Hdr.", "Help Btt.", "Help Mrk.",
                 "Breakpnt.", "Stop Brk.",
                 "Border", "Bord. Bt.", "Text", "Txt Mrk.", "Backgrnd.",
                 "Border", "Bord. Bt.", "Text", "Text Sw.",
                 "Write", "Wrt. Mrk.", "Data", "Data M.A", "Data M.P",
                 "Switch", "Swtch. S.", "Swtch. A.", "Button",
                 "Bttn. Sw.", "Bttn. Ak.",
                 "Text", "Res. Wrd.", "Constants", "Pre-Proc.", "Comments"
               };

/*   draw color box */
void
e_pr_dif_colors (int sw, int xa, int ya, we_window_t * window, int sw2, int md)
{
    int i, rfrb, cfrb, xe = xa + 12, ye, bg;
    char *header;

    if (md == 1) {
        ye = ya + 6;
        bg = 11;
        header = "Desk";
    } else if (md == 2) {
        ye = ya + 16;
        bg = 16;
        header = "Options";
    } else if (md == 3) {
        ye = ya + 6;
        bg = 31;
        header = "C-Prog.";
    } else {
        ye = ya + 12;
        bg = 0;
        header = "Editor";
    }
    rfrb = sw2 == 0 ? window->colorset->nt.fg_bg_color : window->colorset->fs.fg_bg_color;
    e_std_window (xa, ya, xe, ye, header, 0, rfrb, 0);
    for (i = 0; i < ye - ya - 1; i++) {
        cfrb = i == sw ? window->colorset->fz.fg_bg_color : window->colorset->ft.fg_bg_color;
        e_pr_str_wsd (xa + 2, ya + 1 + i, text[i + bg], cfrb, 0, 0, 0, xa + 1,
                      xe - 1);
    }
}

/*    color menu  */
int
e_frb_x_menue (int sw, int xa, int ya, we_window_t * window, int md)
{
    we_color_t *frb = &(window->colorset->er);
    int c = 1, fsv = frb[sw].fg_bg_color, x, y;

    if (md == 1) {
        sw += 11;
    } else if (md == 2) {
        sw += 16;
    } else if (md == 3) {
        sw += 32;
    }
    y = frb[sw].bg_color;
    x = frb[sw].fg_color;
    do {
        if (c == CRI && y < 7) {
            y++;
        } else if (c == CLE && y > 0) {
            y--;
        } else if (c == CUP && x > 0) {
            x--;
        } else if (c == CDO && x < 15) {
            x++;
        } else if (c >= 1000 && c < 1256) {
            x = (c - 1000) / 16;
            y = (c - 1000) % 16;
        }
        e_pr_x_colorsets (xa, ya, x, y, window, 1);
        frb[sw] = e_s_u_clr (x, y);
        e_pr_ed_beispiel (1, 2, window, sw, md);
#if  MOUSE
        if ((c = e_u_getch ()) == -1) {
            c = e_opt_ck_mouse (xa, ya, md);
        }
#else
        c = e_u_getch ();
#endif
    } while (c != WPE_ESC && c != WPE_CR && c > -2);
    if (c == WPE_ESC || c < -1) {
        frb[sw] = e_n_u_clr (fsv);
    }
    return (frb[sw].fg_bg_color);
}

/*   draw color box  */
void
e_pr_x_colorsets (int xa, int ya, int x, int y, we_window_t * window, int sw)
{
    int i, j, rfrb, ffrb, xe = xa + 25, ye = ya + 18;

    rfrb = sw == 0 ? window->colorset->nt.fg_bg_color : window->colorset->fs.fg_bg_color;
    ffrb = rfrb % 16;
    e_std_window (xa - 2, ya - 1, xe, ye, "Colors", 0, rfrb, 0);
    /*     e_pr_str((xa+xe-8)/2, ya-1, "Colors", rfrb, 0, 1,
                                            window->colorset->ms.fg_color+16*(rfrb/16), 0);
    */
    for (j = 0; j < 8; j++)
        for (i = 0; i < 16; i++) {
            e_pr_char (3 * j + xa, i + ya + 1, ' ', 16 * j + i);
            e_pr_char (3 * j + xa + 1, i + ya + 1, 'x', 16 * j + i);
            e_pr_char (3 * j + xa + 2, i + ya + 1, ' ', 16 * j + i);
        }
    for (i = 0; i < 18; i++) {
        e_pr_char (xa - 1, i + ya, ' ', rfrb);
        e_pr_char (xe - 1, i + ya, ' ', rfrb);
    }
    for (j = 0; j < 25; j++) {
        e_pr_char (j + xa - 1, ya, ' ', rfrb);
        e_pr_char (j + xa - 1, ye - 1, ' ', rfrb);
    }
#ifdef NEWSTYLE
    if (!WpeIsXwin ()) {
#endif
        for (i = 0; i < 3; i++) {
            e_pr_char (3 * y + xa + i, x + ya, RE5,
                       x > 0 ? 16 * y + ffrb : rfrb);
            e_pr_char (3 * y + xa + i, x + ya + 2, RE5,
                       x < 15 ? 16 * y + ffrb : rfrb);
        }
        e_pr_char (3 * y + xa - 1, x + ya + 1, RE6,
                   y < 1 ? rfrb : 16 * (y - 1) + ffrb);
        e_pr_char (3 * y + xa + 3, x + ya + 1, RE6,
                   y > 6 ? rfrb : 16 * (y + 1) + ffrb);
        e_pr_char (3 * y + xa - 1, x + ya, RE1,
                   (y < 1 || x < 1) ? rfrb : 16 * (y - 1) + ffrb);
        e_pr_char (3 * y + xa + 3, x + ya, RE2,
                   (y > 6 || x < 1) ? rfrb : 16 * (y + 1) + ffrb);
        e_pr_char (3 * y + xa - 1, x + ya + 2, RE3,
                   (y < 1 || x > 14) ? rfrb : 16 * (y - 1) + ffrb);
        e_pr_char (3 * y + xa + 3, x + ya + 2, RE4,
                   (y > 6 || x > 14) ? rfrb : 16 * (y + 1) + ffrb);
#ifdef NEWSTYLE
    } else {
        e_make_xrect (3 * y + xa, x + ya + 1, 3 * y + xa + 2, x + ya + 1, 1);
    }
#endif
}

/*    draw color example   */
void
e_pr_ed_beispiel (int xa, int ya, we_window_t * window, int sw, int md)
{
    we_color_t *frb = &(window->colorset->er);
    we_colorset_t *fb = window->colorset;
    int i, j, xe = xa + 31, ye = ya + 19;

    frb[sw] = e_s_u_clr (frb[sw].fg_color, frb[sw].bg_color);
    if (md == 1) {
        e_blk (xe - xa + 1, xa + 1, ya, fb->mt.fg_bg_color);
        e_pr_str_wsd (xa + 5, ya, "Edit", fb->mt.fg_bg_color, 0, 1, window->colorset->ms.fg_bg_color,
                      xa + 3, xa + 11);
        e_pr_str_wsd (xa + 18, ya, "Options", fb->mz.fg_bg_color, 0, 0, window->colorset->ms.fg_bg_color,
                      xa + 16, xa + 27);
        for (i = ya + 1; i < ye; i++)
            for (j = xa + 1; j <= xe + 1; j++) {
                e_pr_char (j, i, fb->dc, fb->df.fg_bg_color);
                e_pr_char (j, i, fb->dc, fb->df.fg_bg_color);
            }
        e_std_window (xa + 17, ya + 1, xa + 26, ya + 3, NULL, 0, fb->mr.fg_bg_color, 0);
        e_pr_str (xa + 19, ya + 2, "Colors", fb->mt.fg_bg_color, 0, 1, fb->ms.fg_bg_color, 0);
        e_blk (xe - xa + 1, xa + 1, ye, fb->mt.fg_bg_color);
        e_pr_str_wsd (xa + 4, ye, "Alt-F3 Close Window",
                      fb->mt.fg_bg_color, 0, 6, fb->ms.fg_bg_color, xa + 2, xa + 25);
    } else if (md == 2) {
        e_std_window (xa, ya, xe, ye, "Message", 1, fb->nr.fg_bg_color, window->colorset->ne.fg_bg_color);
        for (i = ya + 1; i < ye; i++) {
            e_blk (xe - xa - 1, xa + 1, i, fb->nt.fg_bg_color);
        }
        e_pr_str (xa + 4, ya + 2, "Name:", window->colorset->nt.fg_bg_color, 0, 1,
                  window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
        e_pr_str (xa + 5, ya + 3, "Active Write-Line ", fb->fa.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 4, ya + 5, "Name:", window->colorset->nt.fg_bg_color, 0, 1,
                  window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
        e_pr_str (xa + 5, ya + 6, "Passive Write-Line", fb->fr.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 4, ya + 8, "Data:", window->colorset->nt.fg_bg_color, 0, 1,
                  window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
        e_pr_str (xa + 5, ya + 9, "Active Marked ", window->colorset->fz.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 5, ya + 10, "Passive Marked", window->colorset->frft.fg_bg_color, 0, 0, 0,
                  0);
        e_pr_str (xa + 5, ya + 11, "Data Text     ", window->colorset->ft.fg_bg_color, 0, 0, 0, 0);
        e_pr_str (xa + 4, ya + 13, "Switches:", window->colorset->nt.fg_bg_color, 0, 1,
                  window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
        e_pr_str (xa + 5, ya + 14, "[X] Active Switch ", window->colorset->fsm.fg_bg_color, 0, 0, 0,
                  0);
        e_pr_str (xa + 5, ya + 15, "[ ] Passive Switch", window->colorset->fs.fg_bg_color, 4, 1,
                  window->colorset->nsft.fg_bg_color, window->colorset->fs.fg_bg_color);
        e_pr_str (xa + 6, ye - 2, "Button", window->colorset->nz.fg_bg_color, 0, -1, window->colorset->ns.fg_bg_color,
                  window->colorset->nt.fg_bg_color);
        e_pr_str (xe - 12, ye - 2, "Active", window->colorset->nm.fg_bg_color, 0, -1, window->colorset->nm.fg_bg_color,
                  window->colorset->nt.fg_bg_color);
#ifdef NEWSTYLE
        if (WpeIsXwin ()) {
            e_make_xrect (xa + 4, ya + 3, xa + 23, ya + 3, 1);
            e_make_xrect (xa + 4, ya + 6, xa + 23, ya + 6, 1);
            e_make_xrect (xa + 4, ya + 9, xa + 19, ya + 11, 1);
            e_make_xrect_abs (xa + 4, ya + 9, xa + 19, ya + 9, 0);
            e_make_xrect (xa + 4, ya + 14, xa + 23, ya + 15, 1);
            e_make_xrect_abs (xa + 4, ya + 14, xa + 23, ya + 14, 0);
        }
#endif
    } else {
        e_std_window (xa, ya, xe, ye, "Filename", 1, fb->er.fg_bg_color, fb->es.fg_bg_color);
        e_mouse_bar (xe, ya + 1, ye - ya - 1, 0, fb->em.fg_bg_color);
        e_mouse_bar (xa + 20, ye, 11, 1, fb->em.fg_bg_color);
        e_pr_char (xe - 3, ya, WZN, fb->es.fg_bg_color);
#ifdef NEWSTYLE
        if (!WpeIsXwin ()) {
#endif
            e_pr_char (xe - 4, ya, '[', fb->er.fg_bg_color);
            e_pr_char (xe - 2, ya, ']', fb->er.fg_bg_color);
#ifdef NEWSTYLE
        } else {
            e_make_xrect (xe - 4, ya, xe - 2, ya, 0);
        }
#endif
        for (i = ya + 1; i < ye; i++) {
            e_blk (xe - xa - 1, xa + 1, i, fb->et.fg_bg_color);
        }
        if (md == 3) {
            e_pr_str (xa + 4, ya + 3, "#Preprozessor Comands", fb->cp.fg_bg_color, 0, 0,
                      0, 0);
            e_pr_str (xa + 4, ya + 5, "This are C-Text Colors", fb->ct.fg_bg_color, 0, 0,
                      0, 0);
            e_pr_str (xa + 4, ya + 7, "int char {} [] ; ,", fb->cr.fg_bg_color, 0, 0, 0,
                      0);
            e_pr_str (xa + 4, ya + 9, "\"Constants\" 12 0x13", fb->ck.fg_bg_color, 0, 0,
                      0, 0);
            e_pr_str (xa + 4, ya + 11, "/*   Comments    */", fb->cc.fg_bg_color, 0, 0,
                      0, 0);
        } else {
            e_pr_str (xa + 4, ya + 3, "This are the Editor Colors", fb->et.fg_bg_color,
                      0, 0, 0, 0);
            e_pr_str (xa + 4, ya + 4, "And this is a marked Line", fb->ez.fg_bg_color, 0,
                      0, 0, 0);
            e_pr_str (xa + 4, ya + 5, "This is a found word", fb->et.fg_bg_color, 0, 0,
                      0, 0);
            e_pr_str (xa + 14, ya + 5, "found", fb->ek.fg_bg_color, 0, 0, 0, 0);
            e_pr_str (xa + 4, ya + 8, "Help Header", fb->hh.fg_bg_color, 0, 0, 0, 0);
            e_pr_str (xa + 4, ya + 9, "This is a marked Word", fb->et.fg_bg_color, 0, 0,
                      0, 0);
            e_pr_str (xa + 14, ya + 9, "marked", fb->hm.fg_bg_color, 0, 0, 0, 0);
            e_pr_str (xa + 4, ya + 10, "in the Help File", fb->et.fg_bg_color, 0, 0, 0,
                      0);
            e_pr_str (xa + 4, ya + 11, "Help Button", fb->hb.fg_bg_color, 0, 0, 0, 0);
            e_pr_str (xa + 4, ya + 14, "This is a Breakpoint", fb->db.fg_bg_color, 0, 0,
                      0, 0);
            e_pr_str (xa + 4, ya + 15, "Stop at Breakpoint", fb->dy.fg_bg_color, 0, 0, 0,
                      0);
        }
    }
    frb[sw] = e_s_u_clr (frb[sw].fg_color, frb[sw].bg_color);
}

/*   Save - Options - Menu   */
int
e_opt_save (we_window_t * window)
{
    int ret;
    char tmp[256];

    strcpy (tmp, window->edit_control->optfile);
    ret = e_add_arguments (tmp, "Save Option File", window, 0, AltS, NULL);
    if (ret) {
        window->edit_control->optfile =
            realloc (window->edit_control->optfile, (strlen (tmp) + 1) * sizeof (char));
        strcpy (window->edit_control->optfile, tmp);
        e_save_opt (window);
    }
    return (ret);
}

char *
WpeStringToValue (const char *str)
{
    char *answer, *cur_ans;
    const char *cur_str;
    int i, len;

    len = strlen (str);
    answer = malloc ((len * 2) * sizeof (char));
    for (i = strlen (str), cur_ans = answer, cur_str = str; i; i--, cur_str++) {
        if ((*cur_str == '\n') || (*cur_str == '\\')) {
            len++;
            cur_ans[0] = '\\';
            cur_ans[1] = ((*cur_str == '\n') ? 'n' : '\\');
            cur_ans++;
        } else {
            *cur_ans = *cur_str;
        }
        cur_ans++;
    }
    *cur_ans = 0;
    return answer;
}

char *
WpeValueToString (const char *value)
{
    char *answer, *cur_ans;
    const char *cur_val;
    int i;

    answer = malloc ((strlen (value) + 1) * sizeof (char));
    for (i = strlen (value), cur_ans = answer, cur_val = value; i;
            i--, cur_ans++) {
        if (*cur_val == '\\') {
            cur_val++;
            *cur_ans = ((*cur_val == 'n') ? '\n' : '\\');
        } else {
            *cur_ans = *cur_val;
        }
        cur_val++;
    }
    *cur_ans = 0;
    return answer;
}

int
WpeReadGeneral (we_control_t * control, char *section, char *option, char *value)
{
    UNUSED (section);
    if (WpeStrccmp ("Data", option) == 0) {
        control->dtmd = atoi (value);
    } else if (WpeStrccmp ("Autosave", option) == 0) {
        control->autosv = atoi (value);
    } else if (WpeStrccmp ("MaxColumn", option) == 0) {
        control->maxcol = atoi (value);
    } else if (WpeStrccmp ("Tab", option) == 0) {
        control->tabn = atoi (value);
    } else if (WpeStrccmp ("MaxChanges", option) == 0) {
        control->maxchg = atoi (value);
    } else if (WpeStrccmp ("NumUndo", option) == 0) {
        control->numundo = atoi (value);
    } else if (WpeStrccmp ("Options1", option) == 0) {
        control->flopt = atoi (value);
    } else if (WpeStrccmp ("Options2", option) == 0) {
        control->edopt = atoi (value);
    } else if (WpeStrccmp ("InfoDir", option) == 0) {
        info_file = WpeStrdup (value);
    } else if (WpeStrccmp ("AutoIndent", option) == 0) {
        control->autoindent = atoi (value);
    } else if (WpeStrccmp ("PrintCmd", option) == 0) {
        control->print_cmd = WpeStrdup (value);
    } else if (WpeStrccmp ("Version", option) == 0) {
        sscanf (value, "%d.%d.%d", &control->major, &control->minor, &control->patch);
    }
    return 0;
}

int
WpeWriteGeneral (we_control_t * control, char *section, FILE * opt_file)
{
    UNUSED (section);
    fprintf (opt_file, "Version : %s\n", VERSION);
    fprintf (opt_file, "Data : %d\n", control->dtmd);
    fprintf (opt_file, "Autosave : %d\n", control->autosv);
    fprintf (opt_file, "MaxColumn : %d\n", control->maxcol);
    fprintf (opt_file, "Tab : %d\n", control->tabn);
    fprintf (opt_file, "MaxChanges : %d\n", control->maxchg);
    fprintf (opt_file, "NumUndo : %d\n", control->numundo);
    fprintf (opt_file, "Options1 : %d\n", control->flopt);
    fprintf (opt_file, "Options2 : %d\n", control->edopt);
    fprintf (opt_file, "InfoDir : %s\n", info_file);
    fprintf (opt_file, "AutoIndent : %d\n", control->autoindent);
    fprintf (opt_file, "PrintCmd : %s\n", control->print_cmd);
    return 0;
}

int
WpeReadColor (we_control_t * control, char *section, char *option, char *value)
{
    we_colorset_t *fb = NULL;
    we_color_t *c = NULL;
    int convert = 0;		/* Convert old X11 colors to new colors */

    if (WpeStrccmp ("Term", section + strlen (OPT_SECTION_COLOR) + 1) == 0) {
        if (!u_fb) {
            u_fb = malloc (sizeof (we_colorset_t));
            we_colorset_init (u_fb);
        }
        fb = u_fb;
    }
    if (WpeStrccmp ("X11", section + strlen (OPT_SECTION_COLOR) + 1) == 0) {
        if (!x_fb) {
            x_fb = malloc (sizeof (we_colorset_t));
            we_colorset_init (x_fb);
        }
        fb = x_fb;
        if ((control->major <= 1) && (control->minor <= 5) && (control->patch <= 27)) {
            convert = 1;
        }
    }
    if (WpeStrccmp ("er", option) == 0) {
        c = &fb->er;
    } else if (WpeStrccmp ("es", option) == 0) {
        c = &fb->es;
    } else if (WpeStrccmp ("et", option) == 0) {
        c = &fb->et;
    } else if (WpeStrccmp ("ez", option) == 0) {
        c = &fb->ez;
    } else if (WpeStrccmp ("ek", option) == 0) {
        c = &fb->ek;
    } else if (WpeStrccmp ("em", option) == 0) {
        c = &fb->em;
    } else if (WpeStrccmp ("hh", option) == 0) {
        c = &fb->hh;
    } else if (WpeStrccmp ("hb", option) == 0) {
        c = &fb->hb;
    } else if (WpeStrccmp ("hm", option) == 0) {
        c = &fb->hm;
    } else if (WpeStrccmp ("db", option) == 0) {
        c = &fb->db;
    } else if (WpeStrccmp ("dy", option) == 0) {
        c = &fb->dy;
    } else if (WpeStrccmp ("mr", option) == 0) {
        c = &fb->mr;
    } else if (WpeStrccmp ("ms", option) == 0) {
        c = &fb->ms;
    } else if (WpeStrccmp ("mt", option) == 0) {
        c = &fb->mt;
    } else if (WpeStrccmp ("mz", option) == 0) {
        c = &fb->mz;
    } else if (WpeStrccmp ("df", option) == 0) {
        c = &fb->df;
    } else if (WpeStrccmp ("nr", option) == 0) {
        c = &fb->nr;
    } else if (WpeStrccmp ("ne", option) == 0) {
        c = &fb->ne;
    } else if (WpeStrccmp ("nt", option) == 0) {
        c = &fb->nt;
    } else if (WpeStrccmp ("nsnt", option) == 0) {
        c = &fb->nsnt;
    } else if (WpeStrccmp ("fr", option) == 0) {
        c = &fb->fr;
    } else if (WpeStrccmp ("fa", option) == 0) {
        c = &fb->fa;
    } else if (WpeStrccmp ("ft", option) == 0) {
        c = &fb->ft;
    } else if (WpeStrccmp ("fz", option) == 0) {
        c = &fb->fz;
    } else if (WpeStrccmp ("frft", option) == 0) {
        c = &fb->frft;
    } else if (WpeStrccmp ("fs", option) == 0) {
        c = &fb->fs;
    } else if (WpeStrccmp ("nsft", option) == 0) {
        c = &fb->nsft;
    } else if (WpeStrccmp ("fsm", option) == 0) {
        c = &fb->fsm;
    } else if (WpeStrccmp ("nz", option) == 0) {
        c = &fb->nz;
    } else if (WpeStrccmp ("ns", option) == 0) {
        c = &fb->ns;
    } else if (WpeStrccmp ("nm", option) == 0) {
        c = &fb->nm;
    } else if (WpeStrccmp ("of", option) == 0) {
        c = &fb->of;
    } else if (WpeStrccmp ("ct", option) == 0) {
        c = &fb->ct;
    } else if (WpeStrccmp ("cr", option) == 0) {
        c = &fb->cr;
    } else if (WpeStrccmp ("ck", option) == 0) {
        c = &fb->ck;
    } else if (WpeStrccmp ("cp", option) == 0) {
        c = &fb->cp;
    } else if (WpeStrccmp ("cc", option) == 0) {
        c = &fb->cc;
    } else if (WpeStrccmp ("dc", option) == 0) {
        fb->dc = atoi (value);
    } else if (WpeStrccmp ("ws", option) == 0) {
        fb->ws = atoi (value);
    }
    if (c != NULL) {
        sscanf (value, "%d%d", &c->fg_color, &c->bg_color);
        if (convert) {
            switch (c->fg_color) {
            case 1:
                c->fg_color = 4;
                break;
            case 3:
                c->fg_color = 6;
                break;
            case 4:
                c->fg_color = 1;
                break;
            case 6:
                c->fg_color = 3;
                break;
            case 9:
                c->fg_color = 12;
                break;
            case 11:
                c->fg_color = 14;
                break;
            case 12:
                c->fg_color = 9;
                break;
            case 14:
                c->fg_color = 11;
                break;
            default:
                break;
            }
            switch (c->bg_color) {
            case 1:
                c->bg_color = 4;
                break;
            case 3:
                c->bg_color = 6;
                break;
            case 4:
                c->bg_color = 1;
                break;
            case 6:
                c->bg_color = 3;
                break;
            case 9:
                c->bg_color = 12;
                break;
            case 11:
                c->bg_color = 14;
                break;
            case 12:
                c->bg_color = 9;
                break;
            case 14:
                c->bg_color = 11;
                break;
            default:
                break;
            }
        }
        *c = e_s_u_clr (c->fg_color, c->bg_color);
    }
    return 0;
}

int
WpeWriteColor (we_control_t * control, char *section, FILE * opt_file)
{
    UNUSED (control);
    we_colorset_t *fb = 0;

    if (WpeStrccmp ("Term", section + strlen (OPT_SECTION_COLOR) + 1) == 0) {
        fb = u_fb;
    }
    if (WpeStrccmp ("X11", section + strlen (OPT_SECTION_COLOR) + 1) == 0) {
        fb = x_fb;
    }
    fprintf (opt_file, "er : %d %d\n", fb->er.fg_color, fb->er.bg_color);
    fprintf (opt_file, "es : %d %d\n", fb->es.fg_color, fb->es.bg_color);
    fprintf (opt_file, "et : %d %d\n", fb->et.fg_color, fb->et.bg_color);
    fprintf (opt_file, "ez : %d %d\n", fb->ez.fg_color, fb->ez.bg_color);
    fprintf (opt_file, "ek : %d %d\n", fb->ek.fg_color, fb->ek.bg_color);
    fprintf (opt_file, "em : %d %d\n", fb->em.fg_color, fb->em.bg_color);
    fprintf (opt_file, "hh : %d %d\n", fb->hh.fg_color, fb->hh.bg_color);
    fprintf (opt_file, "hb : %d %d\n", fb->hb.fg_color, fb->hb.bg_color);
    fprintf (opt_file, "hm : %d %d\n", fb->hm.fg_color, fb->hm.bg_color);
    fprintf (opt_file, "db : %d %d\n", fb->db.fg_color, fb->db.bg_color);
    fprintf (opt_file, "dy : %d %d\n", fb->dy.fg_color, fb->dy.bg_color);
    fprintf (opt_file, "mr : %d %d\n", fb->mr.fg_color, fb->mr.bg_color);
    fprintf (opt_file, "ms : %d %d\n", fb->ms.fg_color, fb->ms.bg_color);
    fprintf (opt_file, "mt : %d %d\n", fb->mt.fg_color, fb->mt.bg_color);
    fprintf (opt_file, "mz : %d %d\n", fb->mz.fg_color, fb->mz.bg_color);
    fprintf (opt_file, "df : %d %d\n", fb->df.fg_color, fb->df.bg_color);
    fprintf (opt_file, "nr : %d %d\n", fb->nr.fg_color, fb->nr.bg_color);
    fprintf (opt_file, "ne : %d %d\n", fb->ne.fg_color, fb->ne.bg_color);
    fprintf (opt_file, "nt : %d %d\n", fb->nt.fg_color, fb->nt.bg_color);
    fprintf (opt_file, "nsnt : %d %d\n", fb->nsnt.fg_color, fb->nsnt.bg_color);
    fprintf (opt_file, "fr : %d %d\n", fb->fr.fg_color, fb->fr.bg_color);
    fprintf (opt_file, "fa : %d %d\n", fb->fa.fg_color, fb->fa.bg_color);
    fprintf (opt_file, "ft : %d %d\n", fb->ft.fg_color, fb->ft.bg_color);
    fprintf (opt_file, "fz : %d %d\n", fb->fz.fg_color, fb->fz.bg_color);
    fprintf (opt_file, "frft : %d %d\n", fb->frft.fg_color, fb->frft.bg_color);
    fprintf (opt_file, "fs : %d %d\n", fb->fs.fg_color, fb->fs.bg_color);
    fprintf (opt_file, "nsft : %d %d\n", fb->nsft.fg_color, fb->nsft.bg_color);
    fprintf (opt_file, "fsm : %d %d\n", fb->fsm.fg_color, fb->fsm.bg_color);
    fprintf (opt_file, "nz : %d %d\n", fb->nz.fg_color, fb->nz.bg_color);
    fprintf (opt_file, "ns : %d %d\n", fb->ns.fg_color, fb->ns.bg_color);
    fprintf (opt_file, "nm : %d %d\n", fb->nm.fg_color, fb->nm.bg_color);
    fprintf (opt_file, "of : %d %d\n", fb->of.fg_color, fb->of.bg_color);
    fprintf (opt_file, "ct : %d %d\n", fb->ct.fg_color, fb->ct.bg_color);
    fprintf (opt_file, "cr : %d %d\n", fb->cr.fg_color, fb->cr.bg_color);
    fprintf (opt_file, "ck : %d %d\n", fb->ck.fg_color, fb->ck.bg_color);
    fprintf (opt_file, "cp : %d %d\n", fb->cp.fg_color, fb->cp.bg_color);
    fprintf (opt_file, "cc : %d %d\n", fb->cc.fg_color, fb->cc.bg_color);
    fprintf (opt_file, "dc : %d\n", fb->dc);
    fprintf (opt_file, "ws : %d\n", fb->ws);
    return 0;
}

int
WpeReadProgramming (we_control_t * control, char *section, char *option, char *value)
{
    UNUSED (control);
    UNUSED (section);
    if (WpeStrccmp ("Arguments", option) == 0) {
        e_prog.arguments = WpeStrdup (value);
    } else if (WpeStrccmp ("Project", option) == 0) {
        e_prog.project = WpeStrdup (value);
    } else if (WpeStrccmp ("Exedir", option) == 0) {
        e_prog.exedir = WpeStrdup (value);
    } else if (WpeStrccmp ("IncludePath", option) == 0) {
        e_prog.sys_include = WpeStrdup (value);
    } else if (WpeStrccmp ("Debugger", option) == 0) {
        e_deb_type = atoi (value);
    }
    return 0;
}

int
WpeWriteProgramming (we_control_t * control, char *section, FILE * opt_file)
{
    UNUSED (control);
    UNUSED (section);
    fprintf (opt_file, "Arguments : %s\n", e_prog.arguments);
    fprintf (opt_file, "Project : %s\n", e_prog.project);
    fprintf (opt_file, "Exedir : %s\n", e_prog.exedir);
    fprintf (opt_file, "IncludePath : %s\n", e_prog.sys_include);
    fprintf (opt_file, "Debugger : %d\n", e_deb_type);
    return 0;
}

int
WpeReadLanguage (we_control_t * control, char *section, char *option, char *value)
{
    UNUSED (control);
    int i, j;
    char *strtmp;

    for (i = 0;
            (i < e_prog.num)
            &&
            (WpeStrccmp
             (e_prog.comp[i]->language,
              section + strlen (OPT_SECTION_LANGUAGE) + 1) != 0); i++)
        ;
    if (i == e_prog.num) {
        e_prog.num++;
        e_prog.comp =
            realloc (e_prog.comp, e_prog.num * sizeof (struct e_s_prog *));
        e_prog.comp[i] = malloc (sizeof (struct e_s_prog));
        e_prog.comp[i]->language =
            WpeStrdup (section + strlen (OPT_SECTION_LANGUAGE) + 1);
        e_prog.comp[i]->compiler = WpeStrdup ("");
        e_prog.comp[i]->comp_str = WpeStrdup ("");
        e_prog.comp[i]->libraries = WpeStrdup ("");
        e_prog.comp[i]->exe_name = WpeStrdup ("");
        e_prog.comp[i]->filepostfix =
            (char **) WpeExpArrayCreate (0, sizeof (char *), 1);
        e_prog.comp[i]->intstr = WpeStrdup ("");
        e_prog.comp[i]->key = '\0';
        e_prog.comp[i]->comp_sw = 0;
        e_prog.comp[i]->x = 0;
    }
    if (WpeStrccmp ("Compiler", option) == 0) {
        if (e_prog.comp[i]->compiler) {
            free (e_prog.comp[i]->compiler);
        }
        e_prog.comp[i]->compiler = WpeStrdup (value);
    } else if (WpeStrccmp ("CompilerOptions", option) == 0) {
        if (e_prog.comp[i]->comp_str) {
            free (e_prog.comp[i]->comp_str);
        }
        e_prog.comp[i]->comp_str = WpeStrdup (value);
    } else if (WpeStrccmp ("Libraries", option) == 0) {
        if (e_prog.comp[i]->libraries) {
            free (e_prog.comp[i]->libraries);
        }
        e_prog.comp[i]->libraries = WpeStrdup (value);
    } else if (WpeStrccmp ("Executable", option) == 0) {
        if (e_prog.comp[i]->exe_name) {
            free (e_prog.comp[i]->exe_name);
        }
        e_prog.comp[i]->exe_name = WpeStrdup (value);
    } else if (WpeStrccmp ("FileExtension", option) == 0) {
        for (j = WpeExpArrayGetSize (e_prog.comp[i]->filepostfix); j; j--)
            if (strcmp (e_prog.comp[i]->filepostfix[j - 1], value) == 0) {
                break;
            }
        if (j == 0) {
            strtmp = WpeStrdup (value);
            WpeExpArrayAdd ((void **) &e_prog.comp[i]->filepostfix, &strtmp);
        }
    } else if (WpeStrccmp ("MessageString", option) == 0) {
        if (e_prog.comp[i]->intstr) {
            free (e_prog.comp[i]->intstr);
        }
        e_prog.comp[i]->intstr = WpeValueToString (value);
    } else if (WpeStrccmp ("Key", option) == 0) {
        e_prog.comp[i]->key = value[0];
    } else if (WpeStrccmp ("CompilerSwitch", option) == 0) {
        e_prog.comp[i]->comp_sw = atoi (value);
    } else if (WpeStrccmp ("X", option) == 0) {
        e_prog.comp[i]->x = atoi (value);
    }
    return 0;
}

int
WpeWriteLanguage (we_control_t * control, char *section, FILE * opt_file)
{
    UNUSED (control);
    int i, j;
    char *str_tmp;

    for (i = 0;
            (i < e_prog.num)
            &&
            (WpeStrccmp
             (e_prog.comp[i]->language,
              section + strlen (OPT_SECTION_LANGUAGE) + 1) != 0); i++)
        ;
    if (i < e_prog.num) {
        fprintf (opt_file, "Compiler : %s\n", e_prog.comp[i]->compiler);
        fprintf (opt_file, "CompilerOptions : %s\n", e_prog.comp[i]->comp_str);
        fprintf (opt_file, "Libraries : %s\n", e_prog.comp[i]->libraries);
        fprintf (opt_file, "Executable : %s\n", e_prog.comp[i]->exe_name);
        for (j = WpeExpArrayGetSize (e_prog.comp[i]->filepostfix); j; j--)
            fprintf (opt_file, "FileExtension : %s\n",
                     e_prog.comp[i]->filepostfix[j - 1]);
        str_tmp = WpeStringToValue (e_prog.comp[i]->intstr);
        fprintf (opt_file, "MessageString : %s\n", str_tmp);
        free (str_tmp);
        fprintf (opt_file, "CompilerSwitch : %d\n", e_prog.comp[i]->comp_sw);
        fprintf (opt_file, "Key : %c\n", e_prog.comp[i]->key);
        fprintf (opt_file, "X : %d\n", e_prog.comp[i]->x);
    }
    return 0;
}

/*   save options    */
int
e_save_opt (we_window_t * window)
{
    we_control_t *control = window->edit_control;
    FILE *fp;
    int i;
    char *str_line;

    str_line = malloc ((strlen (control->optfile) + 1) * sizeof (char));
    strcpy (str_line, control->optfile);
    for (i = strlen (str_line); i > 0 && str_line[i] != DIRC; i--);
    str_line[i] = '\0';
    if (access (str_line, F_OK)) {
        mkdir (str_line, 0700);
    }
    free (str_line);
    fp = fopen (control->optfile, "w");
    if (fp == NULL) {
        e_error (e_msg[ERR_OPEN_OPF], ERROR_MSG, window->colorset);
        return (-1);
    }
    str_line = OPT_SECTION_GENERAL;
    fprintf (fp, "[%s]\n", str_line);
    WpeWriteGeneral (control, str_line, fp);
    str_line = malloc (strlen (OPT_SECTION_COLOR) + 10);
    strcpy (str_line, OPT_SECTION_COLOR);
    if (u_fb) {
        strcat (str_line, "/Term");
        fprintf (fp, "[%s]\n", str_line);
        WpeWriteColor (control, str_line, fp);
    }
    if (x_fb) {
        strcpy (str_line + strlen (OPT_SECTION_COLOR), "/X11");
        fprintf (fp, "[%s]\n", str_line);
        WpeWriteColor (control, str_line, fp);
    }
    free (str_line);
    str_line = OPT_SECTION_PROGRAMMING;
    fprintf (fp, "[%s]\n", str_line);
    WpeWriteProgramming (control, str_line, fp);
    str_line = malloc (strlen (OPT_SECTION_LANGUAGE) + 2);
    strcpy (str_line, OPT_SECTION_LANGUAGE);
    strcat (str_line, "/");
    for (i = 0; i < e_prog.num; i++) {
        str_line =
            realloc (str_line,
                     strlen (OPT_SECTION_LANGUAGE) +
                     strlen (e_prog.comp[i]->language) + 2);
        strcpy (str_line + strlen (OPT_SECTION_LANGUAGE) + 1,
                e_prog.comp[i]->language);
        fprintf (fp, "[%s]\n", str_line);
        WpeWriteLanguage (control, str_line, fp);
    }
    free (str_line);
    fclose (fp);
    return 0;
}

int
e_opt_read (we_control_t * control)
{
    FILE *fp;
    char *str_line;
    char *section;
    char *option;
    char *value;
    char *str_tmp;
    int sz;
    int i;

    fp = fopen (control->optfile, "r");
    if (fp == NULL) {
        char *file = e_mkfilename (LIBRARY_DIR, OPTION_FILE);
        fp = fopen (file, "r");
        free (file);
    }
    if (fp == NULL) {
        return (0);
    }
    sz = 256;
    str_line = (char *) malloc (256 * sizeof (char));
    section = NULL;
    while (!feof (fp)) {
        if (sz != 256) {
            sz = 256;
            str_line = (char *) realloc (str_line, sz * sizeof (char));
        }
        str_line[0] = 0;
        char *read_result = fgets (str_line, sz, fp);
        while ((read_result != NULL && !feof (fp)) &&
                ((str_line[0] == 0)
                 || (str_line[strlen (str_line) - 1] != '\n'))) {
            sz += 255;
            str_line = (char *) realloc (str_line, sz * sizeof (char));
            read_result = fgets (str_line + sz - 256, 256, fp);
        }
        i = strlen (str_line);
        if (i && (str_line[i - 1] == '\n')) {
            str_line[i - 1] = 0;
        }
        for (option = str_line; isspace (*option); option++)
            ;
        if ((*option) && (*option != '#')) {
            if (*option == '[') {
                if (section) {
                    free (section);
                }
                for (value = option + 1; (*value) && (*value != ']'); value++)
                    ;
                if (*value != ']') {
                    free (str_line);
                    return ERR_READ_OPF;
                }
                *value = 0;
                section = WpeStrdup (option + 1);
            } else {
                value = strchr (option, ':');
                if ((value == NULL) || (value == option)) {
                    free (str_line);
                    return ERR_READ_OPF;
                }
                for (str_tmp = value - 1; isspace (*str_tmp); str_tmp--)
                    ;
                for (value++; isspace (*value); value++)
                    ;
                str_tmp++;
                *str_tmp = 0;
                for (i = 0;
                        (i < OPTION_SECTIONS) &&
                        (strncmp
                         (WpeSectionRead[i].section, section,
                          strlen (WpeSectionRead[i].section)) != 0); i++)
                    ;
                if (i < OPTION_SECTIONS) {
                    (*WpeSectionRead[i].function) (control, section, option, value);
                } else {
                    return ERR_READ_OPF;
                }
            }
        }
    }
    fclose (fp);
    return 0;
}

/*  window for entering a text line */
int
e_add_arguments (char *str, char *head, we_window_t * window, int n, int sw,
                 struct dirfile **df)
{
    int ret;
    char *tmp = malloc ((strlen (head) + 2) * sizeof (char));
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o || !tmp) {
        return (-1);
    }
    o->xa = 20;
    o->ya = 4;
    o->xe = 57;
    o->ye = 11;
    o->bgsw = 0;
    o->name = head;
    o->crsw = AltO;
    sprintf (tmp, "%s:", head);
    e_add_wrstr (4, 2, 4, 3, 30, 128, n, sw, tmp, str, df, o);
    e_add_bttstr (7, 5, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (24, 5, -1, WPE_ESC, "Cancel", NULL, o);
    free (tmp);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        strcpy (str, o->wstr[0]->txt);
    }
    freeostr (o);
    return (ret == WPE_ESC ? 0 : 1);
}

W_O_TXTSTR **
e_add_txtstr (int x, int y, const char *txt, W_OPTSTR * o)
{
    if (o->tn == 0) {
        o->tstr = malloc (1);
    }
    (o->tn)++;
    if (!(o->tstr = realloc (o->tstr, o->tn * sizeof (W_O_TXTSTR *)))) {
        return (NULL);
    }
    if (!(o->tstr[o->tn - 1] = malloc (sizeof (W_O_TXTSTR)))) {
        return (NULL);
    }
    if (!(o->tstr[o->tn - 1]->txt = malloc ((strlen (txt) + 1) * sizeof (char)))) {
        return (NULL);
    }
    o->tstr[o->tn - 1]->x = x;
    o->tstr[o->tn - 1]->y = y;
    strcpy (o->tstr[o->tn - 1]->txt, txt);
    return (o->tstr);
}

W_O_WRSTR **
e_add_wrstr (int xt, int yt, int xw, int yw, int nw, int wmx,
             int nc, int sw, char *header, char *txt, struct dirfile ** df,
             W_OPTSTR * o)
{
    if (o->wn == 0) {
        o->wstr = malloc (1);
    }
    (o->wn)++;
    if (!(o->wstr = realloc (o->wstr, o->wn * sizeof (W_O_WRSTR *)))) {
        return (NULL);
    }
    if (!(o->wstr[o->wn - 1] = malloc (sizeof (W_O_WRSTR)))) {
        return (NULL);
    }
    if (!(o->wstr[o->wn - 1]->txt = malloc ((wmx + 1) * sizeof (char)))) {
        return (NULL);
    }
    if (!
            (o->wstr[o->wn - 1]->header =
                 malloc ((strlen (header) + 1) * sizeof (char)))) {
        return (NULL);
    }
    o->wstr[o->wn - 1]->xt = xt;
    o->wstr[o->wn - 1]->yt = yt;
    o->wstr[o->wn - 1]->xw = xw;
    o->wstr[o->wn - 1]->yw = yw;
    o->wstr[o->wn - 1]->nw = nw;
    o->wstr[o->wn - 1]->wmx = wmx;
    o->wstr[o->wn - 1]->nc = nc;
    o->wstr[o->wn - 1]->sw = sw;
    o->wstr[o->wn - 1]->df = df;
    strcpy (o->wstr[o->wn - 1]->header, header);
    strcpy (o->wstr[o->wn - 1]->txt, txt);
    return (o->wstr);
}

W_O_NUMSTR **
e_add_numstr (int xt, int yt, int xw, int yw, int nw, int wmx,
              int nc, int sw, char *header, int num, W_OPTSTR * o)
{
    if (o->nn == 0) {
        o->nstr = malloc (1);
    }
    (o->nn)++;
    if (!(o->nstr = realloc (o->nstr, o->nn * sizeof (W_O_NUMSTR *)))) {
        return (NULL);
    }
    if (!(o->nstr[o->nn - 1] = malloc (sizeof (W_O_NUMSTR)))) {
        return (NULL);
    }
    if (!
            (o->nstr[o->nn - 1]->header =
                 malloc ((strlen (header) + 1) * sizeof (char)))) {
        return (NULL);
    }
    o->nstr[o->nn - 1]->xt = xt;
    o->nstr[o->nn - 1]->yt = yt;
    o->nstr[o->nn - 1]->xw = xw;
    o->nstr[o->nn - 1]->yw = yw;
    o->nstr[o->nn - 1]->nw = nw;
    o->nstr[o->nn - 1]->wmx = wmx;
    o->nstr[o->nn - 1]->nc = nc;
    o->nstr[o->nn - 1]->sw = sw;
    o->nstr[o->nn - 1]->num = num;
    strcpy (o->nstr[o->nn - 1]->header, header);
    return (o->nstr);
}

W_O_SSWSTR **
e_add_sswstr (int x, int y, int nc, int sw, int num,
              const char *header, W_OPTSTR * o)
{
    if (o->sn == 0) {
        o->sstr = malloc (1);
    }
    (o->sn)++;
    if (!(o->sstr = realloc (o->sstr, o->sn * sizeof (W_O_SSWSTR *)))) {
        return (NULL);
    }
    if (!(o->sstr[o->sn - 1] = malloc (sizeof (W_O_SSWSTR)))) {
        return (NULL);
    }
    if (!(o->sstr[o->sn - 1]->header =
                malloc ((strlen (header) + 1) * sizeof (char)))) {
        return (NULL);
    }
    o->sstr[o->sn - 1]->x = x;
    o->sstr[o->sn - 1]->y = y;
    o->sstr[o->sn - 1]->nc = nc;
    o->sstr[o->sn - 1]->sw = sw;
    o->sstr[o->sn - 1]->num = num;
    strcpy (o->sstr[o->sn - 1]->header, header);
    return (o->sstr);
}

W_O_SPSWSTR **
e_add_spswstr (int n, int x, int y, int nc, int sw,
               char *header, W_OPTSTR * o)
{
    if (n >= o->pn) {
        return (NULL);
    }
    if (n < 0) {
        n = 0;
    }
    if (o->pstr[n]->np == 0) {
        o->pstr[n]->ps = malloc (1);
    }
    (o->pstr[n]->np)++;
    if (!(o->pstr[n]->ps =
                realloc (o->pstr[n]->ps, o->pstr[n]->np * sizeof (W_O_SPSWSTR *)))) {
        return (NULL);
    }
    if (!(o->pstr[n]->ps[o->pstr[n]->np - 1] = malloc (sizeof (W_O_SPSWSTR)))) {
        return (NULL);
    }
    if (!(o->pstr[n]->ps[o->pstr[n]->np - 1]->header =
                malloc ((strlen (header) + 1) * sizeof (char)))) {
        return (NULL);
    }
    o->pstr[n]->ps[o->pstr[n]->np - 1]->x = x;
    o->pstr[n]->ps[o->pstr[n]->np - 1]->y = y;
    o->pstr[n]->ps[o->pstr[n]->np - 1]->nc = nc;
    o->pstr[n]->ps[o->pstr[n]->np - 1]->sw = sw;
    strcpy (o->pstr[n]->ps[o->pstr[n]->np - 1]->header, header);
    return (o->pstr[n]->ps);
}

W_O_PSWSTR **
e_add_pswstr (int n, int x, int y, int nc, int sw, int num,
              char *header, W_OPTSTR * o)
{
    if (o->pn == 0) {
        o->pstr = malloc (1);
    }
    if (n >= o->pn) {
        n = o->pn;
        (o->pn)++;
        if (!(o->pstr = realloc (o->pstr, o->pn * sizeof (W_O_PSWSTR *)))) {
            return (NULL);
        }
        if (!(o->pstr[o->pn - 1] = malloc (sizeof (W_O_PSWSTR)))) {
            return (NULL);
        }
        o->pstr[o->pn - 1]->np = 0;
    }
    if (!e_add_spswstr (n, x, y, nc, sw, header, o)) {
        return (NULL);
    }
    o->pstr[o->pn - 1]->num = num;
    return (o->pstr);
}

W_O_BTTSTR **
e_add_bttstr (int x, int y, int nc, int sw, char *header,
              int (*fkt) (we_window_t * window), W_OPTSTR * o)
{
    if (o->bn == 0) {
        o->bstr = malloc (1);
    }
    (o->bn)++;
    if (!(o->bstr = realloc (o->bstr, o->bn * sizeof (W_O_BTTSTR *)))) {
        return (NULL);
    }
    if (!(o->bstr[o->bn - 1] = malloc (sizeof (W_O_BTTSTR)))) {
        return (NULL);
    }
    if (!
            (o->bstr[o->bn - 1]->header =
                 malloc ((strlen (header) + 1) * sizeof (char)))) {
        return (NULL);
    }
    o->bstr[o->bn - 1]->x = x;
    o->bstr[o->bn - 1]->y = y;
    o->bstr[o->bn - 1]->nc = nc;
    o->bstr[o->bn - 1]->sw = sw;
    o->bstr[o->bn - 1]->fkt = fkt;
    strcpy (o->bstr[o->bn - 1]->header, header);
    return (o->bstr);
}

int
freeostr (W_OPTSTR * o)
{
    int i, j;

    if (!o) {
        return (0);
    }
    for (i = 0; i < o->tn; i++) {
        free (o->tstr[i]->txt);
        free (o->tstr[i]);
    }
    if (o->tn) {
        free (o->tstr);
    }
    for (i = 0; i < o->wn; i++) {
        free (o->wstr[i]->txt);
        free (o->wstr[i]->header);
        free (o->wstr[i]);
    }
    if (o->wn) {
        free (o->wstr);
    }
    for (i = 0; i < o->nn; i++) {
        free (o->nstr[i]->header);
        free (o->nstr[i]);
    }
    if (o->nn) {
        free (o->nstr);
    }
    for (i = 0; i < o->pn; i++) {
        for (j = 0; j < o->pstr[i]->np; j++) {
            free (o->pstr[i]->ps[j]->header);
            free (o->pstr[i]->ps[j]);
        }
        if (o->pstr[i]->np) {
            free (o->pstr[i]->ps);
        }
        free (o->pstr[i]);
    }
    if (o->pn) {
        free (o->pstr);
    }
    for (i = 0; i < o->bn; i++) {
        free (o->bstr[i]->header);
        free (o->bstr[i]);
    }
    if (o->bn) {
        free (o->bstr);
    }
    free (o);
    return (0);
}

W_OPTSTR *
e_init_opt_kst (we_window_t * window)
{
    W_OPTSTR *o = malloc (sizeof (W_OPTSTR));
    if (!o) {
        return (NULL);
    }
    o->frt = window->colorset->nr.fg_bg_color;
    o->frs = window->colorset->ne.fg_bg_color;
    o->ftt = window->colorset->nt.fg_bg_color;
    o->fts = window->colorset->nsnt.fg_bg_color;
    o->fst = window->colorset->fs.fg_bg_color;
    o->fss = window->colorset->nsft.fg_bg_color;
    o->fsa = window->colorset->fsm.fg_bg_color;
    o->fwt = window->colorset->fr.fg_bg_color;
    o->fws = window->colorset->fa.fg_bg_color;
    o->fbt = window->colorset->nz.fg_bg_color;
    o->fbs = window->colorset->ns.fg_bg_color;
    o->fbz = window->colorset->nm.fg_bg_color;
    o->tn = o->sn = o->pn = o->bn = o->wn = o->nn = 0;
    o->window = window;
    o->view = NULL;
    return (o);
}

int
e_opt_move (W_OPTSTR * o)
{
    int xa = o->xa, ya = o->ya, xe = o->xe, ye = o->ye;
    int c = 0;
    we_view_t *view;

    e_std_window (o->xa, o->ya, o->xe, o->ye, o->name, 0, o->frt, o->frs);
#ifndef NEWSTYLE
    if (!WpeIsXwin ()) {
        view = e_open_view (o->xa, o->ya, o->xe, o->ye, 0, 2);
    } else {
        view = e_open_view (o->xa, o->ya, o->xe - 2, o->ye - 1, 0, 2);
        e_close_view (view, 2);
    }
#else
    view = e_open_view (o->xa, o->ya, o->xe, o->ye, 0, 2);
#endif
    while ((c = e_u_getch ()) != WPE_ESC && c != WPE_CR) {
        switch (c) {
        case CLE:
            if (xa > 0) {
                xa--;
                xe--;
            }
            break;
        case CRI:
            if (xe < max_screen_cols() - 1) {
                xa++;
                xe++;
            }
            break;
        case CUP:
            if (ya > 1) {
                ya--;
                ye--;
            }
            break;
        case CDO:
            if (ye < max_screen_lines() - 2) {
                ya++;
                ye++;
            }
            break;
        }
        if (xa != o->xa || ya != o->ya || xe != o->xe || ye != o->ye) {
            o->xa = xa;
            o->ya = ya;
            o->xe = xe;
            o->ye = ye;
            o->view =
                e_change_pic (o->xa, o->ya, o->xe, o->ye, o->view, 1, o->frt);
            if (o->view == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, o->window->colorset);
            }
            view->a.x = o->xa;
            view->a.y = o->ya;
            view->e.x = o->xe;
            view->e.y = o->ye;
            e_close_view (view, 2);
        }
    }
    view->a.x = o->xa;
    view->a.y = o->ya;
    view->e.x = o->xe;
    view->e.y = o->ye;
    e_close_view (view, 1);
    e_std_window (o->xa, o->ya, o->xe, o->ye, o->name, 1, o->frt, o->frs);
    return (c);
}

int
e_get_sw_cmp (int xin, int yin, int x, int y, int xmin, int ymin, int c)
{
    return
        ((c == 0 && yin == y && (xin - 1 <= x && xin + xmin >= x)) ||
         ((c == CDO || c == BDO || c == WPE_TAB) && yin > y && (yin < ymin ||
                 (yin == ymin
                  && xin <= x
                  && xin > xmin)))
         || ((c == CUP || c == BUP || c == WPE_BTAB) && yin < y
             && (yin > ymin || (yin == ymin && xin <= x && xin > xmin)))
         || ((c == CLE || c == CCLE) && yin == y && xin < x && xin > xmin)
         || ((c == CRI || c == CCRI) && yin == y && xin > x && xin < xmin));
}

int
e_get_opt_sw (int c, int x, int y, W_OPTSTR * o)
{
    int i, j, xmin, ymin, ret = 0;
    if (c != 0 && c != CUP && c != CDO && c != CLE && c != CRI && c != BUP &&
            c != BDO && c != CCLE && c != CCRI && c != WPE_TAB && c != WPE_BTAB) {
        return (c);
    }
    xmin = (c == CRI || c == CCRI) ? o->xe : o->xa;
    ymin = (c == CUP || c == BUP || c == WPE_BTAB) ? o->ya : o->ye;
    x -= o->xa;
    xmin -= o->xa;
    y -= o->ya;
    ymin -= o->ya;
    for (i = 0; i < o->wn; i++) {
        if (e_get_sw_cmp (o->wstr[i]->xw, o->wstr[i]->yw, x, y,
                          c ? xmin : o->wstr[i]->nw, ymin, c)) {
            xmin = o->wstr[i]->xw;
            ymin = o->wstr[i]->yw;
            ret = o->wstr[i]->sw;
        }
    }
    for (i = 0; i < o->nn; i++) {
        if (e_get_sw_cmp (o->nstr[i]->xw, o->nstr[i]->yw, x, y,
                          c ? xmin : o->nstr[i]->nw, ymin, c)) {
            xmin = o->nstr[i]->xw;
            ymin = o->nstr[i]->yw;
            ret = o->nstr[i]->sw;
        }
    }
    for (i = 0; i < o->sn; i++) {
        if (e_get_sw_cmp (o->sstr[i]->x, o->sstr[i]->y, x, y,
                          c ? xmin : 2, ymin, c)) {
            xmin = o->sstr[i]->x;
            ymin = o->sstr[i]->y;
            ret = o->sstr[i]->sw;
        }
    }
    for (i = 0; i < o->pn; i++)
        for (j = 0; j < o->pstr[i]->np; j++) {
            if (e_get_sw_cmp (o->pstr[i]->ps[j]->x, o->pstr[i]->ps[j]->y, x, y,
                              c ? xmin : 2, ymin, c)) {
                xmin = o->pstr[i]->ps[j]->x;
                ymin = o->pstr[i]->ps[j]->y;
                ret = o->pstr[i]->ps[j]->sw;
            }
        }
    for (i = 0; i < o->bn; i++) {
        if (e_get_sw_cmp (o->bstr[i]->x, o->bstr[i]->y, x, y,
                          c ? xmin : (int) strlen (o->bstr[i]->header), ymin,
                          c)) {
            xmin = o->bstr[i]->x;
            ymin = o->bstr[i]->y;
            ret = o->bstr[i]->sw;
        }
    }
    return (!ret ? c : ret);
}

int
e_opt_kst (W_OPTSTR * o)
{
    int ret = 0, csv, sw = 1, i, j, num, cold, c = o->bgsw;
    char *tmp;
    fk_u_cursor (0);
    o->view =
        e_std_view (o->xa, o->ya, o->xe, o->ye, o->name, 1, o->frt, o->ftt,
                    o->frs);
    if (o->view == NULL) {
        e_error (e_msg[ERR_LOWMEM], ERROR_MSG, o->window->colorset);
        return (-1);
    }
    if (!c) {
        c = e_get_opt_sw (CDO, 0, 0, o);
    }
    for (i = 0; i < o->tn; i++)
        e_pr_str (o->xa + o->tstr[i]->x, o->ya + o->tstr[i]->y, o->tstr[i]->txt,
                  o->ftt, -1, 0, 0, 0);
    for (i = 0; i < o->wn; i++) {
        e_pr_str (o->xa + o->wstr[i]->xt, o->ya + o->wstr[i]->yt,
                  o->wstr[i]->header, o->ftt, o->wstr[i]->nc, 1, o->fts, 0);
        if (!o->wstr[i]->df)
            e_schr_nchar (o->wstr[i]->txt, o->xa + o->wstr[i]->xw,
                          o->ya + o->wstr[i]->yw, 0, o->wstr[i]->nw, o->fwt);
        else
            e_schr_nchar_wsv (o->wstr[i]->txt, o->xa + o->wstr[i]->xw,
                              o->ya + o->wstr[i]->yw, 0, o->wstr[i]->nw, o->fwt,
                              o->fws);
    }
    for (i = 0; i < o->nn; i++) {
        e_pr_str (o->xa + o->nstr[i]->xt, o->ya + o->nstr[i]->yt,
                  o->nstr[i]->header, o->ftt, o->nstr[i]->nc, 1, o->fts, 0);
        e_schr_nzif (o->nstr[i]->num, o->xa + o->nstr[i]->xw,
                     o->ya + o->nstr[i]->yw, o->nstr[i]->nw, o->fwt);
    }
    for (i = 0; i < o->sn; i++) {
        e_pr_str (o->xa + o->sstr[i]->x + 4, o->ya + o->sstr[i]->y,
                  o->sstr[i]->header, o->fst, o->sstr[i]->nc, 1, o->fss, 0);
#ifdef NEWSTYLE
        if (WpeIsXwin ())
            e_pr_str (o->xa + o->sstr[i]->x, o->ya + o->sstr[i]->y, "   ",
                      o->fst, -1, 1, 0, 0);
        else
#endif
            e_pr_str (o->xa + o->sstr[i]->x, o->ya + o->sstr[i]->y, "[ ]", o->fst,
                      -1, 1, 0, 0);
    }
    for (i = 0; i < o->pn; i++) {
        for (j = 0; j < o->pstr[i]->np; j++) {
            e_pr_str (o->xa + o->pstr[i]->ps[j]->x,
                      o->ya + o->pstr[i]->ps[j]->y, "[ ] ", o->fst, -1, 1, 0,
                      0);
            e_pr_str (o->xa + o->pstr[i]->ps[j]->x + 4,
                      o->ya + o->pstr[i]->ps[j]->y, o->pstr[i]->ps[j]->header,
                      o->fst, o->pstr[i]->ps[j]->nc, 1, o->fss, 0);
#ifdef NEWSTYLE
            if (WpeIsXwin ())
                e_pr_str (o->xa + o->pstr[i]->ps[j]->x,
                          o->ya + o->pstr[i]->ps[j]->y, "   ", o->fst, -1, 1, 0,
                          0);
            else
#endif
                e_pr_str (o->xa + o->pstr[i]->ps[j]->x,
                          o->ya + o->pstr[i]->ps[j]->y, "[ ]", o->fst, -1, 1, 0,
                          0);
        }
    }
    for (i = 0; i < o->bn; i++) {
        e_pr_str (o->xa + o->bstr[i]->x, o->ya + o->bstr[i]->y,
                  o->bstr[i]->header, o->fbt, o->bstr[i]->nc, -1, o->fbs,
                  o->ftt);
    }
    cold = c;
    while (c != WPE_ESC || sw) {
#ifdef NEWSTYLE
        if (WpeIsXwin ()) {
            for (i = 0; i < o->sn; i++) {
                if (o->sstr[i]->num)
                    e_make_xrect_abs (o->xa + o->sstr[i]->x,
                                      o->ya + o->sstr[i]->y,
                                      o->xa + o->sstr[i]->x + 2,
                                      o->ya + o->sstr[i]->y, 1);
                else
                    e_make_xrect_abs (o->xa + o->sstr[i]->x,
                                      o->ya + o->sstr[i]->y,
                                      o->xa + o->sstr[i]->x + 2,
                                      o->ya + o->sstr[i]->y, 0);
            }
            for (i = 0; i < o->pn; i++)
                for (j = 0; j < o->pstr[i]->np; j++) {
                    if (o->pstr[i]->num == j)
                        e_make_xrect_abs (o->xa + o->pstr[i]->ps[j]->x,
                                          o->ya + o->pstr[i]->ps[j]->y,
                                          o->xa + o->pstr[i]->ps[j]->x + 2,
                                          o->ya + o->pstr[i]->ps[j]->y, 1);
                    else
                        e_make_xrect_abs (o->xa + o->pstr[i]->ps[j]->x,
                                          o->ya + o->pstr[i]->ps[j]->y,
                                          o->xa + o->pstr[i]->ps[j]->x + 2,
                                          o->ya + o->pstr[i]->ps[j]->y, 0);
                }
        } else
#endif
        {
            for (i = 0; i < o->sn; i++) {
                if (o->sstr[i]->num)
                    e_pr_char (o->xa + o->sstr[i]->x + 1, o->ya + o->sstr[i]->y,
                               'X', o->fst);
                else
                    e_pr_char (o->xa + o->sstr[i]->x + 1, o->ya + o->sstr[i]->y,
                               ' ', o->fst);
            }
            for (i = 0; i < o->pn; i++)
                for (j = 0; j < o->pstr[i]->np; j++) {
                    if (o->pstr[i]->num == j)
                        e_pr_char (o->xa + o->pstr[i]->ps[j]->x + 1,
                                   o->ya + o->pstr[i]->ps[j]->y, SWSYM, o->fst);
                    else
                        e_pr_char (o->xa + o->pstr[i]->ps[j]->x + 1,
                                   o->ya + o->pstr[i]->ps[j]->y, ' ', o->fst);
                }
        }
        if ((c == AF2 && !(o->window->edit_control->edopt & ED_CUA_STYLE))
                || (o->window->edit_control->edopt & ED_CUA_STYLE && c == CtrlL)) {
            e_opt_move (o);
            c = cold;
            continue;
        }
        for (i = 0; i < o->wn; i++)
            if (o->wstr[i]->sw == c || (o->wstr[i]->nc >= 0 &&
                                        toupper (c) ==
                                        o->wstr[i]->header[o->wstr[i]->nc])) {
                cold = c;
                tmp = malloc ((o->wstr[i]->wmx + 1) * sizeof (char));
                strcpy (tmp, o->wstr[i]->txt);
#if MOUSE
                if (!o->wstr[i]->df && (c = e_schreib_leiste (tmp,
                                            o->xa +
                                            o->wstr[i]->xw,
                                            o->ya +
                                            o->wstr[i]->yw,
                                            o->wstr[i]->nw,
                                            o->wstr[i]->wmx,
                                            o->fwt,
                                            o->fws)) < 0) {
                    c = e_opt_mouse (o);
                } else if (o->wstr[i]->df && (c = e_schr_lst_wsv (tmp,
                                                  o->xa +
                                                  o->wstr[i]->xw,
                                                  o->ya +
                                                  o->wstr[i]->yw,
                                                  o->wstr[i]->nw,
                                                  o->wstr[i]->wmx,
                                                  o->fwt, o->fws,
                                                  o->wstr[i]->df,
                                                  o->window)) < 0) {
                    c = e_opt_mouse (o);
                }
#else
                if (!o->wstr[i]->df)
                    c = e_schreib_leiste (tmp,
                                          o->xa + o->wstr[i]->xw,
                                          o->ya + o->wstr[i]->yw, o->wstr[i]->nw,
                                          o->wstr[i]->wmx, o->fwt, o->fws);
                else if (o->wstr[i]->df)
                    c = e_schr_lst_wsv (tmp,
                                        o->xa + o->wstr[i]->xw,
                                        o->ya + o->wstr[i]->yw, o->wstr[i]->nw,
                                        o->wstr[i]->wmx, o->fwt, o->fws,
                                        o->wstr[i]->df, o->window);
#endif
                if (c != WPE_ESC) {
                    strcpy (o->wstr[i]->txt, tmp);
                }
                csv = c;
                if (!o->wstr[i]->df)
                    e_schr_nchar (o->wstr[i]->txt, o->xa + o->wstr[i]->xw,
                                  o->ya + o->wstr[i]->yw, 0, o->wstr[i]->nw,
                                  o->fwt);
                else
                    e_schr_nchar_wsv (o->wstr[i]->txt, o->xa + o->wstr[i]->xw,
                                      o->ya + o->wstr[i]->yw, 0, o->wstr[i]->nw,
                                      o->fwt, o->fws);
                if ((c =
                            e_get_opt_sw (c, o->xa + o->wstr[i]->xw,
                                          o->ya + o->wstr[i]->yw, o)) != csv) {
                    sw = 1;
                } else {
                    sw = 0;
                }
                if (c == WPE_CR) {
                    c = o->crsw;
                } else if (c == WPE_ESC) {
                    ret = WPE_ESC;
                }
                free (tmp);
                fk_u_cursor (0);
                break;
            }
        if (i < o->wn) {
            continue;
        }
        for (i = 0; i < o->nn; i++)
            if (o->nstr[i]->sw == c || (o->nstr[i]->nc >= 0 &&
                                        toupper (c) ==
                                        o->nstr[i]->header[o->nstr[i]->nc])) {
                cold = c;
                num = o->nstr[i]->num;
#if MOUSE
                if ((c = e_schreib_zif (&num, o->xa + o->nstr[i]->xw,
                                        o->ya + o->nstr[i]->yw, o->nstr[i]->nw,
                                        o->fwt, o->fws)) < 0) {
                    c = e_opt_mouse (o);
                }
#else
                c =
                    e_schreib_zif (&num, o->xa + o->nstr[i]->xw,
                                   o->ya + o->nstr[i]->yw, o->nstr[i]->nw, o->fwt,
                                   o->fws);
#endif
                if (c != WPE_ESC) {
                    o->nstr[i]->num = num;
                }
                csv = c;
                if ((c = e_get_opt_sw (c, o->xa + o->nstr[i]->xw,
                                       o->ya + o->nstr[i]->yw, o)) != csv) {
                    sw = 1;
                } else {
                    sw = 0;
                }
                if (c != cold)
                    e_schr_nzif (o->nstr[i]->num, o->xa + o->nstr[i]->xw,
                                 o->ya + o->nstr[i]->yw, o->nstr[i]->nw, o->fwt);
                if (c == WPE_CR) {
                    c = o->crsw;
                } else if (c == WPE_ESC) {
                    ret = WPE_ESC;
                }
                fk_u_cursor (0);
                break;
            }
        if (i < o->nn) {
            continue;
        }
        for (i = 0; i < o->sn; i++)
            if (o->sstr[i]->sw == c || (o->sstr[i]->nc >= 0 &&
                                        toupper (c) ==
                                        o->sstr[i]->header[o->sstr[i]->nc])) {
                if (!sw) {
                    o->sstr[i]->num = !o->sstr[i]->num;
                    sw = 1;
                    c = cold;
                    break;
                }
                cold = c;
                e_pr_str (o->xa + o->sstr[i]->x + 4, o->ya + o->sstr[i]->y,
                          o->sstr[i]->header, o->fsa, o->sstr[i]->nc, 1, o->fsa,
                          0);
#if MOUSE
                if ((c = e_u_getch ()) < 0) {
                    c = e_opt_mouse (o);
                }
#else
                c = e_u_getch ();
#endif
                if (c == WPE_CR) {
                    sw = 0;
                    c = cold;
                    break;
                } else if (c == WPE_ESC) {
                    sw = 1;
                } else {
                    csv = c;
                    if ((c = e_get_opt_sw (c, o->xa + o->sstr[i]->x,
                                           o->ya + o->sstr[i]->y, o)) != csv) {
                        sw = 1;
                    } else {
                        sw = 0;
                    }
                }
                if (c != cold)
                    e_pr_str (o->xa + o->sstr[i]->x + 4, o->ya + o->sstr[i]->y,
                              o->sstr[i]->header, o->fst, o->sstr[i]->nc, 1, o->fss,
                              0);
                break;
            }
        if (i < o->sn) {
            continue;
        }
        for (i = 0; i < o->pn; i++) {
            for (j = 0; j < o->pstr[i]->np; j++)
                if (o->pstr[i]->ps[j]->sw == c || (o->pstr[i]->ps[j]->nc >= 0 &&
                                                   toupper (c) ==
                                                   o->pstr[i]->ps[j]->header[o->
                                                           pstr
                                                           [i]->
                                                           ps
                                                           [j]->
                                                           nc])) {
                    if (!sw) {
                        o->pstr[i]->num = j;
                        sw = 1;
                        c = cold;
                        break;
                    }
                    cold = c;
                    e_pr_str (o->xa + o->pstr[i]->ps[j]->x + 4,
                              o->ya + o->pstr[i]->ps[j]->y,
                              o->pstr[i]->ps[j]->header, o->fsa,
                              o->pstr[i]->ps[j]->nc, 1, o->fsa, 0);
#if MOUSE
                    if ((c = e_u_getch ()) < 0) {
                        c = e_opt_mouse (o);
                    }
#else
                    c = e_u_getch ();
#endif
                    if (c == WPE_CR) {
                        sw = 0;
                        c = cold;
                        break;
                    } else if (c == WPE_ESC) {
                        sw = 1;
                    }
                    {
                        csv = c;
                        if ((c = e_get_opt_sw (c, o->xa + o->pstr[i]->ps[j]->x,
                                               o->ya + o->pstr[i]->ps[j]->y,
                                               o)) != csv) {
                            sw = 1;
                        } else {
                            sw = 0;
                        }
                    }
                    if (c != cold)
                        e_pr_str (o->xa + o->pstr[i]->ps[j]->x + 4,
                                  o->ya + o->pstr[i]->ps[j]->y,
                                  o->pstr[i]->ps[j]->header, o->fst,
                                  o->pstr[i]->ps[j]->nc, 1, o->fss, 0);
                    break;
                }
            if (j < o->pstr[i]->np) {
                break;
            }
        }
        if (i < o->pn) {
            continue;
        }
        for (i = 0; i < o->bn; i++)
            if (o->bstr[i]->sw == c || (o->bstr[i]->nc >= 0 &&
                                        toupper (c) ==
                                        o->bstr[i]->header[o->bstr[i]->nc])) {
                e_pr_str (o->xa + o->bstr[i]->x, o->ya + o->bstr[i]->y,
                          o->bstr[i]->header, o->fbz, o->bstr[i]->nc, -1, o->fbz,
                          o->ftt);
                if (!sw) {
                    if (o->bstr[i]->fkt != NULL) {
                        if ((ret = o->bstr[i]->fkt (o->window)) > 0) {
                            c = WPE_ESC;
                        } else {
                            c = cold;
                            e_pr_str (o->xa + o->bstr[i]->x,
                                      o->ya + o->bstr[i]->y, o->bstr[i]->header,
                                      o->fbt, o->bstr[i]->nc, -1, o->fbs, o->ftt);
                        }
                    } else {
                        ret = o->bstr[i]->sw;
                        c = WPE_ESC;
                    }
                    break;
                }
                cold = c;
#if MOUSE
                if ((c = e_u_getch ()) < 0) {
                    c = e_opt_mouse (o);
                }
#else
                c = e_u_getch ();
#endif
                if (c == WPE_CR) {
                    ret = c = o->bstr[i]->sw;
                    sw = 0;
                    break;
                } else if (c == WPE_ESC) {
                    sw = 0;
                    ret = WPE_ESC;
                    break;
                }
                csv = c;
                if ((c = e_get_opt_sw (c, o->xa + o->bstr[i]->x,
                                       o->ya + o->bstr[i]->y, o)) != csv) {
                    sw = 1;
                } else {
                    sw = 0;
                }
                if (c != cold)
                    e_pr_str (o->xa + o->bstr[i]->x, o->ya + o->bstr[i]->y,
                              o->bstr[i]->header, o->fbt, o->bstr[i]->nc, -1,
                              o->fbs, o->ftt);
                break;
            }
        if (i < o->bn) {
            continue;
        }

        c = cold;
        sw = 1;
    }
    e_close_view (o->view, 1);
    return (ret);
}

int
e_edt_options (we_window_t * window)
{
    int i, ret, edopt = window->edit_control->edopt;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o) {
        return (-1);
    }
    o->xa = 15;
    o->ya = 3;
    o->xe = 64;
    o->ye = 22;
    o->bgsw = AltO;
    o->name = "Editor-Options";
    o->crsw = AltO;
    e_add_txtstr (3, 2, "Display:", o);
    e_add_txtstr (25, 2, "Autosave:", o);
    e_add_txtstr (25, 6, "Keys:", o);
    e_add_txtstr (25, 10, "Auto-Indent:", o);
    e_add_txtstr (3, 5, "Tile:", o);
    e_add_numstr (3, 8, 19, 8, 3, 100, 0, AltM, "Max. Columns:", window->edit_control->maxcol,
                  o);
    e_add_numstr (3, 9, 20, 9, 2, 100, 0, AltT, "Tabstops:", window->edit_control->tabn, o);
    e_add_numstr (3, 10, 19, 10, 3, 1000, 2, AltX, "MaX. Changes:",
                  window->edit_control->maxchg, o);
    e_add_numstr (3, 11, 20, 11, 2, 100, 0, AltN, "Num. Undo:", window->edit_control->numundo,
                  o);
    e_add_numstr (3, 12, 20, 12, 2, 100, 5, AltI, "Auto Ind. Col.:",
                  window->edit_control->autoindent, o);
    e_add_sswstr (4, 3, 0, AltS, window->edit_control->edopt & ED_SHOW_ENDMARKS ? 1 : 0,
                  "Show Endmark ", o);
    e_add_sswstr (26, 3, 1, AltP, window->edit_control->autosv & 1, "OPtions         ", o);
    e_add_sswstr (26, 4, 1, AltH, window->edit_control->autosv & 2 ? 1 : 0, "CHanges         ",
                  o);
    e_add_sswstr (4, 6, 2, AltD, window->edit_control->edopt & ED_OLD_TILE_METHOD ? 1 : 0,
                  "OlD Style    ", o);
    e_add_pswstr (0, 26, 7, 1, AltL, 0, "OLd-Style       ", o);
    e_add_pswstr (0, 26, 8, 0, AltC, window->edit_control->edopt & ED_CUA_STYLE,
                  "CUA-Style       ", o);
    e_add_pswstr (1, 26, 11, 3, AltY, 0, "OnlY Source-Text", o);
    e_add_pswstr (1, 26, 12, 2, AltW, 0, "AlWays          ", o);
    e_add_pswstr (1, 26, 13, 2, AltV,
                  (window->edit_control->edopt & ED_ALWAYS_AUTO_INDENT ? 1 : window->edit_control->
                   edopt & ED_SOURCE_AUTO_INDENT ? 0 : 2), "NeVer           ",
                  o);
    e_add_wrstr (3, 14, 3, 15, 44, 128, 1, AltR, "PRint Command:",
                 window->edit_control->print_cmd, NULL, o);
    e_add_bttstr (12, 17, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (31, 17, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        window->edit_control->autosv = o->sstr[1]->num + (o->sstr[2]->num << 1);
        window->edit_control->maxcol = o->nstr[0]->num;
        window->edit_control->tabn = o->nstr[1]->num;
        window->edit_control->maxchg = o->nstr[2]->num;
        window->edit_control->numundo = o->nstr[3]->num;
        window->edit_control->autoindent = o->nstr[4]->num;
        window->edit_control->edopt = ((window->edit_control->edopt & ~ED_EDITOR_OPTIONS) + o->pstr[0]->num) +
                                      (o->pstr[1]->num == 0 ? ED_SOURCE_AUTO_INDENT : 0) +
                                      (o->pstr[1]->num == 1 ? ED_ALWAYS_AUTO_INDENT : 0) +
                                      (o->sstr[3]->num ? ED_OLD_TILE_METHOD : 0) +
                                      (o->sstr[0]->num ? ED_SHOW_ENDMARKS : 0);
        if (window->edit_control->print_cmd) {
            free (window->edit_control->print_cmd);
        }
        window->edit_control->print_cmd = WpeStrdup (o->wstr[0]->txt);
        if (edopt != window->edit_control->edopt) {
            e_switch_blst (window->edit_control);
            for (i = 0; i <= window->edit_control->mxedt; i++)
                if ((window->edit_control->edopt & ED_ALWAYS_AUTO_INDENT) ||
                        ((window->edit_control->edopt & ED_SOURCE_AUTO_INDENT) && window->edit_control->window[i]->c_st)) {
                    window->edit_control->window[i]->flg = 1;
                } else {
                    window->edit_control->window[i]->flg = 0;
                }
            e_repaint_desk (window);
        }
    }
    freeostr (o);
    return (0);
}

int
e_read_help_str ()
{
    FILE *fp;
    char str[128];
    int i, len;

    sprintf (str, "%s/help.key", LIBRARY_DIR);
    for (i = 0; i < E_HLP_NUM; i++) {
        e_hlp_str[i] = malloc (sizeof (char));
        *e_hlp_str[i] = '\0';
    }
    if (!(fp = fopen (str, "rb"))) {
        return (-1);
    }
    for (i = 0; i < E_HLP_NUM && fgets (str, 128, fp); i++) {
        len = strlen (str);
        if (str[len - 1] == '\n') {
            str[--len] = '\0';
        }
        e_hlp_str[i] = realloc (e_hlp_str[i], (len + 1) * sizeof (char));
        strcpy (e_hlp_str[i], str);
    }
    fclose (fp);
    return (i == E_HLP_NUM ? 0 : -2);
}
