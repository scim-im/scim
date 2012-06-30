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

INCLUDES		= -I$(top_builddir) \
	    		  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/utils \
			  -I$(top_srcdir)/extras/tables/src \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_TABLE_LOCALEDIR=\"$(datadir)/locale\" \
			  -DSCIM_TABLEDIR=\"@SCIM_TABLEDIR@\" \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\"

if SCIM_BUILD_IMENGINE_TABLE
CONFIG_IMENGINE_TABLE_MODULE  = table.la
endif

noinst_HEADERS		= scim_generic_table.h \
			  scim_table_imengine.h \
			  scim_table_private.h

noinst_LTLIBRARIES	= libgentable.la
libgentable_la_SOURCES	= scim_generic_table.cpp

#libgentable_la_CFLAGS	= @SCIM_CFLAGS@ 
#libgentable_la_CXXFLAGS	= @SCIM_CFLAGS@
#libgentable_la_LDFLAGS	= @SCIM_LIBS@ 

moduledir		= @SCIM_MODULEDIR@/IMEngine
module_LTLIBRARIES	= $(CONFIG_IMENGINE_TABLE_MODULE)

table_la_SOURCES 	= scim_table_imengine.cpp

#table_la_CFLAGS		= @SCIM_CFLAGS@
#table_la_CXXFLAGS	= @SCIM_CFLAGS@

table_la_LDFLAGS  = -avoid-version \
                  -rpath $(moduledir) \
              -module \
              @LIBTOOL_EXPORT_OPTIONS@ \
              @LTLIBINTL@

table_la_LIBADD   = libgentable.la $(top_builddir)/src/libscim@SCIM_EPOCH@.la

bin_PROGRAMS		= scim-make-table

scim_make_table_SOURCES	= scim_make_table.cpp
#scim_make_table_CFLAGS	= @SCIM_CFLAGS@
#scim_make_table_CXXFLAGS= @SCIM_CFLAGS@
scim_make_table_LDFLAGS	= @LTLIBINTL@
scim_make_table_LDADD	= libgentable.la $(top_builddir)/src/libscim@SCIM_EPOCH@.la

if SCIM_BUILD_IMENGINE_TABLE
SETUP_MODULE 		= table-imengine-setup.la
endif

setupmoduledir           = @SCIM_MODULEDIR@/SetupUI
setupmodule_LTLIBRARIES  = $(SETUP_MODULE)

table_imengine_setup_la_SOURCES	= scim_table_imengine_setup.cpp

#table_imengine_setup_la_CFLAGS	= @SCIM_GTKUTILS_CFLAGS@ @SCIM_CFLAGS@

table_imengine_setup_la_CXXFLAGS= @GTK_CFLAGS@

table_imengine_setup_la_LDFLAGS	= -avoid-version \
				 -rpath $(setupmoduledir) \
				 -module \
				 @LIBTOOL_EXPORT_OPTIONS@ \
				 @LTLIBINTL@

table_imengine_setup_la_LIBADD  = libgentable.la \
	$(top_builddir)/src/libscim@SCIM_EPOCH@.la \
	$(top_builddir)/utils/libscim-gtkutils@SCIM_EPOCH@.la
