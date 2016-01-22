/* we_unix.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "model.h"		/* exchange for D.S.  */

#ifdef XWPE_DLL
#include <dlfcn.h>
#else
int WpeXtermInit(int *argc, char **argv);
int WpeTermInit(int *argc, char **argv);
#endif

#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <signal.h>
#include <locale.h>
#ifndef TERMCAP
#ifndef DJGPP
#include<curses.h>
#endif
#endif

#include "edit.h"		/*   exchange for D.S.  */
#include "attrb.h"

#ifdef NOSYMLINKS
#define lstat(x,y)  stat(x,y)
#undef S_ISLNK
#define S_ISLNK(x)  0
#endif


char *schirm = NULL;
char e_we_sw = 0;

void WpeSignalUnknown(int sig);
void WpeSignalChild(int sig);

void (*WpeMouseChangeShape)(WpeMouseShape new_shape);
void (*WpeMouseRestoreShape)(void);
void (*WpeDisplayEnd)(void);
int (*fk_u_locate)(int x, int y);
int (*fk_u_cursor)(int x);
int (*e_u_initscr)(int argc, char *argv[]);
int (*fk_u_putchar)(int c);
int (*u_bioskey)(void);
int (*e_frb_u_menue)(int sw, int xa, int ya, FENSTER *f, int md);
COLOR (*e_s_u_clr)(int f, int b);
COLOR (*e_n_u_clr)(int fb);
void (*e_pr_u_col_kasten)(int xa, int ya, int x, int y, FENSTER *f, int sw);
int (*fk_mouse)(int g[]);
int (*e_u_refresh)(void);
int (*e_u_getch)(void);
int (*e_u_sys_ini)(void);
int (*e_u_sys_end)(void);
int (*e_u_system)(const char *exe);
int (*e_make_urect)(int xa, int ya, int xe, int ye, int sw);
int (*e_make_urect_abs)(int xa, int ya, int xe, int ye, int sw);
int (*e_u_d_switch_out)(int sw);
int (*e_u_switch_screen)(int sw);
int (*e_u_deb_out)(struct FNST *f);
int (*e_u_cp_X_to_buffer)(struct FNST *f);
int (*e_u_copy_X_buffer)(struct FNST *f);
int (*e_u_paste_X_buffer)(struct FNST *f);
int (*e_u_kbhit)(void);
int (*e_u_change)(PIC *pic);
int (*e_u_ini_size)(void);
int (*e_get_pic_urect)(int xa, int ya, int xe, int ye, struct PICSTR *pic);
int (*e_u_s_sys_end)(void);
int (*e_u_s_sys_ini)(void);
void (*e_u_setlastpic)(PIC *pic);

FARBE *u_fb, *x_fb;

char MCI, MCA, RD1, RD2, RD3, RD4, RD5, RD6, WBT;
char RE1, RE2, RE3, RE4, RE5, RE6;
char *ctree[5];
int MENOPT = 8;
int e_mn_men = 3;

int MAXSLNS = 24;
int MAXSCOL = 80;
int col_num = 0;
char *att_no;
struct termios otermio, ntermio, ttermio;
int cur_x = -1, cur_y = -1;
void *libxwpe;

#ifdef NEWSTYLE
char *extbyte = NULL, *altextbyte = NULL;
#endif
char *altschirm = NULL;
PIC *e_X_l_pic = NULL;

void WpeNullFunction(void)
{
}

int WpeZeroFunction(void)
{
 return(0);
}

