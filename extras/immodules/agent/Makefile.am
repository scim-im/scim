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

if SCIM_BRIDGE_BUILD_AGENT

INCLUDES		= -I$(top_srcdir) \
                  -I$(top_srcdir)/common \
                  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
                  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
				  -DSCIM_KEYBOARD_ICON_FILE=\"@SCIM_ICONDIR@/keyboard.png\" \
                  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
                  -DSCIM_VERSION=\"@SCIM_VERSION@\" \
                  -DGETTEXT_PACKAGE=\"@GETTEXT_PACKAGE@\"

noinst_HEADERS		= scim-bridge-agent.h \
                      scim-bridge-agent-protected.h \
                      scim-bridge-agent-client-listener.h \
                      scim-bridge-agent-imcontext.h \
                      scim-bridge-agent-interruption-listener.h \
                      scim-bridge-agent-panel-listener.h \
                      scim-bridge-agent-panel-listener-protected.h \
                      scim-bridge-agent-signal-listener.h \
                      scim-bridge-agent-accept-listener.h \
                      scim-bridge-agent-socket-client.h \
					  utils/scim-bridge-agent-panel-client.h
                      
bin_PROGRAMS = scim-bridge

scim_bridge_SOURCES	= scim-bridge-agent-application.cpp \
				      scim-bridge-agent.cpp \
				      scim-bridge-agent-client-listener.cpp \
                      scim-bridge-agent-imcontext.cpp \
                      scim-bridge-agent-interruption-listener.cpp \
                      scim-bridge-agent-output.cpp \
                      scim-bridge-agent-panel-listener.cpp \
                      scim-bridge-agent-signal-listener.cpp \
                      scim-bridge-agent-accept-listener.cpp \
					  utils/scim-bridge-agent-panel-client.cpp

#scim_bridge_CFLAGS	= @SCIM_CFLAGS@
scim_bridge_CXXFLAGS	= @SCIM_CFLAGS@
scim_bridge_LDFLAGS	= @SCIM_LIBS@
scim_bridge_LDADD     = $(top_builddir)/common/libscimbridgecommon.la

endif

MAINTAINERCLEANFILES	= Makefile.in
