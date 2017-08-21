/** \file we_mouse.c                                       */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <string.h>
#include "keys.h"
#include "messages.h"
#include "options.h"
#include "model.h"
#include "edit.h"
#include "we_prog.h"
#include "we_mouse.h"

#if  MOUSE

#include "utils.h"

int e_mouse_cursor ();

/*   mouse button pressed (?)     */
int
e_mshit ()
{
    extern struct mouse e_mouse;
    int g[4];			/* = { 3, 0, 0, 0 };  */
    g[0] = 3;
    g[1] = 0;

    fk_mouse (g);
    e_mouse.x = g[2] / 8;
    e_mouse.y = g[3] / 8;
    return (g[1]);
}

/*     mouse main menu control */
int
e_m1_mouse ()
{
    extern struct mouse e_mouse;
    extern OPT opt[];
    extern int e_mn_men;
    int c, n;

    if (e_mouse.y == MAXSLNS - 1)
        c = e_m3_mouse ();
    else if (e_mouse.y != 0)
        c = WPE_ESC;
    else
    {
        for (n = 1; n < MENOPT; n++)
            if (e_mouse.x < opt[n].x - e_mn_men)
                break;
        c = opt[n - 1].as;
    }
    return (c);
}

/*   mouse options box control */
int
e_m2_mouse (int xa, int ya, int xe, int ye, OPTK * fopt)
{
    extern struct mouse e_mouse;
    int c;

    if (e_mouse.y == MAXSLNS)
        c = e_m3_mouse ();
    else if (e_mouse.y == 0)
        return (e_m1_mouse ());
    else if (e_mouse.x <= xa || e_mouse.x >= xe || e_mouse.y <= ya ||
             e_mouse.y >= ye)
        c = WPE_ESC;
    else
        c = fopt[e_mouse.y - ya - 1].o;
    while (e_mshit ())
        ;
    return (c);
}

/*      Mouse assistant bar control       */
int
e_m3_mouse ()
{
    extern WOPT *blst;
    extern struct mouse e_mouse;
    extern int nblst;
    int i;

    if (e_mouse.y != MAXSLNS - 1)
        return (WPE_ESC);
    while (e_mshit ())
        ;
    for (i = 1; i < nblst; i++)
        if (e_mouse.x < blst[i].x)
            return (blst[i - 1].as);
    return (blst[nblst - 1].as);
}

/*   mouse errors box control */
int
e_er_mouse (int x, int y, int xx, int yy)
{
    extern struct mouse e_mouse;

    while (e_mshit () != 0)
        ;
    if (y == e_mouse.y && x == e_mouse.x)
        return (WPE_CR);
    if (yy == e_mouse.y && xx - 1 <= e_mouse.x && xx + 4 >= e_mouse.x)
        return (WPE_CR);
    return (0);
}

/*   mouse messages box control */
int
e_msg_mouse (int x, int y, int x1, int x2, int yy)
{
    extern struct mouse e_mouse;

    while (e_mshit () != 0)
        ;
    if (y == e_mouse.y && x == e_mouse.x)
        return (WPE_CR);
    if (yy == e_mouse.y)
    {
        if (x1 - 1 <= e_mouse.x && x1 + 5 >= e_mouse.x)
            return ('Y');
        if (x2 - 1 <= e_mouse.x && x2 + 5 >= e_mouse.x)
            return (WPE_ESC);
        if ((x1 = (x1 + x2) / 2 - 1) <= e_mouse.x && x1 + 5 >= e_mouse.x)
            return ('N');
    }
    return (0);
}

int
e_rahmen_mouse (we_window_t * window)
{
    extern struct mouse e_mouse;
    int c = 1;

    if (e_mouse.x == window->a.x + 3 && e_mouse.y == window->a.y)
        c = WPE_ESC;
    else if (e_mouse.x == window->e.x - 3 && e_mouse.y == window->a.y)
        e_ed_zoom (window);
    else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y)
        e_eck_mouse (window, 1);
    else if (e_mouse.x == window->e.x && e_mouse.y == window->a.y)
        e_eck_mouse (window, 2);
    else if (e_mouse.x == window->e.x && e_mouse.y == window->e.y)
        e_eck_mouse (window, 3);
    else if (e_mouse.x == window->a.x && e_mouse.y == window->e.y)
        e_eck_mouse (window, 4);
    else if (e_mouse.y == window->a.y && e_mouse.x > window->a.x && e_mouse.x < window->e.x)
        e_eck_mouse (window, 0);
    else
        c = 0;
    while (e_mshit () != 0)
        ;
    return (c);
}

