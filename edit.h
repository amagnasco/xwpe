#ifndef __EDIT_H
#define __EDIT_H
/* edit.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */
/*
                     Header file for FK-editor
*/

#define VERSION "1.5.29a"

#include "Xwpe.h"
#include "WeString.h"
#include "options.h"

#include "model.h"
#include "keys.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifdef HAVE_LIBGPM
#include <gpm.h>
#endif

#ifdef UNIX
#include <unistd.h>
#include "unixmakr.h"
#include "unixkeys.h"

#ifndef TERMCAP
#include <curses.h>
#endif

extern int MAXSLNS, MAXSCOL, MENOPT;
#define MAXEDT 35
#endif

#define MAXLINES 10
#define MAXCOLUM 120

#define WPE_NOBACKUP 1
#define WPE_BACKUP   0

#if  MOUSE
struct mouse {
 int x;
 int y;
 int k;
};
#endif

#define DTMD_NORMAL        'n' /* Normal text file */
#define DTMD_MSDOS         'm' /* MS-DOS text file */
#define DTMD_HELP          'h' /* Help window */
#define DTMD_DATA          'D' /* Data/project windows */
#define DTMD_FILEMANAGER   'F' /* File manager */
/* File/directory dropdown of previous files/directories on the file manager */
#define DTMD_FILEDROPDOWN  'M'

#define DTMD_ISTEXT(x)     (x > 'Z')
#define DTMD_ISMARKABLE(x) (x > DTMD_HELP) /* Means end marks can be shown*/

struct dirfile {
 int anz;      /* number elements in the list */
 char **name;  /* the list elements */
};

typedef struct PNT {
 int x;
 int y;
} POINT;

typedef struct CLR {
 int f;
 int b;
 int fb;
} COLOR;

typedef struct PICSTR {
 char *p;
 POINT a;
 POINT e;
} PIC;

typedef struct FND {
 char search[80], replace[80];
 char file[80];   /* filename or pattern to search/open */
 char *dirct;
 int sn;
 int rn;
 unsigned int sw;
} FIND;

typedef struct frb {
 COLOR er;   /* editor window border and text */
 COLOR es;   /* special signs (maximize/kill) on editor window border */
 COLOR et;   /* normal text in editor window */
 COLOR ez;   /* marked text in editor window */
 COLOR ek;   /* found/marked word in editor window */
 COLOR em;   /* scrollbar */
 COLOR hh;   /* Help header */
 COLOR hb;   /* button in Help */
 COLOR hm;   /* marked word in Help */
 COLOR db;   /* breakpoint set */
 COLOR dy;   /* stop at breakpoint */
 COLOR mr;   /* submenu border */
 COLOR ms;   /* menu shortkey text */
 COLOR mt;   /* menu text */
 COLOR mz;   /* active menu text */
 COLOR df;   /* desktop */
 COLOR nr;   /* message window border and text */
 COLOR ne;   /* special signs (maximize/kill) on message window border */
 COLOR nt;   /* normal text for widgets in message window */
 COLOR nsnt; /* widget selector shortkey in message window */
 COLOR fr;   /* passive entry */
 COLOR fa;   /* active entry */
 COLOR ft;   /* normal data text */
 COLOR fz;   /* active, marked data text */
 COLOR frft; /* passive, marked data text */
 COLOR fs;   /* passive switch */
 COLOR nsft; /* switch selector shortkey */
 COLOR fsm;  /* active switch */
 COLOR nz;   /* normal/passive button text */
 COLOR ns;   /* button shortkey text */
 COLOR nm;   /* active button text */
 COLOR of;
 COLOR ct;   /* normal program text */
 COLOR cr;   /* reserved keywords in program */
 COLOR ck;   /* constants in program */
 COLOR cp;   /* preprocessor command */
 COLOR cc;   /* comments in program */
 char dc;    /* desktop fill character */
 char ws; 
} FARBE;

typedef struct undo {
 int type;
 POINT b, a, e;
 union {
  char c;
  void *pt;
 } u;
 struct undo *next;
}  Undo;

typedef struct STR {
 unsigned char *s;
 int len; /* Length of string not counting '\n' at the end */
 int nrc;
 /*int size;*/ /* Memory allocated for the string */
} STRING;

