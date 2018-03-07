bin_PROGRAMS = wpe we 
AM_CFLAGS = -Wall -Wextra -std=c11
AM_CPPFLAGS = -DLIBRARY_DIR=\"@libdir@/xwpe\"
AM_CPPFLAGS += -DRANLIB=\"@RANLIB@\"
AM_CPPFLAGS += -DDATADIR=\"@datadir@/xwpe\"
AM_LDFLAGS = -rdynamic
lib_LTLIBRARIES = libxwpe.la
libxwpe_la_SOURCES = we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_file_fkt.c we_file_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c \
		utils.c we_find.c we_control.c \
		we_xterm.c WeXterm.c we_term.c we_screen.c
libxwpe_la_LDFLAGS = -version-info 0:0:0
wpe_SOURCES =	we_main.c 
wpe_LDADD =	libxwpe.la
we_SOURCES =	we_main.c 
we_LDADD =	libxwpe.la
include_HEADERS = 	attrb.h edit.h globals.h keys.h \
		messages.h model.h options.h unixkeys.h unixmakr.h \
		utils.h we_block.h we_control.h we_debug.h \
		we_e_aus.h we_edit.h WeExpArr.h we_find.h \
		we_file_fkt.h we_file_unix.h we_gpm.h we_hfkt.h \
		WeLinux.h we_main.h we_menue.h we_mouse.h \
		we_opt.h we_prog.h WeProg.h we_progn.h \
		WeString.h WeSyntax.h we_term.h we_unix.h \
		we_wind.h we_xterm.h WeXterm.h we_screen.h
if HAVE_X11
   bin_PROGRAMS += xwpe xwe
xwpe_SOURCES = we_main.c 
xwpe_LDADD = libxwpe.la
xwe_SOURCES = we_main.c 
xwe_LDADD =	libxwpe.la
endif

