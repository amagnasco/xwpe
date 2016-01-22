/* we_prog.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"
#include "WeExpArr.h"

#ifdef PROG

#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>

int e_run_sh(FENSTER *f);
int e_make_library(char *library, char *ofile, FENSTER *f);
int e_p_exec(int file, FENSTER *f, PIC *pic);
struct dirfile **e_make_prj_opt(FENSTER *f);

int wfildes[2], efildes[2];
char *wfile = NULL, *efile = NULL;

struct e_s_prog e_s_prog = {  NULL, NULL, NULL, NULL, NULL, NULL, 0, NULL, 0, 0};

struct e_prog e_prog = {  0, NULL, NULL, NULL, NULL, NULL  };

struct ERR_LI  {  char *file, *text, *srch;  int x, y, line;  } *err_li = NULL;
int err_no, err_num;

int e__project = 0, e_argc, e_p_l_comp;
char **e_arg = NULL;
char *e__arguments = NULL;
M_TIME last_time;
char library[80];
int e_save_pid;
struct dirfile **e_p_df;

extern BUFFER *e_p_m_buffer;
extern char *e_tmp_dir;
#ifdef DEBUGGER
extern int e_d_swtch;
#endif

/********************************************************/   
/**** defs for breakpoints and watches manipulations ****/
/*** breakpoints ***/
extern int e_d_nbrpts;
extern char ** e_d_sbrpts;
extern int * e_d_ybrpts;

/*** watches ***/
extern int e_d_nwtchs;
extern char ** e_d_swtchs;
extern int * e_d_nrwtchs;

/********************************************************/   

char *gnu_intstr = "${?*:warning:}${FILE}:${LINE}:* before `${COLUMN=BEFORE}\'*";
char *cc_intstr = "${?*:warning:}\"${FILE}\", line ${LINE}:* at or near * \"${COLUMN=AFTER}\"";
char *pc_intstr = "${?0:e}${?0:w}${?0:s}*:*:* * ${FILE}:\n\n* ${LINE}  ${CMPTEXT}\n*-------\
${COLUMN=PREVIOUS?^+14}\n[EWew] * line*";


char *e_p_msg[] = {
 "Not a C - File",
 "Can\'t open Pipe",
 "Error in Process",
 "Error in Pipe\n Pipe Nr.: %d\n",
 "Error at Command: ",
 "Return-Code: %d",      /*   Number 5   */
 "%s is not a C - File",
 "No Compiler specified",
 "No Files to compile",
 "No Project-Window",
};


int e_prog_switch(FENSTER *f, int c)
{
 switch(c)
 {
  case AltU:
  case CF9:
   e_run(f);
   break;
  case AltM:   /*  Alt M  Make */
  case F9:
   e_p_make(f);
   break;
  case AltC:   /*  Alt C  Compile */
  case AF9:
   e_compile(f);
   break;
  case AltL:   /*  Alt L  InstaLl */
   e_install(f);
   break;
  case AltA:   /*  Alt A  Execute MAke */
   e_exec_make(f);
   break;
  case AltT:   /*  Alt T  NexT Error */
  case AF8:
   e_next_error(f);
   break;
  case AltV:   /*  Alt V  PreVious Error  */
  case AF7:
   e_previous_error(f);
   break;
#ifdef DEBUGGER
  case CtrlG:   /*  Ctrl G DebuG - Modus */
   e_deb_inp(f);
   break;
  default:
   return(e_debug_switch(f, c));
#else
  default:
   return(c);
#endif
 }
 return(0);
}

int e_compile(FENSTER *f)
{
 int ret;

 WpeMouseChangeShape(WpeWorkingShape);
 efildes[0] = efildes[1] = -1;
 wfildes[0] = wfildes[1] = -1;
 ret = e_comp(f);
 WpeMouseRestoreShape();
 return(ret);
}

int e_p_make(FENSTER *f)
{
 ECNT *cn = f->ed;
 char ostr[128], estr[128], mstr[80];
 int len, i, file = -1;
 struct stat cbuf[1], obuf[1];
 PIC *pic = NULL;
 int linkRequest = 1; /* assume linking has to be done */

 WpeMouseChangeShape(WpeWorkingShape);
 efildes[0] = efildes[1] = -1;
 wfildes[0] = wfildes[1] = -1;
 if (e_comp(f))
 {
  WpeMouseRestoreShape();
  return(-1);
 }
 f = cn->f[cn->mxedt-1];
 if (!e__project)
 {
  e_arg = MALLOC(6 * sizeof(char *));
  e_argc = e_make_arg(&e_arg, e_s_prog.libraries);
  e_arg[1] = MALLOC(3);
  strcpy(e_arg[1], "-o");
  strcpy(mstr, f->datnam);
  WpeStringCutChar(mstr, '.');
  len = strlen(e_prog.exedir) - 1;
  if (e_s_prog.exe_name && e_s_prog.exe_name[0])
  {
   if (e_prog.exedir[len] == DIRC)
    sprintf(estr, "%s%s", e_prog.exedir, e_s_prog.exe_name);
   else
    sprintf(estr, "%s%c%s", e_prog.exedir, DIRC, e_s_prog.exe_name);
  }
  else
  {
   if (e_prog.exedir[len] == DIRC)
    sprintf(estr, "%s%s.e", e_prog.exedir, mstr);
   else
    sprintf(estr, "%s%c%s.e", e_prog.exedir, DIRC, mstr);
  }
  if (e_prog.exedir[len] == DIRC)
   sprintf(ostr, "%s%s.o", e_prog.exedir, mstr);
  else
   sprintf(ostr, "%s%c%s.o", e_prog.exedir, DIRC, mstr);
  e_argc = e_add_arg(&e_arg, estr, 2, e_argc);
  e_argc = e_add_arg(&e_arg, ostr, 3, e_argc);
  stat(ostr, cbuf);
  if (!stat(estr, obuf) && obuf->st_mtime >= cbuf->st_mtime)
   linkRequest = 0;
 }
 else
 {
  if (!stat(e_s_prog.exe_name, obuf) && !e_p_l_comp &&
    obuf->st_mtime >= last_time)
   linkRequest = 0;
 }
 if (linkRequest)
 {
#ifdef DEBUGGER
  if (e_d_swtch > 0)
   e_d_quit(f);
#endif
  if (!e_p_mess_win("Linking", e_argc, e_arg, &pic, f))
  {
   e_sys_ini();
   file = e_exec_inf(f, e_arg, e_argc);
   e_sys_end();
  }
  else
   file = 0;
 }
 if (!e__project)
 {
  e_free_arg(e_arg, e_argc);
 }
 if (file != 0)
  i = e_p_exec(file, f, pic);
 else
 {
  i = WPE_ESC;
  if (pic)
   e_close_view(pic, 1);
 }
 WpeMouseRestoreShape();
 return(i);
}

int e_run(FENSTER *f)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 SCHIRM *s;
 char estr[256];
 int len, ret;

 efildes[0] = efildes[1] = -1;
 wfildes[0] = wfildes[1] = -1;
 if (!e_run_sh(f))
  return(0);
 if (e_p_make(f))
  return(-1);
 WpeMouseChangeShape(WpeWorkingShape);
 f = cn->f[cn->mxedt-1];
#ifdef DEBUGGER
 if (e_d_swtch > 0)
  e_d_quit(f);
#endif
 estr[0] = '\0';
 if ((!e_s_prog.exe_name) || (e_s_prog.exe_name[0]!=DIRC))
 {
  strcat(estr, e_prog.exedir);
  len = strlen(estr) - 1;
  if (estr[len] != DIRC)
  {
   estr[++len] = DIRC;
   estr[++len] = '\0';
  }
 }
 if (e_s_prog.exe_name && e_s_prog.exe_name[0])
 {
  strcat(estr, e_s_prog.exe_name);
 }
 else if (!e__project)
 {
  /* Default executable name of the source file - extension + ".e" */
  strcat(estr, f->datnam);
  WpeStringCutChar(estr, '.');
  strcat(estr, ".e");
 }
 else /* Default project executable name of "a.out" */
  strcat(estr, "a.out");
 strcat(estr, " ");
 if (e_prog.arguments)
  strcat(estr, e_prog.arguments);
#ifndef NO_XWINDOWS
 if (WpeIsXwin())
  ret = (*e_u_system)(estr);
 else
#endif
  ret = e_system(estr, cn);
 f = cn->f[cn->mxedt];
 b = cn->f[cn->mxedt]->b;
 s = cn->f[cn->mxedt]->s;

 sprintf(estr, e_p_msg[ERR_RETCODE], ret);
 print_to_end_of_buffer(b, estr, b->mx.x);

 b->b.y = b->mxlines-1;
 e_cursor(f, 1);
 e_schirm(f, 1);
 e_refresh();
 WpeMouseRestoreShape();
 return(0);
}

int e_comp(FENSTER *f)
{
 ECNT *cn = f->ed;
 PIC *pic = NULL;
 char **arg = NULL, fstr[128], ostr[128];
 int i, file = -1, len, argc;
#ifdef CHECKHEADER
 struct stat obuf[1];
#else
 struct stat cbuf[1], obuf[1];
#endif

#ifdef DEBUGGER
 if (e_d_swtch > 0)
 {
  i = e_message(1, "The Debugger is Running\nDo You want to Quit Debugging ?",f);
  if (i == 'Y')
   e_d_quit(f);
  else
   return(-1);
  WpeMouseChangeShape(WpeWorkingShape);
 }
#endif
 if (e_prog.project[0] && !access(e_prog.project, 0))
  e__project = 1;
 else
  e__project = 0;
 if (e__project)
  return(e_c_project(f));
 for (i = cn->mxedt; i > 0; i--)
 {
  if (e_check_c_file(cn->f[i]->datnam))
   break;
 }
 if (i == 0)
 {
  sprintf(ostr, e_p_msg[ERR_S_NO_CFILE], f->datnam);
  e_error(ostr, 0, f->fb);
  return(WPE_ESC);
 }
 else if (cn->f[i]->save)
  e_save(cn->f[i]);
 f = cn->f[i];
 e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
 if (e_new_message(f))
  return(WPE_ESC);
 argc = e_make_arg(&arg, e_s_prog.comp_str);
 arg[1] = MALLOC(3);
 strcpy(arg[1], "-c");
 len = strlen(f->dirct) - 1;
 if (!strcmp(f->ed->dirct, f->dirct))
  strcpy(fstr, f->datnam);
 if (f->dirct[len] == DIRC)
  sprintf(fstr, "%s%s", f->dirct, f->datnam);
 else
  sprintf(fstr, "%s%c%s", f->dirct, DIRC, f->datnam);
 argc = e_add_arg(&arg, fstr, argc, argc);
 if (e_prog.exedir[strlen(e_prog.exedir)-1] == DIRC)
  sprintf(ostr, "%s%s", e_prog.exedir, f->datnam);
 else
  sprintf(ostr, "%s%c%s", e_prog.exedir, DIRC, f->datnam);
 WpeStringCutChar(ostr, '.');
 strcat(ostr, ".o");
#ifndef NO_MINUS_C_MINUS_O
 argc = e_add_arg(&arg, "-o", argc, argc);
 argc = e_add_arg(&arg, ostr, argc, argc);
#endif
 e_sys_ini();
#ifdef CHECKHEADER
 if ((stat(ostr, obuf) || e_check_header(fstr, obuf->st_mtime, cn, 0)))
#else
 stat(f->datnam, cbuf);
 if ((stat(ostr, obuf) || obuf->st_mtime < cbuf->st_mtime))
#endif
 {
  remove(ostr);
  if (!e_p_mess_win("Compiling", argc, arg, &pic, f) &&
    (file = e_exec_inf(f, arg, argc)) == 0)
  {
   e_sys_end();
   e_free_arg(arg, argc);
   if (pic)
    e_close_view(pic, 1);
   return(WPE_ESC);
  }
 }
 e_sys_end();
 e_free_arg(arg, argc);
 i = e_p_exec(file, f, pic);
 return(i);
}

int e_exec_inf(FENSTER *f, char **argv, int n)
{
 int pid;
 char tstr[128];
#ifdef DEBUGGER
 if (e_d_swtch > 0)
  e_d_quit(f);
#endif
 fflush(stdout);
 sprintf(tstr, "%s/we_111", e_tmp_dir);
 if((efildes[1] = creat(tstr, 0777)) < 0)
 {
  e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
  return(0);
 }
 if((efildes[0] = open(tstr, O_RDONLY)) < 0 )
 {
  e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
  return(0);
 }
 efile = MALLOC((strlen(tstr)+1)*sizeof(char));
 strcpy(efile, tstr);
 sprintf(tstr, "%s/we_112", e_tmp_dir);
 if((wfildes[1] = creat(tstr, 0777)) < 0)
 {
  e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
  return(0);
 }
 if((wfildes[0] = open(tstr, O_RDONLY)) < 0 )
 {
  e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb);
  return(0);
 }
 wfile = MALLOC((strlen(tstr)+1)*sizeof(char));
 strcpy(wfile, tstr);

 if((e_save_pid = pid = fork()) > 0)
  return(efildes[1]);
 else if(pid < 0)
 {
  e_error(e_p_msg[ERR_PROCESS], 0, f->fb);
  return(0);
 }

 close(2);   /*  new process   */
 if(fcntl(efildes[1], F_DUPFD, 2) != 2)
 {
  fprintf(stderr, e_p_msg[ERR_PIPEEXEC], efildes[1]);
  exit(1);
 }
 close(1);
 if(fcntl(wfildes[1], F_DUPFD, 1) != 1)
 {
  fprintf(stderr, e_p_msg[ERR_PIPEEXEC], wfildes[1]);
  exit(1);
 }
 e_print_arg(stderr, "", argv, n);
 execvp(argv[0], argv);
 e_print_arg(stderr, e_p_msg[ERR_IN_COMMAND], argv, n);
 exit(1);
 /* Can never get here */
 return 0;
}

