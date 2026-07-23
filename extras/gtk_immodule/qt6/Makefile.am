MAINTAINERCLEANFILES	= Makefile.in
EXTRA_DIST		= scim.json
CLEANFILES		= moc_scimqtinputcontext.cpp

AM_CPPFLAGS	= -I$(top_builddir) \
				  -I$(top_builddir)/src \
				  -I$(top_srcdir) \
				  -I$(top_srcdir)/src \
				  -I$(top_srcdir)/intl \
				  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
				  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
				  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
				  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
				  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
				  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
				  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\" \
				  -DSCIM_KEYBOARD_ICON_FILE=\"@SCIM_ICONDIR@/keyboard.png\"

if SCIM_BUILD_QT6_IMMODULE
CONFIG_QT6_IMMODULE = libscimqtinputcontextplugin.la
endif

moduledir = @QT6_IM_MODULEDIR@

module_LTLIBRARIES = $(CONFIG_QT6_IMMODULE)

noinst_HEADERS = scimqtinputcontext.h

libscimqtinputcontextplugin_la_SOURCES = scimqtinputcontext.cpp
nodist_libscimqtinputcontextplugin_la_SOURCES = moc_scimqtinputcontext.cpp

libscimqtinputcontextplugin_la_CXXFLAGS = @QT6_CFLAGS@ -fPIC
libscimqtinputcontextplugin_la_CFLAGS   = @QT6_CFLAGS@ -fPIC

libscimqtinputcontextplugin_la_LDFLAGS = -rpath $(moduledir) \
				  -avoid-version -no-undefined -module \
				  @QT6_LIBS@

libscimqtinputcontextplugin_la_LIBADD = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

MOC = @QT6_MOC@

moc_scimqtinputcontext.cpp: scimqtinputcontext.h scim.json
	$(AM_V_GEN) $(MOC) -DQT_VERSION=0x060000 $(srcdir)/scimqtinputcontext.h -o $@
