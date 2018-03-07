/** \file we_term.c                                        */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"
#include <string.h>
#include <errno.h>
#include "keys.h"
#include "model.h"
#include "edit.h"
#include "we_screen.h"
#include "we_term.h"
#include "we_unix.h"
#include "we_gpm.h"
#include "we_hfkt.h"
#include "we_gpm.h"
#include "WeLinux.h"

#ifdef UNIX

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
#include <curses.h>
#endif

#include<signal.h>
#define KEYFN 42

#ifndef XWPE_DLL
#define WpeDllInit WpeTermInit
#endif

char *init_kkey (char *key);
void e_endwin (void);
int fk_t_cursor (int x);
int fk_t_putchar (int c);
int fk_attrset (int a);
int e_t_refresh (void);
int e_t_sys_ini (void);
int e_t_sys_end (void);
int e_t_getch (void);
int e_find_key (int c, int j, int sw);
int fk_t_locate (int x, int y);
int fk_t_mouse (int *g);
int e_t_initscr (void);
int e_t_kbhit (void);
int e_t_d_switch_out (int sw);
int e_t_switch_screen (int sw);
int e_t_deb_out (we_window_t * window);
int e_s_sys_end ();
int e_s_sys_ini ();

/** global field also used in we_xterm.c */
int cur_x = -1;
/** global field also used in we_xterm.c */
int cur_y = -1;
/** global field also used in we_xterm.c */
char *ctree[5];

#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
static char *key_f[KEYFN], *key_key;
#endif

static char *spc_st;

extern struct termios otermio;
extern struct termios ntermio; //s
extern struct termios ttermio;

#if !defined HAVE_LIBNCURSES && !defined HAVE_LIBCURSES
char area[315];
char *ap = area;
char tcbuf[1024];
char *tc_screen;
char *tgetstr ();
int tgetnum ();
char *tgoto ();
#define tigetstr(k) tgetstr(k, &ap)

//#define term_move(x,y) e_putp(tgoto(cur_rc, x, y)) --> transformed to function
//#define term_refresh() fflush(stdout) --> transformed to function
#else
/* Because term.h defines "buttons" it messes up we_gpm.c if in edit.h */
#include <term.h>

/* AIX requires that tparm has 10 arguments. */
#define tparm1(aa,bb) tparm((aa), (bb), 0, 0, 0, 0, 0, 0, 0, 0)
#define tparm2(aa,bb,cc) tparm((aa), (bb), (cc), 0, 0, 0, 0, 0, 0, 0)

//#define term_move(x,y) move(y, x) --> transformed to function
//#define term_refresh() refresh() --> transformed to function
#endif

int
WpeDllInit (int *argc, char **argv)
{
    UNUSED (argc);
    UNUSED (argv);
    fk_u_cursor = fk_t_cursor;
    fk_u_locate = fk_t_locate;
    e_u_d_switch_out = e_t_d_switch_out;
    e_u_switch_screen = e_t_switch_screen;
    e_u_deb_out = e_t_deb_out;
    e_u_kbhit = e_t_kbhit;
    e_u_s_sys_end = e_s_sys_end;
    e_u_s_sys_ini = e_s_sys_ini;
    e_u_refresh = e_t_refresh;
    e_u_getch = e_t_getch;
    e_u_sys_ini = e_t_sys_ini;
    e_u_sys_end = e_t_sys_end;
    e_u_system = system;
    fk_u_putchar = fk_t_putchar;
#ifdef HAVE_LIBGPM
    if (WpeGpmMouseInit () == 0) {
        fk_mouse = WpeGpmMouse;
    } else
#endif
        fk_mouse = fk_t_mouse;
    WpeMouseChangeShape = (void (*)(WpeMouseShape)) WpeNullFunction;
    WpeMouseRestoreShape = WpeNullFunction;
    WpeDisplayEnd = e_endwin;
#ifdef __linux__
    u_bioskey = WpeLinuxBioskey;
#else
    u_bioskey = WpeZeroFunction;
#endif
    e_t_initscr ();
    if (is_x_term()) {
        e_pr_u_colorsets = e_pr_x_colorsets;
        e_frb_u_menue = e_frb_x_menue;
        e_s_u_clr = e_s_x_clr;
        e_n_u_clr = e_n_x_clr;
    } else {
        e_pr_u_colorsets = e_pr_t_colorsets;
        e_frb_u_menue = e_frb_t_menue;
        e_s_u_clr = e_s_t_clr;
        e_n_u_clr = e_n_t_clr;
    }
    MCI = '+';
    MCA = '0';
    RD1 = '\01';
    RD2 = '\02';
    RD3 = '\03';
    RD4 = '\04';
    RD5 = '\05';
    RD6 = '\06';
    RE1 = '\01';
    RE2 = '\02';
    RE3 = '\03';
    RE4 = '\04';
    RE5 = '\05';
    RE6 = '\06';
    WBT = '\13';
    ctree[0] = "\11\12\07";	/*  07 -> 30  */
    ctree[1] = "\11\12\12";	/*  11 -> 16  */
    ctree[2] = "\11\07\12";	/*  12 -> 22  */
    ctree[3] = "\10\12\12";	/*  10 -> 25  */
    ctree[4] = "\11\12\12";
    /*
     RD1 = '+';
     RD2 = '+';
     RD3 = '+';
     RD4 = '+';
     RD5 = '-';
       RD6 = '|';
    *//*
RE1 = '.';
RE2 = '.';
RE3 = '.';
RE4 = '.';
RE5 = '.';
RE6 = ':';
WBT = '#';
ctree[0] = "|__";
ctree[1] = "|__";
ctree[2] = "|__";
ctree[3] = "|__";
ctree[4] = "|__";
*/
    return 0;
}

