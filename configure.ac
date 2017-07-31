# Process this file with autoconf to produce a configure script
AC_INIT([scim],[1.4.18],[https://github.com/scim-im/scim])
AC_CONFIG_SRCDIR([src/scim.h])


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
AM_INIT_AUTOMAKE([])
AM_CONFIG_HEADER([config.h])

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

# Init gettext
ALL_LINGUAS="as bn_IN ca cs de es fi fr gu hi it ja kn ko ml mr nl pa pt_BR ru sk sv ta te vi zh_CN zh_TW"
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION(0.14.1)

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

# Check intltool
IT_PROG_INTLTOOL([0.33], [no-xml])

# Checks for libraries.
AC_HEADER_STDC
AC_HEADER_TIME
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
PKG_CHECK_MODULES(GTK2, [gtk+-2.0 >= 2.4.0 pango >= 1.1.0 gdk-pixbuf-2.0 >= 2.4.0],
			[SCIM_HAS_GTK2=yes],
			[SCIM_HAS_GTK2=no])

PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.0.0 pango >= 1.1.0 gdk-pixbuf-2.0 >= 2.4.0],
			[SCIM_HAS_GTK3=yes],
			[SCIM_HAS_GTK3=no])

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

# Check if we have gthread
PKG_CHECK_MODULES(GTHREAD2,[gthread-2.0 >= 2.0.0],
			[SCIM_HAS_GTHREAD2=yes],
			[SCIM_HAS_GTHREAD2=no])


# Check if we should build scim-bridge-qt-immodule
PKG_CHECK_MODULES(QT3, [qt-mt >= 3.3],
            [SCIM_HAS_QT3=yes],
            [SCIM_HAS_QT3=no])

if test "$SCIM_HAS_QT3" = "yes"; then
  if test -z "$QT3_PREFIX"; then
    QT3_PREFIX=`$PKG_CONFIG --variable=prefix qt-mt`
    QT3_LIBDIR=`$PKG_CONFIG --variable=libdir qt-mt`
  fi
  QT3_MOC=$QT3_PREFIX/bin/moc
  QT3_IM_MODULEDIR=$QT3_LIBDIR/qt3/plugins/inputmethods
  AC_SUBST(QT3_MOC)
  AC_SUBST(QT3_IM_MODULEDIR)
  AC_SUBST(QT3_PREFIX)
  AC_SUBST(QT3_LIBDIR)
fi

PKG_CHECK_MODULES(QT4, [QtGui >= 4.0, QtCore >= 4.0],
            [SCIM_HAS_QT4=yes],
            [SCIM_HAS_QT4=no])

if test "$SCIM_HAS_QT4" = "yes"; then
  if test -z "$QT4_PREFIX"; then
    QT4_PREFIX=`$PKG_CONFIG --variable=prefix QtCore`
    QT4_LIBDIR=`$PKG_CONFIG --variable=libdir QtCore`
  fi
  QT4_MOC=$QT4_PREFIX/bin/moc
  QT4_IM_MODULEDIR=$QT4_LIBDIR/qt4/plugins/inputmethods
  AC_SUBST(QT4_MOC)
  AC_SUBST(QT4_IM_MODULEDIR)
  AC_SUBST(QT4_PREFIX)
  AC_SUBST(QT4_LIBDIR)
fi

# Check if we should build scim-bridge-clutter-immodule
PKG_CHECK_MODULES(CLUTTER, clutter-1.0,
           [SCIM_HAS_CLUTTER=yes],
           [SCIM_HAS_CLUTTER=no])

PKG_CHECK_MODULES(CLUTTERIMCONTEXT, clutter-imcontext-0.1,
           [SCIM_HAS_CLUTTERIMCONTEXT=yes],
           [SCIM_HAS_CLUTTERIMCONTEXT=no])

if test "$SCIM_HAS_CLUTTER" = "yes" -a "$SCIM_HAS_CLUTTERIMCONTEXT" = "yes"; then
  CLUTTERIMCONTEXT_LIBDIR=`$PKG_CONFIG --variable=libdir clutter-imcontext-0.1`
  CLUTTER_IM_MODULEDIR=$CLUTTERIMCONTEXT_LIBDIR/clutter-imcontext/immodules
  AC_SUBST(CLUTTER_IM_MODULEDIR)
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

AC_ARG_WITH([qt3-moc],
	[AS_HELP_STRING([--with-qt3-moc=file],
		[Select QT3 moc program (default to QT3_PREFIX/bin/moc)])],
	[QT3_MOC=$with_qt3_moc])

AC_ARG_WITH([qt4-moc],
	[AS_HELP_STRING([--with-qt4-moc=file],
		[Select QT4 moc program (default to QT4_PREFIX/bin/moc)])],
	[QT4_MOC=$with_qt4_moc])

AC_ARG_WITH([qt3-im-module-dir],
	[AS_HELP_STRING([--with-qt3-im-module-dir=dir],
		[Select QT3 immodule dir])],
	[QT3_IM_MODULEDIR=$with_qt3_im_module_dir])

