bin_PROGRAMS = wpe we 
wpe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c we_term.c \
		we_prog.c we_progn.c we_debug.c we_xterm.c WeString.c \
		WeXterm.c WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
we_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c we_term.c \
		we_prog.c we_progn.c we_debug.c we_xterm.c WeString.c \
		WeXterm.c WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
include_HEADERS = 	attrb.h edit.h keys.h \
		model.h unixkeys.h unixmakr.h \
		WeString.h WeXterm.h WeProg.h WeExpArr.h
if HAVE_X11
   bin_PROGRAMS += xwpe xwe
	xwpe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c we_term.c \
		we_prog.c we_progn.c we_debug.c we_xterm.c WeString.c \
		WeXterm.c WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
	xwpe_CFLAGS = @X_CFLAGS@
	xwpe_LDFLAGS = @X_LIBS@ -lX11
	xwe_SOURCES =	we_main.c we_block.c we_unix.c we_e_aus.c \
		we_edit.c we_fl_fkt.c we_fl_unix.c we_hfkt.c \
		we_menue.c we_mouse.c we_opt.c we_wind.c we_term.c \
		we_prog.c we_progn.c we_debug.c we_xterm.c WeString.c \
		WeXterm.c WeSyntax.c WeExpArr.c WeLinux.c we_gpm.c 
	xwe_CFLAGS = @X_CFLAGS@
	xwe_LDFLAGS = @X_LIBS@ -lX11
endif

