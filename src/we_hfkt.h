#ifndef WE_HFKT_H
#define WE_HFKT_H

#include "config.h"
#include "we_edit.h"
#include "we_opt.h"

/*   we_hfkt.c   */
int e_str_len(unsigned char* s);

int e_num_kst(char* s, int num, int max, we_window_t* f, int n, int sw);
COLOR e_s_x_clr(int f, int b);
COLOR e_n_x_clr(int fb);
#ifdef UNIX
COLOR e_s_t_clr(int f, int b);
COLOR e_n_t_clr(int fb);
#endif
POINT e_set_pnt(int x, int y);
int e_pr_uul(we_colorset_t* fb);

#endif
