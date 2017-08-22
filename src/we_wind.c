/** \file we_wind.c                                        */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <ctype.h>
#include <string.h>
#include "keys.h"
#include "messages.h"
#include "options.h"
#include "model.h"
#include "we_control.h"
#include "edit.h"
#include "we_wind.h"
#include "we_progn.h"
#include "we_prog.h"

#ifdef UNIX
#include <unistd.h>
#endif

#define MAXSVSTR 20

#if !defined NEWSTYLE
static void e_pt_col(int x, int y, int c);
#endif
static int e_make_xr_window (int xa, int ya, int xe, int ye, int sw);

extern int num_lines_off_screen_top(we_window_t *window);
extern int num_lines_on_screen(we_window_t *window);
extern int line_num_on_screen_bottom(we_window_t *window);
extern int num_cols_on_screen(we_window_t *window);
extern int num_cols_off_screen_left(we_window_t *window);
extern int num_cols_on_screen_safe(we_window_t *window);
extern int col_num_on_screen_right(we_window_t *window);

void e_pr_char(int x, int y, int c, int color)
{
    *(global_screen + 2*MAXSCOL*y + 2 * x) = c;
    *(global_screen + 2*MAXSCOL*y + 2 * x + 1) = color;
#ifdef NEWSTYLE
    *(extbyte + MAXSCOL * y + x) = 0;
#endif
}

#if !defined NEWSTYLE
static void e_pt_col(int x, int y, int c)
{
    *(global_screen + 2*MAXSCOL*y + 2 * x + 1) = c;
}
#endif

char e_gt_char(int x, int y)
{
    return *(global_screen + 2 * MAXSCOL * y + 2 * x);
}

char e_gt_col(int x, int y)
{
    return *(global_screen + 2 * MAXSCOL * y + 2 * x + 1);
}

char e_gt_byte(int x, int y)
{
    return *(global_screen + 2 * MAXSCOL * y + x);
}

void e_pt_byte(int x, int y, int c)
{
    *(global_screen + 2*MAXSCOL*y + x) = c;
}


/* break string into multiple line to fit into windows

   REM: Caller has to free returned vector!!!
*/
char **
StringToStringArray (char *str, int *maxLen, int minWidth, int *nr_lines_return)
{
    int i, j, k, nr_lines = 0, mxlen = 0, max = 0.8 * MAXSCOL;
    char **s = malloc (sizeof (char *));

    for (k = 0, i = 0; str[i]; i++)
    {
        if ((i - k) == max || str[i] == '\n')
        {
            j = i - 1;
            if (str[i] != '\n')
                for (; j > 0 && !isspace (str[j]); j--)
                    ;
            if (j > k)
                i = j;
            nr_lines++;
            s = realloc (s, nr_lines * sizeof (char *));
            s[nr_lines - 1] = malloc ((i - k + 2) * sizeof (char));
            for (j = k; j <= i; j++)
                s[nr_lines - 1][j - k] = str[j];
            if (isspace (str[j - 1]))
                j--;
            if (mxlen < (j - k))
                mxlen = j - k;
            s[nr_lines - 1][j - k] = '\0';
            k = i + 1;
        }
    }
    nr_lines++;
    s = realloc (s, nr_lines * sizeof (char *));
    s[nr_lines - 1] = malloc ((i - k + 2) * sizeof (char));
    for (j = k; j <= i; j++)
        s[nr_lines - 1][j - k] = str[j];
    if (mxlen < (j - k))
        mxlen = j - k;
    if (mxlen < minWidth)
        mxlen = minWidth;

    *maxLen = mxlen;
    *nr_lines_return = nr_lines;

    return (s);
}

/*
      Print error message        */
int
e_error (char *text, int sw, we_colorset_t * colorset)
{
    we_view_t *view = NULL;
    int len, i, xa, xe, ya = 8, ye = 14;
    char *header = NULL;

    fk_cursor (0);
    WpeMouseChangeShape (WpeErrorShape);
    if ((len = strlen ((char *) text)) < 20)
        len = 20;
    xa = (80 - len) / 2 - 2;
    xe = 82 - (80 - len) / 2;
    if (sw == -1)
        header = "Message";
    else if (sw == 0)
        header = "Error";
    else if (sw == 1)
        header = "Serious Error";
    else if (sw == 2)
        header = "Fatal Error";

    if (sw < 2)
        view = e_std_view (xa, ya, xe, ye, header, 1,
                           colorset->nr.fg_bg_color,
                           colorset->nt.fg_bg_color,
                           colorset->ne.fg_bg_color);
    if (sw == 2 || view == NULL)
    {
        view = e_open_view (xa, ya, xe, ye, 0, 0);
        e_std_window (xa, ya, xe, ye, header, 1, 0, 0);
    }
    if (sw < 2)
    {
        e_pr_str ((xe + xa - e_str_len ((unsigned char *) text)) / 2,
                  ya + 2, text, colorset->nt.fg_bg_color, 0, 0, 0, 0);
        e_pr_str ((xe + xa - 4) / 2, ya + 4, " OK ",
                  colorset->nz.fg_bg_color, 1, -1,
                  colorset->ns.fg_bg_color,
                  colorset->nt.fg_bg_color);
    }
    else
    {
        e_pr_str ((xe + xa - e_str_len ((unsigned char *) text)) / 2,
                  ya + 2, text, 112, 0, 0, 0, 0);
        e_pr_str ((xe + xa - 4) / 2, ya + 4, " OK ", 32, 1, -1, 46, 112);
    }
    do
    {
#if  MOUSE
        if ((i = toupper (e_getch ())) == -1)
            i = e_er_mouse (xa + 3, ya, (xe + xa - 4) / 2, ya + 4);
#else
        i = toupper (e_getch ());
#endif
    }
    while (i != WPE_ESC && i != WPE_CR && i != 'O');
    WpeMouseRestoreShape ();
    if (view != NULL)
        e_close_view (view, 1);
    else
        e_cls (0, ' ');
    fk_cursor (1);
    if (sw == 1)
        e_quit (global_editor_control->window[global_editor_control->mxedt]);
    if (sw > 0)
        WpeExit (sw);
    return (sw);
}

/*   message with selection        */
int
e_message (int sw, char *str, we_window_t * window)
{
    int i, ret, mxlen = 0, nr_lines = 0;
    char **s;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o)
        return (-1);

    s = StringToStringArray (str, &mxlen, 22, &nr_lines);

    o->ye = MAXSLNS - 6;
    o->ya = o->ye - nr_lines - 5;
    o->xa = (MAXSCOL - mxlen - 6) / 2;
    o->xe = o->xa + mxlen + 6;

    o->bgsw = 0;
    o->name = "Message";
    for (i = 0; i < nr_lines; i++)
    {
        e_add_txtstr ((o->xe - o->xa - strlen (s[i])) / 2, 2 + i, s[i], o);
        free (s[i]);
    }
    free (s);
    if (!sw)
    {
        o->crsw = AltO;
        e_add_bttstr ((o->xe - o->xa - 4) / 2, o->ye - o->ya - 2, 0, AltO, "Ok",
                      NULL, o);
    }
    else
    {
        o->crsw = AltY;
        e_add_bttstr (4, o->ye - o->ya - 2, 0, AltY, "Yes", NULL, o);
        e_add_bttstr ((o->xe - o->xa - 2) / 2, o->ye - o->ya - 2, 0, AltN, "No",
                      NULL, o);
        e_add_bttstr (o->xe - o->xa - 9, o->ye - o->ya - 2, -1, WPE_ESC,
                      "Cancel", NULL, o);
    }
    ret = e_opt_kst (o);
    freeostr (o);
    return (ret == WPE_ESC ? WPE_ESC : (ret == AltN ? 'N' : 'Y'));
}

