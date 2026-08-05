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

# Both entries carry a user-visible Name/GenericName/Comment, so both are built
# from a template through msgfmt --desktop, which merges the catalogues in po/
# as Name[xx]= lines (the same treatment extras/setup gives scim-setup.desktop).
# Listed in po/POTFILES.in, or there would be nothing to merge.
scim.desktop: scim.desktop.in $(top_srcdir)/po/LINGUAS
	$(AM_V_GEN) $(MSGFMT) --desktop \
		--template=$(srcdir)/scim.desktop.in -d $(top_srcdir)/po -o $@

# The Exec line is substituted at make time, not by config.status: SCIM_LIBEXECDIR
# still holds an unexpanded ${exec_prefix}, which make resolves but a .desktop
# parser would not. That has to happen before the translation merge, so it goes
# through an intermediate template.
scim-virtual-keyboard.desktop: scim-virtual-keyboard.desktop.in Makefile $(top_srcdir)/po/LINGUAS
	$(AM_V_GEN) sed -e 's|@SCIM_LIBEXECDIR[@]|$(SCIM_LIBEXECDIR)|g' \
		$(srcdir)/scim-virtual-keyboard.desktop.in > $@.tmp
	$(AM_V_at) $(MSGFMT) --desktop --template=$@.tmp \
		-d $(top_srcdir)/po -o $@
	$(AM_V_at) rm -f $@.tmp

CLEANFILES		+= scim.desktop \
			  scim-virtual-keyboard.desktop \
			  scim-virtual-keyboard.desktop.tmp

EXTRA_DIST		= scim.desktop.in scim-virtual-keyboard.desktop.in
