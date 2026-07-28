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

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl

if SCIM_BUILD_GTK_UTILS
libscimgtkutilsincludedir = $(includedir)/scim@SCIM_EPOCH@/gtk
libscimgtkutilsinclude_HEADERS = scimstringview.h scimkeyselection.h
GTK_UTILS = libscim-gtkutils@SCIM_EPOCH@.la
endif

lib_LTLIBRARIES = $(GTK_UTILS)

libscim_gtkutils@SCIM_EPOCH@_la_SOURCES = scimstringview.c \
				  scimkeyselection.cpp

libscim_gtkutils@SCIM_EPOCH@_la_CFLAGS = @GTK_CFLAGS@
libscim_gtkutils@SCIM_EPOCH@_la_CXXFLAGS= @GTK_CFLAGS@

libscim_gtkutils@SCIM_EPOCH@_la_LDFLAGS= -version-info $(SCIM_CURRENT):$(SCIM_REVISION):$(SCIM_AGE) \
				  -export-dynamic \
				  @LIBTOOL_EXPORT_OPTIONS@ \
				  @LTLIBINTL@ \
				  @GTK_LIBS@

libscim_gtkutils@SCIM_EPOCH@_la_LIBADD= $(top_builddir)/src/libscim@SCIM_EPOCH@.la