/*         First opening of a window                 */
void
e_firstl (we_window_t * window, int sw)
{
    window->view = NULL;
    window->view = e_ed_kst (window, window->view, sw);
    if (window->view == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
}

/*         Writing of the file type    */
int
e_pr_filetype (we_window_t * window)
{
    int frb = window->colorset->es.fg_bg_color;

    e_pr_char (window->a.x + 2, window->e.y, 'A', frb);
    if (window->ins == 0 || window->ins == 2)
        e_pr_char (window->a.x + 16, window->e.y, 'O', frb);
    else if (window->ins == 8)
        e_pr_char (window->a.x + 16, window->e.y, 'R', frb);
    else
        e_pr_char (window->a.x + 16, window->e.y, 'I', frb);
    if (window->ins > 1)
        e_pr_char (window->a.x + 17, window->e.y, 'S', frb);
    else
        e_pr_char (window->a.x + 17, window->e.y, 'L', frb);
    return (0);
}

/*   open section of screen and save background  */
we_view_t *
e_open_view (int xa, int ya, int xe, int ye, int col, int sw)
{
    we_view_t *view = malloc (sizeof (we_view_t));
    int i, j;

    if (view == NULL)
        return (NULL);
    view->a.x = xa;
    view->a.y = ya;
#ifndef NEWSTYLE
    if (!WpeIsXwin ())
    {
        view->e.x = xe;
        view->e.y = ye;
    }
    else
    {
        view->e.x = xe < MAXSCOL - 2 ? xe + 2 : xe < MAXSCOL - 1 ? xe + 1 : xe;
        view->e.y = ye < MAXSLNS - 2 ? ye + 1 : ye;
    }
#else
    view->e.x = xe;
    view->e.y = ye;
#endif
    if (sw != 0)
    {
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
        view->p =
            malloc ((view->e.x - view->a.x + 1) * 3 * (view->e.y - view->a.y + 1));
#else
        view->p =
            malloc ((view->e.x - view->a.x + 1) * 2 * (view->e.y - view->a.y + 1));
#endif
        if (view->p == NULL)
        {
            free (view);
            return (NULL);
        }
        for (j = view->a.y; j <= view->e.y; ++j)
            for (i = 2 * view->a.x; i <= 2 * view->e.x + 1; ++i)
                *(view->p + (j - view->a.y) * 2 * (view->e.x - view->a.x + 1) +
                  (i - 2 * view->a.x)) = e_gt_byte (i, j);
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
        e_get_pic_xrect (xa, ya, xe, ye, view);
#endif
    }
    else
        view->p = (char *) 0;
    if (sw < 2)
    {
        for (j = ya; j <= ye; ++j)
            for (i = xa; i <= xe; ++i)
                e_pr_char (i, j, ' ', col);
    }
#ifndef NO_XWINDOWS
    if (WpeIsXwin ())
        (*e_u_setlastpic) (view);
#endif
#ifndef NEWSTYLE
    if (WpeIsXwin ())
    {
        if (sw != 0)
        {
            if (xe < MAXSCOL - 1)
                for (i = ya + 1; i <= ye + 1 && i < MAXSLNS - 1; i++)
                    e_pt_col (xe + 1, i, SHDCOL);
            if (xe < MAXSCOL - 2)
                for (i = ya + 1; i <= ye + 1 && i < MAXSLNS - 1; i++)
                    e_pt_col (xe + 2, i, SHDCOL);
            if (ye < MAXSLNS - 2)
                for (i = xa + 2; i <= xe; i++)
                    e_pt_col (i, ye + 1, SHDCOL);
        }
    }
#endif
    return (view);
}

/*   close screen section - refresh background  */
int
e_close_view (we_view_t * view, int sw)
{
    int i, j;
#ifndef NO_XWINDOWS
    if (WpeIsXwin ())
        (*e_u_setlastpic) (NULL);
#endif
    if (view == NULL)
        return (-1);
    if (sw != 0 && view->p != NULL)
    {
        for (j = view->a.y; j <= view->e.y; ++j)
            for (i = 2 * view->a.x; i <= 2 * view->e.x + 1; ++i)
                e_pt_byte (i, j,
                           *(view->p + (j - view->a.y) * 2 * (view->e.x - view->a.x + 1) + (i - 2 * view-> a.  x)));
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
        e_put_pic_xrect (view);
#endif
    }
    else if (sw != 0)
    {
        for (j = view->a.y; j <= view->e.y; ++j)
            for (i = view->a.x; i <= view->e.x; ++i)
                e_pr_char (i, j, ' ', 0);
    }
    if (sw < 2)
    {
        if (view->p != NULL)
            free (view->p);
        free (view);
    }
    e_refresh ();
    return (sw);
}

/*    Frame for edit window   */
void
e_ed_rahmen (we_window_t * window, int sw)
{
    extern char *e_hlp;
    extern int nblst;
    extern WOPT *blst;
    char *header = NULL;

    if (!DTMD_ISTEXT (window->dtmd))
    {
        if (window->datnam[0])
            header = window->datnam;
        if (window->dtmd == DTMD_FILEDROPDOWN)
            e_std_window (window->a.x, window->a.y, window->e.x, window->e.y, header, sw,
                          window->colorset->er.fg_bg_color, window->colorset->es.fg_bg_color);
        else
            e_std_window (window->a.x, window->a.y, window->e.x, window->e.y, header, sw,
                          window->colorset->nr.fg_bg_color, window->colorset->ne.fg_bg_color);
        if (window->winnum < 10 && window->winnum >= 0)
            e_pr_char (window->e.x - 6, window->a.y, '0' + window->winnum, window->colorset->nr.fg_bg_color);
        else if (window->winnum >= 0)
            e_pr_char (window->e.x - 6, window->a.y, 'A' - 10 + window->winnum, window->colorset->nr.fg_bg_color);
        if (sw > 0 && (window->dtmd == DTMD_FILEMANAGER || window->dtmd == DTMD_DATA))
        {
            if (window->zoom == 0)
                e_pr_char (window->e.x - 3, window->a.y, WZN, window->colorset->ne.fg_bg_color);
            else
                e_pr_char (window->e.x - 3, window->a.y, WZY, window->colorset->ne.fg_bg_color);
#ifdef NEWSTYLE
            if (!WpeIsXwin ())
            {
#endif
                e_pr_char (window->e.x - 4, window->a.y, '[', window->colorset->nr.fg_bg_color);
                e_pr_char (window->e.x - 2, window->a.y, ']', window->colorset->nr.fg_bg_color);
#ifdef NEWSTYLE
            }
            else
                e_make_xrect (window->e.x - 4, window->a.y, window->e.x - 2, window->a.y, 0);
#endif
            blst = window->blst;
            nblst = window->nblst;
            e_hlp = window->hlp_str;
            e_pr_uul (window->colorset);
        }
#ifdef NEWSTYLE
        if (WpeIsXwin ())
            e_make_xr_window (window->a.x, window->a.y, window->e.x, window->e.y, sw);
#endif
        return;
    }
    if (window->datnam[0])
    {
        if (strcmp (window->dirct, window->edit_control->dirct) == 0 ||
                window->dtmd == DTMD_HELP || strcmp (window->datnam, BUFFER_NAME) == 0 ||
                num_cols_on_screen(window) < 40)
        {
            header = (char *) malloc (strlen (window->datnam) + 1);
            strcpy (header, window->datnam);
        }
        else
        {
            header =
                (char *) malloc (strlen (window->dirct) + strlen (window->datnam) + 1);
            strcpy (header, window->dirct);
            strcat (header, window->datnam);
        }
    }
    e_std_window (window->a.x, window->a.y, window->e.x, window->e.y, header, sw, window->colorset->er.fg_bg_color,
                  window->colorset->es.fg_bg_color);
    if (header)
        free (header);
    if (sw > 0)
    {
        e_mouse_bar (window->e.x, window->a.y + 1, num_lines_on_screen(window) - 1, 0,
                     window->colorset->em.fg_bg_color);
        e_mouse_bar (window->a.x + 19, window->e.y, num_cols_on_screen(window) - 20, 1,
                     window->colorset->em.fg_bg_color);
        if (window->zoom == 0)
            e_pr_char (window->e.x - 3, window->a.y, WZN, window->colorset->es.fg_bg_color);
        else
            e_pr_char (window->e.x - 3, window->a.y, WZY, window->colorset->es.fg_bg_color);
#ifdef NEWSTYLE
        if (!WpeIsXwin ())
        {
#endif
            e_pr_char (window->e.x - 4, window->a.y, '[', window->colorset->er.fg_bg_color);
            e_pr_char (window->e.x - 2, window->a.y, ']', window->colorset->er.fg_bg_color);
#ifdef NEWSTYLE
        }
        else
            e_make_xrect (window->e.x - 4, window->a.y, window->e.x - 2, window->a.y, 0);
#endif
        e_pr_filetype (window);
        if (WpeIsXwin () && num_lines_on_screen(window) > 8)
        {
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
            e_pr_char (window->a.x, window->a.y + 2, 'F', window->colorset->em.fg_bg_color);
            e_make_xrect (window->a.x, window->a.y + 2, window->a.x, window->a.y + 2, 0);
            e_pr_char (window->a.x, window->a.y + 4, 'R', window->colorset->em.fg_bg_color);
            e_make_xrect (window->a.x, window->a.y + 4, window->a.x, window->a.y + 4, 0);
            e_pr_char (window->a.x, window->a.y + 6, 'A', window->colorset->em.fg_bg_color);
            e_make_xrect (window->a.x, window->a.y + 6, window->a.x, window->a.y + 6, 0);
            if (window->ins != 8)
            {
                e_pr_char (window->a.x, window->a.y + 8, 'S', window->colorset->em.fg_bg_color);
                e_make_xrect (window->a.x, window->a.y + 8, window->a.x, window->a.y + 8, 0);
            }
#else
            e_pr_char (window->a.x, window->a.y + 2, 'F', window->colorset->em.fg_bg_color);
            e_pr_char (window->a.x, window->a.y + 3, MCI, window->colorset->em.fg_bg_color);
            e_pr_char (window->a.x, window->a.y + 4, 'R', window->colorset->em.fg_bg_color);
            e_pr_char (window->a.x, window->a.y + 5, MCI, window->colorset->em.fg_bg_color);
            e_pr_char (window->a.x, window->a.y + 6, 'A', window->colorset->em.fg_bg_color);
            if (window->ins != 8)
            {
                e_pr_char (window->a.x, window->a.y + 7, MCI, window->colorset->em.fg_bg_color);
                e_pr_char (window->a.x, window->a.y + 8, 'S', window->colorset->em.fg_bg_color);
            }
#endif
        }
        e_zlsplt (window);
        blst = window->blst;
        nblst = window->nblst;
        e_hlp = window->hlp_str;
        e_pr_uul (window->colorset);
    }
    if (window->winnum < 10 && window->winnum >= 0)
        e_pr_char (window->e.x - 6, window->a.y, '0' + window->winnum, window->colorset->er.fg_bg_color);
    else if (window->winnum >= 0)
        e_pr_char (window->e.x - 6, window->a.y, 'A' - 10 + window->winnum, window->colorset->er.fg_bg_color);
#ifdef NEWSTYLE
    if (WpeIsXwin ())
        e_make_xr_window (window->a.x, window->a.y, window->e.x, window->e.y, sw);
#endif
}

/*   Output - screen content */
/**
 * \brief Outputs the screen content.
 *
 * @param window we_window_t
 * @param sw int
 * @return int equal to the line number of the bottom of the screen for this window for an text edit window.
 *         returns zero for data, filemanager windows and dropdown windows.
 *
 */
int
e_write_screen (we_window_t * window, int sw)
{
    int j;

    if (window->dtmd == DTMD_FILEMANAGER)
        return (WpeDrawFileManager (window));
    else if (window->dtmd == DTMD_DATA)
        return (e_data_schirm (window));
    else if (window->dtmd == DTMD_FILEDROPDOWN)
        return (e_pr_file_window
                ((FLWND *) window->buffer, 1, sw,
                 window->colorset->er.fg_bg_color,
                 window->colorset->ez.fg_bg_color,
                 window->colorset->frft.fg_bg_color));
    if (num_lines_off_screen_top(window) < 0)
        window->screen->c.y = 0;

#ifdef PROG
    if (window->c_sw)
        for (j = num_lines_off_screen_top(window);
                j < window->buffer->mxlines && j < line_num_on_screen_bottom(window); j++)
            e_pr_c_line (j, window);
    else
#endif
        for (j = num_lines_off_screen_top(window);
                j < window->buffer->mxlines && j < line_num_on_screen_bottom(window); j++)
            e_pr_line (j, window);
    for (; j < line_num_on_screen_bottom(window); j++)
        e_blk ((num_cols_on_screen(window) - 1), window->a.x + 1,
               j - num_lines_off_screen_top(window) + window->a.y + 1, window->colorset->et.fg_bg_color);
    return (j);
}

/*   Move and modify window */
int
e_size_move (we_window_t * window)
{
    int xa = window->a.x, ya = window->a.y, xe = window->e.x, ye = window->e.y;
    int c = 0, xmin = 26, ymin = 3;

    e_ed_rahmen (window, 0);
    if (window->dtmd == DTMD_FILEDROPDOWN)
        xmin = 15;
    else if (!DTMD_ISTEXT (window->dtmd))
        ymin = 9;
    while ((c = e_getch ()) != WPE_ESC && c != WPE_CR)
    {
        switch (c)
        {
        case CLE:
            if (xa > 0)
            {
                xa--;
                xe--;
            }
            break;
        case CRI:
            if (xe < MAXSCOL - 1)
            {
                xa++;
                xe++;
            }
            break;
        case CUP:
            if (ya > 1)
            {
                ya--;
                ye--;
            }
            break;
        case CDO:
            if (ye < MAXSLNS - 2)
            {
                ya++;
                ye++;
            }
            break;
        case SCLE:
        case CCLE:
            if ((xe - xa) > xmin)
                xe--;
            break;
        case SCRI:
        case CCRI:
            if (xe < MAXSCOL - 1)
                xe++;
            break;
        case SCUP:
        case BUP:
            if ((ye - ya) > ymin)
                ye--;
            break;
        case SCDO:
        case BDO:
            if (ye < MAXSLNS - 2)
                ye++;
            break;
        }
        if (xa != window->a.x || ya != window->a.y || xe != window->e.x || ye != window->e.y)
        {
            window->a.x = xa;
            window->a.y = ya;
            window->e.x = xe;
            window->e.y = ye;
            window->view = e_ed_kst (window, window->view, 0);
            if (window->view == NULL)
                e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
            if (window->dtmd == DTMD_FILEDROPDOWN)
            {
                FLWND *fw = (FLWND *) window->buffer;
                fw->xa = window->a.x + 1;
                fw->xe = window->e.x;
                fw->ya = window->a.y + 1;
                fw->ye = window->e.y;
            }
            e_cursor (window, 0);
            e_write_screen (window, 0);
        }
    }
    e_ed_rahmen (window, 1);
    return (c);
}

/**
 * Standard Box
 */
we_view_t *
e_std_view (int xa, int ya, int xe, int ye, char *name, int sw, int fr,
            int ft, int fes)
{
    we_view_t *view = e_open_view (xa, ya, xe, ye, ft, 1);
    if (view == NULL)
        return (NULL);
    e_std_window (xa, ya, xe, ye, name, sw, fr, fes);
    return (view);
}

we_view_t *
e_change_pic (int xa, int ya, int xe, int ye, we_view_t * view, int sw, int frb)
{
    int i, j;
    int box = 2, ax, ay, ex, ey;
    we_view_t *new_view;
    if (sw < 0)
    {
        sw = -sw;
        box = 1;
    }
    if (view == NULL)
    {
        new_view = e_open_view (xa, ya, xe, ye, frb, box);
        if (new_view == NULL)
            return (NULL);
    }
    else if (xa > view->e.x || xe < view->a.x || ya > view->e.y || ye < view->a.y)
    {
        e_close_view (view, box);
        new_view = e_open_view (xa, ya, xe, ye, frb, box);
        if (new_view == NULL)
            return (NULL);
    }
    else
    {
        new_view = malloc (sizeof (we_view_t));
        if (new_view == NULL)
            return (NULL);
        new_view->a.x = xa;
        new_view->a.y = ya;
#ifndef NEWSTYLE
        if (!WpeIsXwin ())
        {
            new_view->e.x = xe;
            new_view->e.y = ye;
        }
        else
        {
            new_view->e.x =
                xe < MAXSCOL - 2 ? xe + 2 : xe < MAXSCOL - 1 ? xe + 1 : xe;
            new_view->e.y = ye < MAXSLNS - 2 ? ye + 1 : ye;
        }
#else
        new_view->e.x = xe;
        new_view->e.y = ye;
#endif
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
        new_view->p = malloc ((new_view->e.x - new_view->a.x + 1) * 3
                              * (new_view->e.y - new_view->a.y + 1));
#else
        new_view->p = malloc ((new_view->e.x - new_view->a.x + 1) * 2
                              * (new_view->e.y - new_view->a.y + 1));
#endif
        if (new_view->p == NULL)
        {
            free (new_view);
            return (NULL);
        }
        ax = view->a.x > new_view->a.x ? view->a.x : new_view->a.x;
        ay = view->a.y > new_view->a.y ? view->a.y : new_view->a.y;
        ex = view->e.x < new_view->e.x ? view->e.x : new_view->e.x;
        ey = view->e.y < new_view->e.y ? view->e.y : new_view->e.y;
        for (j = ay; j <= ey; ++j)
            for (i = 2 * ax; i <= 2 * ex + 1; ++i)
            {
                *(new_view->p +
                  2 * (new_view->e.x - new_view->a.x + 1) * (j - new_view->a.y) + (i -
                          2 *
                          new_view->
                          a.
                          x)) =
                              *(view->p + 2 * (view->e.x - view->a.x + 1) * (j - view->a.y) +
                                (i - 2 * view->a.x));
            }

        for (j = new_view->a.y; j < ay; ++j)
            for (i = 2 * new_view->a.x; i <= 2 * new_view->e.x + 1; ++i)
            {
                *(new_view->p +
                  2 * (new_view->e.x - new_view->a.x + 1) * (j - new_view->a.y) + (i -
                          2 *
                          new_view->
                          a.
                          x)) =
                              e_gt_byte (i, j);
            }

        for (j = new_view->e.y; j > ey; --j)
            for (i = 2 * new_view->a.x; i <= 2 * new_view->e.x + 1; ++i)
            {
                *(new_view->p +
                  2 * (new_view->e.x - new_view->a.x + 1) * (j - new_view->a.y) + (i -
                          2 *
                          new_view->
                          a.
                          x)) =
                              e_gt_byte (i, j);
            }

        for (j = new_view->a.y; j <= new_view->e.y; ++j)
            for (i = 2 * new_view->a.x; i < 2 * ax; ++i)
            {
                *(new_view->p +
                  2 * (new_view->e.x - new_view->a.x + 1) * (j - new_view->a.y) + (i -
                          2 *
                          new_view->
                          a.
                          x)) =
                              e_gt_byte (i, j);
            }

        for (j = new_view->a.y; j <= new_view->e.y; ++j)
            for (i = 2 * (ex + 1); i <= 2 * new_view->e.x + 1; ++i)
            {
                *(new_view->p +
                  2 * (new_view->e.x - new_view->a.x + 1) * (j - new_view->a.y) + (i -
                          2 *
                          new_view->
                          a.
                          x)) =
                              e_gt_byte (i, j);
            }
        for (j = view->a.y; j < ya; ++j)
            for (i = 2 * view->a.x; i <= 2 * view->e.x + 1; ++i)
                e_pt_byte (i, j, *(view->p + (j - view->a.y) * 2
                                   * (view->e.x - view->a.x + 1) + (i -
                                           2 * view->a.x)));
        for (j = view->a.y; j <= view->e.y; ++j)
            for (i = 2 * view->a.x; i < 2 * xa; i = i + 1)
                e_pt_byte (i, j, *(view->p + (j - view->a.y) * 2
                                   * (view->e.x - view->a.x + 1) + (i -
                                           2 * view->a.x)));
        for (j = view->e.y; j > ye; --j)
            for (i = 2 * view->a.x; i <= 2 * view->e.x + 1; ++i)
                e_pt_byte (i, j, *(view->p + (j - view->a.y) * 2
                                   * (view->e.x - view->a.x + 1) + (i -
                                           2 * view->a.x)));
        for (j = view->e.y; j >= view->a.y; --j)
            for (i = 2 * view->e.x + 1; i > 2 * xe; --i)
                e_pt_byte (i, j, *(view->p + (j - view->a.y) * 2
                                   * (view->e.x - view->a.x + 1) + (i -
                                           2 * view->a.x)));
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
        e_put_pic_xrect (view);
        e_get_pic_xrect (xa, ya, xe, ye, new_view);
#endif
#ifndef NEWSTYLE
        if (WpeIsXwin ())
        {
            if (xe < MAXSCOL - 1)
                for (i = ya + 1; i <= ye + 1 && i < MAXSLNS - 1; i++)
                    e_pt_col (xe + 1, i, SHDCOL);
            if (xe < MAXSCOL - 2)
                for (i = ya + 1; i <= ye + 1 && i < MAXSLNS - 1; i++)
                    e_pt_col (xe + 2, i, SHDCOL);
            if (ye < MAXSLNS - 2)
                for (i = xa + 2; i <= xe; i++)
                    e_pt_col (i, ye + 1, SHDCOL);
        }
#endif
        free (view->p);
        free (view);
    }
#ifndef NO_XWINDOWS
    if (WpeIsXwin ())
        (*e_u_setlastpic) (new_view);
#endif
    return (new_view);
}

we_view_t *
e_ed_kst (we_window_t * window, we_view_t * view, int sw)
{
    we_view_t *new_view = e_change_pic (window->a.x, window->a.y, window->e.x,
                                        window->e.y, view, sw, window->colorset->er.fg_bg_color);
    e_ed_rahmen (window, sw);
    return (new_view);
}

/*    delete buffer     */
int
e_close_buffer (we_buffer_t * buffer)
{
    int i;

    if (buffer != NULL)
    {
        e_remove_undo (buffer->undo, buffer->control->numundo + 1);
        if (buffer->buflines != NULL)
        {
            for (i = 0; i < buffer->mxlines; i++)
            {
                if (buffer->buflines[i].s != NULL)
                    free (buffer->buflines[i].s);
                buffer->buflines[i].s = NULL;
            }
            free (buffer->buflines);
        }
        free (buffer);
    }
    return (0);
}

/*    close window */
int
e_close_window (we_window_t * window)
{
    we_control_t *control = window->edit_control;
    we_window_t *f0 = window->edit_control->window[0];
    int c = 0;
    unsigned long maxname;
    char text[256];

    window = control->window[control->mxedt];
    if (window->dtmd == DTMD_FILEMANAGER)
    {
        FLBFFR *file_buffer = (FLBFFR *) window->buffer;

        free (window->dirct);
        free (file_buffer->rdfile);
        freedf (file_buffer->df);
        freedf (file_buffer->fw->df);
        freedf (file_buffer->dd);
        freedf (file_buffer->cd);
        freedf (file_buffer->dw->df);
        free (file_buffer->fw);
        free (file_buffer->dw);
        free (file_buffer);
        (control->mxedt)--;
        control->curedt = control->edt[control->mxedt];
        e_close_view (window->view, 1);
        if (window != f0 && window != NULL)
        {
            e_free_find (&window->find);
            free (window);
        }
        if (control->mxedt > 0)
        {
            window = control->window[control->mxedt];
            e_ed_rahmen (window, 1);
        }
        return (0);
    }
    if (window->dtmd == DTMD_DATA)
    {
        FLWND *fw = (FLWND *) window->buffer;
        int swt = window->ins;

#ifdef PROG
        if (swt == 4 && window->save)
            e_p_update_prj_fl (window);
#endif
        if (window->dirct)
            free (window->dirct);
        if (swt == 7)
            freedf (fw->df);
        free (fw);
        (control->mxedt)--;
        control->curedt = control->edt[control->mxedt];
        e_close_view (window->view, 1);
        if (window != f0 && window != NULL)
        {
            e_free_find (&window->find);
            free (window);
        }
        if (control->mxedt > 0 && (swt < 5 || swt == 7))
        {
            window = control->window[control->mxedt];
            e_ed_rahmen (window, 1);
        }
        return (0);
    }
    if (window == NULL || window->edit_control->mxedt <= 0)
        return (0);
    if (window != f0)
    {
        if (window->save != 0 && window->ins != 8)
        {
            sprintf (text, "File %s NOT saved!\nDo you want to save File ?",
                     window->datnam);
            c = e_message (1, text, window);
            if (c == WPE_ESC)
                return (c);
            else if (c == 'Y')
                e_save (window);
        }
        /* Check if file system could have an autosave or emergency save file
           >12 check is to eliminate dos file systems */
        if (((maxname =
                    pathconf (window->dirct, _PC_NAME_MAX)) >= strlen (window->datnam) + 4)
                && (maxname > 12))
        {
            remove (e_make_postf (text, window->datnam, ".ASV"));
            remove (e_make_postf (text, window->datnam, ".ESV"));
        }
        if (strcmp (window->datnam, "Messages") && strcmp (window->datnam, "Watches"))
            e_close_buffer (window->buffer);
        if (window->dtmd == DTMD_HELP && window->ins == 8)
            e_help_free (window);
        if (window->datnam != NULL)
            free (window->datnam);
        if (window->dirct != NULL)
            free (window->dirct);
        if (window && window->screen != NULL)
            free (window->screen);
    }
    (control->mxedt)--;
    control->curedt = control->edt[control->mxedt];
    e_close_view (window->view, 1);
    if (window != f0 && window != NULL)
    {
        e_free_find (&window->find);
        free (window);
    }
    if (control->mxedt > 0)
    {
        window = control->window[control->mxedt];
        e_ed_rahmen (window, 1);
    }
    return (c);
}

/*    Toggle among windows  */
int
e_rep_win_tree (we_control_t * control)
{
    int i;

    if (control->mxedt <= 0)
        return (0);
    ini_repaint (control);
    for (i = 1; i < control->mxedt; i++)
    {
        e_firstl (control->window[i], 0);
        e_write_screen (control->window[i], 0);
    }
    e_firstl (control->window[i], 1);
    e_write_screen (control->window[i], 1);
    e_cursor (control->window[i], 1);
    end_repaint ();
    return (0);
}

void
e_switch_window (int num, we_window_t * window)
{
    we_control_t *control = window->edit_control;
    we_window_t *ft;
    int n, i, te;

    for (n = 1; control->edt[n] != num && n < control->mxedt; n++)
        ;
    if (n >= control->mxedt)
        return;
    for (i = control->mxedt; i >= 1; i--)
    {
        free (control->window[i]->view->p);
        free (control->window[i]->view);
    }
    ft = control->window[n];
    te = control->edt[n];
    for (i = n; i < control->mxedt; i++)
    {
        control->edt[i] = control->edt[i + 1];
        control->window[i] = control->window[i + 1];
    }
    control->window[i] = ft;
    control->edt[i] = te;
    control->curedt = num;
    e_rep_win_tree (control);
}

/*    zoom windows   */
int
e_ed_zoom (we_window_t * window)
{
    if (window->edit_control->mxedt > 0)
    {
        if (window->zoom == 0)
        {
            window->sa = e_set_pnt (window->a.x, window->a.y);
            window->se = e_set_pnt (window->e.x, window->e.y);
            window->a = e_set_pnt (0, 1);
            window->e = e_set_pnt (MAXSCOL - 1, MAXSLNS - 2);
            window->zoom = 1;
        }
        else
        {
            window->a = e_set_pnt (window->sa.x, window->sa.y);
            window->e = e_set_pnt (window->se.x, window->se.y);
            window->zoom = 0;
        }
        window->view = e_ed_kst (window, window->view, 1);
        if (window->view == NULL)
            e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
        e_cursor (window, 1);
        e_write_screen (window, 1);
    }
    return (WPE_ESC);
}

/*   cascade windows   */
int
e_ed_cascade (we_window_t * window)
{
    we_control_t *control = window->edit_control;
    int i;

    if (control->mxedt < 1)
        return 0;			/* no windows open */
    for (i = control->mxedt; i >= 1; i--)
    {
        free (control->window[i]->view->p);
        free (control->window[i]->view);
        control->window[i]->a = e_set_pnt (i - 1, i);
        control->window[i]->e =
            e_set_pnt (MAXSCOL - 1 - control->mxedt + i, MAXSLNS - 2 - control->mxedt + i);
    }
    ini_repaint (control);
    for (i = 1; i < control->mxedt; i++)
    {
        e_firstl (control->window[i], 0);
        e_write_screen (control->window[i], 0);
    }
    e_firstl (control->window[i], 1);
    e_write_screen (control->window[i], 1);
    e_cursor (control->window[i], 1);
    end_repaint ();
    return (0);
}

/*   Tile windows   */
int
e_ed_tile (we_window_t * window)
{
    we_control_t *control = window->edit_control;
    we_point_t atmp[MAXEDT + 1];
    we_point_t etmp[MAXEDT + 1];
    int i, j, ni, nj;
    int editwin = 0;		/* number of editor windows */
    int editorwin[MAXEDT + 1];
    int maxlines = MAXSLNS;

    for (i = control->mxedt; i >= 1; i--)
    {
        if ((!(control->edopt & ED_OLD_TILE_METHOD))
                && (!DTMD_ISTEXT (control->window[i]->dtmd)
                    || ((WpeIsProg ())
                        && ((strcmp (control->window[i]->datnam, "Messages") == 0)
                            || (strcmp (control->window[i]->datnam, "Watches") == 0)))))
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
        return (0);
    if ((!(control->edopt & ED_OLD_TILE_METHOD)) && (WpeIsProg ()))
    {
        maxlines -= MAXSLNS / 3 - 1;
    }
    for (i = control->mxedt; i >= 1; i--)
    {
        free (control->window[i]->view->p);
        free (control->window[i]->view);
    }
    for (ni = editwin, nj = 1; ni > 1; ni--)
    {
        nj = editwin / ni;
        if (editwin % ni)
            nj++;
        if (nj >= ni)
            break;
    }
    if (nj * ni < editwin)
        nj++;
    for (j = 0; j < nj; j++)
    {
        for (i = 0; i < ni; i++)
        {
            if (j == 0)
            {
                if (i == 0)
                {
                    atmp[j * ni + i].x = i * MAXSCOL / ni;
                    etmp[j * ni + i].x = (i + 1) * MAXSCOL / ni - 1;
                    if (etmp[j * ni + i].x - atmp[j * ni + i].x < 26)
                        etmp[j * ni + i].x = atmp[j * ni + i].x + 26;
                }
                else
                {
                    etmp[j * ni + i].x = (i + 1) * MAXSCOL / ni - 1;
                    atmp[j * ni + i].x = etmp[j * ni + i - 1].x + 1;
                    if (etmp[j * ni + i].x - atmp[j * ni + i].x < 26)
                        etmp[j * ni + i].x = atmp[j * ni + i].x + 26;
                    if (etmp[j * ni + i].x >= MAXSCOL)
                    {
                        etmp[j * ni + i].x = MAXSCOL - 1;
                        atmp[j * ni + i].x = etmp[j * ni + i].x - 26;
                    }
                }
            }
            else
            {
                atmp[j * ni + i].x = atmp[(j - 1) * ni + i].x;
                etmp[j * ni + i].x = etmp[(j - 1) * ni + i].x;
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
                    atmp[j * ni + i].y = j * (maxlines - 2) / nj + 1;
                    etmp[j * ni + i].y = (j + 1) * (maxlines - 2) / nj;
                    if (etmp[j * ni + i].y - atmp[j * ni + i].y < 3)
                        etmp[j * ni + i].y = atmp[j * ni + i].y + 3;
                }
                else
                {
                    etmp[j * ni + i].y = (j + 1) * (maxlines - 2) / nj;
                    atmp[j * ni + i].y = etmp[(j - 1) * ni + i].y + 1;
                    if (etmp[j * ni + i].y - atmp[j * ni + i].y < 3)
                        etmp[j * ni + i].y = atmp[j * ni + i].y + 3;
                    if (etmp[j * ni + i].y > maxlines - 2)
                    {
                        etmp[j * ni + i].y = maxlines - 2;
                        atmp[j * ni + i].y = etmp[j * ni + i].y - 3;
                    }
                }
            }
            else
            {
                atmp[j * ni + i].y = atmp[j * ni + i - 1].y;
                etmp[j * ni + i].y = etmp[j * ni + i - 1].y;
            }
        }
    }
    for (i = 0, j = 1; i < editwin; i++, j++)
    {
        while (!editorwin[j])
            j++;
        control->window[j]->a = e_set_pnt (atmp[i].x, atmp[i].y);
        control->window[j]->e = e_set_pnt (etmp[i].x, etmp[i].y);
        control->window[j]->zoom = 0;	/* Make sure zoom is off */
    }
    ini_repaint (control);
    for (i = 1; i < control->mxedt; i++)
    {
        e_firstl (control->window[i], 0);
        e_write_screen (control->window[i], 0);
    }
    e_firstl (control->window[i], 1);
    e_write_screen (control->window[i], 1);
    e_cursor (control->window[i], 1);
    end_repaint ();
    return (0);
}

/*   call next window   */
int
e_ed_next (we_window_t * window)
{
    if (window->edit_control->mxedt > 0)
        e_switch_window (window->edit_control->edt[1], window);
    return (0);
}

/*   write a line (screen content)     */
void
e_pr_line (int y, we_window_t * window)
{
    we_buffer_t *buffer = window->buffer;
    we_screen_t *s = window->screen;
    int i, j, k, frb;
#ifdef DEBUGGER
    int fsw = 0;
#endif

    for (i = j = 0; j < num_cols_off_screen_left(window); j++, i++)
    {
        if (*(buffer->buflines[y].s + i) == WPE_TAB)
            j += (window->edit_control->tabn - j % window->edit_control->tabn - 1);
        else if (((unsigned char) *(buffer->buflines[y].s + i)) > 126)
        {
            j++;
            if (((unsigned char) *(buffer->buflines[y].s + i)) < 128 + ' ')
                j++;
        }
        else if (*(buffer->buflines[y].s + i) < ' ')
            j++;
    }
    if (j > num_cols_off_screen_left(window))
        i--;
#ifdef DEBUGGER
    for (j = 1; j <= s->brp[0]; j++)
        if (s->brp[j] == y)
        {
            fsw = 1;
            break;
        }
    for (j = num_cols_off_screen_left(window);
            i < buffer->buflines[y].len && j < col_num_on_screen_right(window); i++, j++)
    {
        if (y == s->da.y && i >= s->da.x && i < s->de.x)
            frb = s->colorset->dy.fg_bg_color;
        else if (fsw)
            frb = s->colorset->db.fg_bg_color;
        /*	else if( (i == s->pt[0].x && y == s->pt[0].y) || (i == s->pt[1].x && y == s->pt[1].y)  */
        else if (y == s->fa.y && i >= s->fa.x && i < s->fe.x)
            frb = s->colorset->ek.fg_bg_color;
#else
    for (j = num_cols_off_screen_left(window);
            i < buffer->buflines[y].len && j < col_num_on_screen_right(window); i++, j++)
    {
        if (y == s->fa.y && i >= s->fa.x && i < s->fe.x)
            frb = s->colorset->ek.fg_bg_color;
#endif
        /*	if( (i == s->pt[0].x && y == s->pt[0].y) || (i == s->pt[1].x && y == s->pt[1].y)
                 || (i == s->pt[2].x && y == s->pt[2].y) || (i == s->pt[3].x && y == s->pt[3].y)
                 || (i == s->pt[4].x && y == s->pt[4].y) || (i == s->pt[5].x && y == s->pt[5].y)
                 || (i == s->pt[6].x && y == s->pt[6].y) || (i == s->pt[7].x && y == s->pt[7].y)
                 || (i == s->pt[8].x && y == s->pt[8].y) || (i == s->pt[9].x && y == s->pt[9].y))
                    frb = s->colorset->ek.fg_bg_color;
        */
        else if ((y < s->mark_end.y && (y > s->mark_begin.y ||
                                        (y == s->mark_begin.y
                                         && i >= s->mark_begin.x)))
                 || (y == s->mark_end.y && i < s->mark_end.x
                     && (y > s->mark_begin.y
                         || (y == s->mark_begin.y && i >= s->mark_begin.x))))
            frb = s->colorset->ez.fg_bg_color;
        else
            frb = s->colorset->et.fg_bg_color;

        if (window->dtmd == DTMD_HELP)
        {
            if (*(buffer->buflines[y].s + i) == HBG || *(buffer->buflines[y].s + i) == HFB ||
                    *(buffer->buflines[y].s + i) == HHD || *(buffer->buflines[y].s + i) == HBB)
            {
                if (*(buffer->buflines[y].s + i) == HHD)
                    frb = s->colorset->hh.fg_bg_color;
                else if (*(buffer->buflines[y].s + i) == HBB)
                    frb = s->colorset->hm.fg_bg_color;
                else
                    frb = s->colorset->hb.fg_bg_color;
#ifdef NEWSTYLE
                if (*(buffer->buflines[y].s + i) != HBB)
                    k = j;
                else
                    k = -1;
#endif
                for (i++; buffer->buflines[y].s[i] != HED && i < buffer->buflines[y].len &&
                        j < col_num_on_screen_right(window); i++, j++)
                    e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                               y - num_lines_off_screen_top(window) + window->a.y + 1,
                               *(buffer->buflines[y].s + i), frb);
                j--;
#ifdef NEWSTYLE
                if (WpeIsXwin () && k >= 0)
                    e_make_xrect (window->a.x - num_cols_off_screen_left(window) + k + 1,
                                  y - num_lines_off_screen_top(window) + window->a.y + 1,
                                  window->a.x - num_cols_off_screen_left(window) + j + 1,
                                  y - num_lines_off_screen_top(window) + window->a.y + 1, 0);
#endif
                continue;
            }
            else if (*(buffer->buflines[y].s + i) == HFE)
            {
                for (; j < col_num_on_screen_right(window); j++)
                    e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                               y - num_lines_off_screen_top(window) + window->a.y + 1, ' ',
                               s->colorset->hh.fg_bg_color);
                return;
            }
            else if (*(buffer->buflines[y].s + i) == HNF)
            {
                for (k = j, i++; buffer->buflines[y].s[i] != ':' && i < buffer->buflines[y].len &&
                        j < col_num_on_screen_right(window); i++, j++)
                    e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                               y - num_lines_off_screen_top(window) + window->a.y + 1,
                               *(buffer->buflines[y].s + i), s->colorset->hb.fg_bg_color);
#ifdef NEWSTYLE
                if (WpeIsXwin ())
                    e_make_xrect (window->a.x - num_cols_off_screen_left(window) + k + 1,
                                  y - num_lines_off_screen_top(window) + window->a.y + 1,
                                  window->a.x - num_cols_off_screen_left(window) + j,
                                  y - num_lines_off_screen_top(window) + window->a.y + 1, 0);
#endif
                for (; buffer->buflines[y].s[i] != HED && i < buffer->buflines[y].len &&
                        j < col_num_on_screen_right(window); i++, j++)
                    e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                               y - num_lines_off_screen_top(window) + window->a.y + 1,
                               *(buffer->buflines[y].s + i), frb);
                for (i++;
                        buffer->buflines[y].s[i] != HED && i < buffer->buflines[y].len
                        && j < col_num_on_screen_right(window); i++, j++)
                    e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                               y - num_lines_off_screen_top(window) + window->a.y + 1, ' ',
                               frb);
                for (;
                        buffer->buflines[y].s[i] != '.' && i < buffer->buflines[y].len
                        && j < col_num_on_screen_right(window); i++, j++)
                    e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                               y - num_lines_off_screen_top(window) + window->a.y + 1, ' ',
                               frb);
                j--;
                continue;
            }
            else if (*(buffer->buflines[y].s + i) == HED)
            {
                j--;
                continue;
            }
        }
        if (*(buffer->buflines[y].s + i) == WPE_TAB)
            for (k = window->edit_control->tabn - j % window->edit_control->tabn; k > 1 &&
                    j < num_cols_on_screen(window) + num_cols_off_screen_left(window) - 2; k--, j++)
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1, ' ', frb);
        else if (!WpeIsXwin () && ((unsigned char) *(buffer->buflines[y].s + i)) > 126)
        {
            e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                       y - num_lines_off_screen_top(window) + window->a.y + 1, '@', frb);
            if (++j >= col_num_on_screen_right(window))
                return;
            if (((unsigned char) *(buffer->buflines[y].s + i)) < 128 + ' ' &&
                    j < col_num_on_screen_right(window))
            {
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1, '^', frb);
                if (++j >= col_num_on_screen_right(window))
                    return;
            }
        }
        else if (*(buffer->buflines[y].s + i) < ' ')
        {
            e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                       y - num_lines_off_screen_top(window) + window->a.y + 1, '^', frb);
            if (++j >= col_num_on_screen_right(window))
                return;
        }
        if (*(buffer->buflines[y].s + i) == WPE_TAB)
            e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                       y - num_lines_off_screen_top(window) + window->a.y + 1, ' ', frb);
        else if (!WpeIsXwin () && ((unsigned char) *(buffer->buflines[y].s + i)) > 126
                 && j < col_num_on_screen_right(window))
        {
            if (((unsigned char) *(buffer->buflines[y].s + i)) < 128 + ' ')
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1,
                           ((unsigned char) *(buffer->buflines[y].s + i)) + 'A' - 129, frb);
            else
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1,
                           ((unsigned char) *(buffer->buflines[y].s + i)) - 128, frb);
        }
        else if (*(buffer->buflines[y].s + i) < ' ' && j < col_num_on_screen_right(window))
            e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                       y - num_lines_off_screen_top(window) + window->a.y + 1,
                       *(buffer->buflines[y].s + i) + 'A' - 1, frb);
        else
            e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                       y - num_lines_off_screen_top(window) + window->a.y + 1,
                       *(buffer->buflines[y].s + i), frb);
    }

    if ((i == buffer->buflines[y].len) && (window->edit_control->edopt & ED_SHOW_ENDMARKS) &&
            (DTMD_ISMARKABLE (window->dtmd)) && (j < col_num_on_screen_right(window)))
    {
        if ((y < s->mark_end.y && (y > s->mark_begin.y ||
                                   (y == s->mark_begin.y
                                    && i >= s->mark_begin.x)))
                || (y == s->mark_end.y && i < s->mark_end.x
                    && (y > s->mark_begin.y
                        || (y == s->mark_begin.y && i >= s->mark_begin.x))))
        {
            if (*(buffer->buflines[y].s + i) == WPE_WR)
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1, PWR,
                           s->colorset->ez.fg_bg_color);
            else
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1, PNL,
                           s->colorset->ez.fg_bg_color);
        }
        else
        {
            if (*(buffer->buflines[y].s + i) == WPE_WR)
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1, PWR,
                           s->colorset->et.fg_bg_color);
            else
                e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                           y - num_lines_off_screen_top(window) + window->a.y + 1, PNL,
                           s->colorset->et.fg_bg_color);
        }
        j++;
    }

    for (; j < col_num_on_screen_right(window); j++)
        e_pr_char (window->a.x - num_cols_off_screen_left(window) + j + 1,
                   y - num_lines_off_screen_top(window) + window->a.y + 1, ' ', s->colorset->et.fg_bg_color);
}

