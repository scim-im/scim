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

if SCIM_BUILD_GTK4_IMMODULE
CONFIG_GTK4_IMMODULE = im-scim.la
endif

if SCIM_LD_VERSION_SCRIPT
LD_VERSION_SCRIPT_OPTION="-Wl,--version-script=$(srcdir)/im-scim.version-script"
endif

# Draw the lookup table in-process with the Cairo candidate renderer, hosted in
# a GtkPopover (works on X11 and Wayland). Links the core lib only -- the popover
# is the surface, so no X11 surface backend is needed.
if SCIM_BUILD_CANDIDATES
CANDIDATES_CFLAGS = -DSCIM_HAS_CANDIDATES \
		    -I$(top_srcdir)/utils/candidates \
		    @CAIRO_CFLAGS@
CANDIDATES_LIBS   = $(top_builddir)/utils/candidates/libscim-candidates@SCIM_EPOCH@.la \
		    @CAIRO_LIBS@
endif

noinst_HEADERS = scimgtkimcontext.h

moduledir = @GTK4_IM_MODULEDIR@

module_LTLIBRARIES = $(CONFIG_GTK4_IMMODULE)

im_scim_la_SOURCES = scimgtkimcontext.cpp imscim.cpp

im_scim_la_CXXFLAGS = @GTK4_CFLAGS@ $(CANDIDATES_CFLAGS)
im_scim_la_CFLAGS   = @GTK4_CFLAGS@ $(CANDIDATES_CFLAGS)

im_scim_la_LDFLAGS = -rpath $(moduledir) \
		     -avoid-version \
		     -module \
		     $(LD_VERSION_SCRIPT_OPTION) \
		     @GTK4_LIBS@

im_scim_la_LIBADD  = $(top_builddir)/src/libscim@SCIM_EPOCH@.la $(CANDIDATES_LIBS)

# Immodules are dlopen-ed, never linked: drop the installed libtool archive.
# The dir is shared with other IMEs, so remove only our own file.
install-data-hook:
	rm -f "$(DESTDIR)$(moduledir)/im-scim.la"
