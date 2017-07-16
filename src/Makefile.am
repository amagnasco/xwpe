bin_PROGRAMS = wpe we 
AM_CFLAGS = -Wall -Wextra
AM_CPPFLAGS = -DLIBRARY_DIR=\"@libdir@/xwpe\"
AM_CPPFLAGS += -DRANLIB=\"@RANLIB@\"
AM_CPPFLAGS += -DDATADIR=\"@datadir@/xwpe\"
lib_LTLIBRARIES = libxwpe-x11.la libxwpe-term.la
libxwpe_x11_la_SOURCES = we_xterm.c WeXterm.c
libxwpe_x11_la_LDFLAGS = -version-info 0:0:0
libxwpe_term_la_SOURCES = we_term.c
libxwpe_term_la_LDFLAGS = -version-info 0:0:0
wpe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c \
		utils.c
wpe_LDADD =	libxwpe-x11.la libxwpe-term.la
we_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c \
		utils.c
we_LDADD =	libxwpe-x11.la libxwpe-term.la
include_HEADERS = 	attrb.h edit.h keys.h \
		model.h unixkeys.h unixmakr.h \
		WeString.h WeXterm.h WeProg.h WeExpArr.h \
		utils.h
if HAVE_X11
   bin_PROGRAMS += xwpe xwe
xwpe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c  \
		utils.c
xwpe_LDADD =	libxwpe-x11.la libxwpe-term.la
xwe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c \
		we_prog.c we_progn.c we_debug.c WeString.c \
		WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c \
		utils.c
xwe_LDADD =	libxwpe-x11.la libxwpe-term.la
endif

