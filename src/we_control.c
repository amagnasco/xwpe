/** \file we_control.c */
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

#include "config.h"
#include <string.h>
#include "messages.h"
#include "keys.h"
#include "we_mouse.h"
#include "options.h"
#include "WeString.h"
#include "we_prog.h"
#include "WeProg.h"
#include "we_fl_fkt.h"
#include "we_control.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "attrb.h"
#endif

/* externals */
extern char *e_tmp_dir;
extern char *e_hlp_str[];

extern int col_num;

extern we_colorset_t *u_fb, *x_fb;

/* globals */
struct CNT *global_editor_control;
int fk__cursor = 0;

WOPT *eblst, *fblst, *mblst, *dblst, *xblst, *wblst, *rblst;
WOPT *ablst, *sblst, *hblst, *gblst, *oblst;

char *e_hlp, *user_shell;
WOPT *blst;
int nblst = 7;

#if MOUSE
int fk__mouse_stat = 1;
struct mouse e_mouse = { 0, 0, 0 };
#endif

/* preinitialized arrays */
char *e_msg[] = { "Not Enough Memory",
                  "Option-File got the Wrong Version",
                  "Can\'t read Option-File",
                  "More Than 35 Windows",
                  "String NOT found",
                  "Can\'t open File %s",	/*  Number 5  */
                  "Can\'t close File",
                  "No more Undo",
                  "Execute Command: %s\n",
                  "Command not found",
                  "\nStrike <Return> to continue\n",	/*  Number 10 */
                  "Not Installed",
                  "Can\'t open Option-File",
                  "Can\'t write to Option-File",
                  "File failed to save",
                  "Not all files saved",	/*  Number 15  */
                  "Can\'t Print File",
                  "No more Redo",
                  "Can\'t open directory",
                  "Can\'t open HOME directory",
                  "System error",		/*  Number 20 */
                  "Wastebasket is not available",
                  "Can\'t create directory",
                  "Can\'t access file or directory",
                  "Can\'t remove file or directory",
                  "Can\'t link file or directory",	/*  Number 25 */
                  "File already exist with \"new.dir\" name",
                  "Cannot change permissions",
                  "Can\'t rename file or directory",
                  "Can\'t open file for reading",
                  "Can\'t open file for writing",	/*  Number 30 */
                  "Can\'t allocate buffer for copying",
                  "Inconsistent copying"
                };

OPT opt[] = { {"#", 3, '#', AltSYS},
    {"File", 10, 'F', AltF},
    {"Edit", 19, 'E', AltE},
    {"Search", 28, 'S', AltS},
    {"Block", 39, 'B', AltB},
    {"Options", 49, 'O', AltO},
    {"Window", 61, 'W', AltW},
    {"Help", 72, 'H', AltH}
#ifdef PROG
    , {NULL, 0, 0, 0}
    , {NULL, 0, 0, 0}
#endif
#ifdef DEBUGGER
    , {NULL, 0, 0, 0}
#endif
};

WOPT eblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"F2 Save", 9, 0, 2, F2},
    {"F3 Files", 18, 0, 2, F3},
    {"Alt-F3 Close W.", 28, 0, 6, AF3},
    {"F4 Search", 45, 0, 2, F4},
    {"^L S.Again", 56, 0, 2, CF4},
    {"Alt-X Quit", 68, 0, 5, AltX}
};

WOPT eblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Alt-F2 Save", 9, 0, 6, AF2},
    {"F2 Files", 22, 0, 2, F2},
    {"^F4 Close ", 32, 0, 3, CF4},
    {"Alt-F3 Srch", 44, 0, 6, AF3},
    {"F3 S.Ag.", 57, 0, 2, F3},
    {"Alt-F4 Quit", 67, 0, 6, AF4}
};

