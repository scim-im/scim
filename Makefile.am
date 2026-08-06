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

AUX_DIST                = $(ac_aux_dir)/config.guess \
			$(ac_aux_dir)/config.sub \
			$(ac_aux_dir)/compile \
			$(ac_aux_dir)/depcomp \
			$(ac_aux_dir)/install-sh \
			$(ac_aux_dir)/ltmain.sh \
			$(ac_aux_dir)/missing

EXTRA_DIST		= config.rpath THANKS \
			  bootstrap

AUTOMAKE_OPTIONS 	= gnu
SUBDIRS 		= src protocols utils modules configs po extras data docs tests

# distcheck installs into a throwaway prefix, but everything we plug into
# another package -- the IBus component manifest, the GTK and Qt immodules --
# installs where that package looks for it: an absolute path from its
# pkg-config, outside the prefix and needing root. Point them back inside for
# the check only; a real install still lands where each toolkit searches.
# AM_ rather than plain DISTCHECK_CONFIGURE_FLAGS, which is the user's to set.
AM_DISTCHECK_CONFIGURE_FLAGS = \
	--with-ibus-component-dir='$${datadir}/ibus/component' \
	--with-gtk2-im-module-dir='$${libdir}/gtk-2.0/immodules' \
	--with-gtk3-im-module-dir='$${libdir}/gtk-3.0/immodules' \
	--with-gtk4-im-module-dir='$${libdir}/gtk-4.0/immodules' \
	--with-qt5-im-module-dir='$${libdir}/qt5/plugins/platforminputcontexts' \
	--with-qt6-im-module-dir='$${libdir}/qt6/plugins/platforminputcontexts'

MAINTAINERCLEANFILES 	= Makefile.in \
			  aclocal.m4 \
			  configure \
			  config.h.in \
			  $(AUX_DIST)

DISTCLEANFILES		= scim.pc scim-gtkutils.pc

CLEANFILES		= *.bak *.tar.*

ACLOCAL			= aclocal -I $(ac_aux_dir)

pkgconfigdir            = $(libdir)/pkgconfig

if SCIM_BUILD_GTK_UTILS
GTK_UTILS_PKGCONFIG     = scim-gtkutils.pc
endif

if SCIM_BUILD_X11_UTILS
X11_UTILS_PKGCONFIG     = scim-x11utils.pc
endif

pkgconfig_DATA          = scim.pc \
			  $(GTK_UTILS_PKGCONFIG) \
			  $(X11_UTILS_PKGCONFIG)

ACLOCAL_AMFLAGS = -I m4

.PHONY: update-ChangeLog docs

update-ChangeLog:
	/bin/sh $(top_srcdir)/svn2cl.sh --authors=$(top_srcdir)/developers -f $(top_srcdir)/ChangeLog

docs:
	(cd $(top_srcdir)/docs ; make html ; cd - ; mv -iv docs/*.tar.* $(top_srcdir)/)
