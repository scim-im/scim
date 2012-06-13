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
 * @brief This header contains all the string constant used in communication between the agent and clients.
 */

#ifndef SCIMBRIDGEMESSAGECONSTANT_H_
#define SCIMBRIDGEMESSAGECONSTANT_H_

#include "scim-bridge.h"

/**
 * The string constant of "set_preedit_mode" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_PREEDIT_MODE[] = "set_preedit_mode";

/**
 * The string constant of "preedit_mode_changed" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_PREEDIT_MODE_CHANGED[] = "preedit_mode_changed";

/**
 * The string constant of "embedded" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_EMBEDDED[] = "embedded";

/**
 * The string constant of "floating" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_FLOATING[] = "floating";

/**
 * The string constant of "hanging" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_HANGING[] = "hanging";

/**
 * The string constant of "any" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_ANY[] = "any";

/**
 * The string constant of "update_preedit" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_UPDATE_PREEDIT[] = "update_preedit";

/**
 * The string constant of "preedit_updated" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_PREEDIT_UPDATED[] = "preedit_updated";

/**
 * The string constant of "set_preedit_string" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_PREEDIT_STRING[] = "set_preedit_string";

/**
 * The string constant of "set_preedit_attributes" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_PREEDIT_ATTRIBUTES[] = "set_preedit_attributes";

/**
 * The string constant of "set_preedit_cursor_position" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_PREEDIT_CURSOR_POSITION[] = "set_preedit_cursor_position";

/**
 * The string constant of "set_preedit_shown" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_PREEDIT_SHOWN[] = "set_preedit_shown";

/**
 * The string constant of "change_focus" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_CHANGE_FOCUS[] = "change_focus";

/**
 * The string constant of "focus_changed" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_FOCUS_CHANGED[] = "focus_changed";

/**
 * The string constant of "handle_key_event" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_HANDLE_KEY_EVENT[] = "handle_key_event";

/**
 * The string constant of "key_event_handled" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_KEY_EVENT_HANDLED[] ="key_event_handled";

/**
 * The string constant "set_cursor_location" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_CURSOR_LOCATION[] = "set_cursor_location";

/**
 * The string constant "cursor_location_changed" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_CURSOR_LOCATION_CHANGED[] = "cursor_location_changed";

/**
 * The string constant of "register_imcontext" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_REGISTER_IMCONTEXT[] = "register_imcontext";

/**
 * The string constant of "imcontext_registered" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_IMCONTEXT_REGISTERED[] = "imcontext_registered";

/**
 * The string constant of "deregister_imcontext" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_DEREGISTER_IMCONTEXT[] = "deregister_imcontext";

/**
 * The string constant of "imcontext_registered" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_IMCONTEXT_DEREGISTERED[] = "imcontext_deregister";

/**
 * The string constant of "reset_imcontext" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_RESET_IMCONTEXT[] = "reset_imcontext";

/**
 * The string constant of "imcontext_reseted" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_IMCONTEXT_RESETED[] = "imcontext_reseted";

/**
 * The string constant of "forward_key_event" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_FORWARD_KEY_EVENT[] = "forward_key_event";

/**
 * The string constant of "set_commit_string" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SET_COMMIT_STRING[] = "set_commit_string";

/**
 * The string constant of "commit_string" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_COMMIT_STRING[] = "commit_string";

/**
 * The string constant of "commit" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_STRING_COMMITED[] = "string_commited";

/**
 * The string constant of "beep" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_BEEP[] = "beep";

/**
 * The string constant of "get_surrounding_text" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_GET_SURROUNDING_TEXT[] = "get_surrounding_text";

/**
 * The string constant of "delete_surrounding_text" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_DELETE_SURROUNDING_TEXT[] = "delete_surrounding_text";

/**
 * The string constant of "replace_surrounding_text" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_REPLACE_SURROUNDING_TEXT[] = "replace_surrounding_text";

/**
 * The string constant of "surrounding_text_gotten" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_GOTTEN[] = "surrounding_text_gotten";

/**
 * The string constant of "surrounding_text_deleted" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_DELETED[] = "surrounding_text_deleted";

/**
 * The string constant of "surrounding_text_replaced" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_REPLACED[] = "surrounding_text_replaced";

/**
 * The string constant of "imengine_status_changed" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_IMENGINE_STATUS_CHANGED[] = "imengine_status_changed";

/**
 * The string constant of "enable_imcontext" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_ENABLE_IMCONTEXT[] = "enable_imcontext";

/**
 * The string constant of "enabled" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_ENABLED[] = "enabled";

/**
 * The string constant of "disable_imcontext" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_DISABLE_IMCONTEXT[] = "disable_imcontext";

/**
 * The string constant of "disabled" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_DISABLED[] = "disabled";

/**
 * The string constant of "shift" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SHIFT[] = "shift";

/**
 * The string constant of "control" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_CONTROL[] = "control";

/**
 * The string constant of "alt" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_ALT[] = "alt";

/**
 * The string constant of "meta" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_META[] = "meta";

/**
 * The string constant of "super" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_SUPER[] = "super";

/**
 * The string constant of "hyper" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_HYPER[] = "hyper";

/**
 * The string constant of "caps_lock" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_CAPS_LOCK[] = "caps_lock";

/**
 * The string constant of "num_lock" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_NUM_LOCK[] = "num_lock";

/**
 * The string constant of "kana_ro" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_KANA_RO[] = "kana_ro";

/**
 * The string constant of "unknown" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_UNKNOWN[] = "unknown";

/**
 * The string constant of "true" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_TRUE[] = "true";

/**
 * The string constant of "false" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_FALSE[] = "false";

/**
 * The string constant of "none" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_NONE[] = "none";

/**
 * The string constant of "decorate" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_DECORATE[] = "decoreate";

/**
 * The string constant of "foreground" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_FOREGROUND[] = "foreground";

/**
 * The string constant of "background" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_BACKGROUND[] = "background";

/**
 * The string constant of "color" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_COLOR[] = "#";

/**
 * The string constant of "underline" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_UNDERLINE[] = "underline";

/**
 * The string constant of "hilight" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_HIGHLIGHT[] = "highlight";

/**
 * The string constant of "reverse" for messages.
 */
static const char SCIM_BRIDGE_MESSAGE_REVERSE[] = "reverse";

#endif                                            /*SCIMBRIDGEMESSAGECONSTANT_H_*/
