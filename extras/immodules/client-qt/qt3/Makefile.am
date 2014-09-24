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

if SCIM_BUILD_QT3_IMMODULE

AM_CPPFLAGS	= -I$(top_builddir) \
	    	  -I$(top_srcdir) \
			  -I$(top_srcdir)/extras/immodules/common \
			  -I$(top_srcdir)/extras/immodules/client-common

noinst_HEADERS = ../scim-bridge-client-qt.h ../scim-bridge-client-imcontext-qt.h  ../scim-bridge-client-key-event-utility-qt.h ../scim-bridge-client-common-qt.h

moduledir = @QT3_IM_MODULEDIR@
module_LTLIBRARIES = im-scim.la

im_scim_la_SOURCES = ../im-scim-bridge-qt.cpp \
							../scim-bridge-client-qt.cpp \
							../scim-bridge-client-imcontext-qt.cpp \
							../scim-bridge-client-key-event-utility-qt.cpp \
							moc_scim-bridge-client-qt.cpp

im_scim_la_CXXFLAGS=@QT3_CFLAGS@ -DQT_IMMODULE
im_scim_la_CFLAGS  =@QT3_CFLAGS@ -DQT_IMMODULE

im_scim_la_LDFLAGS = -rpath $(moduledir) \
							-avoid-version -no-undefined \
							-module \
							@QT3_LIBS@

im_scim_la_LIBADD  = $(top_builddir)/extras/immodules/common/libscimbridgecommon.la \
					$(top_builddir)/extras/immodules/client-common/libscimbridgeclientcommon.la

MOC = @QT3_MOC@

moc_scim-bridge-client-qt.cpp: ../scim-bridge-client-qt.h
	$(MOC) $< -o $@

else

moc_scim-bridge-client-qt.cpp:
	touch $@

endif

dist-hook:
	-rm -f $(distdir)/moc_scim-bridge-client-qt.cpp

MAINTAINERCLEANFILES 	= Makefile.in
CLEANFILES = moc_scim-bridge-client-qt.cpp

