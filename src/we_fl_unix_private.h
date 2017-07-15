#ifndef WE_FL_UNIX_PRIVATE_H
#define WE_FL_UNIX_PRIVATE_H

#include "config.h"

/* This include is for the header of we_fl_unix only, no other header or source file */

/*   we_fl_unix.c   */
int WpeCreateFileManager (int sw, ECNT * cn, char *dirct);
int WpeDrawFileManager (we_window * f);
int WpeManagerFirst (we_window * f);
int WpeManager (we_window * f);
int WpeSaveAsManager (we_window * f);
int WpeExecuteManager (we_window * f);

int WpeHandleFileManager (ECNT * cn);
int WpeGrepFile (char *file, char *string, int sw);
int WpeRemove (char *file, we_window * f);

int WpeFindWindow (we_window * f);
int WpeGrepWindow (we_window * f);

struct dirfile *WpeSearchFiles (we_window * f,
				char *dirct, char *file, char *string,
				struct dirfile *df, int sw);
int WpeShell (we_window * f);
int WpePrintFile (we_window * f);
int e_rename (char *file, char *newname, we_window * f);
int WpeFileManagerOptions (we_window * f);
int WpeShowWastebasket (we_window * f);
int WpeDelWastebasket (we_window * f);
int WpeQuitWastebasket (we_window * f);
int WpeRemoveDir (char *dirct, char *file, we_window * f, int rec);
char *WpeGetWastefile (char *file);
int e_copy (char *file, char *newname, we_window * f);
int e_link (char *file, char *newname, we_window * f);
int e_duplicate (char *file, we_window * f);
int WpeMakeNewDir (we_window * f);
int WpeFileDirAttributes (char *filen, we_window * f);
int WpeRenameCopyDir (char *dirct, char *file, char *newname,
		      we_window * f, int rec, int sw);
int WpeRenameCopy (char *file, char *newname, we_window * f, int sw);
int WpeCopyFileCont (char *oldfile, char *newfile, we_window * f);

int WpeDirDelOptions (we_window * f);
#ifdef HAVE_SYMLINK
int WpeLinkFile (char *fl, char *ln, int sw, we_window * f);
int WpeRenameLink (char *old, char *ln, char *fl, we_window * f);
#endif
int e_ed_man (unsigned char *str, we_window * f);
#endif
