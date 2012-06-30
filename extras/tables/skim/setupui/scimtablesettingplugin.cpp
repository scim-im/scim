/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "scimtablesettingplugin.h"
#include "generictable.h"
#include "generictableui.h"

#include <kgenericfactory.h>
#include <klocale.h>

typedef KGenericFactory<ScimTableSettingPlugin> ScimTableSettingPluginLoaderFactory;

K_EXPORT_COMPONENT_FACTORY( kcm_skimplugin_scim_tables, 
    ScimTableSettingPluginLoaderFactory( "kcm_skimplugin_scim_tables" ) )

class ScimTableSettingPlugin::ScimTableSettingPluginPrivate {
public:
    GenericTableSettingsUI * ui;
};

ScimTableSettingPlugin::ScimTableSettingPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args)
 : KAutoCModule( ScimTableSettingPluginLoaderFactory::instance(), 
     parent, args, GenericTableConfig::self() ),
   d(new ScimTableSettingPluginPrivate)
{
    KGlobal::locale()->insertCatalogue("skim-scim-tables");
    d->ui = new GenericTableSettingsUI(this);
    setMainWidget(d->ui);
}


ScimTableSettingPlugin::~ScimTableSettingPlugin()
{
    KGlobal::locale()->removeCatalogue("skim-scim-tables");
}


#include "scimtablesettingplugin.moc"