/*   draw standard-box frame  */
void
e_std_window (int xa, int ya, int xe, int ye, char *name, int sw, int frb,
              int fes)
{
    int i;
    char rhm[2][6];
    char *short_name;

    rhm[0][0] = RE1;
    rhm[0][1] = RE2;
    rhm[0][2] = RE3;
    rhm[0][3] = RE4;
    rhm[0][4] = RE5;
    rhm[0][5] = RE6;
    rhm[1][0] = RD1;
    rhm[1][1] = RD2;
    rhm[1][2] = RD3;
    rhm[1][3] = RD4;
    rhm[1][4] = RD5;
    rhm[1][5] = RD6;
    for (i = xa + 1; i < xe; i++)
    {
        e_pr_char (i, ya, rhm[sw][4], frb);
        e_pr_char (i, ye, rhm[sw][4], frb);
    }
    for (i = ya + 1; i < ye; i++)
    {
        e_pr_char (xa, i, rhm[sw][5], frb);
        e_pr_char (xe, i, rhm[sw][5], frb);
    }
    e_pr_char (xa, ya, rhm[sw][0], frb);
    e_pr_char (xa, ye, rhm[sw][2], frb);
    e_pr_char (xe, ya, rhm[sw][1], frb);
    e_pr_char (xe, ye, rhm[sw][3], frb);

    if (name)
    {
        if (strlen (name) < (unsigned int) (xe - xa - 14))
            e_pr_str ((xa + xe - strlen (name)) / 2, ya, name, frb, 0, 0, 0, 0);
        else
        {
            short_name = strdup (name);
            strcpy (short_name + xe - xa - 17, "...");
            e_pr_str (xa + 7, ya, short_name, frb, 0, 0, 0, 0);
            free (short_name);
        }
    }
    if (sw != 0)
    {
        e_pr_char (xa + 3, ya, WBT, fes);
#ifdef NEWSTYLE
        if (!WpeIsXwin ())
#endif
        {
            e_pr_char (xa + 2, ya, '[', frb);
            e_pr_char (xa + 4, ya, ']', frb);
        }
#ifdef NEWSTYLE
        else
            e_make_xrect (xa + 2, ya, xa + 4, ya, 0);
    }
    if (WpeIsXwin ())
        e_make_xr_window (xa, ya, xe, ye, sw);
#else
    }
    /*
       if(xe < MAXSCOL-1) for(i = ya+1; i <= ye; i++) e_pt_col(xe+1, i, SHDCOL);
       if(ye < MAXSLNS-2) for(i = xa+1; i <= xe+1 && i < MAXSCOL; i++)
    						e_pt_col(i, ye+1, SHDCOL);
    */
