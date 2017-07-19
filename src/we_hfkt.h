#ifndef WE_HFKT_H
#define WE_HFKT_H

#include "we_edit.h"
#include "we_opt.h"

/*   we_hfkt.c   */
int e_strstr (int x, int n, unsigned char *s, unsigned char *f);
int e_ustrstr (int x, int n, unsigned char *s, unsigned char *f);
int e_urstrstr (int start_offset, int end_offset,
		unsigned char *search_string,
		unsigned char *regular_expression, size_t * end_match);
int e_rstrstr (size_t x, size_t n, unsigned char *s, unsigned char *f,
	       size_t * end_match_str, _Bool case_sensitive);
int e_str_len (unsigned char *s);

int e_num_kst (char *s, int num, int max, we_window * f, int n, int sw);
COLOR e_s_x_clr (int f, int b);
COLOR e_n_x_clr (int fb);
#ifdef UNIX
COLOR e_s_t_clr (int f, int b);
COLOR e_n_t_clr (int fb);
#endif
POINT e_set_pnt (int x, int y);
int e_pr_uul (we_colorset * fb);

#endif
