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
#include "config.h"
#include "model.h"
#include "we_find.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef UNIX
#include "unixkeys.h"
#include "unixmakr.h"

extern int MAXSLNS, MAXSCOL, MENOPT;
#define MAXEDT 35 // Maximum number of editting windows
#endif		  // #ifdef UNIX

#define MAXLINES 10
#define MAXCOLUM 120

#define WPE_NOBACKUP 1
#define WPE_BACKUP 0

/** DTMD is possibly short for Die Terminal Meta Data (German) or meta data for the terminal window */
/** Normal text file */
#define DTMD_NORMAL 'n' /* Normal text file */
/** MS-DOS text file */
#define DTMD_MSDOS 'm' /* MS-DOS text file */
/** Help window */
#define DTMD_HELP 'h' /* Help window */
/** Data/project window */
#define DTMD_DATA 'D' /* Data/project windows */
/** File manager */
#define DTMD_FILEMANAGER 'F' /* File manager */
/** File/directory dropdown of previous files/directories on the file manager */
#define DTMD_FILEDROPDOWN 'M'

/** test to determine: is this a edittable text window or not? */
#define DTMD_ISTEXT(x) (x > 'Z')
/** test to determine: is this window marakable? */
#define DTMD_ISMARKABLE(x) (x > DTMD_HELP) /* Means end marks can be shown */

struct dirfile
{
    int nr_files; /* number elements in the list */
    char** name;  /* the list elements */
};

typedef struct PNT
{
    int x;
    int y;
} POINT;

typedef struct CLR
{
    int f;
    int b;
    int fb;
} COLOR;

typedef struct view_struct
{
    char* p;
    POINT a;
    POINT e;
} view;

typedef struct frb
{
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
    COLOR ct; /* normal program text */
    COLOR cr; /* reserved keywords in program */
    COLOR ck; /* constants in program */
    COLOR cp; /* preprocessor command */
    COLOR cc; /* comments in program */
    char dc;  /* desktop fill character */
    char ws;
} we_colorset_t;

typedef struct undo
{
    int type;
    POINT b, a, e;
    union
    {
        char c;
        void* pt;
    } u;
    struct undo* next;
} Undo;

typedef struct STR
{
    unsigned char* s;
    int len; /* Length of string not counting '\n' at the end */
    size_t nrc;
    /*int size; */ /* Memory allocated for the string */
} STRING;

typedef struct BFF
{
    STRING* bf;  /* bf[i] is the i-th line of the buffer */
    POINT b;     /* cursor coordinates in window (at least in some contexts) */
    POINT mx;    /* maximum column and line */
    int mxlines; /* number of lines */
    int cl, clsv;
    Undo *ud, *rd;
    struct CNT* cn;
    struct FNST* f;
    we_colorset_t* fb;
} BUFFER;

typedef struct SCHRM
{
    POINT mark_begin;
    POINT mark_end;
    POINT ks;
    /** 10 marks you can set with Ctrl-K n and get to with Ctrl-O n */
    POINT pt[9];
    /** Is starting point of something related to marks (needs more research) */
    POINT fa;
    /** Is ending point of something related to marks (needs more research) */
    POINT fe;
    /** starting point of something */
    POINT a;
    /** ending point of something */
    POINT e;
    POINT c;
    we_colorset_t* fb;
#ifdef DEBUGGER
    POINT da, de;
    int* brp;
#endif
} we_screen;

typedef struct OPTION
{
    char* t;
    int x;
    int s;
    int as;
} OPT;

typedef struct WOPTION
{
    char* t;
    int x, s, n, as;
} WOPT;

typedef struct OPTKAST
{
    char* t;
    int x;
    char o;
    int (*fkt)(struct FNST*);
} OPTK;

typedef struct
{
    int position;
    int width;
    int no_of_items;
    OPTK* menuitems;
} MENU;

typedef struct FNST
{
    POINT a; /* start corner of the box */
    POINT e; /* other corner of the box */
    POINT sa;
    POINT se;
    char zoom;
    we_colorset_t* fb; /* color scheme */
    view* pic;	 /* picture save below the box ??? */
    char* dirct;       /* working/actual directory */
    char* datnam;      /* window header text */
    int winnum;
    char ins;
    char dtmd; /* (See DTMD_* defines) */
    int save;
    char* hlp_str;
    WOPT* blst; /* status line text */
    int nblst;  /* no of options in the status line */
    int filemode, flg;
    int* c_sw;
    struct wpeSyntaxRule* c_st;
    struct CNT* ed; /* control structure */
    struct BFF* b;
    struct SCHRM* s;
    FIND fd;
} we_window_t;

