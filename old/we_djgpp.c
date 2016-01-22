/* we_term.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "messages.h"
#include "edit.h"
#ifdef DJGPP

#include<pc.h>
#include<bios.h>
#include<dos.h>
#include<dpmi.h>
#include<mouse.h>
#include<graphics.h>
#include <unistd.h>

#include<signal.h>
#include<fcntl.h>
#include<process.h>
#include "makro.h"

int MAXSLNS = 25;
int MAXSCOL = 80;
u_short e_h_di = 0, e_h_ax = 0;
   
#ifdef PROG
extern int wfildes[2], efildes[2];
extern char *e_p_msg[];
#endif

int cur_x = -1, cur_y = -1, cur_a = 0, cur_e = 0, c_flag = 0;

int e_t_initscr()
{  int i, ret;
   union REGS cpu;
/*
    cpu.h.ah = 3;
    cpu.h.bh = 0;
    int86(0x10, &cpu, &cpu);
*/
   cpu.h.ah = 15;
   int86(0x10, &cpu, &cpu);
   if(cpu.h.al == 2 || cpu.h.al == 7)
   {  cur_a = 11;
      cur_e = 12;
   }
   else
   {  cur_a = 6;
      cur_e = 7;
   }
   e_begscr();
   schirm = (char *) ScreenPrimary;
   e_abs_refr();
   return(0);
}

int svflgs, kbdflgs;

int e_begscr()
{
   int cols, lns;
   if((lns = ScreenRows()) > 0) MAXSLNS = lns;
   if((cols = ScreenCols()) > 0) MAXSCOL = cols;
   
   signal(SIGSEGV, e_exit);
   
   return(0);
}

int e_endwin()
{
   return(0);
}

/*  Cursor Ein- und Ausschalten   */

int fk_t_cursor(int sw)
{
   union REGS cpu;
   cpu.h.ah = 1;
   if(sw == 0)
   {  cpu.h.ch = 1;
      cpu.h.cl = 0;
   }
   else if(sw == 2)
   {  cpu.h.ch = 1;
      cpu.h.cl = cur_e;
   }
   else
   {  cpu.h.ch = cur_a;
      cpu.h.cl = cur_e;
   }
   int86(0x10, &cpu, &cpu);
   return(0);
}

char *tmpschirm, *svschirm;

