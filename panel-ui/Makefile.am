## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Cairo+Pango input-panel renderer (own-Cairo candidate UI) plus its per-
## backend surface shims (X11 cairo-xlib window, Wayland input-popup-surface),
## built into convenience libraries the x11/wayland FrontEnd modules link to
## draw candidates in-process.

MAINTAINERCLEANFILES	= Makefile.in
CLEANFILES		= *.bak

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  @CAIRO_CFLAGS@

noinst_HEADERS		= scim_panel_ui.h \
			  scim_panel_ui_x11.h \
			  scim_panel_ui_wayland.h \
			  scim_kimpanel_agent.h

noinst_LTLIBRARIES	=

# X11 shim: renderer core + cairo-xlib override-redirect window.
if SCIM_BUILD_PANEL_UI
noinst_LTLIBRARIES	+= libscimpanelui.la
libscimpanelui_la_SOURCES  = scim_panel_ui.cpp \
			     scim_panel_ui_x11.cpp
libscimpanelui_la_CPPFLAGS = $(AM_CPPFLAGS) @CAIRO_XLIB_CFLAGS@ @X_CFLAGS@
libscimpanelui_la_LIBADD   = @CAIRO_LIBS@ @CAIRO_XLIB_LIBS@ @X_LIBS@
endif

# Wayland shim: renderer core + input-popup-surface-v2 backend. Links the
# vendored protocol glue at the final module link (wayland.so).
if SCIM_BUILD_FRONTEND_WAYLAND
noinst_LTLIBRARIES	+= libscimpanelui-wayland.la
libscimpanelui_wayland_la_SOURCES  = scim_panel_ui.cpp \
				     scim_panel_ui_wayland.cpp
libscimpanelui_wayland_la_CPPFLAGS = $(AM_CPPFLAGS) @WAYLAND_CFLAGS@ \
				     -I$(top_builddir)/protocols
libscimpanelui_wayland_la_LIBADD   = @CAIRO_LIBS@ @WAYLAND_LIBS@
endif

# kimpanel: KDE D-Bus candidate UI backend (no cairo; pure libdbus + scim).
if SCIM_BUILD_KIMPANEL
noinst_LTLIBRARIES	+= libscimkimpanel.la
libscimkimpanel_la_SOURCES  = scim_kimpanel_agent.cpp
libscimkimpanel_la_CPPFLAGS = -I$(top_builddir) -I$(top_builddir)/src \
			      -I$(top_srcdir) -I$(top_srcdir)/src @DBUS_CFLAGS@
libscimkimpanel_la_LIBADD   = @DBUS_LIBS@
endif
