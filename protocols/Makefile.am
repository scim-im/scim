## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Vendored Wayland protocol XMLs (from wlr-protocols; not shipped in
## wayland-protocols) and their wayland-scanner-generated glue, built into a
## convenience library that the Wayland frontend and candidate UI link against.

MAINTAINERCLEANFILES	= Makefile.in

EXTRA_DIST		= input-method-unstable-v2.xml \
			  virtual-keyboard-unstable-v1.xml

if SCIM_BUILD_FRONTEND_WAYLAND

protocol_headers	= input-method-unstable-v2-client-protocol.h \
			  virtual-keyboard-unstable-v1-client-protocol.h

protocol_sources	= input-method-unstable-v2-protocol.c \
			  virtual-keyboard-unstable-v1-protocol.c

BUILT_SOURCES		= $(protocol_headers) $(protocol_sources)
CLEANFILES		= $(protocol_headers) $(protocol_sources)

noinst_LTLIBRARIES	= libscimwlprotocols.la
nodist_libscimwlprotocols_la_SOURCES = $(protocol_sources)
libscimwlprotocols_la_CFLAGS = @WAYLAND_CFLAGS@

input-method-unstable-v2-client-protocol.h: $(srcdir)/input-method-unstable-v2.xml
	$(AM_V_GEN) $(WAYLAND_SCANNER) client-header $< $@

input-method-unstable-v2-protocol.c: $(srcdir)/input-method-unstable-v2.xml
	$(AM_V_GEN) $(WAYLAND_SCANNER) private-code $< $@

virtual-keyboard-unstable-v1-client-protocol.h: $(srcdir)/virtual-keyboard-unstable-v1.xml
	$(AM_V_GEN) $(WAYLAND_SCANNER) client-header $< $@

virtual-keyboard-unstable-v1-protocol.c: $(srcdir)/virtual-keyboard-unstable-v1.xml
	$(AM_V_GEN) $(WAYLAND_SCANNER) private-code $< $@

endif
