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

NOINST_ICONS		= setup.xpm \
		  	  full-letter.xpm \
			  half-letter.xpm \
			  help.xpm \
			  up.xpm \
			  down.xpm \
			  left.xpm \
			  right.xpm \
			  pin-down.xpm \
			  pin-up.xpm \
			  full-punct.xpm \
			  half-punct.xpm \
			  trademark.xpm \
			  menu.xpm

INST_ICONS		= pin-up.png \
			  pin-down.png \
			  full-letter.png \
			  half-letter.png \
			  full-punct.png \
			  half-punct.png \
			  setup.png \
			  help.png \
			  up.png \
			  down.png \
			  left.png \
			  right.png \
			  trademark.png \
			  rawcode.png \
			  keyboard.png \
			  menu.png

SCTC_FILTER_ICONS	= sctc.png \
			  sctc-sc-to-tc.png \
			  sctc-tc-to-sc.png

if SCIM_BUILD_FILTER_SCTC
INST_SCTC_ICONS		= $(SCTC_FILTER_ICONS)
endif

EXTRA_DIST		= $(NOINST_ICONS) \
			  $(INST_ICONS) \
			  $(SCTC_FILTER_ICONS)

icondir			= @SCIM_ICONDIR@
icon_DATA		= $(INST_ICONS) $(INST_SCTC_ICONS)