#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
char *
init_kkey (char *key)
{
    char *tmp;
    int i;

    tmp = init_key (key);
    if (tmp == NULL) {
        return (NULL);
    }
    if (!key_key) {
        key_key = malloc (2);
        key_key[0] = tmp[1];
        key_key[1] = '\0';
        return (tmp);
    } else {
        for (i = 0; key_key[i] != '\0'; i++)
            if (key_key[i] == tmp[1]) {
                return (tmp);
            }
        key_key = realloc (key_key, i + 2);
        key_key[i] = tmp[1];
        key_key[i + 1] = '\0';
    }
    return (tmp);
}
#endif

char *
init_spchr (char c)
{
    int i;
    char *pt = NULL;

    if (!graphics_charset_pairs() || !start_alt_charset() || !end_alt_charset()) {
        return (NULL);
    }
    for (i = 0; spc_st[i] && spc_st[i + 1] && spc_st[i] != c; i += 2)
        ;
    if (spc_st[i] && spc_st[i + 1]) {
        pt = malloc ((strlen (start_alt_charset()) + strlen (end_alt_charset()) + 2)
                     * sizeof (char));
        if (pt) {
            sprintf (pt, "%s%c%s", start_alt_charset(), spc_st[i + 1], end_alt_charset());
        }
    }
    return (pt);
}

int svflgs;

#define fk_putp(p) ( p ? e_putp(p) : e_putp(attr_normal()) )

int
fk_t_cursor (int x)
{
    return (x);
}

int
fk_t_putchar (int c)
{
//#ifdef NCURSES
#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
    addch (c);
    return c;
#else
    return (fputc (c, stdout));
#endif
}

int
e_t_refresh ()
{
    int x = cur_x, y = cur_y, i, j, c;
    fk_t_cursor (0);
    for (i = 0; i < max_screen_lines(); i++)
        for (j = 0; j < max_screen_cols(); j++) {
            if (i == max_screen_lines() - 1 && j == max_screen_cols() - 1) {
                break;
            }
            if (*(global_screen + 2 * max_screen_cols() * i + 2 * j) !=
                    *(global_alt_screen + 2 * max_screen_cols() * i + 2 * j)
                    || *(global_screen + 2 * max_screen_cols() * i + 2 * j + 1) !=
                    *(global_alt_screen + 2 * max_screen_cols() * i + 2 * j + 1)) {
                if (cur_x != j || cur_y != i) {
                    term_move (j, i);
                }
                if (cur_x < max_screen_cols()) {
                    cur_x = j + 1;
                    cur_y = i;
                } else {
                    cur_x = 0;
                    cur_y = i + 1;
                }
                if (col_num <= 0) {
                    fk_attrset (e_get_col (j, i));
                } else {
                    fk_colset (e_get_col (j, i));
                }
                c = e_gt_char (j, i);
                print_char(c);
                *(global_alt_screen + 2 * max_screen_cols() * i + 2 * j) =
                    *(global_screen + 2 * max_screen_cols() * i + 2 * j);
                *(global_alt_screen + 2 * max_screen_cols() * i + 2 * j + 1) =
                    *(global_screen + 2 * max_screen_cols() * i + 2 * j + 1);
            }
        }
    fk_t_cursor (1);
    fk_t_locate (x, y);
    term_refresh ();
    return (0);
}

