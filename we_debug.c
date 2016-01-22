/* we_debug.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"

#ifndef NO_XWINDOWS
#include "WeXterm.h"
#endif

#ifdef DEBUGGER

#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#ifndef TERMCAP
/* Because term.h defines "buttons" it messes up we_gpm.c if in edit.h */
#include <term.h>
#endif

#define D_CBREAK -2
#define CTRLC CtrlC
#define SVLINES 12

int e_d_delbreak(FENSTER *f);
int e_d_error(char *s);

#define MAXOUT  2 * MAXSCOL * MAXSLNS

char *e_debugger, *e_deb_swtch = NULL, *e_d_save_schirm;
int e_d_swtch = 0, rfildes[2], ofildes, e_d_pid = 0;
int e_d_nbrpts = 0, e_d_zwtchs = 0, *e_d_ybrpts, *e_d_nrbrpts;

/* number of watch expressions in Watches window */
int e_d_nwtchs = 0; 

/* e_d_nrwtchs[i] is the y coordinate (count starts at 0) of the first line of
   the i-th watch in the Watches window */
int *e_d_nrwtchs; 

char **e_d_swtchs; /* e_d_swtchs[i] is the i-th watch expression (a char*) */

int e_d_nstack;
char e_d_file[128], **e_d_sbrpts;
char *e_d_out = NULL;
int e_deb_type = 0, e_deb_mode = 0;
char e_d_out_str[SVLINES][256];
char *e_d_sp[SVLINES];
char e_d_tty[80];

extern int wfildes[2], efildes[2];
extern struct termios otermio, ntermio, ttermio;
extern struct e_s_prog e_sv_prog;
extern BUFFER *e_p_w_buffer;
extern char *att_no;
extern char *e_tmp_dir;

#ifdef NOTPARM
char *tparm();
char *tgoto();
#endif
#ifdef DEFTPUTS
int tputs();
#endif

char *npipe[5] = {  NULL, NULL, NULL, NULL, NULL  };

char *e_d_msg[] = {  "Ctrl C pressed\nQuit Debugger ?",
			"Quit Debugger ?",
			"End of Debugging",
			"Program is not running",
			"No symbol in current context\n",
			"No Source-Code",       /*  Number 5  */
			"Start Debugger",
			"Can\'t start Debugger",
			"Starting program:",
			"Can\'t start Program",
			"Program exited",       /*   Number 10   */
			"Program terminated",
			"Program received signal",
			"Unknown Break\nQuit Debugger ?",
			"Can\'t find file %s",
			"Can\'t create named pipe",  /*   Number 15   */
			"Breakpoint",
			"(sig ",
			"program exited",
			"signal",
			"interrupt",      /* Number 20  */
			"stopped in",
			"BREAKPOINT",
			"Child process terminated normally",
			"software termination",
			"Continuing.\n",  /* Number 25  */
			"No stack.",
			"no process"
		  };

extern char *e_p_msg[];

int e_deb_inp(FENSTER *f)
{
 ECNT *cn = f->ed;
 int c = 0;

 f = cn->f[cn->mxedt];
 c = e_getch();
 switch(c)
 {
  case 'p':
  case ('p' - 'a' + 1):
   e_deb_out(f);
   break;
  case 'o':
  case ('o' - 'a' + 1):
   e_deb_options(f);
   break;
  case 'b':
  case ('b' - 'a' + 1):
   e_breakpoint(f);
   break;
  case 'm':
  case ('m' - 'a' + 1):
   e_remove_breakpoints(f);
   break;
  case 'a':
  case 1:
   e_remove_all_watches(f);
   break;
  case 'd':
  case ('d' - 'a' + 1):
   e_delete_watches(f);
   break;
  case 'w':
  case ('w' - 'a' + 1):
   e_make_watches(f);
   break;
  case 'e':
  case ('e' - 'a' + 1):
   e_edit_watches(f);
   break;
  case 'k':
  case ('k' - 'a' + 1):
   e_deb_stack(f);
   break;
  case 't':
  case ('t' - 'a' + 1):
   e_deb_trace(f);
   break;
  case 's':
  case ('s' - 'a' + 1):
   e_deb_next(f);
   break;
  case 'r':
  case 'c':
  case ('r' - 'a' + 1):
  case ('c' - 'a' + 1):
   e_deb_run(f);
   break;
  case 'q':
  case ('q' - 'a' + 1):
   e_d_quit(f);
   break;
  case 'u':
  case ('u'-'a'+1):
   e_d_goto_cursor(f);
   break;
  case 'f':
  case ('f'-'a'+1):
   e_d_finish_func(f);
   break;
  default:
   return(c);
 }
 return(0);
}

int e_d_q_quit(FENSTER *f)
{
 int ret = e_message(1, e_d_msg[ERR_QUITDEBUG], f);

 if (ret == 'Y')
  e_d_quit(f);
 return(0);
}

int e_debug_switch(FENSTER *f, int c)
{
 switch (c)
 {
  case F7:
   e_deb_trace(f);
   break;
  case F8:
   e_deb_next(f);
   break;
  case CF10:
   e_deb_run(f);
   break;
  case CF2:
   e_d_q_quit(f);
   break;
  default:
   if (f->ed->edopt & ED_CUA_STYLE)
   {
    switch (c)
    {
     case F5:
      e_breakpoint(f);
      break;
     case CF5:
      e_make_watches(f);
      break;
     case CF3:
      e_deb_stack(f);
      break;
     default:
      return(c);
    }
   }
   else
   {
    switch (c)
    {
     case CF8:
      e_breakpoint(f);
      break;
     case CF7:
      e_make_watches(f);
      break;
     case CF6:
      e_deb_stack(f);
      break;
     default:
      return(c);
    }
   }
 }
 return(0);
}

/*  Input Routines   */
int e_e_line_read(int n, signed char *s, int max)
{
 int i, ret = 0;

 for (i = 0; i < max - 1; i++)
 {
  ret = read(n, s + i, 1);
  if (ret != 1 || s[i] == EOF || s[i] == '\n'|| s[i] == '\0')
   break;
 }
 if (ret != 1 && i == 0)
  return(-1);
 s[i+1] = '\0';
 if (e_deb_type == 1 && s[i] == '*')
  return(1);
 if (e_deb_type == 3 && s[i] == '>')
  return(1);
 else if (e_deb_type == 2 && i > 4 && s[i] == ' ' && !strncmp(s+i-5, "(dbx)", 5))
  return(1);
 else if (e_deb_type == 0 && i > 4 && s[i] == ' ' && !strncmp(s+i-5, "(gdb)", 5))
  return(1);
 return(2);
}

int e_d_line_read(int n, signed char *s, int max, int sw, int esw)
{
 static char wt = 0, esc_sv = 0, str[12];
 int i, j, ret = 0, kbdflgs;

 if (esw)
 {  if((ret = e_e_line_read(efildes[0], s, max)) >= 0) return(ret);  }
 else
 {  while(e_e_line_read(efildes[0], s, max) >= 0);  }
 for(i = 0; i < max - 1; i++)
 {
  if (esc_sv)  {  s[i] = WPE_ESC;  esc_sv = 0;  continue;  }
  kbdflgs = fcntl(n, F_GETFL, 0 );
  fcntl( n, F_SETFL, kbdflgs | O_NONBLOCK);
  while ((ret = read(n, s + i, 1)) <= 0 && i == 0 && wt >= sw)
   if(e_d_getchar() == D_CBREAK) return(-1);
/*	Read until no chars are left anymore
   	Return if buffer not empty
        return(-1) if CBREAK
*/
  fcntl( n, F_SETFL, kbdflgs & ~O_NONBLOCK);
  if (ret == -1) break;
  else if (ret != 1 ||  s[i] == EOF || s[i] == '\0') break;
  else if(s[i] == '\n' || s[i] == WPE_CR)  break;
  else if(s[i] == WPE_ESC)
  {  s[i] = '\0';  esc_sv = 1;  break;  }
 }
 if (ret != 1)
 {
  s[i] = '\0';
  if(e_deb_type == 1 && i > 0 && s[i-1] == '*')
  {  str[0] = 0;  wt = 0;   return(1);  }
  else if(e_deb_type == 3 && i > 0 && s[i-1] == '>')
  {  str[0] = 0;  wt = 0;   return(1);  }
  else if(e_deb_type == 0)
  {
   if(i > 5 && !strncmp(s+i-6, "(gdb) ", 6))
   {  str[0] = 0;  wt = 0;   return(1);  }
   else if(i < 6)
   {
    for (j = 0; j <= i; j++) str[6+j] = s[j];
    if (!strncmp(str+i-6, "(gdb) ", 6))
    {  str[0] = '\0';  wt = 0;  return(1);  }
   }
  }
  else if (e_deb_type == 2)
  {
   if(i > 5 && !strncmp(s+i-6, "(dbx) ", 6))
   {  str[0] = 0;  wt = 0;   return(1);  }
   else if(i < 6)
   {
    for(j = 0; j <= i; j++) str[6+j] = s[j];
    if(!strncmp(str+i-6, "(dbx) ", 6))
    {  str[0] = '\0';  wt = 0;  return(1);  }
   }
  }
 }
 else {  s[i+1] = '\0';  }
 if (i != 0) wt = 0;
 else wt++;
 if(i > 4) for(j = 0; j < 6; j++) str[j] = s[j+i-5];
 return(0);
}

int e_d_dum_read()
{
 char str[256];
 int ret;

 while ((ret = e_d_line_read(wfildes[0], str, 128, 0, 0)) == 0 || ret == 2)
  if (ret == 2)
   e_d_error(str);
 return(ret);
}

/* Output Routines */
int e_d_p_exec(FENSTER *f)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 SCHIRM *s;
 int ret, i, is, j;
 char str[512];

 for (i = cn->mxedt; i > 0 && strcmp(cn->f[i]->datnam, "Messages"); i--)
  ;
 if (i <= 0)
 {  e_edit(cn, "Messages");  i = cn->mxedt;  }
 f = cn->f[i];
 b = cn->f[i]->b;
 s = cn->f[i]->s;
 if (b->bf[b->mxlines-1].len != 0)
  e_new_line(b->mxlines, b);
 for (j = 0, i = is = b->mxlines-1;
   (ret = e_d_line_read(wfildes[0], str, 512, 0, 0)) == 0; )
 {
  if (ret == -1)
   return(ret);
  print_to_end_of_buffer(b, str, b->mx.x);
 }
 if (ret == 1)
 {
  e_new_line(i, b);
  for (j = 0; j < NUM_COLS_ON_SCREEN_SAFE - 2 && str[j] != '\n' &&
    str[j] != '\0'; j++)
   *(b->bf[i].s+j) = str[j];
  b->b.y = i;
  b->b.x = b->bf[i].len = j;
  b->bf[i].nrc = j;
 }
 b->b.y = b->mxlines-1;
 b->b.x = 0;

 e_rep_win_tree(cn);
 return(ret);
}

