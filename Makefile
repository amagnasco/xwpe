# Generated automatically from Makefile.in by configure.
#C-Compiler (gcc if exist):
CC=		gcc
CFLAGS=		-g -O2  -I/usr/X11R6/include
#CFLAGS=		-g -O2 @EXTRA_CFLAGS@  -I/usr/X11R6/include

#Source Directory:
srcdir=		.

#Destination Directory
prefix=/usr/local
exec_prefix=${prefix}
DESTDIR=${exec_prefix}/bin
LIBDIR=${exec_prefix}/lib
XWPELIBDIR=	$(LIBDIR)/xwpe
MANDIR=${prefix}/man

LIBS=-lz -lgpm  -lncurses
XLIBS= -L/usr/X11R6/lib  -lSM -lICE  -lX11
DEFS= -DCC_HAS_PROTOS=1 -DHAVE_LIBGPM=1 -DHAVE_LIBZ=1 -DHAVE_MKDTEMP=1 -DRANDLIB=1 -DPRNTCMD=\"lpr\" -DMAN_S_OPT=1  -DLIBRARY_DIR=\"$(XWPELIBDIR)\" -DSELECTION
INSTALL=	cp
LN_S=	ln -s

# In case configure fails a description of many of the compilation options
# are provided below so that administrators can determine the proper flags.
# Please email any problems to Dennis Payne (dulsi@identicalsoftware.com)

# Recognized defines:
# -DNONEWSTYLE          :Use xwpe-style of version < 1.2.0
# -DNO_XWINDOWS         :Don't create the X windows versions
# -DNOPROG              :Don't create the programming environment (Broken)
# -DNODEBUGGER          :Don't include debugging features (Broken)
# -DNOPRINTER           :No printing from within xwpe
# -DPRNTCMD=\"lpr\"     :Command to print is \"lpr\"
# -DMAN_S_OPT           :Man accepts "-s" to select section
# -DNO_MINUS_C_MINUS_O  :Compiler doesn't accept -o and -c on same command
# -DDEFPGC              :No macros for putc and getc
# -DDEFTPUTS            :No prototype for tputs
# -DNOSTRSTR            :No prototype for strstr and getcwd
# -DRANLIB              :Have to run ranlib after building a library
# -DNOSYMLINKS          :No symbolic links
# -DXDB                 :System has xdb not dbx
# -DTERMCAP             :Use termcap instead of curses
# -DSIMPLE_BACKUP_SUFFIX=\"\" 
#                       : The bak-file suffix, default is TurboC-like
# -DDEFAULT_ALTMASK=Mod3Mask
#                       :Mask to determined if alt is pressed in X
# -DINFO_DIR=\"/usr/local/info\"
#                       :Location of info directory
# -DLIBRARY_DIR=\"/usr/local/lib/xwpe\"
#                       :Location of xwpe library directory
# -DDEF_SHELL=\"/bin/ksh\"
#                       :Default shell to run
# -DXTERM_CMD=\"/usr/X11R6/bin/color_xterm\"
#                       :Define the command to execute an xterm
# -DXWPE_DLL            :Builds x and terminal support into libraries
# -DSELECTION           :Use X Selection events instead of XStoreBytes

# Possible include paths needed:
# -I/usr/X11R5/include  :X include path
# -I/usr/5include       :System V include path

# Possible library paths needed:
# -L/usr/X11R5/lib      :X library path
# -L/usr/5lib           :System V library path

# Possible libraries needed:
# -lX11              :X library
# -lcurses           :Curses library
# -lncurses          :Ncurses library (replaces curses on newer systems)
# -ltermlib          :Termcap library
# -lcposix           :Posix Library
# -linet             :Network library

OFILES=		we_main.o we_block.o we_unix.o we_e_aus.o \
		we_edit.o we_fl_fkt.o we_fl_unix.o we_hfkt.o \
		we_menue.o we_mouse.o we_opt.o we_wind.o \
		we_prog.o we_progn.o we_debug.o WeString.o \
		WeSyntax.o WeExpArr.o WeLinux.o we_gpm.o
X_OFILES=	we_xterm.o WeXterm.o
T_OFILES=	we_term.o

CFILES=		we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c we_term.c \
		we_prog.c we_progn.c we_debug.c we_xterm.c WeString.c \
		WeXterm.c WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c

