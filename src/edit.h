#ifndef __EDIT_H
#define __EDIT_H
/** \file edit.h */
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
/** Maximum number of editting windows */
#define MAXEDT 35
#endif		  // #ifdef UNIX

/**
 *  \brief The number of lines used to allocation of new buffer or window or
 *  for adding lines to an already allocated buffer or window.
 */
#define MAXLINES 10
/** The number of columns used for the initial control
 *
 *  This is only used in the function ECNT_Init in we_control.c
 *  \todo: check how important this define is and whether it should be in we_control.h
 *
 * */
#define MAXCOLUM 120

/**
 * \brief WPE_NOBACKUP is a number to determine not to make a backup.
 *
 * The meaning of the number is to make no backup.
 * It is only used in the function e_autosave of we_edit.c.
 */
#define WPE_NOBACKUP 1
/**
 * \brief WPE_BACKUP is a number to determine to make a backup when writing to a file.
 *
 */
#define WPE_BACKUP 0

/** DTMD is possibly short for Die Terminal Meta Data (German) or meta data for the terminal window
    Normal text file */
#define DTMD_NORMAL 'n'
/** MS-DOS text file */
#define DTMD_MSDOS 'm'
/** Help window */
#define DTMD_HELP 'h'
/** Data/project window */
#define DTMD_DATA 'D'
/** File manager */
#define DTMD_FILEMANAGER 'F'
/** File/directory dropdown of previous files/directories on the file manager */
#define DTMD_FILEDROPDOWN 'M'

/** test to determine: is this a edittable text window or not? */
#define DTMD_ISTEXT(x) (x > 'Z')
/** test to determine: is this window markable? */
#define DTMD_ISMARKABLE(x) (x > DTMD_HELP) /* Means end marks can be shown */

struct dirfile
{
    /** The number of elements in the list */
    int nr_files;
    /** The file names in the directory */
    char** name;
};

/**
 * \brief The x and y coordinates in a window or on a screen.
 *
 * The x coordinate represents the offset of the column (starts at zero).
 * The y coordinate represents the offset of the row (starts at zero).
 *
 */
typedef struct we_point_struct
{
    int x;
    int y;
} we_point_t;


/**
 * \brief defines textcolor, foreground and background color
 */
typedef struct we_color_struct
{
    /** The foreground color */
    int f;
    /** The background color */
    int b;
    /** The text color */
    int fb;
} we_color_t;

/**
 * Defines a view with start and endpoint
 */
typedef struct view_struct
{
    char* p;
    /** The starting point of the view */
    we_point_t a;
    /** The end point of the view */
    we_point_t e;
} we_view_t;

typedef struct we_colorset_struct
{
    we_color_t er;   /**< editor window border and text */
    we_color_t es;   /**< special signs (maximize/kill) on editor window border */
    we_color_t et;   /**< normal text in editor window */
    we_color_t ez;   /**< marked text in editor window */
    we_color_t ek;   /**< found/marked word in editor window */
    we_color_t em;   /**< scrollbar */
    we_color_t hh;   /**< Help header */
    we_color_t hb;   /**< button in Help */
    we_color_t hm;   /**< marked word in Help */
    we_color_t db;   /**< breakpoint set */
    we_color_t dy;   /**< stop at breakpoint */
    we_color_t mr;   /**< submenu border */
    we_color_t ms;   /**< menu shortkey text */
    we_color_t mt;   /**< menu text */
    we_color_t mz;   /**< active menu text */
    we_color_t df;   /**< desktop */
    we_color_t nr;   /**< message window border and text */
    we_color_t ne;   /**< special signs (maximize/kill) on message window border */
    we_color_t nt;   /**< normal text for widgets in message window */
    we_color_t nsnt; /**< widget selector shortkey in message window */
    we_color_t fr;   /**< passive entry */
    we_color_t fa;   /**< active entry */
    we_color_t ft;   /**< normal data text */
    we_color_t fz;   /**< active, marked data text */
    we_color_t frft; /**< passive, marked data text */
    we_color_t fs;   /**< passive switch */
    we_color_t nsft; /**< switch selector shortkey */
    we_color_t fsm;  /**< active switch */
    we_color_t nz;   /**< normal/passive button text */
    we_color_t ns;   /**< button shortkey text */
    we_color_t nm;   /**< active button text */
    we_color_t of;
    we_color_t ct;   /**< normal program text */
    we_color_t cr;   /**< reserved keywords in program */
    we_color_t ck;   /**< constants in program */
    we_color_t cp;   /**< preprocessor command */
    we_color_t cc;   /**< comments in program */
    char dc;         /**< desktop fill character */
    char ws;
} we_colorset_t;

