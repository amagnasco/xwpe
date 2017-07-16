/**
 * utils.c
 *
 * Copyright Fred Kruse, Guus Bonnema.
 *
 * User License GPL v 2 (see file COPYING)
 *
 * modification 2017: renamed from makro.h to utils.h and utils.c
 * 			replaced macros isalnum1 and isalpha1 by functions.
 *
 */

#include <ctype.h>
#include "utils.h"

int isalnum1(int x) {
	return (isalnum(x) || x == '_');
}

int isalpha1(int x) {
	return (isalpha(x) || x == '_');
}