void ini_repaint(ECNT *cn)
{
   svschirm = schirm;
   if((tmpschirm = (char *)MALLOC(4000*sizeof(char))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, cn->fb);
   else
   {
      schirm = tmpschirm;
      e_cls(cn->fb->df.fb, cn->fb->dc);
      e_ini_desk(cn);
   }
}

void end_repaint()
{
   int n;
   if(tmpschirm != NULL)
   {  schirm = svschirm;
      for(n = 0; n < 160*MAXSLNS; n++)
	    *(schirm + n) = *(tmpschirm + n);
      FREE((char *)tmpschirm);
   }
}

void ini_repaint_1(FENSTER *f)
{
   svschirm = schirm;
   if((tmpschirm = (char *)MALLOC(4000*sizeof(char))) == NULL)
   e_error(e_msg[ERR_LOWMEM], 1, f->fb);
   else  schirm = tmpschirm;
}

void end_repaint_1(FENSTER *f)
{
   int i, j;
   if(tmpschirm != NULL)
   {  schirm = svschirm;
      for(j = f->a.y+1; j < f->e.y; j++)
      for(i = f->a.x+1; i < f->e.x; i++)
      {  *(schirm+2*MAXSCOL*j+2*i)   = *(tmpschirm+2*MAXSCOL*j+2*i);
	 *(schirm+2*MAXSCOL*j+2*i+1) = *(tmpschirm+2*MAXSCOL*j+2*i+1);
      }
      FREE((char *)tmpschirm);
   }
}


int x_bioskey()
{
   union REGS cpu;
   cpu.h.ah = 2;
   int86(0x16,&cpu,&cpu);
   return(cpu.h.al);
}

int e_kbhit()
{
   if(kbhit()) return(getch());
   return(e_mshit());
}

int e_t_getch(void)
{
#if  MOUSE
   extern struct mouse e_mouse;
   int g[4] = {  1, 0, 0, 0,  };
#endif
   int c = 0, b = 0;
#if  MOUSE
   e_refresh();
   fk_mouse(g);
   while (c == 0)
   {  if (kbhit() == 0)
      {  g[0] = 3;
	 fk_mouse(g);
	 if(g[1] != 0)
	 {  if(g[1] == 2) c = -4;
	    else if(g[1] == 3) c = -2;
	    else c = - g[1];
	    e_mouse.x = g[2]/8;
	    e_mouse.y = g[3]/8;
	 }
      }
      else
      {
#endif
	 if( (c = getxkey()) > 512 ) c -= 256;
	 if( c > 255 )
	 {  c--;
	    if(((b = bioskey()) & 3) != 0) c = 512 + c ;
	    else if(c == CF9 - 1) c++;
	    else if(c == 401 || c == 402) c+= 450;
	    else if((c == 337 || c == 338) && (b & 4)) c+= 514;
	 }
	 else if( c == 255 ) c = 0;
	 else if( c == ' ' && (bioskey() & 8)) c = AltBl;
#if  MOUSE
      }
   }
   g[0] = 2;
   fk_mouse(g);
#endif
   return(c);
}

int fk_t_locate(int x, int y)
{
   cur_x = x;
   cur_y = y;
   ScreenSetCursor(y, x);
   return(0);
}

int fk_t_putchar(char c)
{
   return(fputc(c, stdout));
}

#if MOUSE
int fk_t_mouse(int g[])
{
   extern int fk__mouse_stat;
   union REGS cpu;
   if (fk__mouse_stat==0) return(0);
   if (fk__mouse_stat==1)
   {  cpu.x.ax=0;
      int86(0x33,&cpu,&cpu);
      if (cpu.x.ax==0)
      {  fk__mouse_stat=0;
	 return(-1);  }
      fk__mouse_stat=2;
   }
   cpu.x.ax=g[0];
   cpu.x.bx=g[1];
   cpu.x.cx=g[2];
   cpu.x.dx=g[3];
   int86(0x33,&cpu,&cpu);
   g[0]=cpu.x.ax;
   g[1]=cpu.x.bx;
   g[2]=cpu.x.cx;
   g[3]=cpu.x.dx;
   return(1);
}
#else
int fk_t_mouse(int g[])
{
   return(0);
}
#endif

int setdisk(int x)
{
   union REGS cpu;
   cpu.h.ah = 0x0e;
   cpu.h.dl = x;
   intdos(&cpu, &cpu);
   return(cpu.x.cflag);
}

int getdisk()
{
   union REGS cpu;
   cpu.h.ah = 0x19;
   intdos(&cpu, &cpu);
   return(cpu.h.al);
}

int getcurdir(int n, char *str)
{
   union REGS cpu;
   cpu.h.ah = 0x47;
   cpu.h.dl = n;
   cpu.x.si = (unsigned long) str;
   intdos(&cpu, &cpu);
   return(-cpu.x.cflag);
}

int fk_setdisk(int n)
{
   extern struct EXT h_error;
   char s[256];
   union REGS cpu;
   h_error.sw = 0;
   cpu.h.ah = 0x44;
   cpu.h.al = 0x0f;
   cpu.h.bl = n+1;
   intdos(&cpu, &cpu);
/*   if(n < 2) e_dsk_test(n + 1);  */
   if(h_error.sw) return(e_h_error());
   if(getcurdir(n+1, s) != 0)
   {  if(h_error.sw) return(e_h_error());
      return(-1);
   }
   if(h_error.sw) return(e_h_error());
   setdisk(n);
   return(0);
}
#undef getcwd();
#undef chdir();
#ifdef DJ_OLD
char *e_getcwd(char *buf, int n)
{
   char *p = buf+2;
   union REGS cpu;
   cpu.h.ah = 0x19;
   intdos(&cpu, &cpu);
   buf[0] = cpu.h.al + 'A';
   buf[1] = ':';
   if(!getcwd(p, n-2)) return(NULL);
   for(; *p; p++) if(*p == '/') *p = '\\';
   return(buf);
}
#else
char *e_getcwd(char *buf, int n)
{
   char *p = buf;
   if(!getcwd(p, n)) return(NULL);
   for(; *p; p++) if(*p == '/') *p = '\\';
   return(buf);
}
#endif
int e_chdir(char *buf)
{
   char *p = buf;
   if(buf[1] == ':')
   {  union REGS cpu;
      cpu.h.ah = 0x0e;
      cpu.h.dl = toupper(buf[0]) - 'A';
      intdos(&cpu, &cpu);
      p += 2;
   }
   return(chdir(p));
}

int fk_lfw_test( int disk)
{
   int ret, save;
   union REGS cpu;
   cpu.h.ah = 0x19;
   intdos(&cpu, &cpu);
   save = cpu.h.al;
   cpu.h.ah = 0x44;
   cpu.h.al = 0x0e;
   cpu.h.bl = 1+disk;
   intdos(&cpu, &cpu);
   if(!cpu.x.cflag || cpu.x.ax == 1) return(cpu.h.al);
   cpu.h.ah = 0x0e;
   cpu.h.dl = disk;
   intdos(&cpu, &cpu);
   cpu.h.ah = 0x19;
   intdos(&cpu, &cpu);
   if(cpu.h.al == disk) ret = disk;
   else ret = -1;
   cpu.h.ah = 0x0e;
   cpu.h.dl = save;
   intdos(&cpu, &cpu);
   return(ret);
}

#ifdef PROG
#ifdef GCC_MAIN
#include <setjmp.h>
jmp_buf e_j_buf;
#endif
int e_p_stdout = -1, e_p_stderr = -1, e_p_out_file, e_p_err_file;

int e_exec_inf(FENSTER *f, char **argv, int n)
{
   char *sp, *s_tmp = NULL, w_tmp[128], e_tmp[128];
   int i;
#ifdef DEBUGGER
   if(e_d_swtch > 0) e_d_quit(f);
#endif
   sprintf(w_tmp, "%s/we_%d.111", getenv("GO32TMP"), getpid());
   if(e_p_stdout < 0) e_p_stdout = dup(1);
   remove(w_tmp);
   e_p_out_file = open(w_tmp, O_CREAT);
   if(e_p_out_file < 0) return(0);
   dup2(e_p_out_file, 1);
   
   sprintf(e_tmp, "%s/we_%d.112", getenv("GO32TMP"), getpid());
   if(e_p_stderr < 0) e_p_stderr = dup(2);
   remove(e_tmp);
   e_p_err_file = open(e_tmp, O_CREAT);
   if(e_p_err_file < 0) return(0);
   dup2(e_p_err_file, 2);
#ifdef GCC_MAIN
   i = setjmp(e_j_buf);
   if(!i) gcc_main(n, argv);
#else
   for(i = 0; i < n && argv[i] != NULL; i++)
   {  write(e_p_err_file, argv[i], strlen(argv[i]));
      write(e_p_err_file, " ", 1);
   }
   write(e_p_err_file, "\n", 1);
   s_tmp = MALLOC((strlen(argv[0])+1)*sizeof(char));
   strcpy(s_tmp, argv[0]);
   sp = argv[0];
   argv[0] = s_tmp;
   if(spawnvp(P_WAIT, s_tmp, argv)) e_print_arg(stderr, e_p_msg[ERR_IN_COMMAND], argv, n);
   FREE(s_tmp);
   argv[0] = sp;
/*
   if(system(s_tmp)) e_print_arg(stderr, e_p_msg[ERR_IN_COMMAND], argv, n);
   if(s_tmp) free(s_tmp);
*/
#endif
   dup2(e_p_stdout, 1);
   dup2(e_p_stderr, 2);

   close(e_p_out_file);
   close(e_p_err_file);
   if((wfildes[0] = open(w_tmp, O_RDONLY)) < 0 )
   {  e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb); return(1);  }
   if((efildes[0] = open(e_tmp, O_RDONLY)) < 0 )
   {  e_error(e_p_msg[ERR_PIPEOPEN], 0, f->fb); return(1);  }
   return(1);
}

