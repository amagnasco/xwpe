#ifndef __XWPE_H
#define __XWPE_H
/*-------------------------------------------------------------------------*\
  <Xwpe.h> -- Header file for core Xwpe functions

  Date      Programmer  Description
  04/27/97  Dennis      Created for xwpe reorganization.
\*-------------------------------------------------------------------------*/

typedef enum wpeMouseShape {
 WpeEditingShape, WpeDebuggingShape, WpeWorkingShape, WpeErrorShape,
 WpeSelectionShape, WpeLastShape
} WpeMouseShape;

/* Checks if programming editor is running (old variable currently used) */
#define WpeIsProg() (e_we_sw & 2)

/* Checks if x windows is running (old variable currently used) */
#define WpeIsXwin() (e_we_sw & 1)

#define WpeMalloc(x) malloc(x)
#define WpeRealloc(x, y) realloc(x, y)
#define WpeFree(x) free(x)

#endif

