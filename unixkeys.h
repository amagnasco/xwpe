/* unixkeys.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#ifndef LIBRARY_DIR
#define LIBRARY_DIR "/usr/local/lib/xwpe"
#endif

#ifndef INFO_DIR
#define INFO_DIR "/usr/share/info:/usr/local/info:/usr/info"
#endif

#define XWPE_HOME   ".xwpe"
#define WASTEBASKET ".wastebasket"

#define SYNTAX_FILE "syntax_def"
#define HELP_FILE   "help.xwpe"
#define OPTION_FILE "xwperc"

#define BUFFER_NAME "Buffer"   /*  Clipboard in Buffer changed  */
#define SUDIR "*"
#define MAXREC 15
#ifndef DEF_SHELL
#define DEF_SHELL "sh"
#endif

/*   Characters  */

extern char MCI, MCA, RD1, RD2, RD3, RD4, RD5, RD6;
extern char RE1, RE2, RE3, RE4, RE5, RE6, WBT;

#define MCU '^'
#define MCD 'v'
#define MCL '<'
#define MCR '>'
#define WZY 'z'
#define WZN 'Z'
#define PWR '$'
#define PNL '\\'
#define SCR 20
#define SCD 16
#define WSW 'v'
#define DIRC '/'
#define DIRS "/"
#define SWSYM '+'
#define PTHD ':'
/*  Special colors  */
#define SHDCOL 8

#define STOP  421
#define AGAIN 422
#define PROPS 423
#define UNDO  424
#define FRONT 425
#define COPY  426
#define OPEN  427
#define PASTE 428
#define FID   429
#define CUT   430
#define HELP  431

#define DWR 20       /*  ctrl t  */
#define DWL 18       /*  ctrl r  */
#define DNDL 17      /*  ctrl q  */
#define DGZ 25       /*  ctrl y  */

#define WPE_CR 13
#define WPE_WR 10
#define WPE_ESC 27
#define WPE_DC 8
#define WPE_TAB 9
#define WPE_BTAB 28 /*First value which looks suitable for me*/

#define SCLE CLE+512
#define SCRI CRI+512
#define SCUP CUP+512
#define SCDO CDO+512

