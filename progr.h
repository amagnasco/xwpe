#ifndef __PROGR_H
#define __PROGR_H
/* progr.h						  */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include <time.h>
#include "WeProg.h"

struct e_s_prog {
 char *language, *compiler, *comp_str, *libraries,
   *exe_name, *intstr, key;
 char **filepostfix; /* Expandable array */ 
 int	comp_sw, x;
};

struct e_prog {
 int num;  
 char *arguments, *project, *exedir, *sys_include;
 struct e_s_prog **comp;
};

extern int e__project;
extern struct e_s_prog e_s_prog;
extern struct e_prog e_prog;

typedef struct {  FILE *fp;  BUFFER *b;  POINT p;  }  E_AFILE;

#ifdef DJGPP
typedef long M_TIME;
#else
typedef time_t M_TIME;
#endif

/*   we_prog.c   */

int e_prog_switch(FENSTER *f, int c);
int e_compile(FENSTER *f);
int e_p_make(FENSTER *f);
int e_run(FENSTER *f);
int e_c_project(FENSTER *f);
int e_free_arg(char **arg, int argc);
int e_rel_brkwtch(FENSTER *f);
struct dirfile **e_make_prj_opt(FENSTER *f);
int e_comp(FENSTER *f);
int e_exec_inf(FENSTER *f, char **argv, int n);
int e_print_arg(FILE *fp, char *s, char **argv, int n);
int e_show_error(int n, FENSTER *f);
int e_make_error_list(FENSTER *f);
int e_previous_error(FENSTER *f);
int e_next_error(FENSTER *f);
int e_line_read(int n, char *s, int max);
int e_arguments(FENSTER *f);
int e_check_c_file(char *name);
int e_prj_ob_file(FENSTER *f);
int e_check_header(char *file, M_TIME otime, ECNT *cn, int sw);
char *e_cat_string(char *p, char *str);
int e_make_arg(char ***arg, char *str);
int e_ini_prog(ECNT *cn);
int e_copy_prog(struct e_s_prog *out, struct e_s_prog *in);
int e_run_options(FENSTER *f);
int e_run_c_options(FENSTER *f);
int e_project_options(FENSTER *f);
int e_system(char *estr, ECNT *cn);
int e_d_p_message(char *str, FENSTER *f, int sw);
int e_install(FENSTER *f);
int e_exec_make(FENSTER *f);
int e_run_sh(FENSTER *f);
int e_project(FENSTER *f);
int e_p_mess_win(char *header, int argc, char **argv, PIC **pic, FENSTER *f);
int e_p_add_df(FLWND *fw, int sw);
int e_p_del_df(FLWND *fw, int sw);
int e_p_edit_df(FLWND *fw, int sw);
int e_d_car_ret(FENSTER *f);
int e_d_car_mouse(FENSTER *f);
int e_add_arg(char ***arg, char *str, int n, int argc);
int e_new_message(FENSTER *f);
int e_p_cmp_mess(char *srch, BUFFER *b, int *ii, int *kk, int ret);
int e_project_name(FENSTER *f);
int e_wrt_prj_fl(FENSTER *f);
int e_p_update_prj_fl(FENSTER *f);
int freedfN(struct dirfile **df, int n);
int e_p_red_buffer(BUFFER *b);
int e_read_var(FENSTER *f);

/*   we_progn.c  */

int e_scfbol(int n, int mcsw, unsigned char *str, struct wpeSyntaxRule *cs);
int e_sc_all(FENSTER *f, int sw);
int e_program_opt(FENSTER *f);
int e_sc_nw_txt(int y, BUFFER *b, int sw);
int *e_sc_txt(int *c_sw, BUFFER *b);
void e_pr_c_line(int y, FENSTER *f);
int e_add_synt_tl(char *filename, FENSTER *f);
E_AFILE *e_aopen(char *name, char *path, int mode);
int e_aclose(E_AFILE *ep);
char *e_agets(char *str, int n, E_AFILE *ep);
char *e_sh_spl1(char *sp, char *str, E_AFILE *fp, int *n);
char *e_sh_spl2(char *sp, char *str, E_AFILE *fp, int *n);
char *e_sh_spl3(char *sp, char *str, E_AFILE *fp, int *n);
char *e_sh_spl4(char *sp, char *str, E_AFILE *fp, int *n);
char *e_sh_spl5(char *sp, char *str, E_AFILE *fp, int *n);
struct dirfile *e_c_add_df(char *str, struct dirfile *df);
int e_find_def(char *name, char *startfile, int mode, char *file,
  int *num, int *xn, int nold, char *oldfile, struct dirfile **df,
  int *first);
int e_show_nm_f(char *name, FENSTER *f, int oldn, char **oldname);
int e_sh_def(FENSTER *f);
int e_sh_nxt_def(FENSTER *f);
int e_nxt_brk(FENSTER *f);
int e_mk_beauty(int sw, int ndif, FENSTER *f);
int e_p_beautify(FENSTER *f);

/*   we_fl_unix.c  */

int e_funct(FENSTER *f);
int e_funct_in(FENSTER *f);
int e_data_first(int sw, ECNT *cn, char *nstr);
int e_data_schirm(FENSTER *f);
int e_data_eingabe(ECNT *cn);

#endif