typedef struct BFF {
 STRING *bf; /* bf[i] is the i-th line of the buffer */
 POINT b;    /* cursor coordinates in window (at least in some contexts) */
 POINT mx; /* maximum column and line */
 int mxlines; /* number of lines */
 int cl, clsv;
 Undo *ud, *rd;
 struct CNT *cn;
 struct FNST *f;
 FARBE *fb;
} BUFFER;

typedef struct SCHRM {
 POINT mark_begin;
 POINT mark_end;
 POINT ks;
 POINT pt[9];
 POINT fa;
 POINT fe;
 POINT a;
 POINT e;
 POINT c;
 FARBE *fb;
#ifdef DEBUGGER
 POINT da, de;
 int *brp;
#endif
} SCHIRM;

typedef struct OPTION {
 char *t;
 int x;
 int s;
 int as;
} OPT;

typedef struct WOPTION {
 char *t;
 int x, s, n, as;
} WOPT;

typedef struct OPTKAST {
 char *t;
 int x;
 char o;
 int (*fkt)(struct FNST *);
} OPTK;

typedef struct {
 int position;
 int width;
 int no_of_items;
 OPTK *menuitems;
} MENU;

typedef struct FNST {
 POINT a;         /* start corner of the box */
 POINT e;         /* other corner of the box */
 POINT sa;
 POINT se;
 char zoom;
 FARBE *fb;       /* color scheme */
 PIC *pic;        /* picture save below the box ??? */
 char *dirct;     /* working/actual directory */
 char *datnam;    /* window header text */
 int winnum;      /* ID number in parents structure ??? */
 char ins;
 char dtmd; /* (See DTMD_* defines) */
 int save;
 char *hlp_str;
 WOPT *blst;      /* status line text */
 int nblst;       /* no of options in the status line */
 int filemode, flg;
 int *c_sw;
 struct wpeSyntaxRule *c_st;
 struct CNT *ed;  /* parent control structure ??? */
 struct BFF *b;
 struct SCHRM *s;
 FIND fd;
} FENSTER;

typedef struct CNT {
 int major, minor, patch; /* Version of option file. */
 int maxcol, tabn;
 int maxchg, numundo;
 int flopt, edopt;
 int mxedt;           /* max number of exiting windows */
 int curedt;          /* currently active window */
 int edt[MAXEDT + 1]; /* 1 <= window IDs <= MAXEDT, arbitrary order */
 int autoindent;
 char *print_cmd;
 char *dirct;         /* current directory */
 char *optfile, *tabs; 
 struct dirfile *sdf, *rdf, *fdf, *ddf, *wdf, *hdf, *shdf;
 FIND fd;
 FARBE *fb;
 FENSTER *f[MAXEDT + 1];
 char dtmd, autosv; 
} ECNT;


/* structure for the windows in the file manager ??? */
typedef struct fl_wnd {
 int xa, ya;         /* its own box corner ??? */
 int xe, ye;
 int ia, ja;
 int nf;             /* selected field in dirfile df struct */
 int nxfo, nyfo;
 int mxa, mya;       /* parent box corners ??? */
 int mxe, mye;
 int srcha;
 struct dirfile *df; /* directory tree or file list */
 FENSTER* f;         /* the window itself */
} FLWND;

typedef struct FLBFF {
 struct dirfile *cd; /* current directory */
 struct dirfile *dd; /* list of directories in the current dir. */
 struct dirfile *df; /* list of files in the current dir. */
 struct fl_wnd *fw;  /* window for file list */
 struct fl_wnd *dw;  /* window for dir tree */
 char *rdfile;       /* file pattern entered for searching */
 char sw;
 int xfa, xfd, xda, xdd;
} FLBFFR;

typedef struct {
 int x, y;
 char *txt;
} W_O_TXTSTR;

typedef struct {
 int xt, yt, xw, yw, nw, wmx, nc, sw; 
 char *header;
 char *txt;
 struct dirfile **df;
} W_O_WRSTR;

typedef struct {
 int xt, yt, xw, yw, nw, wmx, nc, num, sw; 
 char *header;
} W_O_NUMSTR;

typedef struct {
 int x, y, nc, sw, num;
 char *header;
} W_O_SSWSTR;

typedef struct {
 int x, y, nc, sw;
 char *header;
} W_O_SPSWSTR;

typedef struct {
 int num, np;
 W_O_SPSWSTR **ps;
} W_O_PSWSTR;