WOPT fblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Edit", 10, 0, 1, AltE},
    {"Attributes", 17, 0, 1, AltA},
    {"Move", 29, 0, 1, AltM},
    {"COpy", 35, 1, 1, AltO},
    {"Remove", 41, 0, 1, AltR},
    {"Alt-F3 Close W.", 49, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT fblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Edit", 9, 0, 1, AltE},
    {"RePlace", 15, 2, 1, AltP},
    {"Move", 24, 0, 1, AltM},
    {"DUplicate", 30, 1, 1, AltU},
    {"Remove", 41, 0, 1, AltR},
    {"^F4 Close W.", 49, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT mblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"PreV. Err.", 9, 3, 1, AltV},
    {"NexT Err.", 21, 3, 1, AltT},
    {"Compile", 32, 0, 1, AltC},
    {"Make", 41, 0, 1, AltM},
    {"RUn", 47, 1, 1, AltU},
    {"Alt-F3 Close", 52, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT mblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"PreV. Err.", 9, 3, 1, AltV},
    {"NexT Err.", 21, 3, 1, AltT},
    {"Compile", 32, 0, 1, AltC},
    {"Make", 41, 0, 1, AltM},
    {"RUn", 47, 1, 1, AltU},
    {"^F4 Close", 52, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT dblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"^F7 Make Watch", 9, 0, 3, CF7},
    {"F7 Trace", 25, 0, 2, F7},
    {"F8 Step", 34, 0, 2, F8},
    {"^F10 Run", 43, 0, 4, F10},
    {"Alt-F3 Close", 53, 0, 6, AF3},
    {"Alt-X Quit", 67, 0, 5, AltX}
};

WOPT dblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"^F5 Make Watch", 9, 0, 3, CF7},
    {"F7 Trace", 25, 0, 2, F7},
    {"F8 Step", 34, 0, 2, F8},
    {"^F10 Run", 43, 0, 4, F10},
    {"^F4 Close", 53, 0, 3, CF4},
    {"Alt-F4 Quit", 64, 0, 6, AF4}
};

