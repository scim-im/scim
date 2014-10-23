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

MAINTAINERCLEANFILES	= Makefile.in scim_types.h
CLEANFILES		= libscim.la *.bak
DISTCLEANFILES		= scim_types.h
EXTRA_DIST		= libscim.version-script

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
			  $(LTDLINCL)


noinst_HEADERS		= \
			  scim_private.h \
			  scim_compose_key_data.h \
			  scim_keyboard_layout_data.h \
			  scim_keyevent_data.h \
			  scim_stl_map.h

libscimincludedir       = $(includedir)/scim@SCIM_EPOCH@

libsciminclude_HEADERS 	= scim.h \
			  scim_attribute.h \
			  scim_backend.h \
			  scim_bind.h \
			  scim_compose_key.h \
			  scim_config_base.h \
			  scim_config_module.h \
			  scim_config_path.h \
			  scim_connection.h \
			  scim_debug.h \
			  scim_event.h \
		      	  scim_exception.h \
			  scim_filter.h \
			  scim_filter_module.h \
			  scim_filter_manager.h \
			  scim_frontend.h \
			  scim_frontend_module.h \
			  scim_global_config.h \
			  scim_helper.h \
			  scim_helper_manager.h \
			  scim_helper_module.h \
			  scim_hotkey.h \
			  scim_iconv.h \
			  scim_imengine.h \
			  scim_imengine_module.h \
			  scim_lookup_table.h \
			  scim_module.h \
			  scim_object.h \
			  scim_panel_agent.h \
			  scim_panel_client.h \
			  scim_panel_common.h \
			  scim_pointer.h \
			  scim_property.h \
			  scim_signals.h \
			  scim_slot.h \
			  scim_socket.h \
			  scim_trans_commands.h \
			  scim_transaction.h \
			  scim_types.h \
			  scim_utility.h


if SCIM_LD_VERSION_SCRIPT
LD_VERSION_SCRIPT_OPTION="-Wl,--version-script=$(srcdir)/libscim.version-script"
endif

lib_LTLIBRARIES 	= libscim@SCIM_EPOCH@.la

libscim@SCIM_EPOCH@_la_SOURCES = scim_backend.cpp \
		      	  scim_chartraits.cpp \
			  scim_compose_key.cpp \
			  scim_config_base.cpp \
			  scim_config_module.cpp \
			  scim_connection.cpp \
			  scim_debug.cpp \
			  scim_event.cpp \
			  scim_filter.cpp \
			  scim_filter_module.cpp \
			  scim_filter_manager.cpp \
			  scim_frontend.cpp \
			  scim_frontend_module.cpp \
			  scim_global_config.cpp \
			  scim_helper.cpp \
			  scim_helper_manager.cpp \
			  scim_helper_module.cpp \
			  scim_hotkey.cpp \
			  scim_iconv.cpp \
			  scim_imengine.cpp \
			  scim_imengine_module.cpp \
			  scim_lookup_table.cpp \
			  scim_module.cpp \
			  scim_object.cpp \
			  scim_panel_agent.cpp \
			  scim_panel_client.cpp \
			  scim_private.cpp \
			  scim_signals.cpp \
			  scim_slot.cpp \
			  scim_socket.cpp \
			  scim_transaction.cpp \
			  scim_utility.cpp

libscim@SCIM_EPOCH@_la_DEPENDENCIES = $(LTDLDEPS)

libscim@SCIM_EPOCH@_la_LDFLAGS= -version-info $(SCIM_CURRENT):$(SCIM_REVISION):$(SCIM_AGE) \
			  -export-dynamic \
			  $(LD_VERSION_SCRIPT_OPTION) \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LIBICONV@ \
			  @LTLIBINTL@ \
			  $(LIBLTDL)


bin_PROGRAMS		= scim scim-config-agent

scim_SOURCES	 	= scim.cpp 
scim_LDADD		= libscim@SCIM_EPOCH@.la
scim_LDFLAGS		= @LTLIBINTL@

scim_config_agent_SOURCES = scim_config_agent.cpp
scim_config_agent_LDADD   = libscim@SCIM_EPOCH@.la
scim_config_agent_LDFLAGS = @LTLIBINTL@

libexecdir		= $(libdir)/scim@SCIM_EPOCH@
libexec_PROGRAMS        = scim-launcher scim-helper-launcher scim-helper-manager

scim_launcher_SOURCES 	= scim_launcher.cpp 
scim_launcher_LDADD	= libscim@SCIM_EPOCH@.la
scim_launcher_LDFLAGS	= @LTLIBINTL@

scim_helper_launcher_SOURCES 	= scim_helper_launcher.cpp 
scim_helper_launcher_LDADD	= libscim@SCIM_EPOCH@.la
scim_helper_launcher_LDFLAGS	= @LTLIBINTL@

scim_helper_manager_SOURCES 	= scim_helper_manager_server.cpp 
scim_helper_manager_LDADD	= libscim@SCIM_EPOCH@.la
scim_helper_manager_LDFLAGS	= @LTLIBINTL@

