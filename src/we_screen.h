/** \file we_screen.h                                     */
/* Copyright (C) 2018 Guus Bonnema                        */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */
/*                                                        */
/* Some of the following code is based on or copied from  */
/* other sources within XWPE. For these lines the         */
/* original copyright was:                                */
/* Copyright (C) 1993 Fred Kruse                          */
/**********************************************************/
#ifndef WE_SCREEN_H
#define WE_SCREEN_H

extern char *global_screen;
extern char *global_alt_screen;

#if defined(NEWSTYLE) && !defined(NO_XWINDOWS)
extern char *extbyte;
extern char *altextbyte;
#endif

/* global fields */
extern int col_num;

/* exported functions */
char *attr_normal();
char *graphics_charset_pairs();
char *enable_alt_charset();
char *start_alt_charset();
char *end_alt_charset();
const char *get_beg_scr();
const char *get_swt_scr();

_Bool is_x_term();
int fk_t_locate(int x, int y);
int fk_t_cursor(int x);
int term_move(int x, int y);
int print_char(int c);
int term_refresh();
int max_screen_lines();
int set_max_screen_lines(const int max_lines);
int max_screen_cols();
int set_max_screen_cols(const int max_cols);

int fk_attrset(int a);
void fk_colset(int a);
void e_endwin (void);

#endif
