## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2002 James Su
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program; if not, write to the Free Software
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

MAINTAINERCLEANFILES	= Makefile.in
CLEANFILES		= *.bak
EXTRA_DIST		= scim.xml.in

# IBus component manifest: fill in the installed scim path and version, then
# install into ibus-daemon's component directory.
if SCIM_BUILD_FRONTEND_IBUS
componentdir		= @IBUS_COMPONENT_DIR@
component_DATA		= scim.xml
CLEANFILES		+= scim.xml

scim.xml: scim.xml.in Makefile
	$(AM_V_GEN) $(SED) -e 's|@bindir[@]|$(bindir)|g' \
	                   -e 's|@VERSION[@]|$(VERSION)|g' \
	                   $(srcdir)/scim.xml.in > $@
endif

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
	    	  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/data \
			  -I$(top_srcdir)/utils/x11utils \
			  -I$(top_srcdir)/extras/panel \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\"

if SCIM_BUILD_FRONTEND_X11
CONFIG_FRONTEND_X11_MODULE	= x11.la
SUBDIRS				= IMdkit
endif

# Draw candidates in-process with the Cairo renderer when it was built.
if SCIM_BUILD_CANDIDATES
PANEL_UI_CFLAGS		= -DSCIM_HAS_CANDIDATES \
			  -I$(top_srcdir)/utils/candidates \
			  @CAIRO_CFLAGS@ @CAIRO_XLIB_CFLAGS@
PANEL_UI_LIBS		= $(top_builddir)/utils/candidates/libscim-candidates-x11@SCIM_EPOCH@.la \
			  @CAIRO_LIBS@ @CAIRO_XLIB_LIBS@
endif

# Delegate candidates to KDE kimpanel over D-Bus when it was built.
if SCIM_BUILD_KIMPANEL
KIMPANEL_CFLAGS		= -DSCIM_HAS_KIMPANEL \
			  -I$(top_srcdir)/panel-ui @DBUS_CFLAGS@
KIMPANEL_LIBS		= $(top_builddir)/panel-ui/libscimkimpanel.la @DBUS_LIBS@
endif

if SCIM_BUILD_FRONTEND_SOCKET
CONFIG_FRONTEND_SOCKET_MODULE	= socket.la
endif

if SCIM_BUILD_FRONTEND_WAYLAND
CONFIG_FRONTEND_WAYLAND_MODULE	= wayland.la
endif

if SCIM_BUILD_FRONTEND_IBUS
CONFIG_FRONTEND_IBUS_MODULE	= ibus.la
endif

noinst_HEADERS		= scim_x11_ic.h \
		  	  scim_x11_frontend.h \
			  scim_socket_frontend.h \
			  scim_wayland_frontend.h \
			  scim_ibus_frontend.h

moduledir		= $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/FrontEnd

module_LTLIBRARIES	= $(CONFIG_FRONTEND_X11_MODULE) \
		     	  $(CONFIG_FRONTEND_SOCKET_MODULE) \
		     	  $(CONFIG_FRONTEND_WAYLAND_MODULE) \
		     	  $(CONFIG_FRONTEND_IBUS_MODULE)

x11_la_SOURCES 		= scim_x11_frontend.cpp \
			  scim_x11_ic.cpp

x11_la_CFLAGS		= @X_CFLAGS@ $(PANEL_UI_CFLAGS) $(KIMPANEL_CFLAGS)

x11_la_CXXFLAGS		= @X_CFLAGS@ $(PANEL_UI_CFLAGS) $(KIMPANEL_CFLAGS)

x11_la_LDFLAGS		= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @X_LIBS@ \
			  @LTLIBINTL@

x11_la_LIBADD		= IMdkit/libXimd.la \
			  $(top_builddir)/src/libscim@SCIM_EPOCH@.la \
			  $(top_builddir)/utils/x11utils/libscim-x11utils@SCIM_EPOCH@.la \
			  $(PANEL_UI_LIBS) \
			  $(KIMPANEL_LIBS)

socket_la_SOURCES 	= scim_socket_frontend.cpp

socket_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@

socket_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

ibus_la_SOURCES 	= scim_ibus_frontend.cpp

ibus_la_CFLAGS		= @IBUS_CFLAGS@ -DSCIM_BINDIR='"$(bindir)"'

ibus_la_CXXFLAGS	= @IBUS_CFLAGS@ -DSCIM_BINDIR='"$(bindir)"'

ibus_la_LDFLAGS		= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @IBUS_LIBS@ \
			  @LTLIBINTL@

ibus_la_LIBADD		= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

wayland_la_SOURCES 	= scim_wayland_frontend.cpp

wayland_la_CFLAGS	= @WAYLAND_CFLAGS@ @CAIRO_CFLAGS@ \
			  -DSCIM_HAS_CANDIDATES_WAYLAND \
			  -I$(top_builddir)/protocols \
			  -I$(top_srcdir)/utils/candidates \
			  $(KIMPANEL_CFLAGS)

wayland_la_CXXFLAGS	= @WAYLAND_CFLAGS@ @CAIRO_CFLAGS@ \
			  -DSCIM_HAS_CANDIDATES_WAYLAND \
			  -I$(top_builddir)/protocols \
			  -I$(top_srcdir)/utils/candidates \
			  $(KIMPANEL_CFLAGS)

wayland_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @WAYLAND_LIBS@ \
			  @CAIRO_LIBS@ \
			  @LTLIBINTL@

wayland_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la \
			  $(top_builddir)/protocols/libscimwlprotocols.la \
			  $(top_builddir)/utils/candidates/libscim-candidates-wayland.la \
			  $(KIMPANEL_LIBS)

