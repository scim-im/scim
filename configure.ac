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
LT_CONFIG_LTDL_DIR([libltdl])
LT_INIT([dlopen])
LTDL_INIT
AC_SUBST(LIBTOOL_DEPS)

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
AC_CHECK_HEADERS([langinfo.h libintl.h string.h dirent.h hash_map ext/hash_map])

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

SCIM_HAS_GTK2_2=no
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
  GTK4_VERSION=4.0.0
  GTK4_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk4`
  GTK4_LIBDIR=`$PKG_CONFIG --variable=libdir gtk4`
  GTK4_IM_MODULEDIR=$GTK4_LIBDIR/gtk-4.0/$GTK4_BINARY_VERSION/immodules
  AC_SUBST(GTK4_IM_MODULEDIR)
fi



# Check if we should build the scim Qt input-method module
PKG_CHECK_MODULES(QT5, [xproto, xcb-keysyms, Qt5X11Extras >= 5.0, Qt5Widgets >= 5.0],
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


# Checks for X11 header files.
AC_PATH_XTRA

# Add -lX11 to X_LIBS
if test "$have_x" = "yes"; then
  X_LIBS="$X_LIBS -lX11"
fi

###########################################################
##             Definiation of compile args.              ##
###########################################################
AC_ARG_WITH([gtk2-im-module-dir],
	[AS_HELP_STRING([--with-gtk2-im-module-dir=dir],
		[Select GTK2 immodule dir])],
	[GTK2_IM_MODULEDIR=$with_gtk2_im_module_dir])

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

AC_ARG_WITH([gtk-version],
	[AS_HELP_STRING([--with-gtk-version=2 or 3], 
		[Select gtk version if both gtk2 and gtk3 are available (default is 3)])],
	[WANT_GTK_VERSION=$withval],
	[WANT_GTK_VERSION=3])

AC_ARG_ENABLE([ld-version-script],
	[AS_HELP_STRING([--enable-ld-version-script],
		[Use ld version script to limit exported symbols])],
	[],
	[enable_ld_version_script=no])

AC_ARG_ENABLE([ltdladvise],
	[AS_HELP_STRING([--enable-ltdladvise],
		[Use lt_dladvise for module loading])],
	[],
	[enable_ltdladvise=no])

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
		[build documents])],
	[], 
	[enable_documents=no])

AC_ARG_ENABLE([hash-map],
	[AS_HELP_STRING([--enable-hash-map],
		[Use hash_map instead of std::map internally])],
	[],
	[enable_hash_map=no])

AC_ARG_ENABLE([config-simple],
	[AS_HELP_STRING([--disable-config-simple],
		[Do not build Simple Config module])],
	[],
	[enable_config_simple=yes])

AC_ARG_ENABLE([config-socket],
	[AS_HELP_STRING([--disable-config-socket],
		[Do not build Socket Config module])],
	[],
	[enable_config_socket=yes])

AC_ARG_ENABLE([frontend-x11],
	[AS_HELP_STRING([--disable-frontend-x11],
		[Do not build X11 FrontEnd module])],
	[],
	[enable_frontend_x11=yes])

AC_ARG_ENABLE([frontend-socket],
	[AS_HELP_STRING([--disable-frontend-socket],
		[Do not build Socket FrontEnd module])],
	[], 
	[enable_frontend_socket=yes])

AC_ARG_ENABLE([im-rawcode],
	[AS_HELP_STRING([--disable-im-rawcode],
		[Do not build RawCode IMEngine module])],
	[], 
	[enable_im_rawcode=yes])

AC_ARG_ENABLE([im-socket],
	[AS_HELP_STRING([--disable-im-socket],
		[Do not build Socket IMEngine module])],
	[],
	[enable_im_socket=yes])

AC_ARG_ENABLE([filter-sctc],
	[AS_HELP_STRING([--disable-filter-sctc],
		[Do not build Simplified/Traditional Chinese conversion Filter module])],
	[],
	[enable_filter_sctc=yes])

AC_ARG_ENABLE([gtk2-immodule],
	[AS_HELP_STRING([--enable-gtk2-immodule],
		[Enable GTK2 IMModule])],
	[],
	[enable_gtk2_immodule=yes])

AC_ARG_ENABLE([gtk3-immodule],
	[AS_HELP_STRING([--enable-gtk3-immodule],
		[Enable GTK3 IMModule])],
	[],
	[enable_gtk3_immodule=yes])

AC_ARG_ENABLE([gtk4-immodule],
	[AS_HELP_STRING([--enable-gtk4-immodule],
		[Enable GTK4 IMModule])],
	[],
	[enable_gtk4_immodule=yes])

AC_ARG_ENABLE([qt5-immodule],
	[AS_HELP_STRING([--enable-qt5-immodule],
		[Enable QT5 IMModule])],
	[],
	[enable_qt5_immodule=yes])

AC_ARG_ENABLE([qt6-immodule],
	[AS_HELP_STRING([--enable-qt6-immodule],
		[Enable QT6 IMModule])],
	[],
	[enable_qt6_immodule=yes])

AC_ARG_ENABLE([panel-gtk],
	[AS_HELP_STRING([--disable-panel-gtk],
		[Do not build GTK2 Panel])],
	[], 
	[enable_panel_gtk=check])

AC_ARG_ENABLE([setup-ui],
	[AS_HELP_STRING([--disable-setup-ui],
		[Do not build Setup UI])],
	[], 
	[enable_setup_ui=yes])

if test "$have_x" = "yes"; then
  SCIM_BUILD_X11_UTILS=1
  enable_x11_utils=yes
else
  SCIM_BUILD_X11_UTILS=0
  enable_x11_utils=no
fi

if test "$enable_hash_map" = "yes"; then
  AC_DEFINE(ENABLE_HASH_MAP,1,[Use hash map instead of map in libscim])
fi

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g"
  CXXFLAGS="$CXXFLAGS -g"
else
  enable_debug=no
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

if test "$enable_ltdladvise" = "yes"; then
  SCIM_LTDLADVISE=1
else
  SCIM_LTDLADVISE=0
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

if test "$enable_filter_sctc" = "yes"; then
  SCIM_BUILD_FILTER_SCTC=1
else
  SCIM_BUILD_FILTER_SCTC=0
  enable_filter_sctc=no
fi

if test "$enable_gtk2_immodule" = "yes" -a "$SCIM_HAS_GTK2" = "yes"; then
  SCIM_BUILD_GTK2_IMMODULE=1
else
  SCIM_BUILD_GTK2_IMMODULE=0
  enable_gtk2_immodule=no
fi

if test "$enable_gtk3_immodule" = "yes" -a "$SCIM_HAS_GTK3" = "yes"; then
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

if test "$SCIM_HAS_GTK4" = "yes" -a "$SCIM_HAS_GTK3" = "yes" -a "$SCIM_HAS_GTK2" = "yes"; then
  if test "$WANT_GTK_VERSION" = "2"; then
	USE_GTK_VERSION="2"
  elif test "$WANT_GTK_VERSION" = "3"; then
	USE_GTK_VERSION="3"
  else
	USE_GTK_VERSION="4"
  fi
elif test "$SCIM_HAS_GTK4" = "yes"; then
  USE_GTK_VERSION="4"
elif test "$SCIM_HAS_GTK3" = "yes"; then
  USE_GTK_VERSION="3"
elif test "$SCIM_HAS_GTK2" = "yes"; then
  USE_GTK_VERSION="2"
fi

if test "$USE_GTK_VERSION" = "4"; then
  SCIM_HAS_GTK=yes
  GTK_LIBDIR=$GTK4_LIBDIR
  GTK_VERSION=$GTK4_VERSION
  GTK_MAJOR_VERSION=4
  GTK_BINARY_VERSION=$GTK4_BINARY_VERSION
  GTK_CFLAGS=$GTK4_CFLAGS
  GTK_LIBS=$GTK4_LIBS
  AC_SUBST(GTK_LIBDIR)
  AC_SUBST(GTK_VERSION)
  AC_SUBST(GTK_MAJOR_VERSION)
  AC_SUBST(GTK_BINARY_VERSION)
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
elif test "$USE_GTK_VERSION" = "3"; then
  SCIM_HAS_GTK=yes
  GTK_LIBDIR=$GTK3_LIBDIR
  GTK_VERSION=$GTK3_VERSION
  GTK_MAJOR_VERSION=3
  GTK_BINARY_VERSION=$GTK3_BINARY_VERSION
  GTK_CFLAGS=$GTK3_CFLAGS
  GTK_LIBS=$GTK3_LIBS
  AC_SUBST(GTK_LIBDIR)
  AC_SUBST(GTK_VERSION)
  AC_SUBST(GTK_MAJOR_VERSION)
  AC_SUBST(GTK_BINARY_VERSION)
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
elif test "$USE_GTK_VERSION" = "2"; then
  SCIM_HAS_GTK=yes
  GTK_LIBDIR=$GTK2_LIBDIR
  GTK_VERSION=$GTK2_VERSION
  GTK_MAJOR_VERSION=2
  GTK_BINARY_VERSION=$GTK2_BINARY_VERSION
  GTK_CFLAGS=$GTK2_CFLAGS
  GTK_LIBS=$GTK2_LIBS
  AC_SUBST(GTK_LIBDIR)
  AC_SUBST(GTK_VERSION)
  AC_SUBST(GTK_MAJOR_VERSION)
  AC_SUBST(GTK_BINARY_VERSION)
  AC_SUBST(GTK_CFLAGS)
  AC_SUBST(GTK_LIBS)
else
  SCIM_HAS_GTK=no
fi

if test "$enable_setup_ui" = "yes" -a "$SCIM_HAS_GTK" = "yes"; then
  SCIM_BUILD_SCIM_SETUP=1
  AC_DEFINE(HAVE_SCIM_SETUP,1,[Have scim-setup program.])
else
  SCIM_BUILD_SCIM_SETUP=0
  enable_setup_ui=no
fi

if test "$SCIM_HAS_GTK" = "yes"; then
  enable_gtk_utils=yes
  SCIM_BUILD_GTK_UTILS=1
else
  enable_gtk_utils=no
  SCIM_BUILD_GTK_UTILS=0
fi

enable_tray_icon=no
if test \( "$SCIM_HAS_GTK4"="yes" -o "$SCIM_HAS_GTK3"="yes" -o "$SCIM_HAS_GTK2_2" = "yes" \) -a "$no_x" != "yes"; then
  enable_tray_icon=yes
  AC_DEFINE(ENABLE_TRAY_ICON,1,[Enable the TrayIcon code.])
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

AM_CONDITIONAL(SCIM_LTDLADVISE,
		[test "$enable_ltdladvise" = "yes"])

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

AM_CONDITIONAL(SCIM_BUILD_FILTER_SCTC,
		[test "$enable_filter_sctc" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_SCIM_SETUP,
		[test "$enable_setup_ui" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK_UTILS,
	        [test "$enable_gtk_utils" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_X11_UTILS,
	        [test "$enable_x11_utils" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_TRAY_ICON,
	        [test "$enable_tray_icon" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_PANEL_GTK,
	        [test "$enable_panel_gtk" = "yes"])

AC_SUBST(SCIM_LTDLADVISE)
AC_SUBST(SCIM_BUILD_TESTS)
AC_SUBST(SCIM_BUILD_DOCUMENTS)
AC_SUBST(SCIM_BUILD_CONFIG_SIMPLE)
AC_SUBST(SCIM_BUILD_CONFIG_SOCKET)
AC_SUBST(SCIM_BUILD_FRONTEND_X11)
AC_SUBST(SCIM_BUILD_FRONTEND_SOCKET)
AC_SUBST(SCIM_BUILD_IMENGINE_RAWCODE)
AC_SUBST(SCIM_BUILD_IMENGINE_TABLE)
AC_SUBST(SCIM_BUILD_IMENGINE_SOCKET)
AC_SUBST(SCIM_BUILD_FILTER_SCTC)
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
		 src/Makefile
		 src/scim_types.h
		 utils/Makefile
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
		 extras/gtk_immodule/gtk2/Makefile
		 extras/gtk_immodule/gtk3/Makefile
		 extras/gtk_immodule/gtk4/Makefile
		 extras/gtk_immodule/qt5/Makefile
		 extras/gtk_immodule/qt6/Makefile
		 extras/setup/Makefile
		 extras/setup/scim-setup
		 extras/panel/Makefile
		 tests/Makefile
		 scim.pc
		 scim-gtkutils.pc
		 scim-x11utils.pc
		 scim.spec])
AC_OUTPUT

AC_MSG_RESULT([
Build options:
  Version                  $SCIM_VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Build tests/*            $enable_tests
  Enable debug             $enable_debug
  Enable ld version script $enable_ld_version_script
  Enable lt_dladvise       $enable_ltdladvise
  Build documents          $enable_documents

Module options:           
  Simple config module     $enable_config_simple
  Socket config module     $enable_config_socket

  X11 FrontEnd module      $enable_frontend_x11
  Socket FrontEnd module   $enable_frontend_socket

  RawCode IMEngine module  $enable_im_rawcode
  Socket IMEngine module   $enable_im_socket

  SCTC Filter module       $enable_filter_sctc

  GTK Utility Library      $enable_gtk_utils
  X11 Utility Library      $enable_x11_utils

  GTK Version              $USE_GTK_VERSION

  GUI Setup Utility        $enable_setup_ui

  GTK Panel GUI            $enable_panel_gtk

  Enable TrayIcon          $enable_tray_icon

  GTK2 IMModule dir        $GTK2_IM_MODULEDIR
  GTK3 IMModule dir        $GTK3_IM_MODULEDIR
  GTK4 IMModule dir        $GTK4_IM_MODULEDIR
  QT5 IMModule dir         $QT5_IM_MODULEDIR

  QT5 moc program          $QT5_MOC

  GTK2 IMModule            $enable_gtk2_immodule
  GTK3 IMModule            $enable_gtk3_immodule
  GTK4 IMModule            $enable_gtk4_immodule
  QT5 IMModule             $enable_qt5_immodule
  QT6 IMModule             $enable_qt6_immodule
])