int e_print_arg(FILE *fp, char *s, char **argv, int n)
{
 int i;

 if ((s) && (s[0]))
  fprintf(fp,"%s ", s);
 for (i = 0; i < n && argv[i] != NULL; i++)
  fprintf(fp,"%s ", argv[i]);
 fprintf(fp,"\n");
 return(n);
}

int e_p_exec(int file, FENSTER *f, PIC *pic)
{
 ECNT *cn = f->ed;
 BUFFER *b = cn->f[cn->mxedt]->b;
 int ret = 0, i = 0, is, fd, stat_loc;
 char str[128];
 char *buff;

 f = cn->f[cn->mxedt];
 while ((ret = wait(&stat_loc)) >= 0 && ret != e_save_pid)
  ;
 ret = 0;
 for (is = b->mxlines-1, fd = efildes[0]; fd > 0; fd = wfildes[0])
 {
  buff=MALLOC(1);
  buff[0]='\0';
  while( e_line_read(fd, str, 128) == 0 )
  {
   buff=REALLOC(buff, strlen(buff) + strlen(str) + 1);
   strcat(buff, str);

   fflush(stdout);
  }
  print_to_end_of_buffer(b, buff, b->mx.x);
  FREE(buff);

  if( fd == wfildes[0] )
    break;
 }
 b->b.y = b->mxlines-1;
 if (efildes[0] >= 0)
  close(efildes[0]);
 if (wfildes[0] >= 0)
  close(wfildes[0]);
 if (efildes[1] >= 0)
  close(efildes[0]);
 if (wfildes[1] >= 0)
  close(wfildes[0]);
 if (wfile)
 {
  remove(wfile);
  FREE(wfile);
  wfile = NULL;
 }
 if (efile)
 {
  remove(efile);
  FREE(efile);
  efile = NULL;
 }
 efildes[0] = efildes[1] = -1;
 wfildes[0] = wfildes[1] = -1;
 if (pic)
  e_close_view(pic, 1);
 if (ret || (b->mxlines - is > 2 && (i = e_make_error_list(f))))
 {
  if (i != -2 && !ret)
   e_show_error(err_no = 0, f);
  return(-1);
 }

 print_to_end_of_buffer(b, "Success", b->mx.x);

 e_cursor(f, 1);
 e_schirm(f, 1);
 e_refresh();
 return(0);
}

/* show source-position of error number "n" from actual errorlist */
int e_show_error(int n, FENSTER *f)
{
 ECNT *cn = f->ed;
 BUFFER *b = cn->f[cn->mxedt]->b;
 int i, j, bg = 0;
 char *filename;
 unsigned char *cp;

 if (!err_li || n >= err_num || n < 0)
  return(1);
 f = cn->f[cn->mxedt];
 if (err_li[n].file[0] == '.' && err_li[n].file[1] == DIRC)
  bg = 2;
 if (err_li[n].file[0] == DIRC)
 {
  filename = e_mkfilename(f->dirct, f->datnam);
 }
 else
  filename = f->datnam;
 if (strcmp(err_li[n].file+bg, filename))
 {
  for (i = cn->mxedt - 1; i > 0; i--)
  {
   if (filename != cn->f[i+1]->datnam)
   {
    FREE(filename);
    filename = e_mkfilename(cn->f[i]->dirct, cn->f[i]->datnam);
   }
   else
    filename = cn->f[i]->datnam;
   if (!strcmp(err_li[n].file+bg, filename))
   {
    if (filename != cn->f[i]->datnam)
     FREE(filename);
    e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
    break;  
   }
  }
  if (i <= 0)
  {
   if (filename != cn->f[i+1]->datnam)
    FREE(filename); 
   if (e_edit(cn, err_li[n].file))
    return(WPE_ESC);
  }
 }
 else if (filename != f->datnam)
  FREE(filename);
 e_pr_str_wsd(1, MAXSLNS - 1, err_li[n].text, f->fb->mt.fb, -1, 0,
   f->fb->mt.fb, 1, MAXSCOL-2);
/*   e_pr_nstr(2, MAXSLNS - 1, MAXSCOL-2, err_li[n].text,
                                                f->fb->mt.fb, f->fb->mt.fb); */
 b = cn->f[cn->mxedt]->b;
 b->b.y = err_li[n].line > b->mxlines ? b->mxlines - 1 : err_li[n].line - 1;
 if (!err_li[n].srch)
 {
  for(i = j = 0; i + j < err_li[n].x && i < b->bf[b->b.y].len; i++)
  {
   if (*(b->bf[b->b.y].s + i) == WPE_TAB)
    j += (f->ed->tabn - ((j + i) % f->ed->tabn) - 1);
#ifdef UNIX
   else if (((unsigned char) *(b->bf[b->b.y].s + i)) > 126)
   {
    j++;
    if (((unsigned char) *(b->bf[b->b.y].s + i)) < 128 + ' ')
     j++;
   }
   else if (*(b->bf[b->b.y].s + i) < ' ')
    j++;
#endif
  }
  b->b.x = i;
 }
 else
 {
  cp = strstr(b->bf[b->b.y].s, err_li[n].srch+1);
  for (i = 0; b->bf[b->b.y].s + i < cp; i++);
  if (err_li[n].srch[0] == 'B')
  {
   for (i--; i >= 0 && isspace(b->bf[b->b.y].s[i]); i--);
   if (i < 0 && b->b.y > 0)
   {
    (b->b.y)--;
    i = b->bf[b->b.y].len+1;
   }
   else
    i++;
  }
/*      else if(err_li[n].x < -1) i++;    */
  b->b.x = i +  err_li[n].x;
 }
 e_cursor(cn->f[cn->mxedt],1);
 return(0);
}

int e_pure_bin(char *str, int ch)
{
 int i;

 for (i = 0; isspace(str[i]); i++)
  ;
 for (; str[i] && str[i] != ch; i++)
  ;
 for(; i >= 0 && str[i] != DIRC; i--)
  ;
 return(i+1);
}

int e_make_error_list(FENSTER *f)
{
 char file[256];
 ECNT *cn = f->ed;
 BUFFER *b = cn->f[cn->mxedt]->b;
 int i, j, k = 0, ret = 0;
 char *spt;

 if (err_li)
 {
  for (i = 0; i < err_num; i++)
  {
   if(err_li[i].file) FREE(err_li[i].file);
   if(err_li[i].text) FREE(err_li[i].text);
   if(err_li[i].srch) FREE(err_li[i].srch);
  }
  FREE(err_li);
 }
 err_li = MALLOC(sizeof(struct ERR_LI) * b->mxlines);
 err_num = 0;
 for (i = 0; i < b->mxlines; i++)
 {
  if (!strncmp((char *)b->bf[i].s, "Error at Command:", 17)) 
   return(!ret ? -2 : ret);
  if ((!strncmp((char *)b->bf[i].s, "ld", 2) &&
    (b->bf[i].s[2] == ' '  || b->bf[i].s[2] == ':')) ||
    !strncmp((char *)b->bf[i].s, "collect:", 8))
   ret = -2;
  else if (!strncmp((char *)b->bf[i].s, "makefile:", 9) ||
    !strncmp((char *)b->bf[i].s, "Makefile:", 9))
  {
   err_li[k].file = MALLOC(9);
   for (j = 0; j < 8; j++)
    err_li[k].file[j] = b->bf[i].s[j];
   err_li[k].file[8] = '\0';
   err_li[k].line = atoi((char *)b->bf[i].s+9);
   err_li[k].y = i;
   err_li[k].x = 0;
   err_li[k].srch = NULL;
   err_li[k].text = MALLOC(strlen((char *)b->bf[i].s) + 1);
   strcpy(err_li[k].text, (char *)b->bf[i].s);
   err_li[k].text[b->bf[i].len] = '\0';
   k++;
   err_num++;
   ret = -1;
   continue;
  }
  else if (!strncmp((char *)b->bf[i].s, "make:", 5) &&
    ((spt = strstr((char *)b->bf[i].s, "makefile")) ||
    (spt = strstr((char *)b->bf[i].s, "Makefile")) ) &&
    (err_li[k].line = atoi(spt+14)) > 0 )
  {
   err_li[k].file = MALLOC(9);
   for (j = 0; j < 8; j++)
    err_li[k].file[j] = spt[j];
   err_li[k].file[8] = '\0';
   err_li[k].y = i;
   err_li[k].x = 0;
   err_li[k].srch = NULL;
   err_li[k].text = MALLOC(strlen((char *)b->bf[i].s) + 1);
   strcpy(err_li[k].text, (char *)b->bf[i].s);
   err_li[k].text[b->bf[i].len] = '\0';
   k++;
   err_num++;
   continue;
  }
  else
  {
   char *tststr = e_s_prog.comp_sw ? e_s_prog.intstr : gnu_intstr;
   if (!(ret = e_p_cmp_mess(tststr, b, &i, &k, ret)))
   {
    int ip, in;
    ip = e_pure_bin(e_s_prog.compiler, ' ');
    in = e_pure_bin(b->bf[i].s, ':');
    sprintf(file, "%s:", e_s_prog.compiler+ip);
    if (!strncmp(file, b->bf[i].s+in, strlen(file)))
     ret = -2;
    else if (!strncmp("ld:", b->bf[i].s+in, 3))
     ret = -2;
    else if (!strncmp("as:", b->bf[i].s+in, 3))
     ret = -2;
   }
  }
 }
 if (!(f->ed->edopt & (ED_ERRORS_STOP_AT | ED_MESSAGES_STOP_AT)) &&
   ret == -1)
  ret = 0;
 return(ret);
}

int e_previous_error(FENSTER *f)
{
 if (err_no > 0)
  return(e_show_error(--err_no, f));
 e_pr_uul(f->fb);
 return(0);
}

int e_next_error(FENSTER *f)
{
 if (err_no < err_num - 1)
  return(e_show_error(++err_no, f));
 e_pr_uul(f->fb);
 return(0);
}

int e_cur_error(int y, FENSTER *f)
{
 int i;

 if(err_num)
 {
  for(i = 1; i < err_num && err_li[i].y <= y; i++);
  return(e_show_error(err_no = i - 1, f));
 }
 e_pr_uul(f->fb);
 return(0);
}

int e_d_car_ret(FENSTER *f)
{
 if (!strcmp(f->datnam, "Messages"))
  return(e_cur_error(f->ed->f[f->ed->mxedt]->b->b.y, f));
#ifdef DEBUGGER
 if (!strcmp(f->datnam, "Watches"))
  return(e_edit_watches(f));
 if(!strcmp(f->datnam, "Stack"))
  return(e_make_stack(f));
#endif
 return(0);
}

int e_line_read(int n, char *s, int max)
{
 int i, ret = 0;

 for (i = 0; i < max - 1; i++)
  if ((ret = read(n, s + i, 1)) != 1 || s[i] == '\n'|| s[i] == '\0')
   break;
 if (ret != 1 && i == 0)
  return(-1);
 if (i == max - 1)
  i--;
 s[i+1] = '\0';
 return(0);
}

int e_arguments(FENSTER *f)
{
 char str[80];

 if (!e_prog.arguments)
 {
  e_prog.arguments = MALLOC(1);
  e_prog.arguments[0] = '\0';
 }
 strcpy(str, e_prog.arguments);
 if (e_add_arguments(str, "Arguments", f, 0 , AltA, NULL))
 {
  e_prog.arguments = REALLOC(e_prog.arguments, strlen(str) + 1);
  strcpy(e_prog.arguments, str);
 }
 return(0);
}

int e_check_c_file(char *name)
{
 int i, j;
 char *postfix;

 postfix = strrchr(name, '.');
 if (postfix)
 {
  for (i = 0; i < e_prog.num; i++)
   for (j = WpeExpArrayGetSize(e_prog.comp[i]->filepostfix); j; j--)
    if(!strcmp(e_prog.comp[i]->filepostfix[j - 1], postfix))
    {
     e_copy_prog(&e_s_prog, e_prog.comp[i]);
     return(i+1);
    }
 }
 return(0);
}

#ifdef CHECKHEADER

int e_check_header(char *file, M_TIME otime, ECNT *cn, int sw)
{
 struct stat cbuf[1];
 FILE *fp;
 char *p, str[120], str2[120];
 int i;

 for (i = cn->mxedt; i > 0; i--)
 {
  if (file[0] == DIRC)
   p = e_mkfilename(cn->f[i]->dirct, cn->f[i]->datnam);
  else
   p = cn->f[i]->datnam;
  if (!strcmp(p, file) && cn->f[i]->save)
  {  e_save(cn->f[i]);  if(p != cn->f[i]->datnam) FREE(p);  break;  }
  if (p != cn->f[i]->datnam)
   FREE(p);
 }
 if ((fp = fopen(file, "r")) == NULL)
  return(sw);
 stat(file, cbuf);
 if (otime < cbuf->st_mtime)
  sw++;
 while (fgets(str, 120, fp))
 {
  for (p = str; isspace(*p); p++);
  if (*p == '/' && *(p+1) == '*')
  {
   p++;
   do
   {
    for (p++; *p && *p != '*'; p++)
     ;
    if (!*p && !fgets((p = str), 120, fp))
     break;
   } while (p != NULL && (*p != '*' || *(p+1) != '/'));
   if (!p) break;
   for (p += 2; isspace(*p); p++)
    ;
  }
  if (*p == '#')
  {
   for (p++; isspace(*p); p++)
    ;
   if (!strncmp(p, "include", 7))
   {
    for (p += 8; isspace(*p); p++)
     ;
    if (*p == '\"')
    {
     for (p++, i = 0; p[i] != '\"' && p[i] != '\0' && p[i] != '\n'; i++)
      str2[i] = p[i];
     str2[i] = '\0';
     sw = e_check_header(str2, otime, cn, sw);
    }
   }
  }
 }
 fclose(fp);
 return(sw);
}
#endif

char *e_cat_string(char *p, char *str)
{
 if(str == NULL) return(p = NULL);
 if(p == NULL)
 {
  if((p = MALLOC(strlen(str)+2)) == NULL) return(NULL);
  p[0] = '\0';
 }
 else if ((p = REALLOC(p, strlen(p) + strlen(str)+2)) == NULL)
  return(NULL);
 strcat(p, " ");
 strcat(p, str);
 return(p);
}