HFILES=		attrb.h edit.h keys.h \
		model.h progr.h unixkeys.h unixmakr.h \
		Xwpe.h WeString.h WeXterm.h WeProg.h WeExpArr.h

xwpe:	$(OFILES) $(X_OFILES) $(T_OFILES)
	$(CC) $(CFLAGS) $(DEFS) $(OFILES) $(X_OFILES) $(T_OFILES) \
	  $(LIBS) $(XLIBS) -o xwpe
#	$(CC) -Wl,-E $(CFLAGS) $(DEFS) $(OFILES) $(LIBS) -o xwpe

libxwpe-x11.so:	$(X_OFILES)
	$(CC) -shared $(X_OFILES) $(XLIBS) -o libxwpe-x11.so

libxwpe-term.so:	$(T_OFILES)
	$(CC) -shared $(T_OFILES) -o libxwpe-term.so

#$(X_OFILES) $(T_OFILES):	$(HFILES)
#	$(CC) -fPIC $(CFLAGS) $(DEFS) -c $(subst .o,.c,$@)

.c.o:	$(HFILES)
	$(CC) $(CFLAGS) $(DEFS) -c $<

clean:
	rm -f *.o xwpe wpe we xwe libxwpe-x11.so libxwpe-term.so core *.ESV

distclean:
	rm -f config.status config.cache config.log Makefile

# Bug Note: Currently xwpe and xwe are installed even if X windows versions
# are not compiled.  (They will function as wpe and we respectively.)
install_fst:	xwpe
	if test ! -d $(DESTDIR); then mkdir -p $(DESTDIR); fi
	if test ! -d $(XWPELIBDIR); then mkdir -p $(XWPELIBDIR); fi
	rm -f $(DESTDIR)/wpe $(DESTDIR)/xwpe $(DESTDIR)/we $(DESTDIR)/xwe
	$(INSTALL) xwpe $(DESTDIR)/we
	chmod ugo+x $(DESTDIR)/we
	(cd $(DESTDIR) && $(LN_S) we xwe)
	(cd $(DESTDIR) && $(LN_S) we xwpe)
	(cd $(DESTDIR) && $(LN_S) we wpe)
	rm -f $(XWPELIBDIR)/help.xwpe
	if test ! -f $(XWPELIBDIR)/syntax_def; then  \
	  $(INSTALL) syntax_def $(XWPELIBDIR)/syntax_def; \
	  chmod ugo=r $(XWPELIBDIR)/syntax_def; fi
	if test ! -d $(MANDIR)/man1; then mkdir -p $(MANDIR)/man1; fi
	rm -f $(MANDIR)/man1/xwpe.1 $(MANDIR)/man1/xwe.1 
	rm -f $(MANDIR)/man1/wpe.1 $(MANDIR)/man1/we.1 
	$(INSTALL) xwe.1 wpe.1 we.1 $(MANDIR)/man1
	chmod ugo=r $(MANDIR)/man1/xwe.1 
	chmod ugo=r $(MANDIR)/man1/wpe.1 $(MANDIR)/man1/we.1 
	if test -f libxwpe-x11.so; then \
	  $(INSTALL) libxwpe-x11.so $(XWPELIBDIR)/libxwpe-x11.so; fi
	if test -f libxwpe-term.so; then \
	  $(INSTALL) libxwpe-term.so $(XWPELIBDIR)/libxwpe-term.so; fi

install:	install_fst
	$(INSTALL) help.xwpe_eng $(XWPELIBDIR)/help.xwpe
	chmod ugo=r $(XWPELIBDIR)/help.xwpe
	$(INSTALL) help.key_eng $(XWPELIBDIR)/help.key
	chmod ugo=r $(XWPELIBDIR)/help.key
	$(INSTALL) xwpe.1_eng $(MANDIR)/man1/xwpe.1
	chmod ugo=r $(MANDIR)/man1/xwpe.1

# German install uses old documention as no updated version is available
install_german:	install_fst
	$(INSTALL) old/we.help_gr $(XWPELIBDIR)/help.xwpe
	chmod ugo=r $(XWPELIBDIR)/help.xwpe
	$(INSTALL) old/xwpe.1_gr $(MANDIR)/man1/xwpe.1
	chmod ugo=r $(MANDIR)/man1/xwpe.1

