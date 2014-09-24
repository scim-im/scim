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
CLEANFILES		= *.bak scim-setup.desktop scim-setup

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
	    	  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\"

noinst_HEADERS		= scim_setup_module.h \
			  scim_setup_ui.h

if SCIM_BUILD_SCIM_SETUP
CONFIG_SCIM_SETUP = scim-setup
CONFIG_SCIM_SETUP_HELPER = setup.la
endif

bin_SCRIPTS		= $(CONFIG_SCIM_SETUP)

moduledir		= $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/Helper

module_LTLIBRARIES	= $(CONFIG_SCIM_SETUP_HELPER)

setup_la_SOURCES	= scim_setup_ui.cpp \
			  scim_setup_module.cpp \
			  scim_setup_helper.cpp

setup_la_CXXFLAGS	= @GTK_CFLAGS@

setup_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @GTK_LIBS@ \
			  @LTLIBINTL@

setup_la_LIBADD		= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

if SCIM_BUILD_SCIM_SETUP
@INTLTOOL_DESKTOP_RULE@

desktopdir       = $(datadir)/applications
desktop_in_files = scim-setup.desktop.in
desktop_DATA     = $(desktop_in_files:.desktop.in=.desktop)

gnomeccdir       = $(datadir)/control-center-2.0/capplets
gnomecc_DATA     = $(desktop_in_files:.desktop.in=.desktop)
endif

EXTRA_DIST = $(desktop_in_files) scim-setup.in

