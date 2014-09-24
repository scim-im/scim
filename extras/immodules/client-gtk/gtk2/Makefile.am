## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Copyright (C) 2006 Ryo Dairiki
##
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation and 
## appearing in the file LICENSE.LGPL included in the package of this file.
## You can also redistribute it and/or modify it under the terms of 
## the GNU General Public License as published by the Free Software Foundation and 
## appearing in the file LICENSE.GPL included in the package of this file.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if SCIM_BUILD_GTK2_IMMODULE

AM_CPPFLAGS		= -I$(top_builddir) \
	    		  -I$(top_srcdir) \
			  -I$(top_srcdir)/extras/immodules/common \
			  -I$(top_srcdir)/extras/immodules/client-common

noinst_HEADERS = ../scim-bridge-client-gtk.h \
				 ../scim-bridge-client-imcontext-gtk.h \
				 ../scim-bridge-client-key-event-utility-gtk.h

moduledir = @GTK2_IM_MODULEDIR@

module_LTLIBRARIES = im-scim.la

im_scim_la_SOURCES = ../im-scim-bridge-gtk.c \
							../scim-bridge-client-gtk.c \
							../scim-bridge-client-imcontext-gtk.c \
							../scim-bridge-client-key-event-utility-gtk.c

im_scim_la_CXXFLAGS=@GTK2_CFLAGS@
im_scim_la_CFLAGS  =@GTK2_CFLAGS@

im_scim_la_LDFLAGS = -rpath $(moduledir) \
		     -avoid-version -no-undefined \
		     -module \
		     @GTK2_LIBS@

im_scim_la_LIBADD  = $(top_builddir)/extras/immodules/common/libscimbridgecommon.la \
					$(top_builddir)/extras/immodules/client-common/libscimbridgeclientcommon.la

endif

MAINTAINERCLEANFILES 	= Makefile.in
