#ifndef WE_GLOBALS_H
#define WE_GLOBALS_H
/** \file globals.h */

#include "config.h"

/* Checks if programming editor is running (old variable currently used) */
#define WpeIsProg() (e_we_sw & 2)

/* Checks if x windows is running (old variable currently used) */
#define WpeIsXwin() (e_we_sw & 1)

#endif