typedef struct {
 int x, y, nc, sw;
 char *header; 
 int (*fkt)(FENSTER *f);
} W_O_BTTSTR;

typedef struct {
 int xa, ya, xe, ye, bgsw, crsw;
 int frt, frs, ftt, fts, fst, fss, fsa, fbt;
 int fbs, fbz, fwt, fws;
 int tn, sn, pn, bn, wn, nn;
 char *name;
 PIC *pic;
 W_O_TXTSTR **tstr;
 W_O_SSWSTR **sstr;
 W_O_PSWSTR **pstr;
 W_O_BTTSTR **bstr;
 W_O_WRSTR  **wstr;
 W_O_NUMSTR **nstr;
 FENSTER *f;
} W_OPTSTR;

typedef struct wpeOptionSection {
 char *section;
 int (*function)(ECNT *cn, char *section, char *option, char *value);
} WpeOptionSection;

/*   we_mouse.c   */
#if  MOUSE
int e_mshit(void);
int e_m1_mouse(void);
int e_m2_mouse(int xa, int ya, int xe, int ye, OPTK *fopt);
int e_m3_mouse(void);
int e_er_mouse(int x, int y, int xx, int yy);
int e_msg_mouse(int x, int y, int x1, int x2, int yy);
int WpeMngMouseInFileManager(FENSTER *f);
int WpeMouseInFileDirList(int k, int sw, FENSTER *f);
int fl_wnd_mouse(int sw, int k, FLWND *fw);
int e_lst_mouse(int x, int y, int n, int sw, int max, int nf);
void e_eck_mouse(FENSTER *f, int sw);
int e_edt_mouse(int c, FENSTER *f);
int e_ccp_mouse(int c, FENSTER *f);
void e_cur_mouse(FENSTER *f);
int e_opt_ck_mouse(int xa, int ya, int md);
int e_opt_cw_mouse(int xa, int ya, int md);
int e_opt_bs_mouse(void);
void e_opt_eck_mouse(W_OPTSTR *o);
int e_opt_mouse(W_OPTSTR *o);

int e_data_ein_mouse(FENSTER *f);
int e_opt_bs_mouse_1(void);
int e_opt_bs_mouse_2(void);
int e_opt_bs_mouse_3(void);
int e_rahmen_mouse(FENSTER *f);
#endif

/*   we_opt.c   */
char *WpeStringToValue(const char *str);
char *WpeValueToString(const char *value);
int e_about_WE(FENSTER *f);
int e_clear_desk(FENSTER *f);
int e_repaint_desk(FENSTER *f);
int e_sys_info(FENSTER *f);
int e_ad_colors(FENSTER *f);
int e_dif_colors(int sw, int xa, int ya, FENSTER *f, int md);
void e_pr_dif_colors(int sw, int xa, int ya, FENSTER *f, int sw2, int md);
void e_pr_x_col_kasten(int xa, int ya, int x, int y, FENSTER *f, int sw);
void e_pr_ed_beispiel(int xa, int ya, FENSTER *f, int sw, int md);
int e_opt_save(FENSTER *f);
int e_save_opt(FENSTER *f);
int e_opt_read(ECNT *cn);
int e_add_arguments(char *str, char *head, FENSTER *f, int n, int sw,
  struct dirfile **df);
W_O_TXTSTR **e_add_txtstr(int x, int y, char *txt, W_OPTSTR *o);
W_O_WRSTR **e_add_wrstr(int xt, int yt, int xw, int yw, int nw, int wmx,
  int nc, int sw, char *header, char *txt, struct dirfile **df, W_OPTSTR *o);
W_O_NUMSTR **e_add_numstr(int xt, int yt, int xw, int yw, int nw, int wmx,
  int nc, int sw, char *header, int num, W_OPTSTR *o);
W_O_SSWSTR **e_add_sswstr(int x, int y, int nc, int sw, int num,
  char *header, W_OPTSTR *o);
W_O_SPSWSTR **e_add_spswstr(int n, int x, int y, int nc, int sw,
  char *header, W_OPTSTR *o);
W_O_PSWSTR **e_add_pswstr(int n, int x, int y, int nc, int sw, int num,
  char *header, W_OPTSTR *o);