/*    Help Routines   */
int e_d_getchar()
{
 int i = 1, fd;
 char c = 0, kbdflgs = 0;

 if (WpeIsXwin()) fd = rfildes[0];
 else fd = 0;

/*   if(WpeIsXwin() || e_deb_mode)    */
 {
  kbdflgs = fcntl(fd, F_GETFL, 0 );
  fcntl( fd, F_SETFL, kbdflgs | O_NONBLOCK);
 }
#ifndef NO_XWINDOWS
 if (WpeIsXwin())
  c = (*e_u_change)(NULL);
#endif
 if (c || (i = read(fd, &c, 1)) == 1)
 {
  if (c == CTRLC)
  {
/*	 if (WpeIsXwin() || e_deb_mode)    */
   fcntl(fd, F_SETFL, kbdflgs & ~O_NONBLOCK);
   e_d_switch_out(0);
   i = e_message(1, e_d_msg[ERR_CTRLCPRESS], WpeEditor->f[WpeEditor->mxedt]);
   if (i == 'Y')
   {
    e_d_quit(WpeEditor->f[WpeEditor->mxedt]);
    return(D_CBREAK);
   }
   else
    return(c);
  }
  else
   write(rfildes[1], &c, 1);
 }
/*   if (WpeIsXwin() || e_deb_mode)   */
 fcntl(fd, F_SETFL, kbdflgs & ~O_NONBLOCK);
 return(i == 1 ? c : 0);
}

int e_d_is_watch(int c, FENSTER *f)
{
 if (strcmp(f->datnam, "Watches"))
  return(0);
 if(c == EINFG)
  return(e_make_watches(f));
 else if (c == ENTF)
  return(e_delete_watches(f));
 else
  return(0);
}

int e_d_quit_basic(FENSTER *f)
{
 int i, kbdflgs;

 if (!e_d_swtch)
  return 0;
 if (rfildes[1] >= 0)
 {
  if (e_deb_type == 0 || e_deb_type == 3)
   write(rfildes[1], "q\ny\n", 4);
  else if (e_deb_type == 1)
   write(rfildes[1], "q\n", 2);
  else if (e_deb_type == 2)
   write(rfildes[1], "quit\n", 5);
 }
 kbdflgs = fcntl(0, F_GETFL, 0 );
 fcntl(0, F_SETFL, kbdflgs & ~O_NONBLOCK);
 e_d_swtch = 0;
 if (e_d_pid)
 {
  kill(e_d_pid, 9);
  e_d_pid = 0;
 }
 if (!WpeIsXwin())
 {
  if (e_d_out)
   FREE(e_d_out);
  e_d_out = NULL;
 }
 if (rfildes[1] >= 0)
  close(rfildes[1]);
 if (wfildes[0] >= 0)
  close(wfildes[0]);
 if (efildes[0] >= 0)
  close(efildes[0]);
 if (rfildes[0] >= 0)
  close(rfildes[0]);
 if (wfildes[1] >= 0)
  close(wfildes[1]);
 if (WpeIsXwin())
 {
  remove(npipe[0]);
  remove(npipe[1]);
  remove(npipe[2]);
  remove(npipe[3]);
  remove(npipe[4]);
 }
 else
 {
  if (efildes[1] >= 0)
   close(efildes[1]);
  if (!e_deb_mode)
   e_g_sys_end();
  else
  {
   e_d_switch_out(1);
   fk_locate(MAXSCOL, MAXSLNS);
   e_putp("\r\n");
   e_putp(att_no);
   e_d_switch_out(0);
  }
 }
}

int e_d_quit(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;
 e_d_quit_basic(f);
 e_d_p_message(e_d_msg[ERR_ENDDEBUG], f, 1);
 WpeMouseChangeShape(WpeEditingShape);
 e_d_delbreak(f);
 for (i = cn->mxedt; i > 0; i--)
  if (!strcmp(cn->f[i]->datnam, "Messages"))
  {
   e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
   break;
  }
 if (i <= 0)
  e_edit(cn, "Messages");
 return(0);
}

/*    Watches   */
int e_d_add_watch(char *str, FENSTER *f)
{
 int ret;

 ret = e_add_arguments(str, "Add Watch", f, 0, AltA, &f->ed->wdf);
 if (ret != WPE_ESC)
 {
  f->ed->wdf = e_add_df(str, f->ed->wdf);
 }
 fk_cursor(1);
 return(ret);
}

int e_remove_all_watches(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i, n;

 if (e_d_nwtchs < 1) return(0);
 for (n = 0; n < e_d_nwtchs;n++) FREE(e_d_swtchs[n]);
 FREE(e_d_swtchs);
 FREE(e_d_nrwtchs);
 e_d_nwtchs = 0;
 for (i = cn->mxedt; i > 0; i--)
 {
  if (!strcmp(cn->f[i]->datnam, "Watches"))
  {
   e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
   e_close_window(cn->f[cn->mxedt]);
   break;
  }
 }
 e_close_buffer(e_p_w_buffer);
 e_p_w_buffer = NULL;
 return(0);
}

int e_delete_watches(FENSTER *f)
{
 ECNT *cn = f->ed;
 BUFFER *b = cn->f[cn->mxedt]->b;
 int n;

 f = cn->f[cn->mxedt];
 if (e_d_nwtchs < 1 || strcmp(f->datnam, "Watches"))
  return(0);
 for (n = 0; n < e_d_nwtchs && e_d_nrwtchs[n] <= b->b.y; n++)
  ;
 FREE(e_d_swtchs[n - 1]);
 for (; n < e_d_nwtchs; n++)
  e_d_swtchs[n - 1] = e_d_swtchs[n];
 e_d_nwtchs--;
 e_d_swtchs = REALLOC(e_d_swtchs, e_d_nwtchs * sizeof(char *));
 e_d_nrwtchs = REALLOC(e_d_nrwtchs, e_d_nwtchs * sizeof(int));
 e_d_p_watches(f, 0);
 return(0);
}

int e_make_watches(FENSTER *f)
{
 char str[128];
 int i, y;

 if ((f->ed->mxedt > 0) && (strcmp(f->datnam, "Watches") == 0))
 { /* sets y=number of watch we're inserting */
  for(y = 0; y < e_d_nwtchs && e_d_nrwtchs[y] < f->b->b.y; y++)
   ;
 }
 else
  y = e_d_nwtchs;
 if (f->ed->wdf && f->ed->wdf->anz > 0)
  strcpy(str, f->ed->wdf->name[0]);
 else
  str[0] = '\0';
 if (e_d_add_watch(str, f))
 {
  e_d_nwtchs++;
  if (e_d_nwtchs == 1)
  {
   e_d_swtchs = MALLOC(sizeof(char *));
   e_d_nrwtchs = MALLOC(sizeof(int));
  }
  else
  {
   e_d_swtchs = REALLOC(e_d_swtchs, e_d_nwtchs * sizeof(char *));
   e_d_nrwtchs = REALLOC(e_d_nrwtchs, e_d_nwtchs * sizeof(int));
  }
  
  /*
    move watch number y and following up one position so that we can insert 
    at position y 
  */
  for (i = e_d_nwtchs - 1; i > y; i--)
  {
   e_d_swtchs[i] = e_d_swtchs[i-1];
   
   /* The following instruction is pointless as e_d_nrwtchs[i] is invalidated
      by inserting the new watch and has to be recomputed by e_d_p_watches()
   */
   e_d_nrwtchs[i] = e_d_nrwtchs[i-1];
  }
  e_d_swtchs[y] = MALLOC(strlen(str) + 1); /* insert...                    */
  strcpy(e_d_swtchs[y], str);              /*       ... new watch at pos y */
  e_d_p_watches(f, 1);
  return(0);
 }
 return(-1);
}

int e_edit_watches(FENSTER *f)
{
 BUFFER *b = f->ed->f[f->ed->mxedt]->b;
 char str[128];
 int l;

 if (strcmp(f->datnam, "Watches"))
  return(0);
 for (l = 0; l < e_d_nwtchs && e_d_nrwtchs[l] <= b->b.y; l++)
  ;
 if (l == e_d_nwtchs && b->bf[b->b.y].len == 0)
  return(e_make_watches(f));
 strcpy(str, e_d_swtchs[l - 1]);
 if (e_d_add_watch(str, f))
 {
  e_d_swtchs[l - 1] = REALLOC(e_d_swtchs[l - 1], strlen(str) + 1);
  strcpy(e_d_swtchs[l - 1], str);
  e_d_p_watches(f, 0);
  return(0);
 }
 return(-1);
}

/* Among other things, e_d_p_watches() must recompute e_d_nrwtchs when
   called from e_edit_watches(), 
   but has code paths that don't do this ==> possible BUG
*/
int e_d_p_watches(FENSTER *f, int sw)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 SCHIRM *s;
 int iw, i, k = 0, l, ret;
 char str1[256], *str; /* is 256 always large enough? */
 char *str2;

 e_d_switch_out(0);
 if ((e_d_swtch > 2) && (e_d_p_stack(f, 0) == -1))
  return(-1);
 /* Find the watch window */
 for (iw = cn->mxedt; iw > 0 && strcmp(cn->f[iw]->datnam, "Watches"); iw--);
 if (iw == 0 && !e_d_nwtchs)
 { /* if no watches and the mysterious iw!=0 then just repaint window tree */
  e_rep_win_tree(cn);
  return(0);
 }
 else if (iw == 0)
 {
  if(e_edit(cn, "Watches"))
  {
   return(-1);
  }
  else
  {
   iw = cn->mxedt;
  }
 }
 f = cn->f[iw];
 b = cn->f[iw]->b;
 s = cn->f[iw]->s;
 
 /* free all lines of BUFFER b */
 e_p_red_buffer(b);
 FREE(b->bf[0].s);
 b->mxlines=0;

 for (i = 0, l = 0; l < e_d_nwtchs; l++)
 {
  str = str1;
  
  /* Create appropriate command for the debugger */
  if (e_deb_type == 0 || e_deb_type == 3)
  {
   sprintf(str1, "p %s\n", e_d_swtchs[l]);
  }
  else if (e_deb_type == 1)
  {
   sprintf(str1, "%s/\n", e_d_swtchs[l]);
  }
  else if (e_deb_type == 2)
  {
   sprintf(str1, "print %s\n", e_d_swtchs[l]);
  }

  /* Send command to debugger */
  if(e_d_swtch)
  {
   write(rfildes[1], str1, strlen(str1));
  }

  /* If no debugger or no response, give message of no symbol in context */
  if (!e_d_swtch || (ret = e_d_line_read(wfildes[0], str1, 256, 0, 0)) == 1)
  {
   strcpy(str1, e_d_msg[ERR_NOSYMBOL]);
   k = 0;
  }
  else /* Debugger successfully returned a value */
  {
   if (ret == -1)
   {
    return(ret); /* BUG? e_d_nrwtchs not initialized if this return is taken */
   }
   str = MALLOC(strlen(str1) + 1);
   strcpy(str, str1);
   while ((ret = e_d_line_read(wfildes[0], str1, 256, 0, 0)) == 0 || ret == 2)
   {
    if (ret == -1) return(ret); /* BUG? e_d_nrwtchs not initialized if this return is taken */
    if (ret == 2) e_d_error(str1);
    str = REALLOC(str, (k = strlen(str)) + strlen(str1) + 1);
    if (str[k-1] == '\n') str[k-1] = ' ';
    strcat(str, str1);
   }

   /* Find the beginning of the information (depends on debugger output
     format) */
   if (e_deb_type == 0 || e_deb_type == 2 || e_deb_type == 3)
   {
    if (e_deb_type == 3 && str[0] == '0')
    {
     for(k = 1; str[k] != '\0' && !isspace(str[k]); k++);
    }
    else
     for(k = 0; str[k] != '\0' && str[k] != '='; k++);
    if (str[k] == '\0')
    {
     if (str != str1) FREE(str);
     str = str1;
     strcpy(str, e_d_msg[ERR_NOSYMBOL]);
     k = 0;
    }
    for(k++; str[k] != '\0' && isspace(str[k]); k++);
   }
   else if(e_deb_type == 1)
   {
    for (k = 0; str[k] != '\0' && str[k] != ':'; k++);
    if (str[k] == '\0') k = 0;
    else k++;
   }
  }

  /* Print variable name */
  for ( ; str[k] != '\0' && isspace(str[k]); k++);
  str2 = WpeMalloc(strlen(e_d_swtchs[l]) + strlen(str + k) + 4);
  sprintf(str2, "%s: %s", e_d_swtchs[l], str + k);

  e_d_nrwtchs[l] = b->mxlines;
  print_to_end_of_buffer(b, str2, b->mx.x);

  /* Free any allocated string */
  WpeFree(str2);
  if(str != str1)
  {
   FREE(str);
  }
 }

 e_new_line(b->mxlines, b);
 fk_cursor(1);
