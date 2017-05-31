#ifndef WE_MAIN_H
#define WE_MAIN_H

extern struct CNT * WpeEditor;

void e_ini_desk(ECNT * cn);
void FARBE_Init(FARBE * fb);
FARBE * e_ini_farbe();
int e_switch_blst(ECNT * cn);
void e_free_find(FIND * fd);

#endif
