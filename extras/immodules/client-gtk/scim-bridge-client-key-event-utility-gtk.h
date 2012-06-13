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
 * @brief This is the header of the functions to translate key events between scim-bridge and gtk.
 */


#ifndef SCIMBRIDGECLIENTKEYEVENTUTILITYGTK_H_
#define SCIMBRIDGECLIENTKEYEVENTUTILITYGTK_H_

#include <gdk/gdk.h>

#include "scim-bridge.h"
#include "scim-bridge-key-event.h"

/**
 * Translate a key event from scim-bridge into gdk.
 *
 * @param gdk_key_event A key event of gdk.
 * @param client_window The gdk window for the key event.
 * @param key_event The key event from scim-bridge.
 */
void scim_bridge_key_event_bridge_to_gdk (GdkEventKey *gdk_key_event, GdkWindow *client_window, const ScimBridgeKeyEvent *key_event);


/**
 * Translate a key event from gdk into scim-bridge.
 *
 * @param bridge_key_event A key event of scim-bridge.
 * @param client_window The gdk window for the key event.
 * @param key_event The key event from gdk.
 */
void scim_bridge_key_event_gdk_to_bridge (ScimBridgeKeyEvent *bridge_key_event, GdkWindow *client_window, const GdkEventKey *key_event);

#endif                                            /*SCIMBRIDGECLIENTKEYEVENTUTILITYGTK_H_*/
