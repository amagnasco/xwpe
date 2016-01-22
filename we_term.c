/* we_term.c                                              */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

#include "edit.h"
#ifdef UNIX

#include <termios.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include<signal.h>
#define KEYFN 42
#define NSPCHR 13

#ifndef XWPE_DLL
#define WpeDllInit WpeTermInit
#endif

/*    we_term.c    */
char *init_key(char *key);
char *init_kkey(char *key);
int init_cursor(void);
int e_begscr(void);
void e_endwin(void);
int fk_t_cursor(int x);
int fk_t_putchar(int c);
int fk_attrset(int a);
int e_t_refresh(void);
int e_t_sys_ini(void);
int e_t_sys_end(void);
int e_t_getch(void);
int e_find_key(int c, int j, int sw);
void e_exitm(char *s, int n);
int fk_t_locate(int x, int y);
int fk_t_mouse(int *g);
int e_t_initscr(void);
int e_t_kbhit(void);
int e_t_d_switch_out(int sw);
int e_t_switch_screen(int sw);
int e_t_deb_out(FENSTER *f);
int e_s_sys_end();
int e_s_sys_ini();

extern int MAXSLNS;
extern int MAXSCOL;


#ifndef NCURSES
char *key_f[KEYFN], *key_key;
#endif
char *cur_rc, *cur_vs, *cur_nvs, *cur_vvs, cur_attr;
char *att_so, *att_ul, *att_rv, *att_bl, *att_dm, *att_bo;
char *ratt_no, *ratt_so, *ratt_ul, *ratt_rv, *ratt_bl, *ratt_dm, *ratt_bo;
char *beg_scr, *swt_scr, *sav_cur, *res_cur; 
extern char *altschirm;
extern char *att_no;
char *col_fg, *col_bg, *spc_st, *spc_in, *spc_bg, *spc_nd;

extern int col_num;
#ifdef NCURSES
chtype sp_chr[NSPCHR];
#else
char *sp_chr[NSPCHR];
#endif

extern int cur_x, cur_y;
extern struct termios otermio, ntermio, ttermio;
#ifdef TERMCAP
char area[315];
char *ap = area;
char tcbuf[1024];
char *tc_screen;
char *tgetstr();
int tgetnum();
char *tgoto();
#define tigetstr(k) tgetstr(k, &ap)

#define term_move(x,y) e_putp(tgoto(cur_rc, x, y))
#define term_refresh() fflush(stdout)
#else
/* Because term.h defines "buttons" it messes up we_gpm.c if in edit.h */
#include <term.h>

/* AIX requires that tparm has 10 arguments. */
#define tparm1(a,b) tparm((a), (b), 0, 0, 0, 0, 0, 0, 0, 0)
#define tparm2(a,b,c) tparm((a), (b), (c), 0, 0, 0, 0, 0, 0, 0)

#ifdef NCURSES
#define term_move(x,y) move(y, x)
#define term_refresh() refresh()
#else
#define term_move(x,y) e_putp(tparm2(cur_rc, y, x))
#define term_refresh() fflush(stdout)
#endif
#endif