int
WpeMngMouseInFileManager (we_window_t * window)
{
    extern struct mouse e_mouse;
    we_control_t *control = window->ed;
    FLBFFR *file_buffer = (FLBFFR *) window->b;
    int i, c = 0, by = 4;

    if (e_mouse.y == 0)
        return (AltBl);
    else if (e_mouse.y == MAXSLNS - 1)
        return (e_m3_mouse ());
    else if (e_mouse.x < window->a.x || e_mouse.x > window->e.x
             || e_mouse.y < window->a.y || e_mouse.y > window->e.y)
    {
        for (i = control->mxedt; i > 0; i--)
        {
            if (e_mouse.x >= control->window[i]->a.x && e_mouse.x <= control->window[i]->e.x
                    && e_mouse.y >= control->window[i]->a.y && e_mouse.y <= control->window[i]->e.y)
            {
                while (e_mshit () != 0);
                return (control->edt[i] <
                        10 ? Alt1 - 1 + control->edt[i] : 1014 + control->edt[i]);
            }
        }
    }
    else if (e_mouse.x == window->a.x + 3 && e_mouse.y == window->a.y)
        c = WPE_ESC;
    else if (e_mouse.x == window->e.x - 3 && e_mouse.y == window->a.y)
        e_ed_zoom (window);
    else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y)
        e_eck_mouse (window, 1);
    else if (e_mouse.x == window->e.x && e_mouse.y == window->a.y)
        e_eck_mouse (window, 2);
    else if (e_mouse.x == window->e.x && e_mouse.y == window->e.y)
        e_eck_mouse (window, 3);
    else if (e_mouse.x == window->a.x && e_mouse.y == window->e.y)
        e_eck_mouse (window, 4);
    else if (e_mouse.y == window->a.y && e_mouse.x > window->a.x && e_mouse.x < window->e.x)
        e_eck_mouse (window, 0);
    else
    {
        if (num_lines_on_screen(window) <= 17)
            by = -1;
        else if (file_buffer->sw != 0 || num_lines_on_screen(window) <= 19)
            by = 2;

        if (num_lines_on_screen(window) > 17)
        {
            if (e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 3
                    && e_mouse.x <= window->a.x + 10)
                c = WPE_ESC;
            else if (e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 13
                     && e_mouse.x <= window->a.x + 24)
                c = AltC;
            else if (file_buffer->sw == 1 && num_cols_on_screen(window) >= 34
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 27
                     && e_mouse.x <= window->a.x + 32)
                c = AltR;
            else if (file_buffer->sw == 2 && num_cols_on_screen(window) >= 35
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 27
                     && e_mouse.x <= window->a.x + 33)
                c = AltW;
            else if (file_buffer->sw == 4 && num_cols_on_screen(window) >= 34
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 27
                     && e_mouse.x <= window->a.x + 32)
                c = AltS;
            else if (file_buffer->sw == 4 && num_cols_on_screen(window) >= 48
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 35
                     && e_mouse.x <= window->a.x + 46)
                c = AltY;
            else if (file_buffer->sw == 3 && num_cols_on_screen(window) >= 37
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 27
                     && e_mouse.x <= window->a.x + 35)
                c = AltE;
            else if (file_buffer->sw == 5 && num_cols_on_screen(window) >= 33
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 27
                     && e_mouse.x <= window->a.x + 31)
                c = AltA;
            else if (file_buffer->sw == 0 && num_cols_on_screen(window) >= 35
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 27
                     && e_mouse.x <= window->a.x + 33)
                c = AltK;
            else if (file_buffer->sw == 0 && num_cols_on_screen(window) >= 49
                     && e_mouse.y == window->e.y - by && e_mouse.x >= window->a.x + 36
                     && e_mouse.x <= window->a.x + 47)
                c = AltA;
        }
        if (file_buffer->sw == 0 && num_lines_on_screen(window) > 19)
        {
            if (e_mouse.y == window->e.y - 2 && e_mouse.x >= window->a.x + 3
                    && e_mouse.x <= window->a.x + 8)
                c = AltM;
            else if (e_mouse.y == window->e.y - 2 && num_cols_on_screen(window) >= 21 &&
                     e_mouse.x >= window->a.x + 12 && e_mouse.x <= window->a.x + 19)
                c = AltR;
            else if (e_mouse.y == window->e.y - 2 && num_cols_on_screen(window) >= 30 &&
                     e_mouse.x >= window->a.x + 23 && e_mouse.x <= window->a.x + 28)
                c = AltL;
            else if (e_mouse.y == window->e.y - 2 && num_cols_on_screen(window) >= 39 &&
                     e_mouse.x >= window->a.x + 32 && e_mouse.x <= window->a.x + 37)
                c = AltO;
            else if (e_mouse.y == window->e.y - 2 && num_cols_on_screen(window) >= 48 &&
                     e_mouse.x >= window->a.x + 41 && e_mouse.x <= window->a.x + 46)
                c = AltE;
        }

        if (e_mouse.y == window->a.y + 3 && e_mouse.x >= window->a.x + file_buffer->xfa
                && e_mouse.x <= window->a.x + file_buffer->xfa + file_buffer->xfd)
            c = AltN;
        else if (e_mouse.y == window->a.y + 3 && e_mouse.x >= window->a.x + file_buffer->xda
                 && e_mouse.x <= window->a.x + file_buffer->xda + file_buffer->xdd)
            c = AltD;
        else if (e_mouse.y >= file_buffer->fw->ya && e_mouse.y <= file_buffer->fw->ye
                 && e_mouse.x >= file_buffer->fw->xa && e_mouse.x <= file_buffer->fw->xe)
            c = AltF;
        else if (e_mouse.y >= file_buffer->dw->ya && e_mouse.y <= file_buffer->dw->ye
                 && e_mouse.x >= file_buffer->dw->xa && e_mouse.x <= file_buffer->dw->xe)
            c = AltT;
    }
    while (e_mshit () != 0);
    return (c);
}

