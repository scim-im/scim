/***************************************************************************
 *   Copyright (C) 2003-2005 by liuspider                                  *
 *   liuspider@users.sourceforge.net                                       *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 **************************************************************************/
#ifndef SCIMTABLESETTINGPLUGIN_H
#define SCIMTABLESETTINGPLUGIN_H

#include "utils/kautocmodule.h"

class ScimTableSettingPlugin : public KAutoCModule
{
Q_OBJECT
public:
    ScimTableSettingPlugin(QWidget *parent, 
  const char */*name*/, const QStringList &args);
    virtual ~ScimTableSettingPlugin();
//     virtual void save();
//     virtual void defaults();
//     virtual void load();
private:
    class ScimTableSettingPluginPrivate;
    ScimTableSettingPluginPrivate * d;
};

#endif
