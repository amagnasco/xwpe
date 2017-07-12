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
 char **reserved_word;  /* Reserved words */
 char **long_operator;  /* Operators longer than a single character*/
 char *single_operator; /* Single character operators */
 char *begin_comment;   /* Comments begin with this string */
 char *end_comment;     /* Comments end with this string */
 char *line_comment;    /* Comments 'til end of line with this */
 char string_constant;  /* Character denoting string constant */
 char char_constant;    /* Character denoting character constant */
 char preproc_cmd;      /* Character denoting preprocessor commnand*/
 char quoting_char;     /* Quote the next character */
 char continue_char;    /* Line continues if this is last character*/
 char insensitive;      /* Set when language is not case sensitive */
 int continue_column;            /* Continues previous line if anything in
                                    the column (works?) */
 int comment_column;             /* Comments from this column on */

 /* If any character from the special comment string is found in the special
   column the rest of the line is a comment */
 int special_column;
 char *special_comment;
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

#define WpeSyntaxGetPersonal(filename)                                      \
 sprintf(filename, "%s/%s/%s", getenv("HOME"), XWPE_HOME, SYNTAX_FILE)

#define WpeSyntaxGetSystem(filename)                                        \
 sprintf(filename, "%s/%s", DATADIR, SYNTAX_FILE)


#ifdef __cplusplus
}
#endif

#endif

