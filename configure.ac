# Process this file with autoconf to produce a configure script
AC_PREREQ([2.71])
AC_INIT([scim],[1.4.18],[https://github.com/scim-im/scim])
AC_CONFIG_SRCDIR([src/scim.h])
AC_CONFIG_MACRO_DIRS([m4])


###########################################################
##          Some definitions of Version macros.          ##
###########################################################

SCIM_MAJOR_VERSION=1
SCIM_MINOR_VERSION=4
SCIM_MICRO_VERSION=18

SCIM_VERSION=$SCIM_MAJOR_VERSION.$SCIM_MINOR_VERSION.$SCIM_MICRO_VERSION

AC_SUBST(SCIM_MAJOR_VERSION)
AC_SUBST(SCIM_MINOR_VERSION)
AC_SUBST(SCIM_MICRO_VERSION)
AC_SUBST(SCIM_VERSION)

# increment if the interface has additions, changes, removals.
SCIM_CURRENT=10

# increment any time the source changes; set to 0 if you increment CURRENT
SCIM_REVISION=6

# increment if any interfaces have been added; set to 0
# if any interfaces have been removed. removal has 
# precedence over adding, so set to 0 if both happened.
SCIM_AGE=2

AC_SUBST(SCIM_CURRENT)
AC_SUBST(SCIM_REVISION)
AC_SUBST(SCIM_AGE)

# Define a string for the earliest version that this release cannot
# coexist with. This is used for directory hierarchies.
#
SCIM_EPOCH=-1.0
AC_SUBST(SCIM_EPOCH)

# Define a string for the earliest version that this release has
# binary compatibility with. This is used for module locations.
#
SCIM_BINARY_VERSION=1.4.0
AC_SUBST(SCIM_BINARY_VERSION)

AC_DEFINE_UNQUOTED(SCIM_BINARY_VERSION, "$SCIM_BINARY_VERSION", [The binary version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_VERSION, "$SCIM_VERSION", [The release version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_MAJOR_VERSION, $SCIM_MAJOR_VERSION, [The Major version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_MINOR_VERSION, $SCIM_MINOR_VERSION, [The Minor version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_MICRO_VERSION, $SCIM_MICRO_VERSION, [The Micro version of SCIM library.])


# for scim-bridge integration
SCIM_BRIDGE_MAJOR_VERSION=$SCIM_MAJOR_VERSION
SCIM_BRIDGE_MINOR_VERSION=$SCIM_MINOR_VERSION
SCIM_BRIDGE_MICRO_VERSION=$SCIM_MICRO_VERSION

SCIM_BRIDGE_VERSION=$SCIM_VERSION
AC_SUBST(SCIM_BRIDGE_MAJOR_VERSION)
AC_SUBST(SCIM_BRIDGE_MINOR_VERSION)
AC_SUBST(SCIM_BRIDGE_MICRO_VERSION)
AC_SUBST(SCIM_BRIDGE_VERSION)

AC_DEFINE_UNQUOTED(SCIM_BRIDGE_VERSION, "$SCIM_BRIDGE_VERSION", [The release version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_BRIDGE_MAJOR_VERSION, $SCIM_BRIDGE_MAJOR_VERSION, [The Major version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_BRIDGE_MINOR_VERSION, $SCIM_BRIDGE_MINOR_VERSION, [The Minor version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_BRIDGE_MICRO_VERSION, $SCIM_BRIDGE_MICRO_VERSION, [The Micro version of SCIM library.])


GETTEXT_PACKAGE=scim
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

###########################################################
##                  Start Configuration.                 ##
###########################################################

# Init automake stuff
AM_INIT_AUTOMAKE([foreign subdir-objects -Wall])
AC_CONFIG_HEADERS([config.h])

# Use a POSIX archiver interface (required by automake >= 1.12 for some libs)
AM_PROG_AR

# Init libtool
dnl dlopen-ed modules and shared libraries only; no static archives (.a).
LT_INIT([disable-static dlopen])
AC_SUBST(LIBTOOL_DEPS)

# Modules are loaded with dlopen(3) directly (libltdl dropped).
# Link libdl on platforms where dlopen lives outside libc.
AC_SEARCH_LIBS([dlopen], [dl])

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
# !!! DO NOT USE IT !!!
LIBTOOL_EXPORT_OPTIONS=''
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C)

# Require a C++17 compiler.
AC_LANG_PUSH([C++])
AC_MSG_CHECKING([whether $CXX supports C++17])
ac_scim_save_CXXFLAGS="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS -std=c++17"
AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([[#include <optional>
                    #include <string_view>
                    #include <memory>]],
                   [[std::optional<int> o; std::string_view sv;
                     auto p = std::make_unique<int>(0); (void)p;]])],
  [AC_MSG_RESULT([yes])],
  [CXXFLAGS="$ac_scim_save_CXXFLAGS"
   AC_MSG_ERROR([a C++17-capable compiler is required to build SCIM])])
AC_LANG_POP([C++])

# Enable a baseline set of warnings for development builds (non-fatal).
SCIM_WARNING_FLAGS="-Wall -Wextra"
CFLAGS="$CFLAGS $SCIM_WARNING_FLAGS"
CXXFLAGS="$CXXFLAGS $SCIM_WARNING_FLAGS"

# Init gettext (languages are listed in po/LINGUAS)
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.21])

# Checks if doxygen is avaiable

AC_ARG_WITH([doxygen],
  AS_HELP_STRING([--without-doxygen], [Don't build API documentation]) )
 
DOXYGEN=no
if test "x$with_doxygen" != "xno"; then
  AC_PATH_PROG(DOXYGEN, doxygen, no)
fi
 
AM_CONDITIONAL(HAVE_DOXYGEN, test x$DOXYGEN != xno)
AC_SUBST(DOXYGEN)

# Checks if graphviz is available
AC_CHECK_PROG(HAVE_GRAPHVIZ, dot, YES, NO)
AC_PATH_PROG(DOT, dot)
DOT=`dirname $DOT`
AC_SUBST(DOT)

AC_PATH_PROG(PERL, perl)
AC_SUBST(PERL)

# Checks if xsltproc is available
AC_PATH_PROG(XSLTPROC, xsltproc, no)
AM_CONDITIONAL(HAVE_XSLTPROC, test x$XSLTPROC != xno)
AC_SUBST(XSLTPROC)

# Checks if docbook-style-xsl is available
AC_CHECK_FILE(
 [/usr/share/sgml/docbook/xsl-stylesheets/html/tldp-html.xsl],
 [DOCBOOK_XSL=/usr/share/sgml/docbook/xsl-stylesheets/html/tldp-html.xsl],
 [AC_CHECK_FILE(
  [/usr/share/sgml/docbook/xsl-stylesheets/html/docbook.xsl],
  [DOCBOOK_XSL=/usr/share/sgml/docbook/xsl-stylesheets/html/docbook.xsl],
   [AC_CHECK_FILE(
    [/usr/share/xml/docbook/stylesheet/nwalsh/current/html/docbook.xsl],
    [DOCBOOK_XSL=/usr/share/xml/docbook/stylesheet/nwalsh/current/html/docbook.xsl],
    [DOCBOOK_XSL=no]
    )]
  )]
)

AC_SUBST(DOCBOOK_XSL)

AM_CONDITIONAL(HAVE_DOCBOOK, test x$DOCBOOK_XSL != xno)
AM_CONDITIONAL(BUILD_MANUAL, test x$XSLTPROC != xno -a x$DOCBOOK_XSL != xno)

# Checks for libraries.
AC_CHECK_HEADERS([langinfo.h libintl.h string.h dirent.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

if test x$ac_cv_type_size_t != xyes; then
  AC_MSG_ERROR([No type size_t, but SCIM needs it!])
fi

AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(unsigned short int)
AC_CHECK_SIZEOF(unsigned int)
AC_CHECK_SIZEOF(unsigned long int)
AC_CHECK_SIZEOF(unsigned long long int)
AC_CHECK_HEADERS([stdint.h],[has_stdint=yes],[has_stdint=no])

if test x$ac_cv_sizeof_unsigned_short_int = x2 && test x$ac_cv_sizeof_char = x1 ; then
  :
else
  AC_MSG_ERROR([
*** SCIM requires
***      sizeof (unsigned short int)  == 2
***      sizeof (char)                == 1
*** You might want to consider using the GNU C compiler.
])
fi

if test x$ac_cv_sizeof_wchar_t = x0 ; then
  AC_MSG_ERROR([
*** SCIM requires a compiler that supports wchar_t,
*** You might want to consider using the GNU C compiler.
])
fi

if test x$has_stdint = xyes; then
  INCLUDE_STDINT="#include <stdint.h>"
  UINT16="uint16_t"
  UINT32="uint32_t"
  UINT64="uint64_t"
else
  INCLUDE_STDINT="//#include <stdint.h>"
  UINT16="unsigned short int"
  if test x$ac_cv_sizeof_unsigned_int = x4; then
    UINT32="unsigned int"
  else
    if test x$ac_cv_sizeof_unsigned_long_int = x4; then
      UINT32="unsigned long int"
    else
      AC_MSG_ERROR([*** No suitable integer type for uint32 found.])
    fi
  fi
  if test x$ac_cv_sizeof_unsigned_long_long_int = x8; then
    UINT64="unsigned long long int"
  else
    AC_MSG_ERROR([*** No suitable integer type for uint64 found.])
  fi
fi
AC_SUBST(INCLUDE_STDINT)
AC_SUBST(UINT16)
AC_SUBST(UINT32)
AC_SUBST(UINT64)

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([gettimeofday memmove memset nl_langinfo setlocale daemon])
AC_CHECK_FUNCS([opendir closedir readdir])
AC_CHECK_FUNCS([usleep nanosleep])
AC_CHECK_FUNCS([gethostbyname gethostbyname_r socket bind accept connect listen],
	       [socket_ok=yes],
	       [socket_ok=no])

AM_ICONV

PKG_PROG_PKG_CONFIG

# Check if we should build scim-panel-gtk daemon
PKG_CHECK_MODULES(GTK2, [gtk+-2.0 >= 2.24.0 pango >= 1.1.0 gdk-pixbuf-2.0 >= 2.4.0],
			[SCIM_HAS_GTK2=yes],
			[SCIM_HAS_GTK2=no])

PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.0.0 pango >= 1.1.0 gdk-pixbuf-2.0 >= 2.4.0],
			[SCIM_HAS_GTK3=yes],
			[SCIM_HAS_GTK3=no])

PKG_CHECK_MODULES(GTK4, [gtk4 >= 4.0.0 pango >= 1.48.0 gdk-pixbuf-2.0 >= 2.42.0],
			[SCIM_HAS_GTK4=yes],
			[SCIM_HAS_GTK4=no])

if test "$SCIM_HAS_GTK2" = "yes"; then
  if $PKG_CONFIG --exists "gtk+-2.0 >= 2.2" ; then
    AC_DEFINE(GDK_MULTIHEAD_SAFE,1,[Force use of GDK's multihead-safe APIs.])
  fi
  GTK2_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
  GTK2_LIBDIR=`$PKG_CONFIG --variable=libdir gtk+-2.0`
  GTK2_IM_MODULEDIR=$GTK2_LIBDIR/gtk-2.0/$GTK2_BINARY_VERSION/immodules
  AC_SUBST(GTK2_IM_MODULEDIR)
fi

if test "$SCIM_HAS_GTK3" = "yes"; then
  GTK3_VERSION=3.0.0
  GTK3_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-3.0`
  GTK3_LIBDIR=`$PKG_CONFIG --variable=libdir gtk+-3.0`
  GTK3_IM_MODULEDIR=$GTK3_LIBDIR/gtk-3.0/$GTK3_BINARY_VERSION/immodules
  AC_SUBST(GTK3_IM_MODULEDIR)
fi

if test "$SCIM_HAS_GTK4" = "yes"; then
  # The installed release, only ever reported in the configure summary -- not a
  # minimum, and not the ABI directory name (that is GTK4_BINARY_VERSION).
  GTK4_VERSION=`$PKG_CONFIG --modversion gtk4`
  GTK4_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk4`
  GTK4_LIBDIR=`$PKG_CONFIG --variable=libdir gtk4`
  GTK4_IM_MODULEDIR=$GTK4_LIBDIR/gtk-4.0/$GTK4_BINARY_VERSION/immodules
  AC_SUBST(GTK4_IM_MODULEDIR)
fi



# Check if we should build the scim Qt input-method module.
# The module is a QPlatformInputContext plugin that uses only Qt's abstracted
# APIs (QGuiApplication, QInputMethod, qe->nativeVirtualKey()), so it needs
# just Qt5Gui -- no X11/xcb/Qt5X11Extras (those were a stale coupling and are
# dropped so the plugin works the same on X11 and Wayland).
PKG_CHECK_MODULES(QT5, [Qt5Gui >= 5.0],
            [SCIM_HAS_QT5=yes],
            [SCIM_HAS_QT5=no])

if test "$SCIM_HAS_QT5" = "yes"; then
  if test -z "$QT5_PREFIX"; then
    QT5_PREFIX=`$PKG_CONFIG --variable=prefix Qt5Core`
    QT5_LIBDIR=`$PKG_CONFIG --variable=libdir Qt5Core`
  fi
  QT5_INCDIR=`$PKG_CONFIG --variable=includedir Qt5Core`
  QT5_VERSION=`$PKG_CONFIG --modversion Qt5Core`
  QT5_CFLAGS="$QT5_CFLAGS -I$QT5_INCDIR/QtCore/$QT5_VERSION -I$QT5_INCDIR/QtCore/$QT5_VERSION/QtCore -I$QT5_INCDIR/QtGui/$QT5_VERSION"
  QT5_MOC=`$PKG_CONFIG --variable=host_bins Qt5Core`/moc
  QT5_IM_MODULEDIR=$QT5_LIBDIR/qt5/plugins/platforminputcontexts
  AC_SUBST(QT5_MOC)
  AC_SUBST(QT5_IM_MODULEDIR)
  AC_SUBST(QT5_PREFIX)
  AC_SUBST(QT5_LIBDIR)

  tmp_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$QT5_CFLAGS -fPIC"
  AC_LANG_PUSH([C++])
  AC_CHECK_HEADERS([QtGui/qpa/qplatforminputcontext.h],
                   [SCIM_HAS_QT5_PRIVATE=yes],
                   [AC_MSG_ERROR([QT5 platform development headers are needed. In Debian based OS, install qtbase5-private-dev package])])
  CPPFLAGS="$tmp_CPPFLAGS"
  AC_LANG_POP
fi

PKG_CHECK_MODULES(QT6, [Qt6Gui >= 6.0],
            [SCIM_HAS_QT6=yes],
            [SCIM_HAS_QT6=no])

if test "$SCIM_HAS_QT6" = "yes"; then
  if test -z "$QT6_PREFIX"; then
    QT6_PREFIX=`$PKG_CONFIG --variable=prefix Qt6Core`
    QT6_LIBDIR=`$PKG_CONFIG --variable=libdir Qt6Core`
  fi
  QT6_INCDIR=`$PKG_CONFIG --variable=includedir Qt6Core`
  QT6_VERSION=`$PKG_CONFIG --modversion Qt6Core`
  QT6_CFLAGS="$QT6_CFLAGS -I$QT6_INCDIR/QtCore/$QT6_VERSION -I$QT6_INCDIR/QtCore/$QT6_VERSION/QtCore -I$QT6_INCDIR/QtGui/$QT6_VERSION"
  QT6_MOC=`$PKG_CONFIG --variable=libexecdir Qt6Core`/moc
  QT6_IM_MODULEDIR=$QT6_LIBDIR/qt6/plugins/platforminputcontexts
  AC_SUBST(QT6_MOC)
  AC_SUBST(QT6_IM_MODULEDIR)
  AC_SUBST(QT6_PREFIX)
  AC_SUBST(QT6_LIBDIR)

  tmp_CPPFLAGS="$CPPFLAGS"
  CPPFLAGS="$QT6_CFLAGS -fPIC"
  AC_LANG_PUSH([C++])
  # Re-test with the QT6 flags (the identically-named QT5 check is cached).
  AS_UNSET([ac_cv_header_QtGui_qpa_qplatforminputcontext_h])
  AC_CHECK_HEADERS([QtGui/qpa/qplatforminputcontext.h],
                   [SCIM_HAS_QT6_PRIVATE=yes],
                   [SCIM_HAS_QT6=no])
  CPPFLAGS="$tmp_CPPFLAGS"
  AC_LANG_POP
fi

# Wayland input-method-v2 FrontEnd + Cairo/Pango candidate renderer dependencies.
PKG_CHECK_MODULES(WAYLAND, [wayland-client >= 1.20 xkbcommon],
			[SCIM_HAS_WAYLAND=yes],
			[SCIM_HAS_WAYLAND=no])
AC_PATH_PROG(WAYLAND_SCANNER, wayland-scanner)
PKG_CHECK_MODULES(CAIRO, [cairo pango pangocairo],
			[SCIM_HAS_CAIRO=yes],
			[SCIM_HAS_CAIRO=no])
# cairo-xlib is needed by the X11 surface shim of the Cairo candidate renderer.
PKG_CHECK_MODULES(CAIRO_XLIB, [cairo-xlib],
			[SCIM_HAS_CAIRO_XLIB=yes],
			[SCIM_HAS_CAIRO_XLIB=no])
if test "$SCIM_HAS_WAYLAND" = "yes"; then
  WAYLAND_PROTOCOLS_DIR=`$PKG_CONFIG --variable=pkgdatadir wayland-protocols 2>/dev/null`
  AC_SUBST(WAYLAND_PROTOCOLS_DIR)
fi

# IBus engine FrontEnd (GNOME).
PKG_CHECK_MODULES(IBUS, [ibus-1.0 >= 1.5],
			[SCIM_HAS_IBUS=yes],
			[SCIM_HAS_IBUS=no])
if test "$SCIM_HAS_IBUS" = "yes"; then
  IBUS_COMPONENT_DIR=`$PKG_CONFIG --variable=pkgdatadir ibus-1.0 2>/dev/null`/component
  AC_SUBST(IBUS_COMPONENT_DIR)
fi

# GIO (GSettings + GFileMonitor), used by the ibus engine-list sync agent to
# watch the config file and mirror the GNOME/ibus input-source list. Optional:
# without it the ibus frontend still works, only the automatic sync is dropped.
PKG_CHECK_MODULES(GIO, [gio-2.0 >= 2.26],
			[SCIM_HAS_GSETTINGS=yes],
			[SCIM_HAS_GSETTINGS=no])
if test "$SCIM_HAS_GSETTINGS" = "yes"; then
  AC_DEFINE(SCIM_HAS_GSETTINGS, 1,
	    [Define if GIO/GSettings is available for the ibus engine-list sync.])
fi

# D-Bus, for the KDE kimpanel candidate UI backend.
PKG_CHECK_MODULES(DBUS, [dbus-1],
			[SCIM_HAS_DBUS=yes],
			[SCIM_HAS_DBUS=no])

# dbusmenu-glib, for the panel's StatusNotifierItem tray menu. The glib flavour
# carries no toolkit dependency, so it is usable from the GTK4 panel (unlike
# libayatana-appindicator, which is GTK3-only).
PKG_CHECK_MODULES(DBUSMENU, [dbusmenu-glib-0.4],
			[SCIM_HAS_DBUSMENU=yes],
			[SCIM_HAS_DBUSMENU=no])

# gtk4-layer-shell, so the panel's toolbar can anchor itself on wayland
# compositors that implement zwlr_layer_shell_v1 (KDE, wlroots). mutter does
# not, so this changes nothing on GNOME.
PKG_CHECK_MODULES(GTK4_LAYER_SHELL, [gtk4-layer-shell-0],
			[SCIM_HAS_GTK4_LAYER_SHELL=yes],
			[SCIM_HAS_GTK4_LAYER_SHELL=no])

###########################################################
##            Definition of compile options.             ##
###########################################################
#
# Grouped by what they affect.  Each help string is written in the direction
# that is NOT the default, which is how ./configure --help reads: a switch shown
# as --disable-foo is on by default, one shown as --enable-foo is off.  "auto"
# means on whenever its dependencies are present; passing --enable-foo
# explicitly then turns a missing dependency into a configure error instead of a
# silent omission.

## --- Build and diagnostics -------------------------------------------------

AC_ARG_ENABLE([debug],
	[AS_HELP_STRING([--enable-debug],
		[Turn on debugging])],
	[],
	[enable_debug=no])

AC_ARG_ENABLE([tests],
	[AS_HELP_STRING([--enable-tests],
		[Build tests/*])],
	[],
	[enable_tests=no])

AC_ARG_ENABLE([documents],
	[AS_HELP_STRING([--enable-documents],
		[Build documents])],
	[],
	[enable_documents=no])

AC_ARG_ENABLE([ordered-map],
	[AS_HELP_STRING([--enable-ordered-map],
		[Use std::map/std::set instead of the unordered hash containers internally])],
	[],
	[enable_ordered_map=no])

AC_ARG_ENABLE([ld-version-script],
	[AS_HELP_STRING([--enable-ld-version-script],
		[Use ld version script to limit exported symbols])],
	[],
	[enable_ld_version_script=no])

## --- Core modules ---------------------------------------------------------

AC_ARG_ENABLE([config-simple],
	[AS_HELP_STRING([--disable-config-simple],
		[Do not build the Simple Config module])],
	[],
	[enable_config_simple=yes])

AC_ARG_ENABLE([config-socket],
	[AS_HELP_STRING([--disable-config-socket],
		[Do not build the Socket Config module])],
	[],
	[enable_config_socket=yes])

AC_ARG_ENABLE([im-rawcode],
	[AS_HELP_STRING([--disable-im-rawcode],
		[Do not build the RawCode IMEngine module])],
	[],
	[enable_im_rawcode=yes])

AC_ARG_ENABLE([im-socket],
	[AS_HELP_STRING([--disable-im-socket],
		[Do not build the Socket IMEngine module])],
	[],
	[enable_im_socket=yes])

AC_ARG_ENABLE([filter-hanconv],
	[AS_HELP_STRING([--disable-filter-hanconv],
		[Do not build the HanConv Filter module (Han script variant conversion)])],
	[],
	[enable_filter_hanconv=yes])

## --- Display-server frontends ---------------------------------------------

AC_ARG_ENABLE([frontend-socket],
	[AS_HELP_STRING([--disable-frontend-socket],
		[Do not build the Socket FrontEnd module])],
	[],
	[enable_frontend_socket=yes])

# Disabling this drops the whole X11 build -- the XIM frontend, x11utils, the
# cairo-xlib candidate window and the GTK2/GTK3 IM modules -- for a
# Wayland-only system.  The detection it feeds runs just below this section,
# before the first thing that reads the result.
AC_ARG_ENABLE([frontend-x11],
	[AS_HELP_STRING([--disable-frontend-x11],
		[Do not build X11/XWayland support (for Wayland-only systems; default: auto)])],
	[],
	[enable_frontend_x11=check])

AC_ARG_ENABLE([frontend-wayland],
	[AS_HELP_STRING([--disable-frontend-wayland],
		[Do not build Wayland support (for X11-only systems; default: auto)])],
	[],
	[enable_frontend_wayland=check])

AC_ARG_ENABLE([frontend-ibus],
	[AS_HELP_STRING([--disable-frontend-ibus],
		[Do not build the IBus FrontEnd module, which registers SCIM as an IBus engine for GNOME (default: auto)])],
	[],
	[enable_frontend_ibus=check])

## --- Toolkit IM modules ---------------------------------------------------
##
## Each is built when its toolkit is found; the GTK2/GTK3 ones need X11 too.

AC_ARG_ENABLE([gtk2-immodule],
	[AS_HELP_STRING([--disable-gtk2-immodule],
		[Do not build the GTK2 IMModule (default: auto)])],
	[],
	[enable_gtk2_immodule=yes])

AC_ARG_ENABLE([gtk3-immodule],
	[AS_HELP_STRING([--disable-gtk3-immodule],
		[Do not build the GTK3 IMModule (default: auto)])],
	[],
	[enable_gtk3_immodule=yes])

AC_ARG_ENABLE([gtk4-immodule],
	[AS_HELP_STRING([--disable-gtk4-immodule],
		[Do not build the GTK4 IMModule (default: auto)])],
	[],
	[enable_gtk4_immodule=yes])

AC_ARG_ENABLE([qt5-immodule],
	[AS_HELP_STRING([--disable-qt5-immodule],
		[Do not build the QT5 IMModule (default: auto)])],
	[],
	[enable_qt5_immodule=yes])

AC_ARG_ENABLE([qt6-immodule],
	[AS_HELP_STRING([--disable-qt6-immodule],
		[Do not build the QT6 IMModule (default: auto)])],
	[],
	[enable_qt6_immodule=yes])

## --- Candidate UI, panel and setup ----------------------------------------

AC_ARG_ENABLE([kimpanel],
	[AS_HELP_STRING([--disable-kimpanel],
		[Do not build the kimpanel candidate UI, which lets KDE draw the candidates over D-Bus (default: auto)])],
	[],
	[enable_kimpanel=check])

AC_ARG_ENABLE([panel-gtk],
	[AS_HELP_STRING([--disable-panel-gtk],
		[Do not build the GTK Panel (default: auto)])],
	[],
	[enable_panel_gtk=check])

AC_ARG_ENABLE([sni],
	[AS_HELP_STRING([--disable-sni],
		[Do not build the panel's SNI tray, a StatusNotifierItem instead of a floating toolbar (default: auto)])],
	[],
	[enable_sni=check])

AC_ARG_ENABLE([layer-shell],
	[AS_HELP_STRING([--disable-layer-shell],
		[Do not use the panel layer-shell anchor, which places the toolbar on Wayland via gtk4-layer-shell (default: auto)])],
	[],
	[enable_layer_shell=check])

AC_ARG_ENABLE([setup-ui],
	[AS_HELP_STRING([--disable-setup-ui],
		[Do not build the Setup UI])],
	[],
	[enable_setup_ui=yes])

AC_ARG_ENABLE([gtk3-legacy-setup],
	[AS_HELP_STRING([--disable-gtk3-legacy-setup],
		[Do not build the frozen GTK3 legacy setup helper (enabled when GTK3 is present and the setup host is GTK4)])],
	[],
	[enable_gtk3_legacy_setup=check])

## --- Installation paths ---------------------------------------------------

AC_ARG_WITH([gtk2-im-module-dir],
	[AS_HELP_STRING([--with-gtk2-im-module-dir=dir],
		[Select GTK2 immodule dir])],
	[GTK2_IM_MODULEDIR=$with_gtk2_im_module_dir])

AC_ARG_WITH([gtk3-im-module-dir],
	[AS_HELP_STRING([--with-gtk3-im-module-dir=dir],
		[Select GTK3 immodule dir])],
	[GTK3_IM_MODULEDIR=$with_gtk3_im_module_dir])

AC_ARG_WITH([gtk4-im-module-dir],
	[AS_HELP_STRING([--with-gtk4-im-module-dir=dir],
		[Select GTK4 immodule dir])],
	[GTK4_IM_MODULEDIR=$with_gtk4_im_module_dir])

AC_ARG_WITH([qt5-moc],
	[AS_HELP_STRING([--with-qt5-moc=file],
		[Select QT5 moc program (default to QT5_PREFIX/bin/moc)])],
	[QT5_MOC=$with_qt5_moc])

AC_ARG_WITH([qt5-im-module-dir],
	[AS_HELP_STRING([--with-qt5-im-module-dir=dir],
		[Select QT5 immodule dir])],
	[QT5_IM_MODULEDIR=$with_qt5_im_module_dir])

AC_ARG_WITH([qt6-moc],
	[AS_HELP_STRING([--with-qt6-moc=file],
		[Select QT6 moc program (default to the libexecdir Qt6Core reports)])],
	[QT6_MOC=$with_qt6_moc])

AC_ARG_WITH([qt6-im-module-dir],
	[AS_HELP_STRING([--with-qt6-im-module-dir=dir],
		[Select QT6 immodule dir])],
	[QT6_IM_MODULEDIR=$with_qt6_im_module_dir])

# Defaults to ibus's own pkgdatadir, which is absolute and ignores --prefix --
# it has to be, since ibus-daemon looks only there. Overridable so a build that
# must stay inside its prefix (make distcheck, a staged package) can say so.
AC_ARG_WITH([ibus-component-dir],
	[AS_HELP_STRING([--with-ibus-component-dir=dir],
		[Select IBus component dir])],
	[IBUS_COMPONENT_DIR=$with_ibus_component_dir])

# Checks for X11 header files.  Kept here, after the switches and before the
# first use of have_x, so --disable-frontend-x11 can behave exactly as if X11
# were absent while an explicit --enable-frontend-x11 without it is an error.
#
# AC_PATH_XTRA brings autoconf's own --with-x/--without-x along with it, which is
# why there are two spellings for one decision: --without-x is equivalent to
# --disable-frontend-x11 and both are honoured.  Wayland needs no counterpart --
# it is found with pkg-config, so --disable-frontend-wayland is the only spelling
# there and adding a --without-wayland would just duplicate it.
AC_PATH_XTRA

if test "$enable_frontend_x11" = "no"; then
  have_x=no
elif test "$enable_frontend_x11" = "yes" -a "$have_x" != "yes"; then
  AC_MSG_ERROR([--enable-frontend-x11 requires X11 headers/libraries (use --disable-frontend-x11 for a Wayland-only build)])
fi
if test "$have_x" = "yes"; then
  enable_frontend_x11=yes
  AC_DEFINE([SCIM_ENABLE_X11], [1], [Define to 1 if X11/XWayland support is enabled.])
  X_LIBS="$X_LIBS -lX11"
else
  enable_frontend_x11=no
fi

if test "$have_x" = "yes"; then
  SCIM_BUILD_X11_UTILS=1
  enable_x11_utils=yes
else
  SCIM_BUILD_X11_UTILS=0
  enable_x11_utils=no
fi

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g"
  CXXFLAGS="$CXXFLAGS -g"
else
  enable_debug=no
fi

if test "$enable_ordered_map" = "yes"; then
  AC_DEFINE(SCIM_USE_ORDERED_MAP,1,[Use std::map/std::set instead of unordered hash containers in libscim])
fi

if test "$enable_tests" = "yes"; then
  SCIM_BUILD_TESTS=1
else
  SCIM_BUILD_TESTS=0
  enable_tests=no
fi

if test "$enable_documents" = "yes"; then
  SCIM_DOCUMENTS=1
else
  SCIM_DOCUMENTS=0
  enable_documents=no
fi

if test "$enable_config_simple" = "yes"; then
  SCIM_BUILD_CONFIG_SIMPLE=1
else
  SCIM_BUILD_CONFIG_SIMPLE=0
  enable_config_simple=no
fi

if test "$enable_config_socket" = "yes" -a "$socket_ok" = "yes"; then
  SCIM_BUILD_CONFIG_SOCKET=1
else
  SCIM_BUILD_CONFIG_SOCKET=0
  enable_config_socket=no
fi
 
if test "$enable_frontend_x11" = "yes" -a "$have_x" = "yes"; then
  SCIM_BUILD_FRONTEND_X11=1
else
  SCIM_BUILD_FRONTEND_X11=0
  enable_frontend_x11=no
fi

if test "$enable_frontend_socket" = "yes" -a "$socket_ok" = "yes"; then
  SCIM_BUILD_FRONTEND_SOCKET=1
else
  SCIM_BUILD_FRONTEND_SOCKET=0
  enable_frontend_socket=no
fi

# Wayland FrontEnd: needs wayland-client, xkbcommon, cairo/pango, wayland-scanner.
if test "$enable_frontend_wayland" != "no"; then
  if test "$SCIM_HAS_WAYLAND" = "yes" -a "$SCIM_HAS_CAIRO" = "yes" -a -n "$WAYLAND_SCANNER"; then
    SCIM_BUILD_FRONTEND_WAYLAND=1
    enable_frontend_wayland=yes
    AC_DEFINE([SCIM_ENABLE_WAYLAND], [1], [Define to 1 if Wayland support is enabled.])
  elif test "$enable_frontend_wayland" = "yes"; then
    AC_MSG_ERROR([--enable-frontend-wayland requires wayland-client, xkbcommon, cairo/pango and wayland-scanner])
  else
    SCIM_BUILD_FRONTEND_WAYLAND=0
    enable_frontend_wayland=no
  fi
else
  SCIM_BUILD_FRONTEND_WAYLAND=0
fi

# IBus FrontEnd: needs ibus-1.0.
if test "$enable_frontend_ibus" != "no"; then
  if test "$SCIM_HAS_IBUS" = "yes"; then
    SCIM_BUILD_FRONTEND_IBUS=1
    enable_frontend_ibus=yes
  elif test "$enable_frontend_ibus" = "yes"; then
    AC_MSG_ERROR([--enable-frontend-ibus requires ibus-1.0])
  else
    SCIM_BUILD_FRONTEND_IBUS=0
    enable_frontend_ibus=no
  fi
else
  SCIM_BUILD_FRONTEND_IBUS=0
fi

# Cairo+Pango input-panel renderer (own-Cairo candidate UI), in two pieces.
#
# The X11 surface backend draws into an override-redirect window, so it needs
# cairo-xlib and X11 on top of cairo/pango.
if test "$SCIM_HAS_CAIRO" = "yes" -a "$SCIM_HAS_CAIRO_XLIB" = "yes" -a "$have_x" = "yes"; then
  SCIM_BUILD_CANDIDATES=1
  enable_candidates=yes
else
  SCIM_BUILD_CANDIDATES=0
  enable_candidates=no
fi

# The core is surface-agnostic and needs only cairo/pango -- but it draws
# nothing on its own, so build it only when something that owns a surface will
# use it: the X11 backend above, or the Wayland frontend's input-popup surface.
# Ungated it would build and install even for a configuration that turned every
# candidate UI off.
if test "$SCIM_HAS_CAIRO" = "yes" &&
   test "$SCIM_BUILD_CANDIDATES" = "1" -o "$SCIM_BUILD_FRONTEND_WAYLAND" = "1"; then
  SCIM_BUILD_CANDIDATES_CORE=1
  enable_candidates_core=yes
else
  SCIM_BUILD_CANDIDATES_CORE=0
  enable_candidates_core=no
fi

# StatusNotifierItem tray for the panel: needs dbusmenu-glib. On a desktop with
# an SNI host (KDE, XFCE, ...) the panel presents its properties as a tray menu
# instead of a floating toolbar, which wayland cannot position.
if test "$enable_sni" != "no"; then
  if test "$SCIM_HAS_DBUSMENU" = "yes"; then
    SCIM_BUILD_SNI=1
    enable_sni=yes
  elif test "$enable_sni" = "yes"; then
    AC_MSG_ERROR([--enable-sni requires dbusmenu-glib-0.4])
  else
    SCIM_BUILD_SNI=0
    enable_sni=no
  fi
else
  SCIM_BUILD_SNI=0
fi

# layer-shell anchoring for the panel toolbar: needs gtk4-layer-shell.
if test "$enable_layer_shell" != "no"; then
  if test "$SCIM_HAS_GTK4_LAYER_SHELL" = "yes"; then
    SCIM_BUILD_LAYER_SHELL=1
    enable_layer_shell=yes
  elif test "$enable_layer_shell" = "yes"; then
    AC_MSG_ERROR([--enable-layer-shell requires gtk4-layer-shell-0])
  else
    SCIM_BUILD_LAYER_SHELL=0
    enable_layer_shell=no
  fi
else
  SCIM_BUILD_LAYER_SHELL=0
fi

# kimpanel D-Bus candidate UI backend: needs dbus-1.
if test "$enable_kimpanel" != "no"; then
  if test "$SCIM_HAS_DBUS" = "yes"; then
    SCIM_BUILD_KIMPANEL=1
    enable_kimpanel=yes
  elif test "$enable_kimpanel" = "yes"; then
    AC_MSG_ERROR([--enable-kimpanel requires dbus-1])
  else
    SCIM_BUILD_KIMPANEL=0
    enable_kimpanel=no
  fi
else
  SCIM_BUILD_KIMPANEL=0
fi

if test "$enable_im_rawcode" = "yes"; then
  SCIM_BUILD_IMENGINE_RAWCODE=1
else
  SCIM_BUILD_IMENGINE_RAWCODE=0
  enable_im_rawcode=no
fi

if test "$enable_im_socket" = "yes" -a "$socket_ok" = "yes"; then
  SCIM_BUILD_IMENGINE_SOCKET=1
else
  SCIM_BUILD_IMENGINE_SOCKET=0
  enable_im_socket=no
fi

if test "$enable_filter_hanconv" = "yes"; then
  SCIM_BUILD_FILTER_HANCONV=1
else
  SCIM_BUILD_FILTER_HANCONV=0
  enable_filter_hanconv=no
fi

# The gtk2/gtk3 IM modules are the X11/XWayland integration path and use the
# x11utils keymask helpers, so they need X11.
if test "$enable_gtk2_immodule" = "yes" -a "$SCIM_HAS_GTK2" = "yes" -a "$have_x" = "yes"; then
  SCIM_BUILD_GTK2_IMMODULE=1
else
  SCIM_BUILD_GTK2_IMMODULE=0
  enable_gtk2_immodule=no
fi

if test "$enable_gtk3_immodule" = "yes" -a "$SCIM_HAS_GTK3" = "yes" -a "$have_x" = "yes"; then
  SCIM_BUILD_GTK3_IMMODULE=1
else
  SCIM_BUILD_GTK3_IMMODULE=0
  enable_gtk3_immodule=no
fi

if test "$enable_gtk4_immodule" = "yes" -a "$SCIM_HAS_GTK4" = "yes"; then
  SCIM_BUILD_GTK4_IMMODULE=1
else
  SCIM_BUILD_GTK4_IMMODULE=0
  enable_gtk4_immodule=no
fi

if test "$enable_qt5_immodule" = "yes" -a "$SCIM_HAS_QT5" = "yes"; then
  SCIM_BUILD_QT5_IMMODULE=1
else
  SCIM_BUILD_QT5_IMMODULE=0
  enable_qt5_immodule=no
fi

if test "$enable_qt6_immodule" = "yes" -a "$SCIM_HAS_QT6" = "yes"; then
  SCIM_BUILD_QT6_IMMODULE=1
else
  SCIM_BUILD_QT6_IMMODULE=0
  enable_qt6_immodule=no
fi

# The GTK parts of the tree -- the panel, the setup host, the SetupUI plugins and
# libscim-gtkutils -- are GTK4 sources: gtk_init () with no arguments,
# gtk_widget_add_controller (), gtk_window_set_child ().  Pointing them at GTK2 or
# GTK3, which --with-gtk-version used to offer, could only produce a build
# failure, so GTK4 is simply what they require.  The GTK2/GTK3/GTK4 IM modules
# are separate and keep their own per-version flags.
if test "$SCIM_HAS_GTK4" = "yes"; then
  SCIM_HAS_GTK=yes
  GTK_LIBDIR=$GTK4_LIBDIR
  GTK_VERSION=$GTK4_VERSION
  GTK_MAJOR_VERSION=4
  GTK_BINARY_VERSION=$GTK4_BINARY_VERSION
  GTK_CFLAGS=$GTK4_CFLAGS
  GTK_LIBS=$GTK4_LIBS
else
  SCIM_HAS_GTK=no
fi

# gtk4 dropped the "gtk+-" prefix; named unconditionally so scim-gtkutils.pc is
# well formed even in a build that leaves the GTK parts out.
GTK_PC_NAME=gtk4

AC_SUBST(GTK_LIBDIR)
AC_SUBST(GTK_VERSION)
AC_SUBST(GTK_MAJOR_VERSION)
AC_SUBST(GTK_BINARY_VERSION)
AC_SUBST(GTK_CFLAGS)
AC_SUBST(GTK_LIBS)
AC_SUBST(GTK_PC_NAME)

if test "$enable_setup_ui" = "yes" -a "$SCIM_HAS_GTK" = "yes"; then
  SCIM_BUILD_SCIM_SETUP=1
  AC_DEFINE(HAVE_SCIM_SETUP,1,[Have scim-setup program.])
else
  SCIM_BUILD_SCIM_SETUP=0
  enable_setup_ui=no
fi

# libelf is used ONLY by the setup host (extras/setup) to classify SetupUI
# plugins by their DT_NEEDED before loading them. It is not a dependency of
# libscim or any other component.
SCIM_HAS_ELF=no
ELF_CFLAGS=""
ELF_LIBS=""
if test "$enable_setup_ui" = "yes"; then
  PKG_CHECK_MODULES(ELF, [libelf],
			[SCIM_HAS_ELF=yes],
			[SCIM_HAS_ELF=no])
  if test "$SCIM_HAS_ELF" = "no"; then
    AC_CHECK_LIB([elf], [elf_begin],
			[SCIM_HAS_ELF=yes; ELF_LIBS="-lelf"],
			[SCIM_HAS_ELF=no])
    AC_CHECK_HEADER([libelf.h], [], [SCIM_HAS_ELF=no])
  fi
  if test "$SCIM_HAS_ELF" = "no"; then
    AC_MSG_ERROR([libelf (with development headers) is required to build the setup UI; install libelf-dev / elfutils-libelf-devel, or configure with --disable-setup-ui])
  fi
fi
AC_SUBST(ELF_CFLAGS)
AC_SUBST(ELF_LIBS)

# The frozen GTK3 legacy setup helper hosts third-party GTK3 setup plugins that
# cannot be embedded in the GTK4 host. It is a separate GTK3-linked program and
# only makes sense when the host itself is GTK4 and GTK3 is available.
SCIM_BUILD_GTK3_LEGACY_SETUP=0
if test "$enable_gtk3_legacy_setup" != "no"; then
  if test "$enable_setup_ui" = "yes" -a "$SCIM_HAS_GTK" = "yes" -a "$SCIM_HAS_GTK3" = "yes"; then
    SCIM_BUILD_GTK3_LEGACY_SETUP=1
    enable_gtk3_legacy_setup=yes
    AC_DEFINE(SCIM_ENABLE_GTK3_LEGACY_SETUP,1,[Build the frozen GTK3 legacy setup helper.])
  else
    enable_gtk3_legacy_setup=no
  fi
fi
AC_SUBST(GTK3_CFLAGS)
AC_SUBST(GTK3_LIBS)

if test "$SCIM_HAS_GTK" = "yes"; then
  enable_gtk_utils=yes
  SCIM_BUILD_GTK_UTILS=1
else
  enable_gtk_utils=no
  SCIM_BUILD_GTK_UTILS=0
fi

SCIM_BUILD_PANEL_GTK=0
if test "$enable_panel_gtk" != "no"; then
  enable_panel_gtk=no
  if test "$SCIM_HAS_GTK" = "yes"; then
    SCIM_BUILD_PANEL_GTK=1
    enable_panel_gtk=yes
  fi
fi

#if nothing needs libscim-gtkutils, just disable it
if test "$enable_panel_gtk" != "yes" -a "$enable_setup_ui" != "yes"; then
  enable_gtk_utils=no
  SCIM_BUILD_GTK_UTILS=0
fi

AM_CONDITIONAL(SCIM_LD_VERSION_SCRIPT,
		[test "$enable_ld_version_script" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_TESTS,
		[test "$enable_tests" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_DOCUMENTS,
        [test "$enable_documents" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_CONFIG_SIMPLE,
		[test "$enable_config_simple" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_CONFIG_SOCKET,
		[test "$enable_config_socket" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_FRONTEND_X11,
		[test "$enable_frontend_x11" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_FRONTEND_SOCKET,
		[test "$enable_frontend_socket" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_FRONTEND_WAYLAND,
		[test "$enable_frontend_wayland" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_FRONTEND_IBUS,
		[test "$enable_frontend_ibus" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GSETTINGS,
		[test "$SCIM_HAS_GSETTINGS" = "yes"])

# The ibussync coordinator frontend needs both GSettings (to watch the GNOME
# input-source list) and ibus (to read the installed engine set live via
# ibus_bus_list_engines()).
AM_CONDITIONAL(SCIM_BUILD_IBUS_SYNC,
		[test "$SCIM_HAS_GSETTINGS" = "yes" -a "$SCIM_HAS_IBUS" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_CANDIDATES,
		[test "$enable_candidates" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_CANDIDATES_CORE,
		[test "$enable_candidates_core" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_SNI,
		[test "$enable_sni" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_LAYER_SHELL,
		[test "$enable_layer_shell" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_KIMPANEL,
		[test "$enable_kimpanel" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_IMENGINE_SOCKET,
		[test "$enable_im_socket" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_IMENGINE_RAWCODE,
		[test "$enable_im_rawcode" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK2_IMMODULE,
		[test "$enable_gtk2_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK3_IMMODULE,
		[test "$enable_gtk3_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK4_IMMODULE,
		[test "$enable_gtk4_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_QT5_IMMODULE,
		[test "$enable_qt5_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_QT6_IMMODULE,
		[test "$enable_qt6_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_FILTER_HANCONV,
		[test "$enable_filter_hanconv" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK3_LEGACY_SETUP,
		[test "$SCIM_BUILD_GTK3_LEGACY_SETUP" = "1"])

AM_CONDITIONAL(SCIM_BUILD_SCIM_SETUP,
		[test "$enable_setup_ui" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK_UTILS,
	        [test "$enable_gtk_utils" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_X11_UTILS,
	        [test "$enable_x11_utils" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_PANEL_GTK,
	        [test "$enable_panel_gtk" = "yes"])

AC_SUBST(SCIM_BUILD_TESTS)
AC_SUBST(SCIM_BUILD_DOCUMENTS)
AC_SUBST(SCIM_BUILD_CONFIG_SIMPLE)
AC_SUBST(SCIM_BUILD_CONFIG_SOCKET)
AC_SUBST(SCIM_BUILD_FRONTEND_X11)
AC_SUBST(SCIM_BUILD_FRONTEND_SOCKET)
AC_SUBST(SCIM_BUILD_FRONTEND_WAYLAND)
AC_SUBST(SCIM_BUILD_FRONTEND_IBUS)
AC_SUBST(SCIM_BUILD_CANDIDATES)
AC_SUBST(SCIM_BUILD_CANDIDATES_CORE)
AC_SUBST(SCIM_BUILD_SNI)
AC_SUBST(SCIM_BUILD_LAYER_SHELL)
AC_SUBST(SCIM_BUILD_KIMPANEL)
AC_SUBST(SCIM_BUILD_IMENGINE_RAWCODE)
AC_SUBST(SCIM_BUILD_IMENGINE_TABLE)
AC_SUBST(SCIM_BUILD_IMENGINE_SOCKET)
AC_SUBST(SCIM_BUILD_FILTER_HANCONV)
AC_SUBST(SCIM_BUILD_SCIM_SETUP)
AC_SUBST(SCIM_BUILD_PANEL_GTK)
AC_SUBST(SCIM_BUILD_GTK_UTILS)
AC_SUBST(SCIM_BUILD_X11_UTILS)
AC_SUBST(SCIM_BUILD_GTK2_IMMODULE)
AC_SUBST(SCIM_BUILD_GTK3_IMMODULE)
AC_SUBST(SCIM_BUILD_GTK4_IMMODULE)
AC_SUBST(SCIM_BUILD_QT5_IMMODULE)
AC_SUBST(SCIM_BUILD_QT6_IMMODULE)


# Output All necessary Paths.
SCIM_DATADIR="$datadir/scim"
SCIM_SYSCONFDIR="$sysconfdir"
SCIM_ICONDIR="$datadir/scim/icons"
SCIM_MODULE_PATH="$libdir/scim$SCIM_EPOCH"
SCIM_LIBEXECDIR="$libdir/scim$SCIM_EPOCH"
SCIM_LOCALEDIR="$datadir/locale"
SCIM_TEMPDIR="/tmp"
SCIM_PREFIX="$prefix"
SCIM_LIBDIR="$libdir"

AC_SUBST(SCIM_DATADIR)
AC_SUBST(SCIM_SYSCONFDIR)
AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULE_PATH)
AC_SUBST(SCIM_LIBEXECDIR)
AC_SUBST(SCIM_LOCALEDIR)
AC_SUBST(SCIM_TEMPDIR)
AC_SUBST(SCIM_PREFIX)
AC_SUBST(SCIM_LIBDIR)

###########################################################
##                      Output files.                    ##
###########################################################
AC_SUBST(ac_aux_dir)
AC_CONFIG_FILES([Makefile
		 po/Makefile.in
		 protocols/Makefile
		 src/Makefile
		 src/scim_types.h
		 utils/Makefile
		 utils/x11utils/Makefile
		 utils/gtkutils/Makefile
		 utils/candidates/Makefile
		 data/Makefile
		 data/icons/Makefile
		 data/pixmaps/Makefile
		 modules/Makefile
		 modules/FrontEnd/IMdkit/Makefile
		 modules/FrontEnd/Makefile
		 modules/IMEngine/Makefile
		 modules/Filter/Makefile
		 modules/Config/Makefile
		 modules/SetupUI/Makefile
	 	 docs/scim.cfg
	 	 docs/Makefile
	 	 docs/manual/Makefile
	 	 configs/Makefile
		 extras/Makefile
		 extras/immodule/gtk2/Makefile
		 extras/immodule/gtk3/Makefile
		 extras/immodule/gtk4/Makefile
		 extras/immodule/qt5/Makefile
		 extras/immodule/qt6/Makefile
		 extras/setup/Makefile
		 extras/setup/gtk3/Makefile
		 extras/setup/scim-setup
		 extras/panel/Makefile
		 tests/Makefile
		 scim.pc
		 scim-gtkutils.pc
		 scim-x11utils.pc])
AC_OUTPUT

AC_MSG_RESULT([
Build options:
  Version                  $SCIM_VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Build tests/*            $enable_tests
  Debugging                $enable_debug
  ld version script        $enable_ld_version_script
  Build documents          $enable_documents
  Ordered (std::map) mode  $enable_ordered_map

Module options -- each is a --enable/--disable switch:
  Simple Config module     $enable_config_simple
  Socket Config module     $enable_config_socket

  X11/XWayland support     $enable_frontend_x11
  Wayland support          $enable_frontend_wayland
  Socket FrontEnd module   $enable_frontend_socket
  IBus FrontEnd module     $enable_frontend_ibus

  kimpanel candidate UI    $enable_kimpanel
  Panel SNI tray           $enable_sni
  Panel layer-shell anchor $enable_layer_shell

  RawCode IMEngine module  $enable_im_rawcode
  Socket IMEngine module   $enable_im_socket

  HanConv Filter module    $enable_filter_hanconv

  Setup UI                 $enable_setup_ui
  GTK3 legacy setup helper $enable_gtk3_legacy_setup
  GTK Panel                $enable_panel_gtk

  GTK2 IMModule            $enable_gtk2_immodule
  GTK3 IMModule            $enable_gtk3_immodule
  GTK4 IMModule            $enable_gtk4_immodule
  QT5 IMModule             $enable_qt5_immodule
  QT6 IMModule             $enable_qt6_immodule

Follows from those and from what was found -- no switch of its own:
  Cairo candidate renderer $enable_candidates_core
  X11 candidate window     $enable_candidates
  GTK Utility Library      $enable_gtk_utils
  X11 Utility Library      $enable_x11_utils
  GTK version used         $GTK_VERSION

Install paths:
  GTK2 IMModule dir        $GTK2_IM_MODULEDIR
  GTK3 IMModule dir        $GTK3_IM_MODULEDIR
  GTK4 IMModule dir        $GTK4_IM_MODULEDIR
  QT5 IMModule dir         $QT5_IM_MODULEDIR
  QT6 IMModule dir         $QT6_IM_MODULEDIR
  IBus component dir       $IBUS_COMPONENT_DIR
  QT5 moc program          $QT5_MOC
  QT6 moc program          $QT6_MOC
])
