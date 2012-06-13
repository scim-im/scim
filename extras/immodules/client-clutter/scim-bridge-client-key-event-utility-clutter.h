/*
 * SCIM Bridge
 *
 * Copyright (c) 2006 Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * Copyright (C) 2009, Intel Corporation.
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
 * @author Raymond liu <raymond.liu@intel.com>
 * @brief This is the header of the functions to translate key events between scim-bridge and clutter.
 */


#ifndef SCIMBRIDGECLIENTKEYEVENTUTILITYCLUTTER_H_
#define SCIMBRIDGECLIENTKEYEVENTUTILITYCLUTTER_H_

#include <clutter/clutter.h>

#include "scim-bridge.h"
#include "scim-bridge-key-event.h"

/**
 * Translate a key event from scim-bridge into clutter.
 *
 * @param clutter_key_event A key event of clutter.
 * @param client_stage The clutter stage for the key event.
 * @param key_event The key event from scim-bridge.
 */
void scim_bridge_key_event_bridge_to_clutter (ClutterKeyEvent *clutter_key_event, ClutterStage *client_stage, const ScimBridgeKeyEvent *key_event);


/**
 * Translate a key event from clutter into scim-bridge.
 *
 * @param bridge_key_event A key event of scim-bridge.
 * @param client_stage The clutter stage for the key event.
 * @param key_event The key event from clutter.
 */
void scim_bridge_key_event_clutter_to_bridge (ScimBridgeKeyEvent *bridge_key_event, ClutterStage *client_stage, const ClutterKeyEvent *key_event);

#endif                                            /*SCIMBRIDGECLIENTKEYEVENTUTILITYCLUTTER_H_*/
