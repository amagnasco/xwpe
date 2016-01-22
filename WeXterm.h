#ifndef __WEXTERM_H
#define __WEXTERM_H
/*-------------------------------------------------------------------------*\
  <WeXterm.h> -- Header file of Xwpe routines for X window support

  Date      Programmer  Description
  05/04/97  Dennis      Created for xwpe reorganization.
\*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include <X11/Xlib.h>
#include "Xwpe.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Defines
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifndef XTERM_CMD
#define XTERM_CMD "xterm"
#endif

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  New Types
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct wpeXStruct {
 Display *display;
 int screen;
 Window window;
 GC gc;
 XFontStruct *font;
 Atom delete_atom, protocol_atom, selection_atom, text_atom, property_atom;
 int font_height, font_width;
 int altmask;
 int colors[16];
 WpeMouseShape shape_list[2];
 char *selection;
} WpeXStruct;

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Global Variables
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
extern WpeXStruct WpeXInfo;


void WpeXInit(int *argc, char **argv);
void WpeXMouseChangeShape(WpeMouseShape new_shape);
void WpeXMouseRestoreShape();

#ifdef __cplusplus
}
#endif

#endif

