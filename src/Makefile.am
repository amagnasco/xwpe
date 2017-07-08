bin_PROGRAMS = wpe we 
AM_CPPFLAGS = -DLIBRARY_DIR=\"@libdir@/xwpe\"
lib_LTLIBRARIES = libxwpe-x11.la libxwpe-term.la
libxwpe_x11_la_SOURCES = we_xterm.c WeXterm.c
libxwpe_term_la_SOURCES = we_term.c
wpe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
wpe_LDADD =	libxwpe-x11.la libxwpe-term.la
we_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
we_LDADD =	libxwpe-x11.la libxwpe-term.la
include_HEADERS = 	attrb.h edit.h keys.h \
		model.h unixkeys.h unixmakr.h \
		WeString.h WeXterm.h WeProg.h WeExpArr.h
if HAVE_X11
   bin_PROGRAMS += xwpe xwe
xwpe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
xwpe_LDADD =	libxwpe-x11.la libxwpe-term.la
#xwpe_CFLAGS = @X_CFLAGS@
#xwpe_LDFLAGS = @X_LIBS@ -lX11
xwe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
xwe_LDADD =	libxwpe-x11.la libxwpe-term.la
#xwe_CFLAGS = @X_CFLAGS@
#xwe_LDFLAGS = @X_LIBS@ -lX11
endif