AC_ARG_WITH([qt4-im-module-dir],
	[AS_HELP_STRING([--with-qt4-im-module-dir=dir],
		[Select QT4 immodule dir])],
	[QT4_IM_MODULEDIR=$with_qt4_im_module_dir])

AC_ARG_WITH([clutter-im-module-dir],
	[AS_HELP_STRING([--with-clutter-im-module-dir=dir],
		[Select clutter immodule dir])],
	[CLUTTER_IM_MODULEDIR=$with_clutter_im_module_dir])

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

AC_ARG_ENABLE([orig-gtk2-immodule],
	[AS_HELP_STRING([--enable-orig-gtk2-immodule],
		[Build original GTK2 IMModule])],
	[],
	[enable_orig_gtk2_immodule=no])

AC_ARG_ENABLE([orig-gtk3-immodule],
	[AS_HELP_STRING([--enable-orig-gtk3-immodule],
		[Build original GTK3 IMModule])],
	[],
	[enable_orig_gtk3_immodule=no])

AC_ARG_ENABLE([agent],
	[AS_HELP_STRING([--disable-im-agent],
		[Do not build scim-im-agent (required by other immodules)])],
	[], 
	[enable_im_agent=yes])

AC_ARG_ENABLE([gtk2-immodule],
	[AS_HELP_STRING([--disable-gtk2-immodule],
		[Do not build GTK2 IMModule])],
	[],
	[enable_gtk2_immodule=yes])

AC_ARG_ENABLE([gtk3-immodule],
	[AS_HELP_STRING([--disable-gtk3-immodule],
		[Do not build GTK3 IMModule])],
	[],
	[enable_gtk3_immodule=yes])

AC_ARG_ENABLE([qt3-immodule],
	[AS_HELP_STRING([--disable-qt3-immodule],
		[Do not build QT3 IMModule])],
	[],
	[enable_qt3_immodule=yes])

AC_ARG_ENABLE([qt4-immodule],
	[AS_HELP_STRING([--disable-qt4-immodule],
		[Do not build QT4 IMModule])],
	[],
	[enable_qt4_immodule=yes])

AC_ARG_ENABLE([clutter-immodule],
	[AS_HELP_STRING([--enable-clutter-immodule],
		[Do not build CLUTTER IMModule])],
	[], 
	[enable_clutter_immodule=no])

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

if test "$enable_orig_gtk2_immodule" = "yes" -a "$SCIM_HAS_GTK2" = "yes"; then
  SCIM_BUILD_ORIG_GTK2_IMMODULE=1
else
  SCIM_BUILD_ORIG_GTK2_IMMODULE=0
  enable_orig_gtk2_immodule=no
fi

if test "$enable_orig_gtk3_immodule" = "yes" -a "$SCIM_HAS_GTK3" = "yes"; then
  SCIM_BUILD_ORIG_GTK3_IMMODULE=1
else
  SCIM_BUILD_ORIG_GTK3_IMMODULE=0
  enable_orig_gtk3_immodule=no
fi

if test "$enable_gtk3_immodule" = "yes" -a "$SCIM_HAS_GTK3" = "yes"; then
  SCIM_BUILD_GTK3_IMMODULE=1
  SCIM_BUILD_IMMODULE=1
  enable_immodule=yes
else
  SCIM_BUILD_GTK3_IMMODULE=0
  enable_gtk3_immodule=no
fi

if test "$enable_im_agent" = "yes"; then
  SCIM_BUILD_IM_AGENT=1
else
  SCIM_BUILD_IM_AGENT=0
  enable_im_agent=no
fi

SCIM_BUILD_IMMODULE=0
enable_immodule=no

if test "$enable_gtk2_immodule" = "yes" -a "$SCIM_HAS_GTK2" = "yes"; then
  SCIM_BUILD_GTK2_IMMODULE=1
  SCIM_BUILD_IMMODULE=1
  enable_immodule=yes
  SCIM_BUILD_IM_AGENT=1
  enable_im_agent=yes
else
  SCIM_BUILD_GTK2_IMMODULE=0
  enable_gtk2_immodule=no
fi

if test "$enable_gtk3_immodule" = "yes" -a "$SCIM_HAS_GTK3" = "yes"; then
  SCIM_BUILD_GTK3_IMMODULE=1
  SCIM_BUILD_IMMODULE=1
  enable_immodule=yes
  SCIM_BUILD_IM_AGENT=1
  enable_im_agent=yes
else
  SCIM_BUILD_GTK3_IMMODULE=0
  enable_gtk3_immodule=no
fi

if test "$enable_qt3_immodule" = "yes" -a "$SCIM_HAS_QT3" = "yes"; then
  SCIM_BUILD_QT3_IMMODULE=1
  SCIM_BUILD_IMMODULE=1
  enable_immodule=yes
  SCIM_BUILD_IM_AGENT=1
  enable_im_agent=yes
else
  SCIM_BUILD_QT3_IMMODULE=0
  enable_qt3_immodule=no
fi

if test "$enable_qt4_immodule" = "yes" -a "$SCIM_HAS_QT4" = "yes"; then
  SCIM_BUILD_QT4_IMMODULE=1
  SCIM_BUILD_IMMODULE=1
  enable_immodule=yes
  SCIM_BUILD_IM_AGENT=1
  enable_im_agent=yes
