#ifndef WE_PROGN_H
#define WE_PROGN_H

/** \file we_progn.h */

#include "config.h"
#include "globals.h"
#include "we_wind.h"

typedef struct
{
    FILE* fp;
    we_buffer_t* b;
    we_point_t p;
} E_AFILE;

int* e_sc_txt(int* c_sw, we_buffer_t* b);
void e_sc_txt_2 (we_window_t *window);
int e_sc_nw_txt(int y, we_buffer_t* b, int sw);
int e_add_synt_tl(char* filename, we_window_t* window);
int e_mk_beauty(int sw, int ndif, we_window_t* window);
int e_sh_def(we_window_t* window);
int e_sh_nxt_def(we_window_t* window);
int e_nxt_brk(we_window_t* window);
int e_p_beautify(we_window_t* window);

/*   we_progn.c  */

int e_scfbol(int n, int mcsw, unsigned char* str, struct wpeSyntaxRule* cs);
int e_sc_all(we_window_t* window, int sw);
int e_program_opt(we_window_t* window);
void e_pr_c_line(int y, we_window_t* window);
E_AFILE* e_aopen(char* name, char* path, int mode);
int e_aclose(E_AFILE* ep);
char* e_agets(char* str, int n, E_AFILE* ep);
char* e_sh_spl1(char* sp, char* str, E_AFILE* fp, int* n);
char* e_sh_spl2(char* sp, char* str, E_AFILE* fp, int* n);
char* e_sh_spl3(char* sp, char* str, E_AFILE* fp, int* n);
char* e_sh_spl4(char* sp, char* str, E_AFILE* fp, int* n);
char* e_sh_spl5(char* sp, char* str, E_AFILE* fp, int* n);
struct dirfile* e_c_add_df(char* str, struct dirfile* df);
int e_find_def(char* name, char* startfile, int mode, char* file,
               int* num, int* xn, int nold, char* oldfile,
               struct dirfile** df, int* first);
int e_show_nm_f(char* name, we_window_t* window, int oldn, char** oldname);

#endif