/**
 * Terminal initialization.
 *
 * \todo Why does this call e_endwin()? That is ncurses way of closing a window!
 */

int
e_t_sys_ini ()
{
    e_t_refresh ();
    tcgetattr (STDIN_FILENO, &ttermio);
    svflgs = fcntl (STDIN_FILENO, F_GETFL, 0);
    e_endwin ();
    return (0);
}

int
e_t_sys_end ()
{
    tcsetattr (STDIN_FILENO, TCSADRAIN, &ttermio);
    fcntl (STDIN_FILENO, F_SETFL, svflgs);
    e_abs_refr ();
    fk_t_locate (0, 0);
    return (0);
}

int
e_t_kbhit ()
{
    int ret;
    char kbdflgs, c;

    e_t_refresh ();
    kbdflgs = fcntl (STDIN_FILENO, F_GETFL, 0);
    fcntl (STDIN_FILENO, F_SETFL, kbdflgs | O_NONBLOCK);
    ret = read (0, &c, 1);
    fcntl (STDIN_FILENO, F_SETFL, kbdflgs & ~O_NONBLOCK);
    return (ret == 1 ? c : 0);
}

#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
int
e_t_getch ()
{
    int c, bk;

    e_t_refresh ();
    c = fk_getch ();
    if (c > KEY_CODE_YES) {
        switch (c) {
        case KEY_F (1):
            c = F1;
            break;
        case KEY_F (2):
            c = F2;
            break;
        case KEY_F (3):
            c = F3;
            break;
        case KEY_F (4):
            c = F4;
            break;
        case KEY_F (5):
            c = F5;
            break;
        case KEY_F (6):
            c = F6;
            break;
        case KEY_F (7):
            c = F7;
            break;
        case KEY_F (8):
            c = F8;
            break;
        case KEY_F (9):
            c = F9;
            break;
        case KEY_F (10):
            c = F10;
            break;
        case KEY_UP:
            c = CUP;
            break;
        case KEY_DOWN:
            c = CDO;
            break;
        case KEY_LEFT:
            c = CLE;
            break;
        case KEY_RIGHT:
            c = CRI;
            break;
        case KEY_SFIND:
            c = EINFG;
            break;
        case KEY_IC:
            c = POS1;
            break;
        case KEY_DC:
            c = BUP;
            break;
        case KEY_PPAGE:
            c = ENDE;
            break;
        case KEY_NPAGE:
            c = BDO;
            break;
        case KEY_BACKSPACE:
            c = WPE_DC;
            break;
        case KEY_HELP:
            c = HELP;
            break;
        case KEY_LL:
            c = ENDE;
            break;
        case KEY_F (17):
            c = SF1;
            break;
        case KEY_F (18):
            c = SF2;
            break;
        case KEY_F (19):
            c = SF3;
            break;
        case KEY_F (20):
            c = SF4;
            break;
        case KEY_F (21):
            c = SF5;
            break;
        case KEY_F (22):
            c = SF6;
            break;
        case KEY_F (23):
            c = SF7;
            break;
        case KEY_F (24):
            c = SF8;
            break;
        case KEY_F (25):
            c = SF9;
            break;
        case KEY_F (26):
            c = SF10;
            break;
        case KEY_PREVIOUS:
            c = CUP + 512;
            break;
        case KEY_NEXT:
            c = CDO + 512;
            break;
        case KEY_HOME:
            c = POS1 + 512;
            break;
        case KEY_END:
            c = ENDE + 512;
            break;
        default:
            c = 0;
            break;
        }
        bk = u_bioskey ();
        if (bk & 3) {
            c += 512;
        } else if (bk & 4) {
            c += 514;
        }
    } else if (c == WPE_TAB) {
        bk = u_bioskey ();
        if (bk & 3) {
            c = WPE_BTAB;
        } else {
            c = WPE_TAB;
        }
    } else if (c == WPE_ESC) {
        c = fk_getch ();
        if (c > KEY_CODE_YES) {
            switch (c) {
            case KEY_F (1):
                c = AF1;
                break;
            case KEY_F (2):
                c = AF2;
                break;
            case KEY_F (3):
                c = AF3;
                break;
            case KEY_F (4):
                c = AF4;
                break;
            case KEY_F (5):
                c = AF5;
                break;
            case KEY_F (6):
                c = AF6;
                break;
            case KEY_F (7):
                c = AF7;
                break;
            case KEY_F (8):
                c = AF8;
                break;
            case KEY_F (9):
                c = AF9;
                break;
            case KEY_F (10):
                c = AF10;
                break;
            case KEY_UP:
                c = BUP;
                break;
            case KEY_DOWN:
                c = BDO;
                break;
            case KEY_LEFT:
                c = CCLE;
                break;
            case KEY_RIGHT:
                c = CCRI;
                break;
            case KEY_SFIND:
                c = EINFG;
                break;
            case KEY_IC:
                c = CPS1;
                break;
            case KEY_DC:
                c = CBUP;
                break;
            case KEY_PPAGE:
                c = CEND;
                break;
            case KEY_NPAGE:
                c = CBDO;
                break;
            case KEY_BACKSPACE:
                c = AltBS;
                break;
            case KEY_HELP:
                c = AF1;
                break;
            case KEY_LL:
                c = CEND;
                break;
            case KEY_PREVIOUS:
                c = BUP + 512;
                break;
            case KEY_NEXT:
                c = BDO + 512;
                break;
            case KEY_HOME:
                c = CPS1 + 512;
                break;
            case KEY_END:
                c = CEND + 512;
                break;
            default:
                c = 0;
                break;
            }
            bk = u_bioskey ();
            if (bk & 3) {
                c += 512;
            } else if (bk & 4) {
                c += 514;
            }
        } else if (c != WPE_ESC) {
            c = e_tast_sim (c);
        }
    }
    return (c);
}