int WpeDllInit(int *argc, char **argv)
{
 fk_u_cursor = fk_t_cursor;
 fk_u_locate = fk_t_locate;
 e_u_d_switch_out = e_t_d_switch_out;
 e_u_switch_screen = e_t_switch_screen;
 e_u_deb_out = e_t_deb_out;
 e_u_kbhit = e_t_kbhit;
 e_u_s_sys_end = e_s_sys_end;
 e_u_s_sys_ini = e_s_sys_ini;
 e_u_refresh = e_t_refresh;
 e_u_getch = e_t_getch;
 e_u_sys_ini = e_t_sys_ini;
 e_u_sys_end = e_t_sys_end;
 e_u_system = system;
 fk_u_putchar = fk_t_putchar;
#ifdef HAVE_LIBGPM
 if (WpeGpmMouseInit() == 0)
 {
  fk_mouse = WpeGpmMouse;
 }
 else
#endif
  fk_mouse = fk_t_mouse;
 WpeMouseChangeShape = (void (*)(WpeMouseShape))WpeNullFunction;
 WpeMouseRestoreShape = WpeNullFunction;
 WpeDisplayEnd = e_endwin;
#ifdef __linux__
 u_bioskey = WpeLinuxBioskey;
#else
 u_bioskey = WpeZeroFunction;
#endif
 e_t_initscr();
 if (col_num > 0)
 {
  e_pr_u_col_kasten = e_pr_x_col_kasten;
  e_frb_u_menue = e_frb_x_menue;
  e_s_u_clr = e_s_x_clr;
  e_n_u_clr = e_n_x_clr;
 }
 else
 {
  e_pr_u_col_kasten = e_pr_t_col_kasten;
  e_frb_u_menue = e_frb_t_menue;
  e_s_u_clr = e_s_t_clr;
  e_n_u_clr = e_n_t_clr;
 }
 MCI = '+';
 MCA = '0';
 RD1 = '\01';
 RD2 = '\02';
 RD3 = '\03';
 RD4 = '\04';
 RD5 = '\05';
 RD6 = '\06';
 RE1 = '\01';
 RE2 = '\02';
 RE3 = '\03';
 RE4 = '\04';
 RE5 = '\05';
 RE6 = '\06';
 WBT = '\13';
 ctree[0] = "\11\12\07";   /*  07 -> 30  */
 ctree[1] = "\11\12\12";	/*  11 -> 16  */
 ctree[2] = "\11\07\12";   /*  12 -> 22  */
 ctree[3] = "\10\12\12";   /*  10 -> 25  */
 ctree[4] = "\11\12\12";
/*
 RD1 = '+';
 RD2 = '+';
 RD3 = '+';
 RD4 = '+';
 RD5 = '-';
 RD6 = '|';
*//*
 RE1 = '.';
 RE2 = '.';
 RE3 = '.';
 RE4 = '.';
 RE5 = '.';
 RE6 = ':';
 WBT = '#';
 ctree[0] = "|__";
 ctree[1] = "|__";
 ctree[2] = "|__";
 ctree[3] = "|__";
 ctree[4] = "|__";
*/
 return 0;
}

char *init_key(char *key)
{
 char *tmp, *keystr;

 tmp = (char *) tigetstr(key);
 if (tmp == NULL || tmp == ((char *) -1))
  return(NULL);
 else
 {
  keystr = MALLOC(strlen(tmp)+1);
  strcpy(keystr, tmp);
 }
 return(keystr);
}

#ifndef NCURSES
char *init_kkey(char *key)
{
 char *tmp;
 int i;

 tmp = init_key(key);
 if (tmp == NULL)
  return(NULL);
 if (!key_key)
 {
  key_key = MALLOC(2);
  key_key[0] = tmp[1];
  key_key[1] = '\0';
  return(tmp);
 }
 else
 {
  for (i = 0; key_key[i] != '\0'; i++)
   if (key_key[i] == tmp[1])
    return(tmp);
  key_key = REALLOC(key_key, i + 2);
  key_key[i] = tmp[1];
  key_key[i + 1] = '\0';
 }
 return(tmp);
}
#endif

char *init_spchr(char c)
{
 int i;
 char *pt = NULL;

 if (!spc_st || !spc_bg || !spc_nd)
  return(NULL);
 for (i = 0; spc_st[i] && spc_st[i+1] && spc_st[i] != c; i+=2)
  ;
 if (spc_st[i] && spc_st[i+1])
 {
  pt = MALLOC((strlen(spc_bg)+strlen(spc_nd)+2)*sizeof(char));
  if(pt)
   sprintf(pt, "%s%c%s", spc_bg, spc_st[i+1], spc_nd);
 }
 return(pt);
}