/* if (b->b.y > i || sw) b->b.y = i;*/
 if (sw && iw != cn->mxedt) e_switch_window(cn->edt[iw], cn->f[cn->mxedt]);
 else e_rep_win_tree(cn);
/* e_d_switch_out(0);   */
 return(0);
}

int e_p_show_watches(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0; i--)
  if (!strcmp(f->ed->f[i]->datnam, "Watches"))
  {
   e_switch_window(f->ed->edt[i], f->ed->f[f->ed->mxedt]);
   break;
  }
 if (i <= 0 && e_edit(f->ed, "Watches"))
 {
  return(-1);
 }
 return(0);
}

/***************************************/
/***  reinitialize watches from prj  ***/
int e_d_reinit_watches(FENSTER * f,char * prj)
{
 int i,e,g,q,y,r;
 char * prj2;

 for(i = f->ed->mxedt; i > 0; i--)
 {
  if (!strcmp(f->ed->f[i]->datnam, "Watches"))
  {  
   e_remove_all_watches(f->ed->f[f->ed->edt[i]]);
   break; 
  }
 }
 g=strlen(prj);
 prj2=MALLOC(sizeof(char)*(g+1));
 strcpy(prj2,prj);
 q=0;
 y=0;
 r=0;
 while(q<g) 
 {
  e=q;
  while(prj2[e]!=';' && e<g) e++;
  prj2[e]='\0';
  q=e+1;
  r++;
 } 
 e_d_nwtchs=r;
 e_d_swtchs = (char **) MALLOC(e_d_nwtchs * sizeof(char *));
 e_d_nrwtchs =(int *) MALLOC(e_d_nwtchs * sizeof(int));   

 for(e=0,q=0;e<r;e++)
 {
  e_d_swtchs[e] = MALLOC(strlen(prj2+q)+1);
  strcpy(e_d_swtchs[e], prj2+q); 
  q+=strlen(prj2+q)+1;
 } 
 FREE(prj2);
 e_d_p_watches(f, 1);   
 return 0;
}
/***************************************/

/*  stack   */
int e_deb_stack(FENSTER *f)
{
 e_d_switch_out(0);
 return(e_d_p_stack(f, 1));
}

int e_d_p_stack(FENSTER *f, int sw)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 SCHIRM *s;
 int is, i, j, k, l, ret;
 char str[256];

 if (e_d_swtch < 3)
  return(e_error(e_d_msg[ERR_NOTRUNNING], 0, f->fb));
 for (i = 0; i < SVLINES; i++)
  e_d_out_str[i][0] = '\0';
 for (is = cn->mxedt; is > 0 && strcmp(cn->f[is]->datnam, "Stack"); is--)
  ;
 if (!sw && is == 0)
  return(0);
 if(is == 0)
 {
  if (e_edit(cn, "Stack"))
   return(-1);
  else
   is = cn->mxedt;
 }
 f = cn->f[is];
 b = cn->f[is]->b;
 s = cn->f[is]->s;
 if (!e_d_swtch)
  return(0);
 if (e_deb_type == 0)
  write(rfildes[1], "bt\n", 3);
 else if (e_deb_type == 1 || e_deb_type == 3)
  write(rfildes[1], "t\n", 2);
 else if (e_deb_type == 2)
  write(rfildes[1], "where\n", 6);
 while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 2)
  e_d_error(str);
 if (ret == -1)
  return(-1);
 i = j = 0;
 if (ret == 1)
 {
  e_d_error(e_d_msg[ERR_PROGEXIT]);
  return(e_d_quit(f));
 }
 while(ret != 1)
 {
  k = 0;
  do
  {
   if (i >= b->mxlines)
    e_new_line(i, b);
   if ((i > 0 && j == 0 && *(b->bf[i-1].s+b->bf[i-1].len-1) == '\\') ||
     (!e_deb_type && j == 0 && (k > 0 || str[k] != '#')))
   {
    for(j = 0; j < 3; j++)
     b->bf[i].s[j] = ' ';
   }
   for(; isspace(str[k]); k++)
    ;
   for(; j < NUM_COLS_ON_SCREEN_SAFE - 2 && str[k] != '\n' && str[k] != '\0';
     j++, k++)
    *(b->bf[i].s+j) = str[k];
   if (str[k] != '\0')
   {
    if (str[k] != '\n')
    {
     for(l = j-1; l > 2 && !isspace(b->bf[i].s[l]) && b->bf[i].s[l] != '=';
       l--)
      ;
     if (l > 2)
     {
      k -= (j - l - 1);
      for (l++; l < j; l++)
       b->bf[i].s[l] = ' ';
     }
     *(b->bf[i].s+j) = '\\';
     *(b->bf[i].s+j+1) = '\n';
     *(b->bf[i].s+j+2) = '\0';
     j++;
    }
    else
    {
     *(b->bf[i].s+j) = '\n';
     *(b->bf[i].s+j+1) = '\0';
    }
   }
   b->bf[i].len = j;
   b->bf[i].nrc = j + 1;
   if (j != 0 && str[k] != '\0')
   {
    i++;
    j = 0;
   }
   else
    j--;
  }
  while (str[k] != '\n' && str[k] != '\0');
  while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 2)
   e_d_error(str);
  if (ret == -1)
   return(-1);
 }
 for (; i < b->mxlines; i++)
  e_del_line(i, b, s);
 if (sw && is != cn->mxedt)
  e_switch_window(cn->edt[is], cn->f[cn->mxedt]);
 e_rep_win_tree(cn);
 return(0);
}

