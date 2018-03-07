/** \file we_fl_unix.c                                     */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <ctype.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include "keys.h"
#include "messages.h"
#include "model.h"
#include "options.h"
#include "we_screen.h"
#include "we_term.h"
#if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES
#include "curses.h"
#endif
#include "edit.h"
#include "we_find.h"
#include "we_file_unix.h"
#include "we_progn.h"
#include "we_prog.h"
#include "WeString.h"

#ifdef UNIX

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

struct dirfile *e_make_win_list (we_window_t * window);
extern char *e_tmp_dir;

#ifndef HAVE_SYMLINK
#define readlink(x, y, z) -1
#define WpeRenameLink(x, y, z, window) 0
#define WpeLinkFile(x, y, sw, window) link(x, y)
#define lstat(x,y)  stat(x,y)
#undef S_ISLNK
#define S_ISLNK(x)  0
#else
#include <unistd.h>
#endif // #ifndef HAVE_SYMLINK

#define WPE_PATHMAX 2048

/* buffer size for copying */
#define E_C_BUFFERSIZE 524288	/*   1/2  Mega Byte   */

#ifdef DEBUG
int
SpecialError (char *text, int sw, we_colorset_t * window, char *file, int line)
{
    fprintf (stderr, "\nFile \"%s\" line %d\n", file, line);
    return e_error (text, sw, window);
}

#define e_error(text, sw, window) SpecialError(text, sw, window, __FILE__, __LINE__)
#endif // #ifdef DEBUG

int WpeGrepFile (char *file, char *string, int sw);
int WpeRemove (char *file, we_window_t * window);

struct dirfile *WpeSearchFiles (we_window_t * window,
                                char *dirct, char *file, char *string,
                                struct dirfile *df, int sw);
int e_rename (char *file, char *newname, we_window_t * window);
int WpeRemoveDir (char *dirct, char *file, we_window_t * window, int rec);
char *WpeGetWastefile (char *file);
int WpeMakeNewDir (we_window_t * window);
int WpeFileDirAttributes (char *filen, we_window_t * window);
int WpeRenameCopyDir (char *dirct, char *file, char *newname,
                      we_window_t * window, int rec, int sw);
int WpeRenameCopy (char *file, char *newname, we_window_t * window, int sw);
int WpeCopyFileCont (char *oldfile, char *newfile, we_window_t * window);

int WpeDirDelOptions (we_window_t * window);
#ifdef HAVE_SYMLINK
int WpeLinkFile (char *fl, char *ln, int sw, we_window_t * window);
int WpeRenameLink (char *old, char *ln, char *fl, we_window_t * window);
#endif

/* setup the file-manager structures
   'dirct' or the current dir. is used to
   search for directories and files */