int init_cursor()
{
#ifndef TERMCAP
   if (!(cur_rc = init_key("cup")))
    return(-1);
   if ((col_fg = init_key("setaf")) && (col_bg = init_key("setab")))
   { /* terminfo 10.2.x color code */
    if ((col_num = tigetnum("colors")) < 0) col_num = 8;
   }
   else if ((col_fg = init_key("setf")) && (col_bg = init_key("setb")))
   { /* Older terminfo color code */
    if((col_num = tigetnum("colors")) < 0) col_num = 8;
   }
   else
   { /* No colors */
    if(col_fg) {  free(col_fg);  col_fg = NULL;   }
    if(col_bg) {  free(col_bg);  col_bg = NULL;   }
   }

   spc_st = init_key("acsc");
   spc_in = init_key("enacs");
   spc_bg = init_key("smacs");
   spc_nd = init_key("rmacs");
   att_no = init_key("sgr0");
   att_so = init_key("smso");
   att_ul = init_key("smul");
   att_rv = init_key("rev");
   att_bl = init_key("blink");
   att_dm = init_key("dim");
   att_bo = init_key("bold");
   ratt_no = init_key("sgr0");
   ratt_so = init_key("rmso");
   ratt_ul = init_key("rmul");
   ratt_rv = init_key("sgr0");
   ratt_bl = init_key("sgr0");
   ratt_dm = init_key("sgr0");
   ratt_bo = init_key("sgr0");
   beg_scr = init_key("smcup");
   swt_scr = init_key("rmcup");
   sav_cur = init_key("sc");
   res_cur = init_key("rc");

#ifndef NCURSES
   key_f[0] = init_kkey("kf1");
   key_f[1] = init_kkey("kf2");
   key_f[2] = init_kkey("kf3");
   key_f[3] = init_kkey("kf4");
   key_f[4] = init_kkey("kf5");
   key_f[5] = init_kkey("kf6");
   key_f[6] = init_kkey("kf7");
   key_f[7] = init_kkey("kf8");
   key_f[8] = init_kkey("kf9");
   key_f[9] = init_kkey("kf10");
   key_f[10] = init_kkey("kcuu1");
   key_f[11] = init_kkey("kcud1");
   key_f[12] = init_kkey("kcub1");
   key_f[13] = init_kkey("kcuf1");
   key_f[14] = init_kkey("kich1");
   key_f[15] = init_kkey("khome");
   key_f[16] = init_kkey("kpp");
   if(!(key_f[17] = init_kkey("kdch1"))) key_f[17] = "\177";
   key_f[18] = init_kkey("kend");
   key_f[19] = init_kkey("knp");
   key_f[20] = init_kkey("kbs");
   key_f[21] = init_kkey("khlp");
   key_f[22] = init_kkey("kll");
   key_f[23] = init_kkey("kf17");
   key_f[24] = init_kkey("kf18");
   key_f[25] = init_kkey("kf19");
   key_f[26] = init_kkey("kf20");
   key_f[27] = init_kkey("kf21");
   key_f[28] = init_kkey("kf22");
   key_f[29] = init_kkey("kf23");
   key_f[30] = init_kkey("kf24");
   key_f[31] = init_kkey("kf25");
   key_f[32] = init_kkey("kf26");
   key_f[33] = init_kkey("kPRV");
   key_f[34] = init_kkey("kNXT");
   key_f[35] = init_kkey("kLFT");
   key_f[36] = init_kkey("kRIT");
   key_f[37] = init_kkey("kHOM");
   key_f[38] = init_kkey("kri");
   key_f[39] = init_kkey("kEND");
   key_f[40] = init_kkey("kind");
   key_f[41] = init_kkey("kext");
#endif

#else

   if(!(cur_rc = init_key("cm"))) return(-1);
   if((col_fg = init_key("Sf")) && (col_bg = init_key("Sb")))
   {  if((col_num = tgetnum("Co")) < 0) col_num = 8;  }
   else
   {  if(col_fg) {  free(col_fg);  col_fg = NULL;   }
      if(col_bg) {  free(col_bg);  col_bg = NULL;   }
   }
   spc_st = init_key("ac");
   spc_in = init_key("eA");
   spc_bg = init_key("as");
   spc_nd = init_key("ae");
   att_no = init_key("me");
   att_so = init_key("so");
   att_ul = init_key("us");
   att_rv = init_key("mr");
   att_bl = init_key("mb");
   att_dm = init_key("mh");
   att_bo = init_key("md");
   ratt_no = init_key("me");
   ratt_so = init_key("se");
   ratt_ul = init_key("ue");
   ratt_rv = init_key("me");
   ratt_bl = init_key("me");
   ratt_dm = init_key("me");
   ratt_bo = init_key("me");
   beg_scr = init_key("ti");
   swt_scr = init_key("te");
   sav_cur = init_key("sc");
   res_cur = init_key("rc");

   key_f[0] = init_kkey("k1");
   key_f[1] = init_kkey("k2");
   key_f[2] = init_kkey("k3");
   key_f[3] = init_kkey("k4");
   key_f[4] = init_kkey("k5");
   key_f[5] = init_kkey("k6");
   key_f[6] = init_kkey("k7");
   key_f[7] = init_kkey("k8");
   key_f[8] = init_kkey("k9");
   key_f[9] = init_kkey("k;");
   key_f[10] = init_kkey("ku");
   key_f[11] = init_kkey("kd");
   key_f[12] = init_kkey("kl");
   key_f[13] = init_kkey("kr");
   key_f[14] = init_kkey("kI");
   key_f[15] = init_kkey("kh");
   key_f[16] = init_kkey("kP");
   key_f[17] = init_kkey("kD");
   key_f[18] = init_kkey("@7");
   key_f[19] = init_kkey("kN");
   key_f[20] = init_kkey("kb");
   key_f[21] = init_kkey("%1");
   key_f[22] = init_kkey("kH");
   key_f[23] = init_kkey("F7");
   key_f[24] = init_kkey("F8");
   key_f[25] = init_kkey("F9");
   key_f[26] = init_kkey("FA");
   key_f[27] = init_kkey("FB");
   key_f[28] = init_kkey("FC");
   key_f[29] = init_kkey("FD");
   key_f[30] = init_kkey("FE");
   key_f[31] = init_kkey("FF");
   key_f[32] = init_kkey("FG");
   key_f[33] = init_kkey("%e");
   key_f[34] = init_kkey("%c");
   key_f[35] = init_kkey("#4");
   key_f[36] = init_kkey("%i");
   key_f[37] = init_kkey("#2");
   key_f[38] = init_kkey("kR");
   key_f[39] = init_kkey("*7");
   key_f[40] = init_kkey("kF");
   key_f[41] = init_kkey("@1");
#endif
#ifdef NCURSES
   sp_chr[0] = 0;
   sp_chr[1] = ACS_ULCORNER;
   sp_chr[2] = ACS_URCORNER;
   sp_chr[3] = ACS_LLCORNER;
   sp_chr[4] = ACS_LRCORNER;
   sp_chr[5] = ACS_HLINE;
   sp_chr[6] = ACS_VLINE;
   sp_chr[7] = ACS_S9;
   sp_chr[8] = ACS_VLINE;
   sp_chr[9] = ACS_VLINE;
   sp_chr[10] = ACS_S9;
   sp_chr[11] = ACS_DIAMOND;
   sp_chr[12] = ' ';
#else
   sp_chr[0] = "";
   if(!(sp_chr[1] = init_spchr('l'))) sp_chr[1] = "+";
   if(!(sp_chr[2] = init_spchr('k'))) sp_chr[2] = "+";
   if(!(sp_chr[3] = init_spchr('m'))) sp_chr[3] = "+";
   if(!(sp_chr[4] = init_spchr('j'))) sp_chr[4] = "+";
   if(!(sp_chr[5] = init_spchr('q'))) sp_chr[5] = "-";
   if(!(sp_chr[6] = init_spchr('x'))) sp_chr[6] = "|";
   if(!(sp_chr[7] = init_spchr('w'))) sp_chr[7] = "_";
   if(!(sp_chr[8] = init_spchr('t'))) sp_chr[8] = "|";
   if(!(sp_chr[9] = init_spchr('m'))) sp_chr[9] = "|";
   if(!(sp_chr[10] = init_spchr('q'))) sp_chr[10] = "_";
   if(!(sp_chr[11] = init_spchr('`'))) sp_chr[11] = "#";
   if(!(sp_chr[12] = init_spchr('a'))) sp_chr[12] = " ";
#endif
   return(0);
}

