#ifndef WE_HFKT_H
#define WE_HFKT_H

/** \file we_hfkt.h */

#include "config.h"
#include "we_edit.h"
#include "we_opt.h"

/*   we_hfkt.c   */
int e_str_len(unsigned char* s);

int e_num_kst(char* s, int num, int max, we_window_t* f, int n, int sw);
we_color_t e_s_x_clr(int f, int b);
we_color_t e_n_x_clr(int fb);
#ifdef UNIX
we_color_t e_s_t_clr(int f, int b);
we_color_t e_n_t_clr(int fb);
#endif
we_point_t e_set_pnt(int x, int y);
int e_pr_uul(we_colorset_t* fb);

#endif
