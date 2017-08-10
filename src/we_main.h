#ifndef WE_MAIN_H
#define WE_MAIN_H

#include "config.h"
#include "globals.h"
#include "we_e_aus.h"
#include "we_edit.h"
#include "we_fl_fkt.h"
#include "we_fl_unix.h"
#include "we_hfkt.h"
#include "we_opt.h"
#include "we_unix.h"
#include <stdlib.h>

void e_ini_desk(we_control_t* cn);
void we_colorset_Init(we_colorset_t* fb);
we_colorset_t* e_ini_farbe();
int e_switch_blst(we_control_t* cn);
void e_free_find(FIND* fd);

extern char* e_msg[];

#endif