W_O_BTTSTR **e_add_bttstr(int x, int y, int nc, int sw, char *header,
  int (*fkt)(FENSTER *f), W_OPTSTR *o);
int freeostr(W_OPTSTR *o);
W_OPTSTR *e_init_opt_kst(FENSTER *f);
int e_opt_move(W_OPTSTR *o);
int e_get_sw_cmp(int xin, int yin, int x, int y, int xmin, int ymin, int c);
int e_get_opt_sw(int c, int x, int y, W_OPTSTR *o);
int e_opt_kst(W_OPTSTR *o);
int e_edt_options(FENSTER *f);
int e_read_colors(FENSTER *f);
int e_ad_colors_md(FENSTER *f, int md);
int e_frb_x_menue(int sw, int xa, int ya, FENSTER *f, int md);
void e_pr_x_col_kasten(int xa, int ya, int x, int y, FENSTER *f, int sw);

/*   we_wind.c   */
int e_error(char *text, int sw, FARBE *f);
int e_message(int sw, char *str, FENSTER *f);
void e_firstl(FENSTER *f, int sw);
int e_pr_filetype(FENSTER *f);
PIC *e_open_view(int xa, int ya, int xe, int ye, int col, int sw);
int e_close_view(PIC *pic, int sw);
void e_pr_line(int y, FENSTER *f);
void e_std_rahmen(int xa, int ya, int xe, int ye, char *name, int sw,
  int frb, int fes);
void e_ed_rahmen(FENSTER *f, int sw);
int e_schirm(FENSTER *f, int sw);
int e_size_move(FENSTER *f);
PIC *e_std_kst(int xa, int ya, int xe, int ye, char *name, int sw, int fr,
  int ft, int fes);
PIC *e_ed_kst(FENSTER *f, PIC *pic, int sw);
int e_close_window(FENSTER *f);
void e_switch_window(int num, FENSTER *f);
int e_ed_zoom(FENSTER *f);
int e_ed_cascade(FENSTER *f);
int e_ed_tile(FENSTER *f);
int e_ed_next(FENSTER *f);
int e_mess_win(char *header, char *str, PIC **pic, FENSTER *f);
PIC *e_change_pic(int xa, int ya, int xe, int ye, PIC *pic, int sw, int frb);
struct dirfile *e_add_df(char *str, struct dirfile *df);
int e_schr_nchar_wsv(char *str, int x, int y, int n, int max, int col,
  int csw);
int e_schr_lst_wsv(char *str, int xa, int ya, int n, int strlen, int ft,
  int fz, struct dirfile **df, FENSTER *f);
int e_rep_win_tree(ECNT *cn);
int e_opt_sec_box(int xa, int ya, int num, OPTK *opt, FENSTER *f, int sw);
int e_close_buffer(BUFFER *b);
int e_list_all_win(FENSTER *f);


#ifdef UNIX

/*   we_unix.c   */
int e_abs_refr(void);
void e_refresh_area(int x, int y, int width, int height);
void WpeNullFunction(void);
int WpeZeroFunction();
int e_tast_sim(int c);
void e_err_save(void);
void e_exit(int n);
char *e_mkfilepath(char *dr, char *fn, char *fl);
int e_compstr(char *a, char *b);
struct dirfile *e_find_files(char *sufile, int sw);
struct dirfile *e_find_dir(char *sufile, int sw);
char *e_file_info(char *filen, char *str, int *num, int sw);
void ini_repaint(ECNT *cn);
void end_repaint(void);
int e_frb_t_menue(int sw, int xa, int ya, FENSTER *f, int md);
void e_pr_t_col_kasten(int xa, int ya, int x, int y, FENSTER *f, int sw);
int e_ini_unix(int *argc, char **argv);
int e_recover(ECNT *cn);
int e_ini_schirm(int argc, char **argv);

extern int (*fk_u_locate)(int x, int y);
extern int (*fk_u_cursor)(int x);
extern int (*e_u_initscr)(int argc, char *argv[]);
extern int (*fk_u_putchar)(int c);
extern int (*u_bioskey)(void);
extern int (*e_frb_u_menue)(int sw, int xa, int ya, FENSTER *f, int md);
extern COLOR (*e_s_u_clr)(int f, int b);
extern COLOR (*e_n_u_clr)(int fb);
extern void (*e_pr_u_col_kasten)(int xa, int ya, int x, 
					int y, FENSTER *f, int sw);
