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
EXTRA_DIST		= config.version-script

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
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\"

moduledir		= $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/Config

noinst_HEADERS		= scim_simple_config.h \
			  scim_socket_config.h

if SCIM_BUILD_CONFIG_SIMPLE
CONFIG_SIMPLE_MODULE	= simple.la
endif

if SCIM_BUILD_CONFIG_SOCKET
CONFIG_SOCKET_MODULE	= socket.la
endif

if SCIM_LD_VERSION_SCRIPT
LD_VERSION_SCRIPT_OPTION="-Wl,--version-script=$(srcdir)/config.version-script"
endif

module_LTLIBRARIES	= \
		     	  $(CONFIG_SIMPLE_MODULE) \
		     	  $(CONFIG_SOCKET_MODULE)

simple_la_SOURCES 	= scim_simple_config.cpp

simple_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  $(LD_VERSION_SCRIPT_OPTION) \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@

simple_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

socket_la_SOURCES 	= scim_socket_config.cpp

socket_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  $(LD_VERSION_SCRIPT_OPTION) \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@

socket_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