int
WpeMouseInFileDirList (int k, int sw, we_window_t * window)
{
    extern struct mouse e_mouse;
    we_control_t *control = window->ed;
    FLBFFR *file_buffer = (FLBFFR *) window->b;
    int i;
    char tmp[256];

    if (e_mouse.x >= window->a.x && e_mouse.x <= window->e.x
            && e_mouse.y >= window->a.y && e_mouse.y <= window->e.y)
        return (0);
    for (i = control->mxedt - 1; i > 0; i--)
    {
        if (e_mouse.x >= control->window[i]->a.x && e_mouse.x <= control->window[i]->e.x
                && e_mouse.y >= control->window[i]->a.y && e_mouse.y <= control->window[i]->e.y)
            break;
    }
    if (i <= 0)
        return (0);
    if (sw)
    {
        if (control->window[i]->dirct[strlen (control->window[i]->dirct) - 1] == DIRC)
            sprintf (tmp, "%s%s", control->window[i]->dirct,
                     file_buffer->dd->name[file_buffer->dw->nf - file_buffer->cd->nr_files]);
        else
            sprintf (tmp, "%s/%s", control->window[i]->dirct,
                     file_buffer->dd->name[file_buffer->dw->nf - file_buffer->cd->nr_files]);
        if (k == -2)
            e_copy (file_buffer->dd->name[file_buffer->dw->nf - file_buffer->cd->nr_files], tmp, window);
        else if (k == -4)
            e_link (file_buffer->dd->name[file_buffer->dw->nf - file_buffer->cd->nr_files], tmp, window);
        else
            e_rename (file_buffer->dd->name[file_buffer->dw->nf - file_buffer->cd->nr_files], tmp, window);
        freedf (file_buffer->cd);
        freedf (file_buffer->dw->df);
        freedf (file_buffer->dd);
        file_buffer->dd = e_find_dir (SUDIR, window->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
        file_buffer->cd = WpeCreateWorkingDirTree (window->save, control);	/* ??? control */
        file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);
        file_buffer->dw->nf = file_buffer->cd->nr_files - 1;
        file_buffer->dw->ia = file_buffer->dw->ja = 0;
        e_pr_file_window (file_buffer->dw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color,
                          window->colorset->frft.fg_bg_color);
    }
    else
    {
        if (control->window[i]->dirct[strlen (control->window[i]->dirct) - 1] == DIRC)
            sprintf (tmp, "%s%s", control->window[i]->dirct, file_buffer->df->name[file_buffer->fw->nf]);
        else
            sprintf (tmp, "%s/%s", control->window[i]->dirct, file_buffer->df->name[file_buffer->fw->nf]);
        if (k == -2)
            e_copy (file_buffer->df->name[file_buffer->fw->nf], tmp, window);
        else if (k == -4)
            e_link (file_buffer->df->name[file_buffer->fw->nf], tmp, window);
        else
            e_rename (file_buffer->df->name[file_buffer->fw->nf], tmp, window);
        freedf (file_buffer->df);
        freedf (file_buffer->fw->df);
        file_buffer->df =
            e_find_files (file_buffer->rdfile, window->ed->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
        file_buffer->fw->df = WpeGraphicalFileList (file_buffer->df, window->ed->flopt >> 9, control);
        file_buffer->fw->ia = file_buffer->fw->nf = 0;
        file_buffer->fw->ja = file_buffer->fw->srcha;
        e_pr_file_window (file_buffer->fw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color,
                          window->colorset->frft.fg_bg_color);
    }
    return (control->edt[i] < 10 ? Alt1 - 1 + control->edt[i] : 1014 + control->edt[i]);
}


/*  File Window  */
char *
e_gt_btstr (int x, int y, int n, char *buffer)
{
    int i;

    for (i = 0; i < n; i++)
        buffer[i] = e_gt_byte (2 * x + i, y);
    return (buffer);
}

char *
e_pt_btstr (int x, int y, int n, char *buffer)
{
    int i;

    for (i = 0; i < n; i++)
        e_pt_byte (2 * x + i, y, buffer[i]);
    return (buffer);
}

int
fl_wnd_mouse (sw, k, fw)
int sw;
int k;
FLWND *fw;
{
    extern struct mouse e_mouse;
    int MLEN, i, c, xa, ya, xn, yn, xdif;
    char *file, *bgrd;
    if (e_mouse.x == fw->xe && e_mouse.y >= fw->ya && e_mouse.y < fw->ye)
    {
        fw->nf = e_lst_mouse (fw->xe, fw->ya, fw->ye - fw->ya, 0,
                              fw->df->nr_files, fw->nf);
        return (0);
    }
    else if (e_mouse.y == fw->ye && e_mouse.x >= fw->xa && e_mouse.x < fw->xe)
    {
        fw->ja = e_lst_mouse (fw->xa, fw->ye, fw->xe - fw->xa, 1,
                              strlen (*(fw->df->name + fw->nf)), fw->ja);
        return (0);
    }
    else if (e_mouse.y >= fw->ya && e_mouse.y < fw->ye &&
             e_mouse.x >= fw->xa && e_mouse.x < fw->xe)
    {
        if (fw->nf == e_mouse.y - fw->ya + fw->ia
                && e_mouse.y - fw->ya + fw->ia < fw->df->nr_files)
        {
            /*  if(k == -2) {  while (e_mshit() != 0);  return(AltU);  }
               else if(k == -4) {  while (e_mshit() != 0);  return(AltM);  }
               else   */
            {
                xa = e_mouse.x;
                ya = e_mouse.y;
                xdif = e_mouse.x - fw->xa;
                if (fw->srcha >= 0)
                    c = fw->srcha;
                else
                {
                    for (c = 0; *(fw->df->name[fw->nf] + c)
                            && (*(fw->df->name[fw->nf] + c) <= 32
                                || *(fw->df->name[fw->nf] + c) >= 127); c++)
                    {
                        ;
                    }
                    if (!WpeIsXwin ())
                        c += 3;
                }
                for (MLEN = 1; *(fw->df->name[fw->nf] + c + MLEN)
                        && *(fw->df->name[fw->nf] + c + MLEN) != ' '; MLEN++);
                MLEN *= 2;
                file = malloc (MLEN * sizeof (char));
                bgrd = malloc (MLEN * sizeof (char));
                while (e_mshit () != 0)
                    if (sw && (e_mouse.x != xa || e_mouse.y != ya))
                    {
                        xn = e_mouse.x;
                        yn = e_mouse.y;
                        if (fw->srcha < 0)
                        {
                            FLBFFR *file_buffer = (FLBFFR *) fw->window->b;
                            if (file_buffer->cd->nr_files > fw->nf)
                            {
                                while (e_mshit () != 0)
                                {
                                    ;
                                }
                                free (file);
                                free (bgrd);
                                return (WPE_CR);
                            }
                            for (c = 0; *(fw->df->name[fw->nf] + c)
                                    && (*(fw->df->name[fw->nf] + c) <= 32
                                        || *(fw->df->name[fw->nf] + c) >= 127); c++);
                            if (!WpeIsXwin ())
                                c += 3;
                            xdif -= (c + 1);
                        }
                        for (i = 0; i < MLEN / 2; i++)
                        {
                            file[2 * i] = *(fw->df->name[fw->nf] + c + i);
                            file[2 * i + 1] = fw->window->colorset->fz.fg_bg_color;
                        }
                        e_gt_btstr (e_mouse.x - xdif, e_mouse.y, MLEN, bgrd);
                        while (e_mshit () != 0)
                        {
                            e_pt_btstr (xn - xdif, yn, MLEN, bgrd);
                            xn = e_mouse.x;
                            yn = e_mouse.y;
                            e_gt_btstr (e_mouse.x - xdif, e_mouse.y, MLEN, bgrd);
                            e_pt_btstr (e_mouse.x - xdif, e_mouse.y, MLEN, file);
                            e_refresh ();
                        }
                        e_pt_btstr (xn - xdif, yn, MLEN, bgrd);
                        free (file);
                        free (bgrd);
                        return (k);
                    }
                free (file);
                free (bgrd);
                if (sw && k == -2)
                    return (AltU);
                else if (sw && k == -4)
                    return (AltM);
                else
                    return (WPE_CR);
            }
        }
        else
        {
            while (e_mshit () != 0)
            {
                ;
            }
            fw->nf = e_mouse.y - fw->ya + fw->ia;
            return (0);
        }
    }
    else
        return (MBKEY);
}

/*   mouse slider bar control */
int
e_lst_mouse (x, y, n, sw, max, nf)
int x;
int y;
int n;
int sw;
int max;
int nf;
{
    extern struct mouse e_mouse;
    int g[4];			/*  = { 1, 0, 0, 0 };  */
    int inew, iold, nret, frb = e_gt_col (x, y);
    double d;
    if (n < 2)
        return (nf);
    d = ((double) max) / ((double) (n - 2));
    g[0] = 1;
    if (sw == 0)
    {
        if (e_mouse.x != x)
            return (nf);
        if (e_mouse.y < y || e_mouse.y >= y + n)
            return (nf);
        if (e_mouse.y == y)
            nret = (nf < 1) ? nf : nf - 1;
        else if (e_mouse.y == y + n - 1)
            nret = (nf >= max - 1) ? nf : nf + 1;
        else
        {
            nret = (int) ((e_mouse.y - y - 1) * d);
            if (e_gt_char (e_mouse.x, e_mouse.y) == MCA)
            {
                iold = e_mouse.y;
#ifdef NEWSTYLE
                e_make_xrect_abs (x, iold, x, iold, 1);
                e_refresh ();
#endif
                fk_mouse (g);
                for (g[1] = 1; g[1] != 0; fk_mouse (g), g[0] = 3)
                {
                    if ((inew = g[3] / 8) < y + 1)
                        inew = y + 1;
                    if (inew > y + n - 2)
                        inew = y + n - 2;
                    if (iold != inew)
                    {
                        g[0] = 2;
                        fk_mouse (g);
                        e_pr_char (x, iold, MCI, frb);
                        e_pr_char (x, inew, MCA, frb);
#ifdef NEWSTYLE
                        e_make_xrect (x, y + 1, x, y + n - 2, 0);
                        e_make_xrect_abs (x, inew, x, inew, 1);
#endif
                        e_refresh ();
                        iold = inew;
                        g[0] = 1;
                        fk_mouse (g);
                    };
                }
                g[0] = 2;
                fk_mouse (g);
                e_pr_char (x, inew, MCI, frb);
                if (inew - y < 2)
                    nret = 0;
                else if (inew - y > n - 3)
                    nret = max - 1;
                else
                    nret = (int) ((inew - y - 0.5) * d);
            }
            else if (nf < nret)
            {
                nret = (nf + n > max) ? max : nf + n - 1;
                while (e_mshit () != 0);
            }
            else
            {
                nret = (nf - n < 0) ? 0 : nf - n + 1;
                while (e_mshit () != 0);
            }
        }
    }
    else
    {
        if (e_mouse.y != y)
            return (nf);
        if (e_mouse.x < x || e_mouse.x >= x + n)
            return (nf);
        if (e_mouse.x == x)
            nret = (nf < 1) ? nf : nf - 1;
        else if (e_mouse.x == x + n - 1)
            nret = (nf >= max - 1) ? nf : nf + 1;
        else
        {
            nret = (int) ((e_mouse.x - x - 1) * d);
            if (e_gt_char (e_mouse.x, e_mouse.y) == MCA)
            {
                iold = e_mouse.x;
#ifdef NEWSTYLE
                e_make_xrect_abs (iold, y, iold, y, 1);
                e_refresh ();
#endif
                fk_mouse (g);
                for (g[1] = 1; g[1] != 0; fk_mouse (g), g[0] = 3)
                {
                    if ((inew = g[2] / 8) < x + 1)
                        inew = x + 1;
                    if (inew > x + n - 2)
                        inew = x + n - 2;
                    if (iold != inew)
                    {
                        g[0] = 2;
                        fk_mouse (g);
                        e_pr_char (iold, y, MCI, frb);
                        e_pr_char (inew, y, MCA, frb);
#ifdef NEWSTYLE
                        e_make_xrect (x + 1, y, x + n - 2, y, 0);
                        e_make_xrect_abs (inew, y, inew, y, 1);
#endif
                        e_refresh ();
                        iold = inew;
                        g[0] = 1;
                        fk_mouse (g);
                    };
                }
                g[0] = 2;
                fk_mouse (g);
                e_pr_char (inew, y, MCI, frb);
                if (inew - y < 2)
                    nret = 0;
                else if (inew - y > n - 3)
                    nret = max - 1;
                else
                    nret = (int) ((inew - y - 0.5) * d);
            }
            else if (nf < nret)
            {
                nret = (nf + n > max) ? max : nf + n - 1;
                while (e_mshit () != 0);
            }
            else
            {
                nret = (nf - n < 0) ? 0 : nf - n + 1;
                while (e_mshit () != 0);
            }
        }
    }
    return (nret);
}

/*   mouse window resizer control */
void
e_eck_mouse (we_window_t * window, int sw)
{
    int g[4];			/*  = { 3, 1, 0, 0 };  */
    int xold, yold, x, y, xa, xmin = 26, ymin = 3;
    we_point_t fa, fe;

    e_ed_rahmen (window, 0);
    memcpy (&fa, &window->a, sizeof (we_point_t));
    memcpy (&fe, &window->e, sizeof (we_point_t));
    g[0] = 3;
    g[1] = 1;
    fk_mouse (g);
    xold = g[2] / 8;
    yold = g[3] / 8;
    xa = xold - window->a.x;
    if (window->dtmd == DTMD_FILEDROPDOWN)
        xmin = 15;
    else if (!DTMD_ISTEXT (window->dtmd))
        ymin = 9;
    while (g[1] != 0)
    {
        x = g[2] / 8;
        y = g[3] / 8;
        if (y < 1)
            y = 1;
        else if (y > MAXSLNS - 2)
            y = MAXSLNS - 2;
        if (x < 0)
            x = 0;
        else if (x > MAXSCOL - 1)
            x = MAXSCOL - 1;
        if (xold != x || yold != y)
        {
            xold = x;
            yold = y;
            if (sw == 0)
            {
                x -= xa;
                if (x < 0)
                    x = 0;
                else if (x + num_cols_on_screen(window) > MAXSCOL - 1)
                    x = MAXSCOL - window->e.x + window->a.x - 1;
                if (window->e.y + y - window->a.y > MAXSLNS - 2)
                    y = MAXSLNS - window->e.y + window->a.y - 2;
                window->e.x = num_cols_on_screen(window) + x;
                window->a.x = x;
                window->e.y = window->e.y + y - window->a.y;
                window->a.y = y;
            }
            else if (sw == 1)
            {
                if (x > window->e.x - xmin)
                    x = window->e.x - xmin;
                if (y > window->e.y - ymin)
                    y = window->e.y - ymin;
                window->a.x = x;
                window->a.y = y;
            }
            else if (sw == 2)
            {
                if (x < window->a.x + xmin)
                    x = window->a.x + xmin;
                if (y > window->e.y - ymin)
                    y = window->e.y - ymin;
                window->e.x = x;
                window->a.y = y;
            }
            else if (sw == 3)
            {
                if (x < window->a.x + xmin)
                    x = window->a.x + xmin;
                if (y < window->a.y + ymin)
                    y = window->a.y + ymin;
                window->e.x = x;
                window->e.y = y;
            }
            else if (sw == 4)
            {
                if (x > window->e.x - xmin)
                    x = window->e.x - xmin;
                if (y < window->a.y + ymin)
                    y = window->a.y + ymin;
                window->a.x = x;
                window->e.y = y;
            }
            g[0] = 2;
            fk_mouse (g);
            window->view = e_ed_kst (window, window->view, 0);
            if (window->view == NULL)
                e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
            if (window->dtmd == DTMD_FILEDROPDOWN)
            {
                FLWND *fw = (FLWND *) window->b;
                fw->xa = window->a.x + 1;
                fw->xe = window->e.x;
                fw->ya = window->a.y + 1;
                fw->ye = window->e.y;
            }
            g[0] = 1;
            fk_mouse (g);
            e_cursor (window, 0);
            e_schirm (window, 0);
            e_refresh ();
        }
        g[0] = 3;
        fk_mouse (g);
    }
    if ((memcmp (&fa, &window->a, sizeof (we_point_t))) ||
            (memcmp (&fe, &window->e, sizeof (we_point_t))))
        window->zoom = 0;
    e_ed_rahmen (window, 1);
}

/*       Mouse edit window control   */
int
e_edt_mouse (int c, we_window_t * window)
{
    int i, ret = 0;
    extern struct mouse e_mouse;
    we_control_t *control = window->ed;

    if (e_mouse.y == 0)
        return (e_m1_mouse ());
    else if (e_mouse.y == MAXSLNS - 1)
        return (e_m3_mouse ());
    else if (e_mouse.x >= window->a.x && e_mouse.x <= window->e.x &&
             e_mouse.y >= window->a.y && e_mouse.y <= window->e.y)
    {
        if (e_mouse.x == window->a.x + 3 && e_mouse.y == window->a.y)
            ret = window->ed->edopt & ED_CUA_STYLE ? CF4 : AF3;
        else if (e_mouse.x == window->e.x - 3 && e_mouse.y == window->a.y)
            e_ed_zoom (window);
        else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y)
            e_eck_mouse (window, 1);
        else if (e_mouse.x == window->e.x && e_mouse.y == window->a.y)
            e_eck_mouse (window, 2);
        else if (e_mouse.x == window->e.x && e_mouse.y == window->e.y)
            e_eck_mouse (window, 3);
        else if (e_mouse.x == window->a.x && e_mouse.y == window->e.y)
            e_eck_mouse (window, 4);
        else if (e_mouse.y == window->a.y)
            e_eck_mouse (window, 0);
        else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y + 2)
            ret = window->ed->edopt & ED_CUA_STYLE ? AF3 : F4;
        else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y + 4)
            ret = window->ed->edopt & ED_CUA_STYLE ? CF3 : AF4;
        else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y + 6)
            ret = window->ed->edopt & ED_CUA_STYLE ? F3 : CF4;
        else if (window->ins != 8 && e_mouse.x == window->a.x && e_mouse.y == window->a.y + 8)
            ret = window->ed->edopt & ED_CUA_STYLE ? AF2 : F2;
        else if (e_mouse.y == window->e.y && e_mouse.x > window->a.x + 4 &&
                 e_mouse.x < window->a.x + 14)
            ret = AltG;
        else if (e_mouse.y == window->e.y && e_mouse.x == window->a.x + 15 && window->ins != 8)
        {
            if (window->ins & 1)
                window->ins &= ~1;
            else
                window->ins |= 1;
            e_pr_filetype (window);
        }
        else if (e_mouse.y == window->e.y && e_mouse.x == window->a.x + 16 && window->ins != 8)
        {
            if (window->ins & 2)
                window->ins &= ~2;
            else
                window->ins |= 2;
            e_pr_filetype (window);
        }
        else if (e_mouse.x > window->a.x && e_mouse.x < window->e.x &&
                 e_mouse.y > window->a.y && e_mouse.y < window->e.y)
        {
            if (c < -1)
                ret = e_ccp_mouse (c, window);
            else if (window->dtmd == DTMD_HELP && window->ins == 8 &&
                     window->b->cursor.y ==
                     e_mouse.y - window->a.y + num_lines_off_screen_top(window) - 1
                     && ((i = window->b->cursor.x) == e_mouse_cursor (window->b, window->s, window)))
                ret = WPE_CR;
            else
#ifdef PROG
#ifdef DEBUGGER
            {
                if (WpeIsProg () && (!strcmp (window->datnam, "Watches") ||
                                     !strcmp (window->datnam, "Messages") ||
                                     !strcmp (window->datnam, "Stack")))
                    ret = e_d_car_mouse (window);
                else
                    e_cur_mouse (window);
            }
#else
            {
                if (WpeIsProg () && !strcmp (window->datnam, "Messages"))
                    ret = e_d_car_mouse (window);
                else
                    e_cur_mouse (window);
            }
#endif
#else
                e_cur_mouse (window);
#endif
        }
        else if (e_mouse.x == window->e.x && e_mouse.y == window->a.y + 1)
        {
            /*changed the while()... to a do...while();  :Mark L */
            do
            {
                window->b->cursor.y = window->b->cursor.y > 0 ? window->b->cursor.y - 1 : 0;
                window->b->cursor.x = e_chr_sp (window->b->clsv, window->b, window);
                e_cursor (window, 1);
                e_refresh ();
            }
            while (e_mshit ());
        }
        else if (e_mouse.x == window->e.x && e_mouse.y == window->e.y - 1)
        {
            /*changed the while()... to a do...while();  :Mark L */
            do
            {
                window->b->cursor.y = window->b->cursor.y < window->b->mxlines - 1 ?
                                      window->b->cursor.y + 1 : window->b->mxlines - 1;
                window->b->cursor.x = e_chr_sp (window->b->clsv, window->b, window);
                e_cursor (window, 1);
                e_refresh ();
            }
            while (e_mshit ());
        }
        else if (e_mouse.x == window->e.x &&
                 e_mouse.y > window->a.y + 1 && e_mouse.y < window->e.y - 1)
        {
            window->b->cursor.y = e_lst_mouse (window->e.x, window->a.y + 1, window->e.y - window->a.y - 1, 0,
                                               window->b->mxlines, window->b->cursor.y);
            e_cursor (window, 1);
            e_refresh ();
        }
        else if (e_mouse.y == window->e.y && e_mouse.x == window->a.x + 19)
        {
            while (e_mshit ())
            {
                window->b->cursor.x = window->b->cursor.x > 0 ? window->b->cursor.x - 1 : 0;
                e_cursor (window, 1);
                e_refresh ();
            }
        }
        else if (e_mouse.y == window->e.y && e_mouse.x == window->e.x - 2)
        {
            while (e_mshit ())
            {
                window->b->cursor.x = window->b->cursor.x < window->b->buflines[window->b->cursor.y].len ?
                                      window->b->cursor.x + 1 : window->b->buflines[window->b->cursor.y].len;
                e_cursor (window, 1);
                e_refresh ();
            }
        }
        else if (e_mouse.y == window->e.y &&
                 e_mouse.x > window->a.x + 19 && e_mouse.x < window->e.x - 2)
        {
            window->b->cursor.x =
                e_lst_mouse (window->a.x + 19, window->e.y, window->e.x - window->a.x - 20, 1,
                             window->b->mx.x, num_cols_off_screen_left(window));
            e_cursor (window, 1);
            e_refresh ();
        }
    }
    else
    {
        for (i = control->mxedt; i > 0; i--)
        {
            if (e_mouse.x >= control->window[i]->a.x && e_mouse.x <= control->window[i]->e.x &&
                    e_mouse.y >= control->window[i]->a.y && e_mouse.y <= control->window[i]->e.y)
            {
                ret =
                    control->edt[i] < 10 ? Alt1 - 1 + control->edt[i] : 1014 + control->edt[i];
                break;
            }
        }
    }
    while (e_mshit () != 0)
        ;
    return (ret);
}

