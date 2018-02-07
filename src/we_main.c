/** \file we_main.c                                        */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

/*
                       Window - Editor (WE)
                           Version 1.0
                                       Copyright by F.Kruse       */

/* includes */
#include "config.h"
#include <string.h>
#include <signal.h>
#include "keys.h"
#include "messages.h"
#include "options.h"
#include "model.h"
#include "edit.h"
#include "we_main.h"
#include "WeProg.h"
#include "we_prog.h"
#include "we_file_unix.h"
#include "we_file_fkt.h"
#include "WeString.h"
#include "we_opt.h"

#ifdef UNIX
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "attrb.h"
#endif

#include "we_control.h"

const char *default_shell = "sh";

int
main (int argc, char **argv)
{
    we_colorset_t *fb;
    we_control_t *control;
    int i, err = 0;
#if MOUSE
    int g[4];
#endif
    int so = 0, sd = 1;
    char *tp;

    control = e_control_new ();
    if (control == NULL) {
        printf (" Fatal Error: Failed to create control.\n");
        return 0;
    }
    e_ini_unix (&argc, argv);
    e_u_switch_screen (1);
    fb = e_ini_colorset ();
    global_editor_control = control;
    control->colorset = fb;

    info_file = malloc ((strlen (INFO_DIR) + 1) * sizeof (char));
    strcpy (info_file, INFO_DIR);
    e_read_help_str ();
    e_hlp = e_hlp_str[0];
    if (!(user_shell = getenv ("SHELL"))) {
        user_shell = (char *) default_shell;
    }
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

    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (*(argv[i] + 1) == 's' && *(argv[i] + 2) == 'o') {
                so = 1;
            } else if (*(argv[i] + 1) == 's' && *(argv[i] + 2) == 'f') {
                sd = 0;
                control->optfile =
                    malloc ((strlen (argv[i + 1]) + 1) * sizeof (char));
                strcpy (control->optfile, argv[i + 1]);
            }
        }
    }
#ifdef PROG
    e_ini_prog (control);
#endif
    if (sd != 0) {
        FILE *opt_file = fopen (OPTION_FILE, "r");
        if (opt_file) {
            fclose (opt_file);
            control->optfile = e_mkfilename (control->dirct, OPTION_FILE);
        } else {
            control->optfile = e_mkfilename (getenv ("HOME"), XWPE_HOME);
            control->optfile = realloc (control->optfile,
                                        strlen (control->optfile) + strlen (OPTION_FILE) +
                                        2);
            strcat (control->optfile, DIRS);
            strcat (control->optfile, OPTION_FILE);
        }
    }
    if (so == 0) {
        err = e_opt_read (control);
    }
    e_edit (control, "");		/* Clipboard (must first read option file) */
    if ((tp = getenv ("INFOPATH")) != NULL) {
        if (info_file) {
            free (info_file);
        }
        info_file = WpeStrdup (tp);
    }
    if (control->edopt & ED_CUA_STYLE) {
        blst = eblst_u;
    } else {
        blst = eblst_o;
    }
    e_ini_desk (control);
    control->window[0]->blst = eblst;
#if MOUSE
    g[0] = 4;
    g[2] = 0;
    g[3] = 0;
    fk_mouse (g);
    g[0] = 1;
    fk_mouse (g);
#endif
    /* this error comes from reading the options file */
    if (err > 0) {
        e_error (e_msg[err], ERROR_MSG, control->colorset);
    }
    if (WpeIsProg ()) {
        WpeSyntaxReadFile (control);
    }
#ifdef UNIX
    for (i = 1; i < argc; i++)
        if (!strcmp (argv[i], "-r")) {
            e_recover (control);
        }
#endif
    for (i = 1; i < argc; i++) {
        if (*argv[i] == '-') {
            if (*(argv[i] + 1) == 's' && *(argv[i] + 2) == 'f') {
                i++;
            }
#if defined(UNIX) && defined(PROG)
            else if (*(argv[i] + 1) == 'p' && *(argv[i] + 2) == 'm') {
                e_we_sw |= 8;
            }
#endif
            continue;
        } else {
            e_edit (control, argv[i]);
        }
    }
    if (control->mxedt == 0) {
        WpeManager (control->window[control->mxedt]);
    }
    do {
        if (control->window[control->mxedt]->dtmd == DTMD_FILEMANAGER) {
            i = WpeHandleFileManager (control);
        } else if (control->window[control->mxedt]->dtmd == DTMD_DATA) {
            i = e_data_eingabe (control);
        } else {
            i = e_eingabe (control);
        }
        if (i == AltX) {
            i = e_quit (control->window[control->mxedt]);
        }
    } while (i != AltX);
    e_exit (0);
    return 0;
}
