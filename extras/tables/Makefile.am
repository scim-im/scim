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
			$(ac_aux_dir)/install-sh \
			$(ac_aux_dir)/ltconfig \
			$(ac_aux_dir)/ltmain.sh \
			$(ac_aux_dir)/missing \
			$(ac_aux_dir)/mkinstalldirs
EXTRA_DIST		= config.rpath mkinstalldirs  bootstrap scim-tables.spec

AUTOMAKE_OPTIONS 	= gnu

if SCIM_TABLES_ENABLE_SKIM_SUPPORT
  SKIM_SUBDIR=skim
endif

SUBDIRS 		= docs intl icons m4 po src tables $(SKIM_SUBDIR)

MAINTAINERCLEANFILES 	= Makefile.in aclocal.m4 configure $(AUX_DIST)

CLEANFILES		= *.bak 

ACLOCAL_AMFLAGS = -I m4

.PHONY: update-ChangeLog

update-ChangeLog:
	/bin/sh $(top_srcdir)/cvs2cl.pl --gmt -P -r -b -T -S -U $(top_srcdir)/developers -f $(top_srcdir)/ChangeLog
