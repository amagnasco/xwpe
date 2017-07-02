/* messages.h                                             */
/* Copyright (C) 1997 Stefan Hille                        */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#ifndef MESSAGES_H
#define MESSAGES_H

/* errors defined in e_msg */
#define	ERR_LOWMEM	0
#define	ERR_VER_OPF	1
#define ERR_READ_OPF	2
#define ERR_MAXWINS	3
#define	ERR_GETSTRING	4
#define	ERR_FOPEN	5
#define ERR_FCLOSE	6
#define	ERR_UNDO	7
#define	ERR_EXEC	8
#define	ERR_COMMAND	9
#define	ERR_HITCR	10
#define	ERR_NOTINSTALL	11
#define	ERR_OPEN_OPF	12
#define	ERR_WRITE_OPF	13
#define	ERR_MSG_SAVE	14
#define ERR_MSG_SAVEALL	15
#define	ERR_NOPRINT	16
#define	ERR_REDO	17
#define ERR_WORKDIRACCESS  18
#define ERR_HOMEDIRACCESS  19
#define ERR_SYSTEM      20
#define ERR_NOWASTE     21
#define ERR_NONEWDIR    22
#define ERR_ACCFILE     23
#define ERR_DELFILE     24
#define ERR_LINKFILE    25
#define ERR_NEWDIREXIST 26
#define ERR_CHGPERM     27
#define ERR_RENFILE     28
#define ERR_OREADFILE   29
#define ERR_OWRITEFILE  30
#define ERR_ALLOC_CBUF  31
#define ERR_INCONSCOPY  32

/* errors defined in e_p_msg */
#define ERR_NO_CFILE    0
#define ERR_PIPEOPEN    1
#define ERR_PROCESS     2
#define ERR_PIPEEXEC    3
#define ERR_IN_COMMAND  4
#define ERR_RETCODE     5
#define ERR_S_NO_CFILE  6
#define ERR_NO_COMPILER 7
#define ERR_NOTHING     8
#define ERR_NOPROJECT   9

/* errors defined in e_d_msg */
#define ERR_CTRLCPRESS  0
#define ERR_QUITDEBUG   1
#define ERR_ENDDEBUG    2
#define ERR_NOTRUNNING  3
#define ERR_NOSYMBOL    4
#define ERR_NOSOURCE    5
#define ERR_STARTDEBUG  6
#define ERR_CANTDEBUG   7
#define ERR_STARTPROG   8
#define ERR_CANTPROG    9
#define ERR_PROGEXIT    10
#define ERR_PROGTERM    11
#define ERR_PROGSIGNAL  12
#define ERR_UNKNOWNBRK  13
#define ERR_CANTFILE    14
#define ERR_CANTPIPE    15
#define ERR_BREAKPOINT  16
#define ERR_SIGNAL      17
#define ERR_PROGEXIT2   18
#define ERR_SIGNAL2     19
#define ERR_INTERRUPT   20
#define ERR_STOPPEDIN   21
#define ERR_BREAKPOINT2 22
#define ERR_NORMALTERM  23
#define ERR_SOFTTERM    24
#define ERR_CONTINUE    25
#define ERR_NOSTACK     26
#define ERR_NOPROCESS   27

#endif

