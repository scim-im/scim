## Makefile.am -- Process this file with automake to produce Makefile.in
##
## kimpanel: the KDE D-Bus candidate UI backend. (The Cairo+Pango candidate
## renderer moved to utils/candidates/.)

MAINTAINERCLEANFILES	= Makefile.in
CLEANFILES		= *.bak

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/src

noinst_HEADERS		= scim_kimpanel_agent.h

noinst_LTLIBRARIES	=

# kimpanel: KDE D-Bus candidate UI backend (no cairo; pure libdbus + scim).
if SCIM_BUILD_KIMPANEL
noinst_LTLIBRARIES	+= libscimkimpanel.la
libscimkimpanel_la_SOURCES  = scim_kimpanel_agent.cpp
libscimkimpanel_la_CPPFLAGS = -I$(top_builddir) -I$(top_builddir)/src \
			      -I$(top_srcdir) -I$(top_srcdir)/src @DBUS_CFLAGS@
libscimkimpanel_la_LIBADD   = @DBUS_LIBS@
endif