#endif
}

struct dirfile *
e_add_df (char *str, struct dirfile *df)
{
    int i, n;
    char *tmp;

    if (df == NULL)
    {
        df = malloc (sizeof (struct dirfile));
        df->nr_files = 0;
        df->name = malloc (sizeof (char *));
    }
    for (n = 0; n < df->nr_files && *df->name[n] && strcmp (df->name[n], str); n++);
    if (n == df->nr_files)
    {
        if (df->nr_files == MAXSVSTR - 1)
            free (df->name[df->nr_files - 1]);
        else
        {
            df->nr_files++;
            df->name = realloc (df->name, df->nr_files * sizeof (char *));
        }
        for (i = df->nr_files - 1; i > 0; i--)
            df->name[i] = df->name[i - 1];
        df->name[0] = malloc ((strlen (str) + 1) * sizeof (char));
        strcpy (df->name[0], str);
    }
    else
    {
        tmp = df->name[n];
        for (i = n; i > 0; i--)
            df->name[i] = df->name[i - 1];
        if (!tmp[0])
        {
            free (tmp);
            df->name[0] = malloc ((strlen (str) + 1) * sizeof (char));
            strcpy (df->name[0], str);
        }
        else
            df->name[0] = tmp;
    }
    return (df);
}

int
e_sv_window (int xa, int ya, int *n, struct dirfile *df, we_window_t * window)
{
    we_control_t *control = window->edit_control;
    int ret, ye = ya + 6;
    int xe = xa + 21;
    FLWND *fw = malloc (sizeof (FLWND));

    if ((window = (we_window_t *) malloc (sizeof (we_window_t))) == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, control->colorset);
    if (xe > MAXSCOL - 3)
    {
        xe = MAXSCOL - 3;
        xa = xe - 21;
    }
    if (ye > MAXSLNS - 3)
    {
        ye = MAXSLNS - 3;
        ya = ye - 6;
    }
    window->colorset = control->colorset;
    window->a = e_set_pnt (xa, ya);
    window->e = e_set_pnt (xe, ye);
    window->dtmd = DTMD_FILEDROPDOWN;
    window->zoom = 0;
    window->edit_control = control;
    window->c_sw = NULL;
    window->c_st = NULL;
    window->find.dirct = NULL;
    window->winnum = -1;
    window->datnam = "";
    if (!(window->view = e_ed_kst (window, NULL, 1)))
    {
        e_error (e_msg[ERR_LOWMEM], 0, window->colorset);
        return (0);
    }
    window->buffer = (we_buffer_t *) fw;
    fw->mxa = xa;
    fw->mxe = xe;
    fw->mya = ya;
    fw->mye = ye;
    fw->xa = xa + 1;
    fw->xe = xe;
    fw->ya = ya + 1;
    fw->ye = ye;
    fw->df = df;
    fw->srcha = 0;
    fw->window = window;
    fw->nf = fw->ia = fw->ja = 0;
    do
    {
        ret = e_file_window (0, fw, window->colorset->er.fg_bg_color, window->colorset->ez.fg_bg_color);
#if MOUSE
        if (ret < 0)
            ret = e_rahmen_mouse (window);
#endif
        if ((ret == AF2 && !(window->edit_control->edopt & ED_CUA_STYLE)) ||
                (window->edit_control->edopt & ED_CUA_STYLE && ret == CtrlL))
            e_size_move (window);
    }
    while (ret != WPE_CR && ret != WPE_ESC);
    *n = fw->nf;
    e_close_view (window->view, 1);
    free (fw);
    free (window);
    return (ret);
}