int e_t_initscr()
{
 int ret, i, k;
#ifndef TERMCAP
 WINDOW * stdscr;
#endif

 ret = tcgetattr(1, &otermio); /* save old settings */
/*
 if(ret)
 {
  printf("Error in Terminal Initialisation Code: %d\n", ret);
  printf("c_iflag = %o, c_oflag = %o, c_cflag = %o,\n",
    otermio.c_iflag, otermio.c_oflag, otermio.c_cflag);
  printf("c_lflag = %o, c_line = %o, c_cc = {\"\\%o\\%o\\%o\\%o\\%o\\%o\\%o\\%o\"}\n",
    otermio.c_lflag, otermio.c_line, otermio.c_cc[0], otermio.c_cc[1], 
    otermio.c_cc[2], otermio.c_cc[3], otermio.c_cc[4], otermio.c_cc[5], 
    otermio.c_cc[6], otermio.c_cc[7]);
  WpeExit(1);
 }
*/
#ifndef TERMCAP
 if ((stdscr=initscr())==(WINDOW *)ERR) exit(27);
#ifdef NCURSES
 cbreak();
 noecho();
 nonl();
 intrflush(stdscr,FALSE);
 keypad(stdscr,TRUE);
#endif
 if (has_colors())
 {
  start_color();
  for (i = 0; i < COLORS; i++)
  {
   for (k = 0; k < COLORS; k++)
   {
    if (i != 0 || k != 0)
    {
     init_pair(i * 8 + k, k, i);
    }
   }
  }
 }
#endif
 e_begscr();
 schirm = MALLOC(2 * MAXSCOL * MAXSLNS);
 altschirm = MALLOC(2 * MAXSCOL * MAXSLNS);
#if !defined(NO_XWINDOWS) && defined(NEWSTYLE)
 extbyte = MALLOC(MAXSCOL * MAXSLNS);
#endif
 e_abs_refr();
 if(init_cursor())
 {
  printf("Terminal Not in the right mode\n");
  e_exit(1);
 }
 tcgetattr(0, &ntermio);
 ntermio.c_iflag = 0;            /* setup new settings */
 ntermio.c_oflag = 0;
 ntermio.c_lflag = 0;
 ntermio.c_cc[VMIN] = 1;
 ntermio.c_cc[VTIME] = 0;
#ifdef VSWTCH
 ntermio.c_cc[VSWTCH] = 0;
#endif
 tcsetattr(0, TCSADRAIN, &ntermio);
 if (spc_in) e_putp(spc_in);
 return(0);
}

