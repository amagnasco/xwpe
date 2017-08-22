#ifndef WE_WIND_H
#define WE_WIND_H

/** \file we_wind.h */

#include "config.h"
#include "globals.h"
#include "we_main.h"
#include "we_mouse.h"

/*******************************************************************************/
/* (we_window_t *)window                                   */
/*            |                                   */
/*            +->(we_point_t)e.(int)x,y                */
/*            |         e.x=max_visible_cols_+1   */
/*            |         e.y=max_visible_lines_+1  */
/*            |                                   */
/*            +->(we_point_t)a.(int)x,y                */
/*            |         a.x=seems to always be 0  */
/*            |         a.y=seems to always be 1  */
/*            |                                   */
/*            +->(we_screen_t)s                        */
/*            |                                   */
/*            |  there's lots more in this struct */
/*            |                                   */
/**
 * These functions replace the macros names like NUM_COLS_ON_SCREEN etc.
 * to remove the dependency on variable naming. These macros only worked if
 * the function using it had a we_window_t variable `window`.
 *
 * (line_num_on_screen_bottom(window),col_num_on_screen_right(window))
 * is the coordinate for the lower right corner
 *
 */
inline int num_lines_off_screen_top(we_window_t* window)
{
    return window->screen->c.y;
}
inline int num_lines_on_screen(we_window_t* window)
{
    return window->e.y - window->a.y;
}
inline int line_num_on_screen_bottom(we_window_t* window)
{
    return num_lines_on_screen(window) + num_lines_off_screen_top(window) - 1;
}
inline int num_cols_on_screen(we_window_t* window)
{
    return window->e.x - window->a.x;
}
inline int num_cols_off_screen_left(we_window_t* window)
{
    return window->screen->c.x;
}
/*this seems to include the scroll bar to the right*/
/*it's values is always +1 to the actual visible columns*/
inline int num_cols_on_screen_safe(we_window_t* window)
{
    int result;
    if (window->e.x - window->a.x < window->buffer->mx.x + 1)
    {
        result = window->e.x - window->a.x;
    }
    else
    {
        result = window->buffer->mx.x + 1;
    }
    return result;
}
inline int col_num_on_screen_right(we_window_t* window)
{
    return num_cols_on_screen(window) + num_cols_off_screen_left(window) - 1;
}

/*   we_wind.c   */
int e_error(char* text, int sw, we_colorset_t* colorset);
int e_message(int sw, char* str, we_window_t* window);
void e_firstl(we_window_t* window, int sw);
int e_pr_filetype(we_window_t* window);
we_view_t* e_open_view(int xa, int ya, int xe, int ye, int col, int sw);
int e_close_view(we_view_t* view, int sw);
void e_pr_line(int y, we_window_t* window);
void e_std_rahmen(int xa, int ya, int xe, int ye, char* name, int sw,
                  int frb, int fes);
void e_ed_rahmen(we_window_t* window, int sw);
int e_write_screen(we_window_t* window, int sw);
int e_size_move(we_window_t* window);
we_view_t* e_std_kst(int xa, int ya, int xe, int ye, char* name, int sw, int fr,
                     int ft, int fes);
we_view_t* e_ed_kst(we_window_t* window, we_view_t* view, int sw);
int e_close_window(we_window_t* window);
void e_switch_window(int num, we_window_t* window);
int e_ed_zoom(we_window_t* window);
int e_ed_cascade(we_window_t* window);
int e_ed_tile(we_window_t* window);
int e_ed_next(we_window_t* window);
int e_mess_win(char* header, char* str, we_view_t** view, we_window_t* window);
we_view_t* e_change_pic(int xa, int ya, int xe, int ye, we_view_t* view, int sw,
                        int frb);
struct dirfile* e_add_df(char* str, struct dirfile* df);
int e_schr_nchar_wsv(char* str, int x, int y, int n, int max, int col,
                     int csw);
int e_schr_lst_wsv(char* str, int xa, int ya, int n, int strlen, int ft,
                   int fz, struct dirfile** df, we_window_t* window);
int e_rep_win_tree(we_control_t* control);
int e_opt_sec_box(int xa, int ya, int num, OPTK* opt, we_window_t* window, int sw);
int e_close_buffer(we_buffer_t* buffer);
int e_list_all_win(we_window_t* window);

#endif
