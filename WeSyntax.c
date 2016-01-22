/*-------------------------------------------------------------------------*\
  <WeSyntax.c> -- Xwpe routines for syntax highlighting support

  Date      Programmer  Description
  05/24/97  Dennis      Created based on functions from "we_progn.c".
\*-------------------------------------------------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Original header of "we_progn.c"
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* we_progn.c                                             */
/* Copyright (C) 1993 Fred Kruse                          */
/* This is free software; you can redistribute it and/or  */
/* modify it under the terms of the                       */
/* GNU General Public License, see the file COPYING.      */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  Includes
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
#include <stdlib.h>
#include <string.h>
#include "Xwpe.h"
#include "WeExpArr.h"
#include "WeProg.h"
#include "WeString.h"

unsigned char *WpeCReservedWord[] = {
 "auto", "break", "case", "char", "const", "continue", "default", "do",
 "double", "else", "enum", "extern", "float", "for", "goto", "if", "int",
 "long", "register", "return", "short", "signed", "sizeof", "static",
 "struct", "switch", "typedef", "union", "unsigned", "void", "volatile",
 "while", NULL
};

#define WpeCLongOperator NULL

unsigned char *WpeCxxReservedWord[] = {
 "and", "and_eq", "asm", "auto", "bitand", "bitor", "bool", "break", "case",
 "catch", "char", "class", "compl", "const", "const_cast", "continue",
 "default", "delete", "do", "double", "dynamic_cast", "else", "enum",
 "explicit", "extern", "false", "float", "for", "friend", "goto", "if",
 "inline", "int", "long", "mutable", "namespace", "new", "not", "not_eq",
 "operator", "or", "or_eq", "private", "protected", "public", "register",
 "reinterpret_cast", "return", "short", "signed", "sizeof", "static",
 "static_cast", "struct", "switch", "template", "this", "throw", "true",
 "try", "typedef", "typeid", "typename", "union", "unsigned", "using",
 "virtual", "void", "volatile", "wchar_t", "while", "xor", "xor_eq", NULL
};

#define WpeCxxLongOperator NULL

WpeSyntaxRule WpeCSyntaxRule = {
 WpeCReservedWord, WpeCLongOperator, "~^()[]{}<>+-/*%=|&!.?:,;", "/*", "*/",
 "", '\"', '\'', '#', '\\', '\\', '\0', -1, 1000, -1, ""
};

WpeSyntaxRule WpeCxxSyntaxRule = {
 WpeCxxReservedWord, WpeCxxLongOperator, "~^()[]{}<>+-/*%=|&!.?:,;", "/*",
 "*/", "//", '\"', '\'', '#', '\\', '\\', '\0', -1, 1000, -1, ""
};

/*WpeSyntaxExt WpeCSyntaxExt   = { ".c", &WpeCSyntaxRule };
WpeSyntaxExt WpeCxxSyntaxExt = { ".C", &WpeCxxSyntaxRule };
WpeSyntaxExt WpeHSyntaxExt   = { ".h", &WpeCxxSyntaxRule };*/

/* Definition of all programming language syntax (Note: This is an expandable
  array and must be destroyed with WpeExpArrayDestroy()) */
WpeSyntaxExt **WpeSyntaxDef = NULL;


