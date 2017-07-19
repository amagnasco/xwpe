#ifndef WE_FL_UNIX_PRIVATE_H
#define WE_FL_UNIX_PRIVATE_H

#include "config.h"

/* This include is for the header of we_fl_unix only, no other header or source file */

/*   we_fl_unix.c   */
int WpeCreateFileManager (int sw, ECNT * cn, char *dirct);
int WpeDrawFileManager (We_window * f);
int WpeManagerFirst (We_window * f);
int WpeManager (We_window * f);
int WpeSaveAsManager (We_window * f);
int WpeExecuteManager (We_window * f);

int WpeHandleFileManager (ECNT * cn);
int WpeGrepFile (char *file, char *string, int sw);
int WpeRemove (char *file, We_window * f);

int WpeFindWindow (We_window * f);
int WpeGrepWindow (We_window * f);

struct dirfile *WpeSearchFiles (We_window * f,
				char *dirct, char *file, char *string,
				struct dirfile *df, int sw);
int WpeShell (We_window * f);
int WpePrintFile (We_window * f);
int e_rename (char *file, char *newname, We_window * f);
int WpeFileManagerOptions (We_window * f);
int WpeShowWastebasket (We_window * f);
int WpeDelWastebasket (We_window * f);
int WpeQuitWastebasket (We_window * f);
int WpeRemoveDir (char *dirct, char *file, We_window * f, int rec);
char *WpeGetWastefile (char *file);
int e_copy (char *file, char *newname, We_window * f);
int e_link (char *file, char *newname, We_window * f);
int e_duplicate (char *file, We_window * f);
int WpeMakeNewDir (We_window * f);
int WpeFileDirAttributes (char *filen, We_window * f);
int WpeRenameCopyDir (char *dirct, char *file, char *newname,
		      We_window * f, int rec, int sw);
int WpeRenameCopy (char *file, char *newname, We_window * f, int sw);
int WpeCopyFileCont (char *oldfile, char *newfile, We_window * f);

int WpeDirDelOptions (We_window * f);
#ifdef HAVE_SYMLINK
int WpeLinkFile (char *fl, char *ln, int sw, We_window * f);
int WpeRenameLink (char *old, char *ln, char *fl, We_window * f);
#endif
int e_ed_man (unsigned char *str, We_window * f);
#endif
