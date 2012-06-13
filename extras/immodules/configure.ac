# Process this file with autoconf to produce a configure script
AC_INIT(scim-bridge, 0.4.17, ryo-dairiki@users.sourceforge.net)
AC_CONFIG_SRCDIR([common/scim-bridge.h])

###########################################################
##          Some definitions of Version macros.          ##
###########################################################

SCIM_BRIDGE_MAJOR_VERSION=0
SCIM_BRIDGE_MINOR_VERSION=4
SCIM_BRIDGE_MICRO_VERSION=17

SCIM_BRIDGE_VERSION=$SCIM_BRIDGE_MAJOR_VERSION.$SCIM_BRIDGE_MINOR_VERSION.$SCIM_BRIDGE_MICRO_VERSION

AC_SUBST(SCIM_BRIDGE_MAJOR_VERSION)
AC_SUBST(SCIM_BRIDGE_MINOR_VERSION)
AC_SUBST(SCIM_BRIDGE_MICRO_VERSION)
AC_SUBST(SCIM_BRIDGE_VERSION)

AC_DEFINE_UNQUOTED(SCIM_BRIDGE_VERSION, "$SCIM_BRIDGE_VERSION", [The release version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_BRIDGE_MAJOR_VERSION, $SCIM_BRIDGE_MAJOR_VERSION, [The Major version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_BRIDGE_MINOR_VERSION, $SCIM_BRIDGE_MINOR_VERSION, [The Minor version of SCIM library.])
AC_DEFINE_UNQUOTED(SCIM_BRIDGE_MICRO_VERSION, $SCIM_BRIDGE_MICRO_VERSION, [The Micro version of SCIM library.])

#required scim version
SCIM_VERSION=1.4.6

###########################################################
##                  Start Configuration.                 ##
###########################################################

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

AC_GNU_SOURCE

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
# !!! DO NOT USE IT !!!
LIBTOOL_EXPORT_OPTIONS=''
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Init libtool
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AM_CONDITIONAL(HAVE_DOCBOOK, test x$DOCBOOK_XSL != xno)
AM_CONDITIONAL(BUILD_MANUAL, test x$XSLTPROC != xno -a x$DOCBOOK_XSL != xno)

# Check if we should build scim-bridge-gtk2-immodule
PKG_CHECK_MODULES(GTK2, [gtk+-2.0 >= 2.4.0 pango >= 1.1.0 gdk-pixbuf-2.0 >= 2.4.0],
			[SCIM_BRIDGE_HAS_GTK2=yes],
			[SCIM_BRIDGE_HAS_GTK2=no])

# Check if we should build scim-bridge-gtk3-immodule
PKG_CHECK_MODULES(GTK3, [gtk+-3.0 >= 3.0.0 pango >= 1.24.0 gdk-pixbuf-2.0 >= 2.22.1],
			[SCIM_BRIDGE_HAS_GTK3=yes],
			[SCIM_BRIDGE_HAS_GTK3=no])

SCIM_BRIDGE_HAS_GTK2_2=no
if test "$SCIM_BRIDGE_HAS_GTK2" = "yes"; then
  if $PKG_CONFIG --exists "gtk+-2.0 >= 2.2" ; then
    SCIM_BRIDGE_HAS_GTK2_2=yes
    GTK2_VERSION=2.2.0
    AC_DEFINE(GDK_MULTIHEAD_SAFE,1,[Force use of GDK's multihead-safe APIs.])
  fi
  if $PKG_CONFIG --exists "gtk+-2.0 >= 2.3.5" ; then
    SCIM_BRIDGE_HAS_GTK2_4=yes
    GTK2_VERSION=2.3.5
    AC_DEFINE(HAVE_GTK_DRAW_INSERTION_CURSOR,1,[Have gtk_draw_insertion_cursor ().])
  fi
  GTK2_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-2.0`
  GTK2_LIBDIR=`$PKG_CONFIG --variable=libdir gtk+-2.0`
  if test -z "$GTK2_LIBDIR)"; then
    GTK2_LIBDIR="$libdir"
  fi
  AC_SUBST(GTK2_LIBDIR)
  AC_SUBST(GTK2_VERSION)
  AC_SUBST(GTK2_BINARY_VERSION)
fi

if test "$SCIM_BRIDGE_HAS_GTK3" = "yes"; then
  GTK3_BINARY_VERSION=`$PKG_CONFIG --variable=gtk_binary_version gtk+-3.0`
  GTK3_LIBDIR=`$PKG_CONFIG --variable=libdir gtk+-3.0`
  if test -z "$GTK3_LIBDIR)"; then
    GTK3_LIBDIR="$libdir"
  fi
  AC_SUBST(GTK3_LIBDIR)
  AC_SUBST(GTK3_VERSION)
  AC_SUBST(GTK3_BINARY_VERSION)
fi

# Check if we should build scim-bridge-qt-immodule
PKG_CHECK_MODULES(QT3, [qt-mt >= 3.3],
			[SCIM_BRIDGE_HAS_QT3=yes],
			[SCIM_BRIDGE_HAS_QT3=no])

if test "$SCIM_BRIDGE_HAS_QT3" = "yes"; then
  if test -z "$QT3_PREFIX"; then
    QT3_PREFIX=`$PKG_CONFIG --variable=prefix qt-mt`
  fi
  AC_SUBST(QT3_PREFIX)
fi

PKG_CHECK_MODULES(QT4, [QtGui >= 4.0, QtCore >= 4.0],
			[SCIM_BRIDGE_HAS_QT4=yes],
			[SCIM_BRIDGE_HAS_QT4=no])

if test "$SCIM_BRIDGE_HAS_QT4" = "yes"; then
  if test -z "$QT4_PREFIX"; then
    QT4_PREFIX=`$PKG_CONFIG --variable=prefix QtCore`
  fi
  AC_SUBST(QT4_PREFIX)
fi

# Check if we should build scim-bridge-clutter-immodule
PKG_CHECK_MODULES(CLUTTER, clutter-0.9 clutter-imcontext-0.1,
           [SCIM_BRIDGE_HAS_CLUTTER=yes],
           [SCIM_BRIDGE_HAS_CLUTTER=no])

if test "$SCIM_BRIDGE_HAS_CLUTTER" = "yes"; then
  CLUTTER_IMCONTEXT_LIBDIR=`$PKG_CONFIG --variable=libdir clutter-imcontext-0.1`
  AC_SUBST(CLUTTER_LIBDIR)
  AC_SUBST(CLUTTER_VERSION)
  AC_SUBST(CLUTTER_BINARY_VERSION)
  AC_SUBST(CLUTTER_IMCONTEXT_LIBDIR)
  enable_clutter_immodule="yes";
fi

###########################################################
##             Definiation of compile args.              ##
###########################################################
AC_ARG_ENABLE(debug,
	      [AS_HELP_STRING([--enable-debug], [turn on debugging])], , 
              enable_debug=no)

AC_ARG_ENABLE(tests,
	      [AS_HELP_STRING([--enable-tests], [build tests])], , 
       	      enable_tests=yes)

AC_ARG_ENABLE(documents,
	      [AS_HELP_STRING([--enable-documents], [build documents])], , 
       	      enable_documents=yes)

AC_ARG_ENABLE(agent,
	      [AS_HELP_STRING([--enable-agent], [build scim-bridge-agent])], , 
       	      enable_agent=yes)

AC_ARG_ENABLE(gtk2-immodule,
	      [AS_HELP_STRING([--enable-gtk2-immodule], [build GTK2 IM module])], , 
		  enable_gtk2_immodule=yes)

AC_ARG_ENABLE(gtk3-immodule,
	      [AS_HELP_STRING([--enable-gtk3-immodule], [build GTK3 IM module])], , 
		  enable_gtk3_immodule=yes)

AC_ARG_ENABLE(qt3-immodule,
	      [AS_HELP_STRING([--enable-qt3-immodule], [build Qt3 IM module])], , 
		  enable_qt3_immodule=yes)

AC_ARG_ENABLE(qt4-immodule,
	      [AS_HELP_STRING([--enable-qt4-immodule], [build Qt4 IM module])], , 
		  enable_qt4_immodule=yes)

AC_ARG_ENABLE(clutter-immodule,
	      [AS_HELP_STRING([--enable-clutter-immodule], [build CLUTTER IM module])], , )

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g -Wall -Wmissing-declarations -Wreturn-type"
  CXXFLAGS="$CXXFLAGS -g -Wall -Wshadow -Woverloaded-virtual -Wreturn-type"
else
  enable_debug=no
fi

if test "$enable_tests" = "yes"; then
  SCIM_BRIDGE_BUILD_TESTS=1
else
  SCIM_BRIDGE_BUILD_TESTS=0
  enable_tests=no
fi

if test "$enable_documents" = "yes"; then
  SCIM_BRIDGE_BUILD_DOCUMENTS=1
else
  SCIM_BRIDGE_BUILD_DOCUMENTS=0
  enable_documents=no
fi

if test "$enable_agent" = "yes"; then
  PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])
  SCIM_BRIDGE_BUILD_AGENT=1
else
  SCIM_BRIDGE_BUILD_AGENT=0
  enable_agent=no
fi

SCIM_BRIDGE_BUILD_IMMODULE=0
enable_immodule=no

if test "$enable_gtk2_immodule" = "yes" -a "$SCIM_BRIDGE_HAS_GTK2" = "yes"; then
  SCIM_BRIDGE_BUILD_GTK2_IMMODULE=1
  SCIM_BRIDGE_BUILD_IMMODULE=1
  enable_immodule=yes
else
  SCIM_BRIDGE_BUILD_GTK2_IMMODULE=0
  enable_gtk2_immodule=no
fi

if test "$enable_gtk3_immodule" = "yes" -a "$SCIM_BRIDGE_HAS_GTK3" = "yes"; then
  SCIM_BRIDGE_BUILD_GTK3_IMMODULE=1
  SCIM_BRIDGE_BUILD_IMMODULE=1
  enable_immodule=yes
else
  SCIM_BRIDGE_BUILD_GTK3_IMMODULE=0
  enable_gtk3_immodule=no
fi

if test "$enable_qt3_immodule" = "yes" -a "$SCIM_BRIDGE_HAS_QT3" = "yes"; then
  SCIM_BRIDGE_BUILD_QT3_IMMODULE=1
  SCIM_BRIDGE_BUILD_IMMODULE=1
  enable_immodule=yes
else
  SCIM_BRIDGE_BUILD_QT3_IMMODULE=0
  enable_qt3_immodule=no
fi

if test "$enable_qt4_immodule" = "yes" -a "$SCIM_BRIDGE_HAS_QT4" = "yes"; then
  SCIM_BRIDGE_BUILD_QT4_IMMODULE=1
  SCIM_BRIDGE_BUILD_IMMODULE=1
  enable_immodule=yes
else
  SCIM_BRIDGE_BUILD_QT4_IMMODULE=0
  enable_qt4_immodule=no
fi

if test "$enable_clutter_immodule" = "yes"; then
  SCIM_BRIDGE_BUILD_CLUTTER_IMMODULE=1
  SCIM_BRIDGE_BUILD_IMMODULE=1
  if test "$SCIM_BRIDGE_HAS_CLUTTER" = "no" ; then
     AC_MSG_ERROR([CLUTTER can not be found!])
  fi
  enable_immodule=yes
else
  SCIM_BRIDGE_BUILD_CLUTTER_IMMODULE=0
  enable_clutter_immodule=no
fi

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_TESTS,
		[test "$enable_tests" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_DOCUMENTS,
		[test "$enable_documents" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_AGENT,
		[test "$enable_agent" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_GTK2_IMMODULE,
		[test "$enable_gtk2_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_GTK3_IMMODULE,
		[test "$enable_gtk3_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_QT3_IMMODULE,
		[test "$enable_qt3_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_QT4_IMMODULE,
		[test "$enable_qt4_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_CLUTTER_IMMODULE,
		[test "$enable_clutter_immodule" = "yes"])

AM_CONDITIONAL(SCIM_BRIDGE_BUILD_IMMODULE, [test "$enable_immodule" = "yes"])

AC_SUBST(SCIM_BRIDGE_BUILD_TESTS)
AC_SUBST(SCIM_BRIDGE_BUILD_DOCUMENTS)
AC_SUBST(SCIM_BRIDGE_BUILD_AGENT)
AC_SUBST(SCIM_BRIDGE_BUILD_GTK2_IMMODULE)
AC_SUBST(SCIM_BRIDGE_BUILD_GTK3_IMMODULE)
AC_SUBST(SCIM_BRIDGE_BUILD_QT3_IMMODULE)
AC_SUBST(SCIM_BRIDGE_BUILD_QT4_IMMODULE)
AC_SUBST(SCIM_BRIDGE_BUILD_CLUTTER_IMMODULE)
AC_SUBST(SCIM_BRIDGE_BUILD_IMMODULE)

# Output All necessary Paths.
SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`
SCIM_PREFIX=`$PKG_CONFIG --variable=prefix scim`
SCIM_LIBDIR=`$PKG_CONFIG --variable=libdir scim`
SCIM_LOCALEDIR=`$PKG_CONFIG --variable=localedir scim`

if test "x$SCIM_ICONDIR" = "x"; then
  SCIM_ICONDIR=${datadir}/scim/icons
fi

if test "x$SCIM_MODULEDIR" = "x"; then
  SCIM_MODULEDIR=${libdir}/scim-1.0
fi

if test "x$SCIM_DATADIR" = "x"; then
  SCIM_DATADIR=${datadir}/scim
fi

if test "x$SCIM_LIBEXECDIR" = "x"; then
  SCIM_LIBEXECDIR="${SCIM_LIBDIR}/scim-1.0"
fi

AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)
AC_SUBST(SCIM_LIBEXECDIR)
AC_SUBST(SCIM_VERSION)
AC_SUBST(SCIM_LOCALDIR)

###########################################################
##                      Output files.                    ##
###########################################################
AC_SUBST(ac_aux_dir)
AC_CONFIG_FILES([Makefile common/Makefile agent/Makefile client-common/Makefile client-gtk/gtk2/Makefile client-gtk/gtk3/Makefile client-qt/qt3/Makefile client-qt/qt4/Makefile client-clutter/Makefile tests/Makefile doc/Makefile])

AC_OUTPUT

AC_MSG_RESULT([
Build options:
  Version                  $SCIM_BRIDGE_VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Enable debug             $enable_debug

Module options:
  Build Tests              $enable_tests
  Build Documents          $enable_documents
  Build Agent              $enable_agent
  Build GTK2 IMModule      $enable_gtk2_immodule
  Build GTK3 IMModule      $enable_gtk3_immodule
  Build QT3 IMModule       $enable_qt3_immodule
  Build QT4 IMModule       $enable_qt4_immodule
  Build CLUTTER IMModule   $enable_clutter_immodule
])