int e_make_arg(char ***arg, char *str)
{
 int i, j;
 char tmp[128], *p = tmp;

 if (!(*arg))
  *arg = (char **) MALLOC(4*sizeof(char *));
 else
  *arg = (char **) REALLOC(*arg, 4*sizeof(char *));
 (*arg)[0] = MALLOC(strlen(e_s_prog.compiler) + 1);
 strcpy((*arg)[0], e_s_prog.compiler);
 if (!str)
 {
  (*arg)[1] = NULL;
  (*arg)[2] = NULL;
  return(2);
 }
 strcpy(tmp, str);
 for (j = 2, i = 0; p[i] != '\0'; j++)
 {
  for (; p[i] != '\0' && p[i] != ' '; i++)
   ;
  (*arg)[j] = MALLOC(i + 1);
  strncpy((*arg)[j], p, i);
  (*arg)[j][i] = '\0';
  *arg = (char **) REALLOC(*arg, (j + 3)*sizeof(char *));
  if (p[i] != '\0')
  {
   p += (i + 1);
   i = 0;
  }
 }
 (*arg)[j] = NULL;
 return(j);
}

int e_add_arg(char ***arg, char *str, int n, int argc)
{
 int i;

 argc++;
 *arg = (char **) REALLOC(*arg, (argc+1)*sizeof(char *));
 for(i = argc; i > n; i--)
  (*arg)[i] = (*arg)[i-1];
 (*arg)[n] = MALLOC(strlen(str) + 1);
 strcpy((*arg)[n], str);
 return(argc);
}

int e_ini_prog(ECNT *cn)
{
 int i;

 e_prog.num = 4;
 if (e_prog.arguments) FREE(e_prog.arguments);
 e_prog.arguments = WpeStrdup("");
 if (e_prog.project) FREE(e_prog.project);
 e_prog.project = WpeStrdup("project.prj");
 if (e_prog.exedir) FREE(e_prog.exedir);
 e_prog.exedir = WpeStrdup(".");
 if (e_prog.sys_include) FREE(e_prog.sys_include);
 e_prog.sys_include =
   WpeStrdup("/usr/include:/usr/local/include:/usr/include/X11");
 if (e_prog.comp == NULL)
  e_prog.comp = MALLOC(e_prog.num * sizeof(struct e_s_prog *));
 else
  e_prog.comp = REALLOC(e_prog.comp, e_prog.num * sizeof(struct e_s_prog *));
 for (i = 0; i < e_prog.num; i++)
  e_prog.comp[i] = MALLOC(sizeof(struct e_s_prog));
 e_prog.comp[0]->compiler = WpeStrdup("gcc");
 e_prog.comp[0]->language = WpeStrdup("C");
 e_prog.comp[0]->filepostfix = (char **)WpeExpArrayCreate(1, sizeof(char *), 1);
 e_prog.comp[0]->filepostfix[0] = WpeStrdup(".c");
 e_prog.comp[0]->key = 'C';
 e_prog.comp[0]->x = 0;
 e_prog.comp[0]->intstr = WpeStrdup(cc_intstr);
 e_prog.comp[2]->compiler = WpeStrdup("f77");
 e_prog.comp[1]->compiler = WpeStrdup("g++");
 e_prog.comp[1]->language = WpeStrdup("C++");
 e_prog.comp[1]->filepostfix = (char **)WpeExpArrayCreate(4, sizeof(char *), 1);
 e_prog.comp[1]->filepostfix[0] = WpeStrdup(".C");
 e_prog.comp[1]->filepostfix[1] = WpeStrdup(".cc");
 e_prog.comp[1]->filepostfix[2] = WpeStrdup(".cpp");
 e_prog.comp[1]->filepostfix[3] = WpeStrdup(".cxx");
 e_prog.comp[1]->key = '+';
 e_prog.comp[1]->x = 1;
 e_prog.comp[1]->intstr = WpeStrdup(cc_intstr);
 e_prog.comp[2]->language = WpeStrdup("Fortran");
 e_prog.comp[2]->filepostfix = (char **)WpeExpArrayCreate(1, sizeof(char *), 1);
 e_prog.comp[2]->filepostfix[0] = WpeStrdup(".f");
 e_prog.comp[2]->key = 'F';
 e_prog.comp[2]->x = 0;
 e_prog.comp[2]->intstr = WpeStrdup(cc_intstr);
 e_prog.comp[3]->compiler = WpeStrdup("pc");
 e_prog.comp[3]->language = WpeStrdup("Pascal");
 e_prog.comp[3]->filepostfix = (char **)WpeExpArrayCreate(1, sizeof(char *), 1);
 e_prog.comp[3]->filepostfix[0] = WpeStrdup(".p");
 e_prog.comp[3]->key = 'P';
 e_prog.comp[3]->x = 0;
 e_prog.comp[3]->intstr = WpeStrdup(pc_intstr);
 for (i = 0; i < e_prog.num; i++)
 {
  e_prog.comp[i]->comp_str = WpeStrdup("-g");
  e_prog.comp[i]->libraries = WpeStrdup("");
  e_prog.comp[i]->exe_name = WpeStrdup("");
  e_prog.comp[i]->comp_sw = i < 2 ? 0 : 1;
 }
 e_copy_prog(&e_s_prog, e_prog.comp[0]);
 return(0);
}

int e_copy_prog(struct e_s_prog *out, struct e_s_prog *in)
{
 int i;

 if (out->language) FREE(out->language);
 out->language = WpeStrdup(in->language);
 if (out->filepostfix)
 {
  for (i = WpeExpArrayGetSize(out->filepostfix); i; i--)
   WpeFree(out->filepostfix[i - 1]);
  WpeExpArrayDestroy(out->filepostfix);
 }
 out->filepostfix = (char **)WpeExpArrayCreate(WpeExpArrayGetSize(in->filepostfix), sizeof(char *), 1);
 for (i = WpeExpArrayGetSize(out->filepostfix); i; i--)
  out->filepostfix[i - 1] = WpeStrdup(in->filepostfix[i - 1]);
 if (out->compiler) FREE(out->compiler);
 out->compiler = WpeStrdup(in->compiler);
 if (out->comp_str) FREE(out->comp_str);
 out->comp_str = WpeStrdup(in->comp_str);
 if (out->libraries) FREE(out->libraries);
 out->libraries = WpeStrdup(in->libraries);
 if (out->exe_name) FREE(out->exe_name);
 out->exe_name = WpeStrdup(in->exe_name);
 if (out->intstr) FREE(out->intstr);
 out->intstr = WpeStrdup(in->intstr);
 out->key = in->key;
 out->comp_sw = in->comp_sw;
 return(0);
}

int e_prj_ob_btt(FENSTER *f, int sw)
{
 FLWND *fw;

 e_data_first(sw+4, f->ed, f->ed->dirct);
 if (sw > 0)
 {
  if (!(f->ed->edopt & ED_CUA_STYLE))
   while (e_data_eingabe(f->ed) != AF3)
    ;
  else
   while (e_data_eingabe(f->ed) != CF4)
    ;
  fw = (FLWND *)f->ed->f[f->ed->mxedt]->b;
  fw->df = NULL;
  e_close_window(f->ed->f[f->ed->mxedt]);
 }
 return(0);
}

int e_prj_ob_file(FENSTER *f)
{
 return(e_prj_ob_btt(f, 0));
}

int e_prj_ob_varb(FENSTER *f)
{
 return(e_prj_ob_btt(f, 1));
}

int e_prj_ob_inst(FENSTER *f)
{
 return(e_prj_ob_btt(f, 2));
}

int e_prj_ob_svas(FENSTER *f)
{
 return(e_project_name(f) ? 0 : AltS);
}

int e_project_options(FENSTER *f)
{
 int ret;
 W_OPTSTR *o = e_init_opt_kst(f);
 char *messagestring;

 if (!o)
  return(-1);
 if (!(e_make_prj_opt(f)))
 {
  freeostr(o);
  return(-1);
 }
 o->xa = 8;  o->ya = 2;  o->xe = 68;  o->ye = 22;
 o->bgsw = 0;
 o->name = "Project-Options";
 o->crsw = AltS;
 e_add_txtstr(4, 12, "Compiler-Style:", o);
 e_add_wrstr(4, 2, 22, 2, 36, 128, 0, AltC, "Compiler:", e_s_prog.compiler, NULL, o);
 e_add_wrstr(4, 4, 22, 4, 36, 256, 3, AltP, "ComPiler-Options:", e_s_prog.comp_str, NULL, o);
 e_add_wrstr(4, 6, 22, 6, 36, 256, 0, AltL, "Loader-Options:", e_s_prog.libraries, NULL, o);
 e_add_wrstr(4, 8, 22, 8, 36, 128, 0, AltE, "Executable:", e_s_prog.exe_name, NULL, o);
 e_add_wrstr(4, 10, 22, 10, 36, 128, 2, AltB, "LiBrary:", library, NULL, o);
 messagestring = WpeStringToValue(e_s_prog.intstr);
 e_add_wrstr(22, 12, 22, 13, 36, 256, 0, AltM, "Message-String:", messagestring, NULL, o);
 WpeFree(messagestring);
 e_add_pswstr(0, 5, 13, 0, AltG, 0, "GNU       ", o);
 e_add_pswstr(0, 5, 14, 1, AltT, e_s_prog.comp_sw, "OTher     ", o);
 e_add_bttstr(9, 18, 0, AltS, "Save", NULL, o);
 e_add_bttstr(44, 18, -1, WPE_ESC, "Cancel", NULL, o);
 e_add_bttstr(26, 18, 5, AltA, "Save As", e_prj_ob_svas, o);
/* e_add_bttstr(7, 16, 0, AltF, "Files ...", e_prj_ob_file, o);  */
 e_add_bttstr(12, 16, 0, AltV, "Variables ...", e_prj_ob_varb, o);
 e_add_bttstr(35, 16, 0, AltI, "Install ...", e_prj_ob_inst, o);
 ret = e_opt_kst(o);
 if (ret != WPE_ESC)
 {
  if (e_s_prog.compiler) FREE(e_s_prog.compiler);
  e_s_prog.compiler = WpeStrdup(o->wstr[0]->txt);
  if (e_s_prog.comp_str) FREE(e_s_prog.comp_str);
  e_s_prog.comp_str = WpeStrdup(o->wstr[1]->txt);
  if (e_s_prog.libraries) FREE(e_s_prog.libraries);
  e_s_prog.libraries = WpeStrdup(o->wstr[2]->txt);
  if (e_s_prog.exe_name) FREE(e_s_prog.exe_name);
  e_s_prog.exe_name = WpeStrdup(o->wstr[3]->txt);
  if (e_s_prog.intstr) FREE(e_s_prog.intstr);
  e_s_prog.intstr = WpeValueToString(o->wstr[5]->txt);
  strcpy(library, o->wstr[4]->txt);
  e_s_prog.comp_sw = o->pstr[0]->num;
  e_wrt_prj_fl(f);
 }
 freeostr(o);
 if (f->ed->mxedt > 0)
  e_ed_rahmen(f, 1);
 return(0);
}

int e_run_c_options(FENSTER *f)
{
 int i, j, ret;
 W_OPTSTR *o = e_init_opt_kst(f);
 char filepostfix[128];
 char *newpostfix;
 char *messagestring;

 if (!o)
  return(-1);
 o->xa = 8;  o->ya = 2;  o->xe = 68;  o->ye = 22;
 o->bgsw = 0;
 o->name = "Compiler-Options";
 o->crsw = AltO;
 e_add_txtstr(4, 14, "Compiler-Style:", o);
 e_add_wrstr(4, 2, 22, 2, 36, 128, 1, AltA, "LAnguage:", e_s_prog.language, NULL, o);
 e_add_wrstr(4, 4, 22, 4, 36, 128, 0, AltC, "Compiler:", e_s_prog.compiler, NULL, o);
 e_add_wrstr(4, 6, 22, 6, 36, 128, 3, AltP, "ComPiler-Options:", e_s_prog.comp_str, NULL, o);
 e_add_wrstr(4, 8, 22, 8, 36, 128, 0, AltL, "Loader-Options:", e_s_prog.libraries, NULL, o);
 e_add_wrstr(4, 10, 22, 10, 36, 128, 0, AltE, "Executable:", e_s_prog.exe_name, NULL, o);
 filepostfix[0] = 0;
 if ((j = WpeExpArrayGetSize(e_s_prog.filepostfix)))
 {
  strcpy(filepostfix, e_s_prog.filepostfix[0]);
  for (i = 1; i < j; i++)
  {
   strcat(filepostfix, " ");
   strcat(filepostfix, e_s_prog.filepostfix[i]);
  }
 }
 e_add_wrstr(4, 12, 22, 12, 36, 128, 0, AltF, "File-Postfix:", filepostfix, NULL, o);
 messagestring = WpeStringToValue(e_s_prog.intstr);
 e_add_wrstr(22, 14, 22, 15, 36, 128, 0, AltM, "Message-String:", messagestring, NULL, o);
 WpeFree(messagestring);
 e_add_pswstr(0, 5, 15, 0, AltG, 0, "GNU      ", o);
 e_add_pswstr(0, 5, 16, 1, AltT, e_s_prog.comp_sw, "OTher    ", o);
 e_add_bttstr(16, 18, 1, AltO, " Ok ", NULL, o);
 e_add_bttstr(37, 18, -1, WPE_ESC, "Cancel", NULL, o);
 ret = e_opt_kst(o);
 if (ret != WPE_ESC)
 {
  if (e_s_prog.language) FREE(e_s_prog.language);
  e_s_prog.language = WpeStrdup(o->wstr[0]->txt);
  if (e_s_prog.compiler) FREE(e_s_prog.compiler);
  e_s_prog.compiler = WpeStrdup(o->wstr[1]->txt);
  if (e_s_prog.comp_str) FREE(e_s_prog.comp_str);
  e_s_prog.comp_str = WpeStrdup(o->wstr[2]->txt);
  if (e_s_prog.libraries) FREE(e_s_prog.libraries);
  e_s_prog.libraries = WpeStrdup(o->wstr[3]->txt);
  if (e_s_prog.exe_name) FREE(e_s_prog.exe_name);
  e_s_prog.exe_name = WpeStrdup(o->wstr[4]->txt);
  for (i = 0; i < j; i++)
   WpeFree(e_s_prog.filepostfix[i]);
  WpeExpArrayDestroy(e_s_prog.filepostfix);
  e_s_prog.filepostfix = (char **)WpeExpArrayCreate(0, sizeof(char *), 1);
  for (i = 0; o->wstr[5]->txt[i]; i++)
  {
   if (isspace(o->wstr[5]->txt[i]))
    continue;
   for (j = i; (o->wstr[5]->txt[j]) && (!isspace(o->wstr[5]->txt[j])); j++)
    ;
   newpostfix = (char *)WpeMalloc(sizeof(char) * j - i + 1);
   strncpy(newpostfix, o->wstr[5]->txt + i, j - i);
   newpostfix[j - i] = 0;
   WpeExpArrayAdd((void **)&e_s_prog.filepostfix, &newpostfix);
   i = j - 1;
  }
  if (e_s_prog.intstr) FREE(e_s_prog.intstr);
  e_s_prog.intstr = WpeValueToString(o->wstr[6]->txt);
  e_s_prog.comp_sw = o->pstr[0]->num;
 }
 freeostr(o);
 return(0);
}