int
e_schr_lst_wsv (char *str, int xa, int ya, int n, int len, int ft,
                int fz, struct dirfile **df, we_window_t * window)
{
#if MOUSE
    extern struct mouse e_mouse;
#endif
    int ret, num;
    do
    {
        *df = e_add_df (str, *df);
#ifndef NEWSTYLE
        ret = e_schreib_leiste (str, xa, ya, n - 4, len, ft, fz);
#else
        ret = e_schreib_leiste (str, xa, ya, n - 3, len, ft, fz);
#endif
#if MOUSE
        if (ret < 0 && e_mouse.y == ya && e_mouse.x >= xa + n - 3
                && e_mouse.x <= xa + n - 1)
            ret = CDO;
#endif
        if (ret == CDO && e_sv_window (xa + n, ya, &num, *df, window) == WPE_CR)
            strcpy (str, (*df)->name[num]);
    }
    while (ret == CDO);
    return (ret);
}

int
e_schr_nchar_wsv (char *str, int x, int y, int n, int max, int col, int csw)
#ifdef NEWSTYLE
{
    e_pr_char (x + max - 3, y, ' ', csw);
    e_pr_char (x + max - 2, y, WSW, csw);
    e_pr_char (x + max - 1, y, ' ', csw);
    e_make_xrect (x + max - 3, y, x + max - 1, y, 0);
    return (e_schr_nchar (str, x, y, n, max - 3, col));
}
#else
{
#if !defined(NO_XWINDOWS)
    int swcol = (e_gt_col (x + max, y) / 16) * 16;
    if (WpeIsXwin ())
    {
        e_pr_char (x + max, y, SCR, swcol);
        e_pr_char (x + max, y + 1, SCD, swcol);
        e_pr_char (x + max - 1, y + 1, SCD, swcol);
        e_pr_char (x + max - 2, y + 1, SCD, swcol);
    }
#endif
    e_pr_char (x + max - 3, y, ' ', csw);
    e_pr_char (x + max - 2, y, WSW, csw);
    e_pr_char (x + max - 1, y, ' ', csw);
    return (e_schr_nchar (str, x, y, n, max - 4, col));
}
#endif

