#ifndef WE_MAIN_H
#define WE_MAIN_H

#include "globals.h"
#include "we_edit.h"
#include "we_e_aus.h"
#include "we_fl_unix.h"
#include "we_fl_fkt.h"
#include "we_hfkt.h"
#include "we_opt.h"

void e_ini_desk(ECNT * cn);
void FARBE_Init(FARBE * fb);
FARBE * e_ini_farbe();
int e_switch_blst(ECNT * cn);
void e_free_find(FIND * fd);

#endif
