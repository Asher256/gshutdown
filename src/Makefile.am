INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" \
	-DPACKAGE_LOCALE_DIR=\""$(prefix)/$(DATADIRNAME)/locale"\" \
	@PACKAGE_CFLAGS@

bin_PROGRAMS = gshutdown
gshutdown_SOURCES = \
	args.c args.h \
	debug.c debug.h \
	binreloc.c binreloc.h \
	detectwm.c detectwm.h \
	values.c values.h \
	callback.c callback.h \
	callback_main_window.c callback_main_window.h \
	misc.c misc.h \
	timer.c timer.h \
	action.c action.h \
	gui.c gui.h \
	main.c main.h \
	gdm.c gdm.h \
	pconfirm.c pconfirm.h \
	bacon.c bacon.h \
	eggtrayicon.c eggtrayicon.h

gshutdown_LDADD = @PACKAGE_LIBS@ $(INTLLIBS)
gshutdown_LDFLAGS = -Wl,-export-dynamic

sharedir = $(pkgdatadir)
share_DATA = tray_icon2.png tray_icon.png gshutdown.png clock.png gshutdown.glade

Pixmapsdir = $(datadir)/pixmaps
Pixmaps_DATA = gshutdown.png

EXTRA_DIST = $(share_DATA)

