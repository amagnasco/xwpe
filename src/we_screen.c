/** \file we_screen.c                                     */
/* Copyright (C) 2018 Guus Bonnema                        */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */
/*                                                        */
/* Some of the following code is based on or copied from  */
/* other sources within XWPE. For these lines the         */
/* original copyright was:                                */
/* Copyright (C) 1993 Fred Kruse                          */
/**********************************************************/
#include "config.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef UNIX
#include <unistd.h>
#include <termios.h>
#endif

#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
#include <curses.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#endif

#include "stdio.h"

#include "we_screen.h"
#include "we_unix.h"

/* AIX requires that tparm has 10 arguments. */
#define tparm1(aa,bb) tparm((aa), (bb), 0, 0, 0, 0, 0, 0, 0, 0)
#define tparm2(aa,bb,cc) tparm((aa), (bb), (cc), 0, 0, 0, 0, 0, 0, 0)

/** global field also used in we_xterm.c */
int cur_x = -1;
/** global field also used in we_xterm.c */
int cur_y = -1;

/** Private functions */
char *init_key(char *key);
void e_exitm (char *s, int n);

static char *cur_rc; //t
static char cur_attr; //t

/* It seems these attributes are positive */
static char *att_no; // turns all attributes off
static char *start_att_standout;
static char *start_att_underline;
static char *start_att_reversed;
static char *start_att_blink;
static char *start_att_dim;
static char *start_att_bold;

/* The attribute leave_att_normal switches all attributes off, leaving none. So the
 * conventional meaning of leave is double. It leaves all attributes and returns
 * to normal (no attributes set). So it is the same as enter attr normal.
 */
static char *leave_att_normal; // attr leave normal: turn off all attributes
static char *leave_att_standout;
static char *leave_att_underline;
static char *leave_att_reversed;
static char *leave_att_blink;
static char *leave_att_dim;
static char *leave_att_bold;

static char *spc_st;

static char *sav_cur; //t
static char *res_cur; //t

static char *col_fg; //t
static char *col_bg; //t

static char *spc_in; //t
static char *spc_bg;
static char *spc_nd;

static char *beg_scr;
static char *swt_scr;

#define NSPCHR 13
#if defined(HAVE_LIBNCURSES)
static chtype sp_chr[NSPCHR];
#else
static char *sp_chr[NSPCHR];
#endif

/** global field also used in we_xterm.c */
char *global_screen = NULL;
/** global field also used in we_xterm.c */
char *global_alt_screen = NULL;
/** global field also used in we_xterm.c and we_wind.c */
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
char *extbyte = NULL;
char *altextbyte = NULL;
#endif

static int data_max_screen_lines = 80;
static int data_max_screen_cols = 24;

/** Global fields. TODO The intention is to replace them by functions. */
struct termios ntermio;
struct termios otermio;
struct termios ttermio;
int col_num = 0;

/* prototypes not being exported */
int e_begscr();
int init_cursor(void);

/* function definitions */
char *attr_normal()
{
    return att_no;
}

char *graphics_charset_pairs()
{
    return spc_st;
}

char get_graphics_charset_pair(int i)
{
    if (i < 0) {
        return '\0';
    }
    // TODO add border checking for spc_st
    // TODO add error handling
    return spc_st[i];
}

const char *get_beg_scr()
{
    return beg_scr;
}

const char *get_swt_scr()
{
    return swt_scr;
}

/**
 * Get index for char in graphics_charset_pair.
 *
 * Find the index for a given character in the graphics_charset_pair.
 * Return a position >= 0 if found and -1 if not found.
 *
 * \param c char containing character to be found
 * \return index >= 0 if found, -1 if not found
 */
int get_index_graphics_charset_pair(char c)
{
    // Look for the character in spc_st
    int i = 0;
    for ( ; spc_st[i] && spc_st[i + 1] && spc_st[i] != c; i += 2)
        ;
    // check whether character was found and return -1 on not found
    if (!spc_st[i] || !spc_st[i + 1] || spc_st[i] != c) {
        return -1;
    } else {
        return 0;
    }
}

char *enable_alt_charset()
{
    return spc_in;
}

char *start_alt_charset()
{
    return spc_bg;
}

char *end_alt_charset()
{
    return spc_nd;
}

/**
 * is_x_term.
 *
 * Function to determine during initialization whether
 * we need the methods for xterms or for tterms.
 */
_Bool is_x_term()
{
    return col_num > 0;
}