int e_ini_unix(int *argc, char **argv)
{
 extern OPT opt[];
 int i, debug;
 struct sigaction act;
 int (*initfunc)(int *argc, char **argv);

 setlocale(LC_ALL, "");
 u_fb = NULL;
 x_fb = NULL;
 debug = 0;
 for (i = 1; i < *argc; i++)
 {
  if (strcmp("--debug", argv[i]) == 0)
  {
   debug = 1;
  }
 }
 for (i = strlen(argv[0])-1; i >= 0 && *(argv[0] + i) != DIRC; i--)
  ;
#ifndef NO_XWINDOWS
 if (*(argv[0]+i+1) == 'x')
  e_we_sw = 1;
 else
  e_we_sw = 0;
#endif
#ifdef PROG
 if (!strncmp("wpe", (argv[0] + i + e_we_sw + 1), 3))
  e_we_sw |= 2;
#endif
#ifdef XWPE_DLL
 if (WpeIsXwin())
 {
  libxwpe = dlopen(LIBRARY_DIR"/libxwpe-x11.so", RTLD_NOW);
 }
 else
 {
  libxwpe = dlopen(LIBRARY_DIR"/libxwpe-term.so",RTLD_NOW);
 }
 if (!libxwpe)
 {
  printf("%s\n", dlerror());
  exit(0);
 }
 initfunc = dlsym(libxwpe, "WpeDllInit");
 if (initfunc)
 {
  (*initfunc)(argc, argv);
 }
 else
 {
  printf("%s\n", dlerror());
  exit(0);
 }
#else
#ifndef NO_XWINDOWS
 if (WpeIsXwin())
 {
  WpeXtermInit(argc, argv);
 }
 else
#endif
 {
  WpeTermInit(argc, argv);
 }
#endif
 if (WpeIsProg())
 {
#ifndef DEBUGGER
  opt[0].x = 2, opt[1].x = 7, opt[2].x = 14; opt[3].x = 21; opt[4].x = 30;
  opt[9].t = opt[7].t; opt[9].x = 74; opt[9].s = opt[7].s; opt[9].as = opt[7].as;
  opt[8].t = opt[6].t; opt[8].x = 65; opt[8].s = opt[6].s; opt[8].as = opt[6].as;
  opt[7].t = opt[5].t; opt[7].x = 55; opt[7].s = opt[5].s; opt[7].as = opt[5].as;
  opt[6].t = "Project"; opt[6].x = 45; opt[6].s = 'P'; opt[6].as = AltP;
  opt[5].t = "Run"; opt[5].x = 38; opt[5].s = 'R'; opt[5].as = AltR;
  MENOPT = 10;
  e_mn_men = 2;
#else
  opt[0].x = 2, opt[1].x = 6, opt[2].x = 12; opt[3].x = 18; opt[4].x = 26;
  opt[10].t = opt[7].t; opt[10].x = 74; opt[10].s = opt[7].s; opt[10].as = opt[7].as;
  opt[9].t = opt[6].t; opt[9].x = 65; opt[9].s = opt[6].s; opt[9].as = opt[6].as;
  opt[8].t = opt[5].t; opt[8].x = 56; opt[8].s = opt[5].s; opt[8].as = opt[5].as;
  opt[7].t = "Project"; opt[7].x = 47; opt[7].s = 'P'; opt[7].as = AltP;
  opt[6].t = "Debug"; opt[6].x = 40; opt[6].s = 'D'; opt[6].as = AltD;
  opt[5].t = "Run"; opt[5].x = 34; opt[5].s = 'R'; opt[5].as = AltR;
  MENOPT = 11;
  e_mn_men = 1;
#endif
 }

 /* Unknown error signal handling */
 act.sa_handler = WpeSignalUnknown;
 sigfillset(&act.sa_mask); /* Mask all signals while running */
 act.sa_flags = 0;
 if (!debug)
 {
  sigaction(SIGQUIT, &act, NULL);
  sigaction(SIGILL, &act, NULL);
  sigaction(SIGABRT, &act, NULL);
  sigaction(SIGFPE, &act, NULL);
  sigaction(SIGSEGV, &act, NULL);
#ifdef SIGTRAP
  sigaction(SIGTRAP, &act, NULL);
#endif
#ifdef SIGIOT
  sigaction(SIGIOT, &act, NULL);
#endif
#ifdef SIGBUS
  sigaction(SIGBUS, &act, NULL);
#endif
 }
 /* Ignore SIGINT */
 act.sa_handler = SIG_IGN;
 sigaction(SIGINT, &act, NULL);
 /* Catch SIGCHLD */
 act.sa_handler = WpeSignalChild;
 act.sa_flags = SA_NOCLDSTOP;
 sigaction(SIGCHLD, &act, NULL);
 return(*argc);
}

int e_abs_refr()
{
 extern char *altschirm;
 int i;

 for(i = 0; i < 2 * MAXSCOL * MAXSLNS; i++)
  altschirm[i] = 0;
 return(0);
}

void e_refresh_area(int x, int y, int width, int height)
{
 extern char *altschirm;
 char *curloc;
 int i,j;

 if (width + x > MAXSCOL)
 {
  width = MAXSCOL - x;
 }
 if (height + y > MAXSLNS)
 {
  height = MAXSLNS - y;
 }
 curloc = altschirm + ((x + (y * MAXSCOL)) * 2);
 for (j = 0; j < height; j++, curloc += MAXSCOL * 2)
 {
  for (i = 0; i < width; i ++)
  {
   curloc[i * 2] = 0;
   curloc[i * 2 + 1] = 0;
  }
 }
}