int e_run_options(FENSTER *f)
{
 int i, n, xa = 48, ya = 2, num = 2 + e_prog.num;
 OPTK *opt = MALLOC(num * sizeof(OPTK));
 char tmp[80];

 tmp[0] = '\0';
 opt[0].t = "Add Compiler   ";     opt[0].x = 0;  opt[0].o = 'A';
 opt[1].t = "Remove Compiler";     opt[1].x = 0;  opt[1].o = 'R';

 for (i = 0; i < e_prog.num; i++)
 {
  opt[i+2].t = e_prog.comp[i]->language;  opt[i+2].x = e_prog.comp[i]->x;
  opt[i+2].o = e_prog.comp[i]->key;
 }
 n = e_opt_sec_box(xa, ya, num, opt, f, 1);

 if (n == 0)
 {
  if (!e_run_c_options(f))
  {
   e_prog.num++;
   e_prog.comp = REALLOC(e_prog.comp, e_prog.num * sizeof(struct e_s_prog *));
   e_prog.comp[e_prog.num - 1] = MALLOC(sizeof(struct e_s_prog));
   e_prog.comp[e_prog.num - 1]->language = (char *)WpeMalloc(1);
   e_prog.comp[e_prog.num - 1]->language[0] = 0;
   e_prog.comp[e_prog.num - 1]->compiler = (char *)WpeMalloc(1);
   e_prog.comp[e_prog.num - 1]->compiler[0] = 0;
   e_prog.comp[e_prog.num - 1]->comp_str = (char *)WpeMalloc(1);
   e_prog.comp[e_prog.num - 1]->comp_str[0] = 0;
   e_prog.comp[e_prog.num - 1]->libraries = (char *)WpeMalloc(1);
   e_prog.comp[e_prog.num - 1]->libraries[0] = 0;
   e_prog.comp[e_prog.num - 1]->exe_name = (char *)WpeMalloc(1);
   e_prog.comp[e_prog.num - 1]->exe_name[0] = 0;
   e_prog.comp[e_prog.num - 1]->intstr = (char *)WpeMalloc(1);
   e_prog.comp[e_prog.num - 1]->intstr[0] = 0;
   e_prog.comp[e_prog.num - 1]->filepostfix =
     (char **)WpeExpArrayCreate(0, sizeof(char *), 1);
   e_copy_prog(e_prog.comp[e_prog.num-1], &e_s_prog);
   for (n = 0; e_prog.comp[e_prog.num-1]->language[n]; n++)
   {
    for (i = 0; i <= e_prog.num &&
      toupper(e_prog.comp[e_prog.num-1]->language[n]) != opt[i].o; i++)
     ;
    if (i > e_prog.num)
     break;
   }
   e_prog.comp[e_prog.num-1]->key =
    toupper(e_prog.comp[e_prog.num-1]->language[n]);
   e_prog.comp[e_prog.num-1]->x = n;
  }
 }
 else if (n == 1)
 {
  if(e_add_arguments(tmp, "Remove Compiler", f, 0, AltR, NULL))
  {
   for (i = 0; i < e_prog.num && strcmp(e_prog.comp[i]->language, tmp); i++)
    ;
   if (i >= e_prog.num)
   {
    e_error(e_p_msg[ERR_NO_COMPILER], 0, f->fb);
    FREE(opt);
    return(0);
   }
   FREE(e_prog.comp[i]);
   for(; i < e_prog.num-1; i++) e_prog.comp[i] = e_prog.comp[i+1];
    e_prog.num--;
  }
 }
 else if (n > 1)
 {
  e_copy_prog(&e_s_prog, e_prog.comp[n-2]);
  e_run_c_options(f);
  e_copy_prog(e_prog.comp[n-2], &e_s_prog);
 }
 FREE(opt);
 return(n < 0 ? WPE_ESC : 0);
}

int e_project_name(FENSTER *f)
{
 char str[80];

 if (!e_prog.project)
 {
  e_prog.project = MALLOC(1);
  e_prog.project[0] = '\0';
 }
 strcpy(str, e_prog.project);
 if (e_add_arguments(str, "Project", f, 0, AltP, NULL))
 {
  e_prog.project = REALLOC(e_prog.project, strlen(str) + 1);
  strcpy(e_prog.project, str);
  return(0);
 }
 return(WPE_ESC);
}

int e_project(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;
 if (!e_project_name(f))
 {
  for (i = cn->mxedt; i > 0 && (cn->f[i]->dtmd != DTMD_DATA || cn->f[i]->ins != 4);
    i--)
   ;
  if (i > 0)
  {
   e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
   e_close_window(cn->f[cn->mxedt]);
  }
  f = cn->f[cn->mxedt];
  e_make_prj_opt(f);
  e_rel_brkwtch(f);
  e_prj_ob_file(f);
  return(0);
 }
 return(WPE_ESC);
}

int e_show_project(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 for (i = cn->mxedt; i > 0 && (cn->f[i]->dtmd != DTMD_DATA || cn->f[i]->ins != 4);
   i--)
  ;
 if (i > 0)
  e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
 else
 {
  e_make_prj_opt(f);
  e_prj_ob_file(f);
 }
 return(0);
}

int e_cl_project(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 if (!e_prog.project)
  e_prog.project = MALLOC(sizeof(char));
 else
  e_prog.project = REALLOC(e_prog.project, sizeof(char));
 e_prog.project[0] = '\0';
 for (i = cn->mxedt; i > 0 && (cn->f[i]->dtmd != DTMD_DATA || cn->f[i]->ins != 4);
   i--)
  ;
 if (i > 0)
 {
  e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
  e_close_window(cn->f[cn->mxedt]);
 }
 return(0);
}

int e_p_add_item(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 for (i = cn->mxedt; i > 0 && (cn->f[i]->dtmd != DTMD_DATA || cn->f[i]->ins != 4);
   i--)
  ;
 if (i > 0)
  e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
 else
 {
  FLWND *fw;

  e_make_prj_opt(f);
  e_prj_ob_file(f);
  fw = (FLWND *) cn->f[cn->mxedt]->b;
  fw->nf = fw->df->anz - 1;
 }
 cn->f[cn->mxedt]->save = 1;
 WpeCreateFileManager(5, cn, NULL);
 WpeHandleFileManager(cn);
 return(0);
}

int e_p_del_item(FENSTER *f)
{
 ECNT *cn = f->ed;
 int i;

 for (i = cn->mxedt;
   i > 0 && (cn->f[i]->dtmd != DTMD_DATA || cn->f[i]->ins != 4); i--)
  ;
 if (i > 0)
  e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
 else
  return(e_error(e_p_msg[ERR_NOPROJECT], 0, f->fb));
 f = cn->f[cn->mxedt];
 f->save = 1;
 e_p_del_df((FLWND *) f->b, f->ins);
 return 0;
}

int e_make_library(char *library, char *ofile, FENSTER *f)
{
 char *ar_arg[5] = {  NULL, NULL, NULL, NULL, NULL  };
 int ret = 0, file = -1;
 PIC *pic = NULL;

 ar_arg[0] = "ar";
 if (access(library, 0))
  ar_arg[1] = "-cr";
 else
  ar_arg[1] = "-r";
 ar_arg[2] = library;
 ar_arg[3] = ofile;
 if ((ret = e_p_mess_win("Insert into Archive", 4, ar_arg, &pic, f)) == 0)
 {
  e_sys_ini();
  file = e_exec_inf(f, ar_arg, 4);
  e_sys_end();
  if ((file) && ((ret = e_p_exec(file, f, pic)) == 0))
  {
   pic = NULL;
/*
#ifdef RANLIB
    ar_arg[0] = "ranlib";
    ar_arg[1] = library;
    ar_arg[2] = NULL;
    if(ret = e_p_mess_win("Convert Archive", 2, ar_arg, &pic, f)) goto m_l_ende;
    e_sys_ini();
    file = e_exec_inf(f, ar_arg, 2);
    e_sys_end();
    if(file) ret = e_p_exec(file, f, pic);
#endif
*/
  }
 }
 return((!ret && file) ? 0 : -1);
}

int e_system(char *estr, ECNT *cn)
{
#if MOUSE
 int g[4];
#endif
 int ret;
 PIC *outp;
 FENSTER *f;

#if  MOUSE
 g[0] = 2;
 fk_mouse(g);
#endif
 f = cn->f[cn->mxedt-1];
 outp = e_open_view(0,0,MAXSCOL-1,MAXSLNS-1,cn->fb->ws,1);
 fk_locate(0,0);
 fk_cursor(1);
 (*e_u_s_sys_ini)();
 ret = system(estr);
 if (!WpeIsXwin())
 {
  printf(e_msg[ERR_HITCR]);
  fflush(stdout);
  fk_getch();
 }
 (*e_u_s_sys_end)();
 e_close_view(outp, 1);
 fk_cursor(0);
#if  MOUSE
 g[0] = 1;
 fk_mouse(g);
#endif
 return(ret);
}

/* arranges string str into buffer b and eventually wrappes string around
 wrap_limit columns */
int print_to_end_of_buffer(BUFFER * b,char * str,int wrap_limit)
{
 int i,k,j;

 k = 0;
 do
 {
  if (wrap_limit != 0)
   for (j = 0;
     ((j < wrap_limit) && (!((str[j + k] == '\n') || (str[j + k] == '\0'))));
     j++)
    ;
  else
   for (j = 0; (!((str[j + k] == '\n') || (str[j + k] == '\0'))); j++)
    ;
  /* Don't add blank lines */
  if (j == k)
   break;

/* b->mxlines - count of lines in b
   so add one more line at the end of buffer */
  e_new_line(b->mxlines, b);
  i = b->mxlines-1;

/* copy char from string (str) to buffer */

  if (str[j + k]!='\0')
   b->bf[i].s = REALLOC(b->bf[i].s, j + 2);
  else
   b->bf[i].s = REALLOC(b->bf[i].s, j + 1);
  strncpy(b->bf[i].s,str + k,j);

/* if this is not end of string, then we created substring
 if *(b->bf[i].s+j) is not '\0' then it is soft break is not written to file */

  if (str[j + k]!='\0')
  {
   *(b->bf[i].s + j) = '\n';
   *(b->bf[i].s + j + 1) = '\0';
  }
  else
  {
   *(b->bf[i].s + j) = '\0';
  }
/* update len of line in buffer */
  b->bf[i].len = j;
  b->bf[i].nrc = j + 1;

  if (str[j + k]=='\n')
  {
   j++;
  }

  k += j;

/* loop until end of string
 } while (str[k] != '\n' && str[k] != '\0');*/
 } while (str[k] != '\0');

 return 0;
}

/* print to message window */
int e_d_p_message(char *str, FENSTER *f, int sw)
{
 ECNT *cn = f->ed;
 BUFFER *b;
 SCHIRM *s;
 int i;

 if (str[0] == '\0' || str[0] == '\n')
  return(0);
 for (i = cn->mxedt; i > 0 && strcmp(cn->f[i]->datnam, "Messages"); i--)
  ;
 if (i == 0)
 {
  if (e_edit(cn, "Messages"))
   return(-1);
  else
   i = cn->mxedt;
 }

/* f - window */
 f = cn->f[i];

/* b - buffer */
 b = cn->f[i]->b;

/* s - content of window */
 s = cn->f[i]->s;

 print_to_end_of_buffer(b, str, b->mx.x);

/* place cursor on the last line */
 b->b.y = b->mxlines-1;

 if (sw)
  e_rep_win_tree(cn);
 else if (WpeIsXwin())
 {
  e_schirm(f, 0);
  e_cursor(f, 0);
  e_refresh();
 }
 return(0);
}

#if MOUSE
int e_d_car_mouse(FENSTER *f)
{
 extern struct mouse e_mouse;
 BUFFER *b = f->ed->f[f->ed->mxedt]->b;
 SCHIRM *s = f->ed->f[f->ed->mxedt]->s;

 if (e_mouse.y-f->a.y+s->c.y-1 == b->b.y)
  return(WPE_CR);
 else
 {
  b->b.y = e_mouse.y-f->a.y+s->c.y-1;
  b->b.x = e_mouse.x-f->a.x+s->c.x-1;
 }
 return(0);
}
#endif

