/** \file we_edit.c                                        */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <string.h>
#include <ctype.h>
#include "keys.h"
#include "messages.h"
#include "options.h"
#include "model.h"
#include "we_control.h"
#include "edit.h"
#include "we_edit.h"
#include "utils.h"
#include "we_progn.h"
#include "we_prog.h"
#include "WeString.h"
#include "we_wind.h"
#include "we_fl_fkt.h"

#ifdef UNIX
#include<sys/types.h>		/*  included for digital station  */
#include<sys/stat.h>
#include <unistd.h>
#endif

int global_disable_add_undo = 0;
/**
 * e_phases enumerates the phases of editting as far as undo and redo are concerned.
 *
 * In the EDIT_PHASE the program is editting and changes are queued in the undo queue.
 * In the UNDO_PHASE the program undoes previous changes and moves the undo registration to
 * the redo queue.
 * In the UNDO_PHASE the program is redoing an undone action and moved the undo registration
 * from the redo queue to the undo queue.
 *
 */
enum e_phases {EDIT_PHASE, UNDO_PHASE, REDO_PHASE};
static enum e_phases e_phase = EDIT_PHASE;

/* prototypes of private functions */
int e_make_rudo(we_window_t* window, int sw);
int e_del_a_ind (we_buffer_t * buffer, we_screen_t * s);
int e_tab_a_ind (we_buffer_t * buffer, we_screen_t * s);
int e_car_ret(we_buffer_t* buffer, we_screen_t* s);
_Bool e_undo_is_active();
void e_prepare_buffer_for_undo(we_buffer_t *buffer);
we_undo_t * e_create_undo(int undo_type, we_buffer_t *buffer, int x, int y, int n);
void e_add_new_undo(we_buffer_t *buffer, we_undo_t *next);
int e_process_undo_default (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_a_char_delete (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_p_char_put (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_r_search_replace (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_s_search_replace (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_l_line_delete (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_c_copy_paste (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_v_copy_paste (we_buffer_t * buffer, we_undo_t *next);
int e_process_undo_d_block_delete (we_buffer_t * buffer, we_undo_t *next);

#ifdef PROG
we_buffer_t *e_p_m_buffer = NULL;
#ifdef DEBUGGER
we_buffer_t *e_p_w_buffer = NULL;
#endif
#endif

/* open edit window */
int
e_edit (we_control_t * control, char *filename)
{
    extern char *e_hlp_str[];
    extern WOPT *eblst, *hblst, *mblst, *dblst;
    FILE *fp = NULL;
    we_window_t *window, *fo;
    char *complete_fname, *path, *file;
    int ftype = 0, i, j, st = 0;
    struct stat buf[1];

    /* Allows project files to be open automatically */
    j = strlen (filename);
    if ((WpeIsProg ()) && (j > 4) && (!strcmp (&filename[j - 4], ".prj")))
    {
        if (!e_prog.project)
        {
            e_prog.project = malloc (1);
            e_prog.project[0] = '\0';
        }
        else
        {
            for (i = control->mxedt; i > 0 &&
                    (control->window[i]->dtmd != DTMD_DATA || control->window[i]->ins != 4); i--)
                ;
            if (i > 0)
            {
                e_switch_window (control->edt[i], control->window[control->mxedt]);
                e_close_window (control->window[control->mxedt]);
            }
        }
        e_prog.project = realloc (e_prog.project, j + 1);
        strcpy (e_prog.project, filename);
        e_make_prj_opt (control->window[control->mxedt]);
        /************************************/
        e_rel_brkwtch (control->window[control->mxedt]);
        /************************************/
        e_prj_ob_file (control->window[control->mxedt]);
        return 0;
    }

    /* Check to see if the file is already opened BD */
    WpeFilenameToPathFile (filename, &path, &file);
    /** \todo Should check for error here */
    for (i = control->mxedt; i >= 0; i--)
    {
        if ((strcmp (control->window[i]->datnam, file) == 0) &&
                (strcmp (control->window[i]->dirct, path) == 0))
        {
            e_switch_window (control->edt[i], control->window[control->mxedt]);
            free (path);
            free (file);
            return (0);
        }
    }

    if (control->mxedt >= MAXEDT)
    {
        e_error (e_msg[ERR_MAXWINS], 0, control->colorset);
        return (-1);
    }
    if (stat (filename, buf) == 0)
    {
        if (!S_ISREG (buf->st_mode))
        {
            /* error message should go here */
            return (-1);
        }
    }
    for (j = 1; j <= MAXEDT; j++)
    {
        for (i = 1; i <= control->mxedt && control->edt[i] != j; i++);
        if (i > control->mxedt)
            break;
    }
    control->curedt = j;
    (control->mxedt)++;
    control->edt[control->mxedt] = j;

    if ((window = (we_window_t *) malloc (sizeof (we_window_t))) == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, control->colorset);

    window->colorset = control->colorset;
    control->window[control->mxedt] = window;

    if ((window->buffer = (we_buffer_t *) malloc (sizeof (we_buffer_t))) == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
    if ((window->screen = (we_screen_t *) malloc (sizeof (we_screen_t))) == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
    if ((window->buffer->buflines = (STRING *) malloc (MAXLINES * sizeof (STRING))) == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, window->colorset);
#ifdef PROG
    for (i = control->mxedt - 1;
            i > 0 && (!strcmp (control->window[i]->datnam, "Messages")
                      || !DTMD_ISTEXT (control->window[i]->dtmd)
                      || !strcmp (control->window[i]->datnam, "Watches")
                      || !strcmp (control->window[i]->datnam, "Stack")); i--)
        ;
    for (j = control->mxedt - 1; j > 0 && !st; j--)
        if (!strcmp (control->window[j]->datnam, "Stack"))
            st = 1;
#else
    for (i = control->mxedt - 1; i > 0 && !DTMD_ISTEXT (control->window[i]->dtmd); i--)
        ;
#endif
#ifdef PROG
    if (WpeIsProg ())
    {
        if ((e_we_sw & 8) || !strcmp (filename, "Messages") ||
                !strcmp (filename, "Watches"))
        {
            window->a = e_set_pnt (0, 2 * MAXSLNS / 3 + 1);
            window->e = e_set_pnt (MAXSCOL - 1, MAXSLNS - 2);
        }
        else if (!strcmp (filename, "Stack"))
        {
            window->a = e_set_pnt (2 * MAXSCOL / 3, 1);
            window->e = e_set_pnt (MAXSCOL - 1, 2 * MAXSLNS / 3);
        }
        else
        {
            if (i < 1)
            {
                window->a = e_set_pnt (0, 1);
                window->e =
                    e_set_pnt (st ? 2 * MAXSCOL / 3 - 1 : MAXSCOL - 1,
                               2 * MAXSLNS / 3);
            }
            else
            {
                window->a = e_set_pnt (control->window[i]->a.x + 1, control->window[i]->a.y + 1);
                window->e =
                    e_set_pnt (st ? 2 * MAXSCOL / 3 - 1 : control->window[i]->e.x,
                               control->window[i]->e.y);
            }
        }
    }
    else
#endif
    {
        if (i < 1)
        {
            window->a = e_set_pnt (0, 1);
            window->e = e_set_pnt (MAXSCOL - 1, MAXSLNS - 2);
        }
        else
        {
            window->a = e_set_pnt (control->window[i]->a.x + 1, control->window[i]->a.y + 1);
            window->e = e_set_pnt (control->window[i]->e.x, control->window[i]->e.y);
        }
    }
    if (num_cols_on_screen(window) < 26)
        window->a.x = window->e.x - 26;
    if (num_lines_on_screen(window) < 3)
        window->a.y = window->e.y - 3;
    window->winnum = control->curedt;
    window->dtmd = control->dtmd;
    window->ins = 1;
    window->save = 0;
    window->zoom = 0;
    window->edit_control = control;
    window->view = NULL;
    window->hlp_str = e_hlp_str[0];
    window->blst = eblst;
    window->nblst = 7;
    window->buffer->window = window;
    window->buffer->cursor = e_set_pnt (0, 0);
    window->buffer->cl = window->buffer->clsv = 0;
    window->buffer->mx = e_set_pnt (control->maxcol, MAXLINES);
    window->buffer->mxlines = 0;
    window->buffer->colorset = window->colorset;
    window->buffer->control = control;
    window->buffer->undo = NULL;
    window->buffer->redo = NULL;
    window->find.dirct = NULL;
    if (WpeIsProg ())
        e_add_synt_tl (filename, window);
    else
    {
        window->c_st = NULL;
        window->c_sw = NULL;
    }
    if ((window->edit_control->edopt & ED_ALWAYS_AUTO_INDENT) ||
            ((window->edit_control->edopt & ED_SOURCE_AUTO_INDENT) && window->c_st))
    {
        window->flg = 1;
    }
    else
    {
        window->flg = 0;
    }
    window->screen->c = e_set_pnt (0, 0);
    window->screen->ks = e_set_pnt (0, 0);
    window->screen->mark_begin = e_set_pnt (0, 0);
    window->screen->mark_end = e_set_pnt (0, 0);
    window->screen->fa = e_set_pnt (0, 0);
    window->screen->fe = e_set_pnt (0, 0);
    window->screen->colorset = window->colorset;
#ifdef DEBUGGER
    window->screen->brp = malloc (sizeof (int));
    window->screen->brp[0] = 0;
    window->screen->da.y = -1;
#endif
    window->dirct = path;
    for (i = 0; i < 9; i++)
        window->screen->pt[i] = e_set_pnt (-1, -1);
    if (control->mxedt == 0)		/*  Clipboard  */
    {
        control->curedt = 0;
        control->edt[control->mxedt] = 0;
        free (file);
        file = window->datnam = WpeStrdup (BUFFER_NAME);
#ifdef UNIX
        window->filemode = 0600;
#endif
        e_new_line (0, window->buffer);
        *(window->buffer->buflines[0].s) = WPE_WR;
        *(window->buffer->buflines[0].s + 1) = '\0';
        window->buffer->buflines[0].len = 0;
        window->buffer->buflines[0].nrc = 1;
        return (0);
    }
    if (strcmp (file, "") == 0)
    {
        free (file);
        file = window->datnam = WpeStrdup ("Noname");
    }
    else
    {
        window->datnam = file;
    }
    if (strcmp (filename, "Help") == 0)
    {
        complete_fname = e_mkfilename (LIBRARY_DIR, HELP_FILE);
        window->dtmd = DTMD_HELP;
        window->ins = 8;
        window->hlp_str = e_hlp_str[25];
        window->nblst = 7;
        window->blst = hblst;
        ftype = 1;
    }
    else
        complete_fname = e_mkfilename (window->dirct, window->datnam);
#ifdef PROG
    if (WpeIsProg ())
    {
        if (!strcmp (filename, "Messages"))
        {
            window->ins = 8;
            window->hlp_str = e_hlp_str[3];
            window->nblst = 8;
            window->blst = mblst;
            ftype = 2;
        }
        else if (!strcmp (filename, "Watches"))
        {
            window->ins = 8;
            window->hlp_str = e_hlp_str[1];
            window->blst = dblst;
            ftype = 3;
        }
        else if (!strcmp (filename, "Stack"))
        {
            window->ins = 8;
            window->hlp_str = e_hlp_str[2];
            window->blst = dblst;
            ftype = 4;
        }
    }
#endif
    if (ftype != 1)
        fp = fopen (complete_fname, "rb");
    if (fp != NULL && access (complete_fname, W_OK) != 0)
        window->ins = 8;
#ifdef UNIX
    if (fp != NULL)
    {
        stat (complete_fname, buf);
        window->filemode = buf->st_mode;
    }
    else
    {
        umask (i = umask (077));
        window->filemode = 0666 & ~i;
    }
#endif
    free (complete_fname);

    if (fp != NULL && ftype != 1)
    {
        e_readin (0, 0, fp, window->buffer, &window->dtmd);
        if (fclose (fp) != 0)
            e_error (e_msg[ERR_FCLOSE], 0, control->colorset);
        if (control->dtmd == DTMD_HELP)
            control->dtmd = DTMD_NORMAL;
#ifdef PROG
        if (WpeIsProg ())
        {
            if (e_we_sw & 8)
            {
                strcpy (window->datnam, "Messages");
                e_we_sw &= ~8;
            }
            if (!strcmp (window->datnam, "Messages"))
            {
                e_make_error_list (window);
                window->ins = 8;
            }
        }
#endif
    }
    else
    {
        e_new_line (0, window->buffer);
        *(window->buffer->buflines[0].s) = WPE_WR;
        *(window->buffer->buflines[0].s + 1) = '\0';
        window->buffer->buflines[0].len = 0;
        window->buffer->buflines[0].nrc = 1;
    }
#ifdef PROG
    if (ftype == 2)
    {
        if (e_p_m_buffer != NULL)
        {
            e_close_buffer (window->buffer);
            window->buffer = e_p_m_buffer;
            window->buffer->window = window;
        }
        else
        {
            e_p_m_buffer = window->buffer;
            free (window->buffer->buflines[0].s);
            window->buffer->mxlines = 0;
        }
    }
#ifdef DEBUGGER
    if (ftype == 3)
    {
        if (e_p_w_buffer != NULL)
        {
            e_close_buffer (window->buffer);
            window->buffer = e_p_w_buffer;
            window->buffer->window = window;
        }
        else
        {
            e_p_w_buffer = window->buffer;
            /*  e_ins_nchar(window->buffer, window->s, "No Watches", 0, 0, 10);*/
        }
    }
#endif
#endif
    if (window->c_sw)
    {
        window->c_sw = e_sc_txt (window->c_sw, window->buffer);
    }
    if (control->mxedt > 1)
    {
        fo = control->window[control->mxedt - 1];
        e_ed_rahmen (fo, 0);
    }
    e_firstl (window, 1);
    e_zlsplt (window);
    e_brk_schirm (window);
    e_write_screen (window, 1);
    e_cursor (window, 1);
    return (0);
}

/*   keyboard output routine */
int
e_eingabe (we_control_t * e)
{
    we_buffer_t *buffer = e->window[e->mxedt]->buffer;
    we_screen_t *s = e->window[e->mxedt]->screen;
    we_window_t *window = e->window[e->mxedt];
    int ret, c = 0;
    unsigned char cc;

    fk_cursor (1);
    while (c != WPE_ESC)
    {
        if (e->mxedt < 1)
            c = WpeHandleMainmenu (-1, window);
        else if (!DTMD_ISTEXT (window->dtmd))
            return (0);
        else
        {
            if (window->save > window->edit_control->maxchg)
                e_autosave (window);
#if  MOUSE
            if ((c = e_getch ()) < 0)
                cc = c = e_edt_mouse (c, window);
            else
                cc = c;
#else
            cc = c = e_getch ();
#endif
        }
        if ((c > 31 || (c == WPE_TAB && !(window->flg & 1)) ||
                (window->ins > 1 && window->ins != 8)) && c < 255)
        {
            if (window->ins == 8)
                continue;
            if (window->ins == 0 || window->ins == 2)
                e_put_char (c, buffer, s);
            else
                e_ins_nchar (buffer, s, &cc, buffer->cursor.x, buffer->cursor.y, 1);
            e_write_screen (window, 1);
        }
        else if (c == WPE_DC)
        {
            if (window->ins == 8)
            {
                if (window->dtmd == DTMD_HELP)
                    e_help_last (window);
                continue;
            }
            if (buffer->cursor.y > 0 || buffer->cursor.x > 0)
            {
                if (buffer->buflines[buffer->cursor.y].len == 0)
                {
                    e_del_line (buffer->cursor.y, buffer, s);
                    buffer->cursor.y--;
                    buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
                    if (*(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) == '\0')
                        buffer->cursor.x--;
                }
                else
                {
                    if (buffer->cursor.x > 0)
                        buffer->cursor.x--;
                    else
                    {
                        buffer->cursor.y--;
                        buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
                        if (*(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) == '\0')
                            buffer->cursor.x--;
                    }
                    if (window->flg & 1)
                        e_del_a_ind (buffer, s);
                    else
                        e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, 1);
                }
                e_write_screen (window, 1);
            }
        }
        else if (c == ENTF || c == 4)
        {
            if (window->ins == 8)
            {
#ifdef DEBUGGER
                if (WpeIsProg ())
                    e_d_is_watch (c, window);
#endif
                continue;
            }
            if (*(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) != '\0' &&
                    (buffer->cursor.y < buffer->mxlines - 1
                     || *(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) != WPE_WR))
            {
                e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, 1);
                e_write_screen (window, 1);
            }
        }
        else if (c == WPE_CR)
        {
#ifdef PROG
            if (window->ins == 8)
            {
                if (window->dtmd == DTMD_HELP)
                {
                    e_help_ret (window);
                    goto weiter;
                }
                if (WpeIsProg ())
                {
                    e_d_car_ret (window);
                    goto weiter;
                }
                else
                    continue;
            }
#else
            if (window->ins == 8)
                continue;
#endif
            e_car_ret (buffer, s);
            e_write_screen (window, 1);
        }
        else if (c == WPE_TAB)
        {
            e_tab_a_ind (buffer, s);
            e_write_screen (window, 1);
        }
        /*
                  else if(c == WPE_TAB)
                  {    if(window->ins == 8) continue;
                       if (window->ins == 0 || window->ins == 2)
                         for(c = window->edit_control->tabn - buffer->cursor.x % window->edit_control->tabn; c > 0
                                                      && buffer->cursor.x < buffer->mx.x; c--)
                      		{   e_put_char(' ', buffer, s);  buffer->cursor.x++;  }
                       else  e_ins_nchar(buffer, s, window->edit_control->tabs, buffer->cursor.x, buffer->cursor.y,
                                               window->edit_control->tabn - buffer->cursor.x % window->edit_control->tabn);
                       e_write_screen(buffer, s, window, 1);
                  }
        */
        else
        {
            ret = e_tst_cur (c, e);	/*up/down arrows go this way */
            if (ret != 0)
                ret = e_tst_fkt (c, e);
        }
weiter:
        window = e->window[e->mxedt];
        if (e->mxedt > 0 && DTMD_ISTEXT (window->dtmd))
        {
            buffer = window->buffer;
            s = window->screen;
            s->ks.x = buffer->cursor.x;
            s->ks.y = buffer->cursor.y;
            e_cursor (window, 1);
            if ((c & 511) != CUP && (c & 511) != CDO)
                buffer->clsv = buffer->cl;
            e_zlsplt (window);
        }
    }
    return (c);
}

/*              Interpretation of cursor keys
                     ( Primitive editor )                    */
int
e_tst_cur (int c, we_control_t * e)
{
    we_window_t *window = e->window[e->mxedt];
    we_buffer_t *buffer = window->buffer;
    we_screen_t *s = window->screen;

    switch (c)
    {
    case CtrlP:
    case CUP:
    case CUP + 512:
        if (buffer->cursor.y > 0)
            (buffer->cursor.y)--;
        buffer->cursor.x = e_chr_sp (buffer->clsv, buffer, window);
        break;
    case CtrlN:
    case CDO:
    case CDO + 512:
        if (buffer->cursor.y < buffer->mxlines - 1)
            (buffer->cursor.y)++;
        buffer->cursor.x = e_chr_sp (buffer->clsv, buffer, window);
        break;
    case CtrlB:
    case CLE:
    case CLE + 512:
        (buffer->cursor.x)--;
        if (buffer->cursor.x < 0)
        {
            if (buffer->cursor.y > 0)
            {
                (buffer->cursor.y)--;
                buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
            }
            else
            {
                buffer->cursor.x = 0;
            }
        }
        break;
    case CtrlF:
    case CRI:
    case CRI + 512:
        (buffer->cursor.x)++;
        if (buffer->cursor.x > buffer->buflines[buffer->cursor.y].len)
        {
            if (buffer->cursor.y < buffer->mxlines - 1)
            {
                (buffer->cursor.y)++;
                buffer->cursor.x = 0;
            }
            else
            {
                buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
            }
        }
        break;

    case CCLE:
    case CCLE + 512:
        if (buffer->cursor.x <= 0 && buffer->cursor.y > 0)
        {
            buffer->cursor.y--;
            buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
        }
        else if (buffer->cursor.x > 0)
            buffer->cursor.x = e_su_rblk (buffer->cursor.x - 1, buffer->buflines[buffer->cursor.y].s);
        break;
    case CCRI:
    case CCRI + 512:
        if (buffer->cursor.x >= buffer->buflines[buffer->cursor.y].len && buffer->cursor.y < buffer->mxlines)
        {
            buffer->cursor.x = 0;
            buffer->cursor.y++;
        }
        else if (buffer->cursor.x < buffer->buflines[buffer->cursor.y].len)
            buffer->cursor.x = e_su_lblk (buffer->cursor.x, buffer->buflines[buffer->cursor.y].s);
        break;
    case BDO:
    case BDO + 512:
        buffer->cursor.y = buffer->cursor.y + num_lines_on_screen(window) - 2;
        if (buffer->cursor.y > buffer->mxlines - 1)
            buffer->cursor.y = buffer->mxlines - 1;
        e_write_screen (window, 1);
        e_cursor (window, 1);
        break;
    case BUP:
    case BUP + 512:
        buffer->cursor.y = buffer->cursor.y - window->e.y + window->a.y + 2;
        if (buffer->cursor.y < 0)
            buffer->cursor.y = 0;
        e_write_screen (window, 1);
        e_cursor (window, 1);
        break;
    case CBDO:
    case CBDO + 512:
        buffer->cursor.y = buffer->mxlines - 1;
        buffer->cursor.x = buffer->buflines[buffer->mxlines - 1].len;
        e_write_screen (window, 1);
        break;
    case CBUP:
    case CBUP + 512:
        if (buffer->cursor.y != 0)
        {
            buffer->cursor.x = 0;
            buffer->cursor.y = 0;
            e_write_screen (window, 1);
        }
        break;
    case CEND:
    case CEND + 512:
        if (line_num_on_screen_bottom(window) < buffer->mxlines)
            buffer->cursor.y = line_num_on_screen_bottom(window) - 1;
        else
            buffer->cursor.y = buffer->mxlines - 1;
        buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
        break;
    case CPS1:
    case CPS1 + 512:
        buffer->cursor.x = 0;
        buffer->cursor.y = s->c.y;
        break;
    case AltI:
    case EINFG:
        if (window->ins == 8)
        {
#ifdef DEBUGGER
            if (WpeIsProg ())
                e_d_is_watch (c, window);
#endif
            break;
        }
        if (window->ins & 1)
            window->ins &= ~1;
        else
            window->ins |= 1;
#ifdef NEWSTYLE
        e_ed_rahmen (window, 1);
#else
        e_pr_filetype (window);
#endif
        break;
    case AltJ:
        if (window->ins == 8)
            break;
        if (window->ins & 2)
            window->ins &= ~2;
        else
            window->ins |= 2;
#ifdef NEWSTYLE
        e_ed_rahmen (window, 1);
#else
        e_pr_filetype (window);
#endif
        break;
    case CtrlA:
    case POS1:
    case POS1 + 512:
        buffer->cursor.x = 0;
        break;
    case CtrlE:
    case ENDE:
    case ENDE + 512:
        buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
        break;
    case CtrlT:
        if (window->ins == 8)
            break;
        if (buffer->cursor.x < buffer->buflines[buffer->cursor.y].len)
        {
            c = e_su_lblk (buffer->cursor.x, buffer->buflines[buffer->cursor.y].s);
            e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, c - buffer->cursor.x);
        }
        else if (*(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) == WPE_WR)
            e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, 1);
        else if (buffer->cursor.x >= buffer->buflines[buffer->cursor.y].len && buffer->cursor.y < buffer->mxlines)
        {
            buffer->cursor.x = 0;
            (buffer->cursor.y)++;
        }
        e_write_screen (window, 1);
        break;
    case CtrlZ:
        if (window->ins == 8)
            break;
        e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, buffer->buflines[buffer->cursor.y].len - buffer->cursor.x);
        e_write_screen (window, 1);
        break;
    case DGZ:
        if (window->ins == 8)
            break;
        e_del_line (buffer->cursor.y, buffer, s);
        if (buffer->cursor.y > buffer->mxlines - 1)
            (buffer->cursor.y)--;
        e_write_screen (window, 1);
        break;
    case AF7:
    case AltV:
        if (window->dtmd != DTMD_HELP || window->ins != 8)
            return (c);
        e_help_next (window, 0);
        break;
    case AF8:
    case AltT:
        if (window->dtmd != DTMD_HELP || window->ins != 8)
            return (c);
        e_help_next (window, 1);
        break;
    default:
        return (c);
    }
    if (c >= 512)
    {
        if (s->ks.y == s->mark_begin.y && s->ks.x == s->mark_begin.x &&
                (s->mark_begin.y != s->mark_end.y
                 || s->mark_begin.x != s->mark_end.x))
        {
            s->mark_begin.x = buffer->cursor.x;
            s->mark_begin.y = buffer->cursor.y;
        }
        else if (s->ks.y == s->mark_end.y && s->ks.x == s->mark_end.x &&
                 (s->mark_begin.y != s->mark_end.y
                  || s->mark_begin.x != s->mark_end.x))
        {
            s->mark_end.x = buffer->cursor.x;
            s->mark_end.y = buffer->cursor.y;
        }
        else if (s->ks.y < buffer->cursor.y || (s->ks.y == buffer->cursor.y && s->ks.x < buffer->cursor.x))
        {
            s->mark_begin.x = s->ks.x;
            s->mark_begin.y = s->ks.y;
            s->mark_end.x = buffer->cursor.x;
            s->mark_end.y = buffer->cursor.y;
        }
        else
        {
            s->mark_end.x = s->ks.x;
            s->mark_end.y = s->ks.y;
            s->mark_begin.x = buffer->cursor.x;
            s->mark_begin.y = buffer->cursor.y;
        }
        e_write_screen (window, 1);
    }
    return (0);
}

/*   function key (F1-F12) evaluation, editor only */
int
e_tst_fkt (int c, we_control_t * e)
{
    extern OPT opt[];
    int i;
    we_window_t *window = e->window[e->mxedt];

#ifdef PROG
    if (e_tst_dfkt (window, c) == 0 || ((WpeIsProg ()) && e_prog_switch (window, c) == 0))
#else
    if (e_tst_dfkt (window, c) == 0)
#endif
    {
        window = e->window[e->mxedt];
        fk_cursor (1);
        if (e->mxedt > 0)
            e_cursor (window, 1);
        return (0);
    }

    for (i = 0; i < MENOPT; i++)
        if (c == opt[i].as)
            WpeHandleMainmenu (i, window);

    switch (c)
    {
    case CtrlK:
        e_ctrl_k (window);		/*  ctrl k  */
        break;
    case CtrlO:		/*  ctrl o  */
        e_ctrl_o (window);
        break;
    case AltG:
        e_goto_line (window);
        break;
    case CF10:
    case CtrlW:
        e_show_clipboard (window);
        break;
    case CtrlDel:
        e_blck_del (window);
        break;
    case UNDO:
    case AltBS:
    case CtrlU:
        e_make_undo (window);
        break;
    case AGAIN:
    case SABS:
    case CtrlR:
        e_make_redo (window);
        break;
    case CUT:
    case ShiftDel:		/*  shift Del  :  Delete to Clipboard  */
    /*               case 402:     */
    case CtrlX:
        e_edt_del (window);
        break;
    case PASTE:
    case ShiftEin:		/*  shift Einf  */
    case CtrlV:
        e_edt_einf (window);
        break;
#if !defined(NO_XWINDOWS)
    case AltEin:
        e_copy_X_buffer (window);
        break;
    case AltDel:
        e_paste_X_buffer (window);
        break;
#endif
    case COPY:
    case CtrlEin:		/*  ctrl Einf  */
    /*               case 401:    */
    case CtrlC:
        e_edt_copy (window);
        break;
    default:
        if (window->edit_control->edopt & ED_CUA_STYLE)
        {
            switch (c)
            {
            case AF2:
                e_m_save (window);
                break;
            case FID:
            case AF3:
                e_find (window);
                break;
            case SF3:
                e_replace (window);
                break;
            case F3:
                e_repeat_search (window);
                break;
            default:
                return (c);
            }
        }
        else
        {
            switch (c)
            {
            case F2:
                e_m_save (window);
                break;
            case FID:
            case F4:
                e_find (window);
                break;
            case AF4:
                e_replace (window);
                break;
            case CtrlL:
            case CF4:
                e_repeat_search (window);
                break;
            default:
                return (c);
            }
        }
        break;
    }
    fk_cursor (1);
    return (0);
}

int
e_ctrl_k (we_window_t * window)
{
    we_buffer_t *buffer = window->edit_control->window[window->edit_control->mxedt]->buffer;
    we_screen_t *screen = window->edit_control->window[window->edit_control->mxedt]->screen;
    int c;

    c = toupper (e_getch ());
    if (c < 32)
        c = c + 'A' - 1;
    switch (c)
    {
    case 'A':
        buffer->cursor = screen->mark_begin;
        e_write_screen (window, 1);
        break;
    case 'B':
        screen->mark_begin = e_set_pnt (buffer->cursor.x, buffer->cursor.y);
        e_write_screen (window, 1);
        break;
    case 'C':
        e_blck_copy (window);
        break;
    case 'D':
        e_changecase_dialog (window);
        break;
    case 'F':
        e_mk_beauty (1, 3, window);
        break;
    case 'H':
        e_blck_hide (window);
        break;
    case 'I':
        e_blck_to_right (window);
        break;
    case 'K':
        screen->mark_end = e_set_pnt (buffer->cursor.x, buffer->cursor.y);
        e_write_screen (window, 1);
        break;
    case 'L':
        window->screen->mark_begin.x = 0;
        window->screen->mark_begin.y = window->buffer->cursor.y;
        if (window->buffer->cursor.y < window->buffer->mxlines - 1)
        {
            window->screen->mark_end.x = 0;
            window->screen->mark_end.y = window->buffer->cursor.y + 1;
        }
        else
        {
            window->screen->mark_end.x = window->buffer->buflines[window->buffer->cursor.y].len;
            window->screen->mark_end.y = window->buffer->cursor.y;
        }
        e_write_screen (window, 1);
        break;
    case 'R':
        e_blck_read (window);
        break;
    case 'U':
        e_blck_to_left (window);
        break;
    case 'V':
        e_blck_move (window);
        break;
    case 'W':
        e_blck_write (window);
        break;
    case 'X':
        screen->mark_begin.x = 0;
        screen->mark_begin.y = 0;
        screen->mark_end.y = buffer->mxlines - 1;
        screen->mark_end.x = buffer->buflines[buffer->mxlines - 1].len;
        e_write_screen (window, 1);
        break;
    case 'Y':
        e_blck_del (window);
        break;
    case 'Z':
        buffer->cursor = screen->mark_end;
        e_write_screen (window, 1);
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        /* Set marker number n */
        screen->pt[c - '0'] = e_set_pnt (buffer->cursor.x, buffer->cursor.y);
        screen->fa.y = buffer->cursor.y;
        screen->fa.x = buffer->cursor.x;
        screen->fe.x = buffer->cursor.x + 1;
        e_write_screen (window, 1);
        break;
    }
    return 0;
}

/*   Ctrl - O - Dispatcher     */
int
e_ctrl_o (we_window_t * window)
{
    we_buffer_t *buffer = window->edit_control->window[window->edit_control->mxedt]->buffer;
    we_screen_t *s = window->edit_control->window[window->edit_control->mxedt]->screen;
    int i, c;
    unsigned char cc;

    c = toupper (e_getch ());
    if (c < 32)
        c = c + 'A' - 1;
    switch (c)
    {
    case 'Y':			/*  delete end of line    */
        if (window->ins == 8)
            break;
        e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, buffer->buflines[buffer->cursor.y].len - buffer->cursor.x);
        e_write_screen (window, 1);
        e_cursor (window, 1);
        break;
    case 'T':			/*  delete up to beginning of next word    */
        if (window->ins == 8)
            break;
        if (buffer->cursor.x <= 0 && buffer->cursor.y > 0)
        {
            buffer->cursor.y--;
            buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
        }
        else if (buffer->cursor.x > 0)
        {
            c = buffer->cursor.x;
            buffer->cursor.x = e_su_rblk (buffer->cursor.x - 1, buffer->buflines[buffer->cursor.y].s);
            e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, c - buffer->cursor.x);
        }
        e_write_screen (window, 1);
        break;
    case 'F':			/*  find string    */
        e_find (window);
        break;
    case 'A':			/*  replace string    */
        e_replace (window);
        break;
#ifdef PROG
    case 'S':			/*  find declaration    */
        e_sh_def (window);
        break;
    case 'N':			/*  ...next...  */
        e_sh_nxt_def (window);
        break;
    case 'K':			/*  next bracket  */
        e_nxt_brk (window);
        break;
    case 'B':			/*  beautify text  */
        e_p_beautify (window);
        break;
#endif
    case 'U':			/*   for help file: create button */
        if (s->mark_begin.y == s->mark_end.y && s->mark_begin.y >= 0
                && s->mark_begin.x < s->mark_end.x)
        {
            cc = HED;
            e_ins_nchar (buffer, s, &cc, s->mark_end.x, s->mark_end.y, 1);
            cc = HBG;
            e_ins_nchar (buffer, s, &cc, s->mark_begin.x, s->mark_end.y, 1);
            e_write_screen (window, 1);
        }
        break;
    case 'M':			/*   for help file: Mark-Line  */
        if (s->mark_begin.y == s->mark_end.y && s->mark_begin.y >= 0
                && s->mark_begin.x < s->mark_end.x)
        {
            cc = HED;
            e_ins_nchar (buffer, s, &cc, s->mark_end.x, s->mark_end.y, 1);
            cc = HBB;
            e_ins_nchar (buffer, s, &cc, s->mark_begin.x, s->mark_end.y, 1);
            e_write_screen (window, 1);
        }
        break;
    case 'H':			/*   for help file: create Header  */
        if (s->mark_begin.y == s->mark_end.y && s->mark_begin.y >= 0
                && s->mark_begin.x < s->mark_end.x)
        {
            cc = HED;
            e_ins_nchar (buffer, s, &cc, s->mark_end.x, s->mark_end.y, 1);
            cc = HHD;
            e_ins_nchar (buffer, s, &cc, s->mark_begin.x, s->mark_end.y, 1);
            e_write_screen (window, 1);
        }
        break;
    case 'E':			/*   for help file: create end mark  */
        e_new_line (buffer->cursor.y, buffer);
        cc = HFE;
        e_ins_nchar (buffer, s, &cc, 0, buffer->cursor.y, 1);
        e_write_screen (window, 1);
        break;
    case 'L':			/*   for help file: delete "help" special chars  */
        for (i = 0; i < buffer->buflines[buffer->cursor.y].len; i++)
            if (buffer->buflines[buffer->cursor.y].s[i] == HBG || buffer->buflines[buffer->cursor.y].s[i] == HED ||
                    buffer->buflines[buffer->cursor.y].s[i] == HHD || buffer->buflines[buffer->cursor.y].s[i] == HBB ||
                    buffer->buflines[buffer->cursor.y].s[i] == HFE || buffer->buflines[buffer->cursor.y].s[i] == HFB ||
                    buffer->buflines[buffer->cursor.y].s[i] == HFE)
            {
                e_del_nchar (buffer, s, i, buffer->cursor.y, 1);
                i--;
            }
        e_write_screen (window, 1);
        break;
    case 'C':			/*   for help file: check help file   */
        e_help_comp (window);
        break;
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
        /* Move to marker n (set previously with Ctrl-K n, obtained with Ctrl-O n */
        buffer->cursor.x = s->pt[c - '0'].x;
        buffer->cursor.y = s->pt[c - '0'].y;
        s->fa.y = buffer->cursor.y;
        s->fa.x = buffer->cursor.x;
        s->fe.x = buffer->cursor.x + 1;
        e_cursor (window, 1);
        break;
    }
    return (0);
}

/* general function key dispatcher
   basically, every time when it returns with zero
   something has happened */
int
e_tst_dfkt (we_window_t * window, int c)
{
    if (c >= Alt1 && c <= Alt9)
    {
        e_switch_window (c - Alt1 + 1, window);
        return (0);
    }
    if (c >= 1024 && c <= 1049)
    {
        e_switch_window (c - 1014, window);
        return (0);
    }
    switch (c)
    {
    case F1:
    case HELP:
        e_help_loc (window, 0);
        break;
#if defined(PROG)
    case CF1:
        if (WpeIsProg ())
            e_topic_search (window);
        break;
    case AF1:
        if (WpeIsProg ())
            e_funct_in (window);
        break;
#endif
    case WPE_ESC:
    case F10:
    case AltBl:
        WpeHandleMainmenu (-1, window);
        break;
    case Alt0:
        e_list_all_win (window);
        break;
    case AF5:
        e_deb_out (window);
        break;
    default:
        if (window->edit_control->edopt & ED_CUA_STYLE)
        {
            switch (c)
            {
            case CtrlL:
                e_size_move (window);
                break;
            case OPEN:
            case F2:
                WpeManager (window);
                break;
            case SF2:
                WpeManagerFirst (window);
                break;
            case CF4:
            case AltX:
                e_close_window (window);
                break;
            case FRONT:
            case AltZ:
            case SF4:
                e_ed_cascade (window);
                break;
            case SF5:
                e_ed_tile (window);
                break;
            case SF6:
                e_ed_zoom (window);
                break;
            case CF6:
            case AltN:
                e_ed_next (window);
                break;
            case STOP:
            case AF4:
                e_quit (window);
                break;
            default:
                return (c);
            }
        }
        else
        {
            switch (c)
            {
            case CF5:
            case AF2:
                e_size_move (window);
                break;
            case OPEN:
            case F3:
                WpeManager (window);
                break;
            case CF3:
                WpeManagerFirst (window);
                break;
            case AF3:
                e_close_window (window);
                break;
            case FRONT:
            case AltZ:
            case F5:
                e_ed_zoom (window);
                break;
            case F6:
            case AltN:
                e_ed_next (window);
                break;
            case STOP:
            case AltX:
                e_quit (window);
                break;
            default:
                return (c);
            }
        }
    }
    return (0);
}

int
e_chr_sp (int x, we_buffer_t * buffer, we_window_t * window)
{
    int i, j;

    for (i = j = 0; i + j < x && i < buffer->buflines[buffer->cursor.y].len; i++)
    {
        if (*(buffer->buflines[buffer->cursor.y].s + i) == WPE_TAB)
            j += (window->edit_control->tabn - ((j + i) % window->edit_control->tabn) - 1);
#ifdef UNIX
        else if (!WpeIsXwin ()
                 && ((unsigned char) *(buffer->buflines[buffer->cursor.y].s + i)) > 126)
        {
            j++;
            if (((unsigned char) *(buffer->buflines[buffer->cursor.y].s + i)) < 128 + ' ')
                j++;
        }
        else if (*(buffer->buflines[buffer->cursor.y].s + i) < ' ')
            j++;
        if (window->dtmd == DTMD_HELP)
        {
            if (buffer->buflines[buffer->cursor.y].s[i] == HBG
                    || buffer->buflines[buffer->cursor.y].s[i] == HFB
                    || buffer->buflines[buffer->cursor.y].s[i] == HED
                    || buffer->buflines[buffer->cursor.y].s[i] == HHD
                    || buffer->buflines[buffer->cursor.y].s[i] == HFE
                    || buffer->buflines[buffer->cursor.y].s[i] == HBB
                    || buffer->buflines[buffer->cursor.y].s[i] == HNF)
                j -= 2;
        }
#endif
    }
    return (i);
}

/****************************************/
int
FindFirstNospaceChar (we_buffer_t * buffer, int line)
/* returns char number in the line, -1 otherwise */
{
    int k;
    /* advance x while char[k] == space() */
    for (k = 0; k < buffer->buflines[line].len; k++)
    {
        if (!isspace (buffer->buflines[line].s[k]))
        {
            return k;
        }
    }
    return -1;
}

/****************************************/
int
GetXOfCharNum (we_buffer_t * buffer, int line, int char_num)
{
    int x, k;

    for (x = 0, k = 0; k < buffer->buflines[line].len; k++)
    {
        if (k == char_num)
        {
            return x;
        }
        x = ((buffer->buflines[line].s[k] == '\t') ?
             (x / buffer->control->tabn + 1) * buffer->control->tabn : x + 1);
    }
    /* return 0 to secure result interpretation */
    return 0;
}

/****************************************/
int
GetCharNumOfX (we_buffer_t * buffer, int line, int char_x)
{
    int x, k, next_x;

    for (x = 0, k = 0; k < buffer->buflines[line].len; k++)
    {
        next_x = ((buffer->buflines[line].s[k] == '\t') ?
                  (x / buffer->control->tabn + 1) * buffer->control->tabn : x + 1);
        if (next_x > char_x)
        {
            return x;
        }
        x = next_x;
    }
    /* return 0 to secure result interpretation */
    return 0;
}

/****************************************/

/* New version of auto-indent */
int
e_tab_a_ind (we_buffer_t * buffer, we_screen_t * s)
{
    int a_indent = buffer->control->autoindent;
    int line, x, k, char_to_ins;
    unsigned char *str;
    int do_auto_indent = 0;
    int first_nospace_k;

    /* If at the right of current pos there are only space chars, tab up to
       first nospace position (auto-indent pos) of previous lines.
       Tail spaces of current line are erased.
       If left current pos there are only spaces and even if there
       are nospaces at the right, tab up to auto-indent pos.
       Otherwise, insert TAB char.
     */

    line = buffer->cursor.y;		/* current line */

    first_nospace_k = FindFirstNospaceChar (buffer, line);
    if (first_nospace_k > -1)
    {
        /* there is a nospace char in the line */
        /* get its x */
        if (buffer->cursor.x > GetXOfCharNum (buffer, line, first_nospace_k))
        {
            /* nospace char before curr pos */

            /* are there nospace chars under and after cursor pos */
            do_auto_indent = 1;
            for (k = GetCharNumOfX (buffer, line, buffer->cursor.x); k < buffer->buflines[line].len; k++)
            {
                if (!isspace (buffer->buflines[line].s[k]))
                {
                    /* yes, there are */
                    do_auto_indent = 0;
                    break;
                }
            }
            if (do_auto_indent)
            {
                /* erase all tail spaces */
                e_del_nchar (buffer, s, buffer->cursor.x, line, buffer->buflines[line].len - buffer->cursor.x);
            }
        }
        else
        {
            /* nospace char only at or after curr pos */
            do_auto_indent = 1;
        }
    }
    else
    {
        /* there are only space chars in the line */
        do_auto_indent = 1;
        if (buffer->buflines[line].len > 0)
        {
            /* erase all tail spaces */
            e_del_nchar (buffer, s, buffer->cursor.x, line, buffer->buflines[line].len - buffer->cursor.x);
        }
    }

    if (!do_auto_indent)
    {
        /* insert TAB char */
        str = malloc (sizeof (unsigned char));
        str[0] = '\t';
        char_to_ins = 1;
    }
    else
    {
        /* auto-indent */

        /* find x of the first nospace char of lines from above */
        x = 0;
        for (line = buffer->cursor.y - 1; (line > 0); line--)
        {
            k = FindFirstNospaceChar (buffer, line);
            if (k > -1)
            {
                x = GetXOfCharNum (buffer, line, k);
                break;
            }
        }

        if (buffer->cursor.x < x)
        {
            /* indent to x with spaces */
            /* insert chars */
            k = x - buffer->cursor.x;
            str = malloc (k * sizeof (unsigned char));
            for (x = 0; x < k; x++)
                str[x] = ' ';
            char_to_ins = k;
        }
        else if (buffer->cursor.x < (x + a_indent))
        {
            /* indent to x + a_indent with spaces */
            /* insert chars */
            k = x + a_indent - buffer->cursor.x;
            str = malloc (k * sizeof (unsigned char));
            for (x = 0; x < k; x++)
                str[x] = ' ';
            char_to_ins = k;
        }
        else
        {
            /* insert TAB char */
            str = malloc (sizeof (unsigned char));
            str[0] = '\t';
            char_to_ins = 1;
        }
    }

    e_ins_nchar (buffer, s, str, buffer->cursor.x, buffer->cursor.y, char_to_ins);
    free (str);
    return (buffer->cursor.x);
}

int
e_del_a_ind (we_buffer_t * buffer, we_screen_t * s)
{
    int i = 1, j = -1, k;

    if (buffer->cursor.y > 0)
    {
        for (i = 0;
                i < buffer->buflines[buffer->cursor.y].len && i < buffer->cursor.x
                && isspace (buffer->buflines[buffer->cursor.y].s[i]); i++)
            ;
        if (i == buffer->cursor.x)
        {
            for (j = 0, i = 0; j <= buffer->cursor.x; j++)
            {
                i =
                    buffer->buflines[buffer->cursor.y].s[j] ==
                    '\t' ? (i / buffer->control->tabn + 1) * buffer->control->tabn : i + 1;
            }
            if (i != j)
            {
                unsigned char *str = malloc (i * sizeof (unsigned char));
                e_del_nchar (buffer, s, 0, buffer->cursor.y, j);
                for (j = 0; j < i; j++)
                    str[j] = ' ';
                e_ins_nchar (buffer, s, str, 0, buffer->cursor.y, i);
                buffer->cursor.x = i - 1;
                free (str);
            }
            for (j = buffer->cursor.y - 1; j >= 0; j--)
            {
                for (i = 0, k = 0; k < buffer->buflines[j].len && isspace (buffer->buflines[j].s[k]);
                        k++)
                    i =
                        buffer->buflines[j].s[k] ==
                        '\t' ? (i / buffer->control->tabn + 1) * buffer->control->tabn : i + 1;
                if (k < buffer->buflines[j].len && buffer->buflines[j].s[k] != '#' && i <= buffer->cursor.x)
                {
                    i = buffer->cursor.x - i + 1;
                    buffer->cursor.x -= i - 1;
                    break;
                }
            }
            if (j < 0)
            {
                i = buffer->cursor.x;
                buffer->cursor.x = 0;
            }
        }
    }
    if (j < 0)
        i = 1;
    e_del_nchar (buffer, s, buffer->cursor.x, buffer->cursor.y, i);
    return (i);
}

int
e_car_a_ind (we_buffer_t * buffer, we_screen_t * s)
{
    int i, j, k;
    unsigned char *str;

    if (buffer->cursor.y == 0)
        return (0);
    j = buffer->cursor.y;
    do
    {
        j--;
        for (i = 0, k = 0;
                k < buffer->buflines[j].len && (isspace (buffer->buflines[j].s[k])
                                                || buffer->buflines[j].s[i] == '{'); k++)
            i =
                buffer->buflines[j].s[k] == '\t' ? (i / buffer->control->tabn + 1) * buffer->control->tabn : i + 1;
    }
    while (j > 0 && buffer->buflines[j].s[k] == '#');
    if (k == buffer->buflines[j].len && k > 0 && buffer->buflines[j].s[k - 1] == '{')
        i--;
    if (i > 0)
    {
        str = malloc (i * sizeof (char));
        for (j = 0; j < i; j++)
            str[j] = ' ';
        e_ins_nchar (buffer, s, str, 0, buffer->cursor.y, i);
        buffer->cursor.x = i;
        free (str);
    }
    return (i);
}

/*   write n blanks */
int
e_blk (int anz, int xa, int ya, int col)
{
    for (anz--; anz >= 0; anz--)
        e_pr_char (xa + anz, ya, ' ', col);
    return (anz);
}

/*       insert Carriage Return     */
int
e_car_ret (we_buffer_t * buffer, we_screen_t * s)
{
    int len, i;
    len = buffer->buflines[buffer->cursor.y].len;
    e_add_undo ('a', buffer, buffer->cursor.x, buffer->cursor.y, 1);
    (buffer->window->save)++;
    if (buffer->cursor.x != len || *(buffer->buflines[buffer->cursor.y].s + len) != '\0')
    {
        e_new_line (buffer->cursor.y + 1, buffer);
        for (i = 0; i <= len - buffer->cursor.x; i++)
            *(buffer->buflines[buffer->cursor.y + 1].s + i) = *(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x + i);
        *(buffer->buflines[buffer->cursor.y + 1].s + i) = '\0';
        buffer->buflines[buffer->cursor.y + 1].len = e_str_len (buffer->buflines[buffer->cursor.y + 1].s);
        buffer->buflines[buffer->cursor.y + 1].nrc = strlen ((const char *) buffer->buflines[buffer->cursor.y + 1].s);
        if (s->mark_begin.y > buffer->cursor.y)
            (s->mark_begin.y)++;
        else if (s->mark_begin.y == buffer->cursor.y && s->mark_begin.x > buffer->cursor.x)
        {
            (s->mark_begin.y)++;
            (s->mark_begin.x) -= (buffer->cursor.x);
        }
        if (s->mark_end.y > buffer->cursor.y)
            (s->mark_end.y)++;
        else if (s->mark_end.y == buffer->cursor.y && s->mark_end.x > buffer->cursor.x)
        {
            (s->mark_end.y)++;
            (s->mark_end.x) -= (buffer->cursor.x);
        }
    }
    *(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) = WPE_WR;
    *(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x + 1) = '\0';
    buffer->buflines[buffer->cursor.y].len = e_str_len (buffer->buflines[buffer->cursor.y].s);
    buffer->buflines[buffer->cursor.y].nrc = strlen ((const char *) buffer->buflines[buffer->cursor.y].s);
    if(buffer->window->c_sw) e_sc_nw_txt(buffer->cursor.y, buffer, 1);
    /***************************/
    if (buffer->cursor.x > 0)
        e_brk_recalc (buffer->window, buffer->cursor.y + 1, 1);
    else
        e_brk_recalc (buffer->window, buffer->cursor.y, 1);
    /***************************/
    if (buffer->cursor.y < buffer->mxlines - 1)
    {
        (buffer->cursor.y)++;
        buffer->cursor.x = 0;
    }
    if (buffer->window->flg & 1)
        e_car_a_ind (buffer, s);
    return (buffer->cursor.y);
}

/*   cursor placement */
void
e_cursor (we_window_t * window, int sw)
{
    we_buffer_t *buffer = window->buffer;
    we_screen_t *s = window->screen;
    static int iold = 0, jold = 0;
    int i, j;

    if (!DTMD_ISTEXT (window->dtmd))
        return;
    if (buffer->cursor.y > buffer->mxlines - 1)
        buffer->cursor.y = buffer->mxlines - 1;
    if (buffer->cursor.y < 0)
        buffer->cursor.y = 0;
    if (buffer->cursor.x < 0)
        buffer->cursor.x = 0;
    if (buffer->mxlines == 0)
        buffer->cursor.x = 0;			/* the else branch needs buffer->cursor.y < buffer->mxlines, which is not true for buffer->mxlines==0 */
    else if (buffer->cursor.x > buffer->buflines[buffer->cursor.y].len)
        buffer->cursor.x = buffer->buflines[buffer->cursor.y].len;
    for (i = j = 0; i < buffer->cursor.x; i++)
    {
        if (*(buffer->buflines[buffer->cursor.y].s + i) == WPE_TAB)
            j += (window->edit_control->tabn - ((j + i) % window->edit_control->tabn) - 1);
        else if (!WpeIsXwin ()
                 && ((unsigned char) *(buffer->buflines[buffer->cursor.y].s + i)) > 126)
        {
            j++;
            if (((unsigned char) *(buffer->buflines[buffer->cursor.y].s + i)) < 128 + ' ')
                j++;
        }
        else if (*(buffer->buflines[buffer->cursor.y].s + i) < ' ')
            j++;
        if (window->dtmd == DTMD_HELP)
        {
            if (buffer->buflines[buffer->cursor.y].s[i] == HBG || buffer->buflines[buffer->cursor.y].s[i] == HED ||
                    buffer->buflines[buffer->cursor.y].s[i] == HHD || buffer->buflines[buffer->cursor.y].s[i] == HFE ||
                    buffer->buflines[buffer->cursor.y].s[i] == HFB || buffer->buflines[buffer->cursor.y].s[i] == HBB ||
                    buffer->buflines[buffer->cursor.y].s[i] == HNF)
                j -= 2;
        }
    }
    if (buffer->cursor.y - s->c.y < 0 || buffer->cursor.y - s->c.y >= num_lines_on_screen(window) - 1 ||
            s->c.y < 0 || s->c.y >= buffer->mxlines ||
            buffer->cursor.x + j - s->c.x < 0 ||
            buffer->cursor.x + j - s->c.x >= num_cols_on_screen(window) - 1)
    {
#if defined(UNIX)
        /*if(buffer->cursor.y - s->c.y < 0) s->c.y = buffer->cursor.y - (window->e.y - window->a.y)/2;
           else if(buffer->cursor.y - s->c.y >= window->e.y - window->a.y -1)
           s->c.y = buffer->cursor.y - (window->e.y - window->a.y)/2; */
        if (buffer->cursor.y - s->c.y < -1)
        {
            s->c.y = buffer->cursor.y - (num_lines_on_screen(window)) / 2;
        }
        else if (buffer->cursor.y - s->c.y == -1)
        {
            s->c.y -= 1;
        }
        else if (buffer->cursor.y - s->c.y > num_lines_on_screen(window) - 1)
        {
            s->c.y = buffer->cursor.y - (num_lines_on_screen(window)) / 2;
        }
        else if (buffer->cursor.y - s->c.y == num_lines_on_screen(window) - 1)
        {
            s->c.y += 1;
        }
#else
        if (buffer->cursor.y - s->c.y < 0)
            s->c.y = buffer->cursor.y;
        else if (buffer->cursor.y - s->c.y >= num_lines_on_screen(window) - 1)
            (s->c.y) = buffer->cursor.y - window->e.y + window->a.y + 2;
#endif
        if (s->c.y >= buffer->mxlines - 1)
            s->c.y = buffer->mxlines - 2;
        if (s->c.y < 0)
            s->c.y = 0;

        if (buffer->cursor.x + j - s->c.x < 0)
            (s->c.x) = buffer->cursor.x + j - (num_cols_on_screen(window)) / 2;
        else if (buffer->cursor.x + j - s->c.x >= num_cols_on_screen(window) - 1)
            (s->c.x) = buffer->cursor.x + j - (num_cols_on_screen(window)) / 2;
        if (s->c.x < 0)
            s->c.x = 0;
        else if (s->c.x >= buffer->buflines[buffer->cursor.y].len + j)
            s->c.x = buffer->buflines[buffer->cursor.y].len + j;
        e_write_screen (window, sw);
    }
    if (s->fa.y == -1)
    {
        e_write_screen (window, sw);
        s->fa.y--;
    }
    else if (s->fa.y > -1)
        s->fa.y = -1;
    if (sw != 0)
    {
        iold = e_lst_zeichen (window->e.x, window->a.y + 1, window->e.y - window->a.y - 1, 0,
                              window->colorset->em.fg_bg_color, buffer->mxlines, iold, buffer->cursor.y);
        jold = e_lst_zeichen (window->a.x + 19, window->e.y, window->e.x - window->a.x - 20, 1,
                              window->colorset->em.fg_bg_color, buffer->mx.x, jold, buffer->cursor.x);
    }
    buffer->cl = buffer->cursor.x + j;
    fk_locate (window->a.x + buffer->cursor.x - s->c.x + j + 1, window->a.y + buffer->cursor.y - s->c.y + 1);
}

/*   delete one line */
int
e_del_line (int yd, we_buffer_t * buffer, we_screen_t * s)
{
    int i;

    if (buffer->mxlines == 1)
    {
        *(buffer->buflines[0].s) = '\0';
        buffer->buflines[0].nrc = buffer->buflines[0].len = 0;
    }
    else
    {
        e_add_undo ('l', buffer, 0, yd, 1);
        (buffer->mxlines)--;
        for (i = yd; i < buffer->mxlines; i++)
            buffer->buflines[i] = buffer->buflines[i + 1];
        if (s->mark_begin.y > yd)
            (s->mark_begin.y)--;
        else if (s->mark_begin.y == yd)
            (s->mark_begin.x) = 0;
        if (s->mark_end.y > yd)
            (s->mark_end.y)--;
        else if (s->mark_end.y == yd)
        {
            (s->mark_end.y)--;
            s->mark_end.x = buffer->buflines[yd - 1].len;
        }
    }
    if(buffer->window->c_sw) e_sc_nw_txt(yd, buffer, -1);
    if (buffer->window)
        (buffer->window->save) += 10;
    /*******************************/
    e_brk_recalc (buffer->window, yd, -1);
    /*******************************/
    return (buffer->mxlines);
}

/*   delete N chars from buffer */
int
e_del_nchar (we_buffer_t * buffer, we_screen_t * s, int x, int y, int n)
{
    we_window_t *window = global_editor_control->window[global_editor_control->mxedt];
    int len, i, j;

    window->save += n;
    e_add_undo ('r', buffer, x, y, n);
    global_disable_add_undo++;
    len = buffer->buflines[y].len;
    if (*(buffer->buflines[y].s + len) == WPE_WR)
        len++;
    for (j = x; j <= len - n; ++j)
        *(buffer->buflines[y].s + j) = *(buffer->buflines[y].s + j + n);
    if (s->mark_begin.y == y && s->mark_begin.x > x)
        s->mark_begin.x = (s->mark_begin.x - n > x) ? s->mark_begin.x - n : x;
    if (s->mark_end.y == y && s->mark_end.x > x)
        s->mark_end.x = (s->mark_end.x - n > x) ? s->mark_end.x - n : x;

    if (len <= n)
        e_del_line (y, buffer, s);
    else if (y < buffer->mxlines - 1 && *(buffer->buflines[y].s + len - n - 1) != WPE_WR)
    {
        if (buffer->mx.x + n - len > buffer->buflines[y + 1].len)
            i = buffer->buflines[y + 1].len;
        else
            i = e_su_rblk (buffer->mx.x + n - len, buffer->buflines[y + 1].s);
        if (buffer->buflines[y + 1].s[i] == WPE_WR)
            i++;
        if (i > 0)
        {
            for (j = 0; j < i; ++j)
                *(buffer->buflines[y].s + len - n + j) = *(buffer->buflines[y + 1].s + j);
            *(buffer->buflines[y].s + len - n + i) = '\0';
            if (s->mark_begin.y == y + 1 && s->mark_begin.x <= i)
            {
                s->mark_begin.y--;
                s->mark_begin.x += (len - n);
            }
            if (s->mark_end.y == y + 1 && s->mark_end.x <= i)
            {
                s->mark_end.y--;
                s->mark_end.x += (len - n);
            }
            e_del_nchar (buffer, s, 0, y + 1, i);
        }
    }
    if (y < buffer->mxlines)
    {
        buffer->buflines[y].len = e_str_len (buffer->buflines[y].s);
        buffer->buflines[y].nrc = strlen ((const char *) buffer->buflines[y].s);
    }
    global_disable_add_undo--;
    if(buffer->window->c_sw && !global_disable_add_undo)
        e_sc_nw_txt(y, buffer, 0);
    return (x + n);
}

/*   insert N chars in buffer */
int
e_ins_nchar (we_buffer_t * buffer, we_screen_t * sch, unsigned char *s, int xa, int ya,
             int n)
{
    we_window_t *window = global_editor_control->window[global_editor_control->mxedt];
    int i, j;

    window->save += n;
    e_add_undo ('a', buffer, xa, ya, n);
    global_disable_add_undo++;
    if (buffer->buflines[ya].len + n >= buffer->mx.x - 1)
    {
        if (xa < buffer->buflines[ya].len)
        {
            i = buffer->mx.x - n - 1;
            if (i >= buffer->buflines[ya].len - 1)
                i = buffer->buflines[ya].len - 1;
        }
        else
        {
            for (; xa < buffer->mx.x - 1; xa++, s++, n--)
            {
                *(buffer->buflines[ya].s + xa + 1) = *(buffer->buflines[ya].s + xa);
                *(buffer->buflines[ya].s + xa) = *s;
            }
            *(buffer->buflines[ya].s + xa + 1) = '\0';
            i = buffer->mx.x;
            buffer->buflines[ya].len = e_str_len (buffer->buflines[ya].s);
            buffer->buflines[ya].nrc = strlen ((const char *) buffer->buflines[ya].s);
        }
        for (; i > 0 && *(buffer->buflines[ya].s + i) != ' ' && *(buffer->buflines[ya].s + i) != '-';
                i--);
        if (i == 0)
        {
            if (s[n - 1] != ' ' && s[n - 1] != '-')
                i = buffer->buflines[ya].len - 2;
            else
                i--;
        }
        if (*(buffer->buflines[ya].s + buffer->buflines[ya].len) == WPE_WR || ya == buffer->mxlines)
        {
            e_new_line (ya + 1, buffer);
            if (sch->mark_begin.y > ya)
                (sch->mark_begin.y)++;
            else if (sch->mark_begin.y == ya)
            {
                if (sch->mark_begin.x > i)
                {
                    (sch->mark_begin.y)++;
                    (sch->mark_begin.x) -= (i + 1);
                }
                else if (sch->mark_begin.x >= xa)
                    sch->mark_begin.x += n;
            }
            if (sch->mark_end.y > ya)
                (sch->mark_end.y)++;
            else if (sch->mark_end.y == ya)
            {
                if (sch->mark_end.x > i)
                {
                    (sch->mark_end.y)++;
                    (sch->mark_end.x) -= (i + 1);
                }
                else if (sch->mark_end.x >= xa)
                    sch->mark_end.x += n;
            }
            for (j = i + 1;
                    *(buffer->buflines[ya].s + j) != WPE_WR && *(buffer->buflines[ya].s + j) != '\0';
                    j++)
                *(buffer->buflines[ya + 1].s + j - i - 1) = *(buffer->buflines[ya].s + j);
            *(buffer->buflines[ya + 1].s + j - i - 1) = WPE_WR;
            buffer->buflines[ya + 1].len = e_str_len (buffer->buflines[ya + 1].s);
            buffer->buflines[ya + 1].nrc = strlen ((const char *) buffer->buflines[ya + 1].s);
            if(buffer->window->c_sw && !global_disable_add_undo)
                e_sc_nw_txt(ya, buffer, 1);
        }
        else
        {
            e_ins_nchar (buffer, sch, buffer->buflines[ya].s + i + 1, 0, ya + 1,
                         buffer->buflines[ya].len - i - 1);
            if (sch->mark_begin.y == ya)
            {
                if (sch->mark_begin.x > i)
                {
                    (sch->mark_begin.y)++;
                    (sch->mark_begin.x) -= (i + 1);
                }
                else if (sch->mark_begin.x >= xa)
                    sch->mark_begin.x += n;
            }
            if (sch->mark_end.y == ya)
            {
                if (sch->mark_end.x > i)
                {
                    (sch->mark_end.y)++;
                    (sch->mark_end.x) -= (i + 1);
                }
                else if (sch->mark_end.x >= xa)
                    sch->mark_end.x += n;
            }
        }
        /*	 if(*(buffer->buflines[ya].s+i) == ' ') *(buffer->buflines[ya].s+i) = '\0';
                 else
        */
        *(buffer->buflines[ya].s + i + 1) = '\0';
        buffer->buflines[ya].len = e_str_len (buffer->buflines[ya].s);
        buffer->buflines[ya].nrc = strlen ((const char *) buffer->buflines[ya].s);
        if (xa > buffer->buflines[ya].len)
        {
            xa -= (buffer->buflines[ya].len);
            ya++;
            buffer->buflines[ya].len = e_str_len (buffer->buflines[ya].s);
            buffer->buflines[ya].nrc = strlen ((const char *) buffer->buflines[ya].s);
            if (sch->mark_begin.y == ya && sch->mark_begin.x >= xa)
                sch->mark_begin.x += n;
            if (sch->mark_end.y == ya && sch->mark_end.x >= xa)
                sch->mark_end.x += n;
        }
    }
    else
    {
        if (sch->mark_begin.y == ya && sch->mark_begin.x >= xa)
            sch->mark_begin.x += n;
        if (sch->mark_end.y == ya && sch->mark_end.x >= xa)
            sch->mark_end.x += n;
    }
    for (j = buffer->buflines[ya].len; j >= xa; --j)
        *(buffer->buflines[ya].s + j + n) = *(buffer->buflines[ya].s + j);
    for (j = 0; j < n; ++j)
        *(buffer->buflines[ya].s + xa + j) = *(s + j);
    if (buffer->buflines[ya].s[buffer->buflines[ya].len] == WPE_WR)
        buffer->buflines[ya].s[buffer->buflines[ya].len + 1] = '\0';
    buffer->cursor.x = xa + n;
    buffer->cursor.y = ya;
    buffer->buflines[ya].len = e_str_len (buffer->buflines[ya].s);
    buffer->buflines[ya].nrc = strlen ((const char *) buffer->buflines[ya].s);
    global_disable_add_undo--;
    if(buffer->window->c_sw && !global_disable_add_undo)
        e_sc_nw_txt(ya, buffer, 0);
    return (xa + n);
}

/*   insert new line */
int
e_new_line (int yd, we_buffer_t * buffer)
{
    int i;

    if (buffer->mxlines > buffer->mx.y - 2)
    {
        buffer->mx.y += MAXLINES;
        if ((buffer->buflines = realloc (buffer->buflines, buffer->mx.y * sizeof (STRING))) == NULL)
            e_error (e_msg[ERR_LOWMEM], 1, buffer->colorset);
        if (buffer->window->c_sw)
            buffer->window->c_sw = realloc (buffer->window->c_sw, buffer->mx.y * sizeof (int));
    }
    for (i = buffer->mxlines - 1; i >= yd; i--)
    {
        buffer->buflines[i + 1] = buffer->buflines[i];
    }
    (buffer->mxlines)++;
    buffer->buflines[yd].s = malloc (buffer->mx.x + 1);
    if (buffer->buflines[yd].s == NULL)
        e_error (e_msg[ERR_LOWMEM], 1, buffer->colorset);
    *(buffer->buflines[yd].s) = '\0';
    buffer->buflines[yd].len = 0;
    buffer->buflines[yd].nrc = 0;
    return (buffer->mxlines);
}

/*     Overwriting of a character       */
int
e_put_char (int c, we_buffer_t * buffer, we_screen_t * s)
{
    unsigned char cc = c;

    if (buffer->cursor.x == buffer->buflines[buffer->cursor.y].len)
        e_ins_nchar (buffer, s, &cc, buffer->cursor.x, buffer->cursor.y, 1);
    else
    {
        e_add_undo ('p', buffer, buffer->cursor.x, buffer->cursor.y, 1);
        (buffer->window->save)++;
        *(buffer->buflines[buffer->cursor.y].s + buffer->cursor.x) = c;
    }
    (buffer->cursor.x)++;
    return (buffer->cursor.x);
}

/*   search right (left end of word) */
/** FIXME: is unsigned char * really necessary? Do we expect value > 127? */
int
e_su_lblk (int xa, unsigned char *s)
{
    int len = strlen ((const char *) s);

    if (xa >= len)
        xa = len - 1;
    for (; xa < len && isalnum1 (s[xa]); xa++)
        ;
    for (; xa < len && !isalnum1 (s[xa]); xa++)
        ;
    return (xa);
}

/*     Search left (left end of word)     */
int
e_su_rblk (int xa, unsigned char *s)
{
    int len = strlen ((const char *) s);

    if (xa <= 0)
        return (xa);
    if (xa > len)
        xa = len;
    for (xa--; xa > 0 && !isalnum1 (s[xa]); xa--)
        ;
    if (!xa)
        return (xa);
    for (; xa > 0 && isalnum1 (s[xa]); xa--)
        ;
    return (!xa ? xa : xa + 1);
}

/* Prints out the line number and column of cursor */
void
e_zlsplt (we_window_t * window)
{
    char str[20];

    if (!DTMD_ISTEXT (window->dtmd))
        return;
    sprintf (str, "%5d:%-4d", window->buffer->cursor.y + 1, window->buffer->cl + 1);
    e_puts (str, window->a.x + 5, window->e.y, window->colorset->er.fg_bg_color);
    if (window->save)
        e_pr_char (window->a.x + 3, window->e.y, '*', window->colorset->er.fg_bg_color);
    else
        e_pr_char (window->a.x + 3, window->e.y, ' ', window->colorset->er.fg_bg_color);
#ifdef NEWSTYLE
    if (WpeIsXwin ())
    {
        e_make_xrect (window->a.x + 1, window->e.y, window->e.x - 1, window->e.y, 0);
        e_make_xrect (window->a.x + 5, window->e.y, window->a.x + 14, window->e.y, 0);
    }
#endif
}

void
WpeFilenameToPathFile (char *filename, char **path, char **file)
{
    char *tmp;
    char *cur_dir;
    int len;

    *path = NULL;
    tmp = strrchr (filename, DIRC);
    if (tmp)
    {
        *file = WpeStrdup (tmp + 1);
    }
    else
    {
        *file = WpeStrdup (filename);
    }
    if ((!tmp) || ((filename + 1 == tmp) && (*filename == '.')))
    {
        *path = WpeGetCurrentDir (global_editor_control);
    }
    else
    {
        if (*filename != DIRC)
        {
            if ((cur_dir = WpeGetCurrentDir (global_editor_control)) == NULL)
            {
                free (*file);
                *file = NULL;
                return;
            }
            /* Delete the slash at the end. */
            len = strlen (cur_dir);
            cur_dir[len - 1] = 0;
            len = tmp - filename + 1;
            while (strncmp (filename, "../", 3) == 0)
            {
                tmp = strrchr (cur_dir, DIRC);
                if (tmp == cur_dir)
                {
                    cur_dir[1] = 0;
                }
                else
                {
                    *tmp = 0;
                }
                len -= 3;
                filename += 3;
            }
            *path = malloc ((strlen (cur_dir) + len + 2) * sizeof (char));
            strcpy (*path, cur_dir);
            strcat (*path, DIRS);
            strncat (*path, filename, len);
            if ((*path)[strlen (cur_dir) + len] != DIRC)
            {
                (*path)[strlen (cur_dir) + len + 1] = 0;
            }
            else
            {
                (*path)[strlen (cur_dir) + len] = DIRC;
                (*path)[strlen (cur_dir) + len + 1] = 0;
            }
            free (cur_dir);
        }
        else
        {
            len = tmp - filename + 1;
            *path = malloc (len + 1 * sizeof (char));
            strncpy (*path, filename, len);
            (*path)[len] = 0;
        }
    }
}

/*   draw mouse slider cursor */
int
e_lst_zeichen (int x, int y, int n, int sw, int frb, int max, int iold,
               int new)
{
    int inew;
    double d = max ? 1. / (float) max : 0;

    if (n < 3)
        return (1);

    if ((inew = (int) (new * (n - 2) * d + 1.5)) > n - 2)
        inew = n - 2;
    if (iold < 1)
        iold = 1;
    if (inew < 1)
        inew = 1;

    if (sw == 0)
    {
        if (iold < n - 1)
            e_pr_char (x, y + iold, MCI, frb);
        e_pr_char (x, y + inew, MCA, frb);
        e_make_xrect (x, y + 1, x, y + n - 2, 0);
        e_make_xrect (x, y + inew, x, y + inew, 0);
    }
    else
    {
        if (iold < n - 1)
            e_pr_char (x + iold, y, MCI, frb);
        e_pr_char (x + inew, y, MCA, frb);
        e_make_xrect (x + 1, y, x + n - 2, y, 0);
        e_make_xrect (x + inew, y, x + inew, y, 0);
    }
    return (inew);
}

/*   draw mouse slider bar */
void
e_mouse_bar (int x, int y, int n, int sw, int frb)
{
    int sv = n;

    if (sw == 0)
    {
        e_pr_char (x, y, MCU, frb);
        e_pr_char (x, y + n - 1, MCD, frb);
        for (; n > 2; n--)
            e_pr_char (x, y + n - 2, MCI, frb);
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
        e_make_xrect (x, y + 1, x, y + sv - 2, 0);
        e_make_xrect (x, y, x, y, 0);
        e_make_xrect (x, y + sv - 1, x, y + sv - 1, 0);
#else
        UNUSED(sv);
#endif
    }
    else
    {
        e_pr_char (x, y, MCL, frb);
        e_pr_char (x + n - 1, y, MCR, frb);
        for (; n > 2; n--)
            e_pr_char (x + n - 2, y, MCI, frb);
        e_make_xrect (x + 1, y, x + sv - 2, y, 0);
        e_make_xrect (x, y, x, y, 0);
        e_make_xrect (x + sv - 1, y, x + sv - 1, y, 0);
    }
}

int
e_autosave (we_window_t * window)
{
    char *tmp, *str;
    unsigned long maxname;

    window->save = 1;
    if (!(window->edit_control->autosv & 2))
        return (0);
    /* Check if file system could have an autosave or emergency save file
       >12 check is to eliminate dos file systems */
    if (((maxname = pathconf (window->dirct, _PC_NAME_MAX)) >= strlen (window->datnam) + 4)
            && (maxname > 12))
    {
        str = malloc (strlen (window->datnam) + 5);
        str = e_make_postf (str, window->datnam, ".ASV");
        tmp = window->datnam;
        window->datnam = str;
        /*e_save(window); */
        WpeMouseChangeShape (WpeWorkingShape);
        e_write (0, 0, window->buffer->mx.x, window->buffer->mxlines - 1, window, WPE_NOBACKUP);
        WpeMouseRestoreShape ();
        window->datnam = tmp;
        window->save = 1;
        free (str);
    }
    return (0);
}

we_undo_t *
e_remove_undo (we_undo_t * undo, int sw)
{
    if (undo == NULL)
        return (undo);
    undo->next = e_remove_undo (undo->next, sw + 1);
    if (sw > global_editor_control->numundo)
    {
        if (undo->type == 'l')
            free (undo->u.pt);
        else if (undo->type == 'd')
        {
            we_buffer_t *buffer = (we_buffer_t *) undo->u.pt;
            int i;

            free (buffer->window->screen);
            free (buffer->window);
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
        free (undo);
        undo = NULL;
    }
    return (undo);
}

/**
 * \brief Function to add undo or redo information to a queue.
 *
 * Function to add undo information to the list of things to undo
 * or to the list of things to redo.
 * What the function does depends on the value of the integer undo_type.
 * Remark that the options file (if it exists) has a maximum number of undo's and redo's.
 * The default is a maximum of 10 that you can change using the Options/Editor menu-option.
 *
 * @param undo_type int the type of the undo action see table below.
 *
 * type| action
 * ----|----------------------------------
 *  d  | Uses d to remember delete characters in a block
 *  c  | Uses c to remember a block copy
 *  v  | Uses v to paste block
 *  a  | Uses a to add characters
 *  l  | Uses l to delete line
 *  r  | Uses r to remember deleted characters on one line
 *  p  | Uses p to put char over another char (replace)
 *  y  | Uses y to redo a previous undo of l
 *  s  | Uses s to replace a string of characters (verified with test)
 *
 *  Remark: the global_disable_add_undo is a disabler for this function.
 *  if global_disable_add_undo is true, this function does nothing.
 *
 *  @param buffer we_buffer_t the buffer where the undo and search/replace string is stored
 *  @param x int the x coordinate (column)
 *  @param y int the y coordinate (row)
 *  @param n the length of the undo/redo concerned
 *  @return int 0 if the undo was constructed correctly or the undo was disabled,
 *				-1 if an error occurred. The error is accompanied by an error message on stdout.
 */
int
e_add_undo (int undo_type, we_buffer_t * buffer, int x, int y, int n)
{
    int result = 0;

    int (*process_undo)(we_buffer_t * buffer, we_undo_t *next);

    process_undo =
        undo_type == 'a' ? e_process_undo_a_char_delete :
        undo_type == 'p' ? e_process_undo_p_char_put :
        undo_type == 'r' ? e_process_undo_r_search_replace :
        undo_type == 's' ? e_process_undo_s_search_replace :
        undo_type == 'l' ? e_process_undo_l_line_delete :
        undo_type == 'c' ? e_process_undo_c_copy_paste :
        undo_type == 'v' ? e_process_undo_v_copy_paste :
        undo_type == 'd' ? e_process_undo_d_block_delete : e_process_undo_default;

    if (e_undo_is_active())
    {
        e_prepare_buffer_for_undo(buffer);
        we_undo_t *next = e_create_undo(undo_type, buffer, x, y, n);
        if (next == NULL)
            return -1;
        result = process_undo(buffer, next);
        e_add_new_undo(buffer, next);
    }
    return result;
}

_Bool
e_undo_is_active()
{
    return global_disable_add_undo == 0;
}

void
e_prepare_buffer_for_undo(we_buffer_t *buffer)
{
    if (e_phase == EDIT_PHASE && buffer->redo)
        buffer->redo = e_remove_undo (buffer->redo, global_editor_control->numundo + 1);
    return;
}

we_undo_t *
e_create_undo(int undo_type, we_buffer_t *buffer, int x, int y, int n)
{
    we_undo_t *undo = NULL;
    undo = malloc(sizeof(we_undo_t));
    if (undo == NULL)
    {
        e_error (e_msg[ERR_LOWMEM], 0, buffer->colorset);
        return undo;
    }
    undo->type = undo_type;
    undo->cursor_start.x = x;
    undo->cursor_start.y = y;
    undo->begin_block.x = n;
    undo->next = e_phase == UNDO_PHASE ? buffer->redo : buffer->undo;

    return undo;
}

void
e_add_new_undo(we_buffer_t *buffer, we_undo_t *next)
{
    if (e_phase == UNDO_PHASE)
        buffer->redo = next;
    else
    {
        next->next = e_remove_undo (buffer->undo, 1);
        buffer->undo = next;
    }
    return;
}

int
e_process_undo_default(we_buffer_t *buffer, we_undo_t *next)
{
    UNUSED(buffer);
    UNUSED(next);

    return -1;
}

int
e_process_undo_a_char_delete (we_buffer_t * buffer, we_undo_t *next)
{
    UNUSED(buffer);
    UNUSED(next);

    return 0;
}

int
e_process_undo_p_char_put (we_buffer_t * buffer, we_undo_t *next)
{
    int x = next->cursor_start.x;
    int y = next->cursor_start.y;
    next->u.c = buffer->buflines[y].s[x];

    return 0;
}


int
e_process_undo_r_search_replace (we_buffer_t * buffer, we_undo_t *next)
{
    return e_process_undo_s_search_replace(buffer, next);
}

int
e_process_undo_s_search_replace (we_buffer_t * buffer, we_undo_t *next)
{
    int n = next->begin_block.x;
    char *str = malloc (n+1);
    int i;

    if (str == NULL)
    {
        e_error (e_msg[ERR_LOWMEM], 0, buffer->colorset);
        free (next);
        return (-1);
    }
    int x = next->cursor_start.x;
    int y = next->cursor_start.y;
    for (i = 0; i < n; i++)
        str[i] = buffer->buflines[y].s[x + i];
    str[n] = '\0';
    next->u.pt = str;

    next->begin_block.y =
        e_phase == UNDO_PHASE ? buffer->control->find.sn : buffer->control->find.rn;

    return (0);
}

int
e_process_undo_l_line_delete (we_buffer_t * buffer, we_undo_t *next)
{
    int y = next->cursor_start.y;
    next->u.pt = buffer->buflines[y].s;

    return (0);
}

int
e_process_undo_c_copy_paste (we_buffer_t * buffer, we_undo_t *next)
{
    we_screen_t *screen = buffer->control->window[buffer->control->mxedt]->screen;

    next->begin_block = screen->mark_begin;
    next->end_block = screen->mark_end;

    return (0);
}

int
e_process_undo_v_copy_paste (we_buffer_t * buffer, we_undo_t *next)
{
    return e_process_undo_c_copy_paste(buffer, next);
}

int
e_process_undo_d_block_delete (we_buffer_t * buffer, we_undo_t *next)
{

    we_buffer_t *bn = malloc (sizeof (we_buffer_t));
    we_screen_t *sn = malloc (sizeof (we_screen_t));
    we_window_t *fn = malloc (sizeof (we_window_t));
    we_window_t *window = buffer->control->window[buffer->control->mxedt];

    bn->buflines = (STRING *) malloc (MAXLINES * sizeof (STRING));
    if (bn == NULL || sn == 0 || bn->buflines == NULL)
        return (e_error (e_msg[ERR_LOWMEM], 0, buffer->colorset));
    fn->buffer = bn;
    fn->c_sw = NULL;
    fn->c_st = NULL;
    fn->edit_control = NULL;	/* Quick fix so that breakpoints aren't recalculated */
    fn->find.dirct = NULL;
    bn->window = fn;
    bn->window->screen = sn;
    bn->cursor = e_set_pnt (0, 0);
    bn->mx = e_set_pnt (buffer->control->maxcol, MAXLINES);
    bn->mxlines = 0;
    sn->colorset = bn->colorset = buffer->colorset;
    bn->control = buffer->control;
    bn->undo = NULL;
    bn->redo = NULL;
    sn->c = sn->ks
            = sn->mark_begin
              = sn->mark_end
                = sn->fa
                  = sn->fe = e_set_pnt (0, 0);
    e_new_line (0, bn);
    *(bn->buflines[0].s) = WPE_WR;
    *(bn->buflines[0].s + 1) = '\0';
    bn->buflines[0].len = 0;
    bn->buflines[0].nrc = 1;
    next->u.pt = bn;

    // disable undo during the move block
    global_disable_add_undo = 1;
    e_move_block (0, 0, buffer, bn, window);
    global_disable_add_undo = 0;

    return (0);
}

/**
 * \brief executes an undo from the undo queue in the buffer
 *
 * @param window we_window_t the editting window containing the text where a change
 *               is to be undone.
 * @return int 0 if no error was encountered, -1 in case of an error,
 *
 */
int
e_make_undo (we_window_t * window)
{
    return (e_make_rudo (window, 0));
}

/**
 * \brief executes a redo from the redo queue in the buffer
 *
 * @param window we_window_t the editting window containing the text where a change
 *               is to be undone.
 * @return int 0 if no error was encountered, -1 in case of an error,
 *
 */
int
e_make_redo (we_window_t * window)
{
    return (e_make_rudo (window, 1));
}

/**
 * \brief executes an undo or a redo from the undo respectively the redo queue in the buffer
 *
 * @param window we_window_t the editting window containing the text where a change
 *               is to be undone.
 * @param doing_redo int 0 if doing an undo, 1 if doing a redo.
 * @return int 0 if no error was encountered, -1 in case of an error,
 *
 * @see \struct undo for more information on `undo->type`.
 *
 * Remarks:
 *
 * * This function returns 0 but does nothing if it cannot find the editting
 *   containing the text to undo. This is perceived as an not an error!
 *
 * \todo find out how the function e_make_rudo recognizes the right window (uses DTMD_ISTEXT).
 *
 */
int
e_make_rudo (we_window_t * window, int doing_redo)
{
    we_buffer_t *buffer;
    we_screen_t *s;
    we_undo_t *undo;
    int i;

    for (i = window->edit_control->mxedt; i > 0 && !DTMD_ISTEXT (window->edit_control->window[i]->dtmd); i--);
    if (i <= 0)
        return (0);
    e_switch_window (window->edit_control->edt[i], window);
    window = window->edit_control->window[window->edit_control->mxedt];
    buffer = window->buffer;
    s = window->screen;
    undo = doing_redo ? buffer->redo : buffer->undo;
    if (undo == NULL)
    {
        e_error ((doing_redo ? e_msg[ERR_REDO] : e_msg[ERR_UNDO]), 0, buffer->colorset);
        return (-1);
    }
    window = window->edit_control->window[window->edit_control->mxedt];
    e_phase = doing_redo ? REDO_PHASE : UNDO_PHASE;
    buffer->cursor = undo->cursor_start;
    if (undo->type == 'r' || undo->type == 's')
    {
        if (undo->type == 's')
        {
            e_add_undo ('s', buffer, undo->cursor_start.x, undo->cursor_start.y, undo->begin_block.y);
            global_disable_add_undo = 1;
            e_del_nchar (buffer, s, undo->cursor_start.x, undo->cursor_start.y, undo->begin_block.y);
        }
        if (*((char *) undo->u.pt) == '\n' && undo->begin_block.x == 1)
            e_car_ret (buffer, s);
        else if (*((char *) undo->u.pt + undo->begin_block.x - 1) == '\n')
        {
            e_ins_nchar (buffer, s, ((unsigned char *) undo->u.pt), undo->cursor_start.x, undo->cursor_start.y,
                         undo->begin_block.x - 1);
            e_car_ret (buffer, s);
        }
        else
            e_ins_nchar (buffer, s, ((unsigned char *) undo->u.pt), undo->cursor_start.x, undo->cursor_start.y,
                         undo->begin_block.x);
        global_disable_add_undo = 0;
        s->mark_begin = undo->cursor_start;
        s->mark_end.y = undo->cursor_start.y;
        s->mark_end.x = undo->cursor_start.x + undo->begin_block.x;
        free (undo->u.pt);
    }
    else if (undo->type == 'l')
    {
        for (i = buffer->mxlines; i > undo->cursor_start.y; i--)
            buffer->buflines[i] = buffer->buflines[i - 1];
        (buffer->mxlines)++;
        buffer->buflines[buffer->cursor.y].s = undo->u.pt;
        buffer->buflines[buffer->cursor.y].len = e_str_len (buffer->buflines[buffer->cursor.y].s);
        buffer->buflines[buffer->cursor.y].nrc = strlen ((const char *) buffer->buflines[buffer->cursor.y].s);
        s->mark_begin = undo->cursor_start;
        s->mark_end.y = undo->cursor_start.y + 1;
        s->mark_end.x = 0;
        e_add_undo ('y', buffer, 0, buffer->cursor.y, 0);
    }
    else if (undo->type == 'y')
        e_del_line (buffer->cursor.y, buffer, s);
    else if (undo->type == 'a')
        e_del_nchar (buffer, s, undo->cursor_start.x, undo->cursor_start.y, undo->begin_block.x);
    else if (undo->type == 'p')
        buffer->buflines[undo->cursor_start.y].s[undo->cursor_start.x] = undo->u.c;
    else if (undo->type == 'c')
    {
        buffer->cursor = s->mark_begin = undo->begin_block;
        s->mark_end = undo->end_block;
        /*	e_blck_clear(buffer, s);   */
        e_blck_del (window);
    }
    else if (undo->type == 'v')
    {
        buffer->cursor = undo->cursor_start;
        s->mark_begin = undo->begin_block;
        s->mark_end = undo->end_block;
        e_blck_move (window);
    }
    else if (undo->type == 'd')
    {
        we_buffer_t *bn = (we_buffer_t *) undo->u.pt;
        global_disable_add_undo = 1;
        s->mark_begin = bn->window->screen->mark_begin;
        s->mark_end = bn->window->screen->mark_end;
        e_move_block (undo->cursor_start.x, undo->cursor_start.y, bn, buffer, window);
        global_disable_add_undo = 0;
        free (bn->window->screen);
        free (bn->window);
        free (bn->buflines[0].s);
        free (bn->buflines);
        free (undo->u.pt);
        e_add_undo ('c', buffer, undo->cursor_start.x, undo->cursor_start.y, 0);
    }
    if (doing_redo)
        buffer->redo = undo->next;
    else
        buffer->undo = undo->next;
    e_phase = EDIT_PHASE;
    free (undo);
    e_write_screen (window, 1);
    e_cursor (window, 1);
    return (0);
}

char *
e_make_postf (char *out, char *name, char *pf)
{
    strcpy (out, name);
    strcat (out, pf);
    return (out);
}
