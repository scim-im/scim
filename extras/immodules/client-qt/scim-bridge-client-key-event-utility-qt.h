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

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This is the header of the functions to translate key events between scim-bridge and qt.
 */

#ifndef SCIMBRIDGECLIENTKEYEVENTUTILITYQT_H_
#define SCIMBRIDGECLIENTKEYEVENTUTILITYQT_H_

#include "scim-bridge.h"
#include "scim-bridge-key-code.h"
#include "scim-bridge-key-event.h"

#include "scim-bridge-client-common-qt.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/Xutil.h>
static const int XKeyPress = KeyPress;
static const int XKeyRelease = KeyRelease;
#undef KeyPress
#undef KeyRelease
#endif

class QKeyEvent;

/**
 * Translate a key event from scim-bridge into qt.
 *
 * @param bridge_key_event The key event from scim-bridge.
 * @return The key event for Qt.
 */
QKeyEvent *scim_bridge_key_event_bridge_to_qt (const ScimBridgeKeyEvent *bridge_key_event);

/**
 * Translate a key event from qt into scim-bridge.
 *
 * @param qt_key_event The key event from Qt.
 * @return The key event from scim-bridge.
 */
ScimBridgeKeyEvent *scim_bridge_key_event_qt_to_bridge (const QKeyEvent *qt_key_event);

#ifdef Q_WS_X11
/**
 * Translate a key event from scim-bridge into X11.
 *
 * @param bridge_key_event The key event from scim-bridge.
 * @param display The X11 display.
 * @param window_id The id for the focused window.
 * @return The key event for X11.
 */
XEvent *scim_bridge_key_event_bridge_to_x11 (const ScimBridgeKeyEvent *bridge_key_event, Display *display, WId window_id);


/**
 * Translate a key event from X11 into scim-bridge.
 *
 * @param x11_event The event from X11.
 * @return The key event from scim-bridge.
 */
ScimBridgeKeyEvent* scim_bridge_key_event_x11_to_bridge (const XEvent *x11_event);
#endif

#endif                                            /*SCIMBRIDGECLIENTKEYEVENTUTILITYQT_H_*/
