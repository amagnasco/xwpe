#ifndef WE_GPM_H
#define WE_GPM_H

/** \file we_gpm.h */

#include "config.h"

/* we_gpm.c */
#ifdef HAVE_LIBGPM
int WpeGpmMouseInit (void);
int WpeGpmMouse (int *g);
#endif // #ifdef HAVE_LIBGPM

#endif
