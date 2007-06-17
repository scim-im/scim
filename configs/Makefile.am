MAINTAINERCLEANFILES 	= Makefile.in

CLEANFILES  = *.bak

if SCIM_BUILD_CONFIG_SIMPLE
configdir   = $(sysconfdir)/scim
config_DATA = config global
config_source_data = $(foreach file, $(config_DATA), $(srcdir)/$(file))
endif

EXTRA_DIST  = $(config_DATA)
