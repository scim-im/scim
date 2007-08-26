/**
 * @file scim_bridge_key_event.h
 * @brief Stab for key event interfaces.
 */

/* 
 * Smart Common Input Method
 * 
 * Copyright (c) 2002-2005 James Su <suzhe@tsinghua.org.cn>
 *
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */
#ifndef SCIM_BRIDGE_KEY_EVENT_H_
#define SCIM_BRIDGE_KEY_EVENT_H_

 
/**
 * This key mask indicates that no modifier key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_NullMask = 0;

/**
 * This key mask indicates that shift key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_ShiftMask = (1 << 0);

/**
 * This key mask indicates that caps lock key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_CapsLockMask = (1 << 1);

/**
 * This key mask indicates that control key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_ControlMask = (1 << 2);

/**
 * This key mask indicates that alt key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_AltMask = (1<<3);

/**
 * This key mask indicates that caps lock key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_MetaMask = (1<<4);

/**
 * This key mask indicates that super key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_SuperMask = (1<<5);

/**
 * This key mask indicates that hyper key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_HyperMask = (1<<6);

/**
 * This key mask indicates that num lock key is pressed as a modifier.
 */
static const uint16 SCIM_KEY_NumLockMask = (1<<7);

/**
 * This key mask indicates that another backslash on jp106 keyboard is pressed.
 */
static const uint16 SCIM_KEY_QuirkKanaRoMask = (1<<14);

/**
 * This key mask indicates that the key is released.
 */
static const uint16 SCIM_KEY_ReleaseMask = (1<<15);

/**
 * The value for all the valid modifier masks.
 */
static const uint16 SCIM_KEY_AllMasks = 0xC0FF;


/**
 * @brief This struct is used to pack information of key events.
 */
typedef struct _ScimKeyEvent 
{
    uint32 code;
    uint16 modifiers;
    uint16 layout;
} ScimKeyEvent;

/**
 * @brief Check if this is a key releasing event.
 *
 * @param key_event The key event.
 * @return True if the key is released.
 */
bool scim_key_event_is_released (const ScimKeyEvent *key_event);

/**
 * @brief Check if this is a key pressing event.
 *
 * @param key_event The key event.
 * @return True if the key is pressed.
 */
bool scim_key_event_is_pressed (const ScimKeyEvent *key_event);

#endif /*SCIM_BRIDGE_KEY_EVENT_H_*/
