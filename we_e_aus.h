#ifndef WE_E_AUS_H
#define WE_E_AUS_H

#include "globals.h"
#include "we_edit.h"
#include "we_mouse.h"

/*   we_e_aus.c   */
void e_cls(int frb, int chr);
int e_puts(char *s, int xa, int ya, int frb);
void e_pr_str(int x, int y, char *str, int col, int b2, int n2, int col2,
  int col3);
int e_pr_zstring(char *s, int x, int y, int n, int fb);
int e_schr_nchar(char *s, int x, int y, int n, int max, int frb);
void e_pr_nstr(int x, int y, int n, char *str, int col, int col2);
int e_schreib_zif(int *num, int x, int y, int max, int ft, int fs);
int e_schreib_leiste(char *s, int x, int y, int n, int max, int ft, int fs);
int e_schr_nzif(int num, int x, int y, int max, int col);
int e_pr_str_wsd(int x, int y, char *str, int col, int b2, int n2, int col2,
  int bg, int nd);
int e_pr_str_scan(int x, int y, char *str, int col, int b2, int n2, int col2,
  int bg, int nd);

#endif
