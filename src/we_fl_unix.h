#ifndef WE_FL_UNIX_H
#define WE_FL_UNIX_H

#include "config.h"
#include "we_block.h"
#include "we_debug.h"
#include "we_fl_fkt.h"
#include "we_mouse.h"
#include "we_opt.h"
#include "we_wind.h"

#ifdef UNIX

int e_data_eingabe(we_control_t* cn);

char* WpeGetCurrentDir(we_control_t* cn);
struct dirfile* WpeCreateWorkingDirTree(int sw, we_control_t* cn);
char* WpeAssemblePath(char* pth, struct dirfile* cd, struct dirfile* dd,
                      int n, we_window_t* f);
struct dirfile* WpeGraphicalFileList(struct dirfile* df, int sw, we_control_t* cn);
struct dirfile* WpeGraphicalDirTree(struct dirfile* cd, struct dirfile* dd,
                                    we_control_t* cn);
int e_funct(we_window_t* f);
int e_funct_in(we_window_t* f);
int e_data_first(int sw, we_control_t* cn, char* nstr);
int e_data_schirm(we_window_t* f);

#endif

int e_ed_man(unsigned char* str, we_window_t* f);
int e_rename(char* file, char* newname, we_window_t* f);
int e_copy(char* file, char* newname, we_window_t* f);
int e_link(char* file, char* newname, we_window_t* f);
int e_duplicate(char* file, we_window_t* f);

int WpeManager(we_window_t* f);
int WpeManagerFirst(we_window_t* f);
int WpeHandleFileManager(we_control_t* cn);
int WpeCreateFileManager(int sw, we_control_t* cn, char* dirct);
int WpeFileManagerOptions(we_window_t* f);
int WpeDrawFileManager(we_window_t* f);
int WpeFindWindow(we_window_t* f);
int WpeGrepWindow(we_window_t* f);
int WpePrintFile(we_window_t* f);
int WpeShowWastebasket(we_window_t* f);
int WpeDelWastebasket(we_window_t* f);
int WpeQuitWastebasket(we_window_t* f);
int WpeSaveAsManager(we_window_t* f);
int WpeExecuteManager(we_window_t* f);
int WpeRenameCopy(char* file, char* newname, we_window_t* f, int sw);
int WpeShell(we_window_t* f);

#endif
