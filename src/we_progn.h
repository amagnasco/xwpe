#ifndef WE_PROGN_H
#define WE_PROGN_H

#include "globals.h"
#include "we_wind.h"

typedef struct
{
    FILE *fp;
    BUFFER *b;
    POINT p;
} E_AFILE;

int *e_sc_txt (int *c_sw, BUFFER * b);
int e_sc_nw_txt (int y, BUFFER * b, int sw);
int e_add_synt_tl (char *filename, We_window * f);
int e_mk_beauty (int sw, int ndif, We_window * f);
int e_sh_def (We_window * f);
int e_sh_nxt_def (We_window * f);
int e_nxt_brk (We_window * f);
int e_p_beautify (We_window * f);

/*   we_progn.c  */

int e_scfbol (int n, int mcsw, unsigned char *str, struct wpeSyntaxRule *cs);
int e_sc_all (We_window * f, int sw);
int e_program_opt (We_window * f);
void e_pr_c_line (int y, We_window * f);
E_AFILE *e_aopen (char *name, char *path, int mode);
int e_aclose (E_AFILE * ep);
char *e_agets (char *str, int n, E_AFILE * ep);
char *e_sh_spl1 (char *sp, char *str, E_AFILE * fp, int *n);
char *e_sh_spl2 (char *sp, char *str, E_AFILE * fp, int *n);
char *e_sh_spl3 (char *sp, char *str, E_AFILE * fp, int *n);
char *e_sh_spl4 (char *sp, char *str, E_AFILE * fp, int *n);
char *e_sh_spl5 (char *sp, char *str, E_AFILE * fp, int *n);
struct dirfile *e_c_add_df (char *str, struct dirfile *df);
int e_find_def (char *name, char *startfile, int mode, char *file,
                int *num, int *xn, int nold, char *oldfile,
                struct dirfile **df, int *first);
int e_show_nm_f (char *name, We_window * f, int oldn, char **oldname);

#endif
