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

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
	    	  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/utils \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\"

if SCIM_BUILD_FILTER_SCTC
CONFIG_FILTER_SCTC = sctc.la
endif

noinst_HEADERS		= scim_sctc_filter.h \
			  scim_sctc_filter_data.h

moduledir		= $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/Filter
module_LTLIBRARIES	= $(CONFIG_FILTER_SCTC)

sctc_la_SOURCES 	= scim_sctc_filter.cpp

sctc_la_LDFLAGS		= -avoid-version \
		     	  -rpath $(moduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@

sctc_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