int e_make_stack(FENSTER *f)
{
   char file[128], str[128], *tmpstr = MALLOC(1);
   int i, ret, line = 0, dif;
   BUFFER *b = f->ed->f[f->ed->mxedt]->b;
   e_d_switch_out(0);
   if(e_deb_type != 1)
   {  tmpstr[0] = '\0';
      if(e_deb_type == 0)
      {  for(i = dif = 0; i <= b->b.y; i++)
	    if(b->bf[i].s[0] == '#') dif = atoi((char *) (b->bf[i].s + 1));
	 for(i = b->b.y; i >= 0 && b->bf[i].s[0] != '#'; i--);
	 if(i < 0) return(1);
	 for(; i < b->mxlines; i++)
	 {  if(!(tmpstr = REALLOC(tmpstr, strlen(tmpstr) + b->bf[i].len + 2)))
	    {  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
	    strcat(tmpstr, (char *) b->bf[i].s);
	    if(i == b->mxlines-1 || b->bf[i+1].s[0] == '#') break;
	    else if(tmpstr[strlen(tmpstr)-2] == '\\')
	       tmpstr[strlen(tmpstr)-2] = '\0';
	    else if(tmpstr[strlen(tmpstr)-1] == '\n')
	       tmpstr[strlen(tmpstr)-1] = '\0';
	 }
      }
      else
      {  for(i = 1, dif = 0; i <= b->b.y; i++)
	    if(b->bf[i-1].s[b->bf[i-1].len - 1] != '\\') dif++;
	 for(i = b->b.y; i > 0 && b->bf[i-1].s[b->bf[i-1].len - 1] == '\\';
	    i--);
	 if(i == 0 && b->bf[i].len == 0) return(1);
	 for(; i < b->mxlines; i++)
	 {  if(!(tmpstr = REALLOC(tmpstr, strlen(tmpstr) + b->bf[i].len + 2)))
	    {  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
	    strcat(tmpstr, (char *) b->bf[i].s);
	    if(i == b->mxlines-1 || b->bf[i].s[b->bf[i].len - 1] != '\\')
	       break;
	    else if(tmpstr[strlen(tmpstr)-2] == '\\')
	       tmpstr[strlen(tmpstr)-2] = '\0';
	    else if(tmpstr[strlen(tmpstr)-1] == '\n')
	       tmpstr[strlen(tmpstr)-1] = '\0';
	 }
      }

      if(e_deb_type == 3 && (line = e_make_line_num2(tmpstr, file)) < 0)
	 return(e_error(e_d_msg[ERR_NOSOURCE], 0, f->fb));
      else if(e_deb_type != 3 && (line = e_make_line_num(tmpstr, file)) < 0)
	 return(e_error(e_d_msg[ERR_NOSOURCE], 0, f->fb));
      if(dif > e_d_nstack)
	 sprintf(str, "%s %d\n",
	 e_deb_type != 3 ? "up" : "down", dif - e_d_nstack);
      else if(dif < e_d_nstack)
	 sprintf(str, "%s %d\n",
	 e_deb_type != 3 ? "down" : "up", e_d_nstack - dif);
      if(dif != e_d_nstack)
      {  write(rfildes[1], str, strlen(str));
	 while((ret = e_d_line_read(wfildes[0], str, 128, 0, 0)) == 0 || ret == 2)
	    if( ret == 2) e_d_error(str);
	 if(ret == -1) return(ret);
	 e_d_nstack = dif;
      }
   }
   else if(e_deb_type == 1)
   {  for(i = b->b.y; i >= 0 && (line =
	 e_make_line_num2((char *)b->bf[i].s, file)) < 0; i--);
   }
   if(e_d_p_watches(f, 0) == -1) return(-1);
   e_d_goto_break(file, line, f);
   return(0);
}
/*******************************************************/
/** resyncing schirm - screen output with breakpoints **/

int e_brk_schirm(FENSTER *f)
{
 int i;
 int n;

 SCHIRM *s = f->s;
 s->brp= REALLOC(s->brp, sizeof(int));
 s->brp[0]=0;
 for(i=0;i<e_d_nbrpts;i++)
 {
  if(!strcmp(f->datnam,e_d_sbrpts[i]))
  {
   for(n=1;n<= (s->brp[0]);n++) if(e_d_ybrpts[i]==(s->brp[n])) break;
   if(n>s->brp[0]) 
   {
    /****  New break, not in schirm  ****/   
    (s->brp[0])++;
    s->brp = REALLOC(s->brp, (s->brp[0]+1) * sizeof(int));
    s->brp[s->brp[0]] = e_d_ybrpts[i]-1; 
   }
  }
 }
 return 0;
}
/*****************************************/

/*******************************************/
/***  reinitialize breakpoints from prj  ***/
int e_d_reinit_brks(FENSTER * f,char * prj)
{
   int line,e,g,q,r;
   char * p,*name,*prj2;
/***  remove breakpoints, schirms will be synced later  ***/

   e_remove_breakpoints(f);
   g=strlen(prj);
   prj2=MALLOC(sizeof(char)*(g+1));
   strcpy(prj2,prj);
   q=0;
   r=0;
   while(q<g) 
   {
     e=q;
     while(prj2[e]!=';' && e<g) e++;
     prj2[e]='\0';
     q=e+1;
     r++;
   } 
/**** for sure ****/   
   e_d_nbrpts=0;
   
/**** allocate memory for breakpoints ****/   
   e_d_sbrpts = MALLOC(sizeof(char *) * r);
   e_d_ybrpts = MALLOC(sizeof(int) * r);
   e_d_nrbrpts = MALLOC(sizeof(int) * r);
   
   name=prj2;
   for(q=0;q<r;q++)
   {
     p=strrchr(name,':');
     e=strlen(name);
     if(p!=NULL)
     {
       *p='\0';
       {
         p++;
         line=atoi(p);
         if(line>0) {
/**** hopefully we have filename and line number ****/

	   e_d_ybrpts[e_d_nbrpts]=line;
	   e_d_sbrpts[e_d_nbrpts]=MALLOC(sizeof(char)*(strlen(name)+1));
	   strcpy(e_d_sbrpts[e_d_nbrpts],name);
	   e_d_nbrpts++;
	   
/**** needed to keep schirm in sync ****/
	   
	   for(g = f->ed->mxedt; g > 0; g--)
            if(!strcmp(f->ed->f[g]->datnam, name))
            {  
              e_brk_schirm(f->ed->f[g]);
            }
         }
       }
     }
     name+=e+1;
   }
   FREE(prj2);
   return 0;
}


/**** Recalculate breakpoints , because of line/block
    deleting/adding ****/
int e_brk_recalc(FENSTER *f, int start, int len)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 int n, rend, count, yline;
 int *br_lines;

 if ((len == 0) || (cn == NULL))
  return 1;
 b = cn->f[cn->mxedt]->b;

 rend = start - 1 + abs(len);
 yline = b->b.y;

/**** deleting removed breakpoints ****/
 if (len < 0)
 {
  for (n = 0; n < e_d_nbrpts; n++)
   if ((!strcmp(f->datnam, e_d_sbrpts[n])) &&
     (e_d_ybrpts[n] <= (rend + 1)) && (e_d_ybrpts[n] >= (start + 1))) 
   {
    b->b.y = e_d_ybrpts[n] - 1;
    e_make_breakpoint(f, 0);
   }
 }

/**** scanning for breakpoints to move ****/
 for (count = 0, n = 0; n < e_d_nbrpts; n++)
  if ((!strcmp(f->datnam, e_d_sbrpts[n])) && (e_d_ybrpts[n] >= (start + 1)))
   count++;
 if (count == 0)
  return 1;
 br_lines = (int*)malloc(sizeof(int) * count);
 for (n = 0, count = 0; n < e_d_nbrpts; n++)
  if ((!strcmp(f->datnam, e_d_sbrpts[n])) && (e_d_ybrpts[n] >= (start + 1))) 
  {
   br_lines[count++] = e_d_ybrpts[n];
  }

/**** moving breakpoints ****/
 for(n = 0; n < count; n++) 
 {
  b->b.y = br_lines[n] - 1;
  e_make_breakpoint(f, 0);
 }
 for(n = 0; n < count; n++) 
 {
  b->b.y = br_lines[n] + len - 1;
  e_make_breakpoint(f, 0);
 }
 b->b.y = yline;
 free(br_lines);
 return 0;
}
/*****************************************/

/*  Breakpoints   */
int e_breakpoint(FENSTER *f)
{
 return(e_make_breakpoint(f, 0));
}

int e_remove_breakpoints(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 if (e_d_swtch)
 {
  if (!e_deb_type)
   write(rfildes[1], "d\ny\n", 4);
  else if (e_deb_type == 1)
   write(rfildes[1], "D\n", 2);
  else if (e_deb_type == 2)
   write(rfildes[1], "delete all\n", 11);
  else if (e_deb_type == 3)
   write(rfildes[1], "db *\n", 2);
 }
 for (i = 0; i < e_d_nbrpts; i++)
  FREE(e_d_sbrpts[i]);
 for (i = cn->mxedt; i >= 0; i--)
  if (DTMD_ISTEXT(cn->f[i]->dtmd))
   cn->f[i]->s->brp[0] = 0;
 e_d_nbrpts = 0;
 if (e_d_sbrpts)
 {
  FREE(e_d_sbrpts);
  e_d_sbrpts = NULL;
 }
 if (e_d_ybrpts)
 {
  FREE(e_d_ybrpts);
  e_d_ybrpts = NULL;
 }
 if (e_d_nrbrpts)
 {
  FREE(e_d_nrbrpts);
  e_d_nrbrpts = NULL;
 }
 e_rep_win_tree(cn);
 return(0);
}

int e_mk_brk_main(FENSTER *f, int sw)
{
 int i, ret;
 char eing[128], str[256];

 if (sw)
 {
  if (e_d_swtch)
  {
   if (e_deb_type == 0) sprintf(eing, "d %d\n", e_d_nrbrpts[sw-1]);
   else if (e_deb_type == 2)
    sprintf(eing, "delete %d\n", e_d_nrbrpts[sw-1]);
   else if (e_deb_type == 3)
    sprintf(eing, "db %d\n", e_d_nrbrpts[sw-1]);
   else if (e_deb_type == 1)
   {
    sprintf(eing, "e main\n");
    write(rfildes[1], eing, strlen(eing));
    if (e_d_dum_read() == -1) return(-1);
    sprintf(eing, "%d d\n", e_d_ybrpts[sw-1]);
   }
   write(rfildes[1], eing, strlen(eing));
   if (e_d_dum_read() == -1) return(-1);
  }
  FREE(e_d_sbrpts[sw-1]);
  for (i = sw-1; i < e_d_nbrpts - 1; i++)
  {
   e_d_sbrpts[i] = e_d_sbrpts[i+1];
   e_d_ybrpts[i] = e_d_ybrpts[i+1];
   e_d_nrbrpts[i] = e_d_nrbrpts[i+1];
  }
  e_d_nbrpts--;
  if (e_d_nbrpts == 0)
  {
   FREE(e_d_sbrpts);
   e_d_sbrpts = NULL;
   FREE(e_d_ybrpts);
   e_d_ybrpts = NULL;
   FREE(e_d_nrbrpts);
   e_d_nrbrpts = NULL;
  }
 }
 else
 {
  e_d_nbrpts++;
  if (e_d_nbrpts == 1)
  {
   e_d_sbrpts = MALLOC(sizeof(char *));
   e_d_ybrpts = MALLOC(sizeof(int));
   e_d_nrbrpts = MALLOC(sizeof(int));
  }
  else
  {
   e_d_sbrpts = REALLOC(e_d_sbrpts, e_d_nbrpts * sizeof(char *));
   e_d_ybrpts = REALLOC(e_d_ybrpts, e_d_nbrpts * sizeof(int));
   e_d_nrbrpts = REALLOC(e_d_nrbrpts, e_d_nbrpts * sizeof(int));
  }
  e_d_sbrpts[e_d_nbrpts - 1] = MALLOC(1);
  if (e_d_swtch)
  {
   if (e_deb_type == 0)
   {
    sprintf(eing, "b main\n");
    write(rfildes[1], eing, strlen(eing));
    while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
      strncmp(str, "Breakpoint", 10))
     ;
    if (ret == -1) return(ret);
    if (ret == 2) e_d_error(str);
    e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+11);
    if (ret != 1 && e_d_dum_read() == -1) return(-1);
   }
   else if (e_deb_type == 2)
   {
    sprintf(eing, "stop in main\n");
    write(rfildes[1], eing, strlen(eing));
    while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
      str[0] != '(')
     ;
    if (ret == -1) return(ret);
    if (ret == 2) e_d_error(str);
    e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+1);
    if (ret != 1 && e_d_dum_read() == -1) return(-1);
   }
   else if (e_deb_type == 3)
   {
    sprintf(eing, "b main\n");
    write(rfildes[1], eing, strlen(eing));
    while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
      strncmp(str, "Added:", 6))
     ;
    ret = e_d_line_read(wfildes[0], str, 256, 0, 0);
    if (ret == -1) return(ret);
    if (ret == 2) e_d_error(str);
    e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+1);
    if (ret != 1 && e_d_dum_read() == -1) return(-1);
   }
   else if (e_deb_type == 1)
   {
    sprintf(eing, "e main\n");
    write(rfildes[1], eing, strlen(eing));
    if (e_d_dum_read() == -1) return(-1);
    sprintf(eing, "b\n");
    write(rfildes[1], eing, strlen(eing));
    if ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == -1)
     return(ret);
    if (ret == 2) e_d_error(str);
    if (ret != 1)
    {
     for (i = 0; str[i] && str[i] != ':'; i++)
      ;
     if (str[i])
      e_d_ybrpts[e_d_nbrpts - 1] = atoi(str+i+1);
     if (e_d_dum_read() == -1) return(-1);
    }
   }
  }
 }
 return(sw ? 0 : e_d_nbrpts);
}

