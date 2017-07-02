#ifndef __WEPROG_H
#define __WEPROG_H
/*-------------------------------------------------------------------------*\
  <WeProg.h> -- Header file of Xwpe routines for programming support

  Date      Programmer  Description
  05/24/97  Dennis      Created for xwpe reorganization.
\*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/* needed for the time being to call old routines and data types */
#include "model.h"
#include "edit.h"

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  New Types
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
typedef struct wpeSyntaxRule {
 unsigned char **reserved_word;  /* Reserved words */
 unsigned char **long_operator;  /* Operators longer than a single character*/
 unsigned char *single_operator; /* Single character operators */
 unsigned char *begin_comment;   /* Comments begin with this string */
 unsigned char *end_comment;     /* Comments end with this string */
 unsigned char *line_comment;    /* Comments 'til end of line with this */
 unsigned char string_constant;  /* Character denoting string constant */
 unsigned char char_constant;    /* Character denoting character constant */
 unsigned char preproc_cmd;      /* Character denoting preprocessor commnand*/
 unsigned char quoting_char;     /* Quote the next character */
 unsigned char continue_char;    /* Line continues if this is last character*/
 unsigned char insensitive;      /* Set when language is not case sensitive */
 int continue_column;            /* Continues previous line if anything in
                                    the column (works?) */
 int comment_column;             /* Comments from this column on */

 /* If any character from the special comment string is found in the special
   column the rest of the line is a comment */
 int special_column;
 unsigned char *special_comment;
} WpeSyntaxRule;

typedef struct wpeSyntaxExt {
 char **extension;
 WpeSyntaxRule *syntax_rule;
} WpeSyntaxExt;

/* Necessary for the time being */
extern WpeSyntaxExt **WpeSyntaxDef;


/* WeSyntax.h *\
void WpeSyntaxGetPersonal(char *filename);
void WpeSyntaxGetSystem(char *filename,);                                  */
void WpeSyntaxReadFile(ECNT *cn);


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Macros and Machine specific information
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#ifdef DJGPP

#define WpeSyntaxGetPersonal(filename)                                      \
 sprintf(filename, "./%s", SYNTAX_FILE)

#else

#define WpeSyntaxGetPersonal(filename)                                      \
 sprintf(filename, "%s/%s/%s", getenv("HOME"), XWPE_HOME, SYNTAX_FILE)

#endif

#define WpeSyntaxGetSystem(filename)                                        \
 sprintf(filename, "%s/%s", LIBRARY_DIR, SYNTAX_FILE)


#ifdef __cplusplus
}
#endif

#endif