int e_tast_sim(int c)
{
 if (c >= 'A' && c <= 'Z')
  return(c + 1024 - 'A');
 switch(c)
 {
  case 'a':  return(AltA);
  case 'b':  return(AltB);
  case 'c':  return(AltC);
  case 'd':  return(AltD);
  case 'e':  return(AltE);
  case 'f':  return(AltF);
  case 'g':  return(AltG);
  case 'h':  return(AltH);
  case 'i':  return(AltI);
  case 'j':  return(AltJ);
  case 'k':  return(AltK);
  case 'l':  return(AltL);
  case 'm':  return(AltM);
  case 'n':  return(AltN);
  case 'o':  return(AltO);
  case 'p':  return(AltP);
  case 'q':  return(AltQ);
  case 'r':  return(AltR);
  case 's':  return(AltS);
  case 't':  return(AltT);
  case 'u':  return(AltU);
  case 'v':  return(AltV);
  case 'w':  return(AltW);
  case 'x':  return(AltX);
  case 'y':  return(AltY);
  case 'z':  return(AltZ);
  case '1':  return(Alt1);
  case '2':  return(Alt2);
  case '3':  return(Alt3);
  case '4':  return(Alt4);
  case '5':  return(Alt5);
  case '6':  return(Alt6);
  case '7':  return(Alt7);
  case '8':  return(Alt8);
  case '9':  return(Alt9);
  case '0':  return(Alt0);
  case ' ':  return(AltBl);
  case '#':  return(AltSYS);
  case CtrlA:  return(CBUP);
  case CtrlE:  return(CBDO);
  case CtrlB:  return(CCLE);
  case CtrlF:  return(CCRI);
  case CtrlP:  return(CPS1);
  case CtrlN:  return(CEND);
  case CtrlH:  return(AltBS);
  default:  return(0);
 }
}

void WpeSignalUnknown(int sig)
{
/*    psignal(sig, "Xwpe");   */
 printf("Xwpe: unexpected signal %d, exiting ...\n", sig);
 e_exit(1);
}

void WpeSignalChild(int sig)
{
 int statloc;

 wait(&statloc);
}

static int e_bool_exit = 0;

void e_err_save()
{
 ECNT *cn = WpeEditor;
 int i;
 long maxname;
 FENSTER *f;
 BUFFER *b;

 /* Quick fix to multiple emergency save problems */
 if (e_bool_exit)
  return ;
 e_bool_exit = 1;
 for (i = 0; i <= cn->mxedt; i++)
 {
  if (DTMD_ISTEXT(cn->f[i]->dtmd))
  {
   f = cn->f[i];
   b = cn->f[i]->b;
   if (b->mxlines > 1 || b->bf[0].len > 0)
   {
    /* Check if file system could have an autosave or emergency save file
       >12 check is to eliminate dos file systems */
    if ((maxname = pathconf(f->dirct, _PC_NAME_MAX) >= strlen(f->datnam) + 4) &&
      (maxname > 12))
    {
     strcat(f->datnam, ".ESV");
     printf("Try to save %s!\n", f->datnam);
     if (!e_save(f))
      printf("File %s saved!\n", f->datnam);
    }
   }
  }
 }
}

void e_exit(int n)
{
#ifdef DEBUGGER
 extern int e_d_pid;

 if (e_d_pid)
  kill(e_d_pid, 7);
#endif
 (*WpeDisplayEnd)();
 e_switch_screen(0);
 if (n != 0)
 {
  printf("\nError-Exit!   Code: %d!\n", n);
  e_err_save();
 }
 exit(n);
}

char *e_mkfilepath(char *dr, char *fn, char *fl)
{
 strcpy(fl, dr);
 if (dr[strlen(dr)-1] != DIRC)
 {
  strcat(fl, DIRS);
 }
 strcat(fl, fn);
 return(fl);
}

