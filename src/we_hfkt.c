/** \file we_hfkt.c                                        */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <ctype.h>
#include <string.h>
#include <regex.h>
#include "keys.h"
#include "model.h"
#include "edit.h"
#include "we_term.h"
#include "WeString.h"
#include "we_hfkt.h"
#include "utils.h"

/*   numbers box (numbers input/edit)     */
int
e_num_kst (char *s, int num, int max, we_window_t * window, int n, int sw)
{
    int ret, nz = WpeNumberOfPlaces (max);
    char *tmp = malloc ((strlen (s) + 2) * sizeof (char));
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o || !tmp) {
        return (-1);
    }
    o->xa = 20;
    o->ya = 4;
    o->xe = 52;
    o->ye = 10;
    o->bgsw = 0;
    o->name = s;
    o->crsw = AltO;
    sprintf (tmp, "%s:", s);
    e_add_numstr (3, 2, 29 - nz, 2, nz, max, n, sw, tmp, num, o);
    free (tmp);
    e_add_bttstr (6, 4, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (21, 4, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        num = o->nstr[0]->num;
    }
    freeostr (o);
    return (num);
}

/*   determine string length delimited by null or newline (WPE_WR == 10) */
int
e_str_len (unsigned char *s)
{
    int i;

    for (i = 0; *(s + i) != '\0' && *(s + i) != WPE_WR; i++)
        ;
    return (i);
}

/*           we_color_t - fill struct with constants           */
we_color_t
e_s_x_clr (int fg_color, int bg_color)
{
    we_color_t color;

    color.fg_color = fg_color;
    color.bg_color = bg_color;
    color.fg_bg_color = 16 * bg_color + fg_color;
    return (color);
}

we_color_t
e_n_x_clr (int fg_bg_color)
{
    we_color_t color;

    color.fg_bg_color = fg_bg_color;
    color.bg_color = fg_bg_color / 16;
    color.fg_color = fg_bg_color % 16;
    return (color);
}

we_color_t
e_s_t_clr (int fg_color, int bg_color)
{
    we_color_t color;

    color.fg_color = fg_color;
    color.bg_color = bg_color;
    color.fg_bg_color = fg_color;
    return (color);
}

we_color_t
e_n_t_clr (int fg_bg_color)
{
    we_color_t color;

    color.fg_bg_color = fg_bg_color;
    color.bg_color = fg_bg_color;
    color.fg_color = fg_bg_color;
    return (color);
}

/*            we_point_t - fill struct with constants            */
we_point_t
e_set_pnt (int x, int y)
{
    we_point_t p;

    p.x = x;
    p.y = y;
    return (p);
}

int
e_pr_uul (we_colorset_t * fb)
{
    extern WOPT *blst;
    extern int nblst;
    int i;

    e_blk (max_screen_cols(), 0, max_screen_lines() - 1, fb->mt.fg_bg_color);
    for (i = 0; i < nblst && blst[i].x < max_screen_cols(); ++i)
        e_pr_str_scan (blst[i].x + 1, max_screen_lines() - 1, blst[i].t, fb->mt.fg_bg_color,
                       blst[i].s, blst[i].n, fb->ms.fg_bg_color, blst[i].x,
                       i == nblst - 1 ? max_screen_cols() - 1 : blst[i + 1].x - 1);
    return (i);
}