/**
 * Returns the maximum number of screen lines available.
 *
 * \returns maximum number of screen lines available.
 *
 */
int max_screen_lines()
{
    return data_max_screen_lines;
}
/**
 * Sets the new maximum number of screen lines available. if the provided
 * value is zero or less, nothing is set and the old value remains.
 *
 * \param max_lines maximum number of lines.
 * \returns the maximum number of lines.
 *
 */
int set_max_screen_lines(const int max_lines)
{
    if (max_lines > 0) {
        data_max_screen_lines = max_lines;
    }
    return data_max_screen_lines;
}
/**
 * Returns the maximum number of screen columns.
 *
 * \returns the maximum number of screen columns.
 *
 */
int max_screen_cols()
{
    return data_max_screen_cols;
}
/**
 * Sets the new maximum number of columns available. if the provided
 * value is zero or less, nothing is set and the old value remains.
 *
 * \param max_cols maximum number of columns.
 * \returns the maximum number of columns.
 *
 */
int set_max_screen_cols(const int max_cols)
{
    if (max_cols > 0) {
        data_max_screen_cols = max_cols;
    }
    return data_max_screen_cols;
}

int
e_alloc_global_screen ()
{
    if (global_screen) {
        free (global_screen);
    }
    if (global_alt_screen) {
        free (global_alt_screen);
    }
    global_screen = malloc (2 * max_screen_cols() * max_screen_lines());
    global_alt_screen = malloc (2 * max_screen_cols() * max_screen_lines());
#ifdef NEWSTYLE
    if (extbyte) {
        free (extbyte);
    }
    if (altextbyte) {
        free (altextbyte);
    }
    extbyte = malloc (max_screen_cols() * max_screen_lines());
    altextbyte = malloc (max_screen_cols() * max_screen_lines());
    if (!global_screen || !global_alt_screen || !extbyte || !altextbyte) {
        return (-1);
    }
#else
    if (!global_screen || !global_alt_screen) {
        return (-1);
    }
#endif
    return (0);
}

int
e_abs_refr ()
{
    int i;

    for (i = 0; i < 2 * max_screen_cols() * max_screen_lines(); i++) {
        global_alt_screen[i] = 0;
    }
    return (0);
}

int
e_t_initscr ()
{
    int ret, i, k;
#if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES
    WINDOW *stdscr;
#endif

    ret = tcgetattr (STDOUT_FILENO, &otermio);	/* save old settings */
    if (ret) {
        int errno_save = errno;	// save errno before it is overwritten
        const int max_desc = 1024;
        char err_desc[max_desc];
        strerror_r (errno_save, err_desc, max_desc);
        printf
        ("Error in Terminal Initialisation Code nr %d with description: %s\n",
         errno_save, err_desc);
        printf ("c_iflag = %o, c_oflag = %o, c_cflag = %o,\n", otermio.c_iflag,
                otermio.c_oflag, otermio.c_cflag);
        printf
        ("c_lflag = %o, c_line = %o, c_cc = {\"\\%o\\%o\\%o\\%o\\%o\\%o\\%o\\%o\"}\n",
         otermio.c_lflag, otermio.c_line, otermio.c_cc[0], otermio.c_cc[1],
         otermio.c_cc[2], otermio.c_cc[3], otermio.c_cc[4], otermio.c_cc[5],
         otermio.c_cc[6], otermio.c_cc[7]);
        e_exit (1);
    }
#if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES
    if ((stdscr = initscr ()) == (WINDOW *) ERR) {
        exit (27);
    }
    cbreak ();
    noecho ();
    nonl ();
    intrflush (stdscr, FALSE);
    keypad (stdscr, TRUE);
    if (has_colors ()) {
        start_color ();
        for (i = 0; i < COLORS; i++) {
            for (k = 0; k < COLORS; k++) {
                if (i != 0 || k != 0) {
                    init_pair (i * 8 + k, k, i);
                }
            }
        }
    }
#endif
    e_begscr ();
    global_screen = malloc (2 * max_screen_cols() * max_screen_lines());
    global_alt_screen = malloc (2 * max_screen_cols() * max_screen_lines());
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
    extbyte = malloc (max_screen_cols() * max_screen_lines());
#endif
    e_abs_refr ();
    if (init_cursor ()) {
        printf ("Terminal Not in the right mode\n");
        e_exit (1);
    }
    tcgetattr (STDIN_FILENO, &ntermio);
    ntermio.c_iflag = 0;		/* setup new settings */
    ntermio.c_oflag = 0;
    ntermio.c_lflag = 0;
    ntermio.c_cc[VMIN] = 1;
    ntermio.c_cc[VTIME] = 0;
    /**
     * VSWTCH is not supported by linux or defined in POSIX. It is used in some
     * System V systems. I suspect the following conditional code was incorporated to
     * initialize that specific special character.
     */
#ifdef VSWTCH
    ntermio.c_cc[VSWTCH] = 0;
#endif
    tcsetattr (STDIN_FILENO, TCSADRAIN, &ntermio);
#if !defined(HAVE_LIBNCURSES) && !defined(HAVE_LIBCURSES)
    if (spc_in) {
        e_putp (spc_in);
    }
#endif
    return (0);
}

