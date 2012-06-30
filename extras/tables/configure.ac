# Process this file with autoconf to produce a configure script.
AC_INIT(scim-tables, 0.5.10, suzhe@tsinghua.org.cn) #do not forget to edit skim/configure.in.in to change the version
AC_CONFIG_SRCDIR([src/scim_table_imengine.h])

SCIM_VERSION=1.4.9

AC_SUBST(SCIM_VERSION)

GETTEXT_PACKAGE=scim-tables
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext domain])

# Init automake stuff
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])

# Init gettext
ALL_LINGUAS="zh_CN zh_TW ja ko de fr it pa nl"
AM_GNU_GETTEXT

AC_LIBTOOL_WIN32_DLL
AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL
AC_LIB_LTDL
AC_SUBST(LIBTOOL_DEPS)

AC_PROG_CXX
AC_PROG_CC
AC_LANG(C++)

AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T

# Check SCIM 
PKG_CHECK_MODULES(SCIM,[scim >= $SCIM_VERSION])

# Check if we should build setup module
PKG_CHECK_MODULES(SCIM_GTKUTILS,[scim-gtkutils >= $SCIM_VERSION],
		  		[SCIM_HAS_GTKUTILS=yes],
				[SCIM_HAS_GTKUTILS=no])

if test "$SCIM_HAS_GTKUTILS" = "yes"; then
  SCIM_BUILD_SETUP=1
else
  SCIM_BUILD_SETUP=0
fi

AM_CONDITIONAL(SCIM_BUILD_SETUP, [test "$SCIM_HAS_GTKUTILS" = "yes"])
AC_SUBST(SCIM_BUILD_SETUP)

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([langinfo.h libintl.h string.h hash_map ext/hash_map])

# libtool option to control which symbols are exported
# right now, symbols starting with _ are not exported
LIBTOOL_EXPORT_OPTIONS='-export-symbols-regex "^[[^_]].*"'
AC_SUBST(LIBTOOL_EXPORT_OPTIONS)

# Extra args.
AC_ARG_ENABLE(debug,
	      [  --enable-debug          Turn on debugging], ,
	      enable_debug=no)

AC_ARG_ENABLE(skim-support,
              [  --enable-skim-support  Enable skim Setup UI], ,  
              enable_skim_support=yes)

AM_CONDITIONAL(SCIM_BUILD_SKIM_SETUP, [test "$enable_skim_support" = "yes"])

if test "$enable_debug" = "yes"; then
  AC_DEFINE(ENABLE_DEBUG,1,[Define this to enable the debug facility in libscim])
  CFLAGS="$CFLAGS -g"
  CXXFLAGS="$CXXFLAGS -g"
fi

SCIM_ICONDIR=`$PKG_CONFIG --variable=icondir scim`
SCIM_MODULEDIR=`$PKG_CONFIG --variable=moduledir scim`
SCIM_DATADIR=`$PKG_CONFIG --variable=scimdatadir scim`

if test "x$SCIM_ICONDIR" = "x"; then
  SCIM_ICONDIR=${datadir}/scim/icons
fi

if test "x$SCIM_MODULEDIR" = "x"; then
  SCIM_MODULEDIR=${libdir}/scim-1.0
fi

if test "x$SCIM_DATADIR" = "x"; then
  SCIM_DATADIR=${datadir}/scim
fi

AC_SUBST(SCIM_ICONDIR)
AC_SUBST(SCIM_MODULEDIR)
AC_SUBST(SCIM_DATADIR)

SCIM_TABLEDIR=$SCIM_DATADIR/tables

AC_SUBST(SCIM_TABLEDIR)

