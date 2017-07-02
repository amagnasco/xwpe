* Autotools changes *

June 2017 Guus Bonnema Adopting autotool files to the newer conventions and checking the 
contents for obsolete macros.

* Moved `configure.in` and handmade `Makefile.in` to `*.in.old`
* Remark: using autoconf version 2.69 in stead of 2.13
* Created `Makefile.am` to use subdirs. Created src subdirectory for sources.
* Created `Makefile.am` in src to do the source compile. 
* Created `configure.ac` (using `autoscan` to get started).
* Used a generated aclocals.m4 in stead of the handcrafted aclocals.m4 by T.E. Dickey, Jim Spath and
  Philippe De Muyter from 1997. We only used one macro for ensuring ansi. This macro is no
  longer necessary as we compile with all warnings on.
* We use a subdirectory `m4` to distribute local and global macro's used while using aclocal. For that
  reason we call `autoreconf` with the option `--install`. 
  This ensures a copy of globally defined macro's are distributed as well.
* Each local m4 macro has it's own file.
* Each local m4 macro will have the line `#serial nnn` where `nnn` is the version number of the macro. This
  way users can distinguish older from newer macro files
  We have no specific macro's to start with, but one may be necessary in future.
* Added config.h for defines of constants.
* Removed defines and if[n]def for DJGPP completely.

Changes necessary to get compile error free:

* Added config.h to `we_main.c` and `we_opt.c`.
* Removed duplicate definition of VERSION from edit.h (is now in `config.h`)
* Added several `AC_CHECK_LIB` (ncurses, SM and ICE).
