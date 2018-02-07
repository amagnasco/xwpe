/** \file we_control.h */
/* Copyright (C) 1993 Fred Kruse
   Copyright (C) 2017 Guus Bonnema

    This file is part of Xwpe - Xwindows Programming Editor.

    Xwpe - Xwindows Programming Editor is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Xwpe - Xwindows Programming Editor is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Xwpe - Xwindows Programming Editor.  If not, see <http://www.gnu.org/licenses/>.

   */
/**
 * Most of the code in this file was copied from we_main.c and later adapted
 * For that reason the copyright starts with Fred Kruse's original copyright
 * even though he never created this file.
 *
 */

#ifndef WE_CONTROL_H
#define WE_CONTROL_H

#include "config.h"
#include "edit.h"

typedef struct CNT we_control_t;

/* prototypes */
void we_colorset_init(we_colorset_t* fb);
we_colorset_t* e_ini_colorset();
we_control_t *e_control_new();
int max_edit_windows();

/** Maximum number of open edit windows.
 *
 * This define is only meant for internal use. Do not
 * use it outside we_control.c / we_control.h.
 * See \function max_edit_windows() for general access.
 *
 *
 * */
#define MAX_EDIT_WINDOWS 35

struct CNT {
    int major, minor, patch; /**< Version of option file. */
    int maxcol, tabn;
    int maxchg, numundo;
    int flopt, edopt;
    int mxedt;		 /**< max number of editing windows */
    int curedt;		 /**< currently active window */
    int edt[MAX_EDIT_WINDOWS + 1]; /**< 1 <= window IDs <= max_edit_windows(), arbitrary order */
    int autoindent;
    char* print_cmd;
    char* dirct; /**< current directory */
    char *optfile, *tabs;
    struct dirfile *sdf, *rdf, *fdf, *ddf, *wdf, *hdf, *shdf;
    FIND find;
    we_colorset_t* colorset;
    we_window_t* window[MAX_EDIT_WINDOWS + 1];
    char dtmd, autosv;
};

extern char *e_hlp;
extern WOPT *blst;
extern WOPT *eblst;
extern WOPT eblst_u[];
extern WOPT eblst_o[];

extern we_control_t *global_editor_control;

#endif
