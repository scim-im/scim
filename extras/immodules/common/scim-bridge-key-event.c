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

#include <assert.h>
#include <stdlib.h>

#include "scim-bridge-key-event.h"
#include "scim-bridge-message-constant.h"

typedef unsigned int scim_bridge_key_modifier_t;

#define SCIM_BRIDGE_MODIFIER_MASK_NULL (0)
#define SCIM_BRIDGE_MODIFIER_MASK_SHIFT (1 << 0)
#define SCIM_BRIDGE_MODIFIER_MASK_CAPS_LOCK (1 << 1)
#define SCIM_BRIDGE_MODIFIER_MASK_CONTROL (1 << 2)
#define SCIM_BRIDGE_MODIFIER_MASK_ALT (1 << 3)
#define SCIM_BRIDGE_MODIFIER_MASK_META (1 << 4)
#define SCIM_BRIDGE_MODIFIER_MASK_SUPER (1 << 5)
#define SCIM_BRIDGE_MODIFIER_MASK_HYPER (1 << 6)
#define SCIM_BRIDGE_MODIFIER_MASK_NUM_LOCK (1 << 7)

struct _ScimBridgeKeyEvent
{
    boolean pressed;
    scim_bridge_key_code_t code;
    scim_bridge_key_modifier_t mod_state;
    scim_bridge_key_quirk_t quirks;
};

/* Functions */
ScimBridgeKeyEvent *scim_bridge_alloc_key_event ()
{
    ScimBridgeKeyEvent *key_event = malloc (sizeof (ScimBridgeKeyEvent));
    key_event->pressed = FALSE;
    key_event->mod_state = SCIM_BRIDGE_MODIFIER_MASK_NULL;
    key_event->code = SCIM_BRIDGE_KEY_CODE_NullKey;
    key_event->quirks = SCIM_BRIDGE_KEY_NO_QUIRK;

    return key_event;
}


void scim_bridge_free_key_event (ScimBridgeKeyEvent *key_event)
{
    free (key_event);
}


scim_bridge_key_code_t scim_bridge_key_event_get_code (const ScimBridgeKeyEvent *key_event)
{
    return key_event->code;
}


void scim_bridge_key_event_set_code (ScimBridgeKeyEvent *key_event, scim_bridge_key_code_t key_code)
{
    key_event->code = key_code;
}


boolean scim_bridge_key_event_is_pressed (const ScimBridgeKeyEvent *key_event)
{
    return key_event->pressed;
}


void scim_bridge_key_event_set_pressed (ScimBridgeKeyEvent *key_event, boolean pressed)
{
    key_event->pressed = pressed;
}


void scim_bridge_key_event_clear_modifiers (ScimBridgeKeyEvent *key_event)
{
    key_event->mod_state = SCIM_BRIDGE_MODIFIER_MASK_NULL;
}


boolean scim_bridge_key_event_is_shift_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_SHIFT) != 0;
}


void scim_bridge_key_event_set_shift_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_SHIFT;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_SHIFT;
    }
}


boolean scim_bridge_key_event_is_caps_lock_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_CAPS_LOCK) != 0;
}


void scim_bridge_key_event_set_caps_lock_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_CAPS_LOCK;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_CAPS_LOCK;
    }
}


boolean scim_bridge_key_event_is_control_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_CONTROL) != 0;
}


void scim_bridge_key_event_set_control_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_CONTROL;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_CONTROL;
    }
}


boolean scim_bridge_key_event_is_alt_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_ALT) != 0;
}


void scim_bridge_key_event_set_alt_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_ALT;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_ALT;
    }
}


int scim_bridge_key_event_is_meta_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_META) != 0;
}


void scim_bridge_key_event_set_meta_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_META;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_META;
    }
}


int scim_bridge_key_event_is_super_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_SUPER) != 0;
}


void scim_bridge_key_event_set_super_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_SUPER;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_SUPER;
    }
}


int scim_bridge_key_event_is_hyper_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_HYPER) != 0;
}


void scim_bridge_key_event_set_hyper_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_HYPER;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_HYPER;
    }
}


int scim_bridge_key_event_is_num_lock_down (const ScimBridgeKeyEvent *key_event)
{
    return (key_event->mod_state & SCIM_BRIDGE_MODIFIER_MASK_NUM_LOCK) != 0;
}


void scim_bridge_key_event_set_num_lock_down (ScimBridgeKeyEvent *key_event, boolean down)
{
    if (down) {
        key_event->mod_state |= SCIM_BRIDGE_MODIFIER_MASK_NUM_LOCK;
    } else {
        key_event->mod_state &= ~SCIM_BRIDGE_MODIFIER_MASK_NUM_LOCK;
    }
}


boolean scim_bridge_key_event_is_quirk_enabled (const ScimBridgeKeyEvent *key_event, scim_bridge_key_quirk_t quirk)
{
    return (key_event->quirks & quirk) != 0;
}


void scim_bridge_key_event_set_quirk_enabled (ScimBridgeKeyEvent *key_event, scim_bridge_key_quirk_t quirk, boolean enabled)
{
    if (enabled) {
        key_event->quirks |= quirk;
    } else {
        key_event->quirks &= ~quirk;
    }
}


void scim_bridge_key_event_clear_all_quirks (ScimBridgeKeyEvent *key_event)
{
    key_event->quirks = SCIM_BRIDGE_KEY_NO_QUIRK;
}

