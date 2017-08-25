#ifndef __MODEL_H
#define __MODEL_H
/** \file model.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"

/*
   General Model Definitions      */

/*  System Definition   */

#define UNIX

/*  Programming Environment  */

/** PROG is always defined.
 *
 * The old code stated that NOPROG was broken. Still the
 * old code could define NOPROG manually using configure -DNOPROG,
 * but the default was PROG.
 *
 * In the new code we always define PROG.
 *
 */
#define PROG
/** DEBUGGER is always defined.
 *
 * The old code stated that NODEBUGGER was broken. For DJGPP
 * NODEBUGGER was defined. We no longer support DJGPP because the
 * previous team already removed DJGPP support partially so there was
 * no base to support it. We removed support for DJGPP completely.
 *
 * In the new code we always define DEBUGGER.
 *
 * */
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