#endif

/*	catch/handle hardware exceptions */

static char *err_msg[] = {
   "Disk is Write-Protected",
   "Unknown Unit",
   "Drive not ready",
   "Unknown Command",
   "Data-Error (CRC)",
   "rong Length (Request-Struktur)",
   "Search-Error",
   "Unknown Medium-Typ",
   "Sektor not found",
   "Printer got no Paper",
   "Write-Error",
   "Read-Error",
   "General Error",
   "Unknown Error",
   "Unknown Error",
   "Invallid Disk-Change"
};

int e_h_error()
{
   static char msg[200];
   extern struct EXT h_error;
   int drive;
   int errorno;
   drive = e_h_ax & 0x00FF;
   errorno = e_h_di & 0x00FF;
   
   sprintf(msg, "Error at Drive %c: %s", 'A'+drive, err_msg[errorno]);
   
   e_error(msg, 0, h_error.cn->fb);
   return(-1);
}

_go32_dpmi_registers regs;
_go32_dpmi_seginfo info_24, old_vec_24, info_23, old_vec_23;

int e_d_handler(_go32_dpmi_registers *r)
{
   extern struct EXT h_error;
   e_h_ax = r->x.ax;
   e_h_di = r->x.di;
   h_error.sw = 1;
   regs.x.flags = r->x.flags= 0xffff;
   regs.h.dl = r->h.dl = 0x3;
   return 1;
}