int e_compstr(char *a, char *b)
{
 int n, k;
 char *ctmp, *cp;

 if (a[0] == '*' && !a[1])
  return(0);
 if (!a[0] || !b[0])
  return(a[0] - b[0]);
 if (a[0] == '*' && a[1] == '*')
  return(e_compstr(++a, b));
 for (n = a[0] == '*' ? 2 : 1;
   a[n] != '*' && a[n] != '?' && a[n] != '[' && a[n]; n++)
  ;
 if (a[0] == '*')
 {
  n--;
  a++;
  if (a[0] == '?')
  {
   cp = MALLOC((strlen(a)+1)*sizeof(char));
   strcpy(cp, a);
   cp[0] = '*';
   n = e_compstr(cp, ++b);
   FREE(cp);
   return(n);
  }
  else if (a[0] == '[')
  {
   while (*b && (n = e_compstr(a, b)))
    b++;
   return(n);
  }
  ctmp = MALLOC(n+1);
  for (k = 0; k < n; k++)
   ctmp[k] = a[k];
  ctmp[n] = '\0';
  cp = strstr(b, ctmp);
  FREE(ctmp);
  if (cp == NULL)
   return((a[0] - b[0]) ? a[0] - b[0] : -1);
  if (!a[n] && !cp[n])
   return(0);
  if (!a[n])
   return(e_compstr(a-1, cp+1));
  if (!(k = e_compstr(a+n, cp+n)))
   return(0);
  return(e_compstr(a-1, cp+1));
 }
 else if (a[0] == '?')
 {
  n--;  a++;  b++;
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
   n-=(k+1);  a+=(k+1);  b++;
  }
  else
  {
   for (k = 1; a[k] && (a[k] != ']' || k == 1) && a[k] != b[0]; k++)
    if (a[k+1] == '-' && b[0] >= a[k] && b[0] <= a[k+2])
     break;
   if (a[k] == ']' || a[k] == '\0')
    return(-b[0]);
   for (; a[k] && (a[k] != ']'); k++)
    ;
   n-=(k+1);  a+=(k+1);  b++;
  }
 }
 if (n <= 0)
  return(e_compstr(a, b));
 if ((k = strncmp(a, b, n)) != 0)
  return(k);
 return(e_compstr(a+n, b+n));
}

struct dirfile *e_find_files(char *sufile, int sw)
{
 char           *stmp, *tmpst, *sfile, *sdir;
 struct dirfile *df = MALLOC(sizeof(struct dirfile));
 DIR            *dirp;
 struct dirent  *dp;
 struct stat     buf;
 struct stat     lbuf;
 int             i, n, cexist, sizeStmp, sizeSdir;

 df->name = NULL;
 df->anz = 0;
 for (n = strlen(sufile); n >= 0 && sufile[n] != DIRC; n--);
 sfile = sufile + 1 + n;
 if (n <= 0)
 {
  sizeSdir = 2;
  sdir = (char *)WpeMalloc(2 * sizeof(char));
  sdir[0] = n ? '.' : DIRC;
  sdir[1] = '\0';
 }
 else
 {
  sizeSdir = n + 1;
  sdir = (char *)WpeMalloc((n + 1) * sizeof(char));
  for (i = 0; i < n; i++)
   sdir[i] = sufile[i];
  sdir[n] = '\0';
 }
 if (!(dirp = opendir(sdir)))
 {
  FREE(sdir);
  return(df);
 }
 sizeStmp = 256;
 stmp = (char *)WpeMalloc(sizeStmp);
 while((dp = readdir(dirp)) != NULL)
 {
  if (!(sw & 1) && dp->d_name[0] == '.' && sfile[0] != '.')
   continue;
  if (!e_compstr(sfile, dp->d_name))
  {
   if (sizeSdir + strlen(dp->d_name) + 10 > sizeStmp)
   {
    while (sizeSdir + strlen(dp->d_name) + 10 > sizeStmp)
     sizeStmp <<= 1;
    stmp = (char *)WpeRealloc(stmp, sizeStmp);
   }

   e_mkfilepath(sdir, dp->d_name, stmp);
   lstat(stmp, &lbuf);
   stat(stmp, &buf);

   /* check existence of the file */
   cexist = access(stmp, F_OK);

   /* we accept it as a file if
      - a regular file
      - link and it does not point to anything
      - link and it points to a non-directory */
   if ((S_ISREG(buf.st_mode) ||
     (S_ISLNK(lbuf.st_mode) &&
     (cexist || (cexist == 0 && !S_ISDIR(buf.st_mode))))) &&
     (!(sw & 2) || (buf.st_mode & 0111)) )
   {
    if (df->anz == 0)
     df->name = MALLOC((df->anz + 1) * sizeof(char *));
    else
     df->name = REALLOC(df->name, (df->anz + 1) * sizeof(char *));
    if (df->name == NULL || !(tmpst = MALLOC(strlen(dp->d_name) + 1)))
    {
     df->anz = 0;
     closedir(dirp);
     WpeFree(stmp);
     WpeFree(sdir);
     return(df);
    }
    strcpy(tmpst, dp->d_name);
    for (n = df->anz; n > 0 && strcmp(*(df->name + n - 1), tmpst) > 0; n--)
     *(df->name + n) = *(df->name + n - 1);
    *(df->name + n) = tmpst;
    (df->anz)++;
   }
  }
 }
 closedir(dirp);
 WpeFree(stmp);
 WpeFree(sdir);
 return(df);
}

