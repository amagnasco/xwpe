/* we_menue.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"

int             e_p_show_messages(FENSTER *f);
int             e_p_show_watches(FENSTER *f);
int             e_blck_gt_beg(FENSTER *f);
int             e_blck_gt_end(FENSTER *f);
int             e_blck_mrk_line(FENSTER *f);
int             e_blck_mrk_all(FENSTER *f);
int             e_blck_to_left(FENSTER *f);
int             e_blck_to_right(FENSTER *f);
int             e_cl_project(FENSTER *f);
int             e_p_add_item(FENSTER *f);
int             e_p_del_item(FENSTER *f);
int             e_show_project(FENSTER *f);
int             e_info(FENSTER *f);
int             e_help_options(FENSTER *f);
int             e_hp_ret(FENSTER *f);
int             e_hp_back(FENSTER *f);
int             e_hp_prev(FENSTER *f);
int             e_hp_next(FENSTER *f);

/* main menu control bar */
int WpeHandleMainmenu(int n, FENSTER *f)
{
  extern int      e_mn_men;
  int             i, c = 255, nold = n <= 0 ? 1 : n - 1;
  extern OPT      opt[];
  extern char    *e_hlp, *e_hlp_str[];
  ECNT           *cn = f->ed;
  MENU           *mainmenu = MALLOC(MENOPT * sizeof(MENU));

  for(i = 0; i < MENOPT; i++)
    mainmenu[i].width = 0;
  fk_cursor(0);
  if(n < 0)
    n = 0;
  else
    c = WPE_CR;
#ifdef UNIX
#ifdef NEWSTYLE
  if(WpeIsXwin())
    mainmenu[0].position = -3;
  else
#endif
    mainmenu[0].position = -2;
  mainmenu[0].no_of_items = 6;
  mainmenu[0].width = 23;
#else
  mainmenu[0].position = -3;
  mainmenu[0].no_of_items = 4;
  mainmenu[0].width = 20;
#endif
  if((mainmenu[0].menuitems = MALLOC(mainmenu[0].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  mainmenu[0].menuitems[0] = WpeFillSubmenuItem("About WE", 0, 'A', e_about_WE);
  mainmenu[0].menuitems[1] = WpeFillSubmenuItem("Clear Desktop", 0, 'C', e_clear_desk);
  mainmenu[0].menuitems[2] = WpeFillSubmenuItem("Repaint Desktop", 0, 'R', e_repaint_desk);
  mainmenu[0].menuitems[3] = WpeFillSubmenuItem("System Info", 0, 'S', e_sys_info);
#ifdef UNIX
  mainmenu[0].menuitems[4] = WpeFillSubmenuItem("Show Wastebasket", 5, 'W', WpeShowWastebasket);
  mainmenu[0].menuitems[5] = WpeFillSubmenuItem("Delete Wastebasket", 0, 'D', WpeDelWastebasket);
#endif
  mainmenu[1].position = -3;
#ifdef UNIX
  mainmenu[1].width = 24;
  mainmenu[1].no_of_items = 11;
  if((mainmenu[1].menuitems = MALLOC(mainmenu[1].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  if(f->ed->edopt & ED_CUA_STYLE)
  {
    mainmenu[1].menuitems[0] = WpeFillSubmenuItem("File-Manager     F2", 0, 'M', WpeManager);
    mainmenu[1].menuitems[2] = WpeFillSubmenuItem("Save         Alt F2", 0, 'S', e_m_save);
    mainmenu[1].menuitems[10] = WpeFillSubmenuItem("Quit         Alt F4", 0, 'Q', e_quit);
  }
  else
  {
    mainmenu[1].menuitems[0] = WpeFillSubmenuItem("File-Manager     F3", 0, 'M', WpeManager);
    mainmenu[1].menuitems[2] = WpeFillSubmenuItem("Save             F2", 0, 'S', e_m_save);
    mainmenu[1].menuitems[10] = WpeFillSubmenuItem("Quit          Alt X", 0, 'Q', e_quit);
  }
  mainmenu[1].menuitems[1] = WpeFillSubmenuItem("New", 0, 'N', e_new);
  mainmenu[1].menuitems[3] = WpeFillSubmenuItem("Save As", 5, 'A', WpeSaveAsManager);
  mainmenu[1].menuitems[4] = WpeFillSubmenuItem("Save aLl", 6, 'L', e_saveall);
  mainmenu[1].menuitems[5] = WpeFillSubmenuItem("Execute", 0, 'E', WpeExecuteManager);
  mainmenu[1].menuitems[6] = WpeFillSubmenuItem("SHell", 1, 'H', WpeShell);
  mainmenu[1].menuitems[7] = WpeFillSubmenuItem("Find", 0, 'F', WpeFindWindow);
  mainmenu[1].menuitems[8] = WpeFillSubmenuItem("Grep", 0, 'G', WpeGrepWindow);
  mainmenu[1].menuitems[9] = WpeFillSubmenuItem("Print File", 0, 'P', WpePrintFile);
#endif
  mainmenu[2].position = -3;
  mainmenu[2].width = 27;
#if !defined(NO_XWINDOWS) && !defined(DJGPP)
  if(WpeIsXwin())
    mainmenu[2].no_of_items = 9;
  else
#endif
    mainmenu[2].no_of_items = 7;
  if((mainmenu[2].menuitems = MALLOC(mainmenu[2].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  mainmenu[2].menuitems[0] = WpeFillSubmenuItem("Cut     Shift Del / ^X", 2, 'T', e_edt_del);
  mainmenu[2].menuitems[1] = WpeFillSubmenuItem("Copy         ^Ins / ^C", 0, 'C', e_edt_copy);
  mainmenu[2].menuitems[2] = WpeFillSubmenuItem("Paste   Shift Ins / ^V", 0, 'P', e_edt_einf);
  mainmenu[2].menuitems[3] = WpeFillSubmenuItem("Show Buffer         ^W", 0, 'S', e_show_clipboard);
  mainmenu[2].menuitems[4] = WpeFillSubmenuItem("Delete            ^Del", 0, 'D', e_blck_del);
  mainmenu[2].menuitems[5] = WpeFillSubmenuItem("Undo                ^U", 0, 'U', e_make_undo);
  mainmenu[2].menuitems[6] = WpeFillSubmenuItem("Redo                ^R", 0, 'R', e_make_redo);
#ifndef NO_XWINDOWS
  if(WpeIsXwin())
  {
    mainmenu[2].menuitems[7] = WpeFillSubmenuItem("PAste(XBuffer) Alt Ins", 0, 'A', e_u_copy_X_buffer);
    mainmenu[2].menuitems[8] = WpeFillSubmenuItem("COpy(XBuffer)  Alt Del", 0, 'O', e_u_paste_X_buffer);
  }
#endif
  mainmenu[3].position = -3;
  mainmenu[3].width = 28;
  mainmenu[3].no_of_items = 4;
  if((mainmenu[3].menuitems = MALLOC(mainmenu[3].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  if(f->ed->edopt & ED_CUA_STYLE)
  {
    mainmenu[3].menuitems[0] = WpeFillSubmenuItem("Find      Alt F3 / ^O F", 0, 'F', e_find);
    mainmenu[3].menuitems[1] = WpeFillSubmenuItem("Replace Shift F3 / ^O A", 0, 'R', e_replace);
    mainmenu[3].menuitems[2] = WpeFillSubmenuItem("Search again         F3", 0, 'S', e_rep_search);
  }
  else
  {
    mainmenu[3].menuitems[0] = WpeFillSubmenuItem("Find          F4 / ^O F", 0, 'F', e_find);
    mainmenu[3].menuitems[1] = WpeFillSubmenuItem("Replace   Alt F4 / ^O A", 0, 'R', e_replace);
    mainmenu[3].menuitems[2] = WpeFillSubmenuItem("Search again         ^L", 0, 'S', e_rep_search);
  }
  mainmenu[3].menuitems[3] = WpeFillSubmenuItem("Go to Line        Alt G", 0, 'G', e_goto_line);
  mainmenu[4].position = -3;
  mainmenu[4].width = 25;
  mainmenu[4].no_of_items = 15;
  if((mainmenu[4].menuitems = MALLOC(mainmenu[4].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  mainmenu[4].menuitems[0] = WpeFillSubmenuItem("Begin Mark      ^K B", 0, 'B', e_blck_begin);
  mainmenu[4].menuitems[1] = WpeFillSubmenuItem("End Mark        ^K K", 0, 'E', e_blck_end);
  mainmenu[4].menuitems[2] = WpeFillSubmenuItem("Mark WhOle      ^K X", 7, 'O', e_blck_mrk_all);
  mainmenu[4].menuitems[3] = WpeFillSubmenuItem("Mark Line       ^K L", 5, 'L', e_blck_mrk_line);
  mainmenu[4].menuitems[4] = WpeFillSubmenuItem("Goto Begin      ^K A", 0, 'G', e_blck_gt_beg);
  mainmenu[4].menuitems[5] = WpeFillSubmenuItem("Goto ENd        ^K Z", 6, 'N', e_blck_gt_end);
  mainmenu[4].menuitems[6] = WpeFillSubmenuItem("Copy            ^K C", 0, 'C', e_blck_copy);
  mainmenu[4].menuitems[7] = WpeFillSubmenuItem("Move            ^K V", 0, 'M', e_blck_move);
  mainmenu[4].menuitems[8] = WpeFillSubmenuItem("Delete          ^K Y", 0, 'D', e_blck_del);
  mainmenu[4].menuitems[9] = WpeFillSubmenuItem("Hide            ^K H", 0, 'H', e_blck_hide);
  mainmenu[4].menuitems[10] = WpeFillSubmenuItem("Read            ^K R", 0, 'R', e_blck_read);
  mainmenu[4].menuitems[11] = WpeFillSubmenuItem("Write           ^K W", 0, 'W', e_blck_write);
  mainmenu[4].menuitems[12] = WpeFillSubmenuItem("Move to RIght   ^K I", 9, 'I', e_blck_to_right);
  mainmenu[4].menuitems[13] = WpeFillSubmenuItem("Move to LefT    ^K U", 11, 'T', e_blck_to_left);
  mainmenu[4].menuitems[14] = WpeFillSubmenuItem("ChAnge Case     ^K D", 2, 'A', e_changecase_dialog);
#ifdef PROG
  if(WpeIsProg())
  {
    mainmenu[5].position = -3;
    mainmenu[5].width = 35;
    mainmenu[5].no_of_items = 12;
    if((mainmenu[5].menuitems = MALLOC(mainmenu[5].no_of_items * sizeof(OPTK))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->fb);
    mainmenu[5].menuitems[0] = WpeFillSubmenuItem("Compile         Alt F9 / Alt C", 0, 'C', e_compile);
    mainmenu[5].menuitems[1] = WpeFillSubmenuItem("Make                F9 / Alt M", 0, 'M', e_p_make);
    mainmenu[5].menuitems[2] = WpeFillSubmenuItem("Run                ^F9 / Alt U", 0, 'R', e_run);
    mainmenu[5].menuitems[3] = WpeFillSubmenuItem("Install                  Alt L", 0, 'I', e_install);
    mainmenu[5].menuitems[4] = WpeFillSubmenuItem("Execute Make             Alt A", 0, 'E', e_exec_make);
    mainmenu[5].menuitems[5] = WpeFillSubmenuItem("Next Error      Alt F8 / Alt T", 0, 'N', e_next_error);
    mainmenu[5].menuitems[6] = WpeFillSubmenuItem("Previous Error  Alt F7 / Alt V", 0, 'P', e_previous_error);
    mainmenu[5].menuitems[7] = WpeFillSubmenuItem("Show Definition           ^O S", 0, 'S', e_sh_def);
    mainmenu[5].menuitems[8] = WpeFillSubmenuItem("Show NeXt Definition      ^O N", 7, 'X', e_sh_nxt_def);
    mainmenu[5].menuitems[9] = WpeFillSubmenuItem("Matching BracKet          ^O K", 13, 'K', e_nxt_brk);
    mainmenu[5].menuitems[10] = WpeFillSubmenuItem("Beautify                  ^O B", 0, 'B', e_p_beautify);
    mainmenu[5].menuitems[11] = WpeFillSubmenuItem("Arguments                     ", 0, 'A', e_arguments);

    mainmenu[MENOPT - 4].position = -3;
    mainmenu[MENOPT - 4].width = 23;
    mainmenu[MENOPT - 4].no_of_items = 5;
    if((mainmenu[MENOPT - 4].menuitems = MALLOC(mainmenu[MENOPT - 4].no_of_items * sizeof(OPTK))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->fb);
    mainmenu[MENOPT - 4].menuitems[0] = WpeFillSubmenuItem("Open Project", 5, 'P', e_project);
    mainmenu[MENOPT - 4].menuitems[1] = WpeFillSubmenuItem("Close Project", 0, 'C', e_cl_project);
    mainmenu[MENOPT - 4].menuitems[2] = WpeFillSubmenuItem("Add Item", 0, 'A', e_p_add_item);
    mainmenu[MENOPT - 4].menuitems[3] = WpeFillSubmenuItem("Delete Item", 0, 'D', e_p_del_item);
    mainmenu[MENOPT - 4].menuitems[4] = WpeFillSubmenuItem("Options", 0, 'O', e_project_options);
  }
#endif
#ifdef DEBUGGER
  if(WpeIsProg())
  {
    mainmenu[MENOPT - 5].position = -3;
    mainmenu[MENOPT - 5].width = 33;
    mainmenu[MENOPT - 5].no_of_items = 13;
    if((mainmenu[MENOPT - 5].menuitems = MALLOC(mainmenu[MENOPT - 5].no_of_items * sizeof(OPTK))) == NULL)
      e_error(e_msg[ERR_LOWMEM], 1, f->fb);

    if(f->ed->edopt & ED_CUA_STYLE)
    {
      mainmenu[MENOPT - 5].menuitems[0] = WpeFillSubmenuItem("Toggle Breakpoint  F5 / ^G B", 7, 'B', e_breakpoint);
      mainmenu[MENOPT - 5].menuitems[2] = WpeFillSubmenuItem("Make Watch        ^F5 / ^G W", 5, 'W', e_make_watches);
      mainmenu[MENOPT - 5].menuitems[6] = WpeFillSubmenuItem("Show stacK        ^F3 / ^G K", 9, 'K', e_deb_stack);
    }
    else
    {
      mainmenu[MENOPT - 5].menuitems[0] = WpeFillSubmenuItem("Toggle Breakpoint ^F8 / ^G B", 7, 'B', e_breakpoint);
      mainmenu[MENOPT - 5].menuitems[2] = WpeFillSubmenuItem("Make Watch        ^F7 / ^G W", 5, 'W', e_make_watches);
      mainmenu[MENOPT - 5].menuitems[6] = WpeFillSubmenuItem("Show stacK        ^F6 / ^G K", 9, 'K', e_deb_stack);
    }
    mainmenu[MENOPT - 5].menuitems[1] = WpeFillSubmenuItem("ReMove all Breakp.      ^G M", 2, 'M', e_remove_breakpoints);
    mainmenu[MENOPT - 5].menuitems[3] = WpeFillSubmenuItem("Edit watch              ^G E", 0, 'E', e_edit_watches);
    mainmenu[MENOPT - 5].menuitems[4] = WpeFillSubmenuItem("Delete watch            ^G D", 0, 'D', e_delete_watches);
    mainmenu[MENOPT - 5].menuitems[5] = WpeFillSubmenuItem("Remove All watches      ^G A", 7, 'A', e_remove_all_watches);
    mainmenu[MENOPT - 5].menuitems[7] = WpeFillSubmenuItem("Goto cursor             ^G U", 0, 'U', e_d_goto_cursor);
    mainmenu[MENOPT - 5].menuitems[8] = WpeFillSubmenuItem("Finish function         ^G F", 0, 'F', e_d_finish_func);
    mainmenu[MENOPT - 5].menuitems[9] = WpeFillSubmenuItem("Trace              F7 / ^G T", 0, 'T', e_deb_trace);
    mainmenu[MENOPT - 5].menuitems[10] = WpeFillSubmenuItem("Step               F8 / ^G S", 0, 'S', e_deb_next);
    mainmenu[MENOPT - 5].menuitems[11] = WpeFillSubmenuItem("Run/Continue     ^F10 / ^G R", 0, 'R', e_deb_run);
    mainmenu[MENOPT - 5].menuitems[12] = WpeFillSubmenuItem("Quit              ^F2 / ^G Q", 0, 'Q', e_d_quit);
  }
#endif
  mainmenu[MENOPT - 3].position = -4;
  mainmenu[MENOPT - 3].width = 22;
#ifdef PROG
  if(WpeIsProg())
#ifdef DEBUGGER
    mainmenu[MENOPT - 3].no_of_items = 8;
#else
    mainmenu[MENOPT - 3].no_of_items = 7;
#endif
  else
#endif
    mainmenu[MENOPT - 3].no_of_items = 5;
  if((mainmenu[MENOPT - 3].menuitems = MALLOC(mainmenu[MENOPT - 3].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  mainmenu[MENOPT - 3].menuitems[0] = WpeFillSubmenuItem("Adjust Colors", 0, 'A', e_ad_colors);
  mainmenu[MENOPT - 3].menuitems[1] = WpeFillSubmenuItem("Save Options", 0, 'S', e_opt_save);
  mainmenu[MENOPT - 3].menuitems[2] = WpeFillSubmenuItem("Editor", 0, 'E', e_edt_options);
  mainmenu[MENOPT - 3].menuitems[3] = WpeFillSubmenuItem("File-Manager", 0, 'F', WpeFileManagerOptions);
  mainmenu[MENOPT - 3].menuitems[4] = WpeFillSubmenuItem("Help", 0, 'H', e_help_options);
#ifdef PROG
  if(WpeIsProg())
  {
    mainmenu[MENOPT - 3].menuitems[5] = WpeFillSubmenuItem("ProGramming", 3, 'G', e_program_opt);
    mainmenu[MENOPT - 3].menuitems[6] = WpeFillSubmenuItem("Compiler", 0, 'C', e_run_options);
#ifdef DEBUGGER
    mainmenu[MENOPT - 3].menuitems[7] = WpeFillSubmenuItem("Debugger", 0, 'D', e_deb_options);
#endif
  }
#endif
#ifdef NEWSTYLE
  if(WpeIsXwin())
    mainmenu[MENOPT - 2].position = -13;
  else
#endif
    mainmenu[MENOPT - 2].position = -14;
  mainmenu[MENOPT - 2].width = 28;
#ifdef DJGPP
#ifdef PROG
  mainmenu[MENOPT - 2].no_of_items = 10;
#else
  mainmenu[MENOPT - 2].no_of_items = 8;
#endif
#else
#ifdef UNIX
#ifdef PROG
  if(WpeIsProg())
#ifdef DEBUGGER
    mainmenu[MENOPT - 2].no_of_items = !WpeIsXwin()? 11 : 10;
#else
    mainmenu[MENOPT - 2].no_of_items = !WpeIsXwin()? 10 : 9;
#endif
  else
#endif
    mainmenu[MENOPT - 2].no_of_items = !WpeIsXwin()? 8 : 7;
#else
  mainmenu[MENOPT - 2].no_of_items = !WpeIsXwin()? 7 : 6;
#endif
#endif
  if((mainmenu[MENOPT - 2].menuitems = MALLOC(mainmenu[MENOPT - 2].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  if(f->ed->edopt & ED_CUA_STYLE)
  {
    mainmenu[MENOPT - 2].menuitems[0] = WpeFillSubmenuItem("Size/Move            ^L", 0, 'S', e_size_move);
    mainmenu[MENOPT - 2].menuitems[1] = WpeFillSubmenuItem("Zoom   Shift F6 / Alt Z", 0, 'Z', e_ed_zoom);
    mainmenu[MENOPT - 2].menuitems[4] = WpeFillSubmenuItem("Next        ^F6 / Alt N", 2, 'X', e_ed_next);
    mainmenu[MENOPT - 2].menuitems[5] = WpeFillSubmenuItem("Close       ^F4 / Alt X", 0, 'C', e_close_window);
  }
  else
  {
    mainmenu[MENOPT - 2].menuitems[0] = WpeFillSubmenuItem("Size/Move        Alt F2", 0, 'S', e_size_move);
    mainmenu[MENOPT - 2].menuitems[1] = WpeFillSubmenuItem("Zoom         F5 / Alt Z", 0, 'Z', e_ed_zoom);
    mainmenu[MENOPT - 2].menuitems[4] = WpeFillSubmenuItem("Next         F6 / Alt N", 2, 'X', e_ed_next);
    mainmenu[MENOPT - 2].menuitems[5] = WpeFillSubmenuItem("Close            Alt F3", 0, 'C', e_close_window);
  }
  mainmenu[MENOPT - 2].menuitems[2] = WpeFillSubmenuItem("Tile           Shift F4", 0, 'T', e_ed_tile);
  mainmenu[MENOPT - 2].menuitems[3] = WpeFillSubmenuItem("Cascade        Shift F5", 1, 'A', e_ed_cascade);
  mainmenu[MENOPT - 2].menuitems[6] = WpeFillSubmenuItem("List All          Alt 0", 0, 'L', e_list_all_win);
#ifndef DJGPP
  if(!WpeIsXwin())
#endif
    mainmenu[MENOPT - 2].menuitems[7] = WpeFillSubmenuItem("Output    Alt F5 / ^G P", 0, 'O', e_u_deb_out);
#ifdef PROG
  if(WpeIsProg())
  {
    mainmenu[MENOPT - 2].menuitems[mainmenu[MENOPT - 2].no_of_items - 2] = 
      WpeFillSubmenuItem("Messages", 0, 'M', e_p_show_messages);
    mainmenu[MENOPT - 2].menuitems[mainmenu[MENOPT - 2].no_of_items - 1] = 
      WpeFillSubmenuItem("Project", 0, 'P', e_show_project);
#ifdef DEBUGGER
    mainmenu[MENOPT - 2].menuitems[mainmenu[MENOPT - 2].no_of_items - 3] = 
      WpeFillSubmenuItem("Watches", 0, 'W', e_p_show_watches);
#endif
  }
#endif
#ifdef NEWSTYLE
  if(WpeIsXwin())
    mainmenu[MENOPT - 1].position = -21;
  else
#endif
    mainmenu[MENOPT - 1].position = -22;
  mainmenu[MENOPT - 1].width = 27;
#if defined(PROG) && !defined(DJGPP)
  mainmenu[MENOPT - 1].no_of_items = 8;
#else
  mainmenu[MENOPT - 1].no_of_items = 6;
#endif
  if((mainmenu[MENOPT - 1].menuitems = MALLOC(mainmenu[MENOPT - 1].no_of_items * sizeof(OPTK))) == NULL)
    e_error(e_msg[ERR_LOWMEM], 1, f->fb);
  mainmenu[MENOPT - 1].menuitems[0] = WpeFillSubmenuItem("Editor              F1", 0, 'E', e_help);
#if defined(PROG) && !defined(DJGPP)
  mainmenu[MENOPT - 1].menuitems[1] = WpeFillSubmenuItem("Topic Search       ^F1", 0, 'T', e_topic_search);
  mainmenu[MENOPT - 1].menuitems[2] = WpeFillSubmenuItem("FUnction Index  Alt F1", 1, 'U', e_funct_in);
/*   mainmenu[MENOPT-1].menuitems[2] = WpeFillSubmenuItem("Functions      Alt F1", 0, 'F', e_funct); */
#endif
  mainmenu[MENOPT - 1].menuitems[mainmenu[MENOPT - 1].no_of_items - 5] = 
    WpeFillSubmenuItem("Info                  ", 0, 'I', e_info);
  mainmenu[MENOPT - 1].menuitems[mainmenu[MENOPT - 1].no_of_items - 4] = 
    WpeFillSubmenuItem("Goto              <CR>", 0, 'G', e_hp_ret);
  mainmenu[MENOPT - 1].menuitems[mainmenu[MENOPT - 1].no_of_items - 3] = 
    WpeFillSubmenuItem("Back             <BSP>", 0, 'B', e_hp_back);
  mainmenu[MENOPT - 1].menuitems[mainmenu[MENOPT - 1].no_of_items - 2] = 
    WpeFillSubmenuItem("Next    Alt F8 / Alt T", 0, 'G', e_hp_next);
  mainmenu[MENOPT - 1].menuitems[mainmenu[MENOPT - 1].no_of_items - 1] = 
    WpeFillSubmenuItem("Prev.   Alt F7 / Alt V", 0, 'P', e_hp_prev);

  /* check for valid n */
  if(n < 0 || n > MENOPT - 1)
    n = 0;

  /* go until the user leaves the main control bar */
  while(c != WPE_ESC)
  {

    f = cn->f[cn->mxedt];
    if(e_tst_dfkt(f, c) == 0)
    {
      c = 0;
      break;
    }

    /* check for a menu shortcut */
    for(i = 0; i < MENOPT; i++)
      if(c == opt[i].s || c == opt[i].as)
      {
        n = i;
        c = WPE_CR;
      }

    /* if the selection is not the same as before do some drawing */
    if(nold != n)
    {
      /* paint unselected the option */
      e_pr_str_wsd(opt[nold].x, 0, opt[nold].t, f->fb->mt.fb, 0, 1,
                   f->fb->ms.fb, (nold == 0 ? 0 : opt[nold].x - e_mn_men),
       (nold == MENOPT - 1) ? MAXSCOL - 1 : opt[nold + 1].x - e_mn_men - 1);

      /* paint selected the option */
      e_pr_str_wsd(opt[n].x, 0, opt[n].t, f->fb->mz.fb, 0, 1,
                   f->fb->mz.fb, (n == 0 ? 0 : opt[n].x - e_mn_men),
             (n == MENOPT - 1) ? MAXSCOL - 1 : opt[n + 1].x - e_mn_men - 1);

      /* store the selected menu option for later use */
      nold = n;
    }

    if(c == WPE_CR)
#ifdef PROG
#ifdef DEBUGGER
    {
      if(!WpeIsProg() && n > 4)
        e_hlp = e_hlp_str[9 + n];
      else
        e_hlp = e_hlp_str[6 + n];
#else
    {
      if(!WpeIsProg() && n > 4)
        e_hlp = e_hlp_str[8 + n];
      else
        e_hlp = e_hlp_str[6 + n];
#endif
#else
    {
      e_hlp = e_hlp_str[6 + n];
#endif
      /* Handle user interaction with the submenu.
         If the user moved to another main option, the function returns. */
      if(mainmenu[n].width != 0)
        c = WpeHandleSubmenu(opt[n].x + mainmenu[n].position, 1,
                             opt[n].x + mainmenu[n].width + mainmenu[n].position, 
                             mainmenu[n].no_of_items + 2, n, mainmenu[n].menuitems, f);
      if(c < MENOPT)
      {
        n = c;
        c = WPE_CR;
      }
      else if(c != WPE_ESC)
        c = 0;
    }
    else
    {
      e_hlp = e_hlp_str[24];
#if MOUSE
      if((c = e_getch()) == -1)
        c = e_m1_mouse();
#else
      c = e_getch();
#endif
      c = e_toupper(c);
    }
    if(c == CDO || c == CtrlN)        /* down -> submenu open */
      c = WPE_CR;
    else if(c == CLE || c == CtrlB)   /* going left in the main menu */
      n--;
    else if(c == CRI || c == CtrlF)   /* going right in the main menu */
      n++;
    else if(c == POS1 || c == CtrlA)  /* most left in the main menu */
      n = 0;
    else if(c == ENDE || c == CtrlE)  /* most right in the main menu */
      n = MENOPT - 1;
    else if(c == AltX)                /* quit the whole business */
      c = e_quit(f);

    /* adjust the selected main menu */
    if(n < 0)
      n = MENOPT - 1;
    else if(n >= MENOPT)
      n = 0;
  }

  /* paint unselected the last selected main menu option */
  f = cn->f[cn->mxedt];
  e_pr_str_wsd(opt[nold].x, 0, opt[nold].t, f->fb->mt.fb, 0, 1,
               f->fb->ms.fb, (nold == 0 ? 0 : opt[nold].x - e_mn_men),
       (nold == MENOPT - 1) ? MAXSCOL - 1 : opt[nold + 1].x - e_mn_men - 1);

  /* free up the submenu structure */
  for(i = 0; i < MENOPT; i++)
    if(mainmenu[i].width != 0)
      FREE(mainmenu[i].menuitems);
  FREE(mainmenu);

  fk_cursor(1);
  return(c);
}

/* sub menu box */
int WpeHandleSubmenu(int xa, int ya, int xe, int ye, int nm, OPTK * fopt, FENSTER * f)
{
#if MOUSE
  extern struct mouse e_mouse;
  extern int      e_mn_men;
#endif
  PIC            *pic;
  int             i, n = 0, nold = 1, c = 0;
  extern OPT      opt[];

  /* save whatever will be behind the submenu */
#ifdef NEWSTYLE
  if(WpeIsXwin())
    pic = e_open_view(xa + 1, ya, xe - 1, ye, f->fb->mt.fb, 1);
  else
#endif
    pic = e_open_view(xa, ya, xe, ye, f->fb->mt.fb, 1);

  if(pic == NULL)
  {
    e_error(e_msg[ERR_LOWMEM], 0, f->fb);
    return(WPE_ESC);
  }

  /* draw the frame and the background of the submenu */
  e_std_rahmen(xa + 1, ya, xe - 1, ye, NULL, 0, f->fb->mr.fb, 0);

  /* draw the submenu items (colors are default, none selected) */
  for(i = ya + 1; i < ye; i++)
    e_pr_str_scan(xa + 3, i, fopt[i - ya - 1].t, f->fb->mt.fb, fopt[i - ya - 1].x, 1,
                  f->fb->ms.fb, xa + 2, xe - 2);

#if MOUSE
  /* show the actual changes */
  e_refresh();
  /* until mouse button released */
  while(e_mshit() != 0)
  {
    c = -1;
    /* mouse is in the range of the submenu */
    if(e_mouse.y > ya && e_mouse.y < ye)
    {
      n = e_mouse.y - ya - 1;
      /* if a new item is selected */
      if(nold != n)
      {
        /* check whether nold is in the range of the available items and unselect it */
        if(nold < ye - ya - 1 && nold >= 0)
          e_pr_str_scan(xa + 3, nold + ya + 1, fopt[nold].t, f->fb->mt.fb, fopt[nold].x,
                        1, f->fb->ms.fb, xa + 2, xe - 2);
        /* draw the newly selected item */
        e_pr_str_scan(xa + 3, n + ya + 1, fopt[n].t, f->fb->mz.fb, fopt[n].x, 1,
                      f->fb->mz.fb, xa + 2, xe - 2);
        /* save the selection */
        nold = n;
        e_refresh();
      }
    }
    /* mouse is in the main menu area */
    else if(e_mouse.y == 0)
    {
      /* search for the selected main menu option */
      for(i = 1; i < MENOPT; i++)
        if(e_mouse.x < opt[i].x - e_mn_men)
          break;
      /* check whether a new menu option has been selected */
      if(i != nm + 1)
      {
        /* if yes, restore what was behind */
        e_close_view(pic, 1);
        return(i - 1);
      }
    }
  }

  /* there was a selection and mouse is within the submenu !!! */
  if(c < 0 && e_mouse.y > ya && e_mouse.y < ye && e_mouse.x > xa &&
     e_mouse.x < xe)
    c = MBKEY;
#endif

  while(c != WPE_ESC)
  {
    if(nold != n)
    {
      /* check whether nold is in the range of the available items and unselect it */
      if(nold < ye - ya - 1 && nold >= 0)
        e_pr_str_scan(xa + 3, nold + ya + 1, fopt[nold].t, f->fb->mt.fb, fopt[nold].x,
                      1, f->fb->ms.fb, xa + 2, xe - 2);
      /* draw the newly selected item */
      e_pr_str_scan(xa + 3, n + ya + 1, fopt[n].t, f->fb->mz.fb, fopt[n].x, 1,
                    f->fb->mz.fb, xa + 2, xe - 2);
      /* save the selection */
      nold = n;
    }
#if MOUSE
    if(c != MBKEY)
    {
      if((c = e_toupper(e_getch())) == -1)
        c = e_m2_mouse(xa, ya, xe, ye, fopt);
    }
    else
      c = WPE_CR;  /* mouse released at a proper place, submenu item accepted */
#else
    c = e_toupper(e_getch());
#endif

    /* check main menu shortcut keys */
    for(i = 0; i < MENOPT; i++)
      if(c == opt[i].as)
      {
        e_close_view(pic, 1);
        return(i);
      }

    /* check submenu items' shortcut keys */
    for(i = 0; i < ye - ya - 1; i++)
      if(c == fopt[i].o)
      {
        e_close_view(pic, 1);
        fopt[i].fkt(f);
        return(WPE_ESC);
      }

    if(c == Alt0)
      c = WPE_ESC;

    if(i > ye - ya)
      c = WPE_ESC;
    else if(c == WPE_CR)     /* submenu item accepted */
    {
      e_close_view(pic, 1);
      fopt[n].fkt(f);
      return(WPE_ESC);
    }
    else if(c == CUP || c == CtrlP)     /* going up in the submenu */
      n = n > 0 ? n - 1 : ye - ya - 2;
    else if(c == CDO || c == CtrlN)     /* going down in the submenu */
      n = n < ye - ya - 2 ? n + 1 : 0;
    else if(c == CLE || c == CtrlB)     /* going left in the main menu */
    {
      c = nm > 0 ? nm - 1 : MENOPT - 1;
      break;
    }
    else if(c == CRI || c == CtrlF)     /* going right in the main menu */
    {
      c = nm < MENOPT - 1 ? nm + 1 : 0;
      break;
    }
    else if(c == POS1 || c == CtrlA)    /* to the top in the submenu */
      n = 0;
    else if(c == ENDE || c == CtrlE)    /* to the bottom in the submenu */
      n = ye - ya - 2;
    else
    {
      /* this is the anything else case, ESC, keys which are not listed in
         the submenu */
      e_close_view(pic, 1);
      if(c != WPE_ESC && e_tst_dfkt(f, c) == 0)
        return(WPE_ESC);
#ifdef NEWSTYLE
      pic = e_open_view(xa + 1, ya, xe - 1, ye, f->fb->mt.fb, 1);
#else
      pic = e_open_view(xa, ya, xe, ye, f->fb->mt.fb, 1);
#endif
      /* draw the frame for the submenu */
      e_std_rahmen(xa + 1, ya, xe - 1, ye, NULL, 0, f->fb->mr.fb, 0);
      /* draw every item in the submenu, unselected */
      for(i = ya + 1; i < ye; i++)
        e_pr_str_scan(xa + 3, i, fopt[i - ya - 1].t, f->fb->mt.fb, fopt[i - ya - 1].x, 1,
                      f->fb->ms.fb, xa + 2, xe - 2);
      /* draw the selected item */
      e_pr_str_scan(xa + 3, n + ya + 1, fopt[n].t, f->fb->mz.fb, fopt[n].x, 1,
                    f->fb->mz.fb, xa + 2, xe - 2);
    }
  }
  e_close_view(pic, 1);
  return(c == WPE_ESC ? 255 : c);
}

/*      fill "options" struct */
OPTK WpeFillSubmenuItem(char *t, int x, char o, int(*fkt) ())
{
  OPTK            opt;

  opt.t = t;
  opt.x = x;
  opt.o = o;
  opt.fkt = fkt;
  return(opt);
}