int e_exec_make(FENSTER *f)
{
 ECNT *cn = f->ed;
 char **arg = NULL;
 int i, file, argc;

 WpeMouseChangeShape(WpeWorkingShape);
 efildes[0] = efildes[1] = -1;
 wfildes[0] = wfildes[1] = -1;
 for (i = cn->mxedt; i > 0; i--)
  if (!strcmp(cn->f[i]->datnam, "Makefile") ||
    !strcmp(cn->f[i]->datnam, "makefile"))
  {
   e_switch_window(cn->edt[i], cn->f[cn->mxedt]);
   e_save(cn->f[cn->mxedt]);
  }
 if (e_new_message(f))
  return(WPE_ESC);
 f = cn->f[cn->mxedt];
 e_sys_ini();
 if (e_s_prog.compiler)
  FREE(e_s_prog.compiler);
 e_s_prog.compiler = MALLOC(5*sizeof(char));
 strcpy(e_s_prog.compiler, "make");
 argc = e_make_arg(&arg, e_prog.arguments);
 if (argc == 0)
 {
  arg[1] = NULL;
  argc = 2;
 }
 else
 {
  for (i = 1; i < argc; i++)
   arg[i] = arg[i+1];
 }
 if ((file = e_exec_inf(f, arg, argc)) == 0)
 {
  e_sys_end();
  WpeMouseRestoreShape();
  return(WPE_ESC);
 }
 e_sys_end();
 e_free_arg(arg, argc - 1);
 i = e_p_exec(file, f, NULL);
 WpeMouseRestoreShape();
 return(i);
}

int e_run_sh(FENSTER *f)
{
 int ret, len = strlen(f->datnam);
 char estr[128];

 if (strcmp(f->datnam+len-3, ".sh"))
  return(1);

 WpeMouseChangeShape(WpeWorkingShape);   
 f->filemode |= 0100;
 if (f->save)
  e_save(f);
 strcpy(estr, f->datnam);
 strcat(estr, " ");
 if (e_prog.arguments)
  strcat(estr, e_prog.arguments);
#ifndef NO_XWINDOWS
 if (WpeIsXwin())
 {
  ret = (*e_u_system)(estr);
 }
 else
#endif
 ret = e_system(estr, f->ed);
 WpeMouseRestoreShape();
 return(0);
}

/*  new project   */

struct proj_var {  char *var, *string;  }  **p_v = NULL;
int p_v_n = 0;


char *e_interpr_var(char *string)
{
 int i, j;

 for (i = 0; string[i]; i++)
 {
  if (string[i] == '\\') 
   for (j = i; (string[j] = string[j+1]) != '\0'; j++)
    ;
  else if (string[i] == '\'' || string[i] == '\"')
  {
   for(j = i; (string[j] = string[j+1]) != '\0'; j++)
    ;
   i--;
  }
 }
 return(string);
}

char *e_expand_var(char *string, FENSTER *f)
{
 int i, j = 0, k, len, kl = 0;
 char *var = NULL, *v_string, *tmp;

 for (i = 0; string[i]; i++)
 {
  if (string[i] == '\'')
  {
   kl = kl ? 0 : 1;
   for (j = i; (string[j] = string[j+1]) != '\0'; j++);
   i--;
   continue;
  }
  if (string[i] == '\\' && (string[i+1] == 'n' || string[i+1] == 'r'))
  {
   string[i] = string[i+1] == 'n' ? '\n' : '\r';
   for (j = i+1; (string[j] = string[j+1]) != '\0'; j++);
   continue;
  }
  if (string[i] == '$' && !kl && (!i || string[i-1] != '\\'))
  {
   if (string[i+1] == '(')
   {
    for (j = i+2; string[j] && string[j] != ')'; j++);
    if (!string[j]) continue;
   }
   else if (string[i+1] == '{')
   {
    for (j = i+2; string[j] && string[j] != '}'; j++);
    if (!string[j]) continue;
   }
   if (string[i+1] == '(' || string[i+1] == '{')
   {
    if (!(var = MALLOC((j-i-1) * sizeof(char))))
    {
     e_error(e_msg[ERR_LOWMEM], 0, f->fb);
     return(string);
    }
    for (k = i+2; k < j; k++) var[k-i-2] = string[k];
    var[k-i-2] = '\0';
   }
   else
   {
    if (!(var = MALLOC(2 * sizeof(char))))
    {  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(string);  }
    var[0] = string[i+1];  var[1] = '\0';
   }
   if (!(v_string = getenv(var)))
   {
    for (k = 0; k < p_v_n - 1; k++)
    {
     if (!strcmp(p_v[k]->var, var))
     {  v_string = p_v[k]->string;  break;  }
    }
   }
   if (string[i+1] == '(' || string[i+1] == '{') len = (j-i+1);
   else len = 2;
   if (!v_string)
   {
    for(k = i; (string[k] = string[k+len]) != '\0'; k++);
    if (!(string = REALLOC(tmp = string, (strlen(string) + 1) * sizeof(char))))
    {  FREE(var);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(tmp);  }
   }
   else
   {
    len = strlen(v_string) - len;
    if (len >= 0)
    {
     if (!(string = REALLOC(tmp = string, (k = strlen(string) + len + 1) * sizeof(char))))
     {  FREE(var);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(tmp);  }
     for (k--; k > j + len; k--) string[k] = string[k-len];
     for (k = i; v_string[k-i]; k++) string[k] = v_string[k-i];
    }
    else
    {
     for (k = i; (string[k] = string[k-len]) != '\0'; k++);
     for (k = i; v_string[k-i]; k++) string[k] = v_string[k-i];
     if (!(string = REALLOC(tmp = string, (strlen(string) + 1) * sizeof(char))))
     {  FREE(var);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(tmp);  }
    }
   }
   FREE(var);
  }
 }
 return(string);
}