WOPT xblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Execute", 19, 0, 1, AltE},
    {"Alt-F3 Close W.", 38, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT xblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Execute", 19, 0, 1, AltE},
    {"^F4 Close W.", 38, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT wblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Write", 19, 0, 1, AltW},
    {"Alt-F3 Close W.", 38, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT wblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Write", 19, 0, 1, AltW},
    {"^F4 Close W.", 38, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT rblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Read", 19, 0, 1, AltR},
    {"Alt-F3 Close W.", 38, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT rblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Read", 19, 0, 1, AltR},
    {"^F4 Close W.", 38, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT ablst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Add", 19, 0, 1, AltA},
    {"Alt-F3 Close W.", 38, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT ablst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Add", 19, 0, 1, AltA},
    {"^F4 Close W.", 38, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT sblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Save", 14, 0, 1, AltS},
    {"Change Dir", 25, 0, 1, AltC},
    {"Alt-F3 Close W.", 42, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT sblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Save", 14, 0, 1, AltS},
    {"Change Dir", 25, 0, 1, AltC},
    {"^F4 Close W.", 42, 0, 3, CF4},
    {"Alt-F4 Quit", 66, 0, 6, AF4}
};

WOPT hblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"<BSP> Back", 9, 0, 5, WPE_DC},
    {"<CR> Goto", 21, 0, 4, WPE_CR},
    {" PreVious", 32, 4, 1, AltV},
    {" NexT", 45, 4, 1, AltT},
    {"Alt-F3 Close", 54, 0, 6, AF3},
    {"Alt-X Quit", 68, 0, 5, AltX}
};

WOPT hblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"<BSP> Back", 9, 0, 5, WPE_DC},
    {"<CR> Goto", 21, 0, 4, WPE_CR},
    {" PreVious", 32, 4, 1, AltV},
    {" NexT", 45, 4, 1, AltT},
    {"^F4 Close", 54, 0, 3, CF4},
    {"Alt-F4 Quit", 65, 0, 6, AF4}
};

WOPT gblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Show", 19, 0, 1, AltS},
    {"Alt-F3 Close W.", 38, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT gblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Show", 19, 0, 1, AltS},
    {"^F4 Close W.", 38, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

WOPT oblst_o[] = { {"F1 Help", 0, 0, 2, F1},
    {"Add", 14, 0, 1, AltA},
    {"Delete", 25, 0, 1, AltD},
    {"Alt-F3 Close W.", 42, 0, 6, AF3},
    {"Alt-X Quit", 66, 0, 5, AltX}
};

WOPT oblst_u[] = { {"F1 Help", 0, 0, 2, F1},
    {"Add", 14, 0, 1, AltA},
    {"Delete", 25, 0, 1, AltD},
    {"^F4 Close W.", 42, 0, 3, CF4},
    {"Alt-F4 Quit", 63, 0, 6, AF4}
};

void
ECNT_Init (we_control_t * control)
{
    control->mxedt = -1;
    control->curedt = 0;
    control->edt[0] = 0;
    control->colorset = NULL;
    control->print_cmd = WpeStrdup (PRNTCMD);

    control->dirct = WpeGetCurrentDir (control);

    strcpy (control->find.search, "");
    strcpy (control->find.replace, "");
    strcpy (control->find.file, SUDIR);
    control->find.dirct = WpeStrdup (control->dirct);

    control->find.sw = 16;
    control->find.sn = 0;
    control->find.rn = 0;

    control->sdf = control->rdf = control->fdf = control->ddf = control->wdf = control->hdf = control->shdf = NULL;

    /*   standard adjustments    */
    control->dtmd = DTMD_NORMAL;
    control->autosv = 0;
    control->maxchg = 999;
    control->numundo = 10;
    control->maxcol = MAXCOLUM;
    control->tabn = 8;
    control->autoindent = 3;
    control->tabs = malloc ((control->tabn + 1) * sizeof (char));
    WpeStringBlank (control->tabs, control->tabn);
    control->flopt = FM_REKURSIVE_ACTIONS | FM_REMOVE_INTO_WB | FM_MOVE_PROMPT |
                FM_MOVE_PROMPT | FM_PROMPT_DELETE;
    control->edopt = ED_SOURCE_AUTO_INDENT | ED_ERRORS_STOP_AT | ED_SYNTAX_HIGHLIGHT;
}

int
e_switch_blst (we_control_t * control)
{
    int i;
    we_window_t *f;

    if (control->edopt & ED_CUA_STYLE)
    {
        for (i = 0; i <= control->mxedt; i++)
        {
            f = control->f[i];
            if (f->blst == eblst_o)
                f->blst = eblst_u;
            else if (f->blst == fblst_o)
                f->blst = fblst_u;
            else if (f->blst == mblst_o)
                f->blst = mblst_u;
            else if (f->blst == dblst_o)
                f->blst = dblst_u;
            else if (f->blst == xblst_o)
                f->blst = xblst_u;
            else if (f->blst == wblst_o)
                f->blst = wblst_u;
            else if (f->blst == rblst_o)
                f->blst = rblst_u;
            else if (f->blst == ablst_o)
                f->blst = ablst_u;
            else if (f->blst == sblst_o)
                f->blst = sblst_u;
            else if (f->blst == hblst_o)
                f->blst = hblst_u;
            else if (f->blst == gblst_o)
                f->blst = gblst_u;
            else if (f->blst == oblst_o)
                f->blst = oblst_u;
        }
    }
    else
    {
        for (i = 0; i <= control->mxedt; i++)
        {
            f = control->f[i];
            if (f->blst == eblst_u)
                f->blst = eblst_o;
            else if (f->blst == fblst_u)
                f->blst = fblst_o;
            else if (f->blst == mblst_u)
                f->blst = mblst_o;
            else if (f->blst == dblst_u)
                f->blst = dblst_o;
            else if (f->blst == xblst_u)
                f->blst = xblst_o;
            else if (f->blst == wblst_u)
                f->blst = wblst_o;
            else if (f->blst == rblst_u)
                f->blst = rblst_o;
            else if (f->blst == ablst_u)
                f->blst = ablst_o;
            else if (f->blst == sblst_u)
                f->blst = sblst_o;
            else if (f->blst == hblst_u)
                f->blst = hblst_o;
            else if (f->blst == gblst_u)
                f->blst = gblst_o;
            else if (f->blst == oblst_u)
                f->blst = oblst_o;
        }
    }
    return (0);
}

void
e_ini_desk (we_control_t * control)
{
    extern int e_mn_men;
    int i;

    if (control->edopt & ED_CUA_STYLE)
    {
        eblst = eblst_u;
        fblst = fblst_u;
        mblst = mblst_u;
        dblst = dblst_u;
        xblst = xblst_u;
        wblst = wblst_u;
        rblst = rblst_u;
        ablst = ablst_u;
        sblst = sblst_u;
        hblst = hblst_u;
        gblst = gblst_u;
        oblst = oblst_u;
    }
    else
    {
        eblst = eblst_o;
        fblst = fblst_o;
        mblst = mblst_o;
        dblst = dblst_o;
        xblst = xblst_o;
        wblst = wblst_o;
        rblst = rblst_o;
        ablst = ablst_o;
        sblst = sblst_o;
        hblst = hblst_o;
        gblst = gblst_o;
        oblst = oblst_o;
    }
    e_cls (control->colorset->df.fg_bg_color, control->colorset->dc);
    e_blk (MAXSCOL, 0, 0, control->colorset->mt.fg_bg_color);

    /* put out the main menu */
    for (i = 0; i < MENOPT; ++i)
    {
        e_pr_str_wsd (opt[i].x, 0, opt[i].t, control->colorset->mt.fg_bg_color, 0, 1, control->colorset->ms.fg_bg_color,
                      (i == 0 ? 0 : opt[i].x - e_mn_men),
                      (i ==
                       MENOPT - 1) ? MAXSCOL - 1 : opt[i + 1].x - e_mn_men - 1);
    }

    e_pr_uul (control->colorset);
}

void
we_colorset_Init (we_colorset_t * fb)
{
    fb->er = e_s_x_clr (15, 4);	/*  Editor Frames         */
    fb->et = e_s_x_clr (11, 4);	/*  Editor Text           */
    fb->ez = e_s_x_clr (4, 7);	/*  Editors Text highlighted  */
    fb->es = e_s_x_clr (10, 4);	/*  Editors Window-Button */
    fb->ek = e_s_x_clr (11, 6);	/*  Editors highlighted (find) */
    fb->em = e_s_x_clr (4, 6);	/*  Mouse slider bar         */
    fb->mr = e_s_x_clr (0, 7);	/*  Menu Frame             */
    fb->mt = e_s_x_clr (0, 7);	/*  Menu Text               */
    fb->mz = e_s_x_clr (0, 2);	/*  Menu Text highlighted      */
    fb->ms = e_s_x_clr (1, 7);	/*  Menu-Switch           */
    fb->nr = e_s_x_clr (15, 7);	/*  Options Frame         */
    fb->nt = e_s_x_clr (0, 7);	/*  Options Text           */
    fb->nsnt = e_s_x_clr (11, 7);	/*  Options Text-Switch  */
    fb->ne = e_s_x_clr (1, 7);	/*  Options Window-Button */
    fb->fr = e_s_x_clr (15, 4);	/*  Editor Control Bar (?) (Schreibleiste) passive */
    fb->fa = e_s_x_clr (15, 2);	/*  Editor Control Bar (?) (Schreibleiste) active */
    fb->ft = e_s_x_clr (0, 6);	/*  Data Text                 */
    fb->fz = e_s_x_clr (15, 2);	/*  Data active highlighted   */
    fb->frft = e_s_x_clr (15, 6);	/*  Data passive highlighted          */
    fb->fs = e_s_x_clr (0, 6);	/*  Switch Text               */
    fb->nsft = e_s_x_clr (14, 6);	/*  Switch Switch     */
    fb->fsm = e_s_x_clr (14, 6);	/*  Switch active     */
    fb->nz = e_s_x_clr (15, 2);	/*  Button Text             */
    fb->ns = e_s_x_clr (11, 2);	/*  Button Switch         */
    fb->nm = e_s_x_clr (11, 2);	/*  Button Text highlighted    */
    fb->hh = e_s_x_clr (12, 6);	/*  Help Header               */
    fb->hb = e_s_x_clr (11, 2);	/*  Help Button               */
    fb->hm = e_s_x_clr (10, 4);	/*  Help highlighted                  */
    fb->df = e_s_x_clr (7, 0);	/*  Background                */
    fb->of = e_s_x_clr (8, 0);	/*  void    (unused)          */
    fb->db = e_s_x_clr (0, 1);	/*  Breakpoint                        */
    fb->dy = e_s_x_clr (12, 6);	/*  Debugger Stop             */
    fb->ct = e_s_x_clr (11, 4);	/*  C-Prog. Text                 */
    fb->cr = e_s_x_clr (15, 4);	/*  C-Prog. res. Words           */
    fb->ck = e_s_x_clr (14, 4);	/*  C-Prog. Constants           */
    fb->cp = e_s_x_clr (10, 4);	/*  C-Prog. Pre-processor         */
    fb->cc = e_s_x_clr (7, 4);	/*  C-Prog. Comments               */
    fb->dc = ' ';
    fb->ws = 7;
}

we_colorset_t *
e_ini_farbe ()
{
    if (WpeIsXwin ())
    {
        if (!x_fb)
            x_fb = malloc (sizeof (we_colorset_t));
        we_colorset_Init (x_fb);
        return x_fb;
    }
    else
    {
        if (!u_fb)
            u_fb = malloc (sizeof (we_colorset_t));
        if (col_num)
        {
            we_colorset_Init (u_fb);
        }
        else
        {
            u_fb->er = e_n_t_clr (0);
            u_fb->et = e_n_t_clr (0);
            u_fb->ez = e_n_t_clr (A_REVERSE);
            u_fb->es = e_n_t_clr (0);
            u_fb->ek = e_n_t_clr (A_UNDERLINE);
            u_fb->em = e_n_t_clr (A_STANDOUT);
            u_fb->mr = e_n_t_clr (A_STANDOUT);
            u_fb->mt = e_n_t_clr (A_STANDOUT);
            u_fb->mz = e_n_t_clr (0);
            u_fb->ms = e_n_t_clr (0);
            u_fb->nr = e_n_t_clr (A_STANDOUT);
            u_fb->nt = e_n_t_clr (A_REVERSE);
            u_fb->nsnt = e_n_t_clr (A_BOLD);
            u_fb->ne = e_n_t_clr (0);
            u_fb->fr = e_n_t_clr (0);
            u_fb->fa = e_n_t_clr (A_REVERSE);
            u_fb->ft = e_n_t_clr (0);
            u_fb->fz = e_n_t_clr (A_STANDOUT);
            u_fb->frft = e_n_t_clr (0);
            u_fb->fs = e_n_t_clr (0);
            u_fb->nsft = e_n_t_clr (A_BOLD);
            u_fb->fsm = e_n_t_clr (A_BOLD);
            u_fb->nz = e_n_t_clr (0);
            u_fb->ns = e_n_t_clr (A_BOLD);
            u_fb->nm = e_n_t_clr (A_BOLD);
            u_fb->hh = e_n_t_clr (A_REVERSE);
            u_fb->hb = e_n_t_clr (A_REVERSE);
            u_fb->hm = e_n_t_clr (A_BOLD);
            u_fb->of = e_n_t_clr (A_STANDOUT);
            u_fb->df = e_n_t_clr (0);
            u_fb->db = e_n_t_clr (A_STANDOUT);
            u_fb->dy = e_n_t_clr (A_STANDOUT);
            u_fb->ct = e_n_t_clr (0);
            u_fb->cr = e_n_t_clr (0);
            u_fb->ck = e_n_t_clr (0);
            u_fb->cp = e_n_t_clr (0);
            u_fb->cc = e_n_t_clr (0);
            u_fb->dc = 0x20;
            u_fb->ws = 0;
        }
        return u_fb;
    }
}

void
e_free_find (FIND * find)
{
    if (find->dirct)
    {
        free (find->dirct);
        find->dirct = NULL;
    }
}