/* sw | file manager type
   ------------------------
   0  | open/new file
   1  | read block of text from file
   2  | write block of text to file
   3  | execute file
   4  | save as
   5  | add file to project
   6  | wastebasket

*/
int
WpeCreateFileManager (int sw, we_control_t * control, char *dirct)
{
    extern char *e_hlp_str[];
    extern WOPT *fblst, *rblst, *wblst, *xblst, *sblst, *ablst;
    we_window_t *window;
    int i, j;
    FLBFFR *file_buffer;
    int allocate_size;		/* inital memory size for allocation */
    char *sfile;

    /* check whether we reached the maximum number of windows */
    if (control->mxedt >= max_edit_windows()) {
        e_error (e_msg[ERR_MAXWINS], ERROR_MSG, control->colorset);
        return (-1);
    }

    /* search for a not used window ID number (j) */
    for (j = 1; j <= max_edit_windows(); j++) {
        for (i = 1; i <= control->mxedt && control->edt[i] != j; i++)
            ;
        if (i > control->mxedt) {
            break;
        }
    }

    /* change the shape of the mouse :-) */
    WpeMouseChangeShape (WpeWorkingShape);

    /* currently active window,
       one more window in the system,
       its number is j */
    control->curedt = j;
    (control->mxedt)++;
    control->edt[control->mxedt] = j;

    /* allocate window structure */
    if ((window = (we_window_t *) malloc (sizeof (we_window_t))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    /* allocate buffer related to the window (NOT proper type, later casted) */
    if ((file_buffer = (FLBFFR *) malloc (sizeof (FLBFFR))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    window->colorset = control->colorset;
    control->window[control->mxedt] = window;		/* store the window structure at appropriate place */
    window->a = e_set_pnt (11, 2);	/* beginning of the box */
    window->e = e_set_pnt (window->a.x + 55, window->a.y + 20);	/* other coord. of the box */
    window->winnum = control->curedt;
    window->dtmd = DTMD_FILEMANAGER;
    window->ins = 1;
    window->save = 0;
    window->zoom = 0;
    window->edit_control = control;
    window->c_sw = NULL;
    window->c_st = NULL;
    window->view = NULL;
    if (sw == 6) {
        sw = 0;
        window->datnam = "Wastebasket";
        window->save = 1;
    } else {
        window->datnam = "File-Manager";    /* window header text */
    }

    /* status line text for different mode */
    if (sw == 0) {
        window->blst = fblst;
        window->nblst = 8;
    } else if (sw == 1) {
        window->blst = rblst;
        window->nblst = 4;
    } else if (sw == 2) {
        window->blst = wblst;
        window->nblst = 4;
    } else if (sw == 3) {
        window->blst = xblst;
        window->nblst = 4;
    } else if (sw == 4) {
        window->blst = sblst;
        window->nblst = 5;
    } else if (sw == 5) {
        window->blst = ablst;
        window->nblst = 4;
    }

    if (sw == 3) {
        window->hlp_str = e_hlp_str[5];
    } else {
        window->hlp_str = e_hlp_str[4];
    }

    if (!dirct || dirct[0] == '\0') {
        /* no working directory has been given */
        window->dirct = WpeGetCurrentDir (control);
    } else {
        /* working directory is given, copy it over */
        allocate_size = strlen (dirct);
        if ((window->dirct = malloc (allocate_size + 1)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
        }

        strcpy (window->dirct, dirct);
    }

    strcpy (window->find.search, "");
    strcpy (window->find.replace, "");
    strcpy (window->find.file, SUDIR);
    window->find.dirct = WpeStrdup (window->dirct);

    window->find.sw = 16;
    window->find.sn = 0;
    window->find.rn = 0;

    window->buffer = (we_buffer_t *) file_buffer;
    /* the find pattern can only be 79 see FIND structure */
    if ((file_buffer->rdfile = malloc (80)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }
    strcpy (file_buffer->rdfile, window->find.file);	/* find file pattern */

    file_buffer->sw = sw;

    /* window for files */
    if ((file_buffer->fw = (FLWND *) malloc (sizeof (FLWND))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    /* window for directory */
    if ((file_buffer->dw = (FLWND *) malloc (sizeof (FLWND))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }


    if ((sfile = malloc (strlen (window->dirct) + strlen (file_buffer->rdfile) + 2)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    /* determine current directory */
    file_buffer->cd = WpeCreateWorkingDirTree (window->save, control);
    /* it is necessary to do this, because the file manager may not be
       in the appropriate directory here */
    sprintf (sfile, "%s/%s", window->dirct, SUDIR);
    /* find all other directories in the current */
    file_buffer->dd = e_find_dir (sfile, window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
    /* setup the drawing in the dir tree window */
    file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);

    i = window->edit_control->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0;
    if (sw == 3) {
        i |= 2;
    }

    /* finds all files matching the pattern */
    sprintf (sfile, "%s/%s", window->dirct, file_buffer->rdfile);
    file_buffer->df = e_find_files (sfile, i);

    free (sfile);

    /* setup the drawing in the file list window */
    file_buffer->fw->df = WpeGraphicalFileList (file_buffer->df, window->edit_control->flopt >> 9, control);

    /* file box - geometry and vertical slider settings */
    file_buffer->fw->mxa = window->a.x;
    file_buffer->fw->mxe = window->e.x;
    file_buffer->fw->mya = window->a.y;
    file_buffer->fw->mye = window->e.y;
    file_buffer->fw->xa = window->e.x - 33;
    file_buffer->fw->xe = window->e.x - 17;
    file_buffer->fw->ya = window->a.y + 6;
    file_buffer->fw->ye = window->a.y + 17;
    file_buffer->fw->window = window;
    file_buffer->fw->ia = file_buffer->fw->nf = file_buffer->fw->nxfo = file_buffer->fw->nyfo = 0;
    file_buffer->fw->srcha = file_buffer->fw->ja = 12;

    /* directory box - geometry and vertical slider settings */
    file_buffer->dw->mxa = window->a.x;
    file_buffer->dw->mxe = window->e.x;
    file_buffer->dw->mya = window->a.y;
    file_buffer->dw->mye = window->e.y;
    file_buffer->dw->xa = window->a.x + 3;
    file_buffer->dw->xe = window->a.x + 28;
    file_buffer->dw->ya = window->a.y + 6;
    file_buffer->dw->ye = window->a.y + 17;
    file_buffer->dw->window = window;
    file_buffer->dw->ia = file_buffer->dw->ja = file_buffer->dw->nxfo = 0;
    file_buffer->dw->srcha = -1;
    file_buffer->dw->nf = file_buffer->dw->nyfo = file_buffer->cd->nr_files - 1;

    if (control->mxedt > 1) {
        e_ed_rahmen (control->window[control->mxedt - 1], 0);
    }

    e_firstl (window, 1);

    /* basically it draws the window out */
    WpeDrawFileManager (window);

    /* restore the shape of the mouse */
    WpeMouseRestoreShape ();
    return (0);
}

/* drawing out the file-manager,
   first buttons, than the dir tree and file list */
int
WpeDrawFileManager (we_window_t * window)
{
    FLBFFR *file_buffer = (FLBFFR *) window->buffer;
    int i, j;
    int bx1 = 1, bx2 = 1, bx3 = 1, by = 4;

    for (j = window->a.y + 1; j < window->e.y; j++)
        for (i = window->a.x + 1; i < window->e.x; i++) {
            e_pr_char (i, j, ' ', window->colorset->nt.fg_bg_color);
        }

    if (num_lines_on_screen(window) <= 17) {
        by = -1;
    } else if (file_buffer->sw != 0 || num_lines_on_screen(window) <= 19) {
        by = 2;
    }

    if (num_lines_on_screen(window) > 17) {
        e_pr_str ((window->a.x + 4), window->e.y - by, "Cancel", window->colorset->nz.fg_bg_color, -1, -1,
                  window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        e_pr_str ((window->a.x + 14), window->e.y - by, "Change Dir", window->colorset->nz.fg_bg_color, 0, -1,
                  window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        if (file_buffer->sw == 1 && num_cols_on_screen(window) >= 34)
            e_pr_str ((window->a.x + 28), window->e.y - by, "Read", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        else if (file_buffer->sw == 2 && num_cols_on_screen(window) >= 35)
            e_pr_str ((window->a.x + 28), window->e.y - by, "Write", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        else if (file_buffer->sw == 4) {
            if (num_cols_on_screen(window) >= 34)
                e_pr_str ((window->a.x + 28), window->e.y - by, "Save", window->colorset->nz.fg_bg_color, 0, -1,
                          window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        } else if (file_buffer->sw == 3 && num_cols_on_screen(window) >= 37)
            e_pr_str ((window->a.x + 28), window->e.y - by, "Execute", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        else if (file_buffer->sw == 5 && num_cols_on_screen(window) >= 33)
            e_pr_str ((window->a.x + 28), window->e.y - by, "Add", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        else if (file_buffer->sw == 0) {
            if (num_cols_on_screen(window) >= 35)
                e_pr_str ((window->a.x + 28), window->e.y - by, "MKdir", window->colorset->nz.fg_bg_color, 1,
                          -1, window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
            if (num_cols_on_screen(window) >= 49)
                e_pr_str ((window->a.x + 37), window->e.y - by, "Attributes", window->colorset->nz.fg_bg_color,
                          0, -1, window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        }
    }
    if (file_buffer->sw == 0 && num_lines_on_screen(window) > 19) {
        e_pr_str ((window->a.x + 4), window->e.y - 2, "Move", window->colorset->nz.fg_bg_color, 0, -1,
                  window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);

        if (num_cols_on_screen(window) >= 21)
            e_pr_str ((window->a.x + 13), window->e.y - 2, "Remove", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);

        if (num_cols_on_screen(window) >= 30)
            e_pr_str ((window->a.x + 24), window->e.y - 2, "Link", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);

        if (num_cols_on_screen(window) >= 39)
            e_pr_str ((window->a.x + 33), window->e.y - 2, "COpy", window->colorset->nz.fg_bg_color, 1, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);

        if (num_cols_on_screen(window) >= 48)
            e_pr_str ((window->a.x + 42), window->e.y - 2, "Edit", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);

    }
    if (num_cols_on_screen(window) < 45) {
        bx3 = 0;
    }
    if (num_cols_on_screen(window) < 44) {
        bx2 = 0;
    }
    if (num_cols_on_screen(window) < 43) {
        bx1 = 0;
    }
    file_buffer->xfd = (num_cols_on_screen(window) - bx1 - bx2 - bx3 - 6) / 2;
    file_buffer->xdd = num_cols_on_screen(window) - bx1 - bx2 - bx3 - file_buffer->xfd - 6;
    file_buffer->xda = 2 + bx1;
    file_buffer->xfa = 4 + bx1 + bx2 + file_buffer->xdd;

    e_pr_str ((window->a.x + file_buffer->xfa), window->a.y + 2, "Name:", window->colorset->nt.fg_bg_color, 0, 1,
              window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
    /*    e_schr_nchar(file_buffer->rdfile, window->a.x+file_buffer->xfa, window->a.y+3, 0, file_buffer->xfd+1, window->colorset->fr.fg_bg_color);   */
    e_schr_nchar_wsv (file_buffer->rdfile, window->a.x + file_buffer->xfa, window->a.y + 3, 0, file_buffer->xfd + 1,
                      window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color);
    e_pr_str ((window->a.x + file_buffer->xfa), window->a.y + 5, "Files:", window->colorset->nt.fg_bg_color, 0, 1,
              window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);

    e_pr_str ((window->a.x + file_buffer->xda), window->a.y + 2, "Directory:", window->colorset->nt.fg_bg_color, 0, 1,
              window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
    /*    e_schr_nchar(window->dirct, window->a.x+file_buffer->xda, window->a.y+3, 0, file_buffer->xdd+1, window->colorset->fr.fg_bg_color);   */
    e_schr_nchar_wsv (window->dirct, window->a.x + file_buffer->xda, window->a.y + 3, 0, file_buffer->xdd + 1,
                      window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color);
    e_pr_str ((window->a.x + file_buffer->xda), window->a.y + 5, "DirTree:", window->colorset->nt.fg_bg_color, 3, 1,
              window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);

    file_buffer->fw->mxa = window->a.x;
    file_buffer->fw->mxe = window->e.x;
    file_buffer->fw->mya = window->a.y;
    file_buffer->fw->mye = window->e.y;
    file_buffer->fw->xa = window->a.x + file_buffer->xfa;
    file_buffer->fw->xe = file_buffer->fw->xa + file_buffer->xfd;
    file_buffer->fw->ya = window->a.y + 6;
    file_buffer->fw->ye = window->e.y - 2 - by;
    file_buffer->dw->mxa = window->a.x;
    file_buffer->dw->mxe = window->e.x;
    file_buffer->dw->mya = window->a.y;
    file_buffer->dw->mye = window->e.y;
    file_buffer->dw->xa = window->a.x + file_buffer->xda;
    file_buffer->dw->xe = file_buffer->dw->xa + file_buffer->xdd;
    file_buffer->dw->ya = window->a.y + 6;
    file_buffer->dw->ye = window->e.y - 2 - by;

    /* slider bars for file list */
    e_mouse_bar (file_buffer->fw->xe, file_buffer->fw->ya, file_buffer->fw->ye - file_buffer->fw->ya, 0,
                 file_buffer->fw->window->colorset->em.fg_bg_color);
    e_mouse_bar (file_buffer->fw->xa, file_buffer->fw->ye, file_buffer->fw->xe - file_buffer->fw->xa, 1,
                 file_buffer->fw->window->colorset->em.fg_bg_color);
    /* file list window */
    e_pr_file_window (file_buffer->fw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color, window->colorset->frft.fg_bg_color);

    /* slide bars for directory window */
    e_mouse_bar (file_buffer->dw->xe, file_buffer->dw->ya, file_buffer->dw->ye - file_buffer->dw->ya, 0,
                 file_buffer->dw->window->colorset->em.fg_bg_color);
    e_mouse_bar (file_buffer->dw->xa, file_buffer->dw->ye, file_buffer->dw->xe - file_buffer->dw->xa, 1,
                 file_buffer->dw->window->colorset->em.fg_bg_color);
    /* directory window */
    e_pr_file_window (file_buffer->dw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color, window->colorset->frft.fg_bg_color);
    return (0);
}

/* tries to find the required style file manager */
int
WpeCallFileManager (int sw, we_window_t * window)
{
    int i, ret;
    FLBFFR *file_buffer;

    for (i = window->edit_control->mxedt; i > 0; i--)
        if (window->edit_control->window[i]->dtmd == DTMD_FILEMANAGER) {	/* check only file manager windows */
            file_buffer = (FLBFFR *) window->edit_control->window[i]->buffer;
            /* open/new file manager and it is not in save mode */
            if (sw == 0 && file_buffer->sw == sw && window->edit_control->window[i]->save != 1) {
                break;
            }
            /* wastebasket mode required,
               the window is "open/new" file manager and save mode turned on */
            else if (sw == 6 && file_buffer->sw == 0 && window->edit_control->window[i]->save == 1) {
                break;
            }
            /* not open/new or wastebasket filemanager and it is the required style */
            else if (sw != 0 && sw != 6 && file_buffer->sw == sw) {
                break;
            }
        }

    if (i <= 0) {		/* we did not find the required style file-manager */
        if (sw == 6) {	/* wastebasket mode */
            char *tmp;

            if ((tmp = WpeGetWastefile (""))) {
                ret = WpeCreateFileManager (sw, window->edit_control, tmp);
                free (tmp);
                return (ret);
            } else {
                e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
                return 0;		/* Error -- no wastebasket */
            }
        }
        /* create the required style file manager */
        return (WpeCreateFileManager (sw, window->edit_control, ""));
    }
    /* switch to the found file manager */
    e_switch_window (window->edit_control->edt[i], window);
    return (0);
}

/* It will always create a new file manager */
int
WpeManagerFirst (we_window_t * window)
{
    return (WpeCreateFileManager (0, window->edit_control, ""));
}

/* try to find an "open/new" style file manager or create one */
int
WpeManager (we_window_t * window)
{
    return (WpeCallFileManager (0, window));
}

/* try to find an "execute" style file manager or create one */
int
WpeExecuteManager (we_window_t * window)
{
    return (WpeCallFileManager (3, window));
}

/* try to find a "save as" style file manager or create one */
int
WpeSaveAsManager (we_window_t * window)
{
    return (WpeCreateFileManager (4, window->edit_control, ""));
}


/* File Manager Handler */
int
WpeHandleFileManager (we_control_t * control)
{
    we_window_t *window = control->window[control->mxedt], *fe = NULL;
    FLBFFR *file_buffer = (FLBFFR *) window->buffer;
    we_buffer_t *be = NULL;
    we_screen_t *se = NULL;
    int c = AltC, i, j, t;
    int winnum = 0, nco, svmode = -1, fmode, len, start;
    int cold = AltN;
#if MOUSE
    int g[4];
#endif
    char filen[128], *ftmp, *dtp = NULL, *ftp = NULL, *svdir = NULL;
    char *dirtmp = NULL;
    we_view_t *outp = NULL;
    FILE *fp;
    struct stat buf;
    char dtmd;

    /* check whether we really get a file-manager window here */
    if (window->dtmd != DTMD_FILEMANAGER) {
        return (0);
    }

    if (window->save == 1) {
        svmode = window->edit_control->flopt;
        window->edit_control->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                                      FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
    }

    /* if it is project management or saving mode
       save the current directory to return to it */
    if (window->save == 1 || file_buffer->sw == 5) {
        if ((svdir = malloc (strlen (window->edit_control->dirct) + 1)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
        }
        strcpy (svdir, window->edit_control->dirct);
    }

    nco = file_buffer->cd->nr_files - 1;
    /* when searching among files, search hidden ones as well */
    fmode = window->edit_control->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0;
    /* in execution mode show hidden dirs as well */
    if (file_buffer->sw == 3) {
        fmode |= 2;
    }

    /* searching for the last edited/touched file on the desktop */
    for (i = control->mxedt; i > 0; i--) {
        if (DTMD_ISTEXT (control->window[i]->dtmd)) {
            fe = control->window[i];
            be = fe->buffer;
            se = fe->screen;
            winnum = control->edt[i];
            break;
        }
    }
    strcpy (window->find.file, file_buffer->rdfile);

    /* go until quit */
    while (c != WPE_ESC) {
        /* draw out dir tree and file list windows */
        e_pr_file_window (file_buffer->fw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color,
                          window->colorset->frft.fg_bg_color);
        e_pr_file_window (file_buffer->dw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color,
                          window->colorset->frft.fg_bg_color);

        switch (c) {
        /* filename entry box activation */
        case AltN:
            cold = c;
            fk_u_cursor (1);
            /* get some answer from the name entry box,
               result file copied into file_buffer->rdfile, max 79 char + '\0' */
            c =
                e_schr_lst_wsv (file_buffer->rdfile, window->a.x + file_buffer->xfa, window->a.y + 3,
                                file_buffer->xfd + 1, 79, window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color,
                                &window->edit_control->fdf, window);

            /* determine the entered filename, going backward */
            for (i = strlen (file_buffer->rdfile); i >= 0 && file_buffer->rdfile[i] != DIRC; i--)
                ;
            strcpy (window->find.file, file_buffer->rdfile + 1 + i);

            /* there is some directory structure in the filename */
            if (i >= 0) {
                if (i == 0) {
                    i++;
                }
                file_buffer->rdfile[i] = '\0';
                /* change the working directory */
                free (window->dirct);
                if ((window->dirct = malloc (strlen (file_buffer->rdfile) + 1)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
                }
                strcpy (window->dirct, file_buffer->rdfile);

                /* restore original filename */
                strcpy (file_buffer->rdfile, window->find.file);
                c = AltC;
            }
#if  MOUSE
            /* if mouse was used (the reason it returned) get appropriate key interpretation */
            if (c == -1) {
                c = WpeMngMouseInFileManager (window);
            }
#endif
            if ((c >= Alt1 && c <= Alt9) || (c >= 1024 && c <= 1049)) {
                /* window changing, make the entry unhighlighted */
                e_schr_nchar_wsv (file_buffer->rdfile, window->a.x + file_buffer->xfa, window->a.y + 3, 0,
                                  file_buffer->xfd + 1, window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color);
                break;
            }
            if (c == CLE || c == CCLE) {	/* goto dir name window */
                c = AltD;
            } else if (c == CDO || c == BDO || c == WPE_TAB) {	/* goto file list window */
                c = AltF;
            } else if (c == WPE_BTAB) {	/* goto dir tree window */
                c = AltT;
            } else if ((c == WPE_CR
                        || (file_buffer->sw == 0 && c == AltE)
                        || (file_buffer->sw == 1 && c == AltR)
                        || (file_buffer->sw == 2 && c == AltW)
                        || (file_buffer->sw == 3 && c == AltE)
                        || (file_buffer->sw == 5 && c == AltA)
                        || (file_buffer->sw == 4 && (c == AltS || c == AltY)))
                       && (strstr (file_buffer->rdfile, "*")
                           || strstr (file_buffer->rdfile, "?") || strstr (file_buffer->rdfile, "["))) {
                WpeMouseChangeShape (WpeWorkingShape);
                /* free up existing structures */
                freedf (file_buffer->df);
                freedf (file_buffer->fw->df);
                /* find files according to the new pattern */
                file_buffer->df = e_find_files (file_buffer->rdfile, fmode);
                /* setup the drawing in the dir tree window */
                file_buffer->fw->df = WpeGraphicalFileList (file_buffer->df, window->edit_control->flopt >> 9, control);
                file_buffer->fw->ia = file_buffer->fw->nf = 0;
                file_buffer->fw->ja = file_buffer->fw->srcha;
                /* jump to file list window */
                c = AltF;
                WpeMouseRestoreShape ();
            } else {
                strcpy (filen, file_buffer->rdfile);	/* !!! alloc for filen ??? */
                if (c == WPE_CR) {
                    if (file_buffer->sw == 1) {
                        c = AltR;
                    } else if (file_buffer->sw == 2) {
                        c = AltW;
                    } else if (file_buffer->sw == 4) {
                        c = AltS;
                    } else if (file_buffer->sw == 5) {
                        c = AltA;
                    } else {
                        c = AltE;
                    }
                }
            }
            /* entry window is left, make the entry unhighlighted */
            if (c != AltN)
                e_schr_nchar_wsv (file_buffer->rdfile, window->a.x + file_buffer->xfa, window->a.y + 3, 0,
                                  file_buffer->xfd + 1, window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color);
            fk_u_cursor (0);
            break;

        /* directory name entry box activation */
        case AltD:
            cold = c;
            fk_u_cursor (1);

            /* get the directory name */
            if ((dirtmp = malloc (WPE_PATHMAX)) == NULL) {	/* dirct mat not have enough memory */
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }

            if (strlen (window->dirct) >= WPE_PATHMAX) {
                strncpy (dirtmp, window->dirct, WPE_PATHMAX - 1);
                dirtmp[WPE_PATHMAX - 1] = '\0';
            } else {
                strcpy (dirtmp, window->dirct);
            }

            c =
                e_schr_lst_wsv (dirtmp, window->a.x + file_buffer->xda, window->a.y + 3, file_buffer->xdd + 1,
                                WPE_PATHMAX, window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color,
                                &window->edit_control->ddf, window);
            free (window->dirct);
            window->dirct = WpeStrdup (dirtmp);
            free (dirtmp);

#if  MOUSE
            if (c == -1) {
                c = WpeMngMouseInFileManager (window);
            }
#endif
            if (c == CRI || c == CCRI) {	/* goto name entry windwow */
                c = AltN;
            } else if (c == CDO || c == BDO || c == WPE_TAB) {	/* goto dir tree window */
                c = AltT;
            } else if (c == WPE_BTAB) {	/* goto file list window */
                c = AltF;
            } else if (c == WPE_CR) {	/* change dir */
                c = AltC;
            }
            /* window left, make the entry unhighlighted */
            if (c != AltD)
                e_schr_nchar_wsv (window->dirct, window->a.x + file_buffer->xda, window->a.y + 3, 0,
                                  file_buffer->xdd + 1, window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color);
            fk_u_cursor (0);
            break;

        /* directory tree list window activation */
        case AltT:
            cold = c;
            c = e_file_window (1, file_buffer->dw, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color);
#if  MOUSE
            if (c == MBKEY) {	/* handle mouse actions in the window */
                c = WpeMngMouseInFileManager (window);
            } else if (c < 0) {
                c = WpeMouseInFileDirList (c, 1, window);
            }
#endif
            if (c == CCRI) {
                c = AltF;
            } else if (c == BUP) {
                c = AltD;
            } else if (c == WPE_TAB) {
                c = AltN;
            } else if (c == WPE_BTAB) {
                c = AltD;
            } else if (c == AltC || (c == WPE_CR && file_buffer->dw->nf != nco)) {
                if ((dirtmp =
                            WpeAssemblePath (window->dirct, file_buffer->cd, file_buffer->dd, file_buffer->dw->nf, window))) {
                    free (window->dirct);
                    window->dirct = dirtmp;
                    e_schr_nchar_wsv (window->dirct, window->a.x + file_buffer->xda, window->a.y + 3, 0,
                                      file_buffer->xdd + 1, window->colorset->fr.fg_bg_color, window->colorset->fz.fg_bg_color);
                    window->edit_control->ddf = e_add_df (window->dirct, window->edit_control->ddf);
                    c = AltC;
                }
                /* there is only one case when it cannot assemble the path,
                   that it cannot access wastebasket, then quit the file manager */
                else {
                    c = WPE_ESC;
                }
            } else if (c == WPE_CR) {
                c = AltT;
            }
            break;

        /* file list window activation */
        case AltF:
            if (file_buffer->df->nr_files < 1) {
                c = cold;
                break;
            }
            cold = c;
            c = e_file_window (1, file_buffer->fw, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color);
#if  MOUSE
            if (c == MBKEY) {
                c = WpeMngMouseInFileManager (window);
            } else if (c < 0) {
                c = WpeMouseInFileDirList (c, 0, window);
            }
#endif
            if (c == BUP) {	/* goto file name entry window */
                c = AltN;
            } else if (c == CCLE) {	/* goto dir tree window */
                c = AltT;
            } else if (c == WPE_TAB) {	/* goto dir entry window */
                c = AltD;
            } else if (c == WPE_BTAB) {	/* goto file name entry window */
                c = AltN;
            } else if (c == WPE_CR) {	/* action selected */
                if (file_buffer->sw == 1) {
                    c = AltR;
                } else if (file_buffer->sw == 2) {
                    c = AltW;
                } else if (file_buffer->sw == 4) {
                    c = AltS;
                } else if (file_buffer->sw == 5) {
                    c = AltA;
                } else {
                    c = AltE;
                }
            }
            if ((file_buffer->sw == 1 && c == AltR)	/* in case of action store the filename */
                    || (file_buffer->sw == 2 && c == AltW)
                    || (file_buffer->sw == 3 && c == AltE)
                    || (file_buffer->sw == 0 && c == AltE)
                    || (file_buffer->sw == 4 && (c == AltS || c == AltY))) {
                strcpy (filen, *(file_buffer->df->name + file_buffer->fw->nf));	/* !!! alloc for filen ??? */
            }
            break;

        /* change dir button activation */
        case AltC:
            c = cold;
            /* if the current dir is equal to the "newly" entered dir, break */
            if (!strcmp (window->edit_control->dirct, window->dirct)) {
                break;
            }

            /* in wastebasket mode, we do not allow to go out of it through
               a soft link */
            if ((file_buffer->sw == 0) && (window->save == 1)) {
                if (lstat (window->dirct, &buf)) {
                    e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->edit_control->colorset);
                    break;
                }
                if (S_ISLNK (buf.st_mode)) {
                    /* cannot go out through a softlink, restore dir name */
                    if ((window->dirct =
                                realloc (window->dirct, strlen (window->edit_control->dirct) + 1)) == NULL) {
                        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
                    } else {
                        strcpy (window->dirct, window->edit_control->dirct);
                    }
                    break;
                }
            }

            /* change to the desired dir with system error checking */
            if (chdir (window->dirct)) {
                e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, window->colorset);

                /* we cannot determine where we are, try the home */
                if ((dirtmp = getenv ("HOME")) == NULL) {
                    e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                }
                if (chdir (dirtmp)) {
                    e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                }
            }

            /* get current directory */
            dirtmp = WpeGetCurrentDir (window->edit_control);

            /* change the shape of the mouse */
            WpeMouseChangeShape (WpeWorkingShape);

            free (window->dirct);
            window->dirct = dirtmp;

            /* free up all relevant structures */
            freedf (file_buffer->df);
            freedf (file_buffer->fw->df);
            freedf (file_buffer->cd);
            freedf (file_buffer->dw->df);
            freedf (file_buffer->dd);

            /* reset the current dir path in the control structure */
            if ((window->edit_control->dirct =
                        realloc (window->edit_control->dirct, strlen (window->dirct) + 1)) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
            } else {
                strcpy (window->edit_control->dirct, window->dirct);
            }

            /* setup current directory structure */
            file_buffer->cd = WpeCreateWorkingDirTree (window->save, control);
            /* find all other directories in the current dir */
            file_buffer->dd =
                e_find_dir (SUDIR, window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
            /* setup the drawing in the dir tree window */
            file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);

            nco = file_buffer->dw->nf = file_buffer->cd->nr_files - 1;
            file_buffer->dw->ia = file_buffer->dw->ja = 0;

            /* finds all files matching the pattern */
            file_buffer->df = e_find_files (file_buffer->rdfile, fmode);
            /* setup the drawing in the file list window */
            file_buffer->fw->df = WpeGraphicalFileList (file_buffer->df, window->edit_control->flopt >> 9, control);
            file_buffer->fw->nf = file_buffer->fw->ia = 0;
            file_buffer->fw->ja = 12;

            /* change the shape of the mouse back */
            WpeMouseRestoreShape ();
            break;

        /* link file activation */
        case AltL:
        /* copy file activation */
        case AltO:
        /* move file activation */
        case AltM:
            /* moving/copying a file is valid only in mode 0 (open/new file) */
            if (file_buffer->sw != 0) {
                c = cold;
                break;
            }
            j = c;

            /* we are coming from files */
            if (cold == AltF) {
                if ((ftmp = malloc (129)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
                }

                if (strlen (*(file_buffer->df->name + file_buffer->fw->nf)) > 128) {
                    strncpy (ftmp, *(file_buffer->df->name + file_buffer->fw->nf), 128);
                    ftmp[128] = '\0';
                } else {
                    strcpy (ftmp, *(file_buffer->df->name + file_buffer->fw->nf));
                }

                /* make the file name editable */
                c =
                    e_schreib_leiste (ftmp, file_buffer->fw->xa,
                                      file_buffer->fw->ya + file_buffer->fw->nf - file_buffer->fw->ia,
                                      file_buffer->fw->xe - file_buffer->fw->xa, 128, window->colorset->fr.fg_bg_color,
                                      window->colorset->fz.fg_bg_color);
                if (c == WPE_CR) {
                    if (j == AltM) {
                        e_rename (*(file_buffer->df->name + file_buffer->fw->nf), ftmp, window);    /* move */
                    } else if (j == AltL)
                        WpeLinkFile (*(file_buffer->df->name + file_buffer->fw->nf), ftmp,	/* link */
                                     window->edit_control->flopt & FM_TRY_HARDLINK, window);
                    else if (j == AltO) {
                        e_copy (*(file_buffer->df->name + file_buffer->fw->nf), ftmp, window);    /* copy */
                    }

                    /* after copying/moving/linking, free up the old structures */
                    freedf (file_buffer->df);
                    freedf (file_buffer->fw->df);

                    /* generate the new file list */
                    file_buffer->df = e_find_files (file_buffer->rdfile, fmode);
                    /* setup the drawing of it */
                    file_buffer->fw->df =
                        WpeGraphicalFileList (file_buffer->df, window->edit_control->flopt >> 9, control);
                    file_buffer->fw->ia = file_buffer->fw->nf = 0;
                    file_buffer->fw->ja = file_buffer->fw->srcha;
                }
                free (ftmp);
            }
            /* we are coming from dirs */
            else if (cold == AltT && file_buffer->dw->nf >= file_buffer->cd->nr_files) {
                if ((ftmp = malloc (129)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
                }

                /* selected dir */
                t = file_buffer->dw->nf - file_buffer->cd->nr_files;

                if (strlen (*(file_buffer->dd->name + t)) > 128) {
                    strncpy (ftmp, *(file_buffer->dd->name + t), 128);
                    ftmp[128] = '\0';
                } else {
                    strcpy (ftmp, *(file_buffer->dd->name + t));
                }


                /* separate the dir name from other drawings in the line */
                for (i = 0; *(file_buffer->dw->df->name[file_buffer->dw->nf] + i) &&
                        (*(file_buffer->dw->df->name[file_buffer->dw->nf] + i) <= 32 ||
                         *(file_buffer->dw->df->name[file_buffer->dw->nf] + i) >= 127); i++)
                    ;

                if (!WpeIsXwin ()) {
                    i += 3;
                }
                file_buffer->dw->ja = i;
                e_pr_file_window (file_buffer->dw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color,
                                  window->colorset->frft.fg_bg_color);
                /* make the name editable */
                c =
                    e_schreib_leiste (ftmp, file_buffer->dw->xa,
                                      file_buffer->dw->ya + file_buffer->dw->nf - file_buffer->dw->ia,
                                      file_buffer->dw->xe - file_buffer->dw->xa, 128, window->colorset->fr.fg_bg_color,
                                      window->colorset->fz.fg_bg_color);
                if (c == WPE_CR) {
                    if (j == AltM) {
                        e_rename (*(file_buffer->dd->name + t), ftmp, window);    /* move */
                    } else if (j == AltL) {
                        e_link (*(file_buffer->dd->name + t), ftmp, window);    /* link */
                    } else if (j == AltO) {
                        e_copy (*(file_buffer->dd->name + t), ftmp, window);    /* copy */
                    }

                    /* free up structures */
                    freedf (file_buffer->cd);
                    freedf (file_buffer->dw->df);
                    freedf (file_buffer->dd);

                    /* determine current directory */
                    file_buffer->cd = WpeCreateWorkingDirTree (window->save, control);
                    /* find all other directories in the current */
                    file_buffer->dd =
                        e_find_dir (SUDIR,
                                    window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
                    /* setup drawing */
                    file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);
                    nco = file_buffer->dw->nf = file_buffer->cd->nr_files - 1;
                    file_buffer->dw->ia = file_buffer->dw->ja = 0;
                }
                free (ftmp);
            }
            c = cold;
            cold = AltN;		/* go back to name entry */
            break;

        /* remove button activation */
        case ENTF:
            if (file_buffer->sw != 0) {
                c = cold;
                break;
            }
        /* remove button activation */
        case AltR:
            if (file_buffer->sw == 0) {
                /* coming from file list */
                if (cold == AltF) {
                    WpeRemove (*(file_buffer->df->name + file_buffer->fw->nf), window);	/* remove the file */

                    /* free up structures */
                    freedf (file_buffer->df);
                    freedf (file_buffer->fw->df);

                    /* find files according to the pattern */
                    file_buffer->df = e_find_files (file_buffer->rdfile, fmode);

                    /* setup drawing */
                    file_buffer->fw->df =
                        WpeGraphicalFileList (file_buffer->df, window->edit_control->flopt >> 9, control);
                    file_buffer->fw->ia = file_buffer->fw->nf = 0;
                    file_buffer->fw->ja = file_buffer->fw->srcha;
                }
                /* coming from the dir tree list and the selected dir is a subdir of
                   the current directory */
                else if (cold == AltT && file_buffer->dw->nf >= file_buffer->cd->nr_files) {
                    t = file_buffer->dw->nf - file_buffer->cd->nr_files;
                    WpeRemove (*(file_buffer->dd->name + t), window);	/* remove the dir */

                    /* free up structures */
                    freedf (file_buffer->cd);
                    freedf (file_buffer->dw->df);
                    freedf (file_buffer->dd);

                    /* determine current directory */
                    file_buffer->cd = WpeCreateWorkingDirTree (window->save, control);
                    /* find all other directories in the current */
                    file_buffer->dd =
                        e_find_dir (SUDIR,
                                    window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
                    /* setup drawing */
                    file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);
                    nco = file_buffer->dw->nf = file_buffer->cd->nr_files - 1;
                    file_buffer->dw->ia = file_buffer->dw->ja = 0;
                }
                c = cold;
                cold = AltN;	/* go back to name entry */
                break;
            }

        /* edit/execute button activation */
        case AltE:
            if ((c == AltE && file_buffer->sw != 0 && file_buffer->sw != 3)
                    || (c == AltR && file_buffer->sw != 1)) {
                c = cold;
                break;
            }
            if (file_buffer->sw == 3) {	/* file-manager in execution mode */
                if (cold == AltF) {
                    strcpy (filen, *(file_buffer->df->name + file_buffer->fw->nf));    /* !!! alloc filen ??? */
                }
                if (!WpeIsXwin ()) {
                    outp =
                        e_open_view (0, 0, max_screen_cols() - 1, max_screen_lines() - 1, window->colorset->ws,
                                     1);
                    fk_u_locate (0, 0);
                    fk_u_cursor (1);
#if  MOUSE
                    g[0] = 2;
                    fk_mouse (g);
#endif
                    e_u_sys_ini ();
                    printf (e_msg[ERR_EXEC], filen);
                    fflush (stdout);
                }
                if (e_u_system (filen)) {
                    if (!WpeIsXwin ()) {
                        e_u_sys_end ();
                    }
                    e_error (e_msg[ERR_COMMAND], ERROR_MSG, window->colorset);
                } else if (!WpeIsXwin ()) {
                    printf ("%s", e_msg[ERR_HITCR]);
                    fflush (stderr);
                    fflush (stdout);
                    fk_getch ();
                }
                if (!WpeIsXwin ()) {
                    e_u_sys_end ();
                    e_close_view (outp, 1);
                    fk_u_cursor (0);
#if MOUSE
                    g[0] = 1;
                    fk_mouse (g);
#endif
                }
                c = cold;
                break;
            } else {
                /* if there is only a pattern get back to file selection */
                if (strstr (filen, "*") || strstr (filen, "?")) {
                    c = AltF;
                    break;
                }
                /* there is no open ??? file */
                if (file_buffer->sw == 0 || !fe) {
                    /* close on open request */
                    if (window->edit_control->flopt & FM_CLOSE_WINDOW) {
                        e_close_window (window);
                    }
                    /* editing the file */
                    e_edit (control, filen);
                } else {
                    /* try to open the file, no success return */
                    if ((fp = fopen (filen, "rb")) == NULL) {
                        e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->colorset);
                        c = cold;
                        break;
                    }
                    if (access (filen, W_OK) != 0) {
                        window->ins = 8;
                    }
                    e_close_window (window);
                    e_switch_window (winnum, fe);
                    fe = control->window[control->mxedt];
                    be = fe->buffer;
                    se = fe->screen;
                    window = control->window[control->mxedt];
                    if (be->cursor.x != 0) {
                        e_new_line (be->cursor.y + 1, be);
                        if (*(be->buflines[be->cursor.y].s + be->buflines[be->cursor.y].len) != '\0') {
                            (be->buflines[be->cursor.y].len)++;
                        }
                        for (i = be->cursor.x; i <= be->buflines[be->cursor.y].len; i++)
                            *(be->buflines[be->cursor.y + 1].s + i - be->cursor.x) =
                                *(be->buflines[be->cursor.y].s + i);
                        *(be->buflines[be->cursor.y].s + be->cursor.x) = '\0';
                        be->buflines[be->cursor.y].len = be->cursor.x;
                        be->buflines[be->cursor.y + 1].len =
                            e_str_len (be->buflines[be->cursor.y + 1].s);
                        be->buflines[be->cursor.y + 1].nrc =
                            strlen ((const char *) be->buflines[be->cursor.y + 1].s);
                    }
                    se->mark_begin.x = be->cursor.x;
                    start = se->mark_begin.y = be->cursor.y;
                    dtmd = fe->dtmd;
                    se->mark_end = e_readin (be->cursor.x, be->cursor.y, fp, be, &dtmd);
                    fclose (fp);

                    if (se->mark_begin.x > 0) {
                        start++;
                    }
                    len = se->mark_end.y - start;
                    e_brk_recalc (window, start, len);

                    e_write_screen (fe, 1);
                }

                /* if there was no error */
                dirtmp = WpeGetCurrentDir (control);
                free (control->dirct);
                control->dirct = dirtmp;

                if (svmode >= 0) {
                    control->flopt = svmode;
                }

                if (svdir != NULL) {
                    /* go back to the saved directory */
                    if (chdir (svdir)) {
                        e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, control->colorset);

                        /* we cannot determine where we are, try the home */
                        if ((dirtmp = getenv ("HOME")) == NULL) {
                            e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                        }
                        if (chdir (dirtmp)) {
                            e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                        }
                    }

                    /* determine current dir */
                    dirtmp = WpeGetCurrentDir (control);

                    free (control->dirct);
                    control->dirct = dirtmp;

                    free (svdir);
                    svdir = NULL;
                }

                return (0);
            }

        case AltW:		/* write activation */
        case AltS:		/* save activation */
            if ((c == AltW && file_buffer->sw != 2)
                    || (c == AltS && file_buffer->sw != 4) || !fe || fe->ins == 8) {
                c = cold;
                break;
            }
            /* only file pattern, return */
            if (strstr (filen, "*") || strstr (filen, "?")) {
                c = AltF;
                break;
            }
            /* check whether the file exist */
            if (!access (filen, F_OK)) {
                if ((ftmp = malloc (strlen (filen) + 42)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
                }

                sprintf (ftmp, "File %s exist\nDo you want to overwrite it ?",
                         filen);
                i = e_message (1, ftmp, window);
                free (ftmp);

                if (i == WPE_ESC) {
                    c = WPE_ESC;
                    break;
                } else if (i == 'N') {
                    c = AltF;
                    break;
                }
            }
            if (file_buffer->sw != 4) {
                dtp = fe->dirct;
                ftp = fe->datnam;
            } else {		/* save as mode, current dir and window header will/may change */
                free (fe->dirct);
                free (fe->datnam);
            }

            WpeFilenameToPathFile (filen, &fe->dirct, &fe->datnam);
            if (file_buffer->sw == 4) {	/* save as mode */
                e_save (fe);
            } else {
                e_write (se->mark_begin.x, se->mark_begin.y, se->mark_end.x,
                         se->mark_end.y, fe, WPE_BACKUP);
                free (fe->dirct);	/* restore current dir window header */
                free (fe->datnam);
                fe->dirct = dtp;
                fe->datnam = ftp;
            }

            if (file_buffer->sw == 4 && (window->edit_control->edopt & ED_SYNTAX_HIGHLIGHT)) {
                if (fe->c_sw) {
                    free (fe->c_sw);
                }
                if (WpeIsProg ()) {
                    e_add_synt_tl (fe->datnam, fe);
                }
                if (fe->c_st) {
                    if (fe->c_sw) {
                        free (fe->c_sw);
                    }
                    fe->c_sw = e_sc_txt (NULL, fe->buffer);
                }
                e_rep_win_tree (window->edit_control);
            }
            if (svmode >= 0) {
                window->edit_control->flopt = svmode;
            }

            if (svdir != NULL) {
                /* go back to the saved directory */
                if (chdir (svdir)) {
                    e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, control->colorset);

                    /* we cannot determine where we are, try the home */
                    if ((dirtmp = getenv ("HOME")) == NULL) {
                        e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                    }
                    if (chdir (dirtmp)) {
                        e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                    }
                }

                /* determine current dir */
                dirtmp = WpeGetCurrentDir (control);

                free (control->dirct);
                control->dirct = dirtmp;

                free (svdir);
                svdir = NULL;
            }

            e_close_window (window);
            return (0);

        /* make dir button activation */
        case EINFG:
        case AltK:
            if (file_buffer->sw != 0) {
                c = cold;
                break;
            }
            /* create new directory */
            if (WpeMakeNewDir (window) != 0) {
                c = cold;
                break;
            }

            /* free up old structures */
            freedf (file_buffer->dd);
            freedf (file_buffer->dw->df);

            /* create new directory structure */
            file_buffer->dd =
                e_find_dir (SUDIR, window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
            file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);
            /* go to the line where the new dir is */
            for (i = 0; i < file_buffer->dd->nr_files && strcmp (file_buffer->dd->name[i], "new.dir");
                    i++)
                ;
            /* set the slidebar variables */
            if ((file_buffer->dw->nf = file_buffer->cd->nr_files + i) >= file_buffer->dw->df->nr_files) {
                file_buffer->dw->nf = file_buffer->cd->nr_files - 1;
            }
            if (file_buffer->dw->nf - file_buffer->dw->ia >= file_buffer->dw->ye - file_buffer->dw->ya) {
                file_buffer->dw->ia = file_buffer->dw->nf + file_buffer->dw->ya - file_buffer->dw->ye + 1;
            } else if (file_buffer->dw->nf - file_buffer->dw->ia < 0) {
                file_buffer->dw->ia = file_buffer->dw->nf;
            }
            cold = AltT;
            /* let the user modify the newly created dir */
            c = AltM;
            break;

        /* attribute/add file button activation */
        case AltA:
            /* not valid mode */
            if (file_buffer->sw != 0 && file_buffer->sw != 5) {
                c = cold;
                break;
            }
            /* attribute button */
            if (file_buffer->sw == 0) {
                if (cold == AltF) {	/* coming from file list */
                    strcpy (filen, *(file_buffer->df->name + file_buffer->fw->nf));	/* alloc for filen ??? */
                    /* change the file attributes */
                    WpeFileDirAttributes (filen, window);

                    /* free up old file list structures */
                    freedf (file_buffer->df);
                    freedf (file_buffer->fw->df);

                    /* create new file list */
                    file_buffer->df = e_find_files (file_buffer->rdfile, fmode);
                    /* setup drawing */
                    file_buffer->fw->df =
                        WpeGraphicalFileList (file_buffer->df, window->edit_control->flopt >> 9, control);
                } else if (cold == AltT && file_buffer->dw->nf >= file_buffer->cd->nr_files) {	/* coming from dir tree */
                    t = file_buffer->dw->nf - file_buffer->cd->nr_files;

                    if ((ftmp =
                                malloc (strlen (*(file_buffer->dd->name + t)) + 1)) == NULL) {
                        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
                    }

                    strcpy (ftmp, *(file_buffer->dd->name + t));
                    /* change the dir attributes */
                    WpeFileDirAttributes (ftmp, window);

                    free (ftmp);

                    /* free up old file list structures */
                    freedf (file_buffer->dd);
                    freedf (file_buffer->dw->df);
                    /* create new dir list */
                    file_buffer->dd =
                        e_find_dir (SUDIR,
                                    window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
                    /* setup drawing */
                    file_buffer->dw->df = WpeGraphicalDirTree (file_buffer->cd, file_buffer->dd, control);
                }
                c = cold;
            } else if (file_buffer->sw == 5) {	/* it is in project management */
                FLWND *fw = (FLWND *) control->window[control->mxedt - 1]->buffer;
                if (cold != AltN) {
                    strcpy (filen, *(file_buffer->df->name + file_buffer->fw->nf));
                }
                dirtmp = control->window[control->mxedt - 1]->dirct;
                ftmp = malloc (strlen (window->dirct) + strlen (filen) + 2);
                len = strlen (dirtmp);
                if (strncmp (dirtmp, window->dirct, len) == 0) {
                    /* Make path relative to project directory */
                    sprintf (ftmp, "%s%s", window->dirct + len, filen);
                } else {
                    /* Full path */
                    sprintf (ftmp, "%s%s", window->dirct, filen);
                }
                fw->df->nr_files++;
                fw->df->name =
                    realloc (fw->df->name, fw->df->nr_files * sizeof (char *));
                for (i = fw->df->nr_files - 1; i > fw->nf; i--) {
                    fw->df->name[i] = fw->df->name[i - 1];
                }
                fw->df->name[i] = ftmp;
                /* Don't bother notifying the user for each file added to project
                   sprintf(ftmp, "File added to Project:\n%s",
                   fw->df->name[i]);
                   e_message(0, ftmp, window); */
                fw->nf++;
                if (fw->nf - fw->ia >= fw->ye - fw->ya) {
                    fw->ia = fw->nf + fw->ya - fw->ye + 1;
                }
                c = AltN;
            }
            break;

        /* like save */
        case F2:
            c = AltS;
            break;
        /* leave file manager */
        case AltBl:
            c = WPE_ESC;

        default:
            /* not project management */
            if (file_buffer->sw != 5) {
                if (svmode >= 0) {
                    /* restore options */
                    window->edit_control->flopt = svmode;
                }

                if (svdir != NULL) {
                    /* go back to the saved directory */
                    if (chdir (svdir)) {
                        e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, control->colorset);

                        /* we cannot determine where we are, try the home */
                        if ((dirtmp = getenv ("HOME")) == NULL) {
                            e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                        }
                        if (chdir (dirtmp)) {
                            e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                        }
                    }

                    /* determine current dir */
                    dirtmp = WpeGetCurrentDir (window->edit_control);
                    free (window->edit_control->dirct);
                    window->edit_control->dirct = dirtmp;
                }


                /* the key dispatcher returns zero when something has happened
                   and this means that the file-manager lost focus, just return */
#ifdef PROG
                if (!e_tst_dfkt (window, c)
                        || (WpeIsProg () && !e_prog_switch (window, c)))
#else
                if (!e_tst_dfkt (window, c))
#endif
                {
                    if (svdir != NULL) {
                        free (svdir);
                        svdir = NULL;
                    }

                    return (0);
                }
                /* file manager is still active */
                if (svmode >= 0)
                    window->edit_control->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                                                  FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
            } else if (c == WPE_ESC
                       || (!(window->edit_control->edopt & ED_CUA_STYLE) && c == AF3)
                       || (window->edit_control->edopt & ED_CUA_STYLE && c == CF4)) {
                if (svdir != NULL) {
                    /* go back to the saved directory */
                    if (chdir (svdir)) {
                        e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, control->colorset);

                        /* we cannot determine where we are, try the home */
                        if ((dirtmp = getenv ("HOME")) == NULL) {
                            e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                        }
                        if (chdir (dirtmp)) {
                            e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                        }
                    }

                    /* determine current dir */
                    dirtmp = WpeGetCurrentDir (control);

                    free (control->dirct);
                    control->dirct = dirtmp;

                    free (svdir);
                    svdir = NULL;
                }

                /* close file manager */
                e_close_window (window);

                /* restore options */
                if (svmode >= 0) {
                    control->flopt = svmode;
                }

                return (WPE_ESC);
            }
            c = cold;
            break;
        }
    }

    /* change to saved directory and lets hope it works */
    if (svdir != NULL) {
        /* go back to the saved directory */
        if (chdir (svdir)) {
            e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, control->colorset);

            /* we cannot determine where we are, try the home */
            if ((dirtmp = getenv ("HOME")) == NULL) {
                e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
            }
            if (chdir (dirtmp)) {
                e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
            }
        }

        /* determine current dir */
        dirtmp = WpeGetCurrentDir (control);

        free (control->dirct);
        control->dirct = dirtmp;

        free (svdir);
        svdir = NULL;
    }

    /* restore options */
    if (svmode >= 0) {
        control->flopt = svmode;
    }

    /* if in save mode or project management */
    if (window->save == 1 || file_buffer->sw == 5) {
        e_close_window (window);
        return (WPE_ESC);
    } else {
        e_close_window (window);
        return (0);
    }
}


int
WpeGrepFile (char *file, char *string, int sw)
{
    FILE *fp;
    char str[256];
    int ret;
    size_t end_string;

    if ((fp = fopen (file, "r")) == NULL) {
        return (0);
    }

    end_string = strlen (string);
    while (fgets (str, 256, fp)) {
        if (!find_regular_expression(sw)) {
            ret =
                e_strstr (0, strlen (str), (unsigned char *) str,
                          (unsigned char *) string, &end_string,
                          find_case_sensitive(sw));
        } else {
            ret =
                e_rstrstr (0, strlen (str), (unsigned char *) str,
                           (unsigned char *) string, &end_string, find_case_sensitive(sw));
        }
        if (ret >= 0
                && (!(sw & 64)
                    || (isalnum (str[ret + end_string]) == 0
                        && (ret == 0 || isalnum (str[ret - 1]) == 0)))) {
            fclose (fp);
            return (1);
        }
    }
    fclose (fp);
    return (0);
}

int
WpeMakeNewDir (we_window_t * window)
{
    char *dirct;
    int msk, mode, ret;
    struct stat buf;

    umask (msk = umask (077));
    mode = 0777 & ~msk;

    if ((dirct = malloc (strlen (window->dirct) + 9)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }

    if (window->dirct[strlen (window->dirct) - 1] != DIRC) {
        sprintf (dirct, "%s/new.dir", window->dirct);
    } else {
        sprintf (dirct, "%snew.dir", window->dirct);
    }

    /* check existence and status (whether it is directory) */
    if (stat (dirct, &buf)) {
        if ((ret = mkdir (dirct, mode)) != 0) {
            e_error (e_msg[ERR_NONEWDIR], ERROR_MSG, window->edit_control->colorset);
        }
    } else {
        /* check whether the existing file is a directory */
        if (!(buf.st_mode & S_IFDIR)) {
            e_error (e_msg[ERR_NEWDIREXIST], ERROR_MSG, window->edit_control->colorset);
            ret = 1;
        } else {
            ret = 0;
        }
    }

    free (dirct);
    return (ret);
}

int
WpeFileDirAttributes (char *filen, we_window_t * window)
{
    struct stat buf[1];
    int mode, ret;
    W_OPTSTR *o = e_init_opt_kst (window);

    /* if cannot access or error */
    if (stat (filen, buf)) {
        e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->edit_control->colorset);
        return 1;
    }
    mode = buf->st_mode;
    if (o == NULL) {
        return (1);
    }
    o->xa = 14;
    o->ya = 4;
    o->xe = 62;
    o->ye = 13;
    o->bgsw = AltO;
    o->name = "Attributes";
    o->crsw = AltO;
    e_add_txtstr (3, 2, "User:", o);
    e_add_txtstr (33, 2, "Other:", o);
    e_add_txtstr (18, 2, "Group:", o);
    e_add_sswstr (4, 3, 0, AltR, mode & 256 ? 1 : 0, "Read   ", o);
    e_add_sswstr (4, 4, 0, AltW, mode & 128 ? 1 : 0, "Write  ", o);
    e_add_sswstr (4, 5, 1, AltX, mode & 64 ? 1 : 0, "EXecute", o);
    e_add_sswstr (19, 3, 2, AltA, mode & 32 ? 1 : 0, "ReAd   ", o);
    e_add_sswstr (19, 4, 2, AltI, mode & 16 ? 1 : 0, "WrIte  ", o);
    e_add_sswstr (19, 5, 0, AltE, mode & 8 ? 1 : 0, "Execute", o);
    e_add_sswstr (34, 3, 3, AltD, mode & 4 ? 1 : 0, "ReaD   ", o);
    e_add_sswstr (34, 4, 3, AltT, mode & 2 ? 1 : 0, "WriTe  ", o);
    e_add_sswstr (34, 5, 4, AltU, mode & 1, "ExecUte", o);
    e_add_bttstr (12, 7, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (29, 7, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        mode = (o->sstr[0]->num << 8) + (o->sstr[1]->num << 7) +
               (o->sstr[2]->num << 6) + (o->sstr[3]->num << 5) +
               (o->sstr[4]->num << 4) + (o->sstr[5]->num << 3) +
               (o->sstr[6]->num << 2) + (o->sstr[7]->num << 1) + o->sstr[8]->num;
        if (chmod (filen, mode)) {
            e_error (e_msg[ERR_CHGPERM], ERROR_MSG, window->edit_control->colorset);
            freeostr (o);
            return (1);
        }
    }
    freeostr (o);
    return (0);
}

/* determines the current working directory,
   it returns the string
   in case of an error - if the directory cannot be accessed try to access HOME,
                         if it is impossible, give it up and exit with "system error"
                       - otherwise exit with "system error"
 */
char *
WpeGetCurrentDir (we_control_t * control)
{
    int allocate_size;
    char *current_dir = NULL, *check_dir, *dirtmp;
    short home;

    home = 0;
    allocate_size = 256;
    if ((current_dir = (char *) malloc (allocate_size + 1)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
        return NULL;
    }

    do {
        /* allocate space for the directory name */
        if ((current_dir =
                    (char *) realloc (current_dir, allocate_size + 1)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            return NULL;
        }

        check_dir = getcwd (current_dir, allocate_size);
        if (check_dir == NULL) {
            switch (errno) {
            case EACCES:	/* directory cannot be read */
                if (home == 0) {
                    e_error (e_msg[ERR_WORKDIRACCESS], ERROR_MSG, control->colorset);

                    /* we cannot determine where we are, try the home */
                    if ((dirtmp = getenv ("HOME")) == NULL) {
                        e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                    }
                    if (chdir (dirtmp)) {
                        e_error (e_msg[ERR_HOMEDIRACCESS], SERIOUS_ERROR_MSG, control->colorset);
                    }

                    home = 1;
                } else {
                    e_error (e_msg[ERR_SYSTEM], SERIOUS_ERROR_MSG, control->colorset);    /* will not return */
                }
                break;
            case EINVAL:	/* size is equal to 0 */
                allocate_size = 256;	/* impossible !!! */
                break;
            case ERANGE:	/* not enough space for the pathname */
                allocate_size <<= 1;
                break;
            default:		/* System error */
                e_error (e_msg[ERR_SYSTEM], SERIOUS_ERROR_MSG, control->colorset);	/* will not return */
                break;
            }
        }
    } while (check_dir == NULL);

    if (current_dir[strlen (current_dir) - 1] != DIRC) {
        strcat (current_dir, DIRS);
    }
    return (current_dir);
}

/* It always returns the assembled file path, except
   when it is in wastebasket mode and there is an error, return NULL */
char *
WpeAssemblePath (char *pth, struct dirfile *cd, struct dirfile *dd, int n,
                 we_window_t * window)
{
    int i = 0, k = 0, j = 0, t;
    char *adir = NULL;		/* assembled directory */
    int totall = 0;

    UNUSED (pth);

#ifdef UNIX
    if (!strcmp (cd->name[0], "Wastebasket")) {
        if ((adir = WpeGetWastefile (""))) {
            totall = strlen (adir) + 16;
            if ((adir = realloc (adir, totall)) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
            }

            strcat (adir, DIRS);
            i = strlen (adir);
            k++;
        } else {
            /* Error failed to find wastebasket */
            e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
            return NULL;
        }
    }
#endif
    for (; k <= n && k < cd->nr_files; i++, j++) {
        if (i >= totall - 1) {
            totall += 16;
            if ((adir = realloc (adir, totall)) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
            }
        }
        *(adir + i) = *(*(cd->name + k) + j);

        if (*(adir + i) == '\0' || *(adir + i) == DIRC) {
            *(adir + i) = DIRC;
            k++;
            j = -1;
        }
    }
    if (n >= k) {
        t = n - cd->nr_files;
        j = 0;

        do {
            if (i >= totall - 2) {
                totall += 16;
                if ((adir = realloc (adir, totall)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->colorset);
                }
            }
            *(adir + i) = *(*(dd->name + t) + j);
            i++;
            j++;
        } while (*(adir + i - 1) != '\0');

        /*
            for(j = 0; (*(pth + i) = *(*(dd->name + t) + j)) != '\0'; i++, j++)
              ;
        */

    }
    if (n == 0 || n >= k) {
        i++;
    }
    *(adir + i - 1) = '\0';
    return (adir);
}


/* create tree structure up to working directory */
struct dirfile *
WpeCreateWorkingDirTree (int sw, we_control_t * control)
{
    struct dirfile *df;
    char *buf;
    char *tmp, *tmp2;
    char **dftmp;
    int buflen = 256;
    int maxd = 10;		/* inital number of directory levels */
    int i, j, k;

    buf = WpeGetCurrentDir (control);

    buflen = strlen (buf);
    if ((tmp = malloc (buflen + 1)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    /* initialise directory list */
    if (((df = malloc (sizeof (struct dirfile))) == NULL)
            || ((df->name = malloc (sizeof (char *) * maxd)) == NULL)) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    df->nr_files = 0;

    if (sw == 1) {		/* file-manager open to wastebasket */
        /* instead of saving the real wastebasket dir into the structure
           the "Wastebasket" name will appear */
        if ((tmp2 = WpeGetWastefile ("")) == NULL) {
            e_error (e_msg[ERR_NOWASTE], ERROR_MSG, control->colorset);	/* more error check ??? */
            i = 0;
        } else {
            i = strlen (tmp2);
            /* increase the level in the dir tree */
            df->nr_files = 1;
            /* save the name into first level */
            if ((*(df->name) = malloc (12 * sizeof (char))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }
            strcpy (*(df->name), "Wastebasket");

            if (!strncmp (tmp2, buf, i) && buf[i]) {
                free (tmp2);
                i++;
            } else {
                free (tmp2);
                free (buf);
                free (tmp);
                return (df);
            }
        }
    } else {
        i = 0;
    }


    for (j = 0; i <= buflen; i++, j++) {
        tmp[j] = buf[i];
        /* if directory separator or end of string */
        if (tmp[j] == DIRC || tmp[j] == '\0') {
            if (buf[i] == '\0' && j == 0) {
                return (df);
            }
            if (df->nr_files == 0) {	/* for the very first time save the '/' sign */
                j++;
            }
            tmp[j] = '\0';

            /* if we need more space for the directories */
            if (df->nr_files >= maxd) {
                maxd += 10;
                dftmp = df->name;
                if ((df->name = malloc (sizeof (char *) * maxd)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
                }
                for (k = 0; k < maxd - 10; k++) {
                    *(df->name + k) = *(dftmp + k);
                }
                free (dftmp);
            }
            /* save the current directory */
            if ((*(df->name + df->nr_files) =
                        malloc ((strlen (tmp) + 1) * sizeof (char))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }
            strcpy (*(df->name + df->nr_files), tmp);
            df->nr_files++;
            j = -1;
        }
    }
    free (buf);
    free (tmp);
    return (df);
}

/* This function creates a wastebasket directory if needed, then returns a
   string made up of the last filename in the string pointed to by 'file' and
   the wastebasket path.  It returns NULL if there is a memory allocation
   error or the pointer to 'path' otherwise.  The original function did no
   error checking on wastebasket path creation.  The modified version does --
   it also returns NULL on an error in the chdir function or mkdir
   function. */
char *
WpeGetWastefile (char *file)
{
    static char *wastebasket = NULL;
    int i, lw;
    char *tmp;			/* storage for wastebasket path */
    char *tmp2;

    if (!wastebasket) {	/* setup and test access to wastebasket directory */
        if ((tmp2 = getenv ("HOME")) == NULL) {
            return NULL;
        }

        lw = strlen (tmp2) + 1 + strlen (WASTEBASKET) + 1;
        /*    HOME          /     WASTEBASKET       \0    */
        if ((tmp = (char *) malloc (lw)) == NULL) {
            return NULL;
        }

        sprintf (tmp, "%s/%s", tmp2, WASTEBASKET);

        /* if wastebasket dir does not exist, create it with error checking */
        if (access (tmp, F_OK)) {
            if (mkdir (tmp, 0700)) {
                free (tmp);
                return NULL;
            }
        }
        /* check for wastebasket's permissions */
        if (access (tmp, R_OK | W_OK | X_OK)) {
            free (tmp);
            return NULL;
        }
        wastebasket = tmp;
    }

    /* verify that wastebasket directory still exists before proceeding */
    if (access (wastebasket, F_OK | R_OK | W_OK | X_OK)) {
        /* try to recreate it */
        if (mkdir (wastebasket, 0700)) {
            return NULL;
        }
    }

    /* return wastebasket directory path if no filename in 'file' */
    if (file[0] == '\0') {
        if ((tmp2 = malloc (strlen (wastebasket) + 1)) == NULL) {
            return NULL;
        }
        strcpy (tmp2, wastebasket);
        return (tmp2);
    }

    /* else get filename from end of 'file' string,
       append to wastebasket sting */
    for (i = strlen (file) - 1; i >= 0 && file[i] != DIRC; i--)
        ;
    if ((tmp2 =
                malloc (strlen (wastebasket) + strlen (file + 1 + i) + 2)) == NULL) {
        return NULL;
    }

    sprintf (tmp2, "%s/%s", wastebasket, file + 1 + i);
    return (tmp2);
}

struct dirfile *
WpeGraphicalFileList (struct dirfile *df, int sw, we_control_t * control)
{
    struct dirfile *edf;
    char **name, **ename, *stmp, str[256];
    int ntmp, n, i, *num;

    /* allocate the same structure as the argument */
    if ((edf = malloc (sizeof (struct dirfile))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    edf->nr_files = df->nr_files;
    edf->name = NULL;

    /* OSF and AIX fix, for malloc(0) they return NULL */
    if (df->nr_files) {
        if ((edf->name = malloc (df->nr_files * sizeof (char *))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
        }

        if ((num = malloc (df->nr_files * sizeof (int))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
        }

        for (i = 0; i < df->nr_files; i++) {
            e_file_info (*(df->name + i), str, num + i, sw);
            if ((*(edf->name + i) = malloc (strlen (str) + 1)) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }
            strcpy (*(edf->name + i), str);
        }

        /* sort by time or size mode */
        if (sw & 3) {
            if ((ename = malloc (df->nr_files * sizeof (char *))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }

            if ((name = malloc (df->nr_files * sizeof (char *))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }

            for (i = 0; i < df->nr_files; i++) {
                for (ntmp = num[i], n = i; n > 0 && ntmp > num[n - 1]; n--) {
                    *(ename + n) = *(ename + n - 1);
                    *(name + n) = *(name + n - 1);
                    num[n] = num[n - 1];
                }
                *(ename + n) = *(edf->name + i);
                *(name + n) = *(df->name + i);
                num[n] = ntmp;
            }
            free (edf->name);
            free (df->name);
            edf->name = ename;
            df->name = name;
        }

        free (num);

        /* reverse order */
        if (sw & 4) {
            for (i = 0; i < (df->nr_files) / 2; i++) {
                stmp = edf->name[i];
                edf->name[i] = edf->name[edf->nr_files - i - 1];
                edf->name[edf->nr_files - i - 1] = stmp;
                stmp = df->name[i];
                df->name[i] = df->name[df->nr_files - i - 1];
                df->name[df->nr_files - i - 1] = stmp;
            }
        }
    }

    return (edf);
}

struct dirfile *
WpeGraphicalDirTree (struct dirfile *cd, struct dirfile *dd, we_control_t * control)
{
    extern char *ctree[5];
    struct dirfile *edf;
    char str[256];
    int i = 0, j;

    if ((edf = malloc (sizeof (struct dirfile))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }

    /* for the OSF and AIX this should never be zero, we are always somewhere */
    if (cd->nr_files + dd->nr_files > 0) {
        if ((edf->name =
                    malloc ((cd->nr_files + dd->nr_files) * sizeof (char *))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
        }

        for (i = 0; i < cd->nr_files; i++) {
            if (!i) {
                if (*cd->name[0] == DIRC && *(cd->name[0] + 1) == '\0') {
                    strcpy (str, "Root");
                } else {
                    strcpy (str, *(cd->name + i));
                }
            } else {
                for (str[0] = '\0', j = 0; j < i - 1; j++) {
                    strcat (str, "  ");
                }
                if (i == cd->nr_files - 1 && dd->nr_files < 1) {
                    strcat (str, ctree[1]);
                } else if (i == cd->nr_files - 1) {
                    strcat (str, ctree[2]);
                } else {
                    strcat (str, ctree[0]);
                }
                strcat (str, *(cd->name + i));
            }
            if ((*(edf->name + i) =
                        malloc ((strlen (str) + 1) * sizeof (char))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }
            strcpy (*(edf->name + i), str);
        }

        // defined earlier str[256];
        for (; i < cd->nr_files + dd->nr_files; i++) {
            for (str[0] = '\0', j = 0; j < cd->nr_files - 2; j++) {
                strcat (str, "  ");
            }
            strcat (str, " ");
            if (i == cd->nr_files + dd->nr_files - 1) {
                strcat (str, ctree[4]);
            } else {
                strcat (str, ctree[3]);
            }
            {
                int tttt = i - cd->nr_files;
                strcat (str, *(dd->name + tttt));
            }
            if ((*(edf->name + i) =
                        malloc ((strlen (str) + 1) * sizeof (char))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
            }
            strcpy (*(edf->name + i), str);
        }
    }

    edf->nr_files = i;
    return (edf);
}

int
WpeDelWastebasket (we_window_t * window)
{
    char *tmp;
    int ret, mode = window->edit_control->flopt;

    WpeMouseChangeShape (WpeWorkingShape);
    window->edit_control->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                                  FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
    if ((tmp = WpeGetWastefile (""))) {
        ret = WpeRemoveDir (tmp, "*", window, 0);
        free (tmp);

        /* Unfortunately there is this racing condition, so
           this is necessary to get back the deleted wastebasket. */
        if ((tmp = WpeGetWastefile (""))) {
            free (tmp);
            ret = 0;
        } else {
            e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
            ret = 1;
        }
    } else {
        /* Error failed to find wastebasket */
        e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
        ret = 1;
    }
    window->edit_control->flopt = mode;
    WpeMouseRestoreShape ();
    return (ret);
}

int
WpeShowWastebasket (we_window_t * window)
{
    return (WpeCallFileManager (6, window));
}

int
WpeQuitWastebasket (we_window_t * window)
{
    char *tmp;
    int ret = 0, mode = window->edit_control->flopt;

    if (mode & FM_PROMPT_DELETE)
        window->edit_control->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                                      FM_REMOVE_PROMPT | FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;
    else if (mode & FM_DELETE_AT_EXIT)
        window->edit_control->flopt = FM_SHOW_HIDDEN_FILES | FM_SHOW_HIDDEN_DIRS |
                                      FM_MOVE_OVERWRITE | FM_REKURSIVE_ACTIONS;

    if ((mode & FM_PROMPT_DELETE) || (mode & FM_DELETE_AT_EXIT)) {
        WpeMouseChangeShape (WpeWorkingShape);
        if ((tmp = WpeGetWastefile (""))) {
            ret = WpeRemoveDir (tmp, "*", window, 0);
            free (tmp);
        } else {
            /* Error failed to find wastebasket */
            e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
            ret = 0;
        }
        WpeMouseRestoreShape ();
    }

    window->edit_control->flopt = mode;
    return (ret);
}


int
WpeRemoveDir (char *dirct, char *file, we_window_t * window, int rec)
{
    we_view_t *view = NULL;
    char *tmp;
    int i, ret, svmode = window->edit_control->flopt;
    struct dirfile *dd;

    if (rec > MAXREC) {
        return (0);
    }

    /* only copy it to the wastebasket */
    if (window->edit_control->flopt & FM_REMOVE_INTO_WB) {
        if ((tmp = WpeGetWastefile (dirct))) {
            i = strlen (tmp);
            /* we should not use the fact, that the wasbasket is always something
               meaningful !!! */
            if (strncmp (tmp, dirct, i)) {
                ret = WpeRenameCopyDir (dirct, file, tmp, window, 0, 0);
                free (tmp);
                return (ret);
            }
            free (tmp);
        } else {
            e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
            return 1;
        }
    }

    if ((tmp = malloc (strlen (dirct) + strlen (file) + 2)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }

    /* search for files in the directory */
    sprintf (tmp, "%s%c%s", dirct, DIRC, file);
    dd = e_find_files (tmp, window->edit_control->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0);

    /* it is called for the first time and the user should be asked about
       the deletion */
    if (!rec && (window->edit_control->flopt & FM_REMOVE_PROMPT) && dd->nr_files > 0) {
        if ((ret = WpeDirDelOptions (window)) < 0) {
            freedf (dd);
            free (tmp);
            return (ret == WPE_ESC ? 1 : 0);
        }
        if (ret) {
            window->edit_control->flopt |= FM_REMOVE_PROMPT;
        } else {
            window->edit_control->flopt &= ~FM_REMOVE_PROMPT;
        }
        rec = -1;
    }
    free (tmp);

    /* cleans up the files in the directory */
    for (i = 0; i < dd->nr_files; i++) {
        if ((tmp = malloc (strlen (dirct) + strlen (dd->name[i]) + 15)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        sprintf (tmp, "Remove File:\n%s%c%s", dirct, DIRC, dd->name[i]);
        if (window->edit_control->flopt & FM_REMOVE_PROMPT) {
            ret = e_message (1, tmp, window);
        } else {
            ret = 'Y';
        }
        if (ret == WPE_ESC) {
            freedf (dd);
            free (tmp);
            window->edit_control->flopt = svmode;
            return (1);
        } else if (ret == 'Y') {
            /* this should definitely fit in */
            sprintf (tmp, "%s%c%s", dirct, DIRC, dd->name[i]);

            /* put message out */
            if (e_mess_win ("Remove", tmp, &view, window)) {
                free (tmp);
                break;
            }

            if (view) {
                e_close_view (view, 1);
                view = NULL;
            }

            /* try to remove it */
            if (remove (tmp)) {
                e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
                freedf (dd);
                free (tmp);
                window->edit_control->flopt = svmode;
                return (1);
            }
        }
        free (tmp);
    }

    if (view) {
        e_close_view (view, 1);
    }
    freedf (dd);

    /* if recursive action is specified clean up the
       subdirectories as well */
    if (window->edit_control->flopt & FM_REKURSIVE_ACTIONS) {
        if ((tmp = malloc (strlen (dirct) + strlen (SUDIR) + 2)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        /* search for subdirectories */
        sprintf (tmp, "%s%c%s", dirct, DIRC, SUDIR);
        dd = e_find_dir (tmp, window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);

        /* should the user be asked about deletion ? */
        if (!rec && (window->edit_control->flopt & FM_REMOVE_PROMPT) && dd->nr_files > 0) {
            if ((ret = WpeDirDelOptions (window)) < 0) {
                freedf (dd);
                free (tmp);
                return (ret == WPE_ESC ? 1 : 0);
            }
            if (ret) {
                window->edit_control->flopt |= FM_REMOVE_PROMPT;
            } else {
                window->edit_control->flopt &= ~FM_REMOVE_PROMPT;
            }
        } else if (rec < 0) {
            rec = 0;
        }

        free (tmp);

        /* call recursively itself to delete the subdirectories */
        for (rec++, i = 0; i < dd->nr_files; i++) {
            if ((tmp =
                        malloc (strlen (dirct) + strlen (dd->name[i]) + 2)) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
            }

            sprintf (tmp, "%s%c%s", dirct, DIRC, dd->name[i]);
            if (WpeRemoveDir (tmp, file, window, rec)) {
                freedf (dd);
                free (tmp);
                window->edit_control->flopt = svmode;
                return (1);
            }
            free (tmp);
        }
        freedf (dd);
    }

    window->edit_control->flopt = svmode;

    /* remove finally the directory itself */
    if (rmdir (dirct)) {
        e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
        return (1);
    } else {
        return (0);
    }
}

int
WpeRemove (char *file, we_window_t * window)
{
    struct stat buf;
    struct stat lbuf;
    char *tmp2;
    int ret;

    if (lstat (file, &lbuf)) {
        e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->edit_control->colorset);
        return 1;
    }

    WpeMouseChangeShape (WpeWorkingShape);

    /* this is important, first we check whether it is a file,
       this check works even when it is a pointer, if it is not
       a file, then it can be an "invalid" symbolic link, check for that */
    if (((stat (file, &buf) == 0) && S_ISREG (buf.st_mode))
            || S_ISLNK (lbuf.st_mode)) {
        if ((window->edit_control->flopt & FM_REMOVE_INTO_WB)) {
            if ((tmp2 = WpeGetWastefile (file))) {
                ret = strlen (tmp2);
                if (strncmp (tmp2, file, ret)) {
                    e_rename (file, tmp2, window);
                }
                free (tmp2);
                ret = 0;
            } else {
                e_error (e_msg[ERR_NOWASTE], ERROR_MSG, window->edit_control->colorset);
                ret = 1;
            }
        } else {
            if (window->edit_control->flopt & FM_REMOVE_PROMPT) {
                if ((tmp2 = malloc (strlen (file) + 14)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                }

                sprintf (tmp2, "Remove File:\n%s", file);
                ret = e_message (1, tmp2, window);
                free (tmp2);
            } else {
                ret = 'Y';
            }

            if (ret == 'Y') {
                if (remove (file)) {
                    e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
                    ret = 1;
                } else {
                    ret = 0;
                }
            } else {
                ret = 0;
            }
        }
    } else {
        ret = WpeRemoveDir (file, window->find.file, window, 0);
    }

    WpeMouseRestoreShape ();
    return (ret);
}

/* sw = 0  -> renames the directory
   sw = 1  -> copies the directory
   sw = 2  -> links directory */
int
WpeRenameCopyDir (char *dirct, char *file, char *newname, we_window_t * window,
                  int rec, int sw)
{
    char *tmp, *ntmp, *mtmp;
    int i, ret, mode;
    struct dirfile *dd;
    struct stat buf;
    we_view_t *view = NULL;

    if (rec > MAXREC) {
        return (0);
    }

    /* check whether the dir already exist */
    ret = access (newname, F_OK);
    /* rename mode and dir does not exist */
    if (sw == 0 && ret && file[0] == '*' && file[1] == '\0') {
        if ((ret = rename (dirct, newname))) {
            e_error (e_msg[ERR_RENFILE], ERROR_MSG, window->edit_control->colorset);
        }
        return (ret);
    }

    /* directory does not exist */
    if (ret != 0) {
        /* get the permissions */
        if (stat (dirct, &buf)) {
            e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->edit_control->colorset);
            return (1);
        }
        /* with that permission create the new dir */
        if (mkdir (newname, buf.st_mode)) {
            e_error (e_msg[ERR_NONEWDIR], ERROR_MSG, window->edit_control->colorset);
            return (1);
        }
    }

    if (window->edit_control->flopt & FM_REKURSIVE_ACTIONS) {
        if ((tmp = malloc (strlen (dirct) + 2 + strlen (SUDIR))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        sprintf (tmp, "%s%c%s", dirct, DIRC, SUDIR);
        dd = e_find_dir (tmp, window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0);
        free (tmp);

        for (rec++, i = 0; i < dd->nr_files; i++) {

            if ((tmp =
                        malloc (strlen (dirct) + 2 + strlen (dd->name[i]))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
            }

            if ((ntmp =
                        malloc (strlen (newname) + 2 + strlen (dd->name[i]))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
            }

            sprintf (tmp, "%s%c%s", dirct, DIRC, dd->name[i]);
            sprintf (ntmp, "%s%c%s", newname, DIRC, dd->name[i]);

            if (WpeRenameCopyDir (tmp, file, ntmp, window, rec, sw)) {
                free (tmp);
                free (ntmp);
                freedf (dd);
                return (1);
            }
            free (tmp);
            free (ntmp);
        }
        freedf (dd);
    }

    if ((tmp = malloc (strlen (dirct) + 2 + strlen (file))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }
    sprintf (tmp, "%s%c%s", dirct, DIRC, file);
    dd = e_find_files (tmp, window->edit_control->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0);
    free (tmp);

    mode = window->edit_control->flopt;
    window->edit_control->flopt &= ~FM_REMOVE_PROMPT;

    for (i = 0; i < dd->nr_files; i++) {
        if ((ntmp =
                    malloc (strlen (newname) + 2 + strlen (dd->name[i]))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        sprintf (ntmp, "%s%c%s", newname, DIRC, dd->name[i]);
        ret = 'Y';

        if (access (ntmp, F_OK) == 0) {
            if (window->edit_control->flopt & FM_MOVE_PROMPT) {
                if ((tmp = malloc (strlen (ntmp) + 31)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                }

                sprintf (tmp, "File %s exist !\nOverwrite File ?", ntmp);
                if (view) {
                    e_close_view (view, 1);
                    view = NULL;
                }
                ret = e_message (1, tmp, window);
                free (tmp);

                if (ret == 'Y') {
                    if (WpeRemove (ntmp, window)) {
                        free (ntmp);
                        freedf (dd);
                        return (1);
                    }
                } else if (ret == WPE_ESC) {
                    free (ntmp);
                    freedf (dd);
                    return (1);
                }
            } else if (window->edit_control->flopt & FM_MOVE_OVERWRITE) {
                if (WpeRemove (ntmp, window)) {
                    free (ntmp);
                    freedf (dd);
                    return (1);
                }
            }
        }

        if ((tmp = malloc (strlen (dirct) + 2 + strlen (dd->name[i]))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        sprintf (tmp, "%s%c%s", dirct, DIRC, dd->name[i]);

        if (ret == 'Y') {
            if ((mtmp = malloc (strlen (tmp) + 2 + strlen (ntmp))) == NULL) {
                e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
            }

            sprintf (mtmp, "%s %s", tmp, ntmp);
            if (e_mess_win (!sw ? "Rename" : "Copy", mtmp, &view, window)) {
                free (tmp);
                free (ntmp);
                free (mtmp);
                break;
            }
            free (mtmp);

            if (sw == 0) {
                if ((ret = rename (tmp, ntmp))) {
                    e_error (e_msg[ERR_RENFILE], ERROR_MSG, window->edit_control->colorset);
                    free (tmp);
                    free (ntmp);
                    freedf (dd);
                    return (1);
                }
            } else if (sw == 1) {
                if (WpeCopyFileCont (tmp, ntmp, window)) {
                    free (tmp);
                    free (ntmp);
                    freedf (dd);
                    return (1);
                }
            } else if (sw == 2) {
                if (WpeLinkFile (tmp, ntmp, window->edit_control->flopt & FM_TRY_HARDLINK, window)) {
                    free (tmp);
                    free (ntmp);
                    freedf (dd);
                    return (1);
                }
            }
        }

        free (tmp);
        free (ntmp);
    }

    if (view) {
        e_close_view (view, 1);
    }

    window->edit_control->flopt = mode;

    if (sw == 0) {
        if (rmdir (dirct)) {
            e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
        }
    }

    freedf (dd);
    return (0);
}


int
WpeRenameCopy (char *file, char *newname, we_window_t * window, int sw)
{
    struct stat buf;
    struct stat lbuf;
    char *tmp, *tmpl;
    int ln = -1, ret = 'Y';
    int allocate_size, retl = 0;

    WpeMouseChangeShape (WpeWorkingShape);

    /* in copy mode check whether it is a link */
    if (sw == 0) {
        if (lstat (file, &lbuf)) {
            e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->edit_control->colorset);
            return 1;
        }

        if (S_ISLNK (lbuf.st_mode)) {
            ln = 1;
        }
    }

    if ((stat (file, &buf) == 0) && S_ISDIR (buf.st_mode) && ln < 0) {
        retl = WpeRenameCopyDir (file, window->find.file, newname, window, 0, sw);
    } else {
        /* check whether file exist */
        if (access (newname, F_OK) == 0) {
            if (window->edit_control->flopt & FM_MOVE_OVERWRITE) {
                if (WpeRemove (newname, window)) {
                    WpeMouseRestoreShape ();
                    return (1);
                }
            } else if (window->edit_control->flopt & FM_MOVE_PROMPT) {
                if ((tmp = malloc (strlen (newname) + 26)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                }
                sprintf (tmp, "File %s exist\nRemove File ?", newname);
                ret = e_message (1, tmp, window);
                free (tmp);
                if (ret == 'Y') {
                    if (WpeRemove (newname, window)) {
                        WpeMouseRestoreShape ();
                        return (1);
                    }
                }
            }
        }

        if (ret == 'Y') {
            if (sw == 1) {
                retl = WpeCopyFileCont (file, newname, window);
            } else if (sw == 2) {
                retl =
                    WpeLinkFile (file, newname, window->edit_control->flopt & FM_TRY_HARDLINK,
                                 window);
            } else if (sw == 0 && ln < 0) {	/* rename mode, no softlink */
                if ((retl = rename (file, newname)) == -1) {
                    if (errno == EXDEV) {
                        if ((retl = WpeCopyFileCont (file, newname, window)) == 0) {
                            if ((retl = remove (file))) {
                                e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
                            }
                        }
                    } else {
                        e_error (e_msg[ERR_RENFILE], ERROR_MSG, window->edit_control->colorset);
                        retl = 1;
                    }
                }
            } else if (sw == 0) {
                allocate_size = 2;
                tmp = NULL;
                do {
                    if (tmp) {
                        free (tmp);
                    }
                    allocate_size += 4;
                    if ((tmp = malloc (allocate_size)) == NULL) {
                        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                    }

                    ln = readlink (file, tmp, allocate_size - 1);
                } while (!(ln < allocate_size - 1));
                tmp[ln] = '\0';

                for (; ln >= 0 && tmp[ln] != DIRC; ln--)
                    ;
                if (ln < 0) {
                    if ((tmpl =
                                malloc (strlen (window->dirct) + 2 + strlen (tmp))) == NULL) {
                        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                    }

                    sprintf (tmpl, "%s%c%s", window->dirct, DIRC, tmp);
                    retl = WpeRenameLink (file, newname, tmpl, window);
                    free (tmpl);
                } else {
                    retl = WpeRenameLink (file, newname, tmp, window);
                }
            }
        }
    }
    WpeMouseRestoreShape ();
    return (retl);
}

int
WpeCopyFileCont (char *oldfile, char *newfile, we_window_t * window)
{
    struct stat buf;
    size_t ret;
    char *buffer;
    FILE *fpo, *fpn;

    /* get the status of the file */
    if (stat (oldfile, &buf)) {
        e_error (e_msg[ERR_ACCFILE], ERROR_MSG, window->edit_control->colorset);
        return (1);
    }

    /* open files for copying */
    if ((fpo = fopen (oldfile, "rb")) == NULL) {
        e_error (e_msg[ERR_OREADFILE], ERROR_MSG, window->edit_control->colorset);
        return (1);
    }
    if ((fpn = fopen (newfile, "wb")) == NULL) {
        e_error (e_msg[ERR_OWRITEFILE], ERROR_MSG, window->edit_control->colorset);
        return (1);
    }

    /* allocate buffer for copying */
    if ((buffer = malloc (E_C_BUFFERSIZE)) == NULL) {
        e_error (e_msg[ERR_ALLOC_CBUF], ERROR_MSG, window->edit_control->colorset);
        return (1);
    }

    /* copy until end of file */
    do {
        ret = fread (buffer, 1, E_C_BUFFERSIZE, fpo);
        if (ret == 0 && ferror(fpo)) {
            char *msg = "Error during read of stream.";
            e_error(msg, SERIOUS_ERROR_MSG, window->colorset); // does not return
        }
        /* we should be able to write the same amount of info */
        if (fwrite (buffer, 1, ret, fpn) != ret) {
            fclose (fpo);
            fclose (fpn);
            free (buffer);
            e_error (e_msg[ERR_INCONSCOPY], ERROR_MSG, window->edit_control->colorset);
            return (1);
        }
    } while (!feof (fpo));

    fclose (fpo);
    fclose (fpn);
    free (buffer);

    /* Well, we just created the file, so theoretically this
       should succeed. Of course this is a racing condition !!! */
    if (chmod (newfile, buf.st_mode)) {
        e_error (e_msg[ERR_CHGPERM], ERROR_MSG, window->edit_control->colorset);
    }
    return (0);
}

#ifdef HAVE_SYMLINK
/* Link a file according to the required mode
   sw != 0 -> symbolic link
   sw == 0 -> hard link
   When hard linking does not work, it tries to do
   symbolic linking
*/
int
WpeLinkFile (char *fl, char *ln, int sw, we_window_t * window)
{
    int ret;

    if (sw || link (fl, ln)) {
        ret = symlink (fl, ln);
        if (ret) {
            e_error (e_msg[ERR_LINKFILE], ERROR_MSG, window->edit_control->colorset);
        }
        return (ret);
    }
    return (0);
}

int
WpeRenameLink (char *old, char *ln, char *fl, we_window_t * window)
{
    int ret;

    ret = symlink (fl, ln);
    if (ret) {
        e_error (e_msg[ERR_LINKFILE], ERROR_MSG, window->edit_control->colorset);
        return (1);
    } else {
        if (remove (old)) {
            e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
            return (1);
        } else {
            return (0);
        }
    }
}
#endif // #ifdef HAVE_SYMLINK

int
e_rename (char *file, char *newname, we_window_t * window)
{
    return (WpeRenameCopy (file, newname, window, 0));
}

int
e_rename_dir (char *dirct, char *file, char *newname, we_window_t * window, int rec)
{
    return (WpeRenameCopyDir (dirct, file, newname, window, rec, 0));
}


int
e_link (char *file, char *newname, we_window_t * window)
{
    return (WpeRenameCopy (file, newname, window, 2));
}


int
e_copy (char *file, char *newname, we_window_t * window)
{
    return (WpeRenameCopy (file, newname, window, 1));
}


int
WpeFileManagerOptions (we_window_t * window)
{
    int ret;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (o == NULL) {
        return (1);
    }

    o->xa = 8;
    o->ya = 1;
    o->xe = 69;
    o->ye = 22;
    o->bgsw = AltO;
    o->name = "File-Manager-Options";
    o->crsw = AltO;
    e_add_txtstr (4, 2, "Directories:", o);
    e_add_txtstr (35, 13, "Wastebasket:", o);
    e_add_txtstr (4, 7, "Move/Copy:", o);
    e_add_txtstr (35, 8, "Remove:", o);
    e_add_txtstr (35, 2, "Sort Files by:", o);
    e_add_txtstr (4, 12, "Links on Files:", o);
    e_add_txtstr (4, 16, "On Open:", o);

    e_add_sswstr (5, 3, 12, AltF,
                  window->edit_control->flopt & FM_SHOW_HIDDEN_FILES ? 1 : 0,
                  "Show Hidden Files      ", o);
    e_add_sswstr (5, 4, 12, AltD,
                  window->edit_control->flopt & FM_SHOW_HIDDEN_DIRS ? 1 : 0,
                  "Show Hidden Directories", o);
    e_add_sswstr (5, 5, 2, AltK,
                  window->edit_control->flopt & FM_REKURSIVE_ACTIONS ? 1 : 0,
                  "ReKursive Actions      ", o);
    e_add_sswstr (5, 17, 0, AltC,
                  window->edit_control->flopt & FM_CLOSE_WINDOW ? 1 : 0,
                  "Close File Manager     ", o);

    e_add_sswstr (36, 6, 0, AltR,
                  window->edit_control->flopt & FM_REVERSE_ORDER ? 1 : 0,
                  "Reverse Order    ", o);

    e_add_pswstr (0, 36, 14, 0, AltP, 0, "Prompt for Delete", o);
    e_add_pswstr (0, 36, 15, 10, AltE, 0, "Delete at Exit   ", o);
    e_add_pswstr (0, 36, 16, 8, AltL, window->edit_control->flopt & FM_PROMPT_DELETE ? 0 :
                  (window->edit_control->flopt & FM_DELETE_AT_EXIT ? 1 : 2),
                  "Don't DeLete     ", o);
    e_add_pswstr (1, 36, 3, 0, AltN, 0, "Name             ", o);
    e_add_pswstr (1, 36, 4, 1, AltI, 0, "TIme             ", o);
    e_add_pswstr (1, 36, 5, 0, AltB, window->edit_control->flopt & FM_SORT_NAME ? 1 :
                  (window->edit_control->flopt & FM_SORT_TIME ? 2 : 0), "Bytes            ",
                  o);
    e_add_pswstr (2, 5, 8, 12, AltQ, 0, "Prompt for eQual Files ", o);
    e_add_pswstr (2, 5, 9, 1, AltV, 0, "OVerwrite equal Files  ", o);
    e_add_pswstr (2, 5, 10, 4, AltT, window->edit_control->flopt & FM_MOVE_PROMPT ? 0 :
                  (window->edit_control->flopt & FM_MOVE_OVERWRITE ? 1 : 2),
                  "Don'T overwrite        ", o);
    e_add_pswstr (3, 5, 13, 4, AltH, 0, "Try Hardlink           ", o);
    e_add_pswstr (3, 5, 14, 7, AltS,
                  window->edit_control->flopt & FM_TRY_HARDLINK ? 1 : 0,
                  "Always Symbolic Link   ", o);
    e_add_pswstr (4, 36, 9, 5, AltW, 0, "into Wastebasket ", o);
    e_add_pswstr (4, 36, 10, 0, AltA, 0, "Absolute (Prompt)", o);
    e_add_pswstr (4, 36, 11, 6, AltM, window->edit_control->flopt & FM_REMOVE_INTO_WB ? 0 :
                  (window->edit_control->flopt & FM_REMOVE_PROMPT ? 1 : 2),
                  "No ProMpt        ", o);
    e_add_bttstr (16, 19, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (38, 19, -1, WPE_ESC, "Cancel", NULL, o);

    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        window->edit_control->flopt = o->sstr[0]->num +
                                      (o->sstr[1]->num << 1) +
                                      (o->sstr[2]->num ? FM_REKURSIVE_ACTIONS : 0) +
                                      (o->sstr[3]->num ? FM_CLOSE_WINDOW : 0) +
                                      (o->sstr[4]->num ? FM_REVERSE_ORDER : 0) +
                                      (o->pstr[0]->num ? (o->pstr[0]->num == 1 ? 4 : 0) : 8) +
                                      (o->pstr[2]->num ? (o->pstr[2]->num == 1 ? 16 : 0) : 32) +
                                      (o->pstr[4]->num ? (o->pstr[4]->num == 1 ? 128 : 0) : 64) +
                                      (o->pstr[1]->num ? (o->pstr[1]->num == 1 ? 01000 : 02000) : 0) +
                                      (o->pstr[3]->num ? FM_TRY_HARDLINK : 0);
    }

    freeostr (o);
    return (0);
}

int
WpeDirDelOptions (we_window_t * window)
{
    int ret;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (o == NULL) {
        return (1);
    }

    o->xa = 19;
    o->ya = 11;
    o->xe = 53;
    o->ye = 19;
    o->bgsw = AltO;
    o->name = "Message";
    o->crsw = AltO;
    e_add_txtstr (4, 2, "Delete Directory:", o);
    e_add_pswstr (0, 5, 3, 0, AltD, 0, "Delete without Prompt", o);
    e_add_pswstr (0, 5, 4, 0, AltP, 1, "Prompt for Files     ", o);
    e_add_bttstr (7, 6, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (22, 6, -1, WPE_ESC, "Cancel", NULL, o);

    ret = e_opt_kst (o);

    ret = (ret == WPE_ESC) ? -1 : o->pstr[0]->num;

    freeostr (o);
    return (ret);
}

int
WpeShell (we_window_t * window)
{
    we_view_t *outp = NULL;
    int g[4];

    if (!WpeIsXwin ()) {
        outp = e_open_view (0, 0, max_screen_cols() - 1, max_screen_lines() - 1, window->colorset->ws, 1);
        fk_u_locate (0, 0);
        fk_u_cursor (1);
#if  MOUSE
        g[0] = 2;
        fk_mouse (g);
#endif
        e_u_s_sys_ini ();
    }
    e_u_system (user_shell);
    if (!WpeIsXwin ()) {
        e_u_s_sys_end ();
        e_close_view (outp, 1);
        fk_u_cursor (0);
#if  MOUSE
        g[0] = 1;
        fk_mouse (g);
#endif
    }
    return (0);
}

/*   print file */
#ifndef NOPRINTER
int
WpePrintFile (we_window_t * window)
{
    char *str, *dp;
    int c, sins = window->ins;

    for (c = window->edit_control->mxedt; c > 0 && !DTMD_ISTEXT (window->edit_control->window[c]->dtmd); c--)
        ;
    if (c <= 0) {
        return (0);
    }
    window = window->edit_control->window[c];

    if (strcmp (window->edit_control->print_cmd, "") == 0) {
        return (e_error (e_msg[ERR_NOPRINT], ERROR_MSG, window->colorset));
    }
    if ((str = malloc (strlen (window->datnam) + 32)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }

    sprintf (str, "File: %s\nDo you want to print it?", window->datnam);
    c = e_message (1, str, window);
    free (str);
    if (c != 'Y') {
        return (0);
    }

    dp = window->dirct;
    window->dirct = e_tmp_dir;
    window->ins = 0;

    e_save (window);

    window->dirct = dp;
    window->ins = sins;

    if ((str = malloc (strlen (e_tmp_dir) + 7 +
                       strlen (window->edit_control->print_cmd) + strlen (window->datnam))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }

    sprintf (str, "cd %s; %s %s", e_tmp_dir, window->edit_control->print_cmd, window->datnam);
    if (system (str)) {
        e_error (e_msg[ERR_NOTINSTALL], ERROR_MSG, window->colorset);
    }

    sprintf (str, "%s/%s", e_tmp_dir, window->datnam);
    if (remove (str)) {
        e_error (e_msg[ERR_DELFILE], ERROR_MSG, window->edit_control->colorset);
    }

    free (str);
    return (0);
}
#else
int
WpePrintFile (we_window_t * window)
{
    return (e_error (e_msg[ERR_NOPRINT], ERROR_MSG, window->colorset));
}
#endif

struct dirfile *
WpeSearchFiles (we_window_t * window, char *dirct, char *file, char *string,
                struct dirfile *df, int sw)
{
    struct dirfile *dd;
    char **tname, *tp, *tmp, *tmp2;
    int i;
    static int rec = 0;

    if (rec > MAXREC) {
        return (df);
    }

    /* if not absolute path is given */
    if (*dirct != DIRC) {
        tmp2 = WpeGetCurrentDir (window->edit_control);

        tmp = realloc (tmp2, strlen (tmp2) + strlen (dirct) + 4);
        if (tmp == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        tmp2 = tmp;
        if (tmp2[strlen (tmp2) - 1] != DIRC) {
            sprintf (tmp2 + strlen (tmp2), "%c%s%c", DIRC, dirct, DIRC);
        } else {
            sprintf (tmp2 + strlen (tmp2), "%s%c", dirct, DIRC);
        }
    } else {
        if ((tmp2 = malloc (strlen (dirct) + 2)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        if (dirct[strlen (dirct) - 1] != DIRC) {
            sprintf (tmp2, "%s%c", dirct, DIRC);
        } else {
            sprintf (tmp2, "%s", dirct);
        }
    }

    /* assemble total path, dir + filename */
    if ((tmp = malloc (strlen (tmp2) + strlen (file) + 2)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }
    sprintf (tmp, "%s%s", tmp2, file);

    /* initialise structure, only once */
    if (df == NULL) {
        if ((df = malloc (sizeof (struct dirfile))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }

        df->nr_files = 0;
        if ((df->name = malloc (sizeof (char *))) == NULL) {
            e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
        }
    }

    /* search all matching file */
    dd = e_find_files (tmp, 0);

    /* if we found something */
    if (dd && dd->nr_files > 0) {

        /* file find */
        if (!(sw & 1024)) {
            for (i = 0; i < dd->nr_files; i++) {
                if ((tp =
                            malloc (strlen (tmp2) + strlen (dd->name[i]) + 2)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                }

                sprintf (tp, "%s%s", tmp2, dd->name[i]);

                df->nr_files++;

                if ((tname =
                            realloc (df->name, df->nr_files * sizeof (char *))) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                }

                df->name = tname;
                df->name[df->nr_files - 1] = tp;
            }
        }
        /* file grep */
        else {
            for (i = 0; i < dd->nr_files; i++) {
                if ((tp =
                            malloc (strlen (tmp2) + strlen (dd->name[i]) + 2)) == NULL) {
                    e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                }

                sprintf (tp, "%s%s", tmp2, dd->name[i]);

                if (WpeGrepFile (tp, string, sw)) {
                    df->nr_files++;
                    if ((tname =
                                realloc (df->name, df->nr_files * sizeof (char *))) == NULL) {
                        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
                    }

                    df->name = tname;
                    df->name[df->nr_files - 1] = tp;
                } else {
                    free (tp);
                }
            }
        }
    }

    freedf (dd);
    free (tmp2);
    free (tmp);

    /* whether recursive action */
    if (!(sw & 512)) {
        return (df);
    }

    if ((tmp = malloc (strlen (dirct) + strlen (SUDIR) + 2)) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, window->edit_control->colorset);
    }

    if (dirct[strlen (dirct) - 1] != DIRC) {
        sprintf (tmp, "%s%c%s", dirct, DIRC, SUDIR);
    } else {
        sprintf (tmp, "%s%s", dirct, SUDIR);
    }

    /* find directories */
    dd = e_find_dir (tmp, 0);

    free (tmp);

    if (!dd) {
        return (df);
    }

    rec++;
    for (i = 0; i < dd->nr_files; i++) {
        if ((tmp = malloc (strlen (dirct) + strlen (dd->name[i]) + 3)) == NULL) {
            e_error (e_msg[ERR_LOWMEM], ERROR_MSG, window->edit_control->colorset);
            rec--;
            freedf (dd);
            return (df);
        }

        if (dirct[strlen (dirct) - 1] != DIRC) {
            sprintf (tmp, "%s%c%s", dirct, DIRC, dd->name[i]);
        } else {
            sprintf (tmp, "%s%s", dirct, dd->name[i]);
        }

        df = WpeSearchFiles (window, tmp, file, string, df, sw);
        free (tmp);
    }

    freedf (dd);
    rec--;
    return (df);

}


int
WpeGrepWindow (we_window_t * window)
{
    FIND *find = &(window->edit_control->find);
    int ret;
    char strTemp[80];
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o) {
        return (-1);
    }
    if (e_blck_dup (strTemp, window)) {
        strcpy (find->search, strTemp);
        find->sn = strlen (find->search);
    }
    o->xa = 7;
    o->ya = 3;
    o->xe = 63;
    o->ye = 19;
    o->bgsw = 0;
    o->name = "Grep";
    o->crsw = AltO;
    e_add_txtstr (4, 4, "Options:", o);
    e_add_wrstr (4, 2, 18, 2, 35, 128, 0, AltT, "Text to Find:", find->search,
                 &window->edit_control->sdf, o);
    e_add_wrstr (4, 10, 17, 10, 36, 128, 0, AltF, "File:", find->file,
                 &window->edit_control->fdf, o);
    e_add_wrstr (4, 12, 17, 12, 36, WPE_PATHMAX, 0, AltD, "Directory:",
                 find->dirct, &window->edit_control->ddf, o);
    e_add_sswstr (5, 5, 0, AltC, find->sw & 128 ? 1 : 0, "Case sensitive    ", o);
    e_add_sswstr (5, 6, 0, AltW, find->sw & 64 ? 1 : 0, "Whole words only  ", o);
    e_add_sswstr (5, 7, 0, AltR, find->sw & 32 ? 1 : 0, "Regular expression", o);
    e_add_sswstr (5, 8, 0, AltS, 0, "Search Recursive  ", o);
    e_add_bttstr (16, 14, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (34, 14, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        find->sw = 1024 + (o->sstr[0]->num << 7) + (o->sstr[1]->num << 6) +
                   (o->sstr[2]->num << 5) + (o->sstr[3]->num << 9);
        strcpy (find->search, o->wstr[0]->txt);
        find->sn = strlen (find->search);
        strcpy (find->file, o->wstr[1]->txt);
        if (find->dirct) {
            free (find->dirct);
        }
        find->dirct = WpeStrdup (o->wstr[2]->txt);
    }
    freeostr (o);
    if (ret != WPE_ESC) {
        ret = e_data_first (2, window->edit_control, find->dirct);
    }
    return (ret);
}


int
WpeFindWindow (we_window_t * window)
{
    FIND *find = &(window->edit_control->find);
    int ret;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o) {
        return (-1);
    }
    o->xa = 7;
    o->ya = 3;
    o->xe = 61;
    o->ye = 14;
    o->bgsw = 0;
    o->name = "Find File";
    o->crsw = AltO;
    e_add_txtstr (4, 6, "Options:", o);
    e_add_wrstr (4, 2, 15, 2, 36, 128, 0, AltF, "File:", find->file, &window->edit_control->fdf,
                 o);
    e_add_wrstr (4, 4, 15, 4, 36, WPE_PATHMAX, 0, AltD, "Directory:", find->dirct,
                 &window->edit_control->ddf, o);
    e_add_sswstr (5, 7, 0, AltS, 1, "Search Recursive  ", o);
    e_add_bttstr (13, 9, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (33, 9, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        find->sw = (o->sstr[0]->num << 9);
        strcpy (find->file, o->wstr[0]->txt);
        if (find->dirct) {
            free (find->dirct);
        }
        find->dirct = WpeStrdup (o->wstr[1]->txt);
    }
    freeostr (o);
    if (ret != WPE_ESC) {
        ret = e_data_first (3, window->edit_control, find->dirct);
    }
    return (ret);
}

int
e_ed_man (unsigned char *str, we_window_t * window)
{
    char command[256], tstr[_POSIX_PATH_MAX];
    char cc, hstr[80], nstr[10];
    int mdsv = window->edit_control->dtmd, bg, i, j = 0;
    we_buffer_t *buffer = 0;

    if (!str) {
        return (0);
    }
    while (isspace (*str++));
    if (!*--str) {
        return (0);
    }
    for (i = window->edit_control->mxedt; i >= 0; i--) {
        if (!strcmp (window->edit_control->window[i]->datnam, (const char *) str)) {
            e_switch_window (window->edit_control->edt[i], window);
            return (0);
        }
    }
    WpeMouseChangeShape (WpeWorkingShape);
    sprintf (tstr, "%s/%s", e_tmp_dir, str);
    for (i = 0; (hstr[i] = str[i]) && str[i] != '(' && str[i] != ')'; i++);
    hstr[i] = '\0';
    if (str[i] == '(') {
        for (++i; (nstr[j] = str[i]) && str[i] != ')' && str[i] != '(';
                i++, j++);

        /* Some SEE ALSO's are list as "foobar(3X)" but are in section 3 not 3X.
           This is a quick hack to fix the problem.  -- Dennis */
        if (isdigit (nstr[0])) {
            j = 1;
        }
    }
    nstr[j] = '\0';

    while (1) {
#ifdef MAN_S_OPT
        if (!nstr[0]) {
            sprintf (command, " man %s > \'%s\' 2> /dev/null", hstr, tstr);
        } else
            sprintf (command, " man -s %s %s > \'%s\' 2> /dev/null", nstr, hstr,
                     tstr);
#else
        sprintf (command, " man %s %s > \'%s\' 2> /dev/null", nstr, hstr, tstr);
#endif
        int ret = system (command);
        if (WIFSIGNALED (ret)
                && (WTERMSIG (ret) == SIGINT || WTERMSIG (ret) == SIGQUIT)) {
            printf ("System call command %s resulted in an interrupt.\n%s\n",
                    command, "Program will quit executing commands.\n");
            break;
        } else if (ret == 127) {
            printf ("System call command %s failed with code 127\n%s\n%s\n%s\n",
                    command,
                    "This could mean one of two things:",
                    "1. No shell was available (should never happen unless using chroot)",
                    "2. The command returned 127.\n");
            break;
        } else if (ret != 0) {
            printf ("System call command %s failed. Return code = %i.\n",
                    command, ret);
            break;
        }
        chmod (tstr, 0400);
        window->edit_control->dtmd = DTMD_HELP;
        e_edit (window->edit_control, tstr);
        window->edit_control->dtmd = mdsv;
        window = window->edit_control->window[window->edit_control->mxedt];
        buffer = window->buffer;
        if (buffer->mxlines > 1 || !nstr[1]) {
            break;
        }
        nstr[1] = '\0';
        chmod (tstr, 0600);
        remove (tstr);
        e_close_window (window);
    }
    if (buffer->mxlines == 1 && buffer->buflines[0].len == 0) {
        e_ins_nchar (window->buffer, window->screen, (unsigned char *) "No manual entry for ", 0, 0,
                     20);
        e_ins_nchar (window->buffer, window->screen, (unsigned char *) hstr, buffer->cursor.x, buffer->cursor.y,
                     strlen (hstr));
        e_ins_nchar (window->buffer, window->screen, (unsigned char *) ".", buffer->cursor.x, buffer->cursor.y, 1);
    }
    for (i = 0; i < buffer->mxlines; i++)
        if (buffer->buflines[i].len == 0 && (i == 0 || buffer->buflines[i - 1].len == 0)) {
            e_del_line (i, buffer, window->screen);
            i--;
        }
    for (bg = 0; bg < buffer->buflines[0].len && isspace (buffer->buflines[0].s[bg]); bg++);
    if (bg == buffer->buflines[0].len) {
        bg = 0;
    }
    for (i = 0;
            i < buffer->mxlines &&
            WpeStrnccmp ((const char *) buffer->buflines[i].s + bg,
                         (const char *) "\017SEE\005 \017ALSO\005", 12)
            && WpeStrnccmp ((const char *) buffer->buflines[i].s + bg,
                            (const char *) "SEE ALSO", 8); i++);
    if (i < buffer->mxlines)
        for (bg = 0, i++; i < buffer->mxlines && buffer->buflines[i].len > 0 && bg >= 0; i++) {
            bg = 0;
            while (buffer->buflines[i].s[bg]) {
                for (; isspace (buffer->buflines[i].s[bg]); bg++);
                if (!buffer->buflines[i].s[bg]) {
                    continue;
                }
                for (j = bg + 1;
                        buffer->buflines[i].s[j] && buffer->buflines[i].s[j] != ',' && buffer->buflines[i].s[j] != '.'
                        && buffer->buflines[i].s[j] != ' ' && buffer->buflines[i].s[j] != '('; j++);
                if (buffer->buflines[i].s[j] != '(') {
                    bg = -1;
                    break;
                }
                if (buffer->buflines[i].s[j - 1] == 5) {
                    e_del_nchar (buffer, window->screen, j - 1, i, 1);
                }
                for (j++;
                        buffer->buflines[i].s[j] && buffer->buflines[i].s[j] != ','
                        && buffer->buflines[i].s[j] != '.'; j++);
                if (buffer->buflines[i].s[bg] == 15) {
                    buffer->buflines[i].s[bg] = HFB;
                } else {
                    cc = HFB;
                    e_ins_nchar (buffer, window->screen, (unsigned char *) &cc, bg, i, 1);
                    j++;
                }
                cc = HED;
                e_ins_nchar (buffer, window->screen, (unsigned char *) &cc, j, i, 1);
                j++;
                if (buffer->buflines[i].s[j]) {
                    j++;
                }
                bg = j;
            }
        }
    buffer->cursor.x = buffer->cursor.y = 0;
    chmod (tstr, 0600);
    remove (tstr);
    WpeMouseRestoreShape ();
    e_write_screen (window, 1);
    return (0);
}

int
e_funct (we_window_t * window)
{
    char str[80];

    if (window->edit_control->hdf && window->edit_control->hdf->nr_files > 0) {
        strcpy (str, window->edit_control->hdf->name[0]);
    } else {
        str[0] = '\0';
    }
    if (e_add_arguments (str, "Function", window, 0, AltF, &window->edit_control->hdf)) {
        window->edit_control->hdf = e_add_df (str, window->edit_control->hdf);
        e_ed_man ((unsigned char *) str, window);
    }
    return (0);
}

struct dirfile *
e_make_funct (char *man)
{
    struct dirfile *df = NULL, *dout = malloc (sizeof (struct dirfile));
    char *sustr, *subpath, *manpath;
    int ret = 0, n, i = 0, j, k, l = 0;

    manpath = NULL;
    WpeMouseChangeShape (WpeWorkingShape);
    dout->nr_files = 0;
    dout->name = NULL;
    if (getenv ("MANPATH")) {
        manpath = strdup (getenv ("MANPATH"));
    }
    if ((!manpath) || (manpath[0] == '\0')) {
        manpath =
            strdup ("/usr/man:/usr/share/man:/usr/X11R6/man:/usr/local/man");
    }
    /* Allocate the maximum possible rather than continually realloc. */
    sustr = malloc (strlen (manpath) + 10);
    while (manpath[i]) {
        subpath = manpath + i;
        for (n = 0; (manpath[i]) && (manpath[i] != PTHD); i++, n++);
        if (manpath[i] == PTHD) {
            manpath[i] = '\0';
            i++;
        }
        sprintf (sustr, "%s/man%s/*", subpath, man);
        df = e_find_files (sustr, 0);
        if (!df->nr_files) {
            freedf (df);
            continue;
        }
        for (j = 0; j < df->nr_files; j++) {
            k = strlen (df->name[j]) - 1;

            /* If the file is gzipped strip the .gz ending. */
            if ((k > 2) && (strcmp (df->name[j] + k - 2, ".gz") == 0)) {
                k -= 3;
                *(df->name[j] + k + 1) = '\0';
            } else if ((k > 3) && (strcmp (df->name[j] + k - 3, ".bz2") == 0)) {
                k -= 4;
                *(df->name[j] + k + 1) = '\0';
            } else if ((k > 1) && (strcmp (df->name[j] + k - 1, ".Z") == 0)) {
                k -= 2;
                *(df->name[j] + k + 1) = '\0';
            }

            for (k = strlen (df->name[j]) - 1;
                    k >= 0 && *(df->name[j] + k) != '.'; k--)
                ;
            if (k >= 0 /**(df->name[j]+k)*/ ) {
                df->name[j] = realloc (df->name[j],
                                       (l =
                                            strlen (df->name[j]) +
                                            2) * sizeof (char));
                *(df->name[j] + k) = '(';
                *(df->name[j] + l - 2) = ')';
                *(df->name[j] + l - 1) = '\0';
            }
        }
        if (!dout->name) {
            dout->name = malloc (df->nr_files * sizeof (char *));
        } else
            dout->name =
                realloc (dout->name, (df->nr_files + dout->nr_files) * sizeof (char *));
        for (j = 0; j < df->nr_files; j++) {
            for (k = 0; k < dout->nr_files; k++) {
                if (!(ret = strcmp (df->name[j], dout->name[k]))) {
                    free (df->name[j]);
                    break;
                } else if (ret < 0) {
                    break;
                }
            }
            if (!ret && dout->nr_files) {
                continue;
            }
            for (l = dout->nr_files; l > k; l--) {
                dout->name[l] = dout->name[l - 1];
            }
            dout->name[k] = df->name[j];
            dout->nr_files++;
        }
        free (df);
    }
    free (manpath);
    free (sustr);
    WpeMouseRestoreShape ();
    return (dout);
}


#ifdef PROG
extern struct dirfile **e_p_df;
#endif

int
e_data_first (int sw, we_control_t * control, char *nstr)
{
    extern char *e_hlp_str[];
    extern WOPT *gblst, *oblst;
    we_window_t *window;
    int i, j;
    struct dirfile *df = NULL;
    FLWND *fw;

    if (control->mxedt >= max_edit_windows()) {
        e_error (e_msg[ERR_MAXWINS], ERROR_MSG, control->colorset);
        return (-1);
    }
    for (j = 1; j <= max_edit_windows(); j++) {
        for (i = 1; i <= control->mxedt && control->edt[i] != j; i++);
        if (i > control->mxedt) {
            break;
        }
    }
    control->curedt = j;
    (control->mxedt)++;
    control->edt[control->mxedt] = j;

    if ((window = (we_window_t *) malloc (sizeof (we_window_t))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }
    if ((fw = (FLWND *) malloc (sizeof (FLWND))) == NULL) {
        e_error (e_msg[ERR_LOWMEM], SERIOUS_ERROR_MSG, control->colorset);
    }
    window->colorset = control->colorset;
    control->window[control->mxedt] = window;
    window->a = e_set_pnt (22, 3);
    window->e = e_set_pnt (window->a.x + 35, window->a.y + 18);
    window->winnum = control->curedt;
    window->dtmd = DTMD_DATA;
    window->ins = sw;
    window->save = 0;
    window->zoom = 0;
    window->edit_control = control;
    window->c_sw = NULL;
    window->c_st = NULL;
    window->view = NULL;
    window->find.dirct = NULL;

    if (!nstr) {
        window->dirct = NULL;    /* how about a free up ??? */
    } else {
        window->dirct = malloc (strlen (nstr) + 1);
        strcpy (window->dirct, nstr);
    }
    WpeMouseChangeShape (WpeWorkingShape);
    if (sw == 1) {
        window->datnam = "Function-Index";
        df = e_make_funct (nstr);
    } else if (sw == 2) {
        window->datnam = "Grep";
        df = WpeSearchFiles (window, nstr, control->find.file,
                             control->find.search, NULL, control->find.sw);
    } else if (sw == 3) {
        window->datnam = "Find";
        df = WpeSearchFiles (window, nstr, control->find.file,
                             control->find.search, NULL, control->find.sw);
    } else if (sw == 7) {
        window->datnam = "Windows";
        df = e_make_win_list (window);
    }
#ifdef PROG
    else if (sw == 4) {
        window->datnam = "Project";
        df = e_p_df[0];
    } else if (sw == 5) {
        window->datnam = "Variables";
        df = e_p_df[1];
    } else if (sw == 6) {
        window->datnam = "Install";
        df = e_p_df[2];
    }
#endif
    window->hlp_str = e_hlp_str[16 + sw];
    if (sw < 4) {
        window->blst = gblst;
        window->nblst = 4;
    } else {
        window->blst = oblst;
        window->nblst = 4;
    }
    WpeMouseRestoreShape ();
    window->buffer = (we_buffer_t *) fw;
    fw->df = df;

    fw->mxa = window->a.x;
    fw->mxe = window->e.x;
    fw->mya = window->a.y;
    fw->mye = window->e.y;
    fw->xa = window->a.x + 3;
    fw->xe = window->e.x - 13;
    fw->ya = window->a.y + 3;
    fw->ye = window->e.y - 1;
    fw->window = window;
    fw->ia = fw->nf = fw->nxfo = fw->nyfo = 0;
    fw->srcha = fw->ja = 0;

    if (control->mxedt > 1 && (window->ins < 5 || window->ins == 7)) {
        e_ed_rahmen (control->window[control->mxedt - 1], 0);
    }
    e_firstl (window, 1);
    e_data_schirm (window);
    return (0);
}

int
e_data_schirm (we_window_t * window)
{
    int i, j;
    FLWND *fw = (FLWND *) window->buffer;

    for (j = window->a.y + 1; j < window->e.y; j++)
        for (i = window->a.x + 1; i < window->e.x; i++) {
            e_pr_char (i, j, ' ', window->colorset->nt.fg_bg_color);
        }

    if (num_cols_on_screen(window) > 25) {
        if (window->ins < 4 || window->ins == 7)
            e_pr_str ((window->e.x - 9), window->e.y - 4, "Show", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
        else if (window->ins > 3) {
            e_pr_str ((window->e.x - 9), window->e.y - 8, "Add", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
            e_pr_str ((window->e.x - 9), window->e.y - 6, "Edit", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
            e_pr_str ((window->e.x - 9), window->e.y - 4, "Delete", window->colorset->nz.fg_bg_color, 0, -1,
                      window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
            if (window->ins == 4 && window->a.y < window->e.y - 10) {
                e_pr_str ((window->e.x - 9), window->e.y - 10, "Options", window->colorset->nz.fg_bg_color,
                          0, -1, window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
            }
        }
        e_pr_str ((window->e.x - 9), window->e.y - 2, "Cancel", window->colorset->nz.fg_bg_color, -1, -1,
                  window->colorset->ns.fg_bg_color, window->colorset->nt.fg_bg_color);
    }

    if (num_cols_on_screen(window) > 25) {
        fw->xa = window->a.x + 3;
        fw->xe = window->e.x - 13;
    } else {
        fw->xa = window->a.x + 3;
        fw->xe = window->e.x - 2;
    }
    fw->mxa = window->a.x;
    fw->mxe = window->e.x;
    fw->mya = window->a.y;
    fw->mye = window->e.y;
    fw->xa = window->a.x + 3;
    fw->ya = window->a.y + 3;
    fw->ye = window->e.y - 1;
#ifdef PROG
    if (window->ins == 4) {
        fw->df = e_p_df[0];
    }
#endif
    if (window->ins == 1)
        e_pr_str (fw->xa, window->a.y + 2, "Functions:", window->colorset->nt.fg_bg_color, 0, 1,
                  window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
    else if (window->ins == 3)
        e_pr_str (fw->xa, window->a.y + 2, "Directories:", window->colorset->nt.fg_bg_color, 0, 1,
                  window->colorset->nsnt.fg_bg_color, window->colorset->nt.fg_bg_color);
    e_mouse_bar (fw->xe, fw->ya, fw->ye - fw->ya, 0, fw->window->colorset->em.fg_bg_color);
    e_mouse_bar (fw->xa, fw->ye, fw->xe - fw->xa, 1, fw->window->colorset->em.fg_bg_color);
    e_pr_file_window (fw, 0, 1, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color, window->colorset->frft.fg_bg_color);
    return (0);
}

int
e_data_eingabe (we_control_t * control)
{
    we_window_t *window = control->window[control->mxedt];
    FLWND *fw = (FLWND *) window->buffer;
    int c = AltF;

    fk_u_cursor (0);
    if (window->ins == 7) {
        freedf (fw->df);
        fw->df = e_make_win_list (window);
    }
    while (c != WPE_ESC) {
        if (window->dtmd != DTMD_DATA) {
            return (0);
        }
#ifdef PROG
        if (window->ins == 4) {
            fw->df = e_p_df[0];
        }
#endif
        if (c == AltF) {
            c = e_file_window (0, fw, window->colorset->ft.fg_bg_color, window->colorset->fz.fg_bg_color);
        }
#if  MOUSE
        if (c == MBKEY) {
            c = e_data_ein_mouse (window);
        }
#endif
        if (((c == WPE_CR || c == AltS) && (window->ins < 4 || window->ins == 7)) ||
                ((c == AltA || c == EINFG) && (window->ins > 3 && window->ins < 7))) {
            if (window->ins == 1) {
                e_ed_man ((unsigned char *) fw->df->name[fw->nf], window);
            } else if (window->ins == 2) {
                e_edit (window->edit_control, fw->df->name[fw->nf]);
                e_repeat_search (window->edit_control->window[window->edit_control->mxedt]);
            } else if (window->ins == 3) {
                e_edit (window->edit_control, fw->df->name[fw->nf]);

                /*        WpeCreateFileManager(0, window->edit_control, fw->df->name[fw->nf]); */
            } else if (window->ins == 7) {
                e_switch_window (window->edit_control->edt[fw->df->nr_files - fw->nf], window);
            }
#ifdef PROG
            else if (window->ins == 4) {
                WpeCreateFileManager (5, window->edit_control, NULL);
                while (WpeHandleFileManager (window->edit_control) != WPE_ESC);
                e_p_df[window->ins - 4] = fw->df;
                c = AltF;
                window->save = 1;
            } else if (window->ins > 4 && window->ins < 7) {
                e_p_add_df (fw, window->ins);
                e_p_df[window->ins - 4] = fw->df;
                c = AltF;
            }
#endif
            if (window->ins < 4 || window->ins == 7) {
                return (0);
            }
        }
#ifdef PROG
        else if (window->ins > 3 && window->ins < 7 && (c == AltD || c == ENTF)) {
            e_p_del_df (fw, window->ins);
            c = AltF;
            window->save = 1;
        } else if (window->ins > 4 && window->ins < 7 && (c == AltE || c == WPE_CR)) {
            e_p_edit_df (fw, window->ins);
            c = AltF;
            window->save = 1;
        } else if (window->ins == 4 && (c == AltE || c == WPE_CR)) {
            e_edit (window->edit_control, fw->df->name[fw->nf]);
            c = WPE_ESC;
        } else if (window->ins == 4 && c == AltO) {
            e_project_options (window);
            c = AltF;
        }
#endif
        else if (c != AltF) {
            if (c == AltBl) {
                c = WPE_ESC;
            } else if (c == WPE_ESC) {
                c = (window->edit_control->edopt & ED_CUA_STYLE) ? CF4 : AF3;
            }
            if (window->ins == 7 && ((!(window->edit_control->edopt & ED_CUA_STYLE) && c == AF3)
                                     || ((window->edit_control->edopt & ED_CUA_STYLE) && c == CF4))) {
                e_close_window (window);
            }
            if (window->ins == 4 && ((!(window->edit_control->edopt & ED_CUA_STYLE) && c == AF3)
                                     || ((window->edit_control->edopt & ED_CUA_STYLE) && c == CF4))) {
                FLWND *fw = (FLWND *) window->edit_control->window[window->edit_control->mxedt]->buffer;
                fw->df = NULL;
                e_close_window (window->edit_control->window[window->edit_control->mxedt]);
                return (0);
            }
            if (window->ins == 4 && (!e_tst_dfkt (window, c) || !e_prog_switch (window, c))) {
                return (0);
            }
            if (window->ins > 4 && ((!(window->edit_control->edopt & ED_CUA_STYLE) && c == AF3)
                                    || ((window->edit_control->edopt & ED_CUA_STYLE) && c == CF4))) {
                return (c);
            } else if ((window->ins < 4 || window->ins == 7) && !e_tst_dfkt (window, c)) {
                return (0);
            } else {
                c = AltF;
            }
        }
    }
    return ((window->edit_control->edopt & ED_CUA_STYLE) ? CF4 : AF3);
}

int
e_get_funct_in (char *nstr, we_window_t * window)
{
    return (e_data_first (1, window->edit_control, nstr));
}

int
e_funct_in (we_window_t * window)
{
    int n, xa = 37, ya = 2, num = 8;
    OPTK *opt = malloc (num * sizeof (OPTK));
    char nstr[2];

    opt[0].t = "User Commands";
    opt[0].x = 0;
    opt[0].o = 'U';
    opt[1].t = "System Calls";
    opt[1].x = 0;
    opt[1].o = 'S';
    opt[2].t = "C-Lib.-Functions";
    opt[2].x = 0;
    opt[2].o = 'C';
    opt[3].t = "Devices & Netw. I.";
    opt[3].x = 0;
    opt[3].o = 'D';
    opt[4].t = "File Formats";
    opt[4].x = 0;
    opt[4].o = 'F';
    opt[5].t = "Games & Demos";
    opt[5].x = 0;
    opt[5].o = 'G';
    opt[6].t = "Environment, ...";
    opt[6].x = 0;
    opt[6].o = 'E';
    opt[7].t = "Maintenance Com.";
    opt[7].x = 0;
    opt[7].o = 'M';

    n = e_opt_sec_box (xa, ya, num, opt, window, 1);

    free (opt);
    if (n < 0) {
        return (WPE_ESC);
    }

    nstr[0] = '1' + n;
    nstr[1] = '\0';
    return (e_get_funct_in (nstr, window));
}

#endif
