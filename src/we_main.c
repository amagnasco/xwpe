/* we_main.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

/*
                       Window - Editor (WE)
                           Version 1.0
                                       Copyright by F.Kruse       */

/* includes */
#include <string.h>
#include <signal.h>
#include "config.h"
#include "keys.h"
#include "messages.h"
#include "options.h"
#include "model.h"
#include "edit.h"
#include "we_main.h"
#include "WeProg.h"
#include "we_prog.h"
#include "we_fl_unix.h"
#include "we_fl_fkt.h"
#include "WeString.h"
#include "we_opt.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "attrb.h"
#endif

#include "we_control.h"



int
main (int argc, char **argv)
{
  we_colorset *fb;
  ECNT *cn;
  int i, err = 0, g[4];
  int so = 0, sd = 1;
  char *tp;

  if ((cn = (ECNT *) malloc (sizeof (ECNT))) == NULL)
    {
      printf (" Fatal Error: %s\n", e_msg[ERR_LOWMEM]);
      return 0;
    }
  ECNT_Init (cn);
  e_ini_unix (&argc, argv);
  (*e_u_switch_screen) (1);
  fb = e_ini_farbe ();
  WpeEditor = cn;
  cn->fb = fb;

  info_file = malloc ((strlen (INFO_DIR) + 1) * sizeof (char));
  strcpy (info_file, INFO_DIR);
  e_read_help_str ();
  e_hlp = e_hlp_str[0];
  if (!(user_shell = getenv ("SHELL")))
    user_shell = DEF_SHELL;
#ifdef HAVE_MKDTEMP
  e_tmp_dir = strdup ("/tmp/xwpe_XXXXXX");
  if (mkdtemp (e_tmp_dir) == NULL)
#else
#if defined(HAVE_TEMPNAM)
  e_tmp_dir = tempnam (NULL, "xwpe_");
#else
  e_tmp_dir = malloc (128);
  sprintf (e_tmp_dir, "/tmp/we_%u", (unsigned) getpid ());
  e_tmp_dir = realloc (e_tmp_dir, (strlen (e_tmp_dir) + 1) * sizeof (char));
#endif
  if ((e_tmp_dir == NULL) || (mkdir (e_tmp_dir, 0700) != 0))
#endif
    {
      perror ("Xwpe: ");
      return 1;
    }

  for (i = 1; i < argc; i++)
    {
      if (*argv[i] == '-')
	{
	  if (*(argv[i] + 1) == 's' && *(argv[i] + 2) == 'o')
	    so = 1;
	  else if (*(argv[i] + 1) == 's' && *(argv[i] + 2) == 'f')
	    {
	      sd = 0;
	      cn->optfile =
		malloc ((strlen (argv[i + 1]) + 1) * sizeof (char));
	      strcpy (cn->optfile, argv[i + 1]);
	    }
	}
    }
#ifdef PROG
  e_ini_prog (cn);
#endif
  if (sd != 0)
    {
      FILE *f = fopen (OPTION_FILE, "r");
      if (f)
	{
	  fclose (f);
	  cn->optfile = e_mkfilename (cn->dirct, OPTION_FILE);
	}
      else
	{
	  cn->optfile = e_mkfilename (getenv ("HOME"), XWPE_HOME);
	  cn->optfile = realloc (cn->optfile,
				 strlen (cn->optfile) + strlen (OPTION_FILE) +
				 2);
	  strcat (cn->optfile, DIRS);
	  strcat (cn->optfile, OPTION_FILE);
	}
    }
  if (so == 0)
    err = e_opt_read (cn);
  e_edit (cn, "");		/* Clipboard (must first read option file) */
  if ((tp = getenv ("INFOPATH")) != NULL)
    {
      if (info_file)
	free (info_file);
      info_file = WpeStrdup (tp);
    }
  if (cn->edopt & ED_CUA_STYLE)
    blst = eblst_u;
  else
    blst = eblst_o;
  e_ini_desk (cn);
  cn->f[0]->blst = eblst;
#if MOUSE
  g[0] = 4;
  g[2] = 0;
  g[3] = 0;
  fk_mouse (g);
  g[0] = 1;
  fk_mouse (g);
#endif
  /* this error comes from reading the options file */
  if (err > 0)
    e_error (e_msg[err], 0, cn->fb);
  if (WpeIsProg ())
    WpeSyntaxReadFile (cn);
#ifdef UNIX
  for (i = 1; i < argc; i++)
    if (!strcmp (argv[i], "-r"))
      e_recover (cn);
#endif
  for (i = 1; i < argc; i++)
    {
      if (*argv[i] == '-')
	{
	  if (*(argv[i] + 1) == 's' && *(argv[i] + 2) == 'f')
	    i++;
#if defined(UNIX) && defined(PROG)
	  else if (*(argv[i] + 1) == 'p' && *(argv[i] + 2) == 'm')
	    e_we_sw |= 8;
#endif
	  continue;
	}
      else
	e_edit (cn, argv[i]);
    }
  if (cn->mxedt == 0)
    WpeManager (cn->f[cn->mxedt]);
  do
    {
      if (cn->f[cn->mxedt]->dtmd == DTMD_FILEMANAGER)
	i = WpeHandleFileManager (cn);
      else if (cn->f[cn->mxedt]->dtmd == DTMD_DATA)
	i = e_data_eingabe (cn);
      else
	i = e_eingabe (cn);
      if (i == AltX)
	i = e_quit (cn->f[cn->mxedt]);
    }
  while (i != AltX);
  WpeExit (0);
  return 0;
}

