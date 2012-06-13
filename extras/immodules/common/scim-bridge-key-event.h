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
 * @brief This header describes about key events.
 */
#ifndef SCIM_BRIDGE_KEY_EVENT_H_
#define SCIM_BRIDGE_KEY_EVENT_H_

#include "scim-bridge.h"
#include "scim-bridge-key-code.h"

/**
 * The type for quirks of key events.
 */
typedef unsigned int scim_bridge_key_quirk_t;

/**
 * There is no quirk on this key event.
 */
#define SCIM_BRIDGE_KEY_NO_QUIRK (0)

/**
 * All quirks are set on this key event.
 */
#define SCIM_BRIDGE_KEY_ALL_QUIRKS (0xF)

/**
 * This key is not simple backslash but Japanese kana_ro.
 */
#define SCIM_BRIDGE_KEY_QUIRK_KANA_RO (1 << 1)

/**
 * This is the data type of KeyEvent.
 */
typedef struct _ScimBridgeKeyEvent ScimBridgeKeyEvent;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Allocate a new key event.
     *
     * @return The new key event.
     */
    ScimBridgeKeyEvent *scim_bridge_alloc_key_event ();

    /**
     * Free a key event.
     *
     * @param key_event The key event to free.
     * @note Do not use free (). It will cause memleaks.
     */
    void scim_bridge_free_key_event (ScimBridgeKeyEvent *key_event);

    /**
     * Get the key code of an event.
     *
     * @param key_event The key event.
     * @return The key code of the event.
     */
    scim_bridge_key_code_t scim_bridge_key_event_get_code (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the key code of an event.
     *
     * @param key_event The key event.
     * @param key_code The key code.
     */
    void scim_bridge_key_event_set_code (ScimBridgeKeyEvent *key_event, scim_bridge_key_code_t key_code);

    /**
     * Get the key state of an event.
     *
     * @param key_event The key event.
     * @return TRUE if pressed, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_pressed (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the key state of an event.
     *
     * @param key_event The key event.
     * @param pressed Give TRUE if pressed, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_pressed (ScimBridgeKeyEvent *key_event, boolean pressed);


    /**
     * Clear all the modifiers of a key event.
     *
     * @param key_event The key event.
     */
    void scim_bridge_key_event_clear_modifiers (ScimBridgeKeyEvent *key_event);


    /**
     * Get the state of the shift key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the shift key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_shift_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the shift key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the shift key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_shift_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the state of the caps lock key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the caps lock key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_caps_lock_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the caps lock key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the caps lock key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_caps_lock_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the control state of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the control key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_control_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the control key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the control is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_control_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the state of the alt key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the alt key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_alt_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the alt key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the alt key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_alt_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the state of the meta key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the meta key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_meta_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the meta key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the meta key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_meta_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the state of the super key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the super key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_super_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the super key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the super key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_super_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the state of the hyper key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the hyper key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_hyper_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the huper key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the hyper key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_hyper_down (ScimBridgeKeyEvent *key_event, boolean down);

    /**
     * Get the state of the num lock key of an event.
     *
     * @param key_event The key event.
     * @return TRUE when the num lock key is down, otherwise it returns FALSE.
     */
    boolean scim_bridge_key_event_is_num_lock_down (const ScimBridgeKeyEvent *key_event);

    /**
     * Set the state of the num lock key of an event.
     *
     * @param key_event The key event.
     * @param down Give TRUE when the num lock key is down, otherwise give FALSE.
     */
    void scim_bridge_key_event_set_num_lock_down (ScimBridgeKeyEvent *key_event, boolean down);
    
    /**
     * Check if the given quirk is enabled on this key event.
     *
     * @param key_event The key event.
     * @param quirk The quirk to check.
     */
    boolean scim_bridge_key_event_is_quirk_enabled (const ScimBridgeKeyEvent *key_event, scim_bridge_key_quirk_t quirk);

    /**
     * Set the sate of the given quirk of this key event.
     *
     * @param key_event The key event.
     * @param quirk The quirk to change the state.
     * @param enabled The new state.
     */
    void scim_bridge_key_event_set_quirk_enabled (ScimBridgeKeyEvent *key_event, scim_bridge_key_quirk_t quirk, boolean enabled);
    
    /**
     * Clear all the quirks of this key event.
     *
     * @param key_event The key event.
     */
    void scim_bridge_key_event_clear_all_quirks (ScimBridgeKeyEvent *key_event);
    
#ifdef __cplusplus
}
#endif
#endif                                            /*SCIM_BRIDGE_KEY_EVENT_H_*/
