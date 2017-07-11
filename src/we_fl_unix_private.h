#ifndef WE_FL_UNIX_PRIVATE_H
#define WE_FL_UNIX_PRIVATE_H

#include "config.h"

/* This include is for the header of we_fl_unix only, no other header or source file */

/*   we_fl_unix.c   */
int WpeCreateFileManager(int sw, ECNT *cn, char *dirct);
int WpeDrawFileManager(FENSTER *f);
int WpeManagerFirst(FENSTER *f);
int WpeManager(FENSTER *f);
int WpeSaveAsManager(FENSTER *f);
int WpeExecuteManager(FENSTER *f);

int WpeHandleFileManager(ECNT *cn);
int WpeGrepFile(char *file, char *string, int sw);
int WpeRemove(char *file, FENSTER *f);

int WpeFindWindow(FENSTER *f);
int WpeGrepWindow(FENSTER *f);

struct dirfile *WpeSearchFiles(FENSTER *f, 
                               char *dirct, char *file, char *string,
                               struct dirfile *df, int sw);
int WpeShell(FENSTER *f);
int WpePrintFile(FENSTER *f);
int e_rename(char *file, char *newname, FENSTER *f);
int WpeFileManagerOptions(FENSTER *f);
int WpeShowWastebasket(FENSTER *f);
int WpeDelWastebasket(FENSTER *f);
int WpeQuitWastebasket(FENSTER *f);
int WpeRemoveDir(char *dirct, char *file, FENSTER * f, int rec);
char  *WpeGetWastefile(char *file);
int e_copy(char *file, char *newname, FENSTER *f);
int e_link(char *file, char *newname, FENSTER *f);
int e_duplicate(char *file, FENSTER *f);
int WpeMakeNewDir(FENSTER *f);
int WpeFileDirAttributes(char *filen, FENSTER *f);
int WpeRenameCopyDir(char *dirct, char *file, char *newname, 
                     FENSTER *f, int rec, int sw);
int WpeRenameCopy(char *file, char *newname, FENSTER *f, int sw);
int WpeCopyFileCont(char *oldfile, char *newfile, FENSTER *f);

int WpeDirDelOptions(FENSTER *f);
#ifdef HAVE_SYMLINK
int WpeLinkFile(char *fl, char *ln, int sw, FENSTER *f);
int WpeRenameLink(char *old, char *ln, char *fl, FENSTER *f);
#endif
int e_ed_man(char *str, FENSTER *f);
#endif