struct dirfile *e_find_dir(char *sufile, int sw)
{
 char           *stmp, *tmpst, *sfile, *sdir;
 struct dirfile *df = MALLOC(sizeof(struct dirfile));
 DIR            *dirp;
 struct dirent  *dp;
 struct stat     buf;
 int             i, n, sizeStmp, sizeSdir;

 df->name = NULL;
 df->anz = 0;
 for (n = strlen(sufile); n >= 0 && sufile[n] != DIRC; n--);
 sfile = sufile + 1;
 sfile = sfile + n;
 if (n <= 0)
 {
  sizeSdir = 2;
  sdir = MALLOC(2 * sizeof(char));
  sdir[0] = n ? '.' : DIRC;
  sdir[1] = '\0';
 }
 else
 {
  sizeSdir = n + 1;
  sdir = MALLOC((n + 1) * sizeof(char));
  for (i = 0; i < n; i++)
   sdir[i] = sufile[i];
  sdir[n] = '\0';
 }
 if (!(dirp = opendir(sdir)))
 {
  FREE(sdir);
  return(df);
 }
 sizeStmp = 256;
 stmp = (char *)WpeMalloc(sizeStmp);
 while ((dp = readdir(dirp)) != NULL)
 {
  if (!sw && dp->d_name[0] == '.' && sfile[0] != '.')
   continue;
  if (!e_compstr(sfile, dp->d_name) && strcmp(dp->d_name, ".") &&
    strcmp(dp->d_name, ".."))
  {
   if (sizeSdir + strlen(dp->d_name) + 10 > sizeStmp)
   {
    while (sizeSdir + strlen(dp->d_name) + 10 > sizeStmp)
     sizeStmp <<= 1;
    stmp = (char *)WpeRealloc(stmp, sizeStmp);
   }
   stat(e_mkfilepath(sdir, dp->d_name, stmp), &buf);

   /* we accept _only_ real, existing directories */
   if (S_ISDIR(buf.st_mode))
   {

    if (df->anz == 0)
     df->name = MALLOC((df->anz + 1) * sizeof(char *));
    else
     df->name = REALLOC(df->name, (df->anz + 1) * sizeof(char *));
    if (df->name == NULL || !(tmpst = MALLOC(strlen(dp->d_name) + 1)))
    {
     df->anz = 0;
     closedir(dirp);
     FREE(sdir);
     WpeFree(stmp);
     return(df);
    }
    strcpy(tmpst, dp->d_name);
    for (n = df->anz; n > 0 && strcmp(*(df->name + n - 1), tmpst) > 0; n--)
     *(df->name + n) = *(df->name + n - 1);
    *(df->name + n) = tmpst;
    (df->anz)++;
   }
  }
 }
 closedir(dirp);
 FREE(sdir);
 WpeFree(stmp);
 return(df);
}

#include <time.h>

char *e_file_info(char *filen, char *str, int *num, int sw)
{
 struct tm *ttm;
 struct stat buf[1];

 stat(filen, buf);
 ttm = localtime(&(buf->st_mtime));
 sprintf(str, "%c%c%c%c%c%c%c%c%c%c  %-13s  %6ld  %2.2u.%2.2u.%4.4u  %2.2u.%2.2u",
   buf->st_mode & 040000 ? 'd' : '-',
   buf->st_mode & 0400 ? 'r' : '-',
   buf->st_mode & 0200 ? 'w' : '-',
   buf->st_mode & 0100 ? 'x' : '-',
   buf->st_mode & 040 ? 'r' : '-',
   buf->st_mode & 020 ? 'w' : '-',
   buf->st_mode & 010 ? 'x' : '-',
   buf->st_mode & 04 ? 'r' : '-',
   buf->st_mode & 02 ? 'w' : '-',
   buf->st_mode & 01 ? 'x' : '-',
   filen,
   buf->st_size, ttm->tm_mday,
   ttm->tm_mon + 1, ttm->tm_year + 1900, ttm->tm_hour, ttm->tm_min);
 if (sw & 1) *num = buf->st_mtime;
 else if (sw & 2) *num = buf->st_size;
 return(str);
}

