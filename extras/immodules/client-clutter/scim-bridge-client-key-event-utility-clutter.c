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

#include <sys/time.h>

#include <clutter/clutter.h>

#include "scim-bridge-client-key-event-utility-clutter.h"
#include "scim-bridge-key-event.h"


/* Implementations */
void scim_bridge_key_event_bridge_to_clutter (ClutterKeyEvent *clutter_key_event, ClutterStage *client_stage, const ScimBridgeKeyEvent *key_event)
{
    clutter_key_event->flags = 0;
    clutter_key_event->source = NULL;
    clutter_key_event->hardware_keycode = 0; /* not needed */
    clutter_key_event->unicode_value = 0;
    clutter_key_event->modifier_state = 0;
    clutter_key_event->device = NULL; /* not needed */

    if (scim_bridge_key_event_is_shift_down (key_event)) clutter_key_event->modifier_state |= CLUTTER_SHIFT_MASK;
    if (scim_bridge_key_event_is_caps_lock_down (key_event)) clutter_key_event->modifier_state |= CLUTTER_LOCK_MASK;
    if (scim_bridge_key_event_is_control_down (key_event)) clutter_key_event->modifier_state |= CLUTTER_CONTROL_MASK;
    if (scim_bridge_key_event_is_alt_down (key_event)) clutter_key_event->modifier_state |= CLUTTER_MOD1_MASK;
    /*if (scim_bridge_key_event_is_num_lock_down (key_event)) clutter_key_event->modifier_state |= CLUTTER_MOD2_MASK;*/

    if (scim_bridge_key_event_is_pressed (key_event)) {
        clutter_key_event->type = CLUTTER_KEY_PRESS;
    } else {
        clutter_key_event->type = CLUTTER_KEY_RELEASE;
        clutter_key_event->modifier_state |= CLUTTER_RELEASE_MASK;
    }

    clutter_key_event->stage = client_stage;

    struct timeval current_time;
    gettimeofday (&current_time, NULL);

    clutter_key_event->time = current_time.tv_sec * 1000 + current_time.tv_usec / 1000;
    clutter_key_event->keyval = scim_bridge_key_event_get_code (key_event);

}


void scim_bridge_key_event_clutter_to_bridge (ScimBridgeKeyEvent *bridge_key_event, ClutterStage *stage, const ClutterKeyEvent *key_event)
{
    // Use Key Symbole provided by clutter.
    scim_bridge_key_event_set_code (bridge_key_event, (scim_bridge_key_code_t) key_event->keyval);

    scim_bridge_key_event_clear_modifiers (bridge_key_event);
    if (key_event->modifier_state & CLUTTER_SHIFT_MASK || key_event->keyval == CLUTTER_Shift_L || key_event->keyval == CLUTTER_Shift_R) scim_bridge_key_event_set_shift_down (bridge_key_event, TRUE);
    if (key_event->modifier_state & CLUTTER_LOCK_MASK || key_event->keyval == CLUTTER_Caps_Lock) scim_bridge_key_event_set_caps_lock_down (bridge_key_event, TRUE);
    if (key_event->modifier_state & CLUTTER_CONTROL_MASK || key_event->keyval == CLUTTER_Control_L || key_event->keyval == CLUTTER_Control_R) scim_bridge_key_event_set_control_down (bridge_key_event, TRUE);
    if (key_event->modifier_state & CLUTTER_MOD1_MASK || key_event->keyval == CLUTTER_Alt_L || key_event->keyval == CLUTTER_Alt_R) scim_bridge_key_event_set_alt_down (bridge_key_event, TRUE);
    /*if (key_event->modifier_state & CLUTTER_MOD2_MASK) scim_bridge_key_event_set_num_lock_down (bridge_key_event, TRUE);*/

    if (key_event->type != CLUTTER_KEY_RELEASE) {
        scim_bridge_key_event_set_pressed (bridge_key_event, TRUE);
    } else {
        scim_bridge_key_event_set_pressed (bridge_key_event, FALSE);
    }
}