int
e_mess_win (char *header, char *str, we_view_t ** view, we_window_t * window)
{
    we_control_t *control = window->edit_control;
    extern int (*e_u_kbhit) (void);
#if MOUSE
    extern struct mouse e_mouse;
#endif
    int xa, ya, xe, ye, num, anz = 0, mxlen = 0, i, j;
    char **s;

    s = StringToStringArray (str, &mxlen, strlen (header) + 8, &anz);

    ya = (MAXSLNS - anz - 6) / 2;
    ye = ya + anz + 5;
    xa = (MAXSCOL - mxlen - 6) / 2;
    xe = xa + mxlen + 6;
    if (ya < 2)
        ya = 2;
    if (ye > MAXSLNS - 3)
        ye = MAXSLNS - 3;
    num = anz;
    if (num > ye - ya - 5)
    {
        num = ye - ya - 5;
        strcpy (s[num - 1], "...");
    }

    if (!(*view) || (*view)->e.x != xe || (*view)->a.x != xa || (*view)->e.x < xe)
    {
        *view = e_change_pic (xa, ya, xe, ye, *view, 1, control->colorset->nt.fg_bg_color);
        for (i = xa + 1; i < xe; i++)
        {
            e_pr_char (i, ye - 2, ' ', window->colorset->nt.fg_bg_color);
            e_pr_char (i, ye - 1, ' ', window->colorset->nt.fg_bg_color);
        }
        e_pr_str ((xe + xa - 6) / 2, ye - 2, "Ctrl C", control->colorset->nz.fg_bg_color, -1, -1,
                  control->colorset->ns.fg_bg_color, control->colorset->nt.fg_bg_color);
    }
    e_std_window (xa, ya, xe, ye, header, 1, control->colorset->nr.fg_bg_color, control->colorset->ne.fg_bg_color);
    for (i = xa + 1; i < xe; i++)
        e_pr_char (i, ya + 1, ' ', control->colorset->nr.fg_bg_color);
    for (j = 0; j < num; j++)
    {
        e_pr_char (xa + 1, ya + 2 + j, ' ', control->colorset->nt.fg_bg_color);
        e_pr_char (xa + 2, ya + 2 + j, ' ', control->colorset->nt.fg_bg_color);
        e_pr_str (xa + 3, ya + 2 + j, s[j], control->colorset->nt.fg_bg_color, 0, 0, 0, 0);
        for (i = xa + strlen (s[j]) + 3; i < xe; i++)
            e_pr_char (i, ya + 2 + j, ' ', control->colorset->nt.fg_bg_color);
    }
    for (j += ya + 2; j < ye - 2; j++)
        for (i = xa + 1; i < xe; i++)
            e_pr_char (i, j, ' ', control->colorset->nt.fg_bg_color);
    for (i = 0; i < anz; i++)
        free (s[i]);
    free (s);
#ifndef NO_XWINDOWS
    if (WpeIsXwin ())
    {
        while ((i = (*e_u_kbhit) ()))
        {
            if (i == -1 && e_mouse.y == ye - 2 && e_mouse.x > (xe + xa - 10) / 2
                    && e_mouse.x < (xe + xa + 6) / 2)
                i = CtrlC;
            if (i == CtrlC)
                break;
        }
    }
    else
#endif
        while ((i = (*e_u_kbhit) ()) && i != CtrlC)
            ;
    return (i == CtrlC ? 1 : 0);
}

