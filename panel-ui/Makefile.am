## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Cairo+Pango input-panel renderer (own-Cairo candidate UI) and its X11
## surface shim, built into a convenience library shared by the x11/wayland
## FrontEnd modules that draw candidates in-process.

MAINTAINERCLEANFILES	= Makefile.in
CLEANFILES		= *.bak

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  @CAIRO_CFLAGS@ \
			  @CAIRO_XLIB_CFLAGS@ \
			  @X_CFLAGS@

noinst_HEADERS		= scim_panel_ui.h \
			  scim_panel_ui_x11.h

if SCIM_BUILD_PANEL_UI

noinst_LTLIBRARIES	= libscimpanelui.la

libscimpanelui_la_SOURCES = scim_panel_ui.cpp \
			    scim_panel_ui_x11.cpp

libscimpanelui_la_LIBADD  = @CAIRO_LIBS@ \
			    @CAIRO_XLIB_LIBS@ \
			    @X_LIBS@

endif
