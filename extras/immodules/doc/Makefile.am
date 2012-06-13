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

if SCIM_BRIDGE_BUILD_DOCUMENTS

api:
	-rm -rf api; cd ..; doxygen doc/doxygen.conf

all: api

clean:
	rm -rf api

endif

MAINTAINERCLEANFILES = Makefile.in api
CLEANFILES           = *.bak
EXTRA_DIST = doxygen.conf index.html document.css LICENSE.LGPL LICENSE.GPL

dist-hook:
	mkdir -p $(distdir)/user
	mkdir -p $(distdir)/developer
	mkdir -p $(distdir)/tests
	cp user/*.html $(distdir)/user
	cp developer/*.html $(distdir)/developer
	cp tests/*.txt $(distdir)/tests