int
e_begscr ()
{
    int cols, lns;
    int kbdflgs;

    kbdflgs = fcntl (0, F_GETFL, 0);
    if (kbdflgs == -1) {
        int save_errno = errno;
        if (save_errno == EINVAL) {
            // show error msg: kernel does not understand F_GETFL feature.
            e_exitm("[e_begscr] cmd F_GETFL is unknown to current linux O.S.\n", 1);
            exit(0);
        }
    }
//#ifndef TERMCAP
#if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES
    if ((lns = tigetnum ("lines")) > 0) {
        set_max_screen_lines(lns);
    }
    if ((cols = tigetnum ("cols")) > 0) {
        set_max_screen_cols(cols);
    }
#else
    if ((tc_screen = getenv ("TERM")) == NULL) {
        e_exitm ("Environment variable TERM not defined!", 1);
    }
    if ((tgetent (tcbuf, tc_screen)) != 1) {
        e_exitm ("Unknown terminal type!", 1);
    }
    if ((lns = tgetnum ("li")) > 0) {
        set_max_screen_lines(lns);
    }
    if ((cols = tgetnum ("co")) > 0) {
        set_max_screen_cols(cols);
    }
#endif
    return (0);
}

char *
init_key (char *key)
{
    char *tmp, *keystr;

    tmp = (char *) tigetstr (key);
    if (tmp == NULL || tmp == ((char *) -1)) {
        return (NULL);
    } else {
        keystr = malloc (strlen (tmp) + 1);
        strcpy (keystr, tmp);
    }
    return (keystr);
}