int harderr(int (*hnd)(_go32_dpmi_registers *r))
{
   _go32_dpmi_get_real_mode_interrupt_vector(0x24, &old_vec_24);
   info_24.pm_offset = (u_long) hnd;
   _go32_dpmi_allocate_real_mode_callback_iret(&info_24, &regs);
   _go32_dpmi_set_real_mode_interrupt_vector(0x24, &info_24);
   return(0);
}

int reset_harderr(void)
{
   _go32_dpmi_set_real_mode_interrupt_vector(0x24, &old_vec_24);
   _go32_dpmi_free_real_mode_callback(&info_24);
   return(0);
}

int e_t_sys_ini()
{
/*   _go32_want_ctrl_break(0);   */
   reset_harderr();
   e_refresh();
   return(0);
}

int e_t_sys_end()
{
   extern struct EXT h_error;
/*   _go32_want_ctrl_break(1);    */
   harderr(e_d_handler);
   e_chdir(h_error.cn->dirct);
   e_abs_refr();
   fk_locate(0, 0);
}

int e_switch_screen(int sw)
{
   extern struct EXT h_error;
   int i, j, x, y, g[4];
   static int out_cur_x, out_cur_y, save_sw = 32000;
   static PIC *outp = NULL;
   PIC *tpic;
   if(save_sw == sw) return(0);
   save_sw = sw;
   if(!sw)
   {  g[0] = 2;
      fk_mouse(g);
   }
   tpic = e_open_view(0,0,MAXSCOL-1,MAXSLNS-1, 7, 2);
   ScreenGetCursor(&y, &x);  /*  ????   */
   if(outp)
   {  e_close_view(outp, 1);
      fk_t_locate(out_cur_x, out_cur_y);
   }
   if(sw)
   {  g[0] = 1;
      fk_mouse(g);
   }
   outp = tpic;
   out_cur_x = x;
   out_cur_y = y;
   return(sw);
}

int e_deb_out(FENSTER *f)
{
   e_switch_screen(0);
   getch();
   e_switch_screen(1);
   return(0);
}

int e_ctrl_break()
{
   setcbrk(0);
   _go32_want_ctrl_break(1);
   return(0);
}

#undef getenv

struct dj_env {  char *var, *string;  }  **e_djenv = NULL;
int e_djenv_n = 0;

void e_free_djenv()
{
   int i;
   if(e_djenv)
   {  for(i = 0; i < e_djenv_n; i++)
      {  if(e_djenv[i])
         {  if(e_djenv[i]->var) FREE(e_djenv[i]->var);
            if(e_djenv[i]->string) FREE(e_djenv[i]->string);
            FREE(e_djenv[i]->var);
         }
      }
      FREE(e_djenv);
   }
   e_djenv = NULL;
   e_djenv_n = 0;
}

