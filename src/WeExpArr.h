#ifndef __WEEXPARR_H
#define __WEEXPARR_H
/*-------------------------------------------------------------------------*\
  <WeExpArr.h> -- Header file of Xwpe routines for Expanding Arrays

  Date      Programmer  Description
  05/25/97  Dennis      Created for xwpe reorganization.
\*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeExpArrayCreate - Creates an expandable array.

    Parameters:
      initial_num  (In)  Initial number of elements for the array
      elem_size    (In)  Size of each element
      growth_num   (In)  Number by which it increases when necessary
    Returns: The new expandable array
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void *WpeExpArrayCreate(int initial_num, int elem_size, int growth_num);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeExpArrayAdd - Adds another element to an expandable array.

    Parameters:
      exp_array    (In & Out) The expandable array
      new_elem     (In)  The new element to add
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void WpeExpArrayAdd(void **exp_array, void *new_elem);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeExpArrayGetSize - Get the number of used elements of an expandable
array.

    Parameters:
      exp_array    (In)  The expandable array
    Returns: Number of used elements
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int WpeExpArrayGetSize(void *exp_array);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeExpArrayDestroy - Destroys an expandable array.

    Parameters:
      exp_array    (In)  Expandable array to be destroyed
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void WpeExpArrayDestroy(void *exp_array);

#ifdef __cplusplus
}
#endif

#endif

