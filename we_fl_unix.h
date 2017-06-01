#ifndef WE_FL_UNIX_H
#define WE_FL_UNIX_H

#include "we_block.h"
#include "we_wind.h"
#include "we_edit.h"
#include "we_fl_fkt.h"
#include "we_mouse.h"
#include "we_opt.h"

#ifdef UNIX
#include "we_fl_unix_private.h"
/** Commented out #endif and placed after prototypes,
 * because the functions for the following prototypes in
 * the we_fl_unix.c source are all included under #ifdef UNIX.
 */
//#endif  

char *WpeGetCurrentDir(ECNT *cn);
struct dirfile *WpeCreateWorkingDirTree(int sw, ECNT *cn);
char *WpeAssemblePath(char *pth, struct dirfile *cd, struct dirfile *dd, int n, 
                      FENSTER *f);
struct dirfile *WpeGraphicalFileList(struct dirfile *df, int sw, ECNT *cn);
struct dirfile *WpeGraphicalDirTree(struct dirfile *cd, struct dirfile *dd,
                                    ECNT *cn);
#endif

#endif