int
e_mouse_cursor (we_buffer_t * b, we_screen_t * s, we_window_t * window)
{
    extern struct mouse e_mouse;

    b->cursor.x = e_mouse.x - window->a.x + s->c.x - 1;
    b->cursor.y = e_mouse.y - window->a.y + s->c.y - 1;
    if (b->cursor.y < 0)
        b->cursor.y = 0;
    else if (b->cursor.y >= b->mxlines)
        b->cursor.y = b->mxlines - 1;
    return (b->cursor.x = e_chr_sp (b->cursor.x, b, window));
}

/*  Copy, Cut and Paste functions    */
int
e_ccp_mouse (int c, we_window_t * window)
{
    we_buffer_t *b = window->ed->window[window->ed->mxedt]->b;
    we_screen_t *s = window->ed->window[window->ed->mxedt]->s;

    while (e_mshit () != 0)
        ;
    if (c == -2)
    {
        e_mouse_cursor (b, s, window);
        return ((bioskey () & 8) ? AltEin : ShiftEin);
    }
    else if (c == -4)
    {
        return ((bioskey () & 3) ? ShiftDel
                : ((bioskey () & 8) ? AltDel : CEINFG));
    }
    else
        return (0);
}

/*       Mouse cursor in edit window control   */
void
e_cur_mouse (window)
we_window_t *window;
{
    we_buffer_t *b = window->ed->window[window->ed->mxedt]->b;
    we_screen_t *s = window->ed->window[window->ed->mxedt]->s;
    we_point_t bs;
    bs.x = b->cursor.x;
    bs.y = b->cursor.y;
    e_mouse_cursor (b, s, window);
    if ((bioskey () & 3) == 0)
    {
        if (b->cursor.x == bs.x && b->cursor.y == bs.y && window->dtmd != DTMD_HELP)
        {
            if (s->mark_begin.y == b->cursor.y && s->mark_end.y == b->cursor.y
                    && s->mark_begin.x <= b->cursor.x && s->mark_end.x > b->cursor.x)
            {
                s->mark_begin.x = 0;
                s->mark_end.x = b->buflines[b->cursor.y].len;
            }
            else
            {
                s->mark_begin.y = s->mark_end.y = b->cursor.y;
                for (s->mark_begin.x = b->cursor.x; s->mark_begin.x > 0
                        && isalnum1 (b->buflines[b->cursor.y].s[s->mark_begin.x - 1]);
                        s->mark_begin.x--);
                for (s->mark_end.x = b->cursor.x;
                        s->mark_end.x < b->buflines[b->cursor.y].len
                        && isalnum1 (b->buflines[b->cursor.y].s[s->mark_end.x]);
                        s->mark_end.x++);
            }
            e_schirm (window, 1);
        }
        s->ks.x = b->cursor.x;
        s->ks.y = b->cursor.y;
    }
    else
    {
        if (s->mark_end.y < b->cursor.y
                || (s->mark_end.y == b->cursor.y && s->mark_end.x <= b->cursor.x))
        {
            s->mark_end.x = b->cursor.x;
            s->mark_end.y = b->cursor.y;
            s->ks.x = s->mark_begin.x;
            s->ks.y = s->mark_begin.y;
        }
        else if (s->mark_begin.y > b->cursor.y
                 || (s->mark_begin.y == b->cursor.y && s->mark_begin.x >= b->cursor.x))
        {
            s->mark_begin.x = b->cursor.x;
            s->mark_begin.y = b->cursor.y;
            s->ks.x = s->mark_end.x;
            s->ks.y = s->mark_end.y;
        }
        else if (s->mark_end.y < bs.y
                 || (s->mark_end.y == bs.y && s->mark_end.x <= bs.x))
        {
            s->mark_begin.x = b->cursor.x;
            s->mark_begin.y = b->cursor.y;
            s->ks.x = s->mark_end.x;
            s->ks.y = s->mark_end.y;
        }
        else
        {
            s->mark_end.x = b->cursor.x;
            s->mark_end.y = b->cursor.y;
            s->ks.x = s->mark_begin.x;
            s->ks.y = s->mark_begin.y;
        }
    }
    WpeMouseChangeShape (WpeSelectionShape);
    while (e_mshit () != 0)
    {
        bs.x = b->cursor.x;
        bs.y = b->cursor.y;
        e_mouse_cursor (b, s, window);
        if (b->cursor.x < 0)
            b->cursor.x = 0;
        else if (b->cursor.x > b->buflines[b->cursor.y].len)
            b->cursor.x = b->buflines[b->cursor.y].len;
        if (b->cursor.x != bs.x || b->cursor.y != bs.y)
        {
            if (s->ks.y < b->cursor.y || (s->ks.y == b->cursor.y && s->ks.x <= b->cursor.x))
            {
                s->mark_end.x = b->cursor.x;
                s->mark_end.y = b->cursor.y;
                s->mark_begin.x = s->ks.x;
                s->mark_begin.y = s->ks.y;
            }
            else
            {
                s->mark_begin.x = b->cursor.x;
                s->mark_begin.y = b->cursor.y;
                s->mark_end.x = s->ks.x;
                s->mark_end.y = s->ks.y;
            }
        }
        e_cursor (window, 1);
        e_schirm (window, 1);
        e_refresh ();
    }
    s->ks.x = b->cursor.x;
    s->ks.y = b->cursor.y;
    WpeMouseRestoreShape ();
    e_cursor (window, 1);
}