void e_add_djvar(char *var, char *string)
{
   if(!e_djenv) e_djenv = MALLOC(sizeof(struct dj_env *));
   else e_djenv = REALLOC(e_djenv, (e_djenv_n+1) * sizeof(struct dj_env *));
   if(!e_djenv) return;
   e_djenv[e_djenv_n] = MALLOC(sizeof(struct dj_env));
   if(!e_djenv[e_djenv_n]) return;
   e_djenv[e_djenv_n]->var = MALLOC((strlen(var)+1) * sizeof(char));
   e_djenv[e_djenv_n]->string = MALLOC((strlen(string)+1) * sizeof(char));
   if(!e_djenv[e_djenv_n]->var || !e_djenv[e_djenv_n]->var) return;
   strcpy(e_djenv[e_djenv_n]->var, var);
   strcpy(e_djenv[e_djenv_n]->string, string);
   e_djenv_n++;
}
   
char *e_getdjenv(char *var)
{
   int i;
   for(i = 0; i < e_djenv_n; i++)
      if(!strcmp(e_djenv[i]->var, var)) return(e_djenv[i]->string);
   return(NULL);
}

char *e_getdj_var(char *var, char *str, int n)
{
   char *tp;
   int i, j, k, len;
   for(i = 0; var[i] && !isalnum1(var[i]); i++);
   if(!(tp = e_getdjenv(var+i))) tp = getenv(var+i);
   if(!tp) return(NULL);
   for(j = 0; j < n && (str[j] = tp[j]); j++);
   if(j == n) str[--j] = '\0';
   len = j;
   for(k = 0; k < i; k++)
   {  if(var[k] == ':')
      {  for(j = len; j >= 0 && str[j] != '/' && str[j] != '\\'; j--);
         if(j <= 0 || str[j-1] == ':') j++;
         str[j] = '\0';
      }
      else if(var[k] == ';' && len < n-1)
      {  str[len] = ';';
         str[++len] = '\0';
      }
      else if(var[k] == '/')
      {  for(j = 0; str[j]; j++)
            if(str[j] == '\\') str[j] = '/';
      }
      else if(var[k] == '\\')
      {  for(j = 0; str[j]; j++)
            if(str[j] == '/') str[j] = '\\';
      }
      else if(var[k] == '<')
      {  for(j = 0; (str[j] = toupper(str[j])); j++);
      }
      else if(var[k] == '>')
      {  for(j = 0; (str[j] = tolower(str[j])); j++);
      }
   }
   return(str);
}

void e_read_djenv()
{
   char *tp, var[128], string[256], str[256], tmp[128];
   FILE *fp;
   int i, j, k;
   if(!(tp = getenv("DJGPP"))) return;
   if(!(fp = fopen(tp, "r"))) return;
   if(e_djenv) e_free_djenv();
   while(fgets(str, 256, fp))
   {  for(i = 0; str[i] && !isalnum1(str[i]); i++);
      for(j = 0; isalnum1(str[i+j]) && (var[j] = str[i+j]); j++);
      if(str[i+j] != '=') continue;
      var[j] = '\0';
      if(i > 0 && str[i-1] == '+' && (tp = getenv(var))) continue;
      for(i += j + 1, j = 0; str[i] && str[i] != '\n'; i++, j++)
      {  if(str[i] == '%')
         {  for(k = 0; (tmp[k] = str[i+1+k]) != '%' && tmp[k]; k++);
            tmp[k] = '\0';
            if(e_getdj_var(tmp, string+j, 256-j)) j = strlen(string) - 1;
            else j--;
            i = i + k + 1;
         }
         else string[j] = str[i];
      }
      string[j] = '\0';
      e_add_djvar(var, string);
   }
}

char *e_getenv(char *var)
{
   char *tp;
   if(!e_djenv) e_read_djenv();
   tp = e_getdjenv(var);
   if(!tp) tp = getenv(var);
   return(tp);
}         

/* find attached drives */

struct dirfile *e_mk_drives(void)
{
   struct dirfile *df = MALLOC(sizeof(struct dirfile));
   int drvs, i;
   df->name = MALLOC(sizeof(char *) * 26);
   df->anz = 0;
   for ( i = 0; i < 26; i++)
   {  if(fk_lfw_test(i) >= 0)
      {  *(df->name + df->anz) = MALLOC(2 * sizeof(char));
	 *(*(df->name + df->anz)) = 'A'+i;
	 *(*(df->name + df->anz)+1) = '\0';
	 (df->anz)++;
      }
   }
   return(df);
}

#endif  /*  is DJGPP  */

