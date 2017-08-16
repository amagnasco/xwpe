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

extern char *e_hlp;
extern WOPT *blst;
extern WOPT *eblst;
extern WOPT eblst_u[];
extern WOPT eblst_o[];

void ECNT_Init(we_control_t * cn);

extern struct CNT *global_editor_control;

#endif