int
e_opt_ck_mouse (xa, ya, md)
int xa;
int ya;
int md;
{
    UNUSED (md);
    extern struct mouse e_mouse;
    if (e_mouse.x < xa - 2 || e_mouse.x > xa + 25
            || e_mouse.y < ya - 1 || e_mouse.y > ya + 18)
        return (WPE_CR);
    if (e_mouse.x >= xa && e_mouse.x < xa + 24
            && e_mouse.y > ya && e_mouse.y < ya + 17)
        return (1000 + (e_mouse.y - ya - 1) * 16 + (e_mouse.x - xa) / 3);
    else
        return (0);
}

int
e_opt_cw_mouse (xa, ya, md)
int xa;
int ya;
int md;
{
    extern struct mouse e_mouse;
    if (e_mouse.y == 0 || e_mouse.y == MAXSLNS - 1)
        return (WPE_ESC);
    if (e_mouse.y == 1 && e_mouse.x == 3)
        return (WPE_ESC);
    if (e_mouse.x >= xa - 30 && e_mouse.x <= xa - 3
            && e_mouse.y >= ya && e_mouse.y <= ya + 19)
        return (WPE_CR);
    if (e_mouse.x >= 1 && e_mouse.x <= 33 && e_mouse.y >= 2 && e_mouse.y <= 21)
    {
        if (md == 1)
            return (e_opt_bs_mouse_1 ());
        else if (md == 2)
            return (e_opt_bs_mouse_2 ());
        else if (md == 3)
            return (e_opt_bs_mouse_3 ());
        else
            return (e_opt_bs_mouse ());
    }
    if (e_mouse.x > xa && e_mouse.x < xa + 12
            && e_mouse.y > ya && e_mouse.y < ya + 20)
        return (374 + e_mouse.y - ya);
    else
        return (0);
}

