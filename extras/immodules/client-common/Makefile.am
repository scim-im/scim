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

if SCIM_BRIDGE_BUILD_IMMODULE

INCLUDES		= -I$(top_srcdir)/common

noinst_HEADERS		= scim-bridge-client.h  scim-bridge-client-protected.h scim-bridge-client-imcontext.h

noinst_LTLIBRARIES	= libscimbridgeclientcommon.la

libscimbridgeclientcommon_la_SOURCES	= scim-bridge-client.c \
										  scim-bridge-client-output.c \
										  scim-bridge-client-debug.c

libscimbridgeclientcommon_la_CFLAGS	= @SCIM_CFLAGS@
#libscimbridgeclientcommon_la_CXXFLAGS	= @SCIM_CFLAGS@
#libscimbridgeclientcommon_la_LDFLAGS	= @SCIM_LIBS@ 

endif

MAINTAINERCLEANFILES	= Makefile.in

