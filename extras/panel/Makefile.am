MAINTAINERCLEANFILES 	= Makefile.in

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
	    	  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/data \
			  -I$(top_srcdir)/utils/gtkutils \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\" \
			  -DSCIM_TEMPDIR=\"@SCIM_TEMPDIR@\"

if SCIM_BUILD_PANEL_GTK
CONFIG_SCIM_PANEL_GTK    = scim-panel-gtk
endif

libexecdir		= $(libdir)/scim@SCIM_EPOCH@
libexec_PROGRAMS        = $(CONFIG_SCIM_PANEL_GTK)

scim_panel_gtk_SOURCES  = scim_panel_gtk.cpp

# StatusNotifierItem tray menu (dbusmenu-glib) and wayland toolbar anchoring
# (gtk4-layer-shell) are both optional; the panel falls back to a plain toolbar
# window without them.
if SCIM_BUILD_SNI
SNI_CFLAGS              = -DSCIM_HAS_SNI @DBUSMENU_CFLAGS@
SNI_LIBS                = @DBUSMENU_LIBS@
endif

if SCIM_BUILD_LAYER_SHELL
LAYER_SHELL_CFLAGS      = -DSCIM_HAS_LAYER_SHELL @GTK4_LAYER_SHELL_CFLAGS@
LAYER_SHELL_LIBS        = @GTK4_LAYER_SHELL_LIBS@
endif

scim_panel_gtk_CXXFLAGS = @GTK_CFLAGS@ @X_CFLAGS@ $(SNI_CFLAGS) $(LAYER_SHELL_CFLAGS)

scim_panel_gtk_LDFLAGS  = @GTK_LIBS@ @X_LIBS@ @LTLIBINTL@ $(SNI_LIBS) $(LAYER_SHELL_LIBS)

scim_panel_gtk_LDADD    = $(top_builddir)/src/libscim@SCIM_EPOCH@.la \
			  $(top_builddir)/utils/gtkutils/libscim-gtkutils@SCIM_EPOCH@.la
