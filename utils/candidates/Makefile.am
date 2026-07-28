## Makefile.am -- Process this file with automake to produce Makefile.in
##
## libscim-candidates: the Cairo+Pango candidate renderer, split into a
## surface-agnostic core plus per-surface backends:
##
##   libscim-candidates          core draw/model/theme (cairo+pango) -- INSTALLED
##   libscim-candidates-x11      X11 cairo-xlib override-redirect     -- INSTALLED
##   libscim-candidates-wayland  input-popup-surface-v2 backend       -- noinst
##
## The core and the X11 backend are installed shared libraries so the GTK/Qt IM
## modules can link them (they draw candidates in-process). The Wayland backend
## stays a convenience library pulled into wayland.so, because it must share the
## single copy of the vendored protocol glue (the wl_interface globals) with it;
## it is also daemon-only, so it never needs to be installed.

MAINTAINERCLEANFILES	= Makefile.in
CLEANFILES		= *.bak

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
			  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  @CAIRO_CFLAGS@

# Public headers (core + X11 backend); the Wayland header stays internal.
candidatesincludedir	= $(includedir)/scim@SCIM_EPOCH@/candidates
candidatesinclude_HEADERS = scim_candidates.h scim_candidates_x11.h
noinst_HEADERS		= scim_candidates_wayland.h

# --- core renderer (installed): surface-agnostic draw/model/theme ---
lib_LTLIBRARIES		= libscim-candidates@SCIM_EPOCH@.la

libscim_candidates@SCIM_EPOCH@_la_SOURCES = scim_candidates.cpp
libscim_candidates@SCIM_EPOCH@_la_LDFLAGS = \
			  -version-info $(SCIM_CURRENT):$(SCIM_REVISION):$(SCIM_AGE) \
			  -export-dynamic \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@ \
			  @CAIRO_LIBS@
libscim_candidates@SCIM_EPOCH@_la_LIBADD  = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

# --- X11 surface backend (installed): core + cairo-xlib + X ---
if SCIM_BUILD_CANDIDATES
lib_LTLIBRARIES		+= libscim-candidates-x11@SCIM_EPOCH@.la
libscim_candidates_x11@SCIM_EPOCH@_la_SOURCES  = scim_candidates_x11.cpp
libscim_candidates_x11@SCIM_EPOCH@_la_CPPFLAGS = $(AM_CPPFLAGS) @CAIRO_XLIB_CFLAGS@ @X_CFLAGS@
libscim_candidates_x11@SCIM_EPOCH@_la_LDFLAGS  = \
			  -version-info $(SCIM_CURRENT):$(SCIM_REVISION):$(SCIM_AGE) \
			  -export-dynamic \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@ \
			  @CAIRO_LIBS@ @CAIRO_XLIB_LIBS@ @X_LIBS@
libscim_candidates_x11@SCIM_EPOCH@_la_LIBADD   = \
			  libscim-candidates@SCIM_EPOCH@.la \
			  $(top_builddir)/src/libscim@SCIM_EPOCH@.la
endif

# --- Wayland surface backend (convenience, pulled into wayland.so) ---
if SCIM_BUILD_FRONTEND_WAYLAND
noinst_LTLIBRARIES	= libscim-candidates-wayland.la
libscim_candidates_wayland_la_SOURCES  = scim_candidates_wayland.cpp
libscim_candidates_wayland_la_CPPFLAGS = $(AM_CPPFLAGS) @WAYLAND_CFLAGS@ \
					 -I$(top_builddir)/protocols
libscim_candidates_wayland_la_LIBADD   = libscim-candidates@SCIM_EPOCH@.la \
					 @CAIRO_LIBS@ @WAYLAND_LIBS@
endif
