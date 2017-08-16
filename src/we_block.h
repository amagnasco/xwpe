#ifndef WE_BLOCK_H
#define WE_BLOCK_H

/** \file we_block.h */

#include "config.h"
#include "we_debug.h"
#include "we_fl_unix.h"
#include "we_opt.h"

int e_blck_del(we_window_t* f);
int e_blck_dup(char* dup, we_window_t* f);
int e_show_clipboard(we_window_t* f);
int e_edt_del(we_window_t* f);
int e_edt_copy(we_window_t* f);
int e_edt_einf(we_window_t* f);
int e_blck_move(we_window_t* f);
void e_move_block(int x, int y, BUFFER* bv, BUFFER* bz, we_window_t* f);
int e_blck_copy(we_window_t* f);
void e_copy_block(int x, int y, BUFFER* buffer_src, BUFFER* buffer_dst,
                  we_window_t* f);
int e_blck_begin(we_window_t* f);
int e_blck_end(we_window_t* f);
int e_blck_hide(we_window_t* f);
int e_find(we_window_t* f);
int e_replace(we_window_t* f);
int e_goto_line(we_window_t* f);
int e_changecase_dialog(we_window_t* f);
int e_blck_to_left(we_window_t* f);
int e_blck_to_right(we_window_t* f);
int e_blck_read(we_window_t* f);
int e_blck_write(we_window_t* f);
int e_repeat_search(we_window_t* f);

#endif
