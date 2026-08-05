MAINTAINERCLEANFILES 	= Makefile.in
EXTRA_DIST		= im-scim.version-script

AM_CPPFLAGS	= -I$(top_builddir) \
				  -I$(top_builddir)/src \
				  -I$(top_srcdir) \
				  -I$(top_srcdir)/src \
				  -I$(top_srcdir)/intl \
				  -I$(top_srcdir)/data \
				  -I$(top_srcdir)/utils \
				  -I$(top_srcdir)/extras/panel \
				  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
				  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
				  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
				  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
				  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
				  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
				  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\" \
				  -DSCIM_KEYBOARD_ICON_FILE=\"@SCIM_ICONDIR@/keyboard.png\"

# The "lib" prefix is mandatory, not cosmetic: GTK4 finds input methods through
# GIO's module scanner, whose is_valid_module_name() skips any file on Unix that
# is not lib*.so -- silently, with no diagnostic. ibus-gtk4 installs
# libim-ibus.so into this same directory for the same reason. (GTK3 is
# unaffected: it has its own immodules.cache and no such rule.)
if SCIM_BUILD_GTK4_IMMODULE
CONFIG_GTK4_IMMODULE = libim-scim.la
endif

if SCIM_LD_VERSION_SCRIPT
LD_VERSION_SCRIPT_OPTION="-Wl,--version-script=$(srcdir)/im-scim.version-script"
endif

# Draw the lookup table in-process with the Cairo candidate renderer, hosted in
# a GtkPopover (works on X11 and Wayland). The X11 backend comes too, as the
# fallback for a client that sets no widget on its IM context and so leaves the
# popover nothing to be parented to.
if SCIM_BUILD_CANDIDATES
CANDIDATES_CFLAGS = -DSCIM_HAS_CANDIDATES \
		    -I$(top_srcdir)/utils/candidates \
		    @CAIRO_CFLAGS@
CANDIDATES_LIBS   = $(top_builddir)/utils/candidates/libscim-candidates-x11@SCIM_EPOCH@.la \
		    $(top_builddir)/utils/candidates/libscim-candidates@SCIM_EPOCH@.la \
		    @CAIRO_LIBS@
endif

# Let Plasma draw the candidates instead, when a kimpanel widget is there. The
# agent needs no cairo and no display connection of its own, only libdbus.
if SCIM_BUILD_KIMPANEL
KIMPANEL_CFLAGS   = -DSCIM_HAS_KIMPANEL \
		    -I$(top_srcdir)/utils/candidates \
		    @DBUS_CFLAGS@
KIMPANEL_LIBS     = $(top_builddir)/utils/candidates/libscim-candidates-kimpanel@SCIM_EPOCH@.la \
		    @DBUS_LIBS@
endif

noinst_HEADERS = scimgtkimcontext.h

moduledir = @GTK4_IM_MODULEDIR@

module_LTLIBRARIES = $(CONFIG_GTK4_IMMODULE)

libim_scim_la_SOURCES = scimgtkimcontext.cpp imscim.cpp

libim_scim_la_CXXFLAGS = @GTK4_CFLAGS@ $(CANDIDATES_CFLAGS) $(KIMPANEL_CFLAGS)
libim_scim_la_CFLAGS   = @GTK4_CFLAGS@ $(CANDIDATES_CFLAGS) $(KIMPANEL_CFLAGS)

libim_scim_la_LDFLAGS = -rpath $(moduledir) \
		     -avoid-version \
		     -module \
		     $(LD_VERSION_SCRIPT_OPTION) \
		     @GTK4_LIBS@

libim_scim_la_LIBADD  = $(top_builddir)/src/libscim@SCIM_EPOCH@.la $(CANDIDATES_LIBS) $(KIMPANEL_LIBS) @X_LIBS@

# Immodules are dlopen-ed, never linked: drop the installed libtool archive.
# The dir is shared with other IMEs, so remove only our own file.
install-data-hook:
	rm -f "$(DESTDIR)$(moduledir)/libim-scim.la"
