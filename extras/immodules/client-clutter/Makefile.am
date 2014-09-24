## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Copyright (C) 2006 Ryo Dairiki
## Copyright (C) 2009, Intel Corporation.
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

if SCIM_BUILD_CLUTTER_IMMODULE

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/extras/immodules/common \
			  -I$(top_srcdir)/extras/immodules/client-common

noinst_HEADERS = scim-bridge-client-clutter.h scim-bridge-client-imcontext-clutter.h  scim-bridge-client-key-event-utility-clutter.h

moduledir = @CLUTTER_IM_MODULEDIR@

module_LTLIBRARIES = im-scim.la

im_scim_la_SOURCES = im-scim-bridge-clutter.c \
							scim-bridge-client-clutter.c \
							scim-bridge-client-imcontext-clutter.c \
							scim-bridge-client-key-event-utility-clutter.c

im_scim_la_CXXFLAGS=@CLUTTER_CFLAGS@ @CLUTTERIMCONTEXT_CFLAGS@
im_scim_la_CFLAGS  =@CLUTTER_CFLAGS@ @CLUTTERIMCONTEXT_CFLAGS@

im_scim_la_LDFLAGS = -rpath $(moduledir) \
		     -avoid-version -no-undefined \
		     -module \
		     @CLUTTER_LIBS@ @CLUTTERIMCONTEXT_LIBS@

im_scim_la_LIBADD  = $(top_builddir)/extras/immodules/common/libscimbridgecommon.la \
							$(top_builddir)/extras/immodules/client-common/libscimbridgeclientcommon.la

endif

MAINTAINERCLEANFILES 	= Makefile.in
