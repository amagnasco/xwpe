#ifndef WE_XTERM_H
#define WE_XTERM_H

/** \file we_xterm.h */

#include "config.h"
#include "globals.h"
#include "we_main.h"
#include "we_block.h"
#include "we_e_aus.h"

extern char *user_shell;
/** global field necessary for WeXterm only */
extern int old_cursor_x;
/** global field necessary for WeXterm only */
extern int old_cursor_y;

int e_X_sw_color (void);

#ifndef XTERM_CMD
#define XTERM_CMD "xterm"
#endif

#endif
