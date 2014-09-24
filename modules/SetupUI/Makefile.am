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
			  -I$(top_srcdir)/data \
			  -I$(top_srcdir)/utils \
			  -I$(top_srcdir)/modules/IMEngine \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\"

if SCIM_BUILD_SCIM_SETUP

if SCIM_BUILD_PANEL_GTK
CONFIG_PANEL_GTK_SETUP_MODULE= panel-gtk-setup.la
endif

CONFIG_FRONTEND_SETUP_MODULE= aaa-frontend-setup.la
CONFIG_IMENGINE_SETUP_MODULE= aaa-imengine-setup.la

endif

# Rules for setup module
setupmoduledir           = $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/SetupUI

setupmodule_LTLIBRARIES  = $(CONFIG_FRONTEND_SETUP_MODULE) \
			   $(CONFIG_IMENGINE_SETUP_MODULE) \
			   $(CONFIG_PANEL_GTK_SETUP_MODULE)

aaa_frontend_setup_la_SOURCES = scim_frontend_setup.cpp
aaa_frontend_setup_la_CFLAGS  = @GTK_CFLAGS@
aaa_frontend_setup_la_CXXFLAGS= @GTK_CFLAGS@
aaa_frontend_setup_la_LDFLAGS = -avoid-version \
		    	     	  -rpath $(setupmoduledir) \
		    		  -module \
    				  @LIBTOOL_EXPORT_OPTIONS@ \
		    		  @GTK_LIBS@ \
    				  @LTLIBINTL@
aaa_frontend_setup_la_LIBADD  = $(top_builddir)/src/libscim@SCIM_EPOCH@.la \
				$(top_builddir)/utils/libscim-gtkutils@SCIM_EPOCH@.la


aaa_imengine_setup_la_SOURCES = scim_imengine_setup.cpp
aaa_imengine_setup_la_CFLAGS  = @GTK_CFLAGS@
aaa_imengine_setup_la_CXXFLAGS= @GTK_CFLAGS@
aaa_imengine_setup_la_LDFLAGS = -avoid-version \
				-rpath $(setupmoduledir) \
				-module \
				@LIBTOOL_EXPORT_OPTIONS@ \
				@GTK_LIBS@ \
				@LTLIBINTL@
aaa_imengine_setup_la_LIBADD  = $(top_builddir)/src/libscim@SCIM_EPOCH@.la \
				$(top_builddir)/utils/libscim-gtkutils@SCIM_EPOCH@.la


panel_gtk_setup_la_SOURCES     = scim_panel_gtk_setup.cpp
panel_gtk_setup_la_CFLAGS      = @GTK_CFLAGS@
panel_gtk_setup_la_CXXFLAGS    = @GTK_CFLAGS@
panel_gtk_setup_la_LDFLAGS     = -avoid-version \
			     	 -rpath $(setupmoduledir) \
				 -module \
				 @LIBTOOL_EXPORT_OPTIONS@ \
				 @GTK_LIBS@ \
				 @LTLIBINTL@
panel_gtk_setup_la_LIBADD      = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

