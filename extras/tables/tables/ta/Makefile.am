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

SCIM_MAKE_TABLE	= $(top_builddir)/src/scim-make-table

scim_tables_source	= Tamil-inscript.txt.in\
			Tamil-phonetic.txt.in\
			Tamil-remington.txt.in\
			Tamil-tamil99.txt.in
scim_tables		= Tamil-inscript.txt\
			Tamil-phonetic.txt\
			Tamil-remington.txt\
			Tamil-tamil99.txt

scim_tables_bindir = $(SCIM_TABLEDIR)
scim_tables_bin_DATA	= Tamil-inscript.bin\
			Tamil-phonetic.bin\
			Tamil-remington.bin\
			Tamil-tamil99.bin


EXTRA_DIST = $(scim_tables_source)
CLEANFILES = $(scim_tables_bin_DATA) *.bak

DISTCLEANFILES = $(scim_tables)

.txt.bin:
	$(SCIM_MAKE_TABLE) $< -b -o $@

