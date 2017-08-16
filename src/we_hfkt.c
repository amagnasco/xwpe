/* \file we_hfkt.c                                        */
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
#include "WeString.h"
#include "we_hfkt.h"
#include "utils.h"

/*   numbers box (numbers input/edit)     */
int
e_num_kst (char *s, int num, int max, we_window_t * f, int n, int sw)
{
    int ret, nz = WpeNumberOfPlaces (max);
    char *tmp = malloc ((strlen (s) + 2) * sizeof (char));
    W_OPTSTR *o = e_init_opt_kst (f);

    if (!o || !tmp)
        return (-1);
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
    if (ret != WPE_ESC)
        num = o->nstr[0]->num;
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
e_s_x_clr (int f, int b)
{
    we_color_t c;

    c.f = f;
    c.b = b;
    c.fb = 16 * b + f;
    return (c);
}

we_color_t
e_n_x_clr (int fb)
{
    we_color_t f;

    f.fb = fb;
    f.b = fb / 16;
    f.f = fb % 16;
    return (f);
}

we_color_t
e_s_t_clr (int f, int b)
{
    we_color_t c;

    c.f = f;
    c.b = b;
    c.fb = f;
    return (c);
}

we_color_t
e_n_t_clr (int fb)
{
    we_color_t f;

    f.fb = fb;
    f.b = fb;
    f.f = fb;
    return (f);
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

    e_blk (MAXSCOL, 0, MAXSLNS - 1, fb->mt.fb);
    for (i = 0; i < nblst && blst[i].x < MAXSCOL; ++i)
        e_pr_str_scan (blst[i].x + 1, MAXSLNS - 1, blst[i].t, fb->mt.fb,
                       blst[i].s, blst[i].n, fb->ms.fb, blst[i].x,
                       i == nblst - 1 ? MAXSCOL - 1 : blst[i + 1].x - 1);
    return (i);
}