/**
 *  \struct undo
 *
 *  \brief The undo struct functions for redo as well.
 *	Contains points b, a and e. plus either a char c
 *  or a pointer to void.
 *
 *  The types that undo/redo struct exist as are:
 *
 *  - 'a' Undo / redo struct for character insertion.
 *  - 'c' Undo / redo struct for block copy.
 *  - 'd' Undo / redo struct for block delete.
 *  - 'l' Undo / redo struct for line delete.
 *  - 'p' Undo / redo struct for character overwrite ('put').
 *  - 'r' Undo / redo struct for character deletion.
 *  - 's' Undo / redo struct for search / replace.
 *  - 'v' Undo / redo struct for block paste.
 *  - 'y' Undo / redo struct for line delete ( counters undo of 'l'?)
 *
 *
 */
typedef struct we_undo_struct
{
    int type;
    we_point_t b; /* the starting (?) (x, y) cursor position */
    /**
     * Marks
     *  'a' (x,y)=begin of characters to be copied
     *  'r' a.x=len(search_string) a.y=len(result_string)
     *	's' a.x=len(search_string) a.y=len(result_string)
     */
    we_point_t a;
    /**
     * Marks
     *  'a' (x,y)=end of characters to be copied
     *  'r' no value
     *  's' no value
     */
    we_point_t e;
    union
    {
        /**
         * 'p' one character
         *
         */
        char c;
        /**
         *   'd' pointer to a deleted buffer
         *   'l' pointer to start of the line
         *   'r' pointer to string
         *   's' pointer to string
         */
        void* pt;
    } u;
    /**
     * Pointer to the next undo or redo struct
     */
    struct we_undo_struct* next;
} we_undo_t;

typedef struct STR
{
    unsigned char* s;
    int len;       /* Length of string not counting '\n' at the end */
    size_t nrc;
    /*int size; */ /* Memory allocated for the string */
} STRING;

typedef struct BFF
{
    STRING* bf;    /* bf[i] is the i-th line of the buffer */
    we_point_t b;  /* cursor coordinates in window (at least in some contexts) */
    we_point_t mx; /* maximum column and line */
    int mxlines;   /* number of lines */
    int cl, clsv;
    we_undo_t *ud, *rd;	/* pointers to undo and redo structs */
    struct CNT* cn;
    struct FNST* f;
    we_colorset_t* colorset;
} BUFFER;

typedef struct SCHRM
{
    we_point_t mark_begin;
    we_point_t mark_end;
    we_point_t ks;
    /** 10 marks you can set with Ctrl-K n and get to with Ctrl-O n */
    we_point_t pt[9];
    /** Is starting point of something related to marks (needs more research) */
    we_point_t fa;
    /** Is ending point of something related to marks (needs more research) */
    we_point_t fe;
    /** starting point of something */
    we_point_t a;
    /** ending point of something */
    we_point_t e;
    we_point_t c;
    we_colorset_t* colorset;
#ifdef DEBUGGER
    we_point_t da, de;
    int* brp;
#endif
} we_screen_t;

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
    we_point_t a; /**< start corner of the box */
    we_point_t e; /**< other corner of the box */
    we_point_t sa;
    we_point_t se;
    char zoom;
    we_colorset_t* colorset; /**< color scheme */
    we_view_t* view;
    char* dirct;       /**< working/actual directory */
    char* datnam;      /**< window header text */
    int winnum;
    char ins;
    char dtmd;			/**< (See DTMD_* defines) */
    int save;
    char* hlp_str;
    WOPT* blst;			/**< status line text */
    int nblst;			/**< no of options in the status line */
    int filemode, flg;
    int* c_sw;
    struct wpeSyntaxRule* c_st;
    struct CNT* ed;		/**< edit control structure */
    struct BFF* b;		/**< Buffer */
    struct SCHRM* s;	/**< screen */
    FIND find;
} we_window_t;