int svflgs, kbdflgs;

int e_begscr()
{
 int cols, lns;

 kbdflgs = fcntl( 0, F_GETFL, 0 );
#ifndef TERMCAP
#ifndef NCURSES
 setupterm((char *)0, 1, (int *)0);
#endif
 if ((lns = tigetnum("lines")) > 0)
  MAXSLNS = lns;
 if ((cols = tigetnum("cols")) > 0)
  MAXSCOL = cols;
#else
 if ((tc_screen = getenv("TERM")) == NULL)
  e_exitm("Environment variable TERM not defined!", 1);
 if ((tgetent(tcbuf, tc_screen)) != 1)
  e_exitm("Unknown terminal type!", 1);
 if ((lns = tgetnum("li")) > 0)
  MAXSLNS = lns;
 if ((cols = tgetnum("co")) > 0)
  MAXSCOL = cols;
#endif
 return(0);
}

#define fk_putp(p) ( p ? e_putp(p) : e_putp(att_no) )

void e_endwin()
{
#ifdef NCURSES
 endwin();
#else
 fk_putp(ratt_bo);
#endif
 tcsetattr(0, TCSADRAIN, &otermio);
}

int fk_t_cursor(int x)
{
 return(x);
}

int fk_t_putchar(int c)
{
#ifdef NCURSES
 addch(c);
 return c;
#else
 return(fputc(c, stdout));
#endif
}

int fk_attrset(int a)
{
 if(cur_attr == a) return(0);
#ifdef NCURSES
 switch(a)
 {  case 0:  attrset(A_NORMAL);  break;
    case 1:  attrset(A_STANDOUT);  break;
    case 2:  attrset(A_UNDERLINE);  break;
    case 4:  attrset(A_REVERSE);  break;
    case 8:  attrset(A_BLINK);  break;
    case 16:  attrset(A_DIM);  break;
    case 32:  attrset(A_BOLD);  break;
 }
#else
 switch(cur_attr)
 {  case 0:  fk_putp(ratt_no);  break;
    case 1:  fk_putp(ratt_so);  break;
    case 2:  fk_putp(ratt_ul);  break;
    case 4:  fk_putp(ratt_rv);  break;
    case 8:  fk_putp(ratt_bl);  break;
    case 16:  fk_putp(ratt_dm);  break;
    case 32:  fk_putp(ratt_bo);  break;
 }
 switch(a)
 {  case 0:  fk_putp(att_no);  break;
    case 1:  fk_putp(att_so);  break;
    case 2:  fk_putp(att_ul);  break;
    case 4:  fk_putp(att_rv);  break;
    case 8:  fk_putp(att_bl);  break;
    case 16:  fk_putp(att_dm);  break;
    case 32:  fk_putp(att_bo);  break;
 }
#endif
 return(cur_attr = a);
}

void fk_colset(int c)
{
 int bg;
 if (cur_attr == c) return;
 cur_attr = c;
 bg = c / 16;
#ifdef TERMCAP
 if ((c %= 16) >= col_num)
 {
  fk_putp(att_bo);
  e_putp(tgoto(col_fg, 0, c % col_num));
  e_putp(tgoto(col_bg, 0, bg));
 }
 else
 {
  fk_putp(ratt_bo);
  e_putp(tgoto(col_fg, 0, c));
  e_putp(tgoto(col_bg, 0, bg));
 }
#else
#ifdef NCURSES
 if (c & 8)
  attrset(A_BOLD);
 else
  attrset(A_NORMAL);
 color_set((bg * 8) + c % 8, NULL);
#else
 if ((c %= 16) >= col_num)
 {
  fk_putp(att_bo);
  e_putp(tparm1(col_fg, c % col_num));
  e_putp(tparm1(col_bg, bg));
 }
 else
 {
  fk_putp(ratt_bo);
  e_putp(tparm1(col_fg, c));
  e_putp(tparm1(col_bg, bg));
 }
#endif
#endif
}

