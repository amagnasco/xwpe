/** \file we_progn.c                                       */
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
#include "we_control.h"
#include "edit.h"
#include "we_progn.h"
#include "WeExpArr.h"
#include "we_prog.h"
#include "WeProg.h"
#include "WeString.h"
#include "we_wind.h"

#ifdef UNIX
#include <unistd.h>
#endif

#undef TESTSDEF

#ifdef PROG
#include "utils.h"

extern struct dirfile *e_p_get_var (char *string);
extern char *e_p_msg[];

#define FRB1 s->colorset->cr.fg_bg_color	/*  key words etc  */
#define FRB2 s->colorset->ck.fg_bg_color	/*  constants           */
#define FRB3 s->colorset->cp.fg_bg_color	/*  pre-processor A.     */
#define FRB4 s->colorset->cc.fg_bg_color	/*  comments           */
#define FRB5 s->colorset->ct.fg_bg_color	/*  text                 */

#define iscase(s) ( (!strncmp(s, "case", 4) && !isalnum1(*(s+4)))	      \
		 || (!strncmp(s, "default", 7) && !isalnum1(*(s+7))) )
#define isstatus(s) ( !strncmp(s, "private:", 8) || !strncmp(s, "public:", 7) \
                 || !strncmp(s, "protected:", 10) )
#define ispif(s) ( !strncmp(s, "#if", 3) || !strncmp(s, "#ifdef", 6)  \
			|| !strncmp(s, "#ifndef", 7) )
#define ispelse(s) ( !strncmp(s, "#else", 5) || !strncmp(s, "#elif", 5) )
#define ispendif(s) ( !strncmp(s, "#endif", 6) )
#define iscop(c) ( c == '<' || c == '>' || c == '+' || c == '-' ||    \
		   c == '/' || c == '*' || c == '%' || c == '=' ||    \
		   c == '|' || c == '&' || c == '!' || c == ':' ||    \
		   c == '?' || c == '.' )

/*************************************************************************/
/**
 * e_mk_col
 *
 * This function was copied 1 on 1 from a define (yes, really, > 200 lines in a define....).
 * The code was left unchanged except for n_bg, frb, mscw and bssw that were turned into pointers.
 * These variables needed to be dereferenced to access the contents or to change the contents.
 *
 * Due to the read-write variables no return was necessary.
 *
 * The read-write variables are:
 *
 * int * frb:	a reference to syntax
 * int * n_bg:
 * int * mcsw:
 * int * bssw:
 *
 * The variable s hidden in a macro FRB1 .. FRB5 refers to an element of a screen.
 *
 * we_screen_t *s:	reference to an element of window->s
 *
 * This function is in desparate need of refactoring. But testing first.
 *
 */
void
e_mk_col (unsigned char *str, int l, int n, int *frb,
          struct wpeSyntaxRule *cs, int n_nd, int *n_bg, int *mcsw, int *bssw,
          int *svmsw, we_screen_t * s)
{
    if (n < l) {
        if (n == cs->special_column) {
            int ii;
            for (ii = 0; cs->special_comment[ii] &&
                    cs->special_comment[ii] != toupper (str[n]); ii++)
                ;
            if (cs->special_comment[ii]) {
                *mcsw = 7;
            }
        }
        if (*mcsw == 6 && (isalnum (str[n]) || str[n] == '_')) {
            *frb = FRB1;
        } else if (*mcsw == 7) {
            *frb = FRB4;
        } else if (*mcsw == 3 && (isalnum (str[n]) || str[n] == '.')) {
            *frb = FRB2;
        } else if (*mcsw == 5) {
            if (str[n] == cs->begin_comment[0]) {
                int jj;
                for (jj = 1; cs->begin_comment[jj] &&
                        str[n + jj] == cs->begin_comment[jj]; jj++)
                    ;
                if (!cs->begin_comment[jj]) {
                    *mcsw = 4;
                    *n_bg = n + jj - 1;
                    *frb = FRB4;
                }
            }
            if (*mcsw == 5 && str[n] == cs->line_comment[0]) {
                int jj;
                for (jj = 1; cs->line_comment[jj] &&
                        str[n + jj] == cs->line_comment[jj]; jj++);
                if (!cs->line_comment[jj]) {
                    *mcsw = 7;
                    *frb = FRB4;
                }
            }
            if (*mcsw == 5) {
                *frb = FRB3;
            }
        } else if (*mcsw == 1) {
            if (str[n] == cs->string_constant && !*bssw) {
                *mcsw = 0;
            }
            *frb = FRB2;
        } else if (*mcsw == 2) {
            if (str[n] == cs->char_constant && !*bssw) {
                *mcsw = 0;
            }
            *frb = FRB2;
        } else if (*mcsw == 4) {
            if (n_nd < n - *n_bg && str[n] == cs->end_comment[n_nd]) {
                int jj;
                for (jj = 1; jj <= n_nd && n - jj >= 0 &&
                        str[n - jj] == cs->end_comment[n_nd - jj]; jj++);
                if (jj > n_nd) {
                    *mcsw = *svmsw;
                }
            }
            *frb = FRB4;
        } else {
            if (n >= cs->comment_column) {
                *mcsw = 7;
                *frb = FRB4;
            } else if (isdigit (str[n])) {
                if (n == 0 || (!isalnum (str[n - 1]) && str[n - 1] != '_')) {
                    *mcsw = 3;
                    *frb = FRB2;
                } else {
                    *frb = FRB5;
                }
            } else if (isalpha (str[n])) {
                if (cs->insensitive && (n == 0 || (!isalnum (str[n - 1]) &&
                                                   str[n - 1] != '_'))) {
                    int ii, jj;
                    int kk = strlen ((const char *) cs->line_comment);
                    if ((WpeStrnccmp
                            ((char *) cs->line_comment, (const char *) str + n,
                             kk) == 0) && (!isalnum (str[n + kk]))
                            && (str[n + kk] != '_')) {
                        *mcsw = 7;
                        *frb = FRB4;
                    } else {
                        for (ii = 0; cs->reserved_word[ii] &&
                                cs->reserved_word[ii][0] < toupper (str[n]); ii++);
                        for (; cs->reserved_word[ii] &&
                                cs->reserved_word[ii][0] == toupper (str[n]); ii++) {
                            for (jj = 0; cs->reserved_word[ii][jj] &&
                                    cs->reserved_word[ii][jj] ==
                                    toupper (str[n + jj]); jj++);
                            if (!cs->reserved_word[ii][jj]
                                    && !isalnum (str[n + jj]) && str[n + jj] != '_') {
                                *mcsw = 6;
                                *frb = FRB1;
                                break;
                            }
                        }
                    }
                } else if (!cs->insensitive
                           && (n == 0
                               || (!isalnum (str[n - 1]) && str[n - 1] != '_'))) {
                    int ii, jj;
                    int kk = strlen ((const char *) cs->line_comment);
                    if ((strncmp
                            ((const char *) cs->line_comment,
                             (const char *) str + n, kk) == 0)
                            && (!isalnum (str[n + kk])) && (str[n + kk] != '_')) {
                        *mcsw = 7;
                        *frb = FRB4;
                    } else {
                        for (ii = 0; cs->reserved_word[ii] &&
                                cs->reserved_word[ii][0] < str[n]; ii++);
                        for (; cs->reserved_word[ii] &&
                                cs->reserved_word[ii][0] == str[n]; ii++) {
                            for (jj = 0; cs->reserved_word[ii][jj] &&
                                    cs->reserved_word[ii][jj] == str[n + jj];
                                    jj++);
                            if (!cs->reserved_word[ii][jj]
                                    && !isalnum (str[n + jj]) && str[n + jj] != '_') {
                                *mcsw = 6;
                                *frb = FRB1;
                                break;
                            }
                        }
                    }
                }
                if (!*mcsw) {
                    *frb = FRB5;
                }
            } else if (isspace (str[n])) {
                *mcsw = 0;
                *frb = FRB5;
            } else {
                if (str[n] == cs->string_constant) {
                    *mcsw = 1;
                    *frb = FRB2;
                } else if (str[n] == cs->char_constant) {
                    *mcsw = 2;
                    *frb = FRB2;
                } else if (str[n] == cs->preproc_cmd) {
                    *svmsw = *mcsw = 5;
                    *frb = FRB3;
                } else {
                    *mcsw = 0;
                    if (str[n] == cs->begin_comment[0]) {
                        int jj;
                        for (jj = 1; cs->begin_comment[jj] &&
                                str[n + jj] == cs->begin_comment[jj]; jj++);
                        if (!cs->begin_comment[jj]) {
                            *mcsw = 4;
                            *n_bg = n + jj - 1;
                            *frb = FRB4;
                        }
                    }
                    if (!*mcsw && str[n] == cs->line_comment[0]) {
                        int jj;
                        for (jj = 1; cs->line_comment[jj] &&
                                str[n + jj] == cs->line_comment[jj]; jj++);
                        if (!cs->line_comment[jj]) {
                            *mcsw = 7;
                            *frb = FRB4;
                        }
                    }
                    if (cs->long_operator && !*mcsw) {
                        if (cs->insensitive) {
                            int ii, jj;
                            for (ii = 0; cs->long_operator[ii] &&
                                    cs->long_operator[ii][0] < str[n]; ii++);
                            for (; cs->long_operator[ii] &&
                                    cs->long_operator[ii][0] == str[n]; ii++) {
                                for (jj = 0; cs->long_operator[ii][jj] &&
                                        cs->long_operator[ii][jj] ==
                                        toupper (str[n + jj]); jj++);
                                if (!cs->long_operator[ii][jj]) {
                                    *mcsw = 6;
                                    *frb = FRB1;
                                    break;
                                }
                            }
                        } else {
                            int ii, jj;
                            for (ii = 0; cs->long_operator[ii] &&
                                    cs->long_operator[ii][0] < str[n]; ii++);
                            for (; cs->long_operator[ii] &&
                                    cs->long_operator[ii][0] == str[n]; ii++) {
                                for (jj = 0; cs->long_operator[ii][jj] &&
                                        cs->long_operator[ii][jj] == str[n + jj];
                                        jj++);
                                if (!cs->long_operator[ii][jj]) {
                                    *mcsw = 6;
                                    *frb = FRB1;
                                    break;
                                }
                            }
                        }
                    }
                    if (!*mcsw) {
                        int jj;
                        for (jj = 0; cs->single_operator[jj] &&
                                str[n] != cs->single_operator[jj]; jj++);
                        if (!cs->single_operator[jj]) {
                            *frb = FRB5;
                        } else {
                            *frb = FRB1;
                        }
                    }
                }
            }
        }
        if (str[n] == cs->quoting_char) {
            *bssw = !*bssw;
        } else {
            *bssw = 0;
        }
    } else {
        if ((*mcsw == 7) || (*mcsw == 4)) {
            *frb = FRB4;
        } else if ((*mcsw == 1) || (*mcsw == 2)) {
            *frb = FRB2;
        } else if (*mcsw == 5) {
            *frb = FRB3;
        } else {
            *frb = FRB5;
        }
    }
}