int
init_cursor ()
{
//#ifndef TERMCAP
// It looks like not defined TERMCAP means TERMINFO is defined
#if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES
    if (!(cur_rc = init_key ("cup"))) {
        return (-1);
    }
    if ((col_fg = init_key ("setaf")) && (col_bg = init_key ("setab"))) {
        /* terminfo 10.2.x color code */
        if ((col_num = tigetnum ("colors")) < 0) {
            col_num = 8;
        }
    } else if ((col_fg = init_key ("setf")) && (col_bg = init_key ("setb"))) {
        /* Older terminfo color code */
        if ((col_num = tigetnum ("colors")) < 0) {
            col_num = 8;
        }
    } else {
        /* No colors */
        if (col_fg) {
            free (col_fg);
            col_fg = NULL;
        }
        if (col_bg) {
            free (col_bg);
            col_bg = NULL;
        }
    }

    spc_st = init_key ("acsc"); // terminfo.5: acsc == ac == graphics charset pairs (vt100)
    spc_in = init_key ("enacs"); // terminfo.5: enacs == eA == enable alternate character set
    spc_bg = init_key ("smacs"); // terminfo.5: smacs == as == start alternate character set
    spc_nd = init_key ("rmacs"); // terminfo.5: rmacs == ae == end alternate character set
    att_no = init_key ("sgr0");  // terminfo.5: sgr0 == me == turn off all attributes
    start_att_standout = init_key ("smso");
    start_att_underline = init_key ("smul");
    start_att_reversed = init_key ("rev");
    start_att_blink = init_key ("blink");
    start_att_dim = init_key ("dim");
    start_att_bold = init_key ("bold");
    leave_att_normal = init_key ("sgr0"); // "me" == "sgr0" == "turn off all attributes" (terminfo.5)
    leave_att_standout = init_key ("rmso");
    leave_att_underline = init_key ("rmul"); // exit underline mode (see terminfo(5))
    leave_att_reversed = init_key ("sgr0");
    leave_att_blink = init_key ("sgr0");
    leave_att_dim = init_key ("sgr0");
    leave_att_bold = init_key ("sgr0");
    beg_scr = init_key ("smcup");
    swt_scr = init_key ("rmcup");
    sav_cur = init_key ("sc");
    res_cur = init_key ("rc");

//#ifndef NCURSES
#if !defined(HAVE_LIBNCURSES)
    key_f[0] = init_kkey ("kf1");
    key_f[1] = init_kkey ("kf2");
    key_f[2] = init_kkey ("kf3");
    key_f[3] = init_kkey ("kf4");
    key_f[4] = init_kkey ("kf5");
    key_f[5] = init_kkey ("kf6");
    key_f[6] = init_kkey ("kf7");
    key_f[7] = init_kkey ("kf8");
    key_f[8] = init_kkey ("kf9");
    key_f[9] = init_kkey ("kf10");
    key_f[10] = init_kkey ("kcuu1");
    key_f[11] = init_kkey ("kcud1");
    key_f[12] = init_kkey ("kcub1");
    key_f[13] = init_kkey ("kcuf1");
    key_f[14] = init_kkey ("kich1");
    key_f[15] = init_kkey ("khome");
    key_f[16] = init_kkey ("kpp");
    if (!(key_f[17] = init_kkey ("kdch1"))) {
        key_f[17] = "\177";
    }
    key_f[18] = init_kkey ("kend");
    key_f[19] = init_kkey ("knp");
    key_f[20] = init_kkey ("kbs");
    key_f[21] = init_kkey ("khlp");
    key_f[22] = init_kkey ("kll");
    key_f[23] = init_kkey ("kf17");
    key_f[24] = init_kkey ("kf18");
    key_f[25] = init_kkey ("kf19");
    key_f[26] = init_kkey ("kf20");
    key_f[27] = init_kkey ("kf21");
    key_f[28] = init_kkey ("kf22");
    key_f[29] = init_kkey ("kf23");
    key_f[30] = init_kkey ("kf24");
    key_f[31] = init_kkey ("kf25");
    key_f[32] = init_kkey ("kf26");
    key_f[33] = init_kkey ("kPRV");
    key_f[34] = init_kkey ("kNXT");
    key_f[35] = init_kkey ("kLFT");
    key_f[36] = init_kkey ("kRIT");
    key_f[37] = init_kkey ("kHOM");
    key_f[38] = init_kkey ("kri");
    key_f[39] = init_kkey ("kEND");
    key_f[40] = init_kkey ("kind");
    key_f[41] = init_kkey ("kext");
#endif

#else // #if defined HAVE_LIBNCURSES || defined HAVE_LIBCURSES i.e. termcap

    if (!(cur_rc = init_key ("cm"))) {
        return (-1);
    }
    if ((col_fg = init_key ("Sf")) && (col_bg = init_key ("Sb"))) {
        if ((col_num = tgetnum ("Co")) < 0) {
            col_num = 8;
        }
    } else {
        if (col_fg) {
            free (col_fg);
            col_fg = NULL;
        }
        if (col_bg) {
            free (col_bg);
            col_bg = NULL;
        }
    }
    spc_st = init_key ("ac"); // terminfo.5: acsc == ac == graphics charset pairs (vt100)
    spc_in = init_key ("eA"); // terminfo.5:
    spc_bg = init_key ("as");
    spc_nd = init_key ("ae");
    att_no = init_key ("me");
    start_att_standout = init_key ("so");
    start_att_underline = init_key ("us");
    start_att_reversed = init_key ("mr");
    start_att_blink = init_key ("mb");
    start_att_dim = init_key ("mh");
    start_att_bold = init_key ("md");
    leave_att_normal = init_key ("me"); // "me" == "sgr0" == "turn off all attributes" (terminfo.5)
    leave_att_standout = init_key ("se");
    leave_att_underline = init_key ("ue");
    leave_att_reversed = init_key ("me");
    leave_att_blink = init_key ("me");// "me" == "sgr0" == "turn off all attributes" (terminfo.5)
    leave_att_dim = init_key ("me");// "me" == "sgr0" == "turn off all attributes" (terminfo.5)
    leave_att_bold = init_key ("me");// "me" == "sgr0" == "turn off all attributes" (terminfo.5)
    beg_scr = init_key ("ti");
    swt_scr = init_key ("te");
    sav_cur = init_key ("sc");
    res_cur = init_key ("rc");

    key_f[0] = init_kkey ("k1");
    key_f[1] = init_kkey ("k2");
    key_f[2] = init_kkey ("k3");
    key_f[3] = init_kkey ("k4");
    key_f[4] = init_kkey ("k5");
    key_f[5] = init_kkey ("k6");
    key_f[6] = init_kkey ("k7");
    key_f[7] = init_kkey ("k8");
    key_f[8] = init_kkey ("k9");
    key_f[9] = init_kkey ("k;");
    key_f[10] = init_kkey ("ku");
    key_f[11] = init_kkey ("kd");
    key_f[12] = init_kkey ("kl");
    key_f[13] = init_kkey ("kr");
    key_f[14] = init_kkey ("kI");
    key_f[15] = init_kkey ("kh");
    key_f[16] = init_kkey ("kP");
    key_f[17] = init_kkey ("kD");
    key_f[18] = init_kkey ("@7");
    key_f[19] = init_kkey ("kN");
    key_f[20] = init_kkey ("kb");
    key_f[21] = init_kkey ("%1");
    key_f[22] = init_kkey ("kH");
    key_f[23] = init_kkey ("F7");
    key_f[24] = init_kkey ("F8");
    key_f[25] = init_kkey ("F9");
    key_f[26] = init_kkey ("FA");
    key_f[27] = init_kkey ("FB");
    key_f[28] = init_kkey ("FC");
    key_f[29] = init_kkey ("FD");
    key_f[30] = init_kkey ("FE");
    key_f[31] = init_kkey ("FF");
    key_f[32] = init_kkey ("FG");
    key_f[33] = init_kkey ("%e");
    key_f[34] = init_kkey ("%c");
    key_f[35] = init_kkey ("#4");
    key_f[36] = init_kkey ("%i");
    key_f[37] = init_kkey ("#2");
    key_f[38] = init_kkey ("kR");
    key_f[39] = init_kkey ("*7");
    key_f[40] = init_kkey ("kF");
    key_f[41] = init_kkey ("@1");
#endif
//#ifdef NCURSES
#if defined(HAVE_LIBNCURSES)
    sp_chr[0] = 0;
    sp_chr[1] = ACS_ULCORNER;
    sp_chr[2] = ACS_URCORNER;
    sp_chr[3] = ACS_LLCORNER;
    sp_chr[4] = ACS_LRCORNER;
    sp_chr[5] = ACS_HLINE;
    sp_chr[6] = ACS_VLINE;
    sp_chr[7] = ACS_S9;
    sp_chr[8] = ACS_VLINE;
    sp_chr[9] = ACS_VLINE;
    sp_chr[10] = ACS_S9;
    sp_chr[11] = ACS_DIAMOND;
    sp_chr[12] = ' ';
#else // #else #ifdef NCURSES (i.e. not ncurses: curses or termcap)
    sp_chr[0] = "";
    if (!(sp_chr[1] = init_spchr ('l'))) {
        sp_chr[1] = "+";
    }
    if (!(sp_chr[2] = init_spchr ('k'))) {
        sp_chr[2] = "+";
    }
    if (!(sp_chr[3] = init_spchr ('m'))) {
        sp_chr[3] = "+";
    }
    if (!(sp_chr[4] = init_spchr ('j'))) {
        sp_chr[4] = "+";
    }
    if (!(sp_chr[5] = init_spchr ('q'))) {
        sp_chr[5] = "-";
    }
    if (!(sp_chr[6] = init_spchr ('x'))) {
        sp_chr[6] = "|";
    }
    if (!(sp_chr[7] = init_spchr ('w'))) {
        sp_chr[7] = "_";
    }
    if (!(sp_chr[8] = init_spchr ('t'))) {
        sp_chr[8] = "|";
    }
    if (!(sp_chr[9] = init_spchr ('m'))) {
        sp_chr[9] = "|";
    }
    if (!(sp_chr[10] = init_spchr ('q'))) {
        sp_chr[10] = "_";
    }
    if (!(sp_chr[11] = init_spchr ('`'))) {
        sp_chr[11] = "#";
    }
    if (!(sp_chr[12] = init_spchr ('a'))) {
        sp_chr[12] = " ";
    }
#endif
    return (0);
}

