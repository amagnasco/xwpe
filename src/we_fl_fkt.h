#ifndef WE_FL_FKT_H
#define WE_FL_FKT_H
/** \file we_fl_fkt.h */

#include "config.h"
#include "we_debug.h"
#include "we_e_aus.h"
#include "we_edit.h"
#include "we_fl_unix.h"
#include "we_mouse.h"
#include "we_opt.h"
#include <stdlib.h>

extern char* info_file;
extern char* e_tmp_dir;

/*   we_fl_fkt.c   */
char* e_mkfilename(char* dr, char* fn);
we_point_t e_readin(int i, int j, FILE* fp, we_buffer_t* b, char* sw);
int e_new(we_window_t* window);
int e_m_save(we_window_t* window);
int e_save(we_window_t* window);
int e_saveall(we_window_t* window);
int e_quit(we_window_t* window);
int e_write(int xa, int ya, int xe, int ye, we_window_t* window, int backup);
char* e_new_qual(char* s, char* ns, char* sb);
char* e_bakfilename(char* s);
int freedf(struct dirfile* df);
int e_file_window(int sw, FLWND* fw, int ft, int fz);
int e_pr_file_window(FLWND* fw, int c, int sw, int ft, int fz, int fs);
int e_help_last(we_window_t* window);
int e_help_comp(we_window_t* window);
int e_help(we_window_t* window);
int e_help_loc(we_window_t* window, int sw);
int e_help_free(we_window_t* window);
int e_help_ret(we_window_t* window);
int e_topic_search(we_window_t* window);

#endif
