#ifndef WE_WIND_H
#define WE_WIND_H

#include "config.h"
#include "globals.h"
#include "we_main.h"
#include "we_mouse.h"

/*******************************************************************************/
/* (We_window *)f                                   */
/*            |                                   */
/*            +->(POINT)e.(int)x,y                */
/*            |         e.x=max_visible_cols_+1   */
/*            |         e.y=max_visible_lines_+1  */
/*            |                                   */
/*            +->(POINT)a.(int)x,y                */
/*            |         a.x=seems to always be 0  */
/*            |         a.y=seems to always be 1  */
/*            |                                   */
/*            +->(we_screen)s                        */
/*            |                                   */
/*            |  there's lots more in this struct */
/*            |                                   */
/**
 * These functions replace the macros names like NUM_COLS_ON_SCREEN etc.
 * to remove the dependency on variable naming. These macros only worked if
 * the function using it had a We_window variable `f`.
 *
 * (line_num_on_screen_bottom(window),col_num_on_screen_right(window))
 * is the coordinate for the lower right corner
 *
 */
inline int num_lines_off_screen_top(We_window* window)
{
    return window->s->c.y;
}
inline int num_lines_on_screen(We_window* window)
{
    return window->e.y - window->a.y;
}
inline int line_num_on_screen_bottom(We_window* window)
{
    return num_lines_on_screen(window) + num_lines_off_screen_top(window) - 1;
}
inline int num_cols_on_screen(We_window* window)
{
    return window->e.x - window->a.x;
}
inline int num_cols_off_screen_left(We_window* window)
{
    return window->s->c.x;
}
/*this seems to include the scroll bar to the right*/
/*it's values is always +1 to the actual visible columns*/
inline int num_cols_on_screen_safe(We_window* window)
{
    int result;
    if (window->e.x - window->a.x < window->b->mx.x + 1)
    {
        result = window->e.x - window->a.x;
    }
    else
    {
        result = window->b->mx.x + 1;
    }
    return result;
}
inline int col_num_on_screen_right(We_window* window)
{
    return num_cols_on_screen(window) + num_cols_off_screen_left(window) - 1;
}

/*   we_wind.c   */
int e_error(char* text, int sw, we_colorset* f);
int e_message(int sw, char* str, We_window* f);
void e_firstl(We_window* f, int sw);
int e_pr_filetype(We_window* f);
view* e_open_view(int xa, int ya, int xe, int ye, int col, int sw);
int e_close_view(view* pic, int sw);
void e_pr_line(int y, We_window* f);
void e_std_rahmen(int xa, int ya, int xe, int ye, char* name, int sw,
                  int frb, int fes);
void e_ed_rahmen(We_window* f, int sw);
int e_schirm(We_window* f, int sw);
int e_size_move(We_window* f);
view* e_std_kst(int xa, int ya, int xe, int ye, char* name, int sw, int fr,
                int ft, int fes);
view* e_ed_kst(We_window* f, view* pic, int sw);
int e_close_window(We_window* f);
void e_switch_window(int num, We_window* f);
int e_ed_zoom(We_window* f);
int e_ed_cascade(We_window* f);
int e_ed_tile(We_window* f);
int e_ed_next(We_window* f);
int e_mess_win(char* header, char* str, view** pic, We_window* f);
view* e_change_pic(int xa, int ya, int xe, int ye, view* pic, int sw,
                   int frb);
struct dirfile* e_add_df(char* str, struct dirfile* df);
int e_schr_nchar_wsv(char* str, int x, int y, int n, int max, int col,
                     int csw);
int e_schr_lst_wsv(char* str, int xa, int ya, int n, int strlen, int ft,
                   int fz, struct dirfile** df, We_window* f);
int e_rep_win_tree(ECNT* cn);
int e_opt_sec_box(int xa, int ya, int num, OPTK* opt, We_window* f, int sw);
int e_close_buffer(BUFFER* b);
int e_list_all_win(We_window* f);

#endif