int e_t_refresh()
{
 int x = cur_x, y = cur_y, i, j, c;
 fk_cursor(0);
 for(i = 0; i < MAXSLNS; i++)
  for(j = 0; j < MAXSCOL; j++)
  {
   if (i == MAXSLNS-1 && j == MAXSCOL-1) break;
   if (*(schirm + 2*MAXSCOL * i + 2 * j) != *(altschirm + 2*MAXSCOL * i + 2 * j) ||
     *(schirm + 2*MAXSCOL * i + 2 * j + 1) != *(altschirm + 2*MAXSCOL * i + 2 * j + 1) )
   {
    if (cur_x != j || cur_y != i)
     term_move(j, i);
    if (cur_x < MAXSCOL)
    {  cur_x = j + 1;  cur_y = i;  }
    else
    {  cur_x = 0;  cur_y = i+1;  }
    if (col_num <= 0)
     fk_attrset(e_gt_col(j, i));
    else
     fk_colset(e_gt_col(j, i));
    c = e_gt_char(j, i);
#ifdef NCURSES
    if (c < NSPCHR)
     addch(sp_chr[c]);
    else
     addch(c);
#else
    if (c < NSPCHR)
     e_putp(sp_chr[c]);
    else
     fputc(c, stdout);
#endif
    *(altschirm + 2*MAXSCOL * i + 2 * j) = *(schirm + 2*MAXSCOL * i + 2 * j);
    *(altschirm + 2*MAXSCOL * i + 2 * j + 1) = *(schirm + 2*MAXSCOL * i + 2 * j + 1);
   }
  }
 fk_cursor(1);
 fk_locate(x, y);
 term_refresh();
 return(0);
}

int e_t_sys_ini()
{
 e_refresh();
 tcgetattr(0, &ttermio);
 svflgs = fcntl( 0, F_GETFL, 0 );
 e_endwin();
 return(0);   
}

int e_t_sys_end()
{
 tcsetattr(0, TCSADRAIN, &ttermio);
 fcntl( 0, F_SETFL, svflgs );
 e_abs_refr();
 fk_locate(0, 0);
 return(0);
}

int e_t_kbhit()
{
 int ret;
 char kbdflgs, c;

 e_refresh();
 kbdflgs = fcntl(0, F_GETFL, 0 );
 fcntl(0, F_SETFL, kbdflgs | O_NONBLOCK);
 ret = read(0, &c, 1);
 fcntl(0, F_SETFL, kbdflgs & ~O_NONBLOCK);
 return (ret == 1 ? c : 0);
}

#ifdef NCURSES
int e_t_getch()
{
 int c, bk;

 e_refresh();
 c = fk_getch();
 if (c > KEY_CODE_YES)
 {
  switch (c)
  {
   case KEY_F(1):  c = F1; break;
   case KEY_F(2):  c = F2; break;
   case KEY_F(3):  c = F3; break;
   case KEY_F(4):  c = F4; break;
   case KEY_F(5):  c = F5; break;
   case KEY_F(6):  c = F6; break;
   case KEY_F(7):  c = F7; break;
   case KEY_F(8):  c = F8; break;
   case KEY_F(9):  c = F9; break;
   case KEY_F(10):  c = F10; break;
   case KEY_UP:  c = CUP; break;
   case KEY_DOWN:  c = CDO; break;
   case KEY_LEFT:  c = CLE; break;
   case KEY_RIGHT:  c = CRI; break;
   case KEY_SFIND:  c = EINFG; break;
   case KEY_IC:  c = POS1; break;
   case KEY_DC:  c = BUP; break;
   case KEY_PPAGE:  c = ENDE; break;
   case KEY_NPAGE:  c = BDO; break;
   case KEY_BACKSPACE:  c = WPE_DC; break;
   case KEY_HELP:  c = HELP; break;
   case KEY_LL:  c = ENDE; break;
   case KEY_F(17):  c = SF1; break;
   case KEY_F(18):  c = SF2; break;
   case KEY_F(19):  c = SF3; break;
   case KEY_F(20):  c = SF4; break;
   case KEY_F(21):  c = SF5; break;
   case KEY_F(22):  c = SF6; break;
   case KEY_F(23):  c = SF7; break;
   case KEY_F(24):  c = SF8; break;
   case KEY_F(25):  c = SF9; break;
   case KEY_F(26):  c = SF10; break;
   case KEY_PREVIOUS:  c = CUP+512; break;
   case KEY_NEXT:  c = CDO+512; break;
   case KEY_HOME:  c = POS1+512; break;
   case KEY_END:  c = ENDE+512; break;
   default:  c = 0; break;
  }
  bk = bioskey();
  if (bk & 3)
   c += 512;
  else if (bk & 4)
   c += 514;
 }
 else if ( c == WPE_TAB )
 {
  bk = bioskey(); 
  if ( bk & 3)
   c = WPE_BTAB;
  else
   c = WPE_TAB;
 }
 else if (c == WPE_ESC)
 {
  c = fk_getch();
  if (c > KEY_CODE_YES)
  {
   switch (c)
   {
    case KEY_F(1):  c = AF1; break;
    case KEY_F(2):  c = AF2; break;
    case KEY_F(3):  c = AF3; break;
    case KEY_F(4):  c = AF4; break;
    case KEY_F(5):  c = AF5; break;
    case KEY_F(6):  c = AF6; break;
    case KEY_F(7):  c = AF7; break;
    case KEY_F(8):  c = AF8; break;
    case KEY_F(9):  c = AF9; break;
    case KEY_F(10):  c = AF10; break;
    case KEY_UP:  c = BUP; break;
    case KEY_DOWN:  c = BDO; break;
    case KEY_LEFT:  c = CCLE; break;
    case KEY_RIGHT:  c = CCRI; break;
    case KEY_SFIND:  c = EINFG; break;
    case KEY_IC:  c = CPS1; break;
    case KEY_DC:  c = CBUP; break;
    case KEY_PPAGE:  c = CEND; break;
    case KEY_NPAGE:  c = CBDO; break;
    case KEY_BACKSPACE:  c = AltBS; break;
    case KEY_HELP:  c = AF1; break;
    case KEY_LL:  c = CEND; break;
    case KEY_PREVIOUS:  c = BUP+512; break;
    case KEY_NEXT:  c = BDO+512; break;
    case KEY_HOME:  c = CPS1+512; break;
    case KEY_END:  c = CEND+512; break;
    default:  c = 0; break;
   }
   bk = bioskey();
   if (bk & 3)
    c += 512;
   else if (bk & 4)
    c += 514;
  }
  else if (c != WPE_ESC)
   c = e_tast_sim(c);
 }
 return(c);
}