/*************************************************************************/

int
e_scfbol (int n, int mcsw, unsigned char *str, WpeSyntaxRule * cs)
{
    int bssw = 0, svmsw = 0, nla = 0, i, j;

    for (i = 0; i < n && isspace (str[i]); i++)
        ;
    if (mcsw == 5) {
        svmsw = 5;
    } else if (mcsw == 0 && str[i] == cs->preproc_cmd) {
        svmsw = mcsw = 5;
        i++;
    }
    for (; i < n; i++) {
        if (i == cs->special_column) {
            return (0);
        } else if (mcsw == 4) {
            if (str[i] == cs->end_comment[0]) {
                for (j = 1;
                        cs->end_comment[j] && str[i + j] == cs->end_comment[j];
                        j++)
                    ;
                if (!cs->end_comment[j]) {
                    i += (j - 1);
                    mcsw = svmsw;
                }
            }
        } else if (mcsw == 1) {
            if (str[i] == cs->string_constant && !bssw) {
                mcsw = svmsw;
            }
        } else if (str[i] == cs->string_constant && !bssw) {
            mcsw = 1;
        } else {
            if (str[i] == cs->line_comment[0]) {
                for (j = 1;
                        cs->line_comment[j] && str[i + j] == cs->line_comment[j];
                        j++)
                    ;
                if (!cs->line_comment[j]) {
                    return (0);
                }
            }
            if (str[i] == cs->begin_comment[0]) {
                for (j = 1;
                        cs->begin_comment[j] && str[i + j] == cs->begin_comment[j];
                        j++)
                    ;
                if (!cs->begin_comment[j]) {
                    mcsw = 4;
                }
                i += (j - 1);
            }
        }
        if (str[i] == cs->quoting_char) {
            bssw = !bssw;
        } else {
            bssw = 0;
        }
    }
    return ((mcsw == 4 || (bssw && n > 0 && str[n - 1] == cs->continue_char) ||
             nla || cs->continue_column >= 0) ? mcsw : 0);
}

int
e_sc_all (we_window_t * window, int sw)
{
    int i;

    if (!WpeIsProg () || !WpeIsXwin ()) {
        return (0);
    }
    if (!sw) {
        for (i = 0; i <= window->edit_control->mxedt; i++)
            if (window->edit_control->window[i]->c_sw) {
                free (window->edit_control->window[i]->c_sw);
                window->edit_control->window[i]->c_sw = NULL;
            }
    } else {
        for (i = 0; i <= window->edit_control->mxedt; i++) {
            if (window->edit_control->window[i]->c_sw) {
                free (window->edit_control->window[i]->c_sw);
            }
            e_add_synt_tl (window->edit_control->window[i]->datnam, window->edit_control->window[i]);
            if (window->edit_control->window[i]->c_st) {
                if (window->edit_control->window[i]->c_sw) {
                    free (window->edit_control->window[i]->c_sw);
                }
                window->edit_control->window[i]->c_sw = e_sc_txt (NULL, window->edit_control->window[i]->buffer);
            }
        }
    }
    e_rep_win_tree (window->edit_control);
    return (0);
}

int
e_program_opt (we_window_t * window)
{
    int ret, sw = window->edit_control->edopt & ED_SYNTAX_HIGHLIGHT ? 1 : 0;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o) {
        return (-1);
    }
    o->xa = 17;
    o->ya = 4;
    o->xe = 59;
    o->ye = 18;
    o->bgsw = AltO;
    o->name = "Programming-Options";
    o->crsw = AltO;
    e_add_txtstr (21, 2, "Screen:", o);
    e_add_txtstr (4, 2, "Stop at", o);
    e_add_sswstr (5, 3, 0, AltE, window->edit_control->edopt & ED_ERRORS_STOP_AT ? 1 : 0,
                  "Errors  ", o);
    e_add_sswstr (5, 4, 0, AltM, window->edit_control->edopt & ED_MESSAGES_STOP_AT ? 1 : 0,
                  "Messages", o);
    e_add_sswstr (22, 3, 0, AltD, window->edit_control->edopt & ED_SYNTAX_HIGHLIGHT ? 1 : 0,
                  "Diff. Colors", o);
    e_add_wrstr (4, 6, 4, 7, 35, 128, 1, AltX, "EXecution-Directory:",
                 e_prog.exedir, NULL, o);
    e_add_wrstr (4, 9, 4, 10, 35, 128, 0, AltS, "System-Include-Path:",
                 e_prog.sys_include, NULL, o);
    e_add_bttstr (10, 12, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (26, 12, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        if (e_prog.exedir) {
            free (e_prog.exedir);
        }
        e_prog.exedir = WpeStrdup (o->wstr[0]->txt);
        if (e_prog.sys_include) {
            free (e_prog.sys_include);
        }
        e_prog.sys_include = WpeStrdup (o->wstr[1]->txt);
        window->edit_control->edopt = (window->edit_control->edopt & ~ED_PROGRAMMING_OPTIONS) +
                                      (o->sstr[0]->num ? ED_ERRORS_STOP_AT : 0) +
                                      (o->sstr[1]->num ? ED_MESSAGES_STOP_AT : 0) +
                                      (o->sstr[2]->num ? ED_SYNTAX_HIGHLIGHT : 0);
        if (sw != o->sstr[2]->num) {
            e_sc_all (window, o->sstr[2]->num);
        }
    }
    freeostr (o);
    return (0);
}

int
e_sc_nw_txt (int y, we_buffer_t * buffer, int sw)
{
    int i, out;

    if (sw < 0) {
        out = buffer->window->c_sw[y + 1];
        for (i = y + 1; i < buffer->mxlines; i++) {
            buffer->window->c_sw[i] = buffer->window->c_sw[i + 1];
        }
        if (out == buffer->window->c_sw[y]) {
            return (0);
        }
    } else if (sw > 0)
        for (i = buffer->mxlines - 1; i > y; i--) {
            buffer->window->c_sw[i] = buffer->window->c_sw[i - 1];
        }

    if (buffer->window->c_st->continue_column < 0) {
        for (i = y; i < buffer->mxlines - 1; i++) {
            if ((out =
                        e_scfbol (buffer->buflines[i].len, buffer->window->c_sw[i], buffer->buflines[i].s,
                                  buffer->window->c_st)) == buffer->window->c_sw[i + 1]) {
                break;
            } else {
                buffer->window->c_sw[i + 1] = out;
            }
        }
    } else {
        if (y != 0) {	/* Quick fix to continue line check - Dennis */
            buffer->window->c_sw[y] =
                isspace (buffer->buflines[y].
                         s[buffer->window->c_st->continue_column]) ? 0 : e_scfbol (buffer->buflines[y -
                                 1].
                                 len,
                                 buffer->window->
                                 c_sw[y -
                                        1],
                                 buffer->buflines[y -
                                                    1].
                                 s,
                                 buffer->window->
                                 c_st);
            for (i = y; i < buffer->mxlines - 1; i++) {
                out =
                    isspace (buffer->buflines[i + 1].s[buffer->window->c_st->continue_column]) ?  0
                    : e_scfbol (buffer->buflines[i].len, buffer-> window-> c_sw [i], buffer->buflines[i].s, buffer->window->c_st);
                if (out == buffer->window->c_sw[i + 1]) {
                    break;
                } else {
                    buffer->window->c_sw[i + 1] = out;
                }
            }
        }
    }
    return (0);
}

int *
e_sc_txt (int *c_sw, we_buffer_t * buffer)
{
    int i;

    if (!c_sw) {
        c_sw = malloc (buffer->mx.y * sizeof (int));
    }
    c_sw[0] = 0;
    if (buffer->window->c_st->continue_column < 0) {
        for (i = 0; i < buffer->mxlines - 1; i++)
            c_sw[i + 1] =
                e_scfbol (buffer->buflines[i].len, c_sw[i], buffer->buflines[i].s, buffer->window->c_st);
    } else {
        for (i = 0; i < buffer->mxlines - 1; i++) {
            c_sw[i + 1] =
                isspace (buffer->buflines[i + 1].
                         s[buffer->window->c_st->continue_column]) ? 0 : e_scfbol (buffer->buflines[i].len,
                                 c_sw[i], buffer->buflines[i].s, buffer->window->c_st);
        }
    }
    return (c_sw);
}

void e_sc_txt_2 (we_window_t *window)
{
    if(window->c_sw) {
        if(window->screen->mark_begin.y == window->screen->mark_end.y) {
            e_sc_nw_txt(window->screen->mark_end.y, window->buffer, 0);
        } else {
            window->c_sw = realloc(window->c_sw, window->buffer->mx.y * sizeof(int));
            window->c_sw = e_sc_txt(window->c_sw, window->buffer);
        }
    }
}

