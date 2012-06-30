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

scim_tables_source = \
		Array30.txt.in		\
		CangJie.txt.in		\
		CangJie3.txt.in		\
		CangJie5.txt.in		\
		SmartCangJie6.txt.in	\
		Cantonese.txt.in	\
		CantonHK.txt.in		\
		CNS11643.txt.in		\
		Quick.txt.in		\
		Stroke5.txt.in		\
		Dayi3.txt.in		\
		Erbi.txt.in		\
		Erbi-QS.txt.in          \
		EZ-Big.txt.in		\
		Jyutping.txt.in		\
		Simplex.txt.in		\
		Wu.txt.in		\
		Wubi.txt.in		\
		Ziranma.txt.in		\
		ZhuYin.txt.in		\
		ZhuYin-Big.txt.in

scim_tables = \
		Array30.txt		\
		CangJie.txt		\
		CangJie3.txt		\
		CangJie5.txt		\
		SmartCangJie6.txt	\
		Cantonese.txt		\
		CantonHK.txt		\
		CNS11643.txt		\
		Quick.txt		\
		Stroke5.txt		\
		Dayi3.txt		\
		Erbi.txt		\
		Erbi-QS.txt             \
		EZ-Big.txt		\
		Jyutping.txt		\
		Simplex.txt		\
		Wu.txt			\
		Wubi.txt		\
		Ziranma.txt		\
		ZhuYin.txt		\
		ZhuYin-Big.txt

scim_tables_bindir = $(SCIM_TABLEDIR)
scim_tables_bin_DATA = \
		Array30.bin	\
		CangJie.bin	\
		CangJie3.bin	\
		CangJie5.bin	\
		SmartCangJie6.bin\
		Cantonese.bin	\
		CantonHK.bin	\
		CNS11643.bin	\
		Quick.bin	\
		Stroke5.bin	\
		Dayi3.bin	\
		Erbi.bin	\
		Erbi-QS.bin     \
		EZ-Big.bin	\
		Jyutping.bin	\
		Simplex.bin	\
		Wu.bin		\
		Wubi.bin	\
		Ziranma.bin	\
		ZhuYin.bin	\
		ZhuYin-Big.bin


EXTRA_DIST = $(scim_tables_source) README-Erbi.txt README-CangJie.txt README-Wu.txt

CLEANFILES = $(scim_tables_bin_DATA) *.bak

DISTCLEANFILES = $(scim_tables)

.txt.bin:
	$(SCIM_MAKE_TABLE) $< -b -o $@

