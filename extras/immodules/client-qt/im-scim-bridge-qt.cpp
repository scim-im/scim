/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.LGPL included in the package of this file.
 * You can also redistribute it and/or modify it under the terms of 
 * the GNU General Public License as published by the Free Software Foundation and 
 * appearing in the file LICENSE.GPL included in the package of this file.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <cassert>

#include "scim-bridge.h"
#include "scim-bridge-client-common-qt.h"
#include "scim-bridge-client-qt.h"
#include "im-scim-bridge-qt.h"

#if QT_VERSION >= 0x040000
using namespace Qt;
#endif

/* Static Variables */
static ScimBridgeClientQt *client = NULL;

ScimBridgeInputContextPlugin::ScimBridgeInputContextPlugin ()
{
}


ScimBridgeInputContextPlugin::~ScimBridgeInputContextPlugin ()
{
    delete client;
    client = NULL;
}

#if QT_VERSION < 0x050000

/* Implementations */
QStringList ScimBridgeInputContextPlugin::scim_languages;


QStringList ScimBridgeInputContextPlugin::keys () const {
    QStringList identifiers;
    identifiers.push_back (SCIM_BRIDGE_IDENTIFIER_NAME);
    return identifiers;
}


QStringList ScimBridgeInputContextPlugin::languages (const QString &key)
{
    if (scim_languages.empty ()) {
        scim_languages.push_back ("zh_CN");
        scim_languages.push_back ("zh_TW");
        scim_languages.push_back ("zh_HK");
        scim_languages.push_back ("ja");
        scim_languages.push_back ("ko");
    }
    return scim_languages;
}


QString ScimBridgeInputContextPlugin::description (const QString &key)
{
    return QString::fromUtf8 ("Qt immodule plugin for SCIM Bridge");
}


QString ScimBridgeInputContextPlugin::displayName (const QString &key)
{
    return key;
}

#endif

#if QT_VERSION >= 0x050000
QInputContext *ScimBridgeInputContextPlugin::create (const QString &key, const QStringList &paramList)
#else
QInputContext *ScimBridgeInputContextPlugin::create (const QString &key)
#endif
{
#if QT_VERSION >= 0x040000
    if (key.toLower () != SCIM_BRIDGE_IDENTIFIER_NAME) {
#else
    if (key.lower () != SCIM_BRIDGE_IDENTIFIER_NAME) {
#endif
        return NULL;
    } else {
        if (client == NULL) client = new ScimBridgeClientQt ();
        return ScimBridgeClientIMContext::alloc ();
    }
}

#if QT_VERSION < 0x040000
Q_EXPORT_PLUGIN (ScimBridgeInputContextPlugin)
#elif QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2 (ScimBridgeInputContextPlugin, ScimBridgeInputContextPlugin)
#endif