#else
int e_t_getch()
{
 int c, c2, pshift, bk;

 pshift = 0;
 e_refresh();
 if ((c = fk_getch()) != WPE_ESC)
 {
  if (key_f[20] && c == *key_f[20])
   return(WPE_DC);
  else if (key_f[17] && c == *key_f[17])
   return(ENTF);
  else if ( c == WPE_TAB )
  {
   bk = bioskey(); 
   if (bk & 3) 
    return (WPE_BTAB); 
   else
    return (WPE_TAB); 
  }
  else
   return(c);
 }
 else if ((c = fk_getch()) == WPE_CR)
  return(WPE_ESC);
 bk = bioskey();
 if (bk & 3)
  pshift = 512;
 else if (bk & 4)
  pshift = 514;
 if (c == WPE_ESC)
 {
  if ((c = fk_getch()) == WPE_ESC)
   return(WPE_ESC);
  if ((c2 = e_find_key(c, 1, 1)))
   return(c2 + pshift);
 }
 if ((c2 = e_find_key((char)c, 1, 0)))
  return(c2 + pshift);
 return(e_tast_sim(c + pshift));
}

char e_key_save[10];

int e_find_key(int c, int j, int sw)
{
   int i, k;
   e_key_save[j] = c;
   e_key_save[j+1] = '\0';
   for(i = 0; i < KEYFN; i++)
   {  if(key_f[i] == NULL) continue;
      for(k = 1; k <= j && e_key_save[k] == *(key_f[i] + k); k++);
      if(k > j)
      {  if(*(key_f[i] + k) != '\0') return(e_find_key(fk_getch(), j+1, sw));
	 else if(sw)
	 {  switch(i)
	    {  case 0:  return(AF1);
	       case 1:  return(AF2);
	       case 2:  return(AF3);
	       case 3:  return(AF4);
	       case 4:  return(AF5);
	       case 5:  return(AF6);
	       case 6:  return(AF7);
	       case 7:  return(AF8);
	       case 8:  return(AF9);
	       case 9:  return(AF10);
	       case 10:  return(BUP);
	       case 11:  return(BDO);
	       case 12:  return(CCLE);
	       case 13:  return(CCRI);
	       case 14:  return(EINFG);
	       case 15:  return(CPS1);
	       case 16:  return(CBUP);
	       case 17:  return(ENTF);
	       case 18:  return(CEND);
	       case 19:  return(CBDO);
	       case 20:  return(AltBS);
	       case 21:  return(AF1);
	       case 22:  return(CEND);
	       case 33:  return(BUP+512);
	       case 34:  return(BDO+512);
	       case 35:  return(CCLE+512);
	       case 36:  return(CCRI+512);
	       case 37:  return(CPS1+512);
	       case 38:  return(CBUP+512);
	       case 39:  return(CEND+512);
	       case 40:  return(CBDO+512);
	       case 41:  return(CEND);
	       default:  return(0);
	    }
	 }
	 else
	 {  switch(i)
	    {  case 0:  return(F1);
	       case 1:  return(F2);
	       case 2:  return(F3);
	       case 3:  return(F4);
	       case 4:  return(F5);
	       case 5:  return(F6);
	       case 6:  return(F7);
	       case 7:  return(F8);
	       case 8:  return(F9);
	       case 9:  return(F10);
	       case 10:  return(CUP);
	       case 11:  return(CDO);
	       case 12:  return(CLE);
	       case 13:  return(CRI);
	       case 14:  return(EINFG);
	       case 15:  return(POS1);
	       case 16:  return(BUP);
	       case 17:  return(ENTF);
	       case 18:  return(ENDE);
	       case 19:  return(BDO);
	       case 20:  return(WPE_DC);
	       case 21:  return(HELP);
	       case 22:  return(ENDE);
	       case 23:  return(SF1);
	       case 24:  return(SF2);
	       case 25:  return(SF3);
	       case 26:  return(SF4);
	       case 27:  return(SF5);
	       case 28:  return(SF6);
	       case 29:  return(SF7);
	       case 30:  return(SF8);
	       case 31:  return(SF9);
	       case 32:  return(SF10);
	       case 33:  return(CUP+512);
	       case 34:  return(CDO+512);
	       case 35:  return(CLE+512);
	       case 36:  return(CRI+512);
	       case 37:  return(POS1+512);
	       case 38:  return(BUP+512);
	       case 39:  return(ENDE+512);
	       case 40:  return(BDO+512);
	       case 41:  return(ENDE);
	       default:  return(0);
	    }
	 }
      }
   }
   return(0);
}
#endif

