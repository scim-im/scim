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

if SCIM_BUILD_QT5_IMMODULE
CONFIG_QT5_IMMODULE = libscimqtinputcontextplugin.la
endif

# Draw the lookup table in-process with the Cairo candidate renderer, blitted
# into a QtGui QRasterWindow. Links the core lib only (the window is the
# surface); QtGui is already pulled in by @QT5_LIBS@.
if SCIM_BUILD_CANDIDATES
CANDIDATES_CFLAGS = -DSCIM_HAS_CANDIDATES \
		    -I$(top_srcdir)/utils/candidates \
		    @CAIRO_CFLAGS@
CANDIDATES_LIBS   = $(top_builddir)/utils/candidates/libscim-candidates@SCIM_EPOCH@.la \
		    @CAIRO_LIBS@
endif

# Let Plasma draw the candidates instead, when a kimpanel widget is there.
if SCIM_BUILD_KIMPANEL
KIMPANEL_CFLAGS   = -DSCIM_HAS_KIMPANEL \
		    -I$(top_srcdir)/utils/candidates \
		    @DBUS_CFLAGS@
KIMPANEL_LIBS     = $(top_builddir)/utils/candidates/libscim-candidates-kimpanel@SCIM_EPOCH@.la \
		    @DBUS_LIBS@
endif

moduledir = @QT5_IM_MODULEDIR@

module_LTLIBRARIES = $(CONFIG_QT5_IMMODULE)

noinst_HEADERS = scimqtinputcontext.h

libscimqtinputcontextplugin_la_SOURCES = scimqtinputcontext.cpp
nodist_libscimqtinputcontextplugin_la_SOURCES = moc_scimqtinputcontext.cpp

libscimqtinputcontextplugin_la_CXXFLAGS = @QT5_CFLAGS@ -fPIC $(CANDIDATES_CFLAGS) $(KIMPANEL_CFLAGS)
libscimqtinputcontextplugin_la_CFLAGS   = @QT5_CFLAGS@ -fPIC $(CANDIDATES_CFLAGS) $(KIMPANEL_CFLAGS)

libscimqtinputcontextplugin_la_LDFLAGS = -rpath $(moduledir) \
				  -avoid-version -no-undefined -module \
				  @QT5_LIBS@

libscimqtinputcontextplugin_la_LIBADD = $(top_builddir)/src/libscim@SCIM_EPOCH@.la $(CANDIDATES_LIBS) $(KIMPANEL_LIBS)

MOC = @QT5_MOC@

moc_scimqtinputcontext.cpp: scimqtinputcontext.h scim.json
	$(AM_V_GEN) $(MOC) -DQT_VERSION=0x050000 $(srcdir)/scimqtinputcontext.h -o $@

# dlopen-ed plugin, never linked: drop the installed libtool archive (shared dir).
install-data-hook:
	rm -f "$(DESTDIR)$(moduledir)/libscimqtinputcontextplugin.la"
