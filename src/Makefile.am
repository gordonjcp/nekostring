## Process this file with automake to produce Makefile.in

bindir = $(libdir)/dssi/nekostring

plugindir = $(libdir)/dssi

plugin_LTLIBRARIES = nekostring.la

nekostring_la_LDFLAGS = -module -avoid-version -Wc,-nostartfiles

nekostring_la_SOURCES = \
	gui_data.h \
	gui_friendly_patches.c \
	minblep_tables.c \
	neko-dssi.c \
	neko.h \
	neko_data.c \
	neko_ports.c \
	neko_ports.h \
	neko_synth.c \
	neko_synth.h \
	neko_types.h \
	neko_voice.c \
	neko_voice.h \
	neko_ensemble.c \
	neko_voice_render.c

nekostring_la_LIBADD = -lm