int e_make_breakpoint(FENSTER *f, int sw)
{
 ECNT *cn = f->ed;
 SCHIRM *s = cn->f[cn->mxedt]->s;
 BUFFER *b = cn->f[cn->mxedt]->b;
 int ret, i;
 char eing[128], str[256];

 if (!sw)
 {
  if (!e_check_c_file(f->datnam))
   return(e_error(e_p_msg[ERR_NO_CFILE], 0, f->fb));
  for(i = 0; i < s->brp[0] && s->brp[i+1] != b->b.y; i++)
   ;
  if (i < s->brp[0])
  {
   for (i++; i < s->brp[0]; i++) s->brp[i] = s->brp[i+1];
   (s->brp[0])--;
   for (i = 0; i < e_d_nbrpts && (strcmp(e_d_sbrpts[i], f->datnam) ||
     e_d_ybrpts[i] != b->b.y+1); i++)
    ;
   if (e_d_swtch)
   {
    if (e_deb_type == 0) sprintf(eing, "d %d\n", e_d_nrbrpts[i]);
    else if (e_deb_type == 2)
     sprintf(eing, "delete %d\n", e_d_nrbrpts[i]);
    else if (e_deb_type == 3)
     sprintf(eing, "db %d\n", e_d_nrbrpts[i]);
    else if (e_deb_type == 1)
    {
     sprintf(eing, "e %s\n", e_d_sbrpts[i]);
     write(rfildes[1], eing, strlen(eing));
     if (e_d_dum_read() == -1) return(-1);
     sprintf(eing, "%d d\n", e_d_ybrpts[i]);
    }
    write(rfildes[1], eing, strlen(eing));
    if (e_d_dum_read() == -1) return(-1);
   }
   FREE(e_d_sbrpts[i]);
   for (; i < e_d_nbrpts - 1; i++)
   {
    e_d_sbrpts[i] = e_d_sbrpts[i+1];
    e_d_ybrpts[i] = e_d_ybrpts[i+1];
    e_d_nrbrpts[i] = e_d_nrbrpts[i+1];
   }
   e_d_nbrpts--;
   if (e_d_nbrpts == 0)
   {
    FREE(e_d_sbrpts);
    e_d_sbrpts = NULL;
    FREE(e_d_ybrpts);
    e_d_ybrpts = NULL;
    FREE(e_d_nrbrpts);
    e_d_nrbrpts = NULL;
   }
  }
  else
  {
   e_d_nbrpts++;
   if (e_d_nbrpts == 1)
   {
    e_d_sbrpts = MALLOC(sizeof(char *));
    e_d_ybrpts = MALLOC(sizeof(int));
    e_d_nrbrpts = MALLOC(sizeof(int));
   }
   else
   {
    e_d_sbrpts = REALLOC(e_d_sbrpts, e_d_nbrpts * sizeof(char *));
    e_d_ybrpts = REALLOC(e_d_ybrpts, e_d_nbrpts * sizeof(int));
    e_d_nrbrpts = REALLOC(e_d_nrbrpts, e_d_nbrpts * sizeof(int));
   }
   e_d_sbrpts[e_d_nbrpts - 1] = MALLOC(strlen(f->datnam) + 1);
   strcpy(e_d_sbrpts[e_d_nbrpts - 1], f->datnam);
   e_d_ybrpts[e_d_nbrpts - 1] = b->b.y + 1;
   if (e_d_swtch)
   {
    if (e_deb_type == 0)
    {
     sprintf(eing, "b %s:%d\n", f->datnam, b->b.y + 1);
     write(rfildes[1], eing, strlen(eing));
     while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
       strncmp(str, "Breakpoint", 10))
      ;
     if (ret == -1) return(ret);
     if (ret == 2) e_d_error(str);
     e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+11);
     if (ret != 1 && e_d_dum_read() == -1) return(-1);
    }
    else if (e_deb_type == 2)
    {
     sprintf(eing, "stop at \"%s\":%d\n", f->datnam, b->b.y + 1);
     write(rfildes[1], eing, strlen(eing));
     while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
       str[0] != '(')
      ;
     if (ret == -1) return(ret);
     if (ret == 2) e_d_error(str);
     e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+1);
     if (ret != 1 && e_d_dum_read() == -1) return(-1);
    }
    else if (e_deb_type == 3)
    {
     sprintf(eing, "b %s:%d\n", f->datnam, b->b.y + 1);
     write(rfildes[1], eing, strlen(eing));
     while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
       strncmp(str, "Added:", 6))
      ;
     ret = e_d_line_read(wfildes[0], str, 256, 0, 0);
     if (ret == -1) return(ret);
     if (ret == 2) e_d_error(str);
     e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+1);
     if (ret != 1 && e_d_dum_read() == -1) return(-1);
    }
    else if (e_deb_type == 1)
    {
     sprintf(eing, "e %s\n", f->datnam);
     write(rfildes[1], eing, strlen(eing));
     if (e_d_dum_read() == -1) return(-1);
     sprintf(eing, "%d b\n", b->b.y + 1);
     write(rfildes[1], eing, strlen(eing));
     if (e_d_dum_read() == -1) return(-1);
    }
   }
   (s->brp[0])++;
   s->brp = REALLOC(s->brp, (s->brp[0]+1) * sizeof(int));
   s->brp[s->brp[0]] = b->b.y;
  }
 }
 else
 {
  if(e_deb_type == 0)
  {
   for (i = 0; i < e_d_nbrpts; i++)
   {
    sprintf(eing, "b %s:%d\n", e_d_sbrpts[i], e_d_ybrpts[i]);
    write(rfildes[1], eing, strlen(eing));
    while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
      strncmp(str, "Breakpoint", 10))
     ;
    if (ret == -1) return(ret);
    if (ret == 2) e_d_error(str);
    e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+11);
    if (ret != 1 && e_d_dum_read() == -1) return(-1);
   }
  }
  else if (e_deb_type == 2)
  {
   for (i = 0; i < e_d_nbrpts; i++)
   {
    sprintf(eing, "stop at \"%s\":%d\n", e_d_sbrpts[i], e_d_ybrpts[i]);
    write(rfildes[1], eing, strlen(eing));
    while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
      str[0] != '(')
     ;
    if (ret == -1) return(ret);
    if (ret == 2) e_d_error(str);
    e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+1);
    if (ret != 1 && e_d_dum_read() == -1) return(-1);
   }
  }
  else if (e_deb_type == 3)
  {
   for (i = 0; i < e_d_nbrpts; i++)
   {
    sprintf(eing, "b %s:%d\n", f->datnam, b->b.y + 1);
    write(rfildes[1], eing, strlen(eing));
    while ((ret = e_d_line_read(wfildes[0], str, 256, 0, 0)) == 0 &&
      strncmp(str, "Added:", 6))
     ;
    ret = e_d_line_read(wfildes[0], str, 256, 0, 0);
    if (ret == -1) return(ret);
    if (ret == 2) e_d_error(str);
    e_d_nrbrpts[e_d_nbrpts - 1] = atoi(str+1);
    if (ret != 1 && e_d_dum_read() == -1) return(-1);
   }
  }
  else
  {
   for (i = 0; i < e_d_nbrpts; i++)
   {
    sprintf(eing, "e %s\n", e_d_sbrpts[i]);
    write(rfildes[1], eing, strlen(eing));
    if (e_d_dum_read() == -1) return(-1);
    sprintf(eing, "%d b\n", e_d_ybrpts[i]);
    write(rfildes[1], eing, strlen(eing));
    if (e_d_dum_read() == -1) return(-1);
   }
  }
 }
 e_schirm(f, 1);
 return(1);
}

/*   start Debugger   */
int e_exec_deb(FENSTER *f, char *prog)
{
 int i;

 if (e_d_swtch)
  return(1);
 e_d_swtch = 1;
 fflush(stdout);
 if (WpeIsXwin())
 {
  for (i = 0; i < 5; i++)
  {
   if (npipe[i])
    FREE(npipe[i]);
   npipe[i] = MALLOC(128);
   sprintf(npipe[i], "%s/we_pipe%d", e_tmp_dir, i);
   remove(npipe[i]);
  }
  if (mkfifo(npipe[0], S_IRUSR | S_IWUSR) < 0 ||
    mkfifo(npipe[1], S_IRUSR | S_IWUSR) < 0 ||
    mkfifo(npipe[2], S_IRUSR | S_IWUSR) < 0 ||
    mkfifo(npipe[3], S_IRUSR | S_IWUSR) < 0 ||
    mkfifo(npipe[4], S_IRUSR | S_IWUSR) < 0)
  {
   e_error(e_d_msg[ERR_CANTPIPE], 0, f->fb);
   return(0);
  }
 }
 else
 {
  if (pipe(rfildes))
  {
   e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
   return(0);
  }
  if (pipe(wfildes))
  {
   e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
   return(0);
  }
  if (pipe(efildes))
  {
   e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
   return(0);
  }
 }

 if ((e_d_pid = fork()) > 0)
 {
  if (WpeIsXwin())
  {
   if ((wfildes[0] = open(npipe[1], O_RDONLY)) < 0)
   {
    e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
    return(0);
   }
   for (i = 0;
     i < 80 && read(wfildes[0], &e_d_tty[i], 1) == 1 && e_d_tty[i] != '\0' &&
       e_d_tty[i] != '\n';
     i++)
    ;
   if (e_d_tty[i] == '\n')
    e_d_tty[i] = '\0';
   if ((rfildes[0] = open(e_d_tty, O_RDONLY)) < 0 ||
     (wfildes[1] = open(e_d_tty, O_WRONLY)) < 0)
   {
    e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
    return(0);
   }
   if ((rfildes[1] = open(npipe[0], O_WRONLY)) < 0 ||
     (wfildes[0] = open(npipe[1], O_RDONLY)) < 0 ||
     (efildes[0] = open(npipe[2], O_RDONLY)) < 0)
   {
    e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
    return(0);
   }
   if (e_deb_mode)
   {
    tcgetattr(rfildes[0], &ntermio);
/*   ioctl(rfildes[0], TCGETA, &ntermio);*/
    ntermio.c_iflag = 0;
    ntermio.c_oflag = 0;
    ntermio.c_lflag = 0;
    ntermio.c_cc[VMIN] = 1;
    ntermio.c_cc[VTIME] = 0;
#ifdef VSWTCH
    ntermio.c_cc[VSWTCH] = 0;
#endif
/*   ioctl(rfildes[0], TCSETA, &ntermio);*/
    tcsetattr(rfildes[0], TCSADRAIN, &ntermio);
   }
  }
  else
  {
   FILE *fpp;

   if (!(fpp = popen("tty", "r")))
   {
    e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
    return(0);
   }
   fgets(e_d_tty, 80, fpp);
   pclose(fpp);
  }
  return(wfildes[1]);
 }
 else if (e_d_pid < 0)
 {
  e_error(e_p_msg[ERR_PROCESS], 0, f->fb);
  return(0);
 }

#ifndef NO_XWINDOWS
 if (WpeIsXwin())
 {
  FILE *fp;
  char file[128];

  sprintf(file, "%s/we_sys", e_tmp_dir);
  fp = fopen(file, "w+");
  fprintf(fp, "#!/bin/sh\n");
  fprintf(fp, "tty > %s\n", npipe[1]);
  if (!e_deb_swtch)
   fprintf(fp,
     "%s %s < %s > %s 2> %s\necho type \\<Return\\> to continue\nread i\n",
     e_debugger, prog, npipe[0], npipe[1], npipe[2]);
  else
   fprintf(fp,
     "%s %s %s < %s > %s 2> %s\necho type \\<Return\\> to continue\nread i\n",
     e_debugger, e_deb_swtch, prog, npipe[0], npipe[1], npipe[2]);
  fprintf(fp, "rm -f %s\n", file);
  fclose(fp);
  chmod(file, 0755);

  execlp(XTERM_CMD, XTERM_CMD, "+sb", "-geometry", "80x25-0-0", "-e",
    user_shell, "-c", file, NULL);
  remove(file);
 }
 else
#endif
 {
  int kbdflgs;

  close(0);
  if (fcntl(rfildes[0], F_DUPFD, 0) != 0)
  {
   fprintf(stderr, e_p_msg[ERR_PIPEEXEC], rfildes[0]);
   exit(1);
  }
  close(1);
  if (fcntl(wfildes[1], F_DUPFD, 1) != 1)
  {
   fprintf(stderr, e_p_msg[ERR_PIPEEXEC], wfildes[1]);
   exit(1);
  }
  close(2);
  if (fcntl(efildes[1], F_DUPFD, 2) != 2)
  {
   fprintf(stderr, e_p_msg[ERR_PIPEEXEC], efildes[1]);
   exit(1);
  }
  kbdflgs = fcntl(1, F_GETFL, 0 );
  fcntl(1, F_SETFL, kbdflgs | O_NONBLOCK);
  kbdflgs = fcntl(2, F_GETFL, 0 );
  fcntl( 2, F_SETFL, kbdflgs | O_NONBLOCK);
  if (!e_deb_swtch)
   execlp(e_debugger, e_debugger, prog, NULL);
  else
   execlp(e_debugger, e_debugger, e_deb_swtch, prog, NULL);
 }
 fprintf(stderr,"%s %s %s\n", e_p_msg[ERR_IN_COMMAND], e_debugger, prog);
 exit(1);
}

