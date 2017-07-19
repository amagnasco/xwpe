#ifndef WE_FL_UNIX_H
#define WE_FL_UNIX_H

#include "we_block.h"
#include "we_wind.h"
#include "we_fl_fkt.h"
#include "we_mouse.h"
#include "we_opt.h"
#include "we_debug.h"

#ifdef UNIX

int e_data_eingabe (ECNT * cn);

char *WpeGetCurrentDir (ECNT * cn);
struct dirfile *WpeCreateWorkingDirTree (int sw, ECNT * cn);
char *WpeAssemblePath (char *pth, struct dirfile *cd, struct dirfile *dd,
		       int n, We_window * f);
struct dirfile *WpeGraphicalFileList (struct dirfile *df, int sw, ECNT * cn);
struct dirfile *WpeGraphicalDirTree (struct dirfile *cd, struct dirfile *dd,
				     ECNT * cn);
int e_funct (We_window * f);
int e_funct_in (We_window * f);
int e_data_first (int sw, ECNT * cn, char *nstr);
int e_data_schirm (We_window * f);

#endif

int e_ed_man (unsigned char *str, We_window * f);
int e_rename (char *file, char *newname, We_window * f);
int e_copy (char *file, char *newname, We_window * f);
int e_link (char *file, char *newname, We_window * f);
int e_duplicate (char *file, We_window * f);

int WpeManager (We_window * f);
int WpeManagerFirst (We_window * f);
int WpeHandleFileManager (ECNT * cn);
int WpeCreateFileManager (int sw, ECNT * cn, char *dirct);
int WpeFileManagerOptions (We_window * f);
int WpeDrawFileManager (We_window * f);
int WpeFindWindow (We_window * f);
int WpeGrepWindow (We_window * f);
int WpePrintFile (We_window * f);
int WpeShowWastebasket (We_window * f);
int WpeDelWastebasket (We_window * f);
int WpeQuitWastebasket (We_window * f);
int WpeSaveAsManager (We_window * f);
int WpeExecuteManager (We_window * f);
int WpeRenameCopy (char *file, char *newname, We_window * f, int sw);
int WpeShell (We_window * f);

#endif
