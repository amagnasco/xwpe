#ifndef UNIXMAKR_H
#define UNIXMAKR_H

/** \file unixmakr.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "config.h"

/**
 * it is unclear to me how these prototypes can point
 * to a definition if configure says there is no strstr() function.
 *
 */
#ifndef HAVE_STRSTR
char *strstr (char *s1, char *s2);
char *getcwd (char *dir, int n);
#endif

#endif