extern int (*fk_mouse)(int g[]);
extern int (*e_u_refresh)(void);
extern int (*e_u_getch)(void);
extern int (*e_u_sys_ini)(void);
extern int (*e_u_sys_end)(void);
extern void (*WpeMouseChangeShape)(WpeMouseShape new_shape);
extern void (*WpeMouseRestoreShape)(void);
extern int (*e_u_d_switch_out)(int sw);
extern int (*e_u_switch_screen)(int sw);
extern int (*e_u_deb_out)(FENSTER *f);
extern int (*e_u_cp_X_to_buffer)(FENSTER *f);
extern int (*e_u_copy_X_buffer)(FENSTER *f);
extern int (*e_u_paste_X_buffer)(FENSTER *f);
extern int (*e_u_change)(PIC *pic);
extern int (*e_u_ini_size)(void);
extern int (*e_u_s_sys_end)(void);
extern int (*e_u_s_sys_ini)(void);
extern void (*e_u_setlastpic)(PIC *pic);
extern int (*e_u_system)(const char *exe);
extern int (*e_u_kbhit)(void);
extern void (*WpeDisplayEnd)(void);

int e_put_pic_xrect(PIC *pic);
int e_get_pic_xrect(int xa, int ya, int xe, int ye, PIC *pic);
#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
int e_make_xrect(int xa, int ya, int xe, int ye, int sw);
int e_make_xrect_abs(int xa, int ya, int xe, int ye, int sw);
#else
#define e_make_xrect(a,b,c,d,e)
#define e_make_xrect_abs(a,b,c,d,e)
#endif
#endif

#ifdef PROG
#include "progr.h"
#include "WeProg.h"
#endif

#ifdef DEBUGGER
int e_deb_inp(FENSTER *f);
int e_e_line_read(int n, signed char *s, int max);
int e_d_dum_read(void);
int e_d_p_exec(FENSTER *f);
int e_d_getchar(void);
int e_d_quit_basic(FENSTER *f);
int e_d_quit(FENSTER *f);
int e_d_add_watch(char *str, FENSTER *f);
int e_remove_all_watches(FENSTER *f);
int e_make_watches(FENSTER *f);
int e_edit_watches(FENSTER *f);
int e_delete_watches(FENSTER *f);
int e_d_p_watches(FENSTER *f, int sw);
int e_deb_stack(FENSTER *f);
int e_d_p_stack(FENSTER *f, int sw);
int e_make_stack(FENSTER *f);
int e_breakpoint(FENSTER *f);
int e_remove_breakpoints(FENSTER *f);
int e_make_breakpoint(FENSTER *f, int sw);
int e_exec_deb(FENSTER *f, char *prog);
int e_start_debug(FENSTER *f);
int e_run_debug(FENSTER *f);
int e_deb_run(FENSTER *f);
int e_deb_trace(FENSTER *f);
int e_deb_next(FENSTER *f);
int e_d_goto_cursor(FENSTER *f);
int e_d_finish_func(FENSTER *f);
int e_deb_options(FENSTER *f);
int e_d_step_next(FENSTER *f, int sw);
int e_read_output(FENSTER *f);
int e_d_pr_sig(char *str, FENSTER *f);
int e_make_line_num(char *str, char *file);
int e_make_line_num2(char *str, char *file);
int e_d_goto_break(char *file, int line, FENSTER *f);
int e_d_is_watch(int c, FENSTER *f);
int e_debug_switch(FENSTER *f, int c);
int e_d_putchar(int c);
int e_g_sys_ini(void);
int e_g_sys_end(void);
int e_test_command(char *str);

/**** functions for breakpoints resyncing, reloading etc ****/
int e_brk_schirm(FENSTER *f);
int e_brk_recalc(FENSTER *f,int start,int len);
int e_d_reinit_brks(FENSTER *f,char * prj);

/**** for reloading watches ****/
int e_d_reinit_watches(FENSTER *f,char * prj);

#endif

/* we_gpm.c */
#ifdef HAVE_LIBGPM
int WpeGpmInit(void);
int WpeGpmMouse(int *g);
#endif

/* WeLinux.c */
#ifdef __linux__
int WpeLinuxBioskey(void);
#endif

extern char *e_msg[];
extern char e_we_sw;

#endif