int WpeReservedWordCompare(const void *x, const void *y)
{
 return (strcmp(*((char **)x), *((char **)y)));
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeSyntaxReadFile - Reads the programming language syntax file.

    Parameters:
      cn           (In)  Part of old code needed for e_error
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
void WpeSyntaxReadFile(ECNT *cn)
{
 FILE *syntax_file;
 WpeSyntaxExt *new_syntax;
 char tmp[128];
 int reserved_num, long_op_num;
 int i, k;

 WpeSyntaxDef = (WpeSyntaxExt **)WpeExpArrayCreate(0, sizeof(WpeSyntaxExt *), 4);
 WpeSyntaxGetPersonal(tmp);
 if ((syntax_file = fopen(tmp, "r")) == NULL)
 {
  WpeSyntaxGetSystem(tmp);
  if ((syntax_file = fopen(tmp, "r")) == NULL)
  {
   /* C Syntax (".c" extension) */
   new_syntax = WpeMalloc(sizeof(WpeSyntaxExt));
   new_syntax->extension = (char **)WpeExpArrayCreate(1, sizeof(char *), 1);
   new_syntax->extension[0] = strdup(".c");
   new_syntax->syntax_rule = &WpeCSyntaxRule;
   WpeExpArrayAdd((void **)&WpeSyntaxDef, &new_syntax);
   /* C++ Syntax (".C", ".cpp", ".cxx", ".cc", ".h", and ".hpp" extensions) */
   new_syntax = WpeMalloc(sizeof(WpeSyntaxExt));
   new_syntax->extension = (char **)WpeExpArrayCreate(6, sizeof(char *), 1);
   new_syntax->extension[0] = strdup(".C");
   new_syntax->extension[1] = strdup(".cpp");
   new_syntax->extension[2] = strdup(".cxx");
   new_syntax->extension[3] = strdup(".cc");
   new_syntax->extension[4] = strdup(".h");
   new_syntax->extension[5] = strdup(".hpp");
   new_syntax->syntax_rule = &WpeCxxSyntaxRule;
   WpeExpArrayAdd((void **)&WpeSyntaxDef, &new_syntax);
   /*WpeSyntaxDef[2] = &WpeHSyntaxExt;*/
   return ;
  }
 }
 while (fscanf(syntax_file, "%s", tmp) == 1)
 {
  new_syntax = WpeMalloc(sizeof(WpeSyntaxExt));
  i = atoi(tmp);
  if (i > 0)
  {
   new_syntax->extension = (char **)WpeExpArrayCreate(i, sizeof(char *), 1);
   for (k = 0; k < i; k++)
   {
    if (fscanf(syntax_file, "%s", tmp) != 1)
    {
     e_error("Error reading syntax_def", 0, cn->fb);
     return ;
    }
    new_syntax->extension[k] = WpeStrdup(tmp);
   }
  }
  else
  {
   new_syntax->extension = (char **)WpeExpArrayCreate(1, sizeof(char *), 1);
   new_syntax->extension[0] = WpeStrdup(tmp);
  }
  new_syntax->syntax_rule = WpeMalloc(sizeof(WpeSyntaxRule));
  if (fscanf(syntax_file, "%d", &reserved_num) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->reserved_word = WpeMalloc((reserved_num + 1) *
                                                     sizeof(char *));
  for (i = 0; i < reserved_num; i++)
  {
   if (fscanf(syntax_file, "%s", tmp) != 1)
   {
    e_error("Error reading syntax_def", 0, cn->fb);
    return ;
   }
   new_syntax->syntax_rule->reserved_word[i] = WpeStrdup(tmp);
  }
  new_syntax->syntax_rule->reserved_word[i] = NULL;
  if (fscanf(syntax_file, "%d", &long_op_num) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->long_operator = WpeMalloc((long_op_num + 1) *
                                                     sizeof(char *));
  for (i = 0; i < long_op_num; i++)
  {
   if (fscanf(syntax_file, "%s", tmp) != 1)
   {
    e_error("Error reading syntax_def", 0, cn->fb);
    return ;
   }
   new_syntax->syntax_rule->long_operator[i] = WpeStrdup(tmp);
  }
  new_syntax->syntax_rule->long_operator[i] = NULL;
  if (fscanf(syntax_file, "%s", tmp) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->single_operator = WpeStrdup(strcmp(tmp, "NULL") ? tmp : "");
  if (fscanf(syntax_file, "%s", tmp) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->begin_comment = WpeStrdup(strcmp(tmp, "NULL") ? tmp : "");
  if (fscanf(syntax_file, "%s", tmp) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->end_comment = WpeStrdup(strcmp(tmp, "NULL") ? tmp : "");
  if (fscanf(syntax_file, "%s", tmp) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->line_comment = WpeStrdup(strcmp(tmp, "NULL") ? tmp : "");
  if (fscanf(syntax_file, "%s", tmp) != 1)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  new_syntax->syntax_rule->special_comment = WpeStrdup(strcmp(tmp, "NULL") ? tmp : "");
  if (fscanf(syntax_file, " %c%c%c%c%c%c",
             &new_syntax->syntax_rule->string_constant,
             &new_syntax->syntax_rule->char_constant,
             &new_syntax->syntax_rule->preproc_cmd,
             &new_syntax->syntax_rule->quoting_char,
             &new_syntax->syntax_rule->continue_char,
             &new_syntax->syntax_rule->insensitive) != 6)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  /* This is currently a mess but this format of syntax_def is unlikely to
    stay - Dennis */
  if (new_syntax->syntax_rule->string_constant == ' ')
  {
   new_syntax->syntax_rule->string_constant = '\0';
  }
  if (new_syntax->syntax_rule->char_constant == ' ')
  {
   new_syntax->syntax_rule->char_constant = '\0';
  }
  if (new_syntax->syntax_rule->preproc_cmd == ' ')
  {
   new_syntax->syntax_rule->preproc_cmd = '\0';
  }
  if (new_syntax->syntax_rule->quoting_char == ' ')
  {
   new_syntax->syntax_rule->quoting_char = '\0';
  }
  if (new_syntax->syntax_rule->continue_char == ' ')
  {
   new_syntax->syntax_rule->continue_char = '\0';
  }
  if (new_syntax->syntax_rule->insensitive == ' ')
  {
   new_syntax->syntax_rule->insensitive = '\0';
  }
  else
  {
   /* Convert all reserved words to upper case for case insensitive syntax */
   for (i = 0; new_syntax->syntax_rule->reserved_word[i]; i++)
   {
    WpeStringToUpper(new_syntax->syntax_rule->reserved_word[i]);
   }
   /* Convert all long operators to upper case for case insensitive syntax */
   for (i = 0; new_syntax->syntax_rule->long_operator[i]; i++)
   {
    WpeStringToUpper(new_syntax->syntax_rule->long_operator[i]);
   }
  }
  if (fscanf(syntax_file, "%d%d%d", &new_syntax->syntax_rule->special_column,
             &new_syntax->syntax_rule->continue_column,
             &new_syntax->syntax_rule->comment_column) != 3)
  {
   e_error("Error reading syntax_def", 0, cn->fb);
   return ;
  }
  qsort(new_syntax->syntax_rule->reserved_word, reserved_num, sizeof(char *),
        WpeReservedWordCompare);
  qsort(new_syntax->syntax_rule->long_operator, long_op_num, sizeof(char *),
        WpeReservedWordCompare);
  WpeExpArrayAdd((void **)&WpeSyntaxDef, &new_syntax);
 }
}