int
e_opt_bs_mouse_1 ()
{
    extern struct mouse e_mouse;	/*  return = sw + 375;  */
    int sw = 0;
    if (e_mouse.y < 2 || e_mouse.y > 21 || e_mouse.x < 2 || e_mouse.x > 33)
        return (0);
    else if (e_mouse.y == 2 && e_mouse.x == 6)
        sw = 1;
    else if (e_mouse.y == 2 && e_mouse.x >= 17 && e_mouse.x <= 28)
        sw = 3;
    else if (e_mouse.y == 21 && e_mouse.x >= 5 && e_mouse.x <= 10)
        sw = 1;
    else if (e_mouse.y == 2 || e_mouse.y == 21)
        sw = 2;
    else if ((e_mouse.y == 3 || e_mouse.y == 5)
             && e_mouse.x >= 18 && e_mouse.x <= 27)
        sw = 0;
    else if (e_mouse.y == 4 && (e_mouse.x == 18 || e_mouse.x == 27))
        sw = 0;
    else if (e_mouse.y == 4 && e_mouse.x == 20)
        sw = 1;
    else if (e_mouse.y == 4 && e_mouse.x > 18 && e_mouse.x < 27)
        sw = 2;
    else
        sw = 4;
    return (sw + 375);
}

int
e_opt_bs_mouse_2 ()
{
    extern struct mouse e_mouse;	/*  return = sw + 375;  */
    int sw = 0;
    if (e_mouse.y < 2 || e_mouse.y > 21 || e_mouse.x < 1 || e_mouse.x > 32)
        return (0);
    else if (e_mouse.y == 2 && e_mouse.x == 4)
        sw = 1;
    else if (e_mouse.y == 2 || e_mouse.y == 21 ||
             e_mouse.x == 1 || e_mouse.x == 32)
        sw = 0;
    else if (e_mouse.y == 4 && e_mouse.x == 5)
        sw = 3;
    else if (e_mouse.y == 5 && e_mouse.x >= 5 && e_mouse.x <= 24)
        sw = 5;
    else if (e_mouse.y == 7 && e_mouse.x == 5)
        sw = 3;
    else if (e_mouse.y == 8 && e_mouse.x >= 5 && e_mouse.x <= 24)
        sw = 4;
    else if (e_mouse.y == 10 && e_mouse.x == 5)
        sw = 3;
    else if (e_mouse.y == 11 && e_mouse.x >= 5 && e_mouse.x <= 20)
        sw = 7;
    else if (e_mouse.y == 12 && e_mouse.x >= 5 && e_mouse.x <= 20)
        sw = 8;
    else if (e_mouse.y == 13 && e_mouse.x >= 5 && e_mouse.x <= 20)
        sw = 6;
    else if (e_mouse.y == 15 && e_mouse.x == 5)
        sw = 3;
    else if (e_mouse.y == 16 && e_mouse.x >= 5 && e_mouse.x <= 24)
        sw = 11;
    else if (e_mouse.y == 17 && e_mouse.x == 10)
        sw = 10;
    else if (e_mouse.y == 17 && e_mouse.x >= 5 && e_mouse.x <= 24)
        sw = 9;
    else if (e_mouse.y == 19 && e_mouse.x == 7)
        sw = 13;
    else if (e_mouse.y == 19 && e_mouse.x >= 6 && e_mouse.x <= 13)
        sw = 12;
    else if (e_mouse.y == 19 && e_mouse.x >= 19 && e_mouse.x <= 26)
        sw = 14;
    else
        sw = 2;
    return (sw + 375);
}

