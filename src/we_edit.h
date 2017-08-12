#ifndef WE_EDIT_H
#define WE_EDIT_H

#include "config.h"
#include "globals.h"
#include "we_block.h"
#include "we_debug.h"
#include "we_e_aus.h"
#include "we_fl_fkt.h"
#include "we_fl_unix.h"
#include "we_menue.h"
#include "we_mouse.h"
#include "we_wind.h"

extern int disable_add_undo;

int e_edit(we_control_t* cn, char* filename);
int e_eingabe(we_control_t* e);
int e_tst_cur(int c, we_control_t* e);
int e_tst_fkt(int c, we_control_t* e);
int e_ctrl_k(we_window_t* f);
int e_ctrl_o(we_window_t* f);
int e_tst_dfkt(we_window_t* f, int c);
int e_blk(int anz, int xa, int ya, int col);
void e_cursor(we_window_t* f, int sw);
int e_del_line(int yd, BUFFER* b, we_screen_t* s);
int e_del_nchar(BUFFER* b, we_screen_t* s, int x, int y, int n);
int e_ins_nchar(BUFFER* b, we_screen_t* sch, unsigned char* s, int xa, int ya,
                int n);
int e_new_line(int yd, BUFFER* b);
int e_put_char(int c, BUFFER* b, we_screen_t* s);
int e_su_lblk(int xa, unsigned char* s);
int e_su_rblk(int xa, unsigned char* s);
void e_zlsplt(we_window_t* f);
void WpeFilenameToPathFile(char* filename, char** path, char** file);
int e_lst_zeichen(int x, int y, int n, int sw, int frb, int max, int iold,
                  int new);
void e_mouse_bar(int x, int y, int n, int sw, int frb);
int e_chr_sp(int x, BUFFER* b, we_window_t* f);
Undo* e_remove_undo(Undo* ud, int sw);
int e_add_undo(int undo_type, BUFFER* b, int x, int y, int n);
int e_make_undo(we_window_t* f);
int e_make_redo(we_window_t* f);
int e_make_rudo(we_window_t* f, int sw);
int e_autosave(we_window_t* f);
char* e_make_postf(char* out, char* name, char* pf);

#endif
