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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <limits.h>

#ifdef UNIX
#include "unixmakr.h"
#include "unixkeys.h"

extern int MAXSLNS, MAXSCOL, MENOPT;
#define MAXEDT 35
#endif // #ifdef UNIX

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

#ifdef UNIX

int e_put_pic_xrect(PIC *pic);
int e_get_pic_xrect(int xa, int ya, int xe, int ye, PIC *pic);

#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
int e_make_xrect(int xa, int ya, int xe, int ye, int sw);
int e_make_xrect_abs(int xa, int ya, int xe, int ye, int sw);
#else
#define e_make_xrect(a,b,c,d,e)
#define e_make_xrect_abs(a,b,c,d,e)
#endif // #if defined(NEWSTYLE) && !defined(NO_XWINDOWS)

#endif // #ifdef UNIX

#endif // #ifndef __EDIT_H