/*       Writing of a line (content of a screen)      */
void
e_pr_c_line (int y, we_window_t * window)
{
    we_buffer_t *buffer = window->buffer;
    we_screen_t *s = window->screen;
    int i, j, k, frb = 0;
    int mcsw = window->c_sw[y], svmsw = window->c_sw[y] == 5 ? 5 : 0, bssw = 0;
    int n_bg = -1, n_nd = strlen ((const char *) window->c_st->end_comment) - 1;
#ifdef DEBUGGER
    int fsw = 0;
#endif

    for (i = j = 0; j < s->c.x; j++, i++) {
        if (*(buffer->buflines[y].s + i) == WPE_TAB) {
            j += (window->edit_control->tabn - j % window->edit_control->tabn - 1);
        }
#ifdef UNIX
        else if (((unsigned char) *(buffer->buflines[y].s + i)) > 126) {
            j++;
            if (((unsigned char) *(buffer->buflines[y].s + i)) < 128 + ' ') {
                j++;
            }
        } else if (*(buffer->buflines[y].s + i) < ' ') {
            j++;
        }
#endif
    }
    if (j > s->c.x) {
        i--;
    }
    for (k = 0; k < i; k++)
        e_mk_col (buffer->buflines[y].s, buffer->buflines[y].len, k, &frb, window->c_st, n_nd, &n_bg, &mcsw,
                  &bssw, &svmsw, s);
#ifdef DEBUGGER
    for (j = 1; j <= s->breakpoint[0]; j++)
        if (s->breakpoint[j] == y) {
            fsw = 1;
            break;
        }
    for (j = s->c.x;
            i < buffer->buflines[y].len && j < num_cols_on_screen_safe(window) + s->c.x - 1; i++, j++) {
        e_mk_col (buffer->buflines[y].s, buffer->buflines[y].len, i, &frb, window->c_st, n_nd, &n_bg,
                  &mcsw, &bssw, &svmsw, s);
        if (y == s->da.y && i >= s->da.x && i < s->de.x) {
            frb = s->colorset->dy.fg_bg_color;
        } else if (fsw) {
            frb = s->colorset->db.fg_bg_color;
        } else if (y == s->fa.y && i >= s->fa.x && i < s->fe.x) {
            frb = s->colorset->ek.fg_bg_color;
        }
#else
    for (j = s->c.x;
            i < buffer->buflines[y].len && j < num_cols_on_screen_safe(window) + s->c.x - 1; i++, j++) {
        e_mk_col (buffer->buflines[y].s, buffer->buflines[y].len, i, &frb, window->c_st, n_nd, &n_bg,
                  &mcsw, &bssw, &svmsw, s);
        if (y == s->fa.y && i >= s->fa.x && i < s->fe.x) {
            frb = s->colorset->ek.fg_bg_color;
        }
#endif
        else if ((y < s->mark_end.y && (y > s->mark_begin.y ||
                                        (y == s->mark_begin.y
                                         && i >= s->mark_begin.x)))
                 || (y == s->mark_end.y && i < s->mark_end.x
                     && (y > s->mark_begin.y
                         || (y == s->mark_begin.y && i >= s->mark_begin.x)))) {
            frb = s->colorset->ez.fg_bg_color;
        }
        if (*(buffer->buflines[y].s + i) == WPE_TAB)
            for (k = window->edit_control->tabn - j % window->edit_control->tabn;
                    k > 1 && j < num_cols_on_screen(window) + s->c.x - 2; k--, j++)
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1, ' ',
                           frb);
#ifdef UNIX
        else if (!WpeIsXwin () && ((unsigned char) *(buffer->buflines[y].s + i)) > 126) {
            e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1, '@',
                       frb);
            if (++j >= num_cols_on_screen(window) + s->c.x - 1) {
                return;
            }
            if (((unsigned char) *(buffer->buflines[y].s + i)) < 128 + ' '
                    && j < num_cols_on_screen(window) + s->c.x - 1) {
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           '^', frb);
                if (++j >= num_cols_on_screen(window) + s->c.x - 1) {
                    return;
                }
            }
        } else if (*(buffer->buflines[y].s + i) < ' ') {
            e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1, '^',
                       frb);
            if (++j >= num_cols_on_screen(window) + s->c.x - 1) {
                return;
            }
        }
#endif
        if (*(buffer->buflines[y].s + i) == WPE_TAB)
            e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1, ' ',
                       frb);
#ifdef UNIX
        else if (!WpeIsXwin () && ((unsigned char) *(buffer->buflines[y].s + i)) > 126 &&
                 j < num_cols_on_screen(window) + s->c.x - 1) {
            if (((unsigned char) *(buffer->buflines[y].s + i)) < 128 + ' ')
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           ((unsigned char) *(buffer->buflines[y].s + i)) + 'A' - 129, frb);
            else
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           ((unsigned char) *(buffer->buflines[y].s + i)) - 128, frb);
        } else if (*(buffer->buflines[y].s + i) < ' ' && j < num_cols_on_screen(window) + s->c.x - 1)
            e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                       *(buffer->buflines[y].s + i) + 'A' - 1, frb);
#endif
        else
            e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                       *(buffer->buflines[y].s + i), frb);
    }

    e_mk_col (buffer->buflines[y].s, buffer->buflines[y].len, i, &frb, window->c_st, n_nd, &n_bg, &mcsw,
              &bssw, &svmsw, s);

    if ((i == buffer->buflines[y].len) && (window->edit_control->edopt & ED_SHOW_ENDMARKS) &&
            (DTMD_ISMARKABLE (window->dtmd))
            && (j < num_cols_on_screen_safe(window) + s->c.x - 1)) {
        if ((y < s->mark_end.y && (y > s->mark_begin.y ||
                                   (y == s->mark_begin.y
                                    && i >= s->mark_begin.x)))
                || (y == s->mark_end.y && i < s->mark_end.x
                    && (y > s->mark_begin.y
                        || (y == s->mark_begin.y && i >= s->mark_begin.x)))) {
            if (*(buffer->buflines[y].s + i) == WPE_WR)
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           PWR, s->colorset->ez.fg_bg_color);
            else
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           PNL, s->colorset->ez.fg_bg_color);
        } else {
            if (*(buffer->buflines[y].s + i) == WPE_WR)
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           PWR, frb);
            else
                e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1,
                           PNL, frb);
        }
        j++;
    }
    for (; j < num_cols_on_screen(window) + s->c.x - 1; j++) {
        e_pr_char (window->a.x - s->c.x + j + 1, y - s->c.y + window->a.y + 1, ' ', frb);
    }
}

int
e_add_synt_tl (char *filename, we_window_t * window)
{
    int i, k;

    window->c_st = NULL;
    window->c_sw = NULL;
    if (!WpeSyntaxDef || !filename) {
        return 0;
    }
    filename = strrchr (filename, '.');
    if (!filename) {
        return 0;
    }
    for (i = 0; i < WpeExpArrayGetSize (WpeSyntaxDef); i++) {
        for (k = 0; k < WpeExpArrayGetSize (WpeSyntaxDef[i]->extension); k++) {
            if (!strcmp (filename, WpeSyntaxDef[i]->extension[k])) {
                window->c_st = WpeSyntaxDef[i]->syntax_rule;
                if (window->edit_control->edopt & ED_SYNTAX_HIGHLIGHT) {
                    window->c_sw = malloc (window->buffer->mx.y * sizeof (int));
                }
            }
        }
    }
    return (0);
}

/*  Browser   */
E_AFILE *
e_aopen (char *name, char *path, int mode)
{
    we_control_t *control = global_editor_control;
    E_AFILE *ep = malloc (sizeof (E_AFILE));
    char str[256];
    int i, j;

    ep->buffer = NULL;
    ep->fp = NULL;
    if (mode & 1) {
        for (i = 1; i <= control->mxedt && strcmp (control->window[i]->datnam, name); i++)
            ;
        if (i <= control->mxedt) {
            ep->buffer = control->window[i]->buffer;
            ep->p.x = ep->p.y = 0;
        }
    }
    if ((mode & 2) && !ep->buffer && !access (name, R_OK)) {
        ep->fp = fopen (name, "r");
    }
    if ((mode & 4) && !ep->buffer && !ep->fp) {
        for (i = 0; path[i] && !ep->fp; i++) {
            for (j = 0; (str[j] = path[i]) && path[i] != ':'; j++, i++)
                ;
            if (!path[i]) {
                i--;
            }
            str[j] = '/';
            str[j + 1] = '\0';
            strcat (str, name);
            if (!access (str, R_OK)) {
                ep->fp = fopen (str, "r");
            }
            if (ep->fp) {
                strcpy (name, str);
            }
        }
    }
    if (!ep->buffer && !ep->fp) {
        free (ep);
        return NULL;
    }
    return (ep);
}

int
e_aclose (E_AFILE * ep)
{
    int ret = 0;

    if (ep->fp) {
        ret = fclose (ep->fp);
    }
    free (ep);
    return (ret);
}

char *
e_agets (char *str, int n, E_AFILE * ep)
{
    int i, j;

    if (ep->fp) {
        return (fgets (str, n, ep->fp));
    }
    if (ep->p.y >= ep->buffer->mxlines ||
            (ep->p.y == ep->buffer->mxlines - 1 && ep->p.x > ep->buffer->buflines[ep->p.y].len)) {
        return (NULL);
    }
    for (i = 0; ep->p.y < ep->buffer->mxlines; (ep->p.y)++) {
        for (j = ep->p.x; i < n - 1 && j <= ep->buffer->buflines[ep->p.y].len; i++, j++) {
            str[i] = ep->buffer->buflines[ep->p.y].s[j];
        }

        if (str[i - 1] == '\n') {
            ep->p.x = 0;
            (ep->p.y)++;
            break;
        } else if (i == n - 1) {
            ep->p.x = j;
            break;
        } else {
            ep->p.x = 0;
            (ep->p.y)++;
        }
    }
    str[i] = '\0';
    return (str);
}

