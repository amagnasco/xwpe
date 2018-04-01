#ifndef WE_BLOCK_H
#define WE_BLOCK_H

/** \file we_block.h */

#include "config.h"
#include "we_debug.h"
#include "we_file_unix.h"
#include "we_opt.h"

int e_block_del(we_window_t* window);
int e_block_dup(char* dup, we_window_t* window);
int e_show_clipboard(we_window_t* window);
int e_edt_del(we_window_t* window);
int e_edt_copy(we_window_t* window);
int e_edt_einf(we_window_t* window);
int e_block_move(we_window_t* window);
void e_move_block(int x, int y, we_buffer_t* bv, we_buffer_t* bz, we_window_t* window);
int e_block_copy(we_window_t* window);
void e_copy_block(int x, int y, we_buffer_t* buffer_src, we_buffer_t* buffer_dst,
                  we_window_t* window);
int e_block_begin(we_window_t* window);
int e_block_end(we_window_t* window);
int e_block_hide(we_window_t* window);
int e_find(we_window_t* window);
int e_replace(we_window_t* window);
int e_goto_line(we_window_t* window);
int e_changecase_dialog(we_window_t* window);
int e_block_to_left(we_window_t* window);
int e_block_to_right(we_window_t* window);
int e_block_read(we_window_t* window);
int e_block_write(we_window_t* window);
int e_repeat_search(we_window_t* window);

#endif
