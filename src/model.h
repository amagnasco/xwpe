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

/*  Programming Environment  */

#define PROG
#define DEBUGGER

/*  XWindow Definitions  */

#if !defined(NO_XWINDOWS)  || defined (HAVE_LIBGPM)
#define MOUSE   1		/*  activate mouse  */
#else
#define MOUSE   0		/*  deactivate mouse  */
#endif

/*  Newstyle only for XWindow   */
#if !defined(NO_XWINDOWS)
#define NEWSTYLE
#endif

#endif
