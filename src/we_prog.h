#ifndef WE_PROG_H
#define WE_PROG_H

/** \file we_prog.h */

#include "config.h"
#include "we_edit.h"
#include "we_hfkt.h"
#include "we_mouse.h"
#include "we_unix.h"
#include <stdlib.h>
#include <time.h>

typedef time_t M_TIME;

extern int e__project;

struct e_s_prog
{
    char *language, *compiler, *comp_str, *libraries, *exe_name, *intstr, key;
    char** filepostfix; /* Expandable array */
    int comp_sw, x;
};
extern struct e_s_prog e_s_prog;

int e_ini_prog(we_control_t* control);

struct e_prog
{
    int num;
    char *arguments, *project, *exedir, *sys_include;
    struct e_s_prog** comp;
};
extern struct e_prog e_prog;

/********** prototypes ****************/
struct dirfile** e_make_prj_opt(we_window_t* window);
int e_rel_brkwtch(we_window_t* window);
int e_prj_ob_file(we_window_t* window);
int e_make_error_list(we_window_t* window);
int e_d_car_ret(we_window_t* window);
int e_prog_switch(we_window_t* window, int c);

int print_to_end_of_buffer(BUFFER* b, char* str, int wrap_limit);

/*   we_prog.c   */

int e_compile(we_window_t* window);
int e_p_make(we_window_t* window);
int e_run(we_window_t* window);
int e_c_project(we_window_t* window);
int e_free_arg(char** arg, int argc);
int e_comp(we_window_t* window);
int e_exec_inf(we_window_t* window, char** argv, int n);
int e_print_arg(FILE* fp, char* s, char** argv, int n);
int e_show_error(int n, we_window_t* window);
int e_previous_error(we_window_t* window);
int e_next_error(we_window_t* window);
int e_line_read(int n, char* s, int max);
int e_arguments(we_window_t* window);
int e_check_c_file(char* name);
int e_check_header(char* file, M_TIME otime, we_control_t* control, int sw);
char* e_cat_string(char* p, char* str);
int e_make_arg(char*** arg, char* str);
int e_copy_prog(struct e_s_prog* out, struct e_s_prog* in);
int e_run_options(we_window_t* window);
int e_run_c_options(we_window_t* window);
int e_project_options(we_window_t* window);
int e_system(char* estr, we_control_t* control);
int e_d_p_message(char* str, we_window_t* window, int sw);
int e_install(we_window_t* window);
int e_exec_make(we_window_t* window);
int e_run_sh(we_window_t* window);
int e_project(we_window_t* window);
int e_p_mess_win(char* header, int argc, char** argv, we_view_t** view,
                 we_window_t* window);
int e_p_add_df(FLWND* fw, int sw);
int e_p_del_df(FLWND* fw, int sw);
int e_p_edit_df(FLWND* fw, int sw);
int e_d_car_mouse(we_window_t* window);
int e_add_arg(char*** arg, char* str, int n, int argc);
int e_new_message(we_window_t* window);
int e_p_cmp_mess(char* srch, BUFFER* b, int* ii, int* kk, int ret);
int e_project_name(we_window_t* window);
int e_wrt_prj_fl(we_window_t* window);
int e_p_update_prj_fl(we_window_t* window);
int freedfN(struct dirfile** df, int n);
int e_p_red_buffer(BUFFER* b);
int e_read_var(we_window_t* window);

#endif