int
e_opt_bs_mouse_3 ()
{
    extern struct mouse e_mouse;	/*  return = sw + 375;  */
    int sw = 0;
    if (e_mouse.y < 2 || e_mouse.y > 21 || e_mouse.x < 1 || e_mouse.x > 32)
        return (0);
    else if (e_mouse.y == 5)
        sw = 3;
    else if (e_mouse.y == 9)
        sw = 1;
    else if (e_mouse.y == 11)
        sw = 2;
    else if (e_mouse.y == 13)
        sw = 4;
    return (sw + 375);
}

int
e_opt_bs_mouse ()
{
    extern struct mouse e_mouse;
    int sw = 0;
    if (e_mouse.y < 2 || e_mouse.y > 21 || e_mouse.x < 1 || e_mouse.x > 32)
        return (0);
    else if (e_mouse.y == 2 && (e_mouse.x == 4 || e_mouse.x == 29))
        sw = 1;
    else if (e_mouse.x == 32 && e_mouse.y > 2 && e_mouse.y < 21)
        sw = 5;
    else if (e_mouse.y == 21 && e_mouse.x > 20 && e_mouse.y < 32)
        sw = 5;
    else if (e_mouse.y == 2 || e_mouse.y == 21 ||
             e_mouse.x == 1 || e_mouse.x == 32)
        sw = 0;
    else if (e_mouse.y == 6 && e_mouse.x >= 4 && e_mouse.x <= 30)
        sw = 3;
    else if (e_mouse.y == 7 && e_mouse.x >= 14 && e_mouse.x <= 21)
        sw = 4;
    else if (e_mouse.y == 10 && e_mouse.x >= 4 && e_mouse.x <= 16)
        sw = 6;
    else if (e_mouse.y == 11 && e_mouse.x >= 15 && e_mouse.x <= 20)
        sw = 8;
    else if (e_mouse.y == 13 && e_mouse.x >= 4 && e_mouse.x <= 17)
        sw = 7;
    else if (e_mouse.y == 16 && e_mouse.x >= 4 && e_mouse.x <= 25)
        sw = 9;
    else if (e_mouse.y == 17 && e_mouse.x >= 4 && e_mouse.x <= 23)
        sw = 10;
    else
        sw = 2;
    return (sw + 375);
}