char *
e_sh_spl1 (char *sp, char *str, E_AFILE * fp, int *n)
{
    while (1) {
        while (isspace (*++sp))
            ;
        if (sp[0] == '\\') {
            (*n)++;
            if (!e_agets ((sp = str), 256, fp)) {
                return (NULL);
            }
            --sp;
        } else if (sp[0] == '/' && sp[1] == '*') {
            while (!(sp = strstr (sp, "*/"))) {
                (*n)++;
                if (!e_agets ((sp = str), 256, fp)) {
                    return (NULL);
                }
            }
            sp++;
        } else {
            break;
        }
    }
    return (sp);
}

char *
e_sh_spl2 (char *sp, char *str, E_AFILE * fp, int *n)
{
    while (1) {
        while (isspace (*++sp))
            ;
        if (!sp[0] || sp[0] == '\n' || sp[0] == '\\') {
            (*n)++;
            if (!e_agets ((sp = str), 256, fp)) {
                return (NULL);
            }
            --sp;
        } else if (sp[0] == '/' && sp[1] == '*') {
            while (!(sp = strstr (sp, "*/"))) {
                (*n)++;
                if (!e_agets ((sp = str), 256, fp)) {
                    return (NULL);
                }
            }
            sp++;
        } else {
            break;
        }
    }
    return (sp);
}

char *
e_sh_spl3 (sp, str, fp, n)
char *sp;
char *str;
E_AFILE *fp;
int *n;
{
    int brk = 1;
    while (*++sp != '}' || brk > 1) {
        if (!sp[0] || sp[0] == '\n') {
            (*n)++;
            if (!e_agets ((sp = str), 256, fp)) {
                return (NULL);
            }
            if (isspace (*sp))
                while (isspace (*++sp));
            if (sp[0] == '#' && ispelse (sp)) {
                do {
                    (*n)++;
                    if (!e_agets ((sp = str), 256, fp)) {
                        return (NULL);
                    }
                    if (isspace (*sp))
                        while (isspace (*++sp));
                } while (sp[0] != '#' || !ispendif (sp));
            }
            sp--;
        } else if (sp[0] == '/' && sp[1] == '*') {
            while (!(sp = strstr (sp, "*/"))) {
                (*n)++;
                if (!e_agets ((sp = str), 256, fp)) {
                    return (NULL);
                }
            }
            sp++;
        } else if (sp[0] == '{') {
            brk++;
        } else if (sp[0] == '}') {
            brk--;
        } else if (sp[0] == '\'') {
            int bsl = 0;
            while (*++sp && (sp[0] != '\'' || bsl)) {
                bsl = *sp == '\\' ? !bsl : 0;
            }
            if (!*sp) {
                sp--;
            }
        } else if (sp[0] == '\"') {
            int bsl = 0;
            while (*++sp && (*sp != '\"' || bsl)) {
                bsl = *sp == '\\' ? !bsl : 0;
            }
            if (!*sp) {
                sp--;
            }
        }
    }
    return (sp);
}

char *
e_sh_spl5 (sp, str, fp, n)
char *sp;
char *str;
E_AFILE *fp;
int *n;
{
    int brk = 1;
    while (*++sp != ')' || brk > 1) {
        if (!sp[0] || sp[0] == '\n') {
            (*n)++;
            if (!e_agets ((sp = str), 256, fp)) {
                return (NULL);
            }
            if (isspace (*sp))
                while (isspace (*++sp));
            if (sp[0] == '#' && ispelse (sp)) {
                do {
                    (*n)++;
                    if (!e_agets ((sp = str), 256, fp)) {
                        return (NULL);
                    }
                    if (isspace (*sp))
                        while (isspace (*++sp));
                } while (sp[0] != '#' || !ispendif (sp));
            }
            sp--;
        } else if (sp[0] == '/' && sp[1] == '*') {
            while (!(sp = strstr (sp, "*/"))) {
                (*n)++;
                if (!e_agets ((sp = str), 256, fp)) {
                    return (NULL);
                }
            }
            sp++;
        } else if (sp[0] == '(') {
            brk++;
        } else if (sp[0] == ')') {
            brk--;
        } else if (sp[0] == '\'') {
            int bsl = 0;
            while (*++sp && (sp[0] != '\'' || bsl)) {
                bsl = *sp == '\\' ? !bsl : 0;
            }
            if (!*sp) {
                sp--;
            }
        } else if (sp[0] == '\"') {
            int bsl = 0;
            while (*++sp && (*sp != '\"' || bsl)) {
                bsl = *sp == '\\' ? !bsl : 0;
            }
            if (!*sp) {
                sp--;
            }
        }
    }
    return (sp);
}

char *
e_sh_spl4 (sp, str, fp, n)
char *sp;
char *str;
E_AFILE *fp;
int *n;
{
    int brk = 0;
    while ((*++sp != ',' && *sp != ';' && *sp != '(') || brk) {
        if (!sp[0] || sp[0] == '\n') {
            (*n)++;
            if (!e_agets ((sp = str), 256, fp)) {
                return (NULL);
            }
            --sp;
        } else if (sp[0] == '/' && sp[1] == '*') {
            while (!(sp = strstr (sp, "*/"))) {
                (*n)++;
                if (!e_agets ((sp = str), 256, fp)) {
                    return (NULL);
                }
            }
            sp++;
        } else if (sp[0] == '\"') {
            int bsl = 0;
            while (*++sp && (sp[0] != '\"' || bsl)) {
                bsl = *sp == '\\' ? !bsl : 0;
            }
            if (!*sp) {
                sp--;
            }
        } else if (sp[0] == '\'') {
            int bsl = 0;
            while (*++sp && (sp[0] != '\'' || bsl)) {
                bsl = *sp == '\\' ? !bsl : 0;
            }
            if (!*sp) {
                sp--;
            }
        } else if (sp[0] == '{') {
            brk++;
        } else if (sp[0] == '}') {
            brk--;
        }
    }
    return (sp);
}

struct dirfile *
e_c_add_df (char *str, struct dirfile *df)
{
    if (df == NULL) {
        df = malloc (sizeof (struct dirfile));
        df->nr_files = 0;
        df->name = malloc (sizeof (char *));
    }
    df->nr_files++;
    df->name = realloc (df->name, df->nr_files * sizeof (char *));
    df->name[df->nr_files - 1] = malloc ((strlen (str) + 1) * sizeof (char));
    strcpy (df->name[df->nr_files - 1], str);
    return (df);
}

