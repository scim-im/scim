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

MAINTAINERCLEANFILES	= Makefile.in *.html

CLEANFILES		= *.bak

SUBDIRS			= figures

if BUILD_MANUAL

HTML_FILES		= user-manual.html
XML_FILES		= user-manual.xml

.xml.html:
	$(XSLTPROC) -o $@ $(DOCBOOK_XSL) $<

html_zh_CN:	$(HTML_FILES)

dist-hook: html_zh_CN
	cp -Rp $(srcdir)/$(HTML_FILES) $(distdir)

else

HTML_FILES		=
XML_FILES		=

html_zh_CN:
	echo "***"
	echo "*** Warning: Manual not built"
	echo "***"

dist-hook:
	echo "***"
	echo "*** Warning: Manual not built"
	echo "*** DISTRIBUTION IS INCOMPLETE"
	echo "***"
endif

EXTRA_DIST		= $(XML_FILES)

.PHONY:	html_zh_CN