void ini_repaint(ECNT *cn)
{
 e_cls(cn->fb->df.fb, cn->fb->dc);
 e_ini_desk(cn);
}

void end_repaint()
{
   e_refresh();
}

int e_recover(ECNT *cn)
{
 struct dirfile *files;
 FENSTER *f = NULL;
 BUFFER *b;
 SCHIRM *s;
 int i;

 files = e_find_files("*.ESV", 1);
 for (i = 0; i < files->anz; i++)
 {
  e_edit(cn, files->name[i]);
  f = cn->f[cn->mxedt];
  f->datnam[strlen(f->datnam)-4] = '\0';
  if (!strcmp(f->datnam, BUFFER_NAME))
  {
   s = cn->f[cn->mxedt]->s;
   b = cn->f[cn->mxedt]->b;
   s->mark_end.y = b->mxlines - 1;
   s->mark_end.x = b->bf[b->mxlines-1].len;
   e_edt_copy(f);
   e_close_window(f);
  }
  else f->save = 1;
#ifdef PROG
  if (WpeIsProg()) e_add_synt_tl(f->datnam, f);
#endif
  if ((f->ed->edopt & ED_ALWAYS_AUTO_INDENT) ||
    ((f->ed->edopt & ED_SOURCE_AUTO_INDENT) && f->c_st)) 
   f->flg = 1;
 }
 freedf(files);
 return(0);
}

int e_frb_t_menue(int sw, int xa, int ya, FENSTER *f, int md)
{
 COLOR *frb = &(f->fb->er);
 int i, j, y, c=1, fb, fsv;

 if (md == 1) sw += 11;
 else if (md == 2) sw += 16;
 else if (md == 3) sw += 32;
 fsv = fb = frb[sw].fb;
 if (fb == 0) y = 0;
 else
  for (y = 1, j = fb; j > 1; y++)
   j /= 2;
 do
 {
  if (c == CDO) y = y < 6 ? y + 1 : 0;
  else if (c == CUP) y = y > 0 ? y - 1 : 6;
  if (y == 0) fb = 0;
  else
   for (i = 1, fb = 1; i < y; i++)
    fb *= 2;
  frb[sw] = e_n_clr(fb);
  e_pr_t_col_kasten(xa, ya, fb, fb, f, 1);
  e_pr_ed_beispiel(1, 2, f, sw, md);
#if  MOUSE
  if ((c=e_getch()) == -1) c = e_opt_ck_mouse(xa, ya, md);
#else
  c = e_getch();
#endif
 } while (c != WPE_ESC && c != WPE_CR && c > -2);
 if (c == WPE_ESC || c < -1) frb[sw] = e_n_clr(fsv);
 return(frb[sw].fb);
}

/*   draw colors box  */
void e_pr_t_col_kasten(int xa, int ya, int x, int y, FENSTER * f, int sw)
{
 int rfrb, xe = xa + 14, ye = ya + 8;

 if (x == 0) y = 0;
 else
  for (rfrb = x, y = 1; rfrb > 1; y++)
   rfrb /= 2;
 rfrb = sw == 0 ? f->fb->nt.fb : f->fb->fs.fb;
 e_std_rahmen(xa, ya, xe, ye, "Colors", 0, rfrb, 0);
/*     e_pr_str((xa+xe-8)/2, ya, "Colors", rfrb, 0, 1, 
                                        f->fb->ms.f+16*(rfrb/16), 0);
*/
 e_pr_nstr(xa+2, ya+1, xe-xa-1, "A_NORMAL   ", 0, 0);
 e_pr_nstr(xa+2, ya+2, xe-xa-1, "A_STANDOUT ", A_STANDOUT, A_STANDOUT);
 e_pr_nstr(xa+2, ya+3, xe-xa-1, "A_UNDERLINE", A_UNDERLINE, A_UNDERLINE);
 e_pr_nstr(xa+2, ya+4, xe-xa-1, "A_REVERSE  ", A_REVERSE, A_REVERSE);
 e_pr_nstr(xa+2, ya+5, xe-xa-1, "A_BLINK    ", A_BLINK, A_BLINK);
 e_pr_nstr(xa+2, ya+6, xe-xa-1, "A_DIM      ", A_DIM, A_DIM);
 e_pr_nstr(xa+2, ya+7, xe-xa-1, "A_BOLD     ", A_BOLD, A_BOLD);

 fk_locate(xa+4, ya + y + 1);
}

