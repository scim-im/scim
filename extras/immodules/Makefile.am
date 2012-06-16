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

## @start 1
#AUX_DIST                = $(ac_aux_dir)/config.guess \
#			$(ac_aux_dir)/config.sub \
#			$(ac_aux_dir)/install-sh \
#			$(ac_aux_dir)/ltconfig \
#			$(ac_aux_dir)/ltmain.sh \
#			$(ac_aux_dir)/missing \
#			$(ac_aux_dir)/depcomp
#
#EXTRA_DIST		= config.rpath bootstrap 
#
#AUTOMAKE_OPTIONS 	= gnu
#SUBDIRS = common client-common client-gtk/gtk2 client-gtk/gtk3 client-qt/qt3 client-qt/qt4 client-clutter agent tests doc

## @start 1
#MAINTAINERCLEANFILES 	= Makefile.in aclocal.m4 configure config.h.in $(AUX_DIST)

#CLEANFILES		= *.bak src/*.bak

MAINTAINERCLEANFILES    = Makefile.in
CLEANFILES      = *.bak

SUBDIRS = common client-common client-gtk/gtk2 client-gtk/gtk3 client-qt/qt3 client-qt/qt4 client-clutter agent doc
