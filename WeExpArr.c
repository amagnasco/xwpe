/*-------------------------------------------------------------------------*\
  <WeExpArr.c> -- Xwpe routines for Expanding Arrays

  Date      Programmer  Description
  05/25/97  Dennis      Created for xwpe reorganization.
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include <stdlib.h>
#include <string.h>
#include "Xwpe.h"
#include "WeExpArr.h"


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Expandable Arrays
       Expandable arrays store four integers before the first element of
  the array.  The first integer is the current size of the array including
  unsed elements.  The second integer is the size of each element.  This is
  followed by the growth factor.  Finally the number of currently used
  elements is listed.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

void *WpeExpArrayCreate(int initial_num, int elem_size, int growth_num)
{
 int *exp_array;

 exp_array = (int *)WpeMalloc(initial_num * elem_size + sizeof(int) * 4);
 *(exp_array) = initial_num;
 *(exp_array + 1) = elem_size;
 *(exp_array + 2) = growth_num;
 *(exp_array + 3) = initial_num;
 return ((void *)(exp_array + 4));
}

void WpeExpArrayAdd(void **exp_array, void *new_elem)
{
 int *real_array;

 real_array = ((int *)*exp_array) - 4;
 if (*real_array == *(real_array + 3))
 {
  *(real_array) += *(real_array + 2);
  real_array = WpeRealloc(real_array, (*real_array) * (*(real_array + 1)) +
                                      sizeof(int) * 4);
  if (real_array == NULL)
  {
   /* Some error handling should be done here */
   return ;
  }
  *exp_array = (void *)(real_array + 4);
 }
 memcpy(((char *)*exp_array) + (*(real_array + 3)) * (*(real_array + 1)),
        new_elem, (*(real_array + 1)));
 *(real_array + 3) += 1;
}

int WpeExpArrayGetSize(void *exp_array)
{
 int *real_array;

 real_array = ((int *)exp_array) - 4;
 return (*(real_array + 3));
}

void WpeExpArrayDestroy(void *exp_array)
{
 int *real_array;

 real_array = ((int *)exp_array) - 4;
 WpeFree(real_array);
}

