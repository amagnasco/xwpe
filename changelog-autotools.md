* Autotools changes *

June 2017 Guus Bonnema Adopting autotool files to the newer conventions and checking the 
contents for obsolete macros.

* Moved `configure.in` and handmade `Makefile.in` to `*.in.old`
* Remark: using autoconf version 2.69 in stead of 2.13
* Created `Makefile.am` to use subdirs. Created src subdirectory for sources.
* Added man subdir for manpages
* Created `Makefile.am` in src to do the source compile. 
* Created `configure.ac` (using `autoscan` to get started).
* Used a generated aclocals.m4 in stead of the handcrafted aclocals.m4 by T.E. Dickey, Jim Spath and
  Philippe De Muyter from 1997. We only used one macro for ensuring ansi. This macro is no
  longer necessary as we compile with all warnings on.
* We use a subdirectory `m4` to distribute local and global macro's used while using aclocal. 
  For that reason call `autoreconf` with the option `--install` (or `-i`). 
  This ensures a copy of globally defined macro's are distributed as well.
* Each local m4 macro has it's own file.
* Each local m4 macro will have the line `#serial nnn` where `nnn` is the version number of the macro. This
  way users can distinguish older from newer macro files
  We have no specific macro's to start with, but one may be necessary in future.
* Added config.h for defines of constants.
* Removed defines and if[n]def for DJGPP completely.
* Added `LIBRARY_DIR` for storing datafiles 
* Created libraries for term and x11 (`libxwpe-x11` and `libxwpe-term`) 
  using libtool for portability
* Dropped support for old German helpfiles as they no longer represent the current helpfiles.
* Changed define `NOSYMLINKS` to `HAVE_SYMLINK` which is standard for `AC_CHECK_FUNCS`.

Changes necessary to get compile error free:

* Added config.h to `we_main.c` and `we_opt.c`.
* Removed duplicate definition of VERSION from edit.h (is now in `config.h`)
* Added several `AC_CHECK_LIB` (ncurses, SM and ICE).
  Decided to support ncurses only (not the older curses or termlib alone)
* The old define NCURSES was never set in Makefile.in or configure.in. This means, that
  every test for #ifdef NCURSES results in false i.e. same as #if FALSE, while every test for
  #ifndef NCURSES results in true, i.e. same as #if TRUE. Replacing the tests for NCURSES was
  different than for the other defines. I had to judge every test separately, I could not do a
  global change like with the other defines.
* The old defines for CURSES and TERMCAP had special semantics. `CURSES` was defined in configure
  if and only if either ncurses lib or curses lib was present. If `CURSES` was not set, then
  automatically TERMCAP was set, irrespective of the presence of the termcap lib or termlib lib.
  This implies that *every* test for `CURSES` or for `TERMCAP` has to be studied separately and
  treated carefully to change it to the correct `HAVE_XXXLIB`.
