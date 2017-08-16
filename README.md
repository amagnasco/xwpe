# XWPE-GIT README #

* Master branch [![Build Status](https://travis-ci.org/amagnasco/xwpe.svg?branch=master)](https://travis-ci.org/amagnasco/xwpe)
* Development [![Build Status](https://travis-ci.org/amagnasco/xwpe.svg?branch=devel)](https://travis-ci.org/amagnasco/xwpe)
* Experimental [![Build Status](https://travis-ci.org/amagnasco/xwpe.svg?branch=experimental)](https://travis-ci.org/amagnasco/xwpe)

## What is xwpe? ##

Xwpe is a development environment designed for use on UNIX
systems.  Fred Kruse wrote xwpe and released the software for free
under the GNU Public License.  The user interface was designed to
mimic the Borland C and Pascal family.  Extensive support is provided
for programming.

Syntax highlighting for many programming languages are included
and others may be added if necessary.  Any compiler can easily be
used by the program.  By compiling within xwpe, errors in the source
code can be jumped to and swiftly corrected.  Support for three
different debuggers are provided within the development environment.
Variables and the stack can be easily displayed.  Setting and
unsetting breakpoints can done directly within the source code.

The program can be run in several forms.  Xwpe runs the X
windows version of the programming environment.  Wpe simply runs a
terminal programming environment.  Xwe and we provide a simple text
editor for X windows and terminal modes respectively.

Online help describes the complete use of xwpe.

## What is xwpe-alpha? ##

The xwpe-alpha project was an attempt to reorganize the source code to
improve readability.  It also incorporated as many bug fixes as
available.  Attempts to contact the author of xwpe have received no
response so xwpe-alpha should be considered unsupported by Fred Kruse.
To signify the difference xwpe-alpha has increased the version number
from 1.4.x to 1.5.x despite relatively few changes at present.  Also
xwpe-alpha release end in 'a' (e.g. 1.5.4a).

## What is xwpe-git? ##

Since more than a decade has passed since the xwpe-alpha project closed,
the code has been ported to GitHub and made available to the community.
In keeping with the spirit of xwpe-alpha, xwpe-git has increased the 
version number from 1.5.x to 1.6.x despite relatively few changes as well.

## Why release it now? ##

The source code is still largely unchanged at this point.  The purpose
in the release is to discover problems early in the modifications.  Since
not all platforms are accessible to the developers come changes could
inadvertently break compilation on another system.  Also understanding and
modifying the structure of xwpe is a large undertaking and will take a long
time to complete.

## Use ##

See INSTALL for generic instructions on compile and install. This
README is tailored to our project and how we work. Get
to the main project directory and execute:

    `autoreconf -iv`
    `mkdir build # if necessary`
    `cd build`
    `../configure ` 
    `make`
    `sudo make install`

From then on xwpe/xwe/wpe/we will be installed in /usr/local/bin. If you
don't have X11, only wpe/we will be installed.

Should you want to check the result, do a `make check`. This will execute the 
testprograms running from the framework `check`.

## Documentation ##

The original documentation is mostly part of the program in the form of info
files accessible through the menu. Additionally a man page was provided.

### Doxygen ###

For refactoring of the program we use the `doxygen` software package.
It generates documentation from the source and diagrams for the struct 
defines. This eases our refactoring of the system.

If you don't have `doxygen` installed, the program will not generate anything. 

If you do have `doxygen` installed, make sure you also have installed `latex` plus among other 
`latex` modules `adjustbox`. Finally, make sure you have `graphviz` installed. It generates
the beautiful graphs with dot.

For Ubuntu we have installed the packages:

* `texlive`
* `textlive-latex-extra`
* `graphviz`
* `doxygen`
* `doxygen-latex`
* `doxygen-doc`
* `doxygen-gui`

For Fedora you need to install the packages:

* `doxygen`
* `doxygen-doxywizard`
* `graphviz`
* `texlive`
* `texlive-tocloft`
* `texlive-adjustbox`
* `texlive-tabu`

For other distributions you will have to figure out the exact package names needed.

You might have to check with the doxygen website to check for required software as this
may change over time.

REMARK: 

* Always compile from a separate build directory to prevent cluttering the source
directory. Should you have used `./configure` from the project root first,
then do a `make distclean` first, to remove cached files. 
Then follow the above instructions.

## Development ##

if you want to develop and use debugging do:

    `../configure CFLAGS="-g -O0"`

from the build directory in stead of `../configure`. This enables the debugger
to show the contents of all accessible variables. You can still debug with `../configure`,
but the default compile options are `-g -O2`, which means sometimes variables will be 
`optimized out`.

For debugging I recommend using `cgdb` in place of `gdb`. It is an ncurses interface
where you see the code in the top half and `gdb` in the lower half.

## PDF ##

If you want this file or any other markdown file (.md files) in PDF,
then you could use the tool `pandoc`. In its simplest form do: 

    `pandoc -o README.pdf README.md`.

## Copyright ##

Copyright (C) 1993 Fred Kruse. xwpe is free; anyone may
redistribute copies of xwpe to anyone under the terms stated in the
GNU General Public License.  The author assumes no responsibility
for errors or omissions or damages resulting from the use of xwpe or
this manual.

## Maintainer ##

This version is an unofficial update to xwpe and is not supported 
by Fred Kruse. Updates have previously been made available on the unofficial 
xwpe-alpha homepage, http://www.identicalsoftware.com/xwpe/. The previous
maintainer was Dennis Payne, dulsi@identicalsoftware.com.

As evidenced by their discussion list, xwpe-alpha has not seen active 
development since at least 2003. This GitHub version is maintained by 
Alessandro G. Magnasco at https://github.com/amagnasco/xwpe/. 

Guus Bonnema is contributor, bravely trying to refactor and clean up 
this mess: https://github.com/gbonnema.
    
Caveat emptor ; this software hails from last century. I can try to help
figure out any questions or problems you might have, but purely out of 
academic interest. Solutions are left as an exercise for the hacker.
If you have a patch, please let me know so we can review it.
    
## DEPENDENCIES ##

If you are having difficulties compiling your code, be aware that xwpe
requires a backport version of libcurses. As most modern systems should keep 
far, far away from the original UNIX library, at time of writing (02017-05)
this dependency is satisfied by libncurses5-dev or libncurses6.
    
## ENJOY ! ##