int e_start_debug(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i, file;
 char estr[128];

 efildes[0] = efildes[1] = -1;
 wfildes[0] = wfildes[1] = -1;
 rfildes[0] = rfildes[1] = -1;
 if (e_d_swtch)
  return(0);
/*    e_copy_prog(&e_sv_prog, &e_prog);  */
 if (e_p_make(f))
  return(-1);
 if (!e__project)
 {
  for (i = cn->mxedt; i > 0; i--)
   if (e_check_c_file(cn->f[i]->datnam))
    break;
  if (i > 0)
   strcpy(e_d_file, cn->f[i]->datnam);
 }
 else
  strcpy(e_d_file, cn->f[cn->mxedt-1]->datnam);
 f = cn->f[cn->mxedt-1];
 for (i = 0; i < SVLINES; i++)
 {  e_d_sp[i] = e_d_out_str[i];  e_d_out_str[i][0] = '\0';  }
 if (e_deb_type == 1) {  e_debugger = "sdb";  e_deb_swtch = NULL;  }
 else if (e_deb_type == 2) {  e_debugger = "dbx";  e_deb_swtch = "-i";  }
 else if (e_deb_type == 3) {  e_debugger = "xdb";  e_deb_swtch = "-L";  }
 else {  e_debugger = "gdb";  e_deb_swtch = NULL;  }
 e_d_pid = 0;
 if (e_test_command(e_debugger))
 {
  sprintf(estr, "Debugger \'%s\' not in Path", e_debugger);
  e_error(estr, 0, f->fb);
  return(-1);
 }
 e_sys_ini();
 if (e__project && (file = e_exec_deb(f, e_s_prog.exe_name )) == 0)
 {
  e_sys_end();
  return(-2);
 }
 else if (!e__project)
 {
  if (e_s_prog.exe_name && e_s_prog.exe_name[0])
  {
   strcpy(estr, e_s_prog.exe_name);
  }
  else
  {
   strcpy(estr, f->datnam);
   WpeStringCutChar(estr, '.');
   strcat(estr, ".e");
  }
  if ((file = e_exec_deb(f, estr)) == 0)
  {
   e_sys_end();
   return(-2);
  }
 }
 e_sys_end();
 e_d_p_message(e_d_msg[ERR_STARTDEBUG], f, 1);
 WpeMouseChangeShape(WpeDebuggingShape);
 if (cn->mxedt > 1)
  e_switch_window(cn->edt[cn->mxedt - 1], cn->f[cn->mxedt]);
 return(0);
}

int e_run_debug(FENSTER *f)
{
 ECNT *cn = f->ed;
 int kbdflgs, ret;

 if (e_d_swtch < 1 && (ret = e_start_debug(f)) < 0) return(ret);
 if (e_d_swtch < 2)
 {
  kbdflgs = fcntl(efildes[0], F_GETFL, 0);
  fcntl( efildes[0], F_SETFL, kbdflgs | O_NONBLOCK);
  kbdflgs = fcntl(wfildes[0], F_GETFL, 0);
  fcntl( wfildes[0], F_SETFL, kbdflgs | O_NONBLOCK);

  if (e_d_dum_read() == -1) return(-1);
  if (e_deb_type == 3)
  {
   write(rfildes[1], "sm\n", 3);
   if (e_d_dum_read() == -1) return(-1);
  }
  if (e_make_breakpoint(cn->f[cn->mxedt], 1) == -1) return(-1);
  e_d_swtch = 2;
 }
 return(0);
}

/*  Run  */
int e_deb_run(FENSTER *f)
{
 ECNT *cn = f->ed;
 char eing[256];
 int ret, len, prsw = 0;

 if (e_d_swtch < 2 && (ret = e_run_debug(f)) < 0)
 {
  e_d_quit(f);
  if (ret == -1) {  e_show_error(0, f);  return(ret);  }
  return(e_error(e_d_msg[ERR_CANTDEBUG], 0, f->fb));
 }
 for (ret = 0; isspace(e_d_tty[ret]); ret++)
  ;
 if (e_d_tty[ret] != DIRC)
 {
  e_d_quit(f);
  sprintf(eing, "tty error: %s", e_d_tty);
  return(e_d_error(eing));
 }
 if (e_deb_type == 2)
 {
  if (e_d_swtch < 3)
  {
   if (e_prog.arguments)
    sprintf(eing, "run %s > %s\n", e_prog.arguments, e_d_tty);
   else
    sprintf(eing, "run > %s\n", e_d_tty);
  }
  else
  {
   strcpy(eing, "cont\n");
   prsw = 1;
  }
 }
 else
 {
  if (e_d_swtch < 3)
  {
   if (e_prog.arguments)
    sprintf(eing, "r %s > %s\n", e_prog.arguments, e_d_tty);
   else
    sprintf(eing, "r > %s\n", e_d_tty);
  }
  else
  {
   strcpy(eing, "c\n");
   prsw = 1;
  }
 }
 f = cn->f[cn->mxedt];
 e_d_nstack = 0;
 e_d_delbreak(f);
 e_d_switch_out(1);
 write(rfildes[1], eing, strlen(eing));
 if (e_deb_type == 0 || ((e_deb_type == 2  || e_deb_type == 3) && !prsw))
 {
  while((ret = e_d_line_read(wfildes[0], eing, 256, 0, 0)) == 2 ||
    !eing[0] || (!e_deb_type && prsw && ((len = (strlen(eing)-12)) < 0 ||
    strcmp(eing + len, e_d_msg[ERR_CONTINUE]))) ||
    (e_d_swtch < 3 && ((!e_deb_type && strncmp(e_d_msg[ERR_STARTPROG],eing, 17)) ||
    (e_deb_type == 2 && strncmp("Running:",eing, 8)))))
  {
   if (ret == 2)
    e_d_error(eing);
   else if (ret < 0)
    return(e_d_quit(f));
  }
 }
 if (!prsw)
  e_d_p_message(eing, f, 0);
 if (e_d_swtch < 3 && ((!e_deb_type && strncmp(e_d_msg[ERR_STARTPROG],eing, 17)) ||
   (e_deb_type == 2 && strncmp("Running:",eing, 8))))
 {
  e_d_quit(f);
  return(e_error(e_d_msg[ERR_CANTPROG], 0, f->fb));
 }
 e_d_swtch = 3;
 return(e_read_output(f));
}

int e_deb_trace(FENSTER *f)
{
 return(e_d_step_next(f, 0));
}

int e_deb_next(FENSTER *f)
{
 return(e_d_step_next(f, 1));
}

int e_d_step_next(FENSTER *f, int sw)
{
 int ret, main_brk = 0;

 if (e_d_swtch < 2 && (ret = e_run_debug(f)) < 0)
 {
  e_d_quit(f);
  if (ret == -1)
  {  e_show_error(0, f);  return(ret);  }
  return(e_error(e_d_msg[ERR_CANTDEBUG], 0, f->fb));
 }
 if (e_d_swtch < 3)
 {
  if ((main_brk = e_mk_brk_main(f, 0)) < -1) return(main_brk);
  ret = e_deb_run(f);
  e_mk_brk_main(f, main_brk);
  return(ret);
 }
 e_d_delbreak(f);
 e_d_switch_out(1);
 if (sw && e_deb_type == 0) write(rfildes[1], "n\n", 2);
 else if (sw && (e_deb_type == 1 || e_deb_type == 3)) write(rfildes[1], "S\n", 2);
 else if (sw && e_deb_type == 2) write(rfildes[1], "next\n", 5);
 else if (e_deb_type == 2) write(rfildes[1], "step\n", 5);
 else write(rfildes[1], "s\n", 2);
 e_d_nstack = 0;
 return(e_read_output(f));
}

int e_d_goto_func(FENSTER *f, int flag)
{
 ECNT *cn = f->ed;
 BUFFER *b = cn->f[cn->mxedt]->b;
 int ret = 0, main_brk = 0;
 char str[128];

 if (e_deb_type!=0)/* if gdb */
  return 0;
 if (e_d_swtch < 2 && (ret = e_run_debug(f)) < 0)
 {
  e_d_quit(f);
  if (ret == -1)
  {  e_show_error(0, f);  return(ret);  }
  return(e_error(e_d_msg[ERR_CANTDEBUG], 0, f->fb));
 }
 if (e_d_swtch < 3)
 {
  if ((main_brk = e_mk_brk_main(f, 0)) < -1)
   return(main_brk);
  ret = e_deb_run(f);
  e_mk_brk_main(f, main_brk);
  return(ret);
 }
 e_d_delbreak(f);
 e_d_switch_out(1);
 switch(flag)
 {
  case 'U':
   sprintf(str,"until %d\n",b->b.y+1);
   break;
  case 'F':
   sprintf(str,"finish\n");
   break;
  default:
   *str=0;
   break;
 }
 e_d_nstack = 0;
 if (*str)
 {
  write(rfildes[1], str, strlen(str));
  ret=e_read_output(f);
  /* Executing Finish twice may not work properly. */
 }
 return ret;
}

