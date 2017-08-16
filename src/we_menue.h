#ifndef WE_MENUE_H
#define WE_MENUE_H

/** \file we_menue.h */

#include "config.h"
#include "we_block.h"
#include "we_debug.h"
#include "we_edit.h"
#include "we_mouse.h"
#include "we_opt.h"

/*   we_menue.c   */
int WpeHandleMainmenu(int n, we_window_t* f);
int WpeHandleSubmenu(int xa, int ya, int xe, int ye,
                     int nm, OPTK* fopt, we_window_t* f);

#endif