int
e_opt_sec_box (int xa, int ya, int num, OPTK * opt, we_window_t * window, int sw)
{
    we_view_t *view;
    int n, nold, max = 0, i, c = 0, xe, ye = ya + num + 1;
    for (i = 0; i < num; i++)
        if ((n = strlen (opt[i].t)) > max)
            max = n;
    xe = xa + max + 3;
    view =
        e_std_view (xa, ya, xe, ye, NULL, sw, window->colorset->nr.fg_bg_color, window->colorset->nt.fg_bg_color,
                    window->colorset->ne.fg_bg_color);
    if (view == NULL)
    {
        e_error (e_msg[ERR_LOWMEM], 0, window->colorset);
        return (-2);
    }
    for (i = 0; i < num; i++)
        e_pr_str_wsd (xa + 2, ya + i + 1, opt[i].t, window->colorset->mt.fg_bg_color, opt[i].x,
                      1, window->colorset->ms.fg_bg_color, xa + 1, xe - 1);
#if  MOUSE
    while (e_mshit () != 0);
#endif
    n = 0;
    nold = 1;
    while (c != WPE_ESC && c != WPE_CR)
    {
        if (nold != n)
        {
            e_pr_str_wsd (xa + 2, nold + ya + 1, opt[nold].t, window->colorset->mt.fg_bg_color,
                          opt[nold].x, 1, window->colorset->ms.fg_bg_color, xa + 1, xe - 1);
            e_pr_str_wsd (xa + 2, n + ya + 1, opt[n].t, window->colorset->mz.fg_bg_color,
                          opt[n].x, 1, window->colorset->mz.fg_bg_color, xa + 1, xe - 1);
            nold = n;
        }
#if  MOUSE
        if ((c = toupper (e_getch ())) == -1)
            c = e_m2_mouse (xa, ya, xe, ye, opt);
#else
        c = toupper (e_getch ());
#endif
        for (i = 0; i < ye - ya - 1; i++)
            if (c == opt[i].o)
            {
                c = WPE_CR;
                n = i;
                break;
            }
        if (i > ye - ya)
            c = WPE_ESC;
        else if (c == CUP || c == CtrlP)
            n = n > 0 ? n - 1 : ye - ya - 2;
        else if (c == CDO || c == CtrlN)
            n = n < ye - ya - 2 ? n + 1 : 0;
        else if (c == POS1 || c == CtrlA)
            n = 0;
        else if (c == ENDE || c == CtrlE)
            n = ye - ya - 2;
    }
    if (sw == 1)
        e_close_view (view, 1);
    return (c == WPE_ESC ? -1 : n);
}