AC_CONFIG_FILES([Makefile  intl/Makefile po/Makefile.in m4/Makefile 
		 docs/Makefile
		 icons/Makefile
		 src/Makefile
		 scim-tables.spec
		 tables/Makefile
		 tables/additional/IPA-Kirshenbaum.txt
		 tables/additional/IPA-X-SAMPA.txt
		 tables/additional/LaTeX.txt
		 tables/additional/Makefile
		 tables/am/Amharic.txt
		 tables/am/Makefile
		 tables/ar/Arabic.txt
		 tables/ar/Makefile
		 tables/bn/Bengali-inscript.txt
		 tables/bn/Bengali-probhat.txt
		 tables/bn/Makefile
		 tables/gr/Makefile
		 tables/gr/greekpoly.txt
		 tables/gu/Gujarati-inscript.txt
		 tables/gu/Gujarati-phonetic.txt
		 tables/gu/Makefile
		 tables/he/Makefile
		 tables/he/classicalhebrew.txt
		 tables/hi/Hindi-inscript.txt
		 tables/hi/Hindi-phonetic.txt
		 tables/hi/Hindi-remington.txt
		 tables/hi/Makefile
		 tables/ja/HIRAGANA.txt
		 tables/ja/KATAKANA.txt
		 tables/ja/Makefile
		 tables/ja/Nippon.txt
		 tables/kn/Kannada-inscript.txt
		 tables/kn/Kannada-kgp.txt
		 tables/kn/Makefile
		 tables/ko/Hangul.txt
		 tables/ko/HangulRomaja.txt
		 tables/ko/Hanja.txt
		 tables/ko/Makefile
		 tables/ml/Makefile
		 tables/ml/Malayalam-inscript.txt
		 tables/ml/Malayalam-phonetic.txt
		 tables/mr/Makefile
		 tables/mr/Marathi-remington.txt
		 tables/ne/Makefile
		 tables/ne/Nepali_Rom.txt
		 tables/ne/Nepali_Trad.txt
		 tables/pa/Makefile
		 tables/pa/Punjabi-inscript.txt
		 tables/pa/Punjabi-jhelum.txt
		 tables/pa/Punjabi-phonetic.txt
		 tables/pa/Punjabi-remington.txt
		 tables/ru/Makefile
		 tables/ru/RussianTraditional.txt
		 tables/ru/Translit.txt
		 tables/ru/Yawerty.txt
		 tables/ta/Makefile
		 tables/ta/Tamil-inscript.txt
		 tables/ta/Tamil-phonetic.txt
		 tables/ta/Tamil-remington.txt
		 tables/ta/Tamil-tamil99.txt
		 tables/te/Makefile
		 tables/te/Telugu-inscript.txt
		 tables/th/Makefile
		 tables/th/Thai.txt
		 tables/ug/Makefile
		 tables/ug/Uyghur-Romanized.txt
		 tables/ug/Uyghur-Standard.txt
		 tables/uk/Makefile
		 tables/uk/Ukrainian-Translit.txt
		 tables/vi/Makefile
		 tables/vi/Viqr.txt
		 tables/zh/Array30.txt
		 tables/zh/CNS11643.txt
		 tables/zh/CangJie.txt
		 tables/zh/CangJie3.txt
		 tables/zh/CangJie5.txt
		 tables/zh/CantonHK.txt
		 tables/zh/Cantonese.txt
		 tables/zh/Dayi3.txt
		 tables/zh/EZ-Big.txt
		 tables/zh/Erbi-QS.txt
		 tables/zh/Erbi.txt
		 tables/zh/Jyutping.txt
		 tables/zh/Makefile
		 tables/zh/Quick.txt
		 tables/zh/Simplex.txt
		 tables/zh/SmartCangJie6.txt
		 tables/zh/Stroke5.txt
		 tables/zh/Wu.txt
		 tables/zh/Wubi.txt
		 tables/zh/ZhuYin-Big.txt
		 tables/zh/ZhuYin.txt
		 tables/zh/Ziranma.txt
])

if test "$enable_skim_support" = "yes"; then
  REQUIRED_SKIM_VERSION=1.2.1
  dnl skim probably has been installed under prefix=$KDEDIR, so add it to search path of pkgconfig
  kde_libsuffix=`kde-config --libsuffix`
  test -z "$KDEDIR" && KDEDIR=`kde-config --prefix`
  AC_SUBST(KDEDIR)
  export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$KDEDIR/lib${kde_libsuffix}/pkgconfig
  PKG_CHECK_MODULES(SKIM, [skim >= $REQUIRED_SKIM_VERSION],
                          [HAS_SKIM=yes],
                          [HAS_SKIM=no])
else
  HAS_SKIM=no
fi

if test "x$HAS_SKIM" = "xyes"; then
  PKG_CHECK_MODULES(SCIM_KDEUTILS,[scim_kdeutils >= $REQUIRED_SKIM_VERSION],
                                [HAS_KDEUTILS=yes],
                                [HAS_KDEUTILS=no])
  if test "x$HAS_KDEUTILS" = "xyes"; then
      AC_CONFIG_SUBDIRS(skim)
      SCIM_TABLES_ENABLE_SKIM_SUPPORT=1
      enable_skim_support=yes
  else
      SCIM_TABLES_ENABLE_SKIM_SUPPORT=0
      enable_skim_support=no
  fi
else
  SCIM_TABLES_ENABLE_SKIM_SUPPORT=0
  enable_skim_support=no
fi

AM_CONDITIONAL(SCIM_TABLES_ENABLE_SKIM_SUPPORT, [test "$SCIM_TABLES_ENABLE_SKIM_SUPPORT" = "1"])
AC_SUBST(SCIM_TABLES_ENABLE_SKIM_SUPPORT)
AC_OUTPUT

AC_MSG_RESULT([
Build options:
  Version                  $VERSION
  Install prefix           $prefix
  Build shared libs        $enable_shared
  Build static libs        $enable_static
  Enable debug             $enable_debug  

Module options:
  Enable skim support      $enable_skim_support
])
