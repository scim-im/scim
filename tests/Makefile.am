MAINTAINERCLEANFILES 	= Makefile.in

AM_CPPFLAGS	= -I$(top_builddir) \
			  -I$(top_builddir)/src \
	    	  -I$(top_srcdir) \
	    	  -I$(top_srcdir)/intl \
			  -I$(top_srcdir)/src \
			  -DSCIM_DATADIR=\"@SCIM_DATADIR@\" \
			  -DSCIM_LOCALEDIR=\"@SCIM_LOCALEDIR@\" \
			  -DSCIM_SYSCONFDIR=\"@SCIM_SYSCONFDIR@\" \
			  -DSCIM_LIBEXECDIR=\"@SCIM_LIBEXECDIR@\" \
			  -DSCIM_ICONDIR=\"@SCIM_ICONDIR@\" \
			  -DSCIM_MODULE_PATH=\"@SCIM_MODULE_PATH@\"

if SCIM_BUILD_TESTS
CONFIG_TEST_PROGS	= testsocketserver \
			  testsocketclient \
			  testiconvert \
			  testpanel \
			  testlang
CONFIG_TEST_HELPER	= test-helper.la
CONFIG_TEST_IMENGINE	= test-imengine.la
endif

noinst_HEADERS		= scim_test_imengine.h
noinst_PROGRAMS         = $(CONFIG_TEST_PROGS)

testsocketserver_SOURCES  = testsocketserver.cpp
testsocketserver_LDADD    = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

testsocketclient_SOURCES  = testsocketclient.cpp
testsocketclient_LDADD    = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

testiconvert_SOURCES  	  = testiconvert.cpp
testiconvert_LDADD        = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

testpanel_SOURCES  	  = testpanel.cpp
testpanel_LDADD           = $(top_builddir)/src/libscim@SCIM_EPOCH@.la

testlang_SOURCES  	  = testlang.cpp
testlang_LDADD            = $(top_builddir)/src/libscim@SCIM_EPOCH@.la


helpermoduledir		= $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/Helper

helpermodule_LTLIBRARIES= $(CONFIG_TEST_HELPER)
test_helper_la_SOURCES	= scim_test_helper.cpp
test_helper_la_CXXFLAGS	= @GTK_CFLAGS@
test_helper_la_LDFLAGS	= -avoid-version \
		     	  -rpath $(helpermoduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @GTK_LIBS@ \
			  @LTLIBINTL@
test_helper_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

imenginemoduledir	= $(libdir)/scim@SCIM_EPOCH@/$(SCIM_BINARY_VERSION)/IMEngine
imenginemodule_LTLIBRARIES= $(CONFIG_TEST_IMENGINE)
test_imengine_la_SOURCES= scim_test_imengine.cpp
test_imengine_la_LDFLAGS= -avoid-version \
		     	  -rpath $(imenginemoduledir) \
			  -module \
			  @LIBTOOL_EXPORT_OPTIONS@ \
			  @LTLIBINTL@
test_imengine_la_LIBADD	= $(top_builddir)/src/libscim@SCIM_EPOCH@.la