int e_d_goto_cursor(FENSTER *f)
{
 return e_d_goto_func(f,'U');
}

int e_d_finish_func(FENSTER *f)
{
 return e_d_goto_func(f,'F');
}

int e_d_fst_check(FENSTER *f)
{
 int i, j, k = 0, l, ret = 0;

 e_d_switch_out(0);
 for (i = 0; i < SVLINES - 1; i++)
 {
  if ((e_deb_type != 2 && !strncmp(e_d_sp[i], e_d_msg[ERR_PROGEXIT], 14)) ||
    ((e_deb_type == 2 && !strncmp(e_d_sp[i], e_d_msg[ERR_PROGEXIT2], 14)) ||
    (e_deb_type == 3 && !strncmp(e_d_sp[i], e_d_msg[ERR_NORMALTERM], strlen(e_d_msg[ERR_NORMALTERM])))))
  {
   e_d_error(e_d_sp[i]);		/*  Program exited   */
   e_d_quit(f);
   return(i);
  }
  else if ((e_deb_type == 0 || e_deb_type == 1) && !strncmp(e_d_sp[i], e_d_msg[ERR_PROGTERM], 18))
  {
   e_error(e_d_msg[ERR_PROGTERM], 0, f->fb);
   e_d_quit(f);
   return(i);
  }
  else if (e_deb_type == 0 && !strncmp(e_d_sp[i], e_d_msg[ERR_PROGSIGNAL], 23))
  {
   e_d_pr_sig(e_d_sp[i], f);
   return(i);
  }
  else if (e_deb_type == 3 && !strncmp(e_d_sp[i], e_d_msg[ERR_SOFTTERM], strlen(e_d_msg[ERR_SOFTTERM])))
  {
   e_d_pr_sig(e_d_sp[i], f);
   return(i);
  }
  else if (e_deb_type == 2 && (!strncmp(e_d_sp[i], e_d_msg[ERR_SIGNAL], 6) ||
    !strncmp(e_d_sp[i], e_d_msg[ERR_INTERRUPT], 9)))
  {
   e_d_pr_sig(e_d_sp[i], f);
   return(i);
  }
  else if (e_deb_type == 1 && strstr(e_d_sp[i], e_d_msg[ERR_SIGNAL]))
  {
   e_d_pr_sig(e_d_sp[i], f);
   return(i);
  }
  else if (e_deb_type == 3 && i == SVLINES-2 && strstr(e_d_sp[i], ": "))
  {
   e_d_pr_sig(e_d_sp[i-1], f);
   return(i-1);
  }
  else if (!strncmp(e_d_sp[i], e_d_msg[ERR_BREAKPOINT], 10) ||
    (e_deb_type == 1 && !strncmp(e_d_sp[i], e_d_msg[ERR_BREAKPOINT2], 10)))
  {
   if (!e_deb_type)
   {
    for (j = SVLINES - 2; j > i; j--)      /*  Breakpoint   */
    {
     if ((ret = atoi(e_d_sp[j])) > 0)
      for(k = 0; e_d_sp[j][k] && isdigit(e_d_sp[j][k]); k++)
       ;
     if (e_d_sp[j][k] == '\t')
      break;
    }
    if (j > i)
    {
     for (k = strlen(e_d_sp[j-1]); k >= 0 && e_d_sp[j-1][k] != ':'; k--)
      ;
     if (k >= 0 && atoi(e_d_sp[j-1]+k+1) == ret)
     {
      if (e_make_line_num(e_d_sp[j-1], e_d_sp[SVLINES-1]) >= 0)
       strcpy(e_d_file, e_d_sp[SVLINES-1]);
     }
     if (e_d_p_watches(f, 0) == -1)
      return(-1);
     e_d_goto_break(e_d_file, ret, f);
     return(i > 0 ? i-1 : 0);
    }
   }
   else if (e_deb_type == 1)	      /*  Breakpoint   */
   {
    if(!strncmp(e_d_sp[i]+10, " at", 3) &&
      (ret = e_make_line_num(e_d_sp[i+1], e_d_sp[SVLINES-1])) >= 0)
    {
     strcpy(e_d_file, e_d_sp[SVLINES-1]);
     if (e_d_p_watches(f, 0) == -1)
      return(-1);
     e_d_goto_break(e_d_file, ret, f);
     return(i);
    }
   }
  }
  else if (e_deb_type == 2 && !strncmp(e_d_sp[i], e_d_msg[ERR_STOPPEDIN], 10))
  {
   for (j = i + 1; j < SVLINES - 1; j++)		      /*  Breakpoint   */
   {
    for (k = 0; e_d_sp[j][k] == ' ' && e_d_sp[j][k] != '\0'; k++)
     ;
    if ((ret = atoi(e_d_sp[j]+k)) > 0)
    {
     if (!strstr(e_d_sp[j-1], " line "))
      break;
     for (k = strlen(e_d_sp[j-1]); k >= 0 && e_d_sp[j-1][k] != '\"'; k--)
      ;
     for(k--; k >= 0 && e_d_sp[j-1][k] != '\"'; k--)
      ;
     if (k >= 0)
     {
      for(k++, l = 0; e_d_sp[j-1][k] != '\0' && e_d_sp[j-1][k] != '\"';
        k++, l++)
       e_d_file[l] = e_d_sp[j-1][k];
      e_d_file[l] = '\0';
     }
     if (e_d_p_watches(f, 0) == -1)
      return(-1);
     e_d_goto_break(e_d_file, ret, f);
     return(i);
    }
   }
  }
 }
 return(-2);
}

int e_d_snd_check(FENSTER *f)
{
 int i, j, k, ret;

 e_d_switch_out(0);
 for (i = SVLINES - 2; i >= 0; i--)
 {
  if (!e_deb_type && (ret = atoi(e_d_sp[i])) > 0)
  {
   for (k = 0; e_d_sp[i][k] && isdigit(e_d_sp[i][k]); k++)
    ;
   if (e_d_sp[i][k] != '\t')
    continue;
   if (i > 0)
   {
    for (k = strlen(e_d_sp[i-1]); k >= 0 && e_d_sp[i-1][k] != ':'; k--)
     ;
    if (k >= 0 && atoi(e_d_sp[i-1]+k+1) == ret)
    {
     i--;
     if (e_make_line_num(e_d_sp[i], e_d_sp[SVLINES-1]) >= 0)
      strcpy(e_d_file, e_d_sp[SVLINES-1]);
     do
     {
      for (; k >= 0 && e_d_sp[i][k] != ')'; k--)
       ;
      if (k < 0)
      {  i--;  k = strlen(e_d_sp[i]);  }
      else
       break;
     }  while (i >= 0);
     do
     {
      for (j = 1, k--; k >= 0 && j > 0; k--)
      {
       if (e_d_sp[i][k] == ')')
        j++;
       else if (e_d_sp[i][k] == '(')
        j--;
      }
      if (k < 0)
      {  i--;  k = strlen(e_d_sp[i]);  }
     } while (i >= 0 && j > 0);
     if (k == 0 && i > 0)
     {  i--;  k = strlen(e_d_sp[i]);  }
     for (k--; k >= 0 && isspace(e_d_sp[i][k]); k--)
      ;
     if (k < 0 && i > 0)
      i--;
    }
   }
   if (e_d_p_watches(f, 0) == -1)
    return(-1);
   e_d_goto_break(e_d_file, ret, f);
   return(i);
  }
  else if (e_deb_type == 1 && (ret = atoi(e_d_sp[i])) > 0)
  {
   for (; i > 0; i--)
   {
    for (j = strlen(e_d_sp[i-1])-1; j >= 0 && isspace(e_d_sp[i-1][j]); j--)
     ;
    if (j < 0)
    {  i--;  continue;  }
    for (j--; j >= 0 && !isspace(e_d_sp[i-1][j]); j--)
     ;
    if (j < 0)
     i--;
    for (j--; j >= 0 && isspace(e_d_sp[i-1][j]); j--)
     ;
    if (j < 0)
     i--;
    for (j--; j >= 0 && !isspace(e_d_sp[i-1][j]); j--)
     ;
    if (!strncmp(e_d_sp[i-1]+j+1, "in ", 3))
    {
     strcpy(e_d_file, e_d_sp[i-1]+j+4);
     for (k = i+2; !e_d_file[0]; k++)
      strcpy(e_d_file, e_d_sp[i-1]+j+4);
     for (k = strlen(e_d_file)-1; k >= 0 && isspace(e_d_file[k]); k--)
      ;
     e_d_file[k+1] = '\0';
     if (e_d_p_watches(f, 0) == -1)
      return(-1);
     e_d_goto_break(e_d_file, ret, f);
     return(i);
    }
   }
   return(-2);
  }
  else if (e_deb_type == 3 && i == SVLINES-2 &&
    (ret = e_make_line_num(e_d_sp[i], e_d_sp[SVLINES-1])) >= 0)
  {
   strcpy(e_d_file, e_d_sp[SVLINES-1]);
   if (e_d_p_watches(f, 0) == -1)
    return(-1);
   e_d_goto_break(e_d_file, ret, f);
   return(i);
  }
 }
 return(-2);
}

int e_d_trd_check(FENSTER *f)
{
 int ret;
 char str[256];

 str[0] = '\0';
 e_d_switch_out(0);
 if ((ret = e_d_pr_sig(str, f)) == -1) return(-1);
 else if (ret == -2) e_d_error(e_d_msg[ERR_NOSOURCE]);
 return(0);
}

int e_read_output(FENSTER *f)
{
 char *spt;
 int i, ret;

 for (i = 0; i < SVLINES; i++)
 {  e_d_sp[i] = e_d_out_str[i];  e_d_out_str[i][0] = '\0';  }
 while ((ret = e_d_line_read(wfildes[0], e_d_sp[SVLINES-1], 256, 0, 0)) == 2)
  e_d_error(e_d_sp[SVLINES-1]);
 if (ret < 0) return(-1);
 e_d_switch_out(0);
 while(ret != 1)
 {
  spt = e_d_sp[0];
  for (i = 1; i < SVLINES; i++)
   e_d_sp[i-1] = e_d_sp[i];
  e_d_sp[SVLINES-1] = spt;
  do
  {
   while((ret = e_d_line_read(wfildes[0], e_d_sp[SVLINES-1], 256, 0, 0)) == 2)
    e_d_error(e_d_sp[SVLINES-1]);
   if (ret < 0) return(-1);
  } while(!ret && !*e_d_sp[SVLINES-1]);
 }
 if ((i = e_d_fst_check(f)) == -1) return(-1);
 if (i < 0 && (i = e_d_snd_check(f)) == -1) return(-1);
 if (i < 0 && (i = e_d_trd_check(f)) == -1) return(-1);
 if (i < 0)
 {
  e_d_switch_out(0);
  i = e_message(1, e_d_msg[ERR_UNKNOWNBRK], f);
  if (i == 'Y') return(e_d_quit(f));
 }
 return(0);
}