int fk_t_locate(int x, int y)
{
 if (col_num > 0) 
 {
  fk_colset(e_gt_col(cur_x, cur_y));
#ifdef NCURSES
  /* Causes problems.  Reason unknown. - Dennis */
  /*mvaddch(cur_y,cur_x,e_gt_char(cur_x, cur_y));*/
#else
  fputc(e_gt_char(cur_x, cur_y), stdout);
#endif
 }
 cur_x = x;
 cur_y = y;
 term_move(x, y);
 return(y);
}

int fk_t_mouse(int *g)
{
 return(0);
}

int e_t_switch_screen(int sw)
{
 static int sav_sw = -1;

 if (sw == sav_sw)
  return(0);
 sav_sw = sw;
 if (sw && beg_scr) 
 {
  term_refresh();
  if (sav_cur)
   e_putp(sav_cur);
  e_putp(beg_scr);
 }
 else if (!sw && swt_scr)
 {
  e_putp(swt_scr);
  if (res_cur)
   e_putp(res_cur);
  term_refresh();
 }
 else
  return(-1);
 return(0);
}

int e_t_deb_out(FENSTER *f)
{
#ifndef NCURSES
 if (!swt_scr || !beg_scr)
#endif
  return(e_error("Your terminal don\'t use begin/end cup", 0, f->fb));
 e_d_switch_out(1);
 getchar();
 e_d_switch_out(0);
 return(0);
}

int e_d_switch_screen(int sw)
{
#ifdef DEBUGGER
 if (!sw)
  e_g_sys_ini();
 else
  e_g_sys_end();
#endif
 return(e_switch_screen(sw));
}

int e_t_d_switch_out(int sw)
{
 int i, j;
 static int save_sw = 32000;

 if (save_sw == sw)
  return(0);
 save_sw = sw;
 if (sw && e_d_switch_screen(0))
 {
  term_move(0, 0);
  e_putp(att_no);
  for(i = 0; i < MAXSLNS; i++)
   for (j = 0; j < MAXSCOL; j++)
    e_d_putchar(' ');
  term_move(0, 0);
  term_refresh();
 }
 else if (!sw)
 {
  e_d_switch_screen(1);
  e_abs_refr();
  e_refresh();
 }
 return(sw);
}

void e_exitm(char *s, int n)
{
 e_endwin();
 if (n != 0)
  printf("\n%s\n", s);
 exit(n);
}

int e_s_sys_ini()
{
 e_sys_ini();
 return(e_switch_screen(0));
}

int e_s_sys_end()
{
 e_switch_screen(1);
 return(e_sys_end());
}

#endif  /*  Is UNIX       */