else
  SCIM_BUILD_QT4_IMMODULE=0
  enable_qt4_immodule=no
fi

if test "$enable_clutter_immodule" = "yes" -a \
	"$SCIM_HAS_CLUTTER" = "yes" -a \
	"$SCIM_HAS_CLUTTERIMCONTEXT" = "yes" ; then
  SCIM_BUILD_CLUTTER_IMMODULE=1
  SCIM_BUILD_IMMODULE=1
  enable_immodule=yes
  SCIM_BUILD_IM_AGENT=1
  enable_im_agent=yes
else
  SCIM_BUILD_CLUTTER_IMMODULE=0
  enable_clutter_immodule=no
fi

if test "$SCIM_HAS_GTK3" = "yes" -a "$SCIM_HAS_GTK2" = "yes"; then
  if test "$WANT_GTK_VERSION" = "2"; then
	USE_GTK_VERSION="2"
  else
	USE_GTK_VERSION="3"
  fi
elif test "$SCIM_HAS_GTK3" = "yes"; then
  USE_GTK_VERSION="3"
elif test "$SCIM_HAS_GTK2" = "yes"; then
  USE_GTK_VERSION="2"
fi

if test "$USE_GTK_VERSION" = "3"; then
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
if test \( "$SCIM_HAS_GTK3"="yes" -o "$SCIM_HAS_GTK2_2" = "yes" \) -a "$no_x" != "yes"; then
  enable_tray_icon=yes
  AC_DEFINE(ENABLE_TRAY_ICON,1,[Enable the TrayIcon code.])
fi

SCIM_BUILD_PANEL_GTK=0
if test "$enable_panel_gtk" != "no"; then
  enable_panel_gtk=no
  if test "$SCIM_HAS_GTHREAD2" = "yes" -a "$SCIM_HAS_GTK" = "yes"; then
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

AM_CONDITIONAL(SCIM_BUILD_ORIG_GTK2_IMMODULE,
		[test "$enable_orig_gtk2_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_ORIG_GTK3_IMMODULE,
		[test "$enable_orig_gtk3_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_IM_AGENT,
        [test "$enable_im_agent" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK2_IMMODULE,
        [test "$enable_gtk2_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_GTK3_IMMODULE,
        [test "$enable_gtk3_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_QT3_IMMODULE,
        [test "$enable_qt3_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_QT4_IMMODULE,
        [test "$enable_qt4_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_CLUTTER_IMMODULE,
        [test "$enable_clutter_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BUILD_IMMODULE,
        [test "$enable_immodule" = "yes"])

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
AC_SUBST(SCIM_BUILD_ORIG_GTK2_IMMODULE)
AC_SUBST(SCIM_BUILD_ORIG_GTK3_IMMODULE)
AC_SUBST(SCIM_BUILD_SCIM_SETUP)
AC_SUBST(SCIM_BUILD_PANEL_GTK)
AC_SUBST(SCIM_BUILD_GTK_UTILS)
AC_SUBST(SCIM_BUILD_X11_UTILS)
AC_SUBST(SCIM_BUILD_IM_AGENT)
AC_SUBST(SCIM_BUILD_GTK2_IMMODULE)
AC_SUBST(SCIM_BUILD_GTK3_IMMODULE)
AC_SUBST(SCIM_BUILD_QT3_IMMODULE)
AC_SUBST(SCIM_BUILD_QT4_IMMODULE)
AC_SUBST(SCIM_BUILD_CLUTTER_IMMODULE)
AC_SUBST(SCIM_BUILD_IMMODULE)


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
		 extras/setup/Makefile
		 extras/setup/scim-setup
		 extras/panel/Makefile
		 extras/immodules/Makefile 
		 extras/immodules/common/Makefile
		 extras/immodules/agent/Makefile
		 extras/immodules/client-common/Makefile
		 extras/immodules/client-gtk/gtk2/Makefile
		 extras/immodules/client-gtk/gtk3/Makefile
		 extras/immodules/client-qt/qt3/Makefile
		 extras/immodules/client-qt/qt4/Makefile
		 extras/immodules/client-clutter/Makefile
		 extras/immodules/doc/Makefile
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
  QT3 IMModule dir         $QT3_IM_MODULEDIR
  QT4 IMModule dir         $QT4_IM_MODULEDIR
  Clutter IMModule dir     $CLUTTER_IM_MODULEDIR

  QT3 moc program          $QT3_MOC
  QT4 moc program          $QT4_MOC

  Original GTK2 IMModule   $enable_orig_gtk2_immodule
  Original GTK3 IMModule   $enable_orig_gtk3_immodule

  Scim IM Agent            $enable_im_agent
  GTK2 IMModule            $enable_gtk2_immodule
  GTK3 IMModule            $enable_gtk3_immodule
  QT3 IMModule             $enable_qt3_immodule
  QT4 IMModule             $enable_qt4_immodule
  CLUTTER IMModule         $enable_clutter_immodule
])
