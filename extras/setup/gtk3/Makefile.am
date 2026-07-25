## Makefile.am -- Process this file with automake to produce Makefile.in
## Copyright (C) 2026 SCIM developers
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

MAINTAINERCLEANFILES	= Makefile.in

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
			  scim_setup_classify.h

libexecdir		= $(libdir)/scim@SCIM_EPOCH@

if SCIM_BUILD_GTK3_LEGACY_SETUP
libexec_PROGRAMS	= scim-setup-gtk3-host
endif

scim_setup_gtk3_host_SOURCES	= scim_setup_gtk3_host.cpp \
			  scim_setup_module.cpp \
			  scim_setup_classify.cpp

scim_setup_gtk3_host_CXXFLAGS	= @GTK3_CFLAGS@ \
			  @ELF_CFLAGS@

scim_setup_gtk3_host_LDADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

scim_setup_gtk3_host_LDFLAGS	= @GTK3_LIBS@ \
			  @ELF_LIBS@ \
			  @LTLIBINTL@
