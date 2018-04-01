#ifndef WE_MAIN_H
#define WE_MAIN_H

/** \file we_main.h */

#include "config.h"
#include "globals.h"
#include "we_e_aus.h"
#include "we_edit.h"
#include "we_control.h"
#include "we_file_fkt.h"
#include "we_file_unix.h"
#include "we_hfkt.h"
#include "we_opt.h"
#include "we_unix.h"
#include <stdlib.h>

void e_ini_desk(we_control_t* control);
int e_switch_blst(we_control_t* control);
void e_free_find(FIND* find);

extern char* e_msg[];

#endif