#else // #if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
int
e_t_getch ()
{
    int c, c2, pshift, bk;

    pshift = 0;
    e_t_refresh ();
    if ((c = fk_getch ()) != WPE_ESC) {
        if (key_f[20] && c == *key_f[20]) {
            return (WPE_DC);
        } else if (key_f[17] && c == *key_f[17]) {
            return (ENTF);
        } else if (c == WPE_TAB) {
            bk = u_bioskey ();
            if (bk & 3) {
                return (WPE_BTAB);
            } else {
                return (WPE_TAB);
            }
        } else {
            return (c);
        }
    } else if ((c = fk_getch ()) == WPE_CR) {
        return (WPE_ESC);
    }
    bk = u_bioskey ();
    if (bk & 3) {
        pshift = 512;
    } else if (bk & 4) {
        pshift = 514;
    }
    if (c == WPE_ESC) {
        if ((c = fk_getch ()) == WPE_ESC) {
            return (WPE_ESC);
        }
        if ((c2 = e_find_key (c, 1, 1))) {
            return (c2 + pshift);
        }
    }
    if ((c2 = e_find_key ((char) c, 1, 0))) {
        return (c2 + pshift);
    }
    return (e_tast_sim (c + pshift));
}

char e_key_save[10];

int
e_find_key (int c, int j, int sw)
{
    int i, k;
    e_key_save[j] = c;
    e_key_save[j + 1] = '\0';
    for (i = 0; i < KEYFN; i++) {
        if (key_f[i] == NULL) {
            continue;
        }
        for (k = 1; k <= j && e_key_save[k] == *(key_f[i] + k); k++);
        if (k > j) {
            if (*(key_f[i] + k) != '\0') {
                return (e_find_key (fk_getch (), j + 1, sw));
            } else if (sw) {
                switch (i) {
                case 0:
                    return (AF1);
                case 1:
                    return (AF2);
                case 2:
                    return (AF3);
                case 3:
                    return (AF4);
                case 4:
                    return (AF5);
                case 5:
                    return (AF6);
                case 6:
                    return (AF7);
                case 7:
                    return (AF8);
                case 8:
                    return (AF9);
                case 9:
                    return (AF10);
                case 10:
                    return (BUP);
                case 11:
                    return (BDO);
                case 12:
                    return (CCLE);
                case 13:
                    return (CCRI);
                case 14:
                    return (EINFG);
                case 15:
                    return (CPS1);
                case 16:
                    return (CBUP);
                case 17:
                    return (ENTF);
                case 18:
                    return (CEND);
                case 19:
                    return (CBDO);
                case 20:
                    return (AltBS);
                case 21:
                    return (AF1);
                case 22:
                    return (CEND);
                case 33:
                    return (BUP + 512);
                case 34:
                    return (BDO + 512);
                case 35:
                    return (CCLE + 512);
                case 36:
                    return (CCRI + 512);
                case 37:
                    return (CPS1 + 512);
                case 38:
                    return (CBUP + 512);
                case 39:
                    return (CEND + 512);
                case 40:
                    return (CBDO + 512);
                case 41:
                    return (CEND);
                default:
                    return (0);
                }
            } else {
                switch (i) {
                case 0:
                    return (F1);
                case 1:
                    return (F2);
                case 2:
                    return (F3);
                case 3:
                    return (F4);
                case 4:
                    return (F5);
                case 5:
                    return (F6);
                case 6:
                    return (F7);
                case 7:
                    return (F8);
                case 8:
                    return (F9);
                case 9:
                    return (F10);
                case 10:
                    return (CUP);
                case 11:
                    return (CDO);
                case 12:
                    return (CLE);
                case 13:
                    return (CRI);
                case 14:
                    return (EINFG);
                case 15:
                    return (POS1);
                case 16:
                    return (BUP);
                case 17:
                    return (ENTF);
                case 18:
                    return (ENDE);
                case 19:
                    return (BDO);
                case 20:
                    return (WPE_DC);
                case 21:
                    return (HELP);
                case 22:
                    return (ENDE);
                case 23:
                    return (SF1);
                case 24:
                    return (SF2);
                case 25:
                    return (SF3);
                case 26:
                    return (SF4);
                case 27:
                    return (SF5);
                case 28:
                    return (SF6);
                case 29:
                    return (SF7);
                case 30:
                    return (SF8);
                case 31:
                    return (SF9);
                case 32:
                    return (SF10);
                case 33:
                    return (CUP + 512);
                case 34:
                    return (CDO + 512);
                case 35:
                    return (CLE + 512);
                case 36:
                    return (CRI + 512);
                case 37:
                    return (POS1 + 512);
                case 38:
                    return (BUP + 512);
                case 39:
                    return (ENDE + 512);
                case 40:
                    return (BDO + 512);
                case 41:
                    return (ENDE);
                default:
                    return (0);
                }
            }
        }
    }
    return (0);
}
#endif // #if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)

