INCLUDES = -I$(top_srcdir) -I$(top_srcdir)/src $(all_includes) @SCIM_KDEUTILS_CFLAGS@ 
METASOURCES = AUTO
kde_module_LTLIBRARIES = kcm_skimplugin_scim_tables.la
noinst_HEADERS = scimtablesettingplugin.h

kcm_skimplugin_scim_tables_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_skimplugin_scim_tables_la_SOURCES = generictable.kcfgc generictableui.ui scimtablesettingplugin.cpp
kcm_skimplugin_scim_tables_la_LIBADD = $(LIB_QT) $(LIB_KDEUI) $(LIB_KDECORE) @SCIM_KDEUTILS_LIBS@

kde_kcfg_DATA = generictable.kcfg

kcm_DATA = skimplugin_scim_table_config.desktop

kcmdir = $(kde_servicesdir)/skimconfiguredialog
