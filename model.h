#ifndef __MODEL_H
#define __MODEL_H
/* model.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

/*
   General Model Definitions      */

/*  System Definition   */

#define UNIX
#undef DJGPP

/*  Effects of #Defines (do not change)  */

#define CHECKHEADER

#ifdef DJGPP
#define NODEBUGGER
#undef NOPROG
#define NO_XWINDOWS
#define NOPRINTER
#define NONEWSTYLE
#define MOUSE 1
#ifndef UNIX
#define UNIX
#endif
#endif

/*  XWindow Definitions  */

#if defined(DJGPP) || !defined(NO_XWINDOWS)  || defined (HAVE_LIBGPM)
#define MOUSE   1        /*  activate mouse  */
#else
#define MOUSE   0        /*  deactivate mouse  */
#endif

/*  Programming Environment  */

#ifndef NOPROG
#define PROG
#ifndef NODEBUGGER
#define DEBUGGER
#endif
#endif

/*  Newstyle only for XWindow   */
#if !defined(NO_XWINDOWS) && !defined(NONEWSTYLE)
#define NEWSTYLE
#endif

#endif
