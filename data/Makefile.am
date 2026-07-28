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

SUBDIRS			= icons pixmaps

# XDG autostart entry: launches "scim -d" at login. scim detects the
# environment and becomes the native IM daemon or the GNOME/ibus sync
# coordinator (see src/scim.cpp).
autostartdir		= $(sysconfdir)/xdg/autostart
autostart_DATA		= scim.desktop

# Offered to KWin as a selectable input method (System Settings -> Keyboard ->
# Virtual Keyboard). Plasma only grants zwp_input_method_manager_v2 to the
# process it launches for that role, so the wayland frontend needs this entry.
desktopdir		= $(datadir)/applications
desktop_DATA		= scim-virtual-keyboard.desktop

# Substituted at make time, not by config.status: SCIM_LIBEXECDIR still holds an
# unexpanded ${exec_prefix}, which make resolves but a .desktop parser would not.
scim-virtual-keyboard.desktop: scim-virtual-keyboard.desktop.in Makefile
	$(AM_V_GEN) sed -e 's|@SCIM_LIBEXECDIR[@]|$(SCIM_LIBEXECDIR)|g' \
		$(srcdir)/scim-virtual-keyboard.desktop.in > $@

CLEANFILES		+= scim-virtual-keyboard.desktop

EXTRA_DIST		= scim.desktop scim-virtual-keyboard.desktop.in
