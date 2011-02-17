## Process this file with automake to produce Makefile.in

if BUILD_GUI

bin_PROGRAMS = nekostring_gtk
else

bin_PROGRAMS =
endif

INCLUDES = \
	-DPACKAGE_DATA_DIR=\""$(datadir)"\" 
#	@PACKAGE_CFLAGS@

nekostring_gtk_CFLAGS = @GTK_CFLAGS@ $(AM_CFLAGS)
bindir = $(libdir)/dssi/nekostring

nekostring_gtk_SOURCES = \
	main.c \
	support.c support.h \
	interface.c interface.h \
	callbacks.c callbacks.h \
	neko_ports.c \
	widgets.h

nekostring_gtk_LDADD = @PACKAGE_LIBS@ -lm -llo -lpthread

_SOURCES = osc.c
