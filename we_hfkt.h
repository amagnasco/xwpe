#ifndef WE_HFKT_H
#define WE_HFKT_H

#include "we_edit.h"
#include "we_opt.h"

/*   we_hfkt.c   */
int e_strstr(int x, int n, unsigned char *s, unsigned char *f);
int e_ustrstr(int x, int n, unsigned char *s, unsigned char *f);
int e_urstrstr(int x, int n, unsigned char *s, unsigned char *f, int *nn);
int e_rstrstr(int x, int n, unsigned char *s, unsigned char *f, int *nn);
int e_str_len(unsigned char *s);

//#define e_str_nrc(s) strlen(s)
#define e_toupper(c) toupper(c)

int e_num_kst(char *s, int num, int max, FENSTER *f, int n, int sw);
COLOR e_s_x_clr(int f, int b);
COLOR e_n_x_clr(int fb);
#ifdef UNIX
COLOR e_s_t_clr(int f, int b);
COLOR e_n_t_clr(int fb);
#endif
POINT e_set_pnt(int x, int y);
int e_pr_uul(FARBE *fb);

#endif
