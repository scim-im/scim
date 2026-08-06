MAINTAINERCLEANFILES 	= Makefile.in
EXTRA_DIST		= im-scim.version-script

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/data \
			  -I$(top_srcdir)/utils/x11utils \
			  -I$(top_srcdir)/utils/candidates \
			  -I$(top_srcdir)/extras/panel \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\" \
			  -DSCIM_KEYBOARD_ICON_FILE=\"@SCIM_ICONDIR@/keyboard.png\"

if SCIM_BUILD_GTK3_IMMODULE
CONFIG_GTK3_IMMODULE = im-scim.la
endif

if SCIM_BUILD_X11_UTILS
X11_UTILS=$(top_builddir)/utils/x11utils/libscim-x11utils@SCIM_EPOCH@.la
endif

# Draw the lookup table in-process with the Cairo candidate renderer, hosted in
# a GtkPopover. The popover needs no global coordinates, so this follows the
# renderer core and works on a Wayland-only build too.
if SCIM_BUILD_CANDIDATES_CORE
CANDIDATES_CFLAGS = -DSCIM_HAS_CANDIDATES \
		    @CAIRO_CFLAGS@
CANDIDATES_LIBS   = $(top_builddir)/utils/candidates/libscim-candidates@SCIM_EPOCH@.la \
		    @CAIRO_LIBS@
endif

# The standalone X11 window, the fallback for a client the popover cannot be
# anchored to: Chromium attaches no widget to its GdkWindow, so a window of our
# own is the only way to put candidates on screen there. Separate from the above
# because GTK can carry X11 support in a build of ours that has no X11 candidate
# library to draw with.
if SCIM_BUILD_CANDIDATES
CANDIDATES_X11_CFLAGS = -DSCIM_HAS_CANDIDATES_X11_LIB
CANDIDATES_X11_LIBS   = $(top_builddir)/utils/candidates/libscim-candidates-x11@SCIM_EPOCH@.la
endif

# Let Plasma draw the candidates instead, when a kimpanel widget is there. The
# agent needs no cairo and no display connection of its own, only libdbus.
if SCIM_BUILD_KIMPANEL
KIMPANEL_CFLAGS   = -DSCIM_HAS_KIMPANEL \
		    @DBUS_CFLAGS@
KIMPANEL_LIBS     = $(top_builddir)/utils/candidates/libscim-candidates-kimpanel@SCIM_EPOCH@.la \
		    @DBUS_LIBS@
endif

if SCIM_LD_VERSION_SCRIPT
LD_VERSION_SCRIPT_OPTION="-Wl,--version-script=$(srcdir)/im-scim.version-script"
endif

noinst_HEADERS = gtkimcontextscim.h

moduledir = @GTK3_IM_MODULEDIR@

module_LTLIBRARIES = $(CONFIG_GTK3_IMMODULE)

im_scim_la_SOURCES = gtkimcontextscim.cpp imscim.cpp

im_scim_la_CXXFLAGS=@GTK3_CFLAGS@ $(CANDIDATES_CFLAGS) $(CANDIDATES_X11_CFLAGS) $(KIMPANEL_CFLAGS)
im_scim_la_CFLAGS  =@GTK3_CFLAGS@ $(CANDIDATES_CFLAGS) $(CANDIDATES_X11_CFLAGS) $(KIMPANEL_CFLAGS)

im_scim_la_LDFLAGS = -rpath $(moduledir) \
		     -avoid-version \
		     -module \
		     $(LD_VERSION_SCRIPT_OPTION) \
		     @GTK3_LIBS@

im_scim_la_LIBADD  = $(top_builddir)/src/libscim@SCIM_EPOCH@.la $(X11_UTILS) $(CANDIDATES_X11_LIBS) $(CANDIDATES_LIBS) $(KIMPANEL_LIBS)

include $(top_srcdir)/dlopen-module.am
