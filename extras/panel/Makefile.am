MAINTAINERCLEANFILES 	= Makefile.in

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
	    	  -I$(top_srcdir) \
			  -I$(top_srcdir)/src \
			  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/data \
			  -I$(top_srcdir)/utils \
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

scim_panel_gtk_CXXFLAGS = @GTK_CFLAGS@ @GTHREAD2_CFLAGS@

scim_panel_gtk_LDFLAGS  = @GTK_LIBS@ @GTHREAD2_LIBS@ @LTLIBINTL@

scim_panel_gtk_LDADD    = $(top_builddir)/src/libscim@SCIM_EPOCH@.la \
			  $(top_builddir)/utils/libscim-gtkutils@SCIM_EPOCH@.la