int
e_find_def (name, startfile, mode, file, num, xn, nold, oldfile, df, first)
char *name;
char *startfile;
int mode;
char *file;
int *num;
int *xn;
int nold;
char *oldfile;
struct dirfile **df;
int *first;
{
    E_AFILE *fp = NULL;
    char *sp = NULL, str[256], *w, word[256];
    int i, n, com = 0, ret = 1;
    int len = strlen (name);
    if (*df) {
        for (i = 0; i < (*df)->nr_files; i++)
            if (!strcmp ((*df)->name[i], startfile)) {
                return (-2);
            }
    }
    *df = e_c_add_df (startfile, *df);
    if (!fp) {
        fp = e_aopen (startfile, e_prog.sys_include, mode);
    }
    if (!fp) {
        return (-1);
    }
    for (n = 0; com == 2 || e_agets ((sp = str), 256, fp); n++) {
        if (com) {
            if (com == 1 && !(sp = strstr (sp, "*/"))) {
                continue;
            } else {
                com = 0;
            }
        }
        if (isspace (*sp) && !(sp = e_sh_spl2 (sp, str, fp, &n))) {
            goto b_end;
        }
        if (sp[0] == '/' && sp[1] == '*') {
            if (!(sp = strstr (sp, "*/"))) {
                com = 1;
            } else {
                n--;
                com = 2;
                sp += 2;
            }
            continue;
        } else if (*sp == '#') {
            if (!(sp = e_sh_spl1 (sp, str, fp, &n))) {
                goto b_end;
            }
            if (!strncmp (sp, "define", 6)) {
                while (isalpha (*++sp)) {
                    ;
                }
                if (isspace (*sp) && !(sp = e_sh_spl1 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (!strncmp (sp, name, len) && !isalnum1 (sp[len])) {
                    if (*first) {
                        e_aclose (fp);
                        strcpy (file, startfile);
                        *num = n;
                        *xn = ((int) (sp - str)) + len;
                        return (0);
                    } else if (n == nold && !strcmp (startfile, oldfile)) {
                        *first = 1;
                    }
                }
            }
#ifndef TESTSDEF
            else if (!strncmp (sp, "include", 7)) {
                while (isalpha (*++sp)) {
                    ;
                }
                if (isspace (*sp) && !(sp = e_sh_spl1 (sp, str, fp, &n))) {
                    goto b_end;
                }
                for (i = 1; (word[i - 1] = sp[i])
                        && sp[i] != '\"' && sp[i] != '>'; i++);
                word[i - 1] = '\0';
                if (!e_find_def (name, word, sp[i] == '>' ? 4 : 7,
                                 file, num, xn, nold, oldfile, df, first)) {
                    e_aclose (fp);
                    return (0);
                }
            }
#endif
            while (sp[i = (strlen (sp) - 1)] != '\n' || sp[i - 1] == '\\') {
                n++;
                if (!e_agets ((sp = str), 256, fp)) {
                    goto b_end;
                }
            }
        } else if (*sp == '{') {
            if (!(sp = e_sh_spl3 (sp, str, fp, &n))) {
                goto b_end;
            }
            sp++;
            com = 2;
            n--;
        } else if (!strncmp (sp, "extern", 6)) {
            continue;
        } else if (!strncmp (sp, "typedef", 7)) {
            while (!isspace (*++sp)) {
                ;
            }
            if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                goto b_end;
            }
            if (!strncmp (sp, "struct", 6) || !strncmp (sp, "class", 5) ||
                    !strncmp (sp, "union", 5)) {
                while (!isspace (*++sp)) {
                    ;
                }
                if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (*sp == ';') {
                    sp++;
                    com = 2;
                    n--;
                    continue;
                }
                if (!strncmp (sp, name, len) && !isalnum1 (sp[len])) {
                    while (!isspace (*++sp)) {
                        ;
                    }
                    if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (*sp == '{') {
                        if (*first) {
                            e_aclose (fp);
                            strcpy (file, startfile);
                            *num = n;
                            *xn = (int) (sp - str);
                            return (0);
                        } else if (n == nold && !strcmp (startfile, oldfile)) {
                            *first = 1;
                        }
                    }
                }
            }
            while (1) {
                if (isalpha1 (*sp)) {
                    for (w = word; isalnum1 (*w = *sp); w++, sp++) {
                        ;
                    }
                    *w = '\0';
                    if (isspace (*sp) && !(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (*sp == ';') {
                        if (!strncmp (word, name, len)) {
                            if (*first) {
                                e_aclose (fp);
                                strcpy (file, startfile);
                                *num = n;
                                *xn = (int) (sp - str);
                                return (0);
                            } else if (n == nold && !strcmp (startfile, oldfile)) {
                                *first = 1;
                            }
                        }
                        sp++;
                        com = 2;
                        n--;
                        break;
                    }
                } else if (*sp == '{') {
                    if (!(sp = e_sh_spl3 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (*sp == ';') {
                        sp++;
                        com = 2;
                        n--;
                        break;
                    }
                } else if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
            }
        } else if (!strncmp (sp, "struct", 6) || !strncmp (sp, "class", 5) ||
                   !strncmp (sp, "union", 5)) {
            while (!isspace (*++sp)) {
                ;
            }
            if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                goto b_end;
            }
            if (*sp == ';') {
                sp++;
                com = 2;
                n--;
                continue;
            }
            if (!strncmp (sp, name, len) && !isalnum1 (sp[len])) {
                while (!isspace (*++sp)) {
                    ;
                }
                if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (*sp == '{') {
                    if (*first) {
                        e_aclose (fp);
                        strcpy (file, startfile);
                        *num = n;
                        *xn = (int) (sp - str);
                        return (0);
                    } else if (n == nold && !strcmp (startfile, oldfile)) {
                        *first = 1;
                    }
                }
            } else if (*sp != '{') {
                while (!isspace (*++sp)) {
                    ;
                }
                if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
            }
            if (*sp == ';') {
                sp++;
                com = 2;
                n--;
                continue;
            }
            if (*sp == '{') {
                if (!(sp = e_sh_spl3 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (*sp == ';') {
                    sp++;
                    com = 2;
                    n--;
                    continue;
                }
            }
            while (1) {
                while (*sp == '*') {
                    sp++;
                }
                if (isspace (*sp) && !(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (!strncmp (sp, name, len) && !isalnum1 (sp[len])) {
                    while (isalnum1 (*sp)) {
                        sp++;
                    }
                    if (isspace (*sp) && !(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if ((*sp == ';' || *sp == ',' || *sp == '='
                            || *sp == '[' || *sp == ')')) {
                        if (*first) {
                            e_aclose (fp);
                            strcpy (file, startfile);
                            *num = n;
                            *xn = (int) (sp - str);
                            return (0);
                        } else if (n == nold && !strcmp (startfile, oldfile)) {
                            *first = 1;
                            if (*sp == ';') {
                                sp++;
                                com = 2;
                                n--;
                                break;
                            }
                        }
                    } else if (*sp == '(') {
                        if (!(sp = e_sh_spl5 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        if (*sp == '{') {
                            if (*first) {
                                e_aclose (fp);
                                strcpy (file, startfile);
                                *num = n;
                                *xn = (int) (sp - str);
                                return (0);
                            } else if (n == nold && !strcmp (startfile, oldfile)) {
                                *first = 1;
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                } else if (*sp == '(') {
                    sp++;
                } else if (*sp == '*') {
                    while (*sp == '*') {
                        sp++;
                    }
                    continue;
                } else if (*sp == ';') {
                    sp++;
                    com = 2;
                    n--;
                    break;
                } else if (*sp == '{') {
                    if (!(sp = e_sh_spl3 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    sp++;
                    com = 2;
                    n--;
                    break;
                } else {
                    if (!(sp = e_sh_spl4 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (*sp == '(') {
                        break;
                    } else if (*sp == ';') {
                        sp++;
                        com = 2;
                        n--;
                        break;
                    }
                    if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                }
                if (*sp == ';') {
                    sp++;
                    com = 2;
                    n--;
                    break;
                }
            }
        } else if (isalnum1 (*sp)) {
            while (isalnum1 (*sp)) {
                sp++;
            }
            while (1) {
                while (*sp == '*') {
                    sp++;
                }
                if (isspace (*sp) && !(sp = e_sh_spl2 (sp, str, fp, &n))) {
                    goto b_end;
                }
                if (!strncmp (sp, name, len) && !isalnum1 (sp[len])) {
                    while (isalnum1 (*sp)) {
                        sp++;
                    }
                    if (isspace (*sp) && !(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (*sp == ';' || *sp == ',' || *sp == '='
                            || *sp == '[' || *sp == ')') {
                        if (*first) {
                            e_aclose (fp);
                            strcpy (file, startfile);
                            *num = n;
                            *xn = (int) (sp - str);
                            return (0);
                        } else if (n == nold && !strcmp (startfile, oldfile)) {
                            *first = 1;
                            break;
                        }
                    } else if (*sp == '(') {
                        if (!(sp = e_sh_spl5 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        if (*sp == '{') {
                            if (*first) {
                                e_aclose (fp);
                                strcpy (file, startfile);
                                *num = n;
                                *xn = (int) (sp - str);
                                return (0);
                            } else if (n == nold && !strcmp (startfile, oldfile)) {
                                *first = 1;
                                break;
                            }
                        } else {
                            break;
                        }
                    }
                } else if (*sp == '(') {
                    sp++;
                } else if (*sp == '*') {
                    while (*sp == '*') {
                        sp++;
                    }
                    continue;
                } else if (*sp == ';') {
                    sp++;
                    com = 2;
                    n--;
                    break;
                } else if (*sp == '{') {
                    if (!(sp = e_sh_spl3 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    sp++;
                    com = 2;
                    n--;
                    break;
                } else {
                    if (!(sp = e_sh_spl4 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                    if (*sp == '(') {
                        if (!(sp = e_sh_spl5 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        if (*sp == '{' && !(sp = e_sh_spl3 (sp, str, fp, &n))) {
                            goto b_end;
                        }
                        sp++;
                        com = 2;
                        n--;
                        break;
                    } else if (*sp == ';') {
                        sp++;
                        com = 2;
                        n--;
                        break;
                    }
                    if (!(sp = e_sh_spl2 (sp, str, fp, &n))) {
                        goto b_end;
                    }
                }
                if (*sp == ';') {
                    sp++;
                    com = 2;
                    n--;
                    break;
                }
            }
        }
    }
b_end:
    e_aclose (fp);
    return (ret);
}

int
e_show_nm_f (char *name, we_window_t * window, int oldn, char **oldname)
{
    int i, j, len, ret, num, x, first = oldn < 0 ? 1 : 0;
    char str[128], file[128], *filename;
    struct dirfile *df, *fdf = NULL;

#ifndef TESTSDEF
    if (!access (e_prog.project, F_OK)) {
        WpeMouseChangeShape (WpeWorkingShape);
        if (e_read_var (window)) {
            ret = -1;
        } else {
            df = e_p_get_var ("FILES");
            if (!df) {
                e_error (e_p_msg[ERR_NOTHING], ERROR_MSG, window->colorset);
                WpeMouseRestoreShape ();
                return (-1);
            }
            for (i = 0, ret = 1; i < df->nr_files && ret; i++) {
                strcpy (str, df->name[i]);
                ret = e_find_def (name, str, 7, file, &num, &x, oldn,
                                  *oldname, &fdf, &first);
            }
            freedf (df);
        }
    } else
#endif
    {
        if (!DTMD_ISTEXT (window->dtmd)) {
            return (-1);
        }
        WpeMouseChangeShape (WpeWorkingShape);
        strcpy (str, window->datnam);
        ret = e_find_def (name, str, 7, file, &num, &x, oldn,
                          *oldname, &fdf, &first);
    }
    freedf (fdf);
    if (ret) {
        sprintf (str, "%s not found!", name);
        e_error (str, ERROR_MSG, window->colorset);
        WpeMouseRestoreShape ();
        return (-1);
    }
    if (*oldname) {
        free (*oldname);
    }
    *oldname = malloc ((strlen (file) + 1) * sizeof (char));
    strcpy (*oldname, file);
    for (i = strlen (file) - 1; i >= 0 && file[i] != '/'; i--)
        ;
    for (j = window->edit_control->mxedt; j > 0; j--) {
        if (i < 0) {
            filename = window->edit_control->window[j]->datnam;
        } else {
            filename = e_mkfilename (window->edit_control->window[j]->dirct, window->edit_control->window[j]->datnam);
        }
        if (!strcmp (filename, file)) {
            break;
        }
    }
    if (j > 0) {
        e_switch_window (window->edit_control->edt[j], window->edit_control->window[window->edit_control->mxedt]);
    } else {
        e_edit (window->edit_control, file);
    }
    window = window->edit_control->window[window->edit_control->mxedt];
    for (i = num, j = x + 1 - (len = strlen (name)); i >= 0;) {
        for (len = strlen (name);
                j >= 0 && strncmp (name, (const char *) window->buffer->buflines[i].s + j, len);
                j--)
            ;
        if (j < 0 && i >= 0) {
            i--;
            j = window->buffer->buflines[i].len - len + 1;
        } else {
            break;
        }
    }
    if (i >= 0) {
        num = i;
        x = j;
    } else {
        len = 0;
    }
    window->screen->fa.y = window->screen->fe.y = window->buffer->cursor.y = num;
    window->screen->fe.x = window->buffer->cursor.x = x + len;
    window->screen->fa.x = x;
    e_cursor (window, 1);
    window->screen->fa.y = num;
    e_write_screen (window, 1);
    WpeMouseRestoreShape ();
    return (num);
}

struct {
    int num;
    char *str, *file;
} sh_df = {
    -1, NULL, NULL
};

int
e_sh_def (we_window_t * window)
{
    char str[80];

    if (window->edit_control->shdf && window->edit_control->shdf->nr_files > 0) {
        strcpy (str, window->edit_control->shdf->name[0]);
    } else {
        str[0] = '\0';
    }
    if (e_add_arguments (str, "Show Definition", window, 0, AltB, &window->edit_control->shdf)) {
        if (sh_df.str) {
            free (sh_df.str);
        }
        sh_df.str = malloc ((strlen (str) + 1) * sizeof (char));
        strcpy (sh_df.str, str);
        if (sh_df.file) {
            free (sh_df.file);
            sh_df.file = NULL;
        }
        window->edit_control->shdf = e_add_df (str, window->edit_control->shdf);
        sh_df.num = e_show_nm_f (str, window, -1, &sh_df.file);
    }
    return (0);
}

int
e_sh_nxt_def (we_window_t * window)
{
    if (sh_df.num >= 0 && sh_df.str && sh_df.file) {
        sh_df.num = e_show_nm_f (sh_df.str, window, sh_df.num, &sh_df.file);
    }
    return (0);
}

int
e_nxt_brk (we_window_t * window)
{
    int c = window->buffer->buflines[window->buffer->cursor.y].s[window->buffer->cursor.x];
    int i, j, ob, cb, bsp, brk, nif;

    if (c == '{' || c == '(' || c == '[') {
        if (c == '{') {
            ob = '{';
            cb = '}';
        } else if (c == '(') {
            ob = '(';
            cb = ')';
        } else {
            ob = '[';
            cb = ']';
        }
        for (brk = 1, i = window->buffer->cursor.y; i < window->buffer->mxlines; i++)
            for (j = i == window->buffer->cursor.y ? window->buffer->cursor.x + 1 : 0; j < window->buffer->buflines[i].len; j++) {
                if (window->buffer->buflines[i].s[j] == '\"') {
                    for (bsp = 0, j++;
                            j < window->buffer->buflines[i].len && (window->buffer->buflines[i].s[j] != '\"' || bsp);
                            j++) {
                        if (window->buffer->buflines[i].s[j] == '\\') {
                            bsp = !bsp;
                        } else {
                            bsp = 0;
                        }
                        if (j == window->buffer->buflines[i].len - 1 && bsp
                                && i < window->buffer->mxlines - 1) {
                            i++;
                            j = -1;
                            bsp = 0;
                        }
                    }
                } else if (window->buffer->buflines[i].s[j] == '\'') {
                    for (bsp = 0, j++;
                            j < window->buffer->buflines[i].len && (window->buffer->buflines[i].s[j] != '\'' || bsp);
                            j++) {
                        if (window->buffer->buflines[i].s[j] == '\\') {
                            bsp = !bsp;
                        } else {
                            bsp = 0;
                        }
                        if (j == window->buffer->buflines[i].len - 1 && bsp
                                && i < window->buffer->mxlines - 1) {
                            i++;
                            j = -1;
                            bsp = 0;
                        }
                    }
                } else if (window->buffer->buflines[i].s[j] == '/' && window->buffer->buflines[i].s[j + 1] == '*') {
                    for (j += 2;
                            window->buffer->buflines[i].s[j] != '*' || window->buffer->buflines[i].s[j + 1] != '/';
                            j++) {
                        if (j >= window->buffer->buflines[i].len - 1) {
                            if (i < window->buffer->mxlines - 1) {
                                i++;
                                j = -1;
                            } else {
                                break;
                            }
                        }
                    }
                } else if (window->buffer->buflines[i].s[j] == '/' && window->buffer->buflines[i].s[j + 1] == '/') {
                    break;
                } else if (window->buffer->buflines[i].s[j] == '#'
                           && (!strncmp ((const char *) window->buffer->buflines[i].s, "#else", 5)
                               || !strncmp ((const char *) window->buffer->buflines[i].s, "#elif",
                                            5))) {
                    for (nif = 1, i++; i < window->buffer->mxlines - 1; i++) {
                        for (j = 0; isspace (window->buffer->buflines[i].s[j]); j++)
                            ;
                        if (!strncmp
                                ((const char *) window->buffer->buflines[i].s + j, "#endif", 6)) {
                            nif--;
                        } else if (!strncmp
                                   ((const char *) window->buffer->buflines[i].s + j, "#if", 3)) {
                            nif++;
                        }
                        if (!nif) {
                            break;
                        }
                    }
                    continue;
                } else if (window->buffer->buflines[i].s[j] == ob) {
                    brk++;
                } else if (window->buffer->buflines[i].s[j] == cb) {
                    brk--;
                    if (!brk) {
                        window->buffer->cursor.y = i;
                        window->buffer->cursor.x = j;
                        return (0);
                    }
                }
            }
    } else {
        if (c == '}') {
            ob = '{';
            cb = '}';
        } else if (c == ')') {
            ob = '(';
            cb = ')';
        } else if (c == ']') {
            ob = '[';
            cb = ']';
        } else {
            ob = 0;
            cb = 0;
        }
        for (brk = -1, i = window->buffer->cursor.y; i >= 0; i--) {
            if (i == window->buffer->cursor.y)
                for (j = 0;
                        j < window->buffer->cursor.x && (window->buffer->buflines[i].s[j] != '/'
                                || window->buffer->buflines[i].s[j + 1] != '/'); j++)
                    ;
            else
                for (j = 0;
                        j < window->buffer->buflines[i].len && (window->buffer->buflines[i].s[j] != '/'
                                || window->buffer->buflines[i].s[j + 1] != '/'); j++)
                    ;
            for (j--; j >= 0; j--) {
                if (window->buffer->buflines[i].s[j] == '\"') {
                    for (j--; j >= 0; j--) {
                        if (window->buffer->buflines[i].s[j] == '\"') {
                            for (bsp = 0, j--;
                                    j >= 0 && window->buffer->buflines[i].s[j] == '\\'; j--) {
                                bsp = !bsp;
                            }
                            j++;
                            if (!bsp) {
                                break;
                            }
                        }
                        if (j == 0 && i > 0
                                && window->buffer->buflines[i - 1].s[window->buffer->buflines[i - 1].len] == '\\') {
                            i--;
                            j = window->buffer->buflines[i].len;
                        }
                    }
                } else if (window->buffer->buflines[i].s[j] == '\'') {
                    for (j--; j >= 0; j--) {
                        if (window->buffer->buflines[i].s[j] == '\'') {
                            for (bsp = 0, j--;
                                    j >= 0 && window->buffer->buflines[i].s[j] == '\\'; j--) {
                                bsp = !bsp;
                            }
                            j++;
                            if (!bsp) {
                                break;
                            }
                        }
                        if (j == 0 && i > 0
                                && window->buffer->buflines[i - 1].s[window->buffer->buflines[i - 1].len] == '\\') {
                            i--;
                            j = window->buffer->buflines[i].len;
                        }
                    }
                } else if (window->buffer->buflines[i].s[j] == '/' && window->buffer->buflines[i].s[j - 1] == '*') {
                    for (j -= 2;
                            window->buffer->buflines[i].s[j] != '*' || window->buffer->buflines[i].s[j - 1] != '/';
                            j--) {
                        if (j <= 0) {
                            if (i > 0) {
                                i--;
                                j = window->buffer->buflines[i].len;
                            } else {
                                break;
                            }
                        }
                    }
                } else if (window->buffer->buflines[i].s[j] == '#'
                           && (!strncmp ((const char *) window->buffer->buflines[i].s, "#else", 5)
                               || !strncmp ((const char *) window->buffer->buflines[i].s, "#elif",
                                            5))) {
                    for (nif = 1, i--; i > 0; i--) {
                        for (j = 0; isspace (window->buffer->buflines[i].s[j]); j++)
                            ;
                        if (!strncmp
                                ((const char *) window->buffer->buflines[i].s + j, "#endif", 6)) {
                            nif++;
                        } else if (!strncmp
                                   ((const char *) window->buffer->buflines[i].s + j, "#if", 3)) {
                            nif--;
                        }
                        if (!nif) {
                            break;
                        }
                    }
                    continue;
                } else if (!ob) {
                    if (window->buffer->buflines[i].s[j] == '{' || window->buffer->buflines[i].s[j] == '('
                            || window->buffer->buflines[i].s[j] == '[') {
                        brk++;
                        if (!brk) {
                            window->buffer->cursor.y = i;
                            window->buffer->cursor.x = j;
                            return (0);
                        }
                    } else if (window->buffer->buflines[i].s[j] == '}' || window->buffer->buflines[i].s[j] == ')'
                               || window->buffer->buflines[i].s[j] == ']') {
                        brk--;
                    } else if (i == 0 && j == 0) {
                        window->buffer->cursor.y = i;
                        window->buffer->cursor.x = j;
                        return (0);
                    }
                } else {
                    if (window->buffer->buflines[i].s[j] == ob) {
                        brk++;
                        if (!brk) {
                            window->buffer->cursor.y = i;
                            window->buffer->cursor.x = j;
                            return (0);
                        }
                    } else if (window->buffer->buflines[i].s[j] == cb) {
                        brk--;
                    }
                }
            }
        }
    }
    return (e_error ("No Matching Bracket!", ERROR_MSG, window->edit_control->colorset));
}

char *
e_mbt_mk_sp (char *str, int n, int sw, int *m)
{
    int k;

    if (!sw) {
        *m = n;
    } else {
        *m = n / sw + n % sw;
    }
    str = realloc (str, (*m + 1) * sizeof (char));
    if (!sw) {
        k = 0;
    } else
        for (k = 0; k < n / sw; k++) {
            str[k] = '\t';
        }
    for (; k < *m; k++) {
        str[k] = ' ';
    }
    str[*m] = '\0';
    return (str);
}

int
e_mbt_str (we_buffer_t * buffer, int *ii, int *jj, unsigned char c, int n, int sw,
           int *cmnd)
{
    int i = *ii, j = *jj + 1, bsp;

    if (*cmnd != 2) {
        *cmnd = 0;
    }
    for (bsp = 0; j < buffer->buflines[i].len && (buffer->buflines[i].s[j] != c || bsp); j++) {
        if (buffer->buflines[i].s[j] == '\\') {
            bsp = !bsp;
        } else {
            bsp = 0;
        }
        if (j == buffer->buflines[i].len - 1 && bsp && i < buffer->mxlines - 1) {
            char *str = malloc (1);
            int m;

            i++;
            bsp = 0;
            for (j = 0, m = buffer->buflines[i].len; j < m && isspace (buffer->buflines[i].s[j]); j++)
                ;
            if (j > 0) {
                e_del_nchar (buffer, buffer->window->screen, 0, i, j);
            }
            if (j < m) {
                str = e_mbt_mk_sp (str, n + buffer->control->autoindent, sw, &m);
                e_ins_nchar (buffer, buffer->window->screen, (unsigned char *) str, 0, i, m);
            }
            j = -1;
            free (str);
        }
    }
    *ii = i;
    *jj = j;
    return (0);
}

int
e_mbt_cnd (we_buffer_t * buffer, int *ii, int *jj, int n, int sw, int *cmnd)
{
    int i = *ii, j = *jj + 2;

    for (; buffer->buflines[i].s[j] != '*' || buffer->buflines[i].s[j + 1] != '/'; j++) {
        if (j >= buffer->buflines[i].len - 1) {
            if (i < buffer->mxlines - 1) {
                char *str = malloc (1);
                int m;

                i++;
                for (j = 0, m = buffer->buflines[i].len; j < m && isspace (buffer->buflines[i].s[j]);
                        j++)
                    ;
                if (j > 0) {
                    e_del_nchar (buffer, buffer->window->screen, 0, i, j);
                }
                if (j < m && (buffer->buflines[i].s[0] != '*' || buffer->buflines[i].s[1] != '/')) {
                    str = e_mbt_mk_sp (str, n + buffer->control->autoindent, sw, &m);
                    e_ins_nchar (buffer, buffer->window->screen, (unsigned char *) str, 0, i, m);
                }
                j = -1;
                free (str);
                if (*cmnd == 2) {
                    *cmnd = 1;
                }
            } else {
                break;
            }
        }
    }
    *ii = i;
    *jj = j + 1;
    return (0);
}

int
e_mk_beauty (int sw, int ndif, we_window_t * window)
{
    we_buffer_t *buffer;
    we_screen_t *s;
    int bg, nd, m, n, i, j, k, brk, cbrk = 0, nif = 0, nic = 0;
    int nstrct = 0, cmnd, cm_sv;
    char *tstr = malloc (sizeof (char));
    char *bstr = malloc ((ndif + 1) * sizeof (char));
    int *nvek = malloc (sizeof (int));
    int *ifvekb = malloc (sizeof (int));
    int *ifvekr = malloc (sizeof (int));
    int *vkcs = malloc (sizeof (int));
    int *vkcb = malloc (sizeof (int));
    we_point_t sa, se, sb;

    for (i = window->edit_control->mxedt; i > 0 && !DTMD_ISTEXT (window->edit_control->window[i]->dtmd); i--)
        ;
    if (i <= 0) {
        free (tstr);
        free (bstr);
        free (nvek);
        free (ifvekb);
        free (ifvekr);
        free (vkcs);
        free (vkcb);
        return (0);
    }
    *ifvekb = 0;
    *ifvekr = 0;
    *vkcs = 0;
    *vkcb = 0;
    e_switch_window (window->edit_control->edt[i], window);
    WpeMouseChangeShape (WpeWorkingShape);
    window = window->edit_control->window[window->edit_control->mxedt];
    buffer = window->buffer;
    s = window->screen;
    sa = s->mark_begin;
    se = s->mark_end;
    sb = buffer->cursor;
    if (sw & 1) {
        if (sw & 2) {
            bg = i = buffer->cursor.x == 0 ? buffer->cursor.y : buffer->cursor.y + 1;
        } else {
            bg = i = s->mark_begin.x == 0 ? s->mark_begin.y : s->mark_begin.y + 1;
        }
        nd = s->mark_end.x == 0 ? s->mark_end.y : s->mark_end.y + 1;
        if (nd > buffer->mxlines) {
            nd = buffer->mxlines;
        }
    } else {
        if (sw & 2) {
            bg = i = buffer->cursor.x == 0 ? buffer->cursor.y : buffer->cursor.y + 1;
        } else {
            bg = i = 0;
        }
        nd = buffer->mxlines;
    }
    if (s->mark_begin.y < 0 || (s->mark_begin.y == 0 && s->mark_begin.x <= 0)) {
        n = 0;
    } else {
        for (n = j = 0; j < buffer->buflines[i].len && isspace (buffer->buflines[i].s[j]); j++) {
            if (buffer->buflines[i].s[j] == ' ') {
                n++;
            } else if (buffer->buflines[i].s[j] == '\t') {
                n += window->edit_control->tabn - (n % window->edit_control->tabn);
            }
        }
    }
    tstr = e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
    for (k = 0; k < ndif; k++) {
        bstr[k] = ' ';
    }
    bstr[ndif] = '\0';
    nvek[0] = n;
    for (cm_sv = 1, cmnd = 1, brk = 0; i < nd; i++) {
        for (j = 0; j < buffer->buflines[i].len && isspace (buffer->buflines[i].s[j]); j++)
            ;
        if (i > bg) {
            for (k = buffer->buflines[i - 1].len - 1;
                    k >= 0 && isspace (buffer->buflines[i - 1].s[k]); k--);
            if (k >= 0) {
                e_del_nchar (buffer, s, k + 1, i - 1, buffer->buflines[i - 1].len - 1 - k);
            }
            e_del_nchar (buffer, s, 0, i, j);
            if (buffer->buflines[i].len > 0 && buffer->buflines[i].s[0] != '#' &&
                    (buffer->buflines[i].s[0] != '/' || buffer->buflines[i].s[1] != '*')) {
                if ((cmnd == 0 && (!cm_sv || buffer->buflines[i].s[0] != '{')) ||
                        (cmnd == 2 && buffer->buflines[i - 1].s[k] == '\\')) {
                    tstr =
                        e_mbt_mk_sp (tstr,
                                     !cmnd ? n +
                                     buffer->control->autoindent : buffer->control->autoindent,
                                     (sw & 4) ? 0 : window->edit_control->tabn, &m);
                    e_ins_nchar (buffer, s, (unsigned char *) tstr, 0, i, m);
                    j = m;
                    tstr =
                        e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
                } else {
                    e_ins_nchar (buffer, s, (unsigned char *) tstr, 0, i, m);
                    j = m;
                    if (cmnd == 0) {
                        cmnd = 1;
                    }
                }
            }
        }
        if (cmnd == 0) {
            cm_sv = cbrk ? 1 : 0;
        } else {
            cm_sv = cmnd;
        }
        for (cmnd = 1; j < buffer->buflines[i].len; j++) {
            if (buffer->buflines[i].s[j] == '\"') {
                e_mbt_str (buffer, &i, &j, '\"', n, (sw & 4) ? 0 : window->edit_control->tabn, &cmnd);
            } else if (window->buffer->buflines[i].s[j] == '\'') {
                e_mbt_str (buffer, &i, &j, '\'', n, (sw & 4) ? 0 : window->edit_control->tabn, &cmnd);
            } else if (buffer->buflines[i].s[j] == '/' && buffer->buflines[i].s[j + 1] == '*') {
                e_mbt_cnd (buffer, &i, &j, n, (sw & 4) ? 0 : window->edit_control->tabn, &cmnd);
            } else if (buffer->buflines[i].s[j] == '/' && buffer->buflines[i].s[j + 1] == '/') {
                break;
            } else if (buffer->buflines[i].s[j] == ';') {
                cmnd = cbrk ? 0 : 1;
                nstrct = 0;
            } else if (buffer->buflines[i].s[j] == '#' &&
                       (!strncmp ((const char *) buffer->buflines[i].s, "#if", 3)
                        || !strncmp ((const char *) buffer->buflines[i].s, "#ifdef", 6))) {
                nif++;
                ifvekb = realloc (ifvekb, (nif + 1) * sizeof (int));
                ifvekr = realloc (ifvekr, (nif + 1) * sizeof (int));
                ifvekb[nif] = brk;
                ifvekr[nif] = cbrk;
                cmnd = 2;
            } else if (buffer->buflines[i].s[j] == '#' && nif > 0
                       && (!strncmp ((const char *) buffer->buflines[i].s, "#else", 5)
                           || !strncmp ((const char *) buffer->buflines[i].s, "#elif", 5))) {
                brk = ifvekb[nif];
                cbrk = ifvekr[nif];
                n = nvek[brk];
                tstr = e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
                cmnd = 2;
            } else if (buffer->buflines[i].s[j] == '#'
                       && (!strncmp ((const char *) buffer->buflines[i].s, "#endif", 6))) {
                nif--;
                cmnd = 2;
            } else if (buffer->buflines[i].s[j] == '#') {
                cmnd = 2;
                for (j++; j < buffer->buflines[i].len; j++) {
                    if (j == buffer->buflines[i].len - 1 && i < buffer->mxlines - 1
                            && buffer->buflines[i].s[j] == '\\') {
                        i++;
                        j = 0;
                    } else if (buffer->buflines[i].s[j] == '\"')
                        e_mbt_str (buffer, &i, &j, '\"', n, (sw & 4) ? 0 : window->edit_control->tabn,
                                   &cmnd);
                    else if (window->buffer->buflines[i].s[j] == '\'')
                        e_mbt_str (buffer, &i, &j, '\'', n, (sw & 4) ? 0 : window->edit_control->tabn,
                                   &cmnd);
                    else if (buffer->buflines[i].s[j] == '/' && buffer->buflines[i].s[j + 1] == '*')
                        e_mbt_cnd (buffer, &i, &j, n, (sw & 4) ? 0 : window->edit_control->tabn,
                                   &cmnd);
                    else if (buffer->buflines[i].s[j] == '/' && buffer->buflines[i].s[j + 1] == '/') {
                        break;
                    }
                }
                if (buffer->buflines[i].s[j] == '/' && buffer->buflines[i].s[j + 1] == '/') {
                    break;
                }
            } else if (buffer->buflines[i].s[j] == '(') {
                nstrct = 0;
                cmnd = 0;
                cbrk++;
            } else if (buffer->buflines[i].s[j] == ')') {
                nstrct = 0;
                cmnd = 0;
                cbrk--;
            } else if (buffer->buflines[i].s[j] == '{') {
                brk++;
                cmnd = 1;
                for (k = j + 1; k < buffer->buflines[i].len && isspace (buffer->buflines[i].s[k]);
                        k++);
                if (k < buffer->buflines[i].len
                        && (buffer->buflines[i].s[k] != '/'
                            || (buffer->buflines[i].s[k + 1] != '*'
                                && buffer->buflines[i].s[k + 1] != '/'))) {
                    e_del_nchar (buffer, s, j + 1, i, k - j - 1);
                    e_ins_nchar (buffer, s, (unsigned char *) bstr, j + 1, i,
                                 ndif - 1);
                }
                if (nstrct == 1) {
                    if (k >= buffer->buflines[i].len) {
                        n = nvek[brk - 1];
                    } else {
                        for (k = j - 1; k >= 0 && isspace (buffer->buflines[i].s[k]); k--);
                        if (k >= 0) {
                            nstrct++;
                        }
                    }
                    nstrct++;
                    nic++;
                    vkcb = realloc (vkcb, (nic + 1) * sizeof (int));
                    vkcs = realloc (vkcs, (nic + 1) * sizeof (int));
                    vkcs[nic] = 0;
                    vkcb[nic] = brk - 1;
                } else {
                    for (n = k = 0; k < j; k++) {
                        if (buffer->buflines[i].s[k] == '\t') {
                            n += window->edit_control->tabn - (n % window->edit_control->tabn);
                        } else {
                            n++;
                        }
                    }
                }
                n += ndif;
                tstr = e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
                nvek = realloc (nvek, (brk + 1) * sizeof (int));
                nvek[brk] = n;
            } else if (window->buffer->buflines[i].s[j] == '}') {
                brk--;
                cmnd = 1;
                nstrct = 0;
                if (nic > 0 && vkcb[nic] == brk - 1) {
                    n = nvek[brk];
                    nic--;
                    brk--;
                }
                if (brk < 0) {
                    free (tstr);
                    free (bstr);
                    free (nvek);
                    free (ifvekb);
                    free (ifvekr);
                    free (vkcs);
                    free (vkcb);
                    buffer->cursor = sb;
                    s->mark_begin = sa;
                    s->mark_end = se;
                    e_write_screen (window, 1);
                    WpeMouseRestoreShape ();
                    return (0);
                }
                n -= ndif;
                tstr = e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
                for (k = j - 1; k >= 0 && isspace (buffer->buflines[i].s[k]); k--);
                e_del_nchar (buffer, s, k + 1, i, j - 1 - k);
                if (k > 0) {
                    e_ins_nchar (buffer, s, (unsigned char *) bstr, k + 1, i,
                                 ndif - 1);
                    j = k + ndif + 1;
                } else {
                    e_ins_nchar (buffer, s, (unsigned char *) tstr, k + 1, i, m);
                    j = k + m + 2;
                }
                n = nvek[brk];
                tstr = e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
            } else if (((j == 0 || !isalnum1 (buffer->buflines[i].s[j - 1])) &&
                        (iscase ((const char *) buffer->buflines[i].s + j)
                         || isstatus ((const char *) buffer->buflines[i].s + j)))
                       || (nstrct > 1 && isalnum1 (buffer->buflines[i].s[j]))) {
                if (vkcs[nic]) {
                    n = nvek[vkcb[nic] + 1];
                    tstr =
                        e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
                    for (k = j - 1; k >= 0 && isspace (buffer->buflines[i].s[k]); k--);
                    e_del_nchar (buffer, s, k + 1, i, j - 1 - k);
                    if (k > 0) {
                        e_ins_nchar (buffer, s, (unsigned char *) bstr, k + 1, i,
                                     ndif - 1);
                        j = k + ndif + 1;
                    } else {
                        e_ins_nchar (buffer, s, (unsigned char *) tstr, k + 1, i, m);
                        j = k + m + 2;
                    }
                } else {
                    brk++;
                    vkcs[nic] = 1;
                }
                if (nstrct < 2 || isstatus ((const char *) buffer->buflines[i].s + j)) {
                    for (j++; j < buffer->buflines[i].len && buffer->buflines[i].s[j] != ':'; j++);
                    for (k = j + 1; k < buffer->buflines[i].len && isspace (buffer->buflines[i].s[k]);
                            k++);
                    if (k < buffer->buflines[i].len
                            && (buffer->buflines[i].s[k] != '/'
                                || (buffer->buflines[i].s[k + 1] != '*'
                                    && buffer->buflines[i].s[k + 1] != '/'))) {
                        e_del_nchar (buffer, s, j + 1, i, k - j - 1);
                        e_ins_nchar (buffer, s, (unsigned char *) bstr, j + 1, i,
                                     ndif - 1);
                        for (n = k = 0; k < j; k++) {
                            if (buffer->buflines[i].s[k] == '\t') {
                                n += window->edit_control->tabn - (n % window->edit_control->tabn);
                            } else {
                                n++;
                            }
                        }
                    }
                } else if (nstrct == 2) {
                    e_ins_nchar (buffer, s, (unsigned char *) bstr, j, i, ndif);
                } else {
                    j--;
                }
                n += ndif;
                tstr = e_mbt_mk_sp (tstr, n, (sw & 4) ? 0 : window->edit_control->tabn, &m);
                nvek = realloc (nvek, (brk + 1) * sizeof (int));
                nvek[brk] = n;
                cmnd = 3;
                nstrct = 0;
            } else if ((j == 0 || !isalnum1 (buffer->buflines[i].s[j - 1])) &&
                       (!strncmp ((const char *) buffer->buflines[i].s + j, "switch", 6)
                        && !isalnum1 (buffer->buflines[i].s[j + 6]))) {
                nic++;
                vkcb = realloc (vkcb, (nic + 1) * sizeof (int));
                vkcs = realloc (vkcs, (nic + 1) * sizeof (int));
                vkcs[nic] = 0;
                vkcb[nic] = brk;
            } else if ((j == 0 || !isalnum1 (buffer->buflines[i].s[j - 1])) &&
                       ((!strncmp ((const char *) buffer->buflines[i].s + j, "class", 5)
                         && !isalnum1 (buffer->buflines[i].s[j + 5]))
                        || (!strncmp ((const char *) buffer->buflines[i].s + j, "struct", 6)
                            && !isalnum1 (buffer->buflines[i].s[j + 6])))) {
                nstrct = 1;
            } else if (cmnd == 1 && !isspace (window->buffer->buflines[i].s[j])) {
                cmnd = 0;
            } else if (cmnd == 3 && window->buffer->buflines[i].s[j] == ':') {
                cmnd = 1;
            }
        }
    }
    free (nvek);
    free (tstr);
    free (bstr);
    free (ifvekb);
    free (ifvekr);
    free (vkcs);
    free (vkcb);
    s->mark_begin = sa;
    s->mark_end = se;
    buffer->cursor = sb;
    e_write_screen (window, 1);
    WpeMouseRestoreShape ();
    return (0);
}

int
e_p_beautify (we_window_t * window)
{
    static int b_sw = 0;
    int ret;
    W_OPTSTR *o = e_init_opt_kst (window);

    if (!o) {
        return (-1);
    }
    o->xa = 21;
    o->ya = 3;
    o->xe = 52;
    o->ye = 19;
    o->bgsw = AltO;
    o->name = "Beautify";
    o->crsw = AltO;
    e_add_txtstr (4, 2, "Begin:", o);
    e_add_txtstr (4, 6, "Scope:", o);
    e_add_numstr (4, 12, 26, 12, 2, 100, 0, AltN, "Number of Columns:",
                  window->edit_control->autoindent, o);
    e_add_sswstr (5, 10, 3, AltT, (b_sw & 4) ? 1 : 0, "No Tabulators     ", o);
    e_add_pswstr (0, 5, 3, 0, AltG, 0, "Global            ", o);
    e_add_pswstr (0, 5, 4, 0, AltS, b_sw & 1, "Selected Text     ", o);
    e_add_pswstr (1, 5, 7, 0, AltE, 0, "Entire Scope      ", o);
    e_add_pswstr (1, 5, 8, 0, AltF, (b_sw & 2) ? 1 : 0, "From Cursor       ",
                  o);
    e_add_bttstr (7, 14, 1, AltO, " Ok ", NULL, o);
    e_add_bttstr (18, 14, -1, WPE_ESC, "Cancel", NULL, o);
    ret = e_opt_kst (o);
    if (ret != WPE_ESC) {
        b_sw =
            o->pstr[0]->num + (o->pstr[1]->num << 1) + (o->sstr[0]->num << 2);
        window->edit_control->autoindent = o->nstr[0]->num;
        e_mk_beauty (b_sw, window->edit_control->autoindent, window);
    }
    freeostr (o);
    return (0);
}

#endif
