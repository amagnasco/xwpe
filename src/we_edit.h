#ifndef WE_EDIT_H
#define WE_EDIT_H

/** \file we_edit.h */

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

/**
 *  \brief The number of lines used to allocation of new buffer or window or
 *  for incrementing the number of lines to an already allocated buffer or window.
 */
extern const int MAXLINES;
extern int global_disable_add_undo;

int e_edit(we_control_t* control, char* filename);
int e_eingabe(we_control_t* e);
int e_tst_cur(int c, we_control_t* e);
int e_tst_fkt(int c, we_control_t* e);
int e_ctrl_k(we_window_t* window);
int e_ctrl_o(we_window_t* window);
int e_tst_dfkt(we_window_t* window, int c);
int e_blk(int anz, int xa, int ya, int col);
void e_cursor(we_window_t* window, int sw);
int e_del_line(int yd, we_buffer_t* buffer, we_screen_t* s);
int e_del_nchar(we_buffer_t* buffer, we_screen_t* s, int x, int y, int n);
int e_ins_nchar(we_buffer_t* buffer, we_screen_t* sch, unsigned char* s, int xa, int ya,
                int n);
int e_new_line(int yd, we_buffer_t* buffer);
int e_put_char(int c, we_buffer_t* buffer, we_screen_t* s);
int e_su_lblk(int xa, unsigned char* s);
int e_su_rblk(int xa, unsigned char* s);
void e_zlsplt(we_window_t* window);
void WpeFilenameToPathFile(char* filename, char** path, char** file);
int e_lst_zeichen(int x, int y, int n, int sw, int frb, int max, int iold,
                  int new);
void e_mouse_bar(int x, int y, int n, int sw, int frb);
int e_chr_sp(int x, we_buffer_t* buffer, we_window_t* window);
we_undo_t* e_remove_undo(we_undo_t* undo, int sw);
int e_add_undo(int undo_type, we_buffer_t* buffer, int x, int y, int n);
int e_make_undo(we_window_t* window);
int e_make_redo(we_window_t* window);
int e_autosave(we_window_t* window);
char* e_make_postf(char* out, char* name, char* pf);

#endif