int e_read_var(FENSTER *f)
{
 struct proj_var **tmp;
 FILE *fp;
 char str[256], *sp1, *sp2, *stmp;
 int i;

 if ((fp = fopen(e_prog.project, "r")) == NULL) return(-1);
 if (p_v)
 {
  for (i = 0; i < p_v_n; i++)
  {
   if (p_v[i])
   {
    if (p_v[i]->var) FREE(p_v[i]->var);
    if (p_v[i]->string) FREE(p_v[i]->string);
    FREE(p_v[i]);
   }
  }
  FREE(p_v);
 }
 p_v_n = 0;
 if (!(p_v = MALLOC(sizeof(struct proj_var *))))
 {  fclose(fp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
 while (fgets(str, 256, fp))
 {
  for (i = 0; isspace(str[i]); i++);
  if (!str[i]) continue;
  else if (str[i] == '#')
  {
   while (str[strlen(str)-1] != '\n')
   {  fgets(str, 256, fp);  }
   continue;
  }
  sp1 = str + i;
  sp2 = strchr(sp1, '=');
  if (sp2 == NULL) continue;
  for (stmp = sp1; !((isspace(*stmp)) || (*stmp == '=')) && *stmp; stmp++)
   ;
  *stmp = 0;
  for (sp2++; isspace(*sp2) && *sp2 != '\n'; sp2++);
  p_v_n++;
  if (!(p_v = REALLOC(tmp = p_v, sizeof(struct proj_var *) * p_v_n)))
  {  p_v = tmp;  fclose(fp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
  if (!(p_v[p_v_n-1] = MALLOC(sizeof(struct proj_var))))
  {  fclose(fp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
  if (!(p_v[p_v_n-1]->var = MALLOC((strlen(sp1)+1) * sizeof(char))))
  {  fclose(fp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
  strcpy(p_v[p_v_n-1]->var, sp1);
  if (!(p_v[p_v_n-1]->string = MALLOC((strlen(sp2)+1) * sizeof(char))))
  {  fclose(fp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);  }
  strcpy(p_v[p_v_n-1]->string, sp2);
  while (p_v[p_v_n-1]->string[i = strlen(p_v[p_v_n-1]->string) - 1] != '\n' || (i && p_v[p_v_n-1]->string[i-1] == '\\'))
  {
   if (p_v[p_v_n-1]->string[i-1] == '\\') p_v[p_v_n-1]->string[i-1] = '\0';
   if (!fgets(str, 256, fp)) break;
   if (!(p_v[p_v_n-1]->string = REALLOC(stmp = p_v[p_v_n-1]->string,
		(strlen(p_v[p_v_n-1]->string)+strlen(str)+1) * sizeof(char))))
   {
    p_v[p_v_n-1]->string = stmp;
    fclose(fp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);  return(-1);
   }
   strcat(p_v[p_v_n-1]->string, str);
  }
  p_v[p_v_n-1]->string[strlen(p_v[p_v_n-1]->string) - 1] = '\0';
  for (i = 0; p_v[p_v_n-1]->string[i]; i++)
   if (p_v[p_v_n-1]->string[i] == '\t') p_v[p_v_n-1]->string[i] = ' ';
  p_v[p_v_n-1]->string = e_expand_var(p_v[p_v_n-1]->string, f);
 }
 fclose(fp);
 return(0);
}

int e_install(FENSTER *f)
{
 char *tp, *sp, *string, *tmp, text[256];
 FILE *fp;
 int i, j;

 if (e_p_make(f)) return(-1);
 if (!e__project) return(0);
 if ((fp = fopen(e_prog.project, "r")) == NULL)
 {
  sprintf(text, e_msg[ERR_FOPEN], e_prog.project);
  e_error(text, 0, f->fb);
  return(WPE_ESC);
 }
 while ((tp = fgets(text, 256, fp)))
 {
  if (text[0] == '\t') continue;
  for (i = 0; isspace(text[i]); i++)
   ;
  if (!strncmp(text+i, "install:", 8))
  {
   while (tp && (text[j = strlen(text)-1] != '\n' || text[j-1] == '\\'))
    tp = fgets(text, 256, fp);
   break;
  }
 }
 if (!tp)
 {
  fclose(fp);
  return(1);
 }
 while (tp && (tp = fgets(text, 256, fp)))
 {
  for (i = 0; isspace(text[i]); i++)
   ;
  sp = text+i;
  if (sp[0] == '#')
  {
   while (tp && (text[j = strlen(text)-1] != '\n' || text[j-1] == '\\'))
    tp = fgets(text, 256, fp);
   continue;
  }
  if (text[0] != '\t')
   break;
  if (!(string = MALLOC(strlen(sp) + 1)))
  {
   fclose(fp);
   e_error(e_msg[ERR_LOWMEM], 0, f->fb);
   return(-1);
  }
  strcpy(string, sp);
  while (tp && (text[j = strlen(text)-1] != '\n' || text[j-1] == '\\'))
  {
   tp = fgets(text, 256, fp);
   if (tp)
   {
    if (!(string = REALLOC(tmp = string, strlen(string) + strlen(text) + 1)))
    {
     fclose(fp);
     FREE(tmp);
     e_error(e_msg[ERR_LOWMEM], 0, f->fb);
     return(-1);
    }
    strcat(string, text);
   }
  }
  if (p_v_n)
   p_v_n++;
  string = e_expand_var(string, f);
  if (p_v_n) p_v_n--;
  e_d_p_message(string, f, 1);
  system(string);
  FREE(string);
 }
 fclose(fp);
 return(0);
}

struct dirfile *e_p_get_args(char *string)
{
 struct dirfile *df = MALLOC(sizeof(struct dirfile));
 char **tmp;
 int i, j, k;

 if (!df)
  return(NULL);
 if (!(df->name = MALLOC(sizeof(char *))))
 {
  FREE(df);
  return(NULL);
 }
 df->anz = 0;
 for (i = 0; string[i]; )
 {
  for (; isspace(string[i]); i++)
   ;
  for (j = i; string[j] && !isspace(string[j]); j++)
   ;
  if (j == i)
   break;
  df->anz++;
  if (!(df->name = REALLOC(tmp = df->name, df->anz * sizeof(char *))))
  {
   df->anz--;
   df->name = tmp;
   return(df);
  }
  if (!(df->name[df->anz-1] = MALLOC((j-i+1)*sizeof(char))))
  {
   df->anz--;
   return(df);
  }
  for (k = i; k < j; k++)
   *(df->name[df->anz-1] + k - i) = string[k];
  *(df->name[df->anz-1] + k - i) = '\0';
  e_interpr_var(df->name[df->anz-1]);
  i = j;
 }
 return(df);
}

struct dirfile *e_p_get_var(char *string)
{
 int i;

 for (i = 0; i < p_v_n; i++)
 {
  if(!strcmp(p_v[i]->var, string))
   return(e_p_get_args(p_v[i]->string));
 }
 return(NULL);
}

int e_c_project(FENSTER *f)
{
 ECNT *cn = f->ed;
 struct dirfile *df = NULL;
 char **arg;
 int i, j, k, file= -1, len, elen, argc, libsw = 0, exlib = 0, sccs = 0;
 char ofile[128];
#ifdef CHECKHEADER
 struct stat lbuf[1], obuf[1];
#else
 struct stat lbuf[1], cbuf[1], obuf[1];
#endif
 PIC *pic = NULL;

 last_time = (M_TIME) 0;
 e_p_l_comp = 0;
 if (e_new_message(f)) return(WPE_ESC);
 f = cn->f[cn->mxedt];
 if (e_s_prog.comp_str)
 {
  FREE(e_s_prog.comp_str);
  e_s_prog.comp_str = NULL;
 }
 e_s_prog.comp_sw &= ~1;
 e_argc = 1;
 argc = 1;
 for(i = f->ed->mxedt; i > 0 && (f->ed->f[i]->dtmd != DTMD_DATA ||
   f->ed->f[i]->ins != 4 || !f->ed->f[i]->save); i--)
  ;
 if (i > 0) e_p_update_prj_fl(f);
 if (e_read_var(f))
 {
  sprintf(ofile, e_msg[ERR_FOPEN], e_prog.project);
  e_error(ofile, 0, f->fb);
  return(-1);
 }
 e_arg = (char **) MALLOC(e_argc*sizeof(char *));
 arg = (char **) MALLOC(argc*sizeof(char *));
 df = e_p_get_var("CMP");
 if (!df)
 {
  e_error(e_p_msg[ERR_NOTHING], 0, f->fb);
  e_free_arg(arg, argc);  e_free_arg(e_arg, e_argc);
  return(-1);
 }
 for (k = 0; k < df->anz; k++, e_argc++, argc++)
 {
  j = e_argc == 1 ? 1 : 0;
  e_arg = REALLOC(e_arg, (e_argc+2)*sizeof(char *));
  e_arg[e_argc-j] = MALLOC(strlen(df->name[k]) + 1);
  strcpy(e_arg[e_argc-j], df->name[k]);
  arg = REALLOC(arg, (argc+2)*sizeof(char *));
  arg[argc-j] = MALLOC(strlen(df->name[k]) + 1);
  strcpy(arg[argc-j], df->name[k]);
  if (e_argc > 1)
   e_s_prog.comp_str = e_cat_string(e_s_prog.comp_str, e_arg[e_argc-j]);
 }
 freedf(df);
 arg[1] = MALLOC(3);
 strcpy(arg[1], "-c");
 e_arg[1] = MALLOC(3);
 strcpy(e_arg[1], "-o");
 df = e_p_get_var("CMPFLAGS");
 if (df)
 {
  for (k = 0; k < df->anz; k++, e_argc++, argc++)
  {
   j = e_argc == 1 ? 1 : 0;
   e_arg = REALLOC(e_arg, (e_argc+2)*sizeof(char *));
   e_arg[e_argc-j] = MALLOC(strlen(df->name[k]) + 1);
   strcpy(e_arg[e_argc-j], df->name[k]);
   arg = REALLOC(arg, (argc+2)*sizeof(char *));
   arg[argc-j] = MALLOC(strlen(df->name[k]) + 1);
   strcpy(arg[argc-j], df->name[k]);
   if (e_argc > 1)
    e_s_prog.comp_str = e_cat_string(e_s_prog.comp_str, e_arg[e_argc-j]);
  }
  freedf(df);
 }
 df = e_p_get_var("EXENAME");
 elen = strlen(e_prog.exedir)-1;
 if (e_prog.exedir[elen] == '/')
  sprintf(ofile, "%s%s", e_prog.exedir,
    (df && df->anz > 0 && df->name[0][0]) ? df->name[0] : "a.out");
 else
  sprintf(ofile, "%s/%s", e_prog.exedir,
    (df && df->anz > 0 && df->name[0][0]) ? df->name[0] : "a.out");
 if (df) freedf(df);
 if (e_s_prog.exe_name) FREE(e_s_prog.exe_name);
 e_s_prog.exe_name = WpeStrdup(ofile);
 e_argc = e_add_arg(&e_arg, e_s_prog.exe_name, 2, e_argc);
 df = e_p_get_var("LIBNAME");
 if (df)
 {
  strcpy(library, df->name[0]);
  if (access(library, 0)) exlib = 1;
  else stat(library, lbuf);
  freedf(df);
 }
 else
  library[0] = '\0';
 df = e_p_get_var("CMPSWTCH");
 if (df)
 {
  if (!strcmp(df->name[0], "other"))
   e_s_prog.comp_sw = 1;
  freedf(df);
 }
 df = e_p_get_var("CMPMESSAGE");
 if (df)
 {
  char *tmpstr = MALLOC(1);
  tmpstr[0] = '\0';
  for (k = 0; k < df->anz; k++)
  {
   tmpstr = REALLOC(tmpstr,
     (strlen(tmpstr)+strlen(df->name[k])+2)*sizeof(char));
   if (k) strcat(tmpstr, " ");
   strcat(tmpstr, df->name[k]);
  }
  if (e_s_prog.intstr) FREE(e_s_prog.intstr);
  e_s_prog.intstr = WpeStrdup(tmpstr);
  FREE(tmpstr);
  freedf(df);
 }
 else
 {
  if (e_s_prog.intstr) FREE(e_s_prog.intstr);
  e_s_prog.intstr = WpeStrdup(cc_intstr);
 }
 df = e_p_get_var("FILES");
 if (!df)
 {
  e_error(e_p_msg[ERR_NOTHING], 0, cn->fb);
  e_free_arg(arg, argc);  e_free_arg(e_arg, e_argc);
  return(-1);
 }
 arg[argc] = NULL;
 elen = strlen(e_prog.exedir)-1;
 for (k = 0; k < df->anz; k++)
 {
  for (j = cn->mxedt; j > 0; j--)
   if (!strcmp(cn->f[j]->datnam, df->name[k]) && cn->f[j]->save)
    e_save(cn->f[j]);
  for (j = strlen(df->name[k])-1; j >= 0 && df->name[k][j] != DIRC; j--)
   ;
  if (e_prog.exedir[elen] == '/')
   sprintf(ofile, "%s%s ", e_prog.exedir, df->name[k]+j+1);
  else sprintf(ofile, "%s/%s ", e_prog.exedir, df->name[k]+j+1);
  for (j = strlen(ofile); j > 0 && ofile[j] != '.'; j--)
   ;
  ofile[j+1] = 'o';
  ofile[j+2] = '\0';
  if (!stat(ofile, obuf))
  {
   if (obuf->st_mtime > last_time) last_time = obuf->st_mtime;
#ifdef CHECKHEADER
   if (!e_check_header(df->name[k], obuf->st_mtime, cn, 0)) goto gt_library;
#else
   stat(df->name[k], cbuf);
   if (obuf->st_mtime >= cbuf->st_mtime) goto gt_library;
#endif
  }
  argc = e_add_arg(&arg, df->name[k], argc, argc);
#ifndef NO_MINUS_C_MINUS_O
  argc = e_add_arg(&arg, "-o", argc, argc);
  argc = e_add_arg(&arg, ofile, argc, argc);
#endif
  arg[argc] = NULL;
  remove(ofile);
  sccs = 1;
  j = e_p_mess_win("Compiling", argc, arg, &pic, f);
  e_sys_ini();
  if (j != 0 || (file = e_exec_inf(f, arg, argc)) == 0)
  {
   e_sys_end();
   e_free_arg(arg, argc);
   freedf(df);
   e_free_arg(e_arg, e_argc);
   if (pic) e_close_view(pic, 1);
   return(WPE_ESC);
  }
  e_sys_end();
  e_p_l_comp = 1;
  if (e_p_exec(file, f, pic))
  {
   e_free_arg(arg, argc);
   e_free_arg(e_arg, e_argc);
   freedf(df);
   return(-1);
  }
  pic = NULL;
  for (j = strlen(ofile); j >= 0 && ofile[j] != '/'; j--)
   ;
  if (!exlib && library[0] != '\0' && strcmp(ofile+j+1, "main.o") &&
    (strncmp(e_s_prog.exe_name, ofile+j+1,(len = strlen(e_s_prog.exe_name))) ||
    ofile[len] == '.'))
  {
   if(e_make_library(library, ofile, f))
   {
    e_free_arg(arg, argc);
    e_free_arg(e_arg, e_argc);
    freedf(df);
    return(-1);  
   }
   else libsw = 1;
  }
  for (j = 0; j < 3; j++) FREE(arg[argc-j-1])
   ;
  argc -= 3;
gt_library:
  for (j = strlen(ofile); j >= 0 && ofile[j] != '/'; j--)
   ;
  if (library[0] == '\0' || !strcmp(ofile+j+1, "main.o") ||
    (!strncmp(e_s_prog.exe_name, ofile+j+1,(len = strlen(e_s_prog.exe_name))) &&
    ofile[len] == '.'))
   e_argc = e_add_arg(&e_arg, ofile, e_argc, e_argc);
  else if (exlib || obuf->st_mtime >= lbuf->st_mtime)
  {
   if (e_make_library(library, ofile, f))
   {
    e_free_arg(arg, argc);
    e_free_arg(e_arg, e_argc);
    freedf(df);
    return(-1);
   }
   else libsw = 1;
  }
 }
#ifdef RANLIB
 if (libsw && library[0] != '\0')
 {
  char *ar_arg[3];
  ar_arg[0] = "ranlib";
  ar_arg[1] = library;
  ar_arg[2] = NULL;
  if (!(j = e_p_mess_win("Convert Archive", 2, ar_arg, &pic, f)))
  {
   e_sys_ini();
   file = e_exec_inf(f, ar_arg, 2);
   e_sys_end();
   if (file) j = e_p_exec(file, f, pic);
  }
  if (j || !file)
  {
   e_free_arg(arg, argc);
   e_free_arg(e_arg, e_argc);
   freedf(df);
   return(-1);
  }
 }
#endif
 if (library[0] != '\0')
  e_argc = e_add_arg(&e_arg, library, e_argc, e_argc);
 freedf(df);
 df = e_p_get_var("LDFLAGS");
 if (df)
 {
  FREE(e_s_prog.libraries);
  e_s_prog.libraries = NULL;
  for (k = 0; k < df->anz; k++, e_argc++)
  {
   e_arg = REALLOC(e_arg, (e_argc+2)*sizeof(char *));
   e_arg[e_argc] = MALLOC(strlen(df->name[k]) + 1);
   strcpy(e_arg[e_argc], df->name[k]);
   e_s_prog.libraries = e_cat_string(e_s_prog.libraries, e_arg[e_argc]);
  }
  freedf(df);
 }
 e_arg[e_argc] = NULL;
 e_free_arg(arg, argc);
 if (!sccs) e_p_exec(file, f, pic);
 return(0);
}

int e_free_arg(char **arg, int argc)
{
 int i;

 for(i = 0; i < argc; i++)
  if(arg[i])
   FREE(arg[i]);
 FREE(arg);
 return(i);
}

char *e_find_var(char *var)
{
 int i;

 for(i = 0; i < p_v_n && strcmp(p_v[i]->var, var); i++);
 if(i >= p_v_n)
  return(NULL);
 else
  return(p_v[i]->string);
}

/****************************************************/
/**** reloading watches and breakpoints from prj ****/
/**** based on p_v variable ****/

/****
  this function is called only on project opening.
  It is used for additional parsing variables from
  project file, ie for loading BREAKPOINTS and WATCHES.
****/  
  
int e_rel_brkwtch(FENSTER *f)
{
 int i;

 for (i = 0; i < p_v_n; i++)
 {
  if (!strcmp(p_v[i]->var, "BREAKPOINTS"))
  {
   e_d_reinit_brks(f,p_v[i]->string);
  }
  else if (!strcmp(p_v[i]->var, "WATCHES"))
  {
   e_d_reinit_watches(f,p_v[i]->string);
  }
 }
 return 0;
}

/****************************************************/

/****
  this function is called each time options window
  is opened. But unfortunately also reloads variables
  from project file, which is not good for WATCHES
  and BREAKPOINTS.
****/  
struct dirfile **e_make_prj_opt(FENSTER *f)
{
 int i, j, ret;
 char **tmp, *sp, *tp, text[256];
 FILE *fp;
 struct dirfile *save_df = NULL;

 for (i = f->ed->mxedt; i > 0
	&& (f->ed->f[i]->dtmd != DTMD_DATA || f->ed->f[i]->ins != 4
				     || !f->ed->f[i]->save); i--);
 if (i > 0) {  save_df = e_p_df[0];  e_p_df[0] = NULL;  }
 if (e_p_df) freedfN(e_p_df, 3);
 e_p_df = MALLOC(3 * sizeof(struct dirfile *));
 if (!e_p_df) return(e_p_df);
 for (i = 0; i < 3; i++) e_p_df[i] = NULL;
 e_s_prog.comp_sw = 0;
 ret = e_read_var(f);
 if (ret)
 {
  if (e_s_prog.compiler) FREE(e_s_prog.compiler);
  e_s_prog.compiler = WpeStrdup("gcc");
  if (e_s_prog.comp_str) FREE(e_s_prog.comp_str);
  e_s_prog.comp_str = WpeStrdup("-g");
  if (e_s_prog.libraries) FREE(e_s_prog.libraries);
  e_s_prog.libraries = WpeStrdup("");
  if (e_s_prog.exe_name) FREE(e_s_prog.exe_name);
  /* Project my_prog.prj defaults to an executable of my_prog BD */
  strcpy(text, e_prog.project);
  e_s_prog.exe_name = WpeStrdup(WpeStringCutChar(text, '.'));
  /*e_s_prog.exe_name = WpeStrdup("a.out");*/
  if (e_s_prog.intstr) FREE(e_s_prog.intstr);
  e_s_prog.intstr = WpeStrdup(cc_intstr);
  strcpy(library, "");
  for (i = !save_df ? 0 : 1; i < 3; i++)
  {
   e_p_df[i] = MALLOC(sizeof(struct dirfile));
   e_p_df[i]->name = MALLOC(sizeof(char *));
   e_p_df[i]->name[0] = MALLOC(2 * sizeof(char));
   *e_p_df[i]->name[0] = ' '; *(e_p_df[i]->name[0] + 1) = '\0';
   e_p_df[i]->anz = 1;
  }
  if (save_df) e_p_df[0] = save_df;
  return(e_p_df);
 }
 if (!(e_p_df[1] = MALLOC(sizeof(struct dirfile)))) return(e_p_df);
 if (!(e_p_df[1]->name = MALLOC(sizeof(char *)))) return(e_p_df);
 e_p_df[1]->anz = 0;
 if (!(e_p_df[2] = MALLOC(sizeof(struct dirfile)))) return(e_p_df);
 if (!(e_p_df[2]->name = MALLOC(sizeof(char *)))) return(e_p_df);
 e_p_df[2]->anz = 0;
 for (i = 0; i < p_v_n; i++)
 {
  if (!strcmp(p_v[i]->var, "CMP"))
  {
   if (e_s_prog.compiler) FREE(e_s_prog.compiler);
   e_s_prog.compiler = WpeStrdup(p_v[i]->string);
  }
  else if (!strcmp(p_v[i]->var, "CMPFLAGS"))
  {
   if (e_s_prog.comp_str) FREE(e_s_prog.comp_str);
   e_s_prog.comp_str = WpeStrdup(p_v[i]->string);
  }
  else if (!strcmp(p_v[i]->var, "LDFLAGS"))
  {
   if (e_s_prog.libraries) FREE(e_s_prog.libraries);
   e_s_prog.libraries = WpeStrdup(p_v[i]->string);
  }
  else if (!strcmp(p_v[i]->var, "EXENAME"))
  {
   if (e_s_prog.exe_name) FREE(e_s_prog.exe_name);
   e_s_prog.exe_name = WpeStrdup(p_v[i]->string);
  }
  else if (!strcmp(p_v[i]->var, "CMPMESSAGE"))
  {
   if (e_s_prog.intstr) FREE(e_s_prog.intstr);
   e_s_prog.intstr = WpeStrdup(e_interpr_var(p_v[i]->string));
  }

/**************************/
/**** 
  this is needed, because this function needs to understand that
  BREAKPOINTS and WATCHES are project variables.
  These variables will be processed later on in e_rel_brkwtch
  function.
****/  
  else if (!strcmp(p_v[i]->var, "BREAKPOINTS"))
  {
  }
  else if (!strcmp(p_v[i]->var, "WATCHES"))
  {
  }
/**************************/

  else if (!strcmp(p_v[i]->var, "LIBNAME"))
   strcpy(library, p_v[i]->string);
  else if (!strcmp(p_v[i]->var, "CMPSWTCH"))
  {
   if (!strcmp(p_v[i]->string, "other")) e_s_prog.comp_sw = 1;
  }
  else if (!strcmp(p_v[i]->var, "FILES"))
   e_p_df[0] = e_p_get_args(p_v[i]->string);
  else
  {
   e_p_df[1]->anz++;
   if (!(e_p_df[1]->name = REALLOC(tmp =
				e_p_df[1]->name, e_p_df[1]->anz * sizeof(char *))))
   {  e_p_df[1]->anz--;  e_p_df[1]->name = tmp;  return(e_p_df);  }
   if (!(e_p_df[1]->name[e_p_df[1]->anz-1] = MALLOC((strlen(p_v[i]->var)
			+ strlen(p_v[i]->string) + 2)*sizeof(char))))
   {  e_p_df[1]->anz--;  return(e_p_df);  }
   sprintf(e_p_df[1]->name[e_p_df[1]->anz-1], "%s=%s",
					p_v[i]->var, p_v[i]->string);
  }
 }
 if (!e_s_prog.compiler)
  e_s_prog.compiler = WpeStrdup("gcc");
 if (!e_s_prog.comp_str)
  e_s_prog.comp_str = WpeStrdup("-g");
 if (!e_s_prog.libraries)
  e_s_prog.libraries = WpeStrdup("");
 if (!e_s_prog.exe_name)
 {
  /* Project my_prog.prj defaults to an executable of my_prog BD */
  strcpy(text, e_prog.project);
  e_s_prog.exe_name = WpeStrdup(WpeStringCutChar(text, '.'));
  /*e_s_prog.exe_name = WpeStrdup("a.out");*/
 }
 if (!e_s_prog.intstr)
  e_s_prog.intstr = WpeStrdup(cc_intstr);
 if (!e_p_df[0])
 {
  e_p_df[0] = MALLOC(sizeof(struct dirfile));
  e_p_df[0]->anz = 0;
 }
 if ((fp = fopen(e_prog.project, "r")) == NULL)
 {
  sprintf(text, e_msg[ERR_FOPEN], e_prog.project);
  e_error(text, 0, f->fb);
  return(e_p_df);
 }
 while ((tp = fgets(text, 256, fp)))
 {
  if (text[0] == '\t') continue;
  for (i = 0; isspace(text[i]); i++);
  if (!strncmp(text+i, "install:", 8))
  {
   while(tp && (text[j = strlen(text)-1] != '\n' || text[j-1] == '\\'))
    tp = fgets(text, 256, fp);
   break;
  }
 }
 if (!tp) {  fclose(fp);  return(e_p_df);  }
 while(tp && (tp = fgets(text, 256, fp)))
 {
  for (i = 0; isspace(text[i]); i++);
  sp = text+i;
  if (sp[0] == '#')
  {
   while(tp && (text[j = strlen(text)-1] != '\n' || text[j-1] == '\\'))
    tp = fgets(text, 256, fp);
   continue;
  }
  if (text[0] != '\t') break;
  if (sp[0] == '\0') continue;
  e_p_df[2]->anz++;
  if (!(e_p_df[2]->name = REALLOC(tmp =
				e_p_df[2]->name, e_p_df[2]->anz * sizeof(char *))))
  {  e_p_df[2]->anz--;  e_p_df[2]->name = tmp;  fclose(fp);  return(e_p_df);  }
  if (!(e_p_df[2]->name[e_p_df[2]->anz-1] = MALLOC((strlen(sp) + 1))))
  {  e_p_df[2]->anz--;  fclose(fp);  return(e_p_df);  }

  strcpy(e_p_df[2]->name[e_p_df[2]->anz-1], sp);
  while(tp && (text[j = strlen(text)-1] != '\n' || text[j-1] == '\\'))
  {
   tp = fgets(text, 256, fp);
   if (tp)
   {
    j = strlen(e_p_df[2]->name[e_p_df[2]->anz-1]);
    *(e_p_df[2]->name[e_p_df[2]->anz-1]+j-2) = '\0';
    if (!(e_p_df[2]->name[e_p_df[2]->anz-1] =
		    REALLOC(sp = e_p_df[2]->name[e_p_df[2]->anz-1],
			strlen(e_p_df[2]->name[e_p_df[2]->anz-1])
			+ strlen(text) + 1)))
    {  fclose(fp);  FREE(sp);  e_error(e_msg[ERR_LOWMEM], 0, f->fb);
	       return(e_p_df);
    }
    strcat(e_p_df[2]->name[e_p_df[2]->anz-1], text);
   }
  }
  j = strlen(e_p_df[2]->name[e_p_df[2]->anz-1]);
  if (*(e_p_df[2]->name[e_p_df[2]->anz-1]+j-1) == '\n')
   *(e_p_df[2]->name[e_p_df[2]->anz-1]+j-1) = '\0';
 }
 fclose(fp);
 for (i = 0; i < 3; i++)
 {
  if (!e_p_df[i])
  {
   e_p_df[i] = MALLOC(sizeof(struct dirfile));
   e_p_df[i]->name = MALLOC(sizeof(char *));
   e_p_df[i]->anz = 0;
  }
  e_p_df[i]->name = REALLOC(e_p_df[i]->name,
				(e_p_df[i]->anz + 1) * sizeof(char *));
  e_p_df[i]->name[e_p_df[i]->anz] = MALLOC(2*sizeof(char));
  *e_p_df[i]->name[e_p_df[i]->anz] = ' ';
  *(e_p_df[i]->name[e_p_df[i]->anz] + 1) = '\0';
  e_p_df[i]->anz++;
 }
 if (save_df) {  freedf(e_p_df[0]);  e_p_df[0] = save_df;  }
 return(e_p_df);
}

int freedfN(struct dirfile **df, int n)
{
 int i;

 for(i = 0; i < n; i++)
  if(df[i])
   freedf(df[i]);
 FREE(df);
 return(0);
}

int e_wrt_prj_fl(FENSTER *f)
{
 int i, len;
 FILE *fp;
 char text[256];

 for (i = f->ed->mxedt;
   i > 0 &&  (f->ed->f[i]->dtmd != DTMD_DATA || f->ed->f[i]->ins != 4); i--)
  ;
 if (i == 0 || e_prog.project[0] == DIRC)
  strcpy(text, e_prog.project);
 else
  sprintf(text, "%s/%s", f->ed->f[i]->dirct, e_prog.project);
 if ((fp = fopen(text, "w")) == NULL)
 {
  sprintf(text, e_msg[ERR_FOPEN], e_prog.project);
  e_error(text, 0, f->fb);
  return(-1);
 }
 fprintf(fp, "#\n# xwpe - project-file: %s\n", e_prog.project);
 fprintf(fp, "# created by xwpe version %s\n#\n", VERSION);
 for (i = 0; i < e_p_df[1]->anz; i++)
  fprintf(fp, "%s\n", e_p_df[1]->name[i]);
 fprintf(fp, "\nCMP=\t%s\n", e_s_prog.compiler);
 fprintf(fp, "CMPFLAGS=\t%s\n", e_s_prog.comp_str);
 fprintf(fp, "LDFLAGS=\t%s\n", e_s_prog.libraries);
 fprintf(fp, "EXENAME=\t%s\n", e_s_prog.exe_name);
 if (library[0])
  fprintf(fp, "LIBNAME=\t%s\n", library);
 fprintf(fp, "CMPSWTCH=\t%s\n", e_s_prog.comp_sw ? "other" : "gnu");
 fprintf(fp, "CMPMESSAGE=\t\'");
 for (i = 0; e_s_prog.intstr[i]; i++)
 {
  if (e_s_prog.intstr[i] == '\n')
   fprintf(fp, "\\n");
  else if (e_s_prog.intstr[i] == '\r')
   fprintf(fp, "\\r");
  else if (e_s_prog.intstr[i] == '\\' || e_s_prog.intstr[i] == '\'' ||
    e_s_prog.intstr[i] == '\"' )
  {
   fputc('\\', fp);
   fputc(e_s_prog.intstr[i], fp);
  }
  else fputc(e_s_prog.intstr[i], fp);
 }
 fprintf(fp, "\'\n");
 fprintf(fp, "\nFILES=\t");
 for (i = 0, len = 8; i < e_p_df[0]->anz; i++)
 {
  len += strlen(e_p_df[0]->name[i]);
  if (len > 80)
  {
   fprintf(fp, " \\\n\t");
   len = 1;
  }
  fprintf(fp, "%s ", e_p_df[0]->name[i]);
 }
 fprintf(fp, "\n");
   
/*****************************************/   
/****  save WATCHES and BREAKPOINTS   ****/
 if (e_d_nbrpts > 0)
 {
  fprintf(fp, "\nBREAKPOINTS=\t");
  for (i = 0; i < (e_d_nbrpts-1); i++)
  {
   fprintf(fp, "%s:%d;",e_d_sbrpts[i],e_d_ybrpts[i]);
  }
  fprintf(fp, "%s:%d",e_d_sbrpts[e_d_nbrpts-1],e_d_ybrpts[e_d_nbrpts-1]);
 }

 if (e_d_nwtchs > 0)
 {
  fprintf(fp, "\nWATCHES=\t");
  for (i = 0; i < (e_d_nwtchs-1); i++)
  {
   fprintf(fp, "%s;",e_d_swtchs[i]);
  }
  fprintf(fp, "%s",e_d_swtchs[e_d_nwtchs-1]);
 }   
 fprintf(fp, "\n");
/*****************************************/   

 if (e_p_df[2]->anz > 0)
  fprintf(fp, "\ninstall:\n");
 for (i = 0; i < e_p_df[2]->anz; i++)
  fprintf(fp, "\t%s\n", e_p_df[2]->name[i]);
 fclose(fp);
 return(0);
}

int e_p_update_prj_fl(FENSTER *f)
{
 if(!e_make_prj_opt(f))
  return(-1);
 if(e_wrt_prj_fl(f))
  return(-1);
 return(0);
}

int e_p_add_df(FLWND *fw, int sw)
{
 char *title = NULL, str[256];
 int i;

 if (sw == 4)
  title = "Add File";
 else if (sw == 5)
  title = "Add Variable";
 else if (sw == 6)
  title = "Add Command";
 str[0] = '\0'; /* terminate new string to prevent garbage in display */
 if (e_add_arguments(str, title, fw->f, 0, AltA, NULL))
 {
  fw->df->anz++;
  fw->df->name = REALLOC(fw->df->name, fw->df->anz * sizeof(char *));
  for (i = fw->df->anz - 1; i > fw->nf; i--)
   fw->df->name[i] = fw->df->name[i-1];
  fw->df->name[i] = MALLOC(strlen(str)+1);
  strcpy(fw->df->name[i], str);
 }
 return(0);
}

int e_p_edit_df(FLWND *fw, int sw)
{
 char *title = NULL, str[256];
 int new = 0;
 if (sw == 4)
  title = "Change Filename";
 else if (sw == 5)
  title = "Change Variable";
 else if (sw == 6)
  title = "Change Command";
 if (fw->nf < fw->df->anz-1 && fw->df->name[fw->nf])
  strcpy(str, fw->df->name[fw->nf]);
 else
 {
  new = 1;
  str[0] = '\0';
 }
 if (e_add_arguments(str, title, fw->f, 0, AltA, NULL))
 {
  if (fw->nf > fw->df->anz-2)
  {
   fw->nf = fw->df->anz-1;
   fw->df->anz++;
   fw->df->name = REALLOC(fw->df->name, fw->df->anz * sizeof(char *));
   fw->df->name[fw->df->anz-1] = fw->df->name[fw->df->anz-2];
  }
  if (!new)
   FREE(fw->df->name[fw->nf]);
  fw->df->name[fw->nf] = MALLOC(strlen(str)+1);
  if (fw->df->name[fw->nf])
   strcpy(fw->df->name[fw->nf], str);
 }
 return(0);
}

int e_p_del_df(FLWND *fw, int sw)
{
 int i;

 if (fw->nf > fw->df->anz-2)
  return(0);
 fw->df->anz--;
 for (i = fw->nf; i < fw->df->anz; i++)
  fw->df->name[i] = fw->df->name[i+1];
 return(0);
}

int e_p_mess_win(char *header, int argc, char **argv, PIC **pic, FENSTER *f)
{
 char *tmp = MALLOC(sizeof(char));
 int i, ret;

 fk_cursor(0);
 tmp[0] = '\0';
 for (i = 0; i < argc && argv[i] != NULL; i++)
 {
  if(!(tmp = REALLOC(tmp, (strlen(tmp)+strlen(argv[i])+2)*sizeof(char))))
   return(-2);
  strcat(tmp, argv[i]);
  strcat(tmp, " ");
 }
 ret = e_mess_win(header, tmp, pic, f);
 FREE(tmp);
 fk_cursor(1);
 return(ret);
}

/* After this function b has exactly 1 line allocated (b->mxlines==1).
   This line is initialized to the string WPE_WR,0 */
int e_p_red_buffer(BUFFER *b)
{
 int i;

 for (i = 1; i < b->mxlines; i++)
  if (b->bf[i].s != NULL)
   FREE( b->bf[i].s );
 if (b->mxlines==0) e_new_line(0,b);
 b->bf[0].s[0] = WPE_WR;
 b->bf[0].s[1] = '\0';
 b->bf[0].len = 0;
 b->bf[0].nrc = 1;
 b->mxlines = 1;
 return(0);
}

int e_new_message(FENSTER *f)
{
 int i;

 if (e_p_m_buffer)
  e_p_red_buffer(e_p_m_buffer);
 for (i = f->ed->mxedt; i > 0; i--)
  if (!strcmp(f->ed->f[i]->datnam, "Messages"))
  {
   e_switch_window(f->ed->edt[i], f->ed->f[f->ed->mxedt]);
   e_close_window(f->ed->f[f->ed->mxedt]);
  }
 if (access("Messages", 0) == 0)
  remove("Messages");
 if (e_edit(f->ed, "Messages"))
  return(WPE_ESC);
 return(0);
}

int e_p_show_messages(FENSTER *f)
{
 int i;

 for (i = f->ed->mxedt; i > 0; i--)
  if (!strcmp(f->ed->f[i]->datnam, "Messages"))
  {
   e_switch_window(f->ed->edt[i], f->ed->f[f->ed->mxedt]);
   break;
  }
 if (i <= 0 && e_edit(f->ed, "Messages"))
 {
  return(-1);
 }
 f = f->ed->f[f->ed->mxedt];
 if (f->b->mxlines == 0)
 {
  e_new_line(0, f->b);
  e_ins_nchar(f->b, f->s, "No Messages", 0, 0, 11);
  e_schirm(f, 1);
 }
 return(0);
}

int e_p_konv_mess(char *var, char *str, char *txt, char *file, char *cmp,
  int *y, int *x)
{
 int i;
 char *cp;

 if (!strncmp(var, "FILE", 4) && !isalnum(var[4]))
 {
  for (i = strlen(str) - 1; i >= 0 && !isspace(str[i]); i--)
   ;
  strcpy(file, str+i+1);
 }
 else if (!strncmp(var, "CMPTEXT", 7) && !isalnum(var[7]))
  strcpy(cmp, str);
 else if (!strncmp(var, "LINE", 4) && !isalnum(var[4]))
 {
  if (!isdigit(str[0]))
   return(1);
  *y = atoi(str);
  if (var[4] == '+')
   *y += atoi(var+5);
  else if (var[4] == '-')
   *y -= atoi(var+5);
 }
 else if (!strncmp(var, "COLUMN", 6) && !isalnum(var[6]))
 {
  if (!strncmp(var+6, "=BEFORE", 7))
  {
   txt[0] = 'B';
   strcpy(txt+1, str);
   *x = 0;
   var += 13;
  }
  else if (!strncmp(var+6, "=AFTER", 6))
  {
   txt[0] = 'A';
   strcpy(txt+1, str);
   *x = strlen(str);
   var += 12;
  }
  else if (!strncmp(var+6, "=PREVIOUS?", 10))
  {
   if (!str[0])
    return(1);
   for (i = 0; (txt[i] = var[16+i]) && txt[i] != '+' && txt[i] != '-'; i++)
    ;
   txt[i] = '\0';
   var += (16+i);
   cp = strstr(str, txt);
   for (i = 0; str+i < cp; i++)
    ;
   *x = i;
   txt[0] = 'P'; txt[1] = '\0';
  }
  else if (!isdigit(str[0]))
   return(1);
  else
  {
   *x = atoi(str);
   txt[0] = '\0';
   var += 6;
  }
  if (var[0] == '+')
   *x += atoi(var+1);
  else if (var[0] == '-')
   *x -= atoi(var+1);
 }
 return(0);
}

int e_p_comp_mess(char *a, char *b, char *c, char *txt, char *file, char *cmp,
  int *y, int *x)
{
 int i, n, k = 0, bsl = 0;
 char *ctmp, *cp, *var = NULL, *str = NULL;

 if (c > b)
  return(0);
 if (a[0] == '*' && !a[1])
  return(2);
 if (!a[0] && !b[0])
  return(2);
 if (!a[0] || !b[0])
  return(0);
 if (a[0] == '*' && (a[1] == '*' || a[1] == '$'))
  return(e_p_comp_mess(++a, b, c, txt, file, cmp, y, x));
 if (a[0] == '$' && a[1] == '{')
 {
  for (k = 2; a[k] && a[k] != '}'; k++);
  var = MALLOC((k-1) * sizeof(char));
  for (i = 2; i < k; i++)
   var[i-2] = a[i];
  var[k-2] = '\0';
  if (a[k])
   k++;
  if (!a[k])
   return(!e_p_konv_mess(var, b, txt, file, cmp, y, x));
  n = a[k] == '\\' ? k : k+1;
 }
 else if (a[0] == '*'&& a[1] != '\\')
 {
  k = 1;
  n = 2;
 }
 else
  n = 1;
 for(; bsl || (a[n] && a[n] != '*' && a[n] != '?' && a[n] != '[' &&
   (a[n] != '$' || a[n+1] != '{' )); n++)
  bsl = a[n] == '\\' ? !bsl : 0;
 if (a[0] == '*' || a[0] == '$')
 {
  if (a[k] == '?')
  {
   cp = MALLOC((strlen(a)+1)*sizeof(char));
   for (i = 0; i < k && (cp[i] = a[i]); i++);
   for (i++; (cp[i-1] = a[i]) != '\0'; i++);
   FREE(var);
   n = e_p_comp_mess(cp, ++b, ++c, txt, file, cmp, y, x);
   FREE(cp);
   return(n);
  }
  if (a[k] == '[')
  {
   for (i = 0; b[i] &&
     !(n = e_p_comp_mess(a+k, b+i, c+i, txt, file, cmp, y, x)); i++)
    ;
   if (!b[i])
    return(0);
   if (a[0] == '$')
   {
    str = MALLOC((i+1)*sizeof(char));
    for (k = 0; k < i; k++)
     str[k] = b[k];
    str[i] = '\0';
    e_p_konv_mess(var, str, txt, file, cmp, y, x);
    FREE(var);
    FREE(str);
   }
   return(n);
  }
  n -= k;
  ctmp = MALLOC(n+1);
  for (i = 0; i < n; i++)
   ctmp[i] = a[i+k];
  ctmp[n] = '\0';
  cp = strstr(b, ctmp);
  FREE(ctmp);
  if (cp == NULL)
   return(0);
  if (a[0] == '$')
  {
   for (i = 0; c + i < cp; i++);
   str = MALLOC((i+1)*sizeof(char));
   for (i = 0; c + i < cp; i++)
    str[i] = c[i];
   str[i] = '\0';
   i = e_p_konv_mess(var, str, txt, file, cmp, y, x);
   FREE(var);  
   FREE(str);
   if (i)
    return(0);
  }
  if (!a[k+n] && !cp[n])
   return(2);
  if (!a[k+n])
   return(e_p_comp_mess(a, cp+1, cp+1, txt, file, cmp, y, x));
  if ((i = e_p_comp_mess(a+k+n, cp+n, cp+n, txt, file, cmp, y, x)))
   return(i);
  if (file[0] && *y > -1)
   return(0);
  return(e_p_comp_mess(a, cp+1, a[0] == '$' ? c : cp+1, txt, file, cmp, y, x));
 }
 else if (a[0] == '?')
 {
  n--;
  a++;
  b++;
 }
 else if (a[0] == '[')
 {
  if (a[1] == '!')
  {
   for (k = 2; a[k] && (a[k] != ']' || k == 2) && a[k] != b[0]; k++)
    if (a[k+1] == '-' && b[0] >= a[k] && b[0] <= a[k+2])
     return(-b[0]);
   if (a[k] != ']')
    return(-b[0]);
   n-=(k+1);
   a+=(k+1);
   b++;
  }
  else
  {
   for (k = 1; a[k] && (a[k] != ']' || k == 1) && a[k] != b[0]; k++)
    if (a[k+1] == '-' && b[0] >= a[k] && b[0] <= a[k+2])
     break;
   if (a[k] == ']' || a[k] == '\0')
    return(0);
   for(; a[k] && (a[k] != ']'); k++);
   n-=(k+1);
   a+=(k+1);
   b++;
  }
 }
 if (n <= 0)
  return(e_p_comp_mess(a, b, c, txt, file, cmp, y, x));
 if ((k = strncmp(a, b, n)) != 0)
  return(0);
 return(e_p_comp_mess(a+n, b+n, c+n, txt, file, cmp, y, x));
}

int e_p_cmp_mess(char *srch, BUFFER *b, int *ii, int *kk, int ret)
{
 char *cp, cmp[128], file[128], search[80], tmp[4][128], **wtxt = NULL;
 int j, l, m, n, iy, iorig, i = *ii, k = *kk, x = 0, y = -1, wnum = 0;
 int *wn = NULL;

 cmp[0] = search[0] = file[0] = '\0';
 wtxt = MALLOC(1);
 wn = MALLOC(1);
 for (j = 0, n = 0; n < 4 && srch[j]; n++)
 {
  for (l = 0; (tmp[n][l] = srch[j]); j++, l++)
  {
   if (j > 1 && srch[j] == '?' && srch[j-1] == '{' && srch[j-2] == '$')
   {
    wnum++;
    wn = REALLOC(wn, wnum * sizeof(int));
    wtxt = REALLOC(wtxt, wnum * sizeof(char *));
    if (srch[j+1] == '*')
     wn[wnum-1] = -1;
    else
     wn[wnum-1] = atoi(srch+j+1);
    for (j++; srch[j] && srch[j] != ':'; j++);
    if (!srch[j])
    {
     wnum--;
     break;
    }
    for (m = 0; srch[j+m] && srch[j+m] != '}'; m++);
    wtxt[wnum-1] = MALLOC((m+1) * sizeof(char));
    for (m = 0, j++; (wtxt[wnum-1][m] = srch[j]) && srch[j] != '}'; j++, m++);
    wtxt[wnum-1][m] = '\0';
    l -= 3;
   }
   else if (srch[j] == '\r' || srch[j] == '\n')
   {
    if (srch[j+1] == '\r' || srch[j+1] == '\n')
    {
     tmp[n][l] = '\n';
     tmp[n][l+1] = '\0';
     j++;
    }
    else
     tmp[n][l] = '\0';
    j++;
    break;
   }
  }
 }
 e_p_comp_mess(tmp[0], b->bf[i].s, b->bf[i].s, search, file, cmp, &y, &x);
 iy = i;
 iorig = i;
 do
 {
  if (n > 1 && file[0] && i < b->mxlines-1)
  {
   y = -1;
   while (b->bf[i].s[b->bf[i].len-1] == '\\')
    i++;
   i++;
   e_p_comp_mess(tmp[1], b->bf[i].s, b->bf[i].s, search, file, cmp, &y, &x);
   iy = i;
  }
  do
  {
   if (n > 2 && file[0] && y >= 0 && i < b->mxlines-1)
   {
    while (b->bf[i].s[b->bf[i].len-1] == '\\')
     i++;
    i++;
    l = e_p_comp_mess(tmp[2], b->bf[i].s, b->bf[i].s, search, file, cmp, &y, &x);
    if (!l && n > 3)
     l = e_p_comp_mess(tmp[3], b->bf[i].s, b->bf[i].s, search, file, cmp, &y, &x);
   }
   else
    l = 1;
   if (file[0] && y >= 0 && l != 0)
   {
    err_li[k].file = MALLOC((strlen(file)+1)*sizeof(char));
    strcpy(err_li[k].file, file);
    err_li[k].line = y;
    if (search[0] == 'P')
    {
     cp = strstr(b->bf[iy].s, cmp);
     if (!cp)
      x = 0;
     else
     {
      for (m = 0; b->bf[iy].s + m < (unsigned char *)cp; m++);
      x -= m;
     }
     err_li[k].srch = MALLOC((strlen(cmp)+2)*sizeof(char));
     err_li[k].srch[0] = 'P';
     strcpy(err_li[k].srch+1, cmp);
    }
    else if (search[0])
    {
     err_li[k].srch = MALLOC((strlen(search)+1)*sizeof(char));
     strcpy(err_li[k].srch, search);
    }
    else
     err_li[k].srch = NULL;
    err_li[k].x = x;
    err_li[k].y = iorig;
    err_li[k].text = MALLOC(strlen((char *)b->bf[i].s) + 1);
    strcpy(err_li[k].text, (char *)b->bf[i].s);
    err_li[k].text[b->bf[i].len] = '\0';
    k++;
    err_num++;
    if (!ret)
    {
     for (ret = -1, m = 0; ret && m < wnum; m++)
     {
      if (wn[m] == -1 && !(b->cn->edopt & ED_MESSAGES_STOP_AT) &&
        strstr(b->bf[i].s, wtxt[m]))
       ret = 0;
      else if (wn[m] > -1 && !(b->cn->edopt & ED_MESSAGES_STOP_AT) &&
        !strncmp(b->bf[i].s+wn[m], wtxt[m], strlen(wtxt[m])))
       ret = 0;
     }
    }
    if (!ret && wnum <= 0)
     ret = -1;
    while (b->bf[i].s[b->bf[i].len-1] == '\\')
     i++;
   }
  } while (n > 2 && file[0] && y >= 0 && l != 0 && i < b->mxlines-1);
  if (n > 2 && file[0] && y >= 0 && l == 0)
   i--;
 } while (n > 1 && file[0] && y >= 0 && i < b->mxlines-1);
 if (n > 1 && file[0] && y < 0)
  i--;
 *ii = i;
 *kk = k;
 for (m = 0; m < wnum; m++)
  FREE(wtxt[m]);
 FREE(wn);
 FREE(wtxt);
 return(ret);
}

#endif

