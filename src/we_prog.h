#ifndef WE_PROG_H
#define WE_PROG_H

#include <stdlib.h>
#include "we_edit.h"
#include "we_hfkt.h"
#include "we_mouse.h"
#include "we_unix.h"

typedef time_t M_TIME;

extern int e__project;

struct e_s_prog {
 char *language, *compiler, *comp_str, *libraries,
   *exe_name, *intstr, key;
 char **filepostfix; /* Expandable array */ 
 int	comp_sw, x;
};
extern struct e_s_prog e_s_prog;

int e_ini_prog(ECNT *cn);

struct e_prog {
 int num;  
 char *arguments, *project, *exedir, *sys_include;
 struct e_s_prog **comp;
};
extern struct e_prog e_prog;

/********** prototypes ****************/
struct dirfile **e_make_prj_opt(FENSTER *f);
int e_rel_brkwtch(FENSTER *f);
int e_prj_ob_file(FENSTER *f);
int e_make_error_list(FENSTER *f);
int e_d_car_ret(FENSTER *f);
int e_prog_switch(FENSTER *f, int c);

int print_to_end_of_buffer(BUFFER * b,char * str,int wrap_limit);

/*   we_prog.c   */

int e_compile(FENSTER *f);
int e_p_make(FENSTER *f);
int e_run(FENSTER *f);
int e_c_project(FENSTER *f);
int e_free_arg(char **arg, int argc);
int e_comp(FENSTER *f);
int e_exec_inf(FENSTER *f, char **argv, int n);
int e_print_arg(FILE *fp, char *s, char **argv, int n);
int e_show_error(int n, FENSTER *f);
int e_previous_error(FENSTER *f);
int e_next_error(FENSTER *f);
int e_line_read(int n, char *s, int max);
int e_arguments(FENSTER *f);
int e_check_c_file(char *name);
int e_check_header(char *file, M_TIME otime, ECNT *cn, int sw);
char *e_cat_string(char *p, char *str);
int e_make_arg(char ***arg, char *str);
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
int e_p_mess_win(char *header, int argc, char **argv, view **pic, FENSTER *f);
int e_p_add_df(FLWND *fw, int sw);
int e_p_del_df(FLWND *fw, int sw);
int e_p_edit_df(FLWND *fw, int sw);
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

#endif
