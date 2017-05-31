#ifndef WE_WIND_H
#define WE_WIND_H

#include "globals.h"
#include "we_main.h"

/*******************************************************************************/
/* (FENSTER *)f                                   */
/*            |                                   */
/*            +->(POINT)e.(int)x,y                */
/*            |         e.x=max_visible_cols_+1   */
/*            |         e.y=max_visible_lines_+1  */
/*            |                                   */
/*            +->(POINT)a.(int)x,y                */
/*            |         a.x=seems to always be 0  */
/*            |         a.y=seems to always be 1  */
/*            |                                   */
/*            +->(SCHIRM)s                        */
/*            |                                   */
/*            |  there's lots more in this struct */
/*            |                                   */
#define NUM_LINES_OFF_SCREEN_TOP	(f->s->c.y)
#define NUM_LINES_ON_SCREEN	(f->e.y - f->a.y)
#define LINE_NUM_ON_SCREEN_BOTTOM	(NUM_LINES_ON_SCREEN + NUM_LINES_OFF_SCREEN_TOP - 1)
      
/*this seems to include the scroll bar to the right*/
/*it's values is always +1 to the actual visible columns*/
#define NUM_COLS_ON_SCREEN_SAFE (((f->e.x - f->a.x) < (f->b->mx.x+1)) ? (f->e.x - f->a.x) : (f->b->mx.x+1))
#define NUM_COLS_ON_SCREEN	(f->e.x - f->a.x)
      
#define NUM_COLS_OFF_SCREEN_LEFT	(f->s->c.x)
#define COL_NUM_ON_SCREEN_RIGHT	(NUM_COLS_ON_SCREEN + NUM_COLS_OFF_SCREEN_LEFT - 1)

/*(LINE_NUM_ON_SCREEN_BOTTOM,COL_NUM_ON_SCREEN_RIGHT)*/
/*	is the coordinate for the lower right corner*/
/******************************************************************************/

#endif