int
fk_attrset (int a)
{
    if (cur_attr == a) {
        return (0);
    }
//#ifdef NCURSES
#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
    switch (a) {
    case 0:
        attrset (A_NORMAL);
        break;
    case 1:
        attrset (A_STANDOUT);
        break;
    case 2:
        attrset (A_UNDERLINE);
        break;
    case 4:
        attrset (A_REVERSE);
        break;
    case 8:
        attrset (A_BLINK);
        break;
    case 16:
        attrset (A_DIM);
        break;
    case 32:
        attrset (A_BOLD);
        break;
    }
#else
    switch (cur_attr) {
    case 0:
        fk_putp (leave_att_normal);
        break;
    case 1:
        fk_putp (leave_att_standout);
        break;
    case 2:
        fk_putp (leave_att_underline);
        break;
    case 4:
        fk_putp (leave_att_reversed);
        break;
    case 8:
        fk_putp (leave_att_blink);
        break;
    case 16:
        fk_putp (leave_att_dim);
        break;
    case 32:
        fk_putp (leave_att_bold);
        break;
    }
    switch (a) {
    case 0:
        fk_putp (att_no);
        break;
    case 1:
        fk_putp (start_att_standout);
        break;
    case 2:
        fk_putp (start_att_underline);
        break;
    case 4:
        fk_putp (start_att_reversed);
        break;
    case 8:
        fk_putp (start_att_blink);
        break;
    case 16:
        fk_putp (start_att_dim);
        break;
    case 32:
        fk_putp (start_att_bold);
        break;
    }
#endif
    return (cur_attr = a);
}