int e_d_pr_sig(char *str, FENSTER *f)
{
 int i, line = -1, ret = 0;
 char file[128], str2[256];

 if (str && str[0])
  e_d_error(str);
 for (i = 0; i < SVLINES; i++)
  e_d_out_str[i][0] = '\0';
 if (e_deb_type != 1 && e_deb_type != 3)
 {
  write(rfildes[1], "where\n", 6);
  for (i = 0; ((ret = e_d_line_read(wfildes[0], str, 256, 0, 1)) == 0 &&
    (line = e_make_line_num(str, file)) < 0) || ret == 2; i++)
  {
   if (!strncmp(str, e_d_msg[ERR_NOSTACK], 9))
   {
    e_error(e_d_msg[ERR_PROGEXIT], 0, f->fb);
    while (ret == 0 || ret == 2)
     if ((ret = e_d_line_read(wfildes[0], str2, 256, 0, 0)) == 2)
      e_d_error(str2);
    e_d_quit(f);
    return(0);
   }
   else if (ret == 2)
    e_d_error(str);
  }
 }
 else
 {
  write(rfildes[1], "t\n", 2);
  for (i = 0; ((ret = e_d_line_read(wfildes[0], str, 256, 0, 1)) == 0 &&
    (line = e_make_line_num2(str, file)) < 0) || ret == 2; i++)
  {
   if (!strncmp(str, e_d_msg[ERR_NOPROCESS], 10))
   {
    e_error(e_d_msg[ERR_PROGEXIT], 0, f->fb);
    while (ret == 0 || ret == 2)
     if ((ret = e_d_line_read(wfildes[0], str2, 256, 0, 0)) == 2)
      e_d_error(str2);
    e_d_quit(f);
    return(0);
   }
   else if (ret == 2)
    e_d_error(str);
  }
 }
 if (ret == 1 && i == 0)
 {
  e_d_error(e_d_msg[ERR_PROGEXIT]);
  return(e_d_quit(f));
 }
 while (ret == 0 || ret == 2)
  if ((ret = e_d_line_read(wfildes[0], str2, 256, 0, 0)) == 2)
   e_d_error(str2);
 if (ret == -1)
  return(ret);
 if (line >= 0)
 {
  strcpy(e_d_file, file);
  e_d_goto_break(file, line, f);
  return(0);
 }
 else
  return(-2);
}

int e_make_line_num(char *str, char *file)
{
 char *sp;
 int i, n, num;

 if (e_deb_type == 0)
 {
  for (n = strlen(str); n >= 0 && str[n] != ':'; n--)
   ;
  if (n < 0)
   return(-1);
  for (i = n-1; i >= 0 && !isspace(str[i]); i--)
   ;
  for (n = i+1; str[n] != ':'; n++)
   file[n-i-1] = str[n];
  file[n-i-1] = '\0';
  return(atoi(str+n+1));
 }
 else if (e_deb_type == 2)
 {
  if (!(sp = strstr(str, " line ")))
   return(-1);
  if (!(num = atoi(sp+6)))
   return(-1);
  for (i = 6;  sp[i] != '\"'; i++)
   ;
  sp += (i+1);
  for (i = 0; (file[i] = sp[i]) != '\"' && file[i] != '\0'; i++)
   ;
  if (file[i] == '\0')
   return(-1);
  file[i] = '\0';
  return(num);
 }
 else if (e_deb_type == 3)
 {
  for (sp = str, i = 0; (file[i] = sp[i]) && sp[i] != ':'; i++)
   ;
  if (!sp[i])
   return(-1);
  file[i] = '\0';
  for (i++; sp[i] && sp[i] != ':'; i++)
   ;
  if (!sp[i])
   return(-1);
  for (i++; sp[i] && isspace(sp[i]); i++)
   ;
  if (!isdigit(sp[i]))
   return(-1);
  sp += i;
  return(atoi(sp));
 }
 else
 {
  for (i = 0; str[i] != '\0' && str[i] != ':'; i++)
   ;
  if ((!str[i]) || (num = atoi(str+i+1)) < 0)
   return(-1);
  write(rfildes[1], "e\n", 2);
  while ((i = e_d_line_read(wfildes[0], str, 256, 0, 0)) ==  2)
   e_d_error(str);
  if (i < 0)
   return(-1);
  for (i = 0; str[i] != '\0' && str[i] != '\"'; i++)
   ;
  for (sp = str + i + 1, i = 0; (file[i] = sp[i]) && file[i] != '\"'; i++)
   ;
  file[i] = '\0';
  if (e_d_dum_read() == -1)
   return(-1);
  return(num);
 }
}

int e_make_line_num2(char *str, char *file)
{
 char *sp;
 int i;

 for (i = 0; str[i] != '[' && str[i] != '\0'; i++)
  ;
 if (!str[i]) return(-1);
 for (sp = str+i+1, i = 0; (file[i] = sp[i]) != ':' && file[i] != '\0'; i++)
  ;
 if (file[i] == '\0') return(-1);
 file[i] = '\0';
 for (i++; isspace(sp[i]); i++)
  ;
 return(atoi(sp+i));
}

int e_d_goto_break(char *file, int line, FENSTER *f)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 SCHIRM *s;
 FENSTER ftmp;
 int i;
 char str[120];

/*   if(schirm != e_d_save_schirm) e_d_switch_out(0);  */
 e_d_switch_out(0);
 ftmp.ed = cn;
 ftmp.fb = f->fb;
 WpeFilenameToPathFile(file, &ftmp.dirct, &ftmp.datnam);
 for (i = 0; i < SVLINES; i++)
  e_d_out_str[i][0] = '\0';
 for (i = cn->mxedt; i > 0; i--)
  if (!strcmp(cn->f[i]->datnam, ftmp.datnam) &&
    !strcmp(cn->f[i]->dirct, ftmp.dirct))
  {
   /*  for(j = 0; j <= cn->mxedt; j++)
	 if(!strcmp(cn->f[j]->datnam, "Stack"))
	 {  if(cn->f[i]->e.x > 2*MAXSCOL/3-1) cn->f[i]->e.x = 2*MAXSCOL/3-1;
	 break;
	 }  */
   e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
   break;
  }
 f = cn->f[cn->mxedt];
 FREE(ftmp.dirct);
 FREE(ftmp.datnam);
 if (i <= 0)
 {
  if (access(file, 0))
  {
   sprintf(str, e_d_msg[ERR_CANTFILE], file);
   return(e_error(str, 0, f->fb));
  }
  if (e_edit(cn, file))
   return(WPE_ESC);
  b = cn->f[cn->mxedt]->b;
  s = cn->f[cn->mxedt]->s;
 }
 f = cn->f[cn->mxedt];
 b = cn->f[cn->mxedt]->b;
 s = cn->f[cn->mxedt]->s;
 s->da.y = b->b.y = line - 1;
 s->da.x = b->b.x = 0;
 s->de.x = MAXSCOL;
 e_schirm(f, 1);
 e_cursor(f, 1);
 return(0);
}

int e_d_delbreak(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 for (i = cn->mxedt; i >= 0; i--)
  if (DTMD_ISTEXT(cn->f[i]->dtmd))
   cn->f[i]->s->da.y = -1;
 e_rep_win_tree(cn);
 e_refresh();
 return(0);
}

int e_d_error(char *s)
{
 int len;

 e_d_switch_out(0);
 if (s[(len = strlen(s) - 1)] == '\n')
  s[len] = '\0';
 return(e_error(s, 0, WpeEditor->fb));
}

int e_d_putchar(int c)
{
 if (!WpeIsXwin()) c = fk_putchar(c);
 else
 {
  char cc = c;
  c = write(wfildes[1], &cc, 1);
 }
 return(c);
}

int e_deb_options(FENSTER *f)
{
 int ret;
 W_OPTSTR *o = e_init_opt_kst(f);

 if (!o) return(-1);
 o->xa = 20;  o->ya = 4;  o->xe = 60;  o->ye = 13;
 o->bgsw = 0;
 o->name = "Debug-Options";
 o->crsw = AltO;
 e_add_txtstr(4, 2, "Debugger:", o);
 e_add_txtstr(20, 2, "Mode:", o);
 e_add_pswstr(0, 5, 3, 0, AltG, 0, "Gdb    ", o);
 e_add_pswstr(0, 5, 4, 0, AltS, 0, "Sdb    ", o);
#ifdef XDB
 e_add_pswstr(0, 5, 5, 0, AltX, e_deb_type == 3 ? 2 : e_deb_type, "Xdb    ", o);
#else
 e_add_pswstr(0, 5, 5, 0, AltD, e_deb_type, "Dbx    ", o);
#endif
 e_add_pswstr(1, 21, 3, 0, AltN, 0, "Normal     ", o);
 e_add_pswstr(1, 21, 4, 0, AltF, e_deb_mode, "Full Screen", o);
 e_add_bttstr(10, 7, 1, AltO, " Ok ", NULL, o);
 e_add_bttstr(25, 7, -1, WPE_ESC, "Cancel", NULL, o);
 ret = e_opt_kst(o);
 if (ret != WPE_ESC)
 {
#ifdef XDB
  e_deb_type = o->pstr[0]->num == 2 ? 3 : o->pstr[0]->num;
#else
  e_deb_type = o->pstr[0]->num;
#endif
  e_deb_mode = o->pstr[1]->num;
 }
 freeostr(o);
 return(0);
}

int e_g_sys_ini()
{
 if (!e_d_swtch || e_deb_mode)
  return(0);
 tcgetattr(0, &ttermio);
 return(tcsetattr(0, TCSADRAIN, &otermio));
}

int e_g_sys_end()
{
 if (!e_d_swtch || e_deb_mode)
  return(0);
 return(tcsetattr(0, TCSADRAIN, &ttermio));
}

int e_test_command(char *str)
{
 int i = -1, k;
 char tmp[256], *path = getenv("PATH");

 if (!path) return(-2);
 do
 {
  for(i++, k = 0; (tmp[k] = path[i]) && path[i] != ':'; k++, i++)
   ;
  if (k == 0)
  {  tmp[0] = '.';  k++;  }
  tmp[k] = '/';
  tmp[k+1] = '\0';
  strcat(tmp, str);
  if (!access(tmp, X_OK)) return(0);
 } while(path[i]);
 return(-1);
}
#endif