int
e_data_ein_mouse (window)
we_window_t *window;
{
    extern struct mouse e_mouse;
    FLWND *fw = (FLWND *) window->b;
    we_control_t *control = window->ed;
    int i, c = 0;
    if (e_mouse.y == 0)
        return (AltBl);
    else if (e_mouse.y == MAXSLNS - 1)
        return (e_m3_mouse ());
    else if (e_mouse.x < window->a.x || e_mouse.x > window->e.x
             || e_mouse.y < window->a.y || e_mouse.y > window->e.y)
    {
        for (i = control->mxedt; i > 0; i--)
        {
            if (e_mouse.x >= control->window[i]->a.x && e_mouse.x <= control->window[i]->e.x
                    && e_mouse.y >= control->window[i]->a.y && e_mouse.y <= control->window[i]->e.y)
                return (control->edt[i] <
                        10 ? Alt1 - 1 + control->edt[i] : 1014 + control->edt[i]);
        }
    }
    else if (e_mouse.x == window->a.x + 3 && e_mouse.y == window->a.y)
        c = WPE_ESC;
    else if (e_mouse.x == window->e.x - 3 && e_mouse.y == window->a.y)
        e_ed_zoom (window);
    else if (e_mouse.x == window->a.x && e_mouse.y == window->a.y)
        e_eck_mouse (window, 1);
    else if (e_mouse.x == window->e.x && e_mouse.y == window->a.y)
        e_eck_mouse (window, 2);
    else if (e_mouse.x == window->e.x && e_mouse.y == window->e.y)
        e_eck_mouse (window, 3);
    else if (e_mouse.x == window->a.x && e_mouse.y == window->e.y)
        e_eck_mouse (window, 4);
    else if (e_mouse.y == window->a.y && e_mouse.x > window->a.x && e_mouse.x < window->e.x)
        e_eck_mouse (window, 0);
    else if (e_mouse.y >= fw->ya && e_mouse.y <= fw->ye &&
             e_mouse.x >= fw->xa && e_mouse.x <= fw->xe)
        c = AltF;
    else if (e_mouse.y == window->e.y - 2 && e_mouse.x >= window->e.x - 9
             && e_mouse.x <= window->e.x - 3)
        c = WPE_ESC;
    else if ((window->ins < 4 || window->ins == 7) && e_mouse.y == window->e.y - 4
             && e_mouse.x >= window->e.x - 9 && e_mouse.x <= window->e.x - 3)
        c = AltS;
    else if (window->ins > 3 && e_mouse.y == window->e.y - 8 && e_mouse.x >= window->e.x - 9
             && e_mouse.x <= window->e.x - 3)
        c = AltA;
    else if (window->ins > 3 && e_mouse.y == window->e.y - 6 && e_mouse.x >= window->e.x - 9
             && e_mouse.x <= window->e.x - 3)
        c = AltE;
    else if (window->ins > 3 && e_mouse.y == window->e.y - 4 && e_mouse.x >= window->e.x - 9
             && e_mouse.x <= window->e.x - 3)
        c = AltD;
    else if (window->ins == 4 && e_mouse.y == window->e.y - 10 && e_mouse.x >= window->e.x - 9
             && e_mouse.x <= window->e.x - 3)
        c = AltO;
    else
        c = AltF;
    while (e_mshit () != 0);
    return (c);
}

void
e_opt_eck_mouse (o)
W_OPTSTR *o;
{
    int g[4];
    int xold, yold, x, y, xa;
    we_view_t *view;
    e_std_rahmen (o->xa, o->ya, o->xe, o->ye, o->name, 0, o->frt, o->frs);
#ifndef NEWSTYLE
    if (!WpeIsXwin ())
        view = e_open_view (o->xa, o->ya, o->xe, o->ye, 0, 2);
    else
        view = e_open_view (o->xa, o->ya, o->xe - 2, o->ye - 1, 0, 2);
#else
    view = e_open_view (o->xa, o->ya, o->xe, o->ye, 0, 2);
#endif
    g[0] = 3;
    g[1] = 1;
    fk_mouse (g);
    xold = g[2] / 8;
    yold = g[3] / 8;
    xa = xold - o->xa;
    while (g[1] != 0)
    {
        x = g[2] / 8;
        y = g[3] / 8;
        if (y < 1)
            y = 1;
        else if (y > MAXSLNS - 2)
            y = MAXSLNS - 2;
        if (x < 0)
            x = 0;
        else if (x > MAXSCOL - 1)
            x = MAXSCOL - 1;
        if (xold != x || yold != y)
        {
            xold = x;
            yold = y;
            x -= xa;
            if (x < 0)
                x = 0;
            else if (x + o->xe - o->xa > MAXSCOL - 1)
                x = MAXSCOL - o->xe + o->xa - 1;
            if (o->ye + y - o->ya > MAXSLNS - 2)
                y = MAXSLNS - o->ye + o->ya - 2;
            o->xe = o->xe - o->xa + x;
            o->xa = x;
            o->ye = o->ye + y - o->ya;
            o->ya = y;
            g[0] = 2;
            fk_mouse (g);
            o->view =
                e_change_pic (o->xa, o->ya, o->xe, o->ye, o->view, 1, o->frt);
            if (o->view == NULL)
                e_error (e_msg[ERR_LOWMEM], 1, o->window->colorset);
            g[0] = 1;
            fk_mouse (g);
            view->a.x = o->xa;
            view->a.y = o->ya;
            view->e.x = o->xe;
            view->e.y = o->ye;
            e_close_view (view, 2);
        }
        g[0] = 3;
        fk_mouse (g);
    }
    view->a.x = o->xa;
    view->a.y = o->ya;
    view->e.x = o->xe;
    view->e.y = o->ye;
    e_close_view (view, 1);
    e_std_rahmen (o->xa, o->ya, o->xe, o->ye, o->name, 1, o->frt, o->frs);
}

int
e_opt_mouse (W_OPTSTR * o)
{
    extern struct mouse e_mouse;
    int c;

    if (e_mouse.y < o->ya || e_mouse.y >= o->ye ||
            e_mouse.x <= o->xa || e_mouse.x >= o->xe)
        return (-1);
    else if (e_mouse.y == o->ya)
    {
        if (e_mouse.x == o->xa + 3)
        {
            while (e_mshit ())
            {
                ;
            }
            return (WPE_ESC);
        }
        else
        {
            e_opt_eck_mouse (o);
            return (-1);
        }
    }
    while (e_mshit ())
    {
        ;
    }
    if ((c = e_get_opt_sw (0, e_mouse.x, e_mouse.y, o)))
        return (c);
    else
        return (-1);
}

#endif // #if MOUSE