typedef struct CNT
{
    int major, minor, patch; /**< Version of option file. */
    int maxcol, tabn;
    int maxchg, numundo;
    int flopt, edopt;
    int mxedt;		 /**< max number of editing windows */
    int curedt;		 /**< currently active window */
    int edt[MAXEDT + 1]; /**< 1 <= window IDs <= MAXEDT, arbitrary order */
    int autoindent;
    char* print_cmd;
    char* dirct; /**< current directory */
    char *optfile, *tabs;
    struct dirfile *sdf, *rdf, *fdf, *ddf, *wdf, *hdf, *shdf;
    FIND find;
    we_colorset_t* colorset;
    we_window_t* f[MAXEDT + 1];
    char dtmd, autosv;
} we_control_t;

/* structure for the windows in the file manager ??? */
typedef struct fl_wnd
{
    int xa, ya; /**< its own box corner ??? */
    int xe, ye;
    int ia, ja;
    int nf; /**< selected field in dirfile df struct */
    int nxfo, nyfo;
    int mxa, mya; /**< parent box corners ??? */
    int mxe, mye;
    int srcha;
    struct dirfile* df; /**< directory tree or file list */
    we_window_t* f;     /**< the window itself */
} FLWND;

typedef struct FLBFF
{
    struct dirfile* cd; /**< current directory */
    struct dirfile* dd; /**< list of directories in the current dir. */
    struct dirfile* df; /**< list of files in the current dir. */
    struct fl_wnd* fw;  /**< window for file list */
    struct fl_wnd* dw;  /**< window for dir tree */
    char* rdfile;       /**< file pattern entered for searching */
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
    int xa, ya;			/**< Upperleft corner */
    int xe, ye;			/**< Lowerright corner */
    int bgsw;
    int crsw;
    int frt, frs;
    int ftt, fts;
    int fst, fss;
    int fsa, fbt;
    int fbs, fbz;
    int fwt, fws;
    int tn;				/**< The number of W_O_TXTSTR in tstr */
    int sn;			    /**< The number of W_O_SSWSTR in sstr */
    int pn;				/**< The number of W_O_PSWSTR in pstr */
    int bn;				/**< The number of W_O_BTTSTR in bstr */
    int wn;				/**< The number of W_O_WRSTR in wstr */
    int nn;				/**< The number of W_O_NUMSTR in nstr */
    char* name;			/**< name of the dialog: "Replace" or "Find" */
    we_view_t* view;
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
    int (*function)(we_control_t* cn, char* section, char* option, char* value);
} WpeOptionSection;

#ifdef UNIX

int e_put_pic_xrect(we_view_t* view);
int e_get_pic_xrect(int xa, int ya, int xe, int ye, we_view_t* view);

#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
int e_make_xrect(int xa, int ya, int xe, int ye, int sw);
int e_make_xrect_abs(int xa, int ya, int xe, int ye, int sw);
#else
#define e_make_xrect(a, b, c, d, e)
#define e_make_xrect_abs(a, b, c, d, e)
#endif // #if defined(NEWSTYLE) && !defined(NO_XWINDOWS)

#endif // #ifdef UNIX

#endif // #ifndef __EDIT_H
