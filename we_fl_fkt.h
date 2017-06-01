#ifndef WE_FL_FKT_H
#define WE_FL_FKT_H

#include "we_edit.h"
#include "we_e_aus.h"
#include "we_fl_unix.h"
#include "we_mouse.h"
#include "we_opt.h"
#include "we_debug.h"

/*   we_fl_fkt.c   */
char *e_mkfilename(char *dr, char *fn);
POINT e_readin(int i, int j, FILE *fp, BUFFER *b, char *sw);
int e_new(FENSTER *f);
int e_m_save(FENSTER *f);
int e_save(FENSTER *f);
int e_saveall(FENSTER *f);
int e_quit(FENSTER *f);
int e_write(int xa, int ya, int xe, int ye, FENSTER *f, int backup);
char *e_new_qual(char *s, char *ns, char *sb);
char *e_bakfilename(char *s);
int freedf(struct dirfile *df);
int e_file_window(int sw, FLWND *fw, int ft, int fz);
int e_pr_file_window(FLWND *fw, int c, int sw, int ft, int fz, int fs);
int e_help_last(FENSTER *f);
int e_help_comp(FENSTER *f);
int e_help(FENSTER *f);
int e_help_loc(FENSTER *f, int sw);
int e_help_free(FENSTER *f);
int e_help_ret(FENSTER *f);
int e_topic_search(FENSTER *f);

#endif
