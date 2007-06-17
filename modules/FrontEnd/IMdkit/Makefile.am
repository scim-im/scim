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

noinst_HEADERS		= FrameMgr.h \
		  	  Xi18n.h \
			  Xi18nX.h \
			  XimProto.h \
			  IMdkit.h \
			  XimFunc.h \
			  Xtrans.h


noinst_LTLIBRARIES 	= libXimd.la

libXimd_la_SOURCES 	= FrameMgr.c \
		     	  i18nIc.c \
			  i18nPtHdr.c \
			  i18nX.c \
			  IMValues.c \
			  i18nAttr.c \
			  i18nIMProto.c \
			  IMConn.c \
			  i18nClbk.c \
			  i18nMethod.c \
			  i18nUtil.c \
			  IMMethod.c 

libXimd_la_CFLAGS	= @X_CFLAGS@
libXimd_la_LDFLAGS	= @X_LIBS@

