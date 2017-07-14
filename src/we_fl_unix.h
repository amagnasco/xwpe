#ifndef WE_FL_UNIX_H
#define WE_FL_UNIX_H

#include "we_block.h"
#include "we_wind.h"
#include "we_fl_fkt.h"
#include "we_mouse.h"
#include "we_opt.h"
#include "we_debug.h"

#ifdef UNIX
#include "we_fl_unix_private.h"

int e_data_eingabe (ECNT * cn);

char *WpeGetCurrentDir (ECNT * cn);
struct dirfile *WpeCreateWorkingDirTree (int sw, ECNT * cn);
char *WpeAssemblePath (char *pth, struct dirfile *cd, struct dirfile *dd,
		       int n, FENSTER * f);
struct dirfile *WpeGraphicalFileList (struct dirfile *df, int sw, ECNT * cn);
struct dirfile *WpeGraphicalDirTree (struct dirfile *cd, struct dirfile *dd,
				     ECNT * cn);
/*   we_fl_unix.c  */

int e_funct (FENSTER * f);
int e_funct_in (FENSTER * f);
int e_data_first (int sw, ECNT * cn, char *nstr);
int e_data_schirm (FENSTER * f);

#endif

#endif