int
fk_t_locate (int x, int y)
{
    if (col_num > 0) {
        fk_colset (e_get_col (cur_x, cur_y));
#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
        /* Causes problems.  Reason unknown. - Dennis */
        /*mvaddch(cur_y,cur_x,e_gt_char(cur_x, cur_y)); */
#else
        fputc (e_gt_char (cur_x, cur_y), stdout);
#endif
    }
    cur_x = x;
    cur_y = y;
    term_move (x, y);
    return (y);
}

int
fk_t_mouse (int *g)
{
    UNUSED (g);
    return (0);
}

int
e_t_switch_screen (int sw)
{
    static int sav_sw = -1;

    if (sw == sav_sw) {
        return (0);
    }
    sav_sw = sw;
    if (sw && get_beg_scr()) {
        term_refresh ();
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
        if (sav_cur) {
            e_putp (sav_cur);
        }
        e_putp (get_beg_scr());
#endif
    } else if (!sw && get_swt_scr()) {
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
        e_putp (get_swt_scr());
        if (res_cur) {
            e_putp (res_cur);
        }
#endif
        term_refresh ();
    } else {
        return (-1);
    }
    return (0);
}

int
e_t_deb_out (we_window_t * window)
{
//#ifndef NCURSES
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
    if (!get_swt_scr() || !get_beg_scr())
#endif
        return (e_error ("Your terminal don\'t use begin/end cup",
                         ERROR_MSG,
                         window->colorset));
    e_u_d_switch_out (1);
    getchar ();
    e_u_d_switch_out (0);
    return (0);
}

/**
 * This function initializes or closes.
 *
 */

int
e_d_switch_screen (int sw)
{
#ifdef DEBUGGER
    if (!sw) {
        e_g_sys_ini ();
    } else {
        e_g_sys_end ();
    }
#endif
    return e_u_switch_screen(sw);
}

int
e_t_d_switch_out (int sw)
{
    int i, j;
    static int save_sw = 32000;

    if (save_sw == sw) {
        return (0);
    }
    save_sw = sw;
    if (sw && e_d_switch_screen (0)) {
        term_move (0, 0);
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
        e_putp (attr_normal());
#endif
        for (i = 0; i < max_screen_lines(); i++)
            for (j = 0; j < max_screen_cols(); j++) {
                e_d_putchar (' ');
            }
        term_move (0, 0);
        term_refresh ();
    } else if (!sw) {
        e_d_switch_screen (1);
        e_abs_refr ();
        e_u_refresh ();
    }
    return (sw);
}

int
e_s_sys_ini ()
{
    e_u_sys_ini ();
    return (e_u_switch_screen (0));
}

int
e_s_sys_end ()
{
    e_u_switch_screen (1);
    return (e_u_sys_end ());
}

#endif /*  Is UNIX       */
