* Autotools usage *

** Normal usage **

Normally you can do:

* `./configure`
* `make`
* `sudo make install`

to create binaries from a fresh checkout. The `make` command should detect
any source changes. If you added features to your system (like X-windows or ncurses)
you may want to rerun `./configure` and `make clean` before you run `make`.

** Developing **

As a developer, if you want to change configure.ac or Makefile.am you need to run
the above sequence after running `autoreconf --install -vv`. Including `--install` 
assures that global macros are copied in a new. Including `-vv` makes `autoreconf` report
progress and warnings. 

If you did not change anything in configure.ac or Makefile.am, but you want to be sure
everything runs ok on your system and is detected properly, you can still run `autoreconf`.

