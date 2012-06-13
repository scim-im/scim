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

MAINTAINERCLEANFILES	= Makefile.in

INCLUDES		= -I$(top_builddir)

noinst_HEADERS		= scim-bridge-attribute.h \
					  scim-bridge-debug.h \
					  scim-bridge-display.h \
					  scim-bridge-imcontext.h \
					  scim-bridge-key-code.h \
					  scim-bridge-key-event.h \
					  scim-bridge-message.h \
					  scim-bridge-message-constant.h \
					  scim-bridge-messenger.h \
					  scim-bridge-output.h \
					  scim-bridge-path.h \
					  scim-bridge-string.h \
					  scim-bridge.h

noinst_LTLIBRARIES	= libscimbridgecommon.la

libscimbridgecommon_la_SOURCES	= scim-bridge-attribute.c \
					  scim-bridge-display.c \
					  scim-bridge-path.c \
					  scim-bridge-key-event.c \
					  scim-bridge-message.c \
					  scim-bridge-messenger.c \
					  scim-bridge-string.c

libscimbridgecommon_la_CFLAGS	= @SCIM_CFLAGS@
#libscimbridgecommon_la_CXXFLAGS	= @SCIM_CFLAGS@
#libscimbridgecommon_la_LDFLAGS	= @SCIM_LIBS@ 
