## Makefile.am -- Process this file with automake to produce Makefile.in
##
## Copyright (C) 2006 Ryo Dairiki
##
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License as published by the Free Software Foundation and 
## appearing in the file LICENSE.LGPL included in the package of this file.
## You can also redistribute it and/or modify it under the terms of 
## the GNU General Public License as published by the Free Software Foundation and 
## appearing in the file LICENSE.GPL included in the package of this file.
##
## This library is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

if SCIM_BRIDGE_BUILD_TESTS

do_subst = sed -e 's,[@]top_builddir[@],@top_builddir@,g'

if SCIM_BRIDGE_BUILD_TESTS
noinst_SCRIPTS = test-exec.sh gtk.immodules

test-exec.sh: test-exec.sh.in
	$(do_subst) < $^ > $@; chmod a+x $@

gtk.immodules: gtk.immodules.in
	$(do_subst) < $^ > $@; chmod a+x $@
endif

endif

MAINTAINERCLEANFILES = Makefile.in

EXTRA_DIST = test-exec.sh.in gtk.immodules.in
