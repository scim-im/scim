/** @file scimqtinputcontext.h
 *  @brief native SCIM input-method module for Qt5 (QPlatformInputContext).
 *
 *  Talks to SCIM through the native client path (CommonBackEnd + PanelClient
 *  over SCIM's own socket protocol), not the scim-bridge protocol.
 */

/*
 * Smart Common Input Method
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */

#pragma once

#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/qpa/qplatforminputcontext.h>
#include <QtGui/qpa/qplatforminputcontextplugin_p.h>

class QEvent;
class QInputMethodEvent;

/* One shared platform input context follows the focused object. The SCIM
 * state (IMEngine instance, preedit, ids) lives in an opaque Impl defined in
 * the .cpp so that moc only ever parses Qt types here. */
class ScimQtInputContext : public QPlatformInputContext
{
    Q_OBJECT

public:
    ScimQtInputContext ();
    ~ScimQtInputContext () override;

    bool isValid () const override;

    void setFocusObject (QObject *object) override;
    void reset () override;
    void commit () override;
    void update (Qt::InputMethodQueries queries) override;
    bool filterEvent (const QEvent *event) override;
    void invokeAction (QInputMethod::Action action, int cursorPosition) override;

    /* Deliver a QInputMethodEvent to the currently focused object. */
    void sendEvent (QInputMethodEvent &event);

    struct Impl;
    Impl *impl;
};

/* The QPlatformInputContext plugin entry point. */
class ScimQtInputContextPlugin : public QPlatformInputContextPlugin
{
    Q_OBJECT
    // Must match QPlatformInputContextFactoryInterface_iid exactly, version suffix
    // included -- it is still ".5.1" in Qt6. Spelled without one, this declares an
    // interface Qt does not recognise and the plugin is passed over in silence: the
    // compose plugin loads instead and no key ever reaches SCIM. Written out rather
    // than using the macro because moc needs a literal here.
    Q_PLUGIN_METADATA (IID "org.qt-project.Qt.QPlatformInputContextFactoryInterface.5.1" FILE "scim.json")

public:
    QPlatformInputContext *create (const QString &key,
                                   const QStringList &paramList) override;
};