void
fk_colset (int c)
{
    int bg;
    if (cur_attr == c) {
        return;
    }
    cur_attr = c;
    bg = c / 16;
//#ifdef TERMCAP
#if !defined HAVE_LIBNCURSES && !defined HAVE_LIBCURSES
    if ((c %= 16) >= col_num) {
        fk_putp (start_att_bold);
        e_putp (tgoto (col_fg, 0, c % col_num));
        e_putp (tgoto (col_bg, 0, bg));
    } else {
        fk_putp (leave_att_bold);
        e_putp (tgoto (col_fg, 0, c));
        e_putp (tgoto (col_bg, 0, bg));
    }
#else // #if !defined HAVE_LIBNCURSES && !defined HAVE_LIBCURSES
#ifdef HAVE_LIBNCURSES
    if (c & 8) {
        attrset (A_BOLD);
    } else {
        attrset (A_NORMAL);
    }
    color_set ((bg * 8) + c % 8, NULL);
#else
    if ((c %= 16) >= col_num) {
        fk_putp(att_bo);
        e_putp(tparm1(col_fg, c % col_num));
        e_putp(tparm1(col_bg, bg));
    } else {
        fk_putp(ratt_bo);
        e_putp(tparm1(col_fg, c));
        e_putp(tparm1(col_bg, bg));
    }
#endif // #ifdef NCURSES
#endif // #ifdef TERMCAP : !defined HAVE_LIBNCURSES && !defined HAVE_LIBCURSES
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

int fk_t_cursor(int x)
{
    return x;
}

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

int term_move(int x, int y)
{
#if !defined HAVE_LIBNCURSES && !defined HAVE_LIBCURSES
    return e_putp(tgoto(cur_rc, y, x));
#else
    return move(y,x);
#endif
}

int print_char(int c)
{
#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
    if (c < NSPCHR) {
        addch (sp_chr[c]);
    } else {
        addch (c);
    }
#else
    if (c < NSPCHR) {
        e_putp (sp_chr[c]);
    } else {
        fputc (c, stdout);
    }
#endif
    return 0;
}

int term_refresh()
{
#if !defined HAVE_LIBNCURSES && !defined HAVE_LIBCURSES
    int rc = fflush(stdout);
    if (rc == EOF) {
        int saverrno = errno;
        perror("[term_refresh()]");
        fprintf(stderr, "refreshing the screen with fflush() failed.\n");
        return -1;
    }
    return 0;
#else
    int rc = refresh();
    if (rc == ERR) {
        fprintf(stderr, "refreshing the screen with refresh() failed.\n");
        return -1;
    }
    return rc;
#endif
}

void
e_exitm (char *s, int n)
{
    e_endwin ();
    if (n != 0) {
        printf ("\n%s\n", s);
    }
    exit (n);
}

void
e_endwin ()
{
#if defined(HAVE_LIBNCURSES) || defined(HAVE_LIBCURSES)
    endwin ();
#else
    fk_putp (leave_att_bold);
#endif
    tcsetattr (STDIN_FILENO, TCSADRAIN, &otermio);
}
