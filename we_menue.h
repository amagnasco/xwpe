#ifndef WE_MENUE_H
#define WE_MENUE_H

#include "we_block.h"
#include "we_edit.h"

/*   we_menue.c   */
int WpeHandleMainmenu(int n, FENSTER *f);
int WpeHandleSubmenu(int xa, int ya, int xe, int ye, 
                     int nm, OPTK *fopt, FENSTER *f);
OPTK WpeFillSubmenuItem(char *t, int x, char o, int (*fkt)());

#endif