typedef struct CNT
{
    int major, minor, patch; /* Version of option file. */
    int maxcol, tabn;
    int maxchg, numundo;
    int flopt, edopt;
    int mxedt;		 /* max number of editing windows */
    int curedt;		 /* currently active window */
    int edt[MAXEDT + 1]; /* 1 <= window IDs <= MAXEDT, arbitrary order */
    int autoindent;
    char* print_cmd;
    char* dirct; /* current directory */
    char *optfile, *tabs;
    struct dirfile *sdf, *rdf, *fdf, *ddf, *wdf, *hdf, *shdf;
    FIND fd;
    we_colorset_t* fb;
    we_window_t* f[MAXEDT + 1];
    char dtmd, autosv;
} ECNT;

/* structure for the windows in the file manager ??? */
typedef struct fl_wnd
{
    int xa, ya; /* its own box corner ??? */
    int xe, ye;
    int ia, ja;
    int nf; /* selected field in dirfile df struct */
    int nxfo, nyfo;
    int mxa, mya; /* parent box corners ??? */
    int mxe, mye;
    int srcha;
    struct dirfile* df; /* directory tree or file list */
    we_window_t* f;     /* the window itself */
} FLWND;

typedef struct FLBFF
{
    struct dirfile* cd; /* current directory */
    struct dirfile* dd; /* list of directories in the current dir. */
    struct dirfile* df; /* list of files in the current dir. */
    struct fl_wnd* fw;  /* window for file list */
    struct fl_wnd* dw;  /* window for dir tree */
    char* rdfile;       /* file pattern entered for searching */
    char sw;
    int xfa, xfd, xda, xdd;
} FLBFFR;

typedef struct
{
    int x, y;
    char* txt;
} W_O_TXTSTR;

typedef struct
{
    int xt, yt, xw, yw, nw, wmx, nc, sw;
    char* header;
    char* txt;
    struct dirfile** df;
} W_O_WRSTR;

typedef struct
{
    int xt, yt, xw, yw, nw, wmx, nc, num, sw;
    char* header;
} W_O_NUMSTR;

typedef struct
{
    int x, y, nc, sw, num;
    char* header;
} W_O_SSWSTR;

typedef struct
{
    int x, y, nc, sw;
    char* header;
} W_O_SPSWSTR;

typedef struct
{
    int num, np;
    W_O_SPSWSTR** ps;
} W_O_PSWSTR;

typedef struct
{
    int x, y, nc, sw;
    char* header;
    int (*fkt)(we_window_t* f);
} W_O_BTTSTR;

typedef struct
{
    int xa, ya, xe, ye, bgsw, crsw;
    int frt, frs, ftt, fts, fst, fss, fsa, fbt;
    int fbs, fbz, fwt, fws;
    int tn, sn, pn, bn, wn, nn;
    char* name;
    view* pic;
    W_O_TXTSTR** tstr;
    W_O_SSWSTR** sstr;
    W_O_PSWSTR** pstr;
    W_O_BTTSTR** bstr;
    W_O_WRSTR** wstr;
    W_O_NUMSTR** nstr;
    we_window_t* f;
} W_OPTSTR;

typedef struct wpeOptionSection
{
    char* section;
    int (*function)(ECNT* cn, char* section, char* option, char* value);
} WpeOptionSection;

#ifdef UNIX

int e_put_pic_xrect(view* pic);
int e_get_pic_xrect(int xa, int ya, int xe, int ye, view* pic);

#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
int e_make_xrect(int xa, int ya, int xe, int ye, int sw);
int e_make_xrect_abs(int xa, int ya, int xe, int ye, int sw);
#else
#define e_make_xrect(a, b, c, d, e)
#define e_make_xrect_abs(a, b, c, d, e)
#endif // #if defined(NEWSTYLE) && !defined(NO_XWINDOWS)

#endif // #ifdef UNIX

#endif // #ifndef __EDIT_H
