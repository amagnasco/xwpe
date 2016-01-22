#ifndef __WESTRING_H
#define __WESTRING_H
/*-------------------------------------------------------------------------*\
  <WeString.h> -- Header file for some string routines for xwpe

  Date      Programmer  Description
  04/27/97  Dennis      Created for xwpe reorganization.
\*-------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStrnccmp - Case-insensitive compare of two strings for a number of
characters.

    Parameters:
      s1           (In)  First string
      s2           (In)  Second string
      n            (In)  Maximum number of characters to compare
    Returns: Zero if equal.  Anything else means not equal.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int WpeStrnccmp(const char *s1, const char *s2, int n);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStrccmp - Case-insensitive compare of two strings.

    Parameters:
      s1           (In)  First string
      s2           (In)  Second string
    Returns: Zero if equal.  Anything else means not equal.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int WpeStrccmp(const char *s1, const char *s2);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStrcstr - Case-insensitive substring search.

    Parameters:
      str          (In)  Main string
      substr       (In)  Substring to search for
    Returns: Pointer to the beginning of the substring in the main string.
  NULL if the substring is not found.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char *WpeStrcstr(char *str, const char *substr);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStrdup - Duplicate a string.

    Parameters:
      str          (In)  String to copy
    Returns: Pointer to new string created with WpeMalloc().  NULL if
  insufficient memory free to create the string.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char *WpeStrdup(const char *str);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeNumberOfPlaces - Number of places in a number excluding the sign.

    Parameters:
      n            (In)  Number
    Returns: The number of places.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int WpeNumberOfPlaces(int n);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeNumberToString - Converts a number to a string.

    Parameters:
      n            (In)  Number
      len          (In)  Length of the string (excluding '\0')
      s            (In & Out) Converted number
    Returns: Pointer to the converted number string
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char *WpeNumberToString(int n, int len, char *s);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStringToNumber - Converts a string to a number.

    Parameters:
      s            (In)  String to be converted to a number
    Returns: The string's number value
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
int WpeStringToNumber(const char *s);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStringToUpper - Converts a string to all uppercase.

    Parameters:
      s            (In & Out) String to be converted to uppercase
    Returns: Pointer to the uppercase string
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char *WpeStringToUpper(char *s);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStringBlank - Fills a string with blank spaces.

    Parameters:
      s            (In & Out) String to be blanked
      len          (In)  Length of the string (excluding '\0')
    Returns: The blanked string.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char *WpeStringBlank(char *s, int len);

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - *\
  WpeStringCutChar - Cuts a string at the last occurance of a character.

    Parameters:
      s            (In & Out) String to be cut
      c            (In) Character to cut off
    Returns: The cut string.
\* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
char *WpeStringCutChar(char *s, char c);

#ifdef __cplusplus
}
#endif

#endif