struct dirfile *
e_make_win_list (we_window_t * window)
{
    int i;
    struct dirfile *df;

    if (!(df = malloc (sizeof (struct dirfile))))
        return (NULL);
    df->nr_files = window->edit_control->mxedt;
    if (!(df->name = malloc (df->nr_files * sizeof (char *))))
    {
        free (df);
        return (NULL);
    }
    for (i = 0; i < df->nr_files; i++)
    {
        if (window->edit_control->window[df->nr_files - i]->datnam)
        {
            if (!(df->name[i] =
                        malloc ((strlen (window->edit_control->window[df->nr_files - i]->datnam) +
                                 1) * sizeof (char))))
            {
                df->nr_files = i;
                freedf (df);
                return (NULL);
            }
            else
                strcpy (df->name[i], window->edit_control->window[df->nr_files - i]->datnam);
        }
        else
        {
            if (!(df->name[i] = malloc (sizeof (char))))
            {
                df->nr_files = i;
                freedf (df);
                return (NULL);
            }
            else
                *df->name[i] = '\0';
        }
    }
    return (df);
}

int
e_list_all_win (we_window_t * window)
{
    int i;

    for (i = window->edit_control->mxedt; i > 0; i--)
        if (window->edit_control->window[i]->dtmd == DTMD_DATA && window->edit_control->window[i]->ins == 7)
        {
            e_switch_window (window->edit_control->edt[i], window);
            return (0);
        }
    return (e_data_first (7, window->edit_control, NULL));
}

#ifdef NEWSTYLE
int
e_get_pic_xrect (int xa, int ya, int xe, int ye, we_view_t * view)
{
    int i = xa, j, ebbg;

    ebbg = (xe - xa + 1) * 2 * (ye - ya + 1);
    for (j = ya; j <= ye; ++j)
        for (i = xa; i <= xe; ++i)
            *(view->p + ebbg + (j - ya) * (xe - xa + 1) + (i - xa)) =
                extbyte[j * MAXSCOL + i];
    return (i);
}

int
e_put_pic_xrect (we_view_t * view)
{
    int i = 0, j;
    int ebbg = (view->e.x - view->a.x + 1) * 2 * (view->e.y - view->a.y + 1);

    for (j = view->a.y; j <= view->e.y; ++j)
        for (i = view->a.x; i <= view->e.x; ++i)
            extbyte[j * MAXSCOL + i] =
                *(view->p + ebbg + (j - view->a.y) * (view->e.x - view->a.x + 1) +
                  (i - view->a.x));
    return (i);
}

int
e_make_xrect_abs (int xa, int ya, int xe, int ye, int sw)
{
    int j;

    for (j = xa; j <= xe; j++)
        *(extbyte + ya * MAXSCOL + j) = *(extbyte + ye * MAXSCOL + j) = 0;
    for (j = ya; j <= ye; j++)
        *(extbyte + j * MAXSCOL + xa) = *(extbyte + j * MAXSCOL + xe) = 0;
    return (e_make_xrect (xa, ya, xe, ye, sw));
}

/**
 * \fn e_make_xrect.
 *
 * This function makes a rectangle withing the functionality of XWindows.
 *
 * \todo TODO: what does this function do? why use the extbyte in stead of global_screen?
 * \todo what is the meaing of sw in this function?
 *
 *
 */
int
e_make_xrect (int xa, int ya, int xe, int ye, int sw)
{
    int j;

    if (sw & 2)
    {
        sw = (sw & 1) ? 16 : 0;
        for (j = xa + 1; j < xe; j++)
        {
            *(extbyte + ya * MAXSCOL + j) |= (sw | 4);
            *(extbyte + ye * MAXSCOL + j) |= (sw | 1);
        }
        for (j = ya + 1; j < ye; j++)
        {
            *(extbyte + j * MAXSCOL + xa) |= (sw | 2);
            *(extbyte + j * MAXSCOL + xe) |= (sw | 8);
        }
    }
    else
    {
        sw = (sw & 1) ? 16 : 0;
        for (j = xa; j <= xe; j++)
        {
            *(extbyte + ya * MAXSCOL + j) |= (sw | 1);
            *(extbyte + ye * MAXSCOL + j) |= (sw | 4);
        }
        for (j = ya; j <= ye; j++)
        {
            *(extbyte + j * MAXSCOL + xa) |= (sw | 8);
            *(extbyte + j * MAXSCOL + xe) |= (sw | 2);
        }
    }
    return (j);
}

static int
e_make_xr_window (int xa, int ya, int xe, int ye, int sw)
{
    if (!sw)
    {
        e_make_xrect (xa, ya, xe, ye, 0);
        e_make_xrect (xa, ya, xe, ye, 2);
    }
    else
    {
        e_make_xrect (xa + 1, ya, xe - 1, ya, 0);
        e_make_xrect (xa + 1, ye, xe - 1, ye, 0);
        e_make_xrect (xa, ya + 1, xa, ye - 1, 0);
        e_make_xrect (xe, ya + 1, xe, ye - 1, 0);
        e_make_xrect (xa, ya, xa, ya, 0);
        e_make_xrect (xe, ya, xe, ya, 0);
        e_make_xrect (xe, ye, xe, ye, 0);
        e_make_xrect (xa, ye, xa, ye, 0);
    }
    return (sw);
}
#endif
