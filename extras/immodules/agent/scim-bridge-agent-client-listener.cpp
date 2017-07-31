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
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <list>

#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_EVENT
#include <scim.h>

#include "scim-bridge-imcontext.h"
#include "scim-bridge-message-constant.h"
#include "scim-bridge-messenger.h"
#include "scim-bridge-string.h"
#include "scim-bridge-output.h"

#include "scim-bridge-agent-client-listener.h"
#include "scim-bridge-agent-protected.h"

using std::list;

using namespace scim;

/* Static Constants */
static const struct timeval TIMEOUT_IMMEDIATELY = {0, 0};

/* Class Definition */
class ScimBridgeAgentClientListenerImpl: public ScimBridgeAgentClientListener
{

    public:

        ScimBridgeAgentClientListenerImpl (int new_socket_fd, ScimBridgeAgentProtected *new_agent);

        ~ScimBridgeAgentClientListenerImpl ();

        int get_socket_fd () const;

        scim_bridge_agent_event_type_t get_trigger_events () const;

        bool handle_event (scim_bridge_agent_event_type_t event_type);

        retval_t imengine_status_changed (scim_bridge_imcontext_id_t imcontext_id, bool enabled);
        retval_t set_preedit_shown (scim_bridge_imcontext_id_t imcontext_id, bool shown);
        retval_t set_preedit_cursor_position (scim_bridge_imcontext_id_t imcontext_id, int cursor_position);
        retval_t set_preedit_string (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstring);
        retval_t set_preedit_attributes (scim_bridge_imcontext_id_t imcontext_id, const AttributeList &attributes);
        retval_t update_preedit (scim_bridge_imcontext_id_t imcontext_id);
        retval_t commit_string (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstring);
        retval_t beep (scim_bridge_imcontext_id_t imcontext_id);
        retval_t forward_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event);
        retval_t get_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, int before_max, int after_max, WideString &wstring, int &cursor_position);
        retval_t delete_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, int offset, int length);
        retval_t replace_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstring, int cursor_position);

    private:

        ScimBridgeAgentProtected *agent;

        ScimBridgeMessenger *messenger;

        list<ScimBridgeMessage*> received_messages;

        int get_surrounding_timeout_count;
        int delete_surrounding_timeout_count;
        int replace_surrounding_timeout_count;

        void push_message (ScimBridgeMessage *message);
        ScimBridgeMessage *poll_message ();
        retval_t process_message (const ScimBridgeMessage *message);

        retval_t register_imcontext ();
        retval_t deregister_imcontext (scim_bridge_imcontext_id_t imcontext_id);
        retval_t reset_imcontext (scim_bridge_imcontext_id_t imcontext_id);
        retval_t enable_imcontext (scim_bridge_imcontext_id_t imcontext_id);
        retval_t disable_imcontext (scim_bridge_imcontext_id_t imcontext_id);
        retval_t change_focus (scim_bridge_imcontext_id_t imcontext_id, bool focus_in);
        retval_t handle_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event);
        retval_t set_cursor_location (scim_bridge_imcontext_id_t imcontext_id, int cursor_x, int cursor_y);
        retval_t set_preedit_mode (scim_bridge_imcontext_id_t imcontext_id, scim_bridge_preedit_mode_t preedit_mode);

};

/* Implementations */
ScimBridgeAgentClientListener *ScimBridgeAgentClientListener::alloc (int new_socket_fd, ScimBridgeAgentProtected *new_agent)
{
    return new ScimBridgeAgentClientListenerImpl (new_socket_fd, new_agent);
}


ScimBridgeAgentClientListenerImpl::ScimBridgeAgentClientListenerImpl (int socket_fd, ScimBridgeAgentProtected *new_agent):
agent (new_agent), messenger (NULL), get_surrounding_timeout_count (0), delete_surrounding_timeout_count (0), replace_surrounding_timeout_count (0)
{
    messenger = scim_bridge_alloc_messenger (socket_fd);
}


ScimBridgeAgentClientListenerImpl::~ScimBridgeAgentClientListenerImpl ()
{
    scim_bridge_free_messenger (messenger);

    for (list<ScimBridgeMessage*>::iterator i = received_messages.begin (); i != received_messages.end (); ++i) {
        ScimBridgeMessage *message = *i;
        scim_bridge_free_message (message);
    }
    received_messages.clear ();

    agent->remove_client_listener (this);
}


int ScimBridgeAgentClientListenerImpl::get_socket_fd () const
{
    return scim_bridge_messenger_get_socket_fd (messenger);
}


scim_bridge_agent_event_type_t ScimBridgeAgentClientListenerImpl::get_trigger_events () const
{
    scim_bridge_agent_event_type_t triggers = SCIM_BRIDGE_AGENT_EVENT_READ | SCIM_BRIDGE_AGENT_EVENT_ERROR;
    if (received_messages.size () > 0) triggers |= SCIM_BRIDGE_AGENT_EVENT_INTERRUPT;
    if (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) triggers |= SCIM_BRIDGE_AGENT_EVENT_WRITE;

    return triggers;
}


bool ScimBridgeAgentClientListenerImpl::handle_event (scim_bridge_agent_event_type_t event_type)
{
    scim_bridge_pdebugln (2, "handle_event ()");

    if (event_type & SCIM_BRIDGE_AGENT_EVENT_ERROR) {
        return false;
    }

    if ((event_type & SCIM_BRIDGE_AGENT_EVENT_WRITE) && scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        if (scim_bridge_messenger_send_message (messenger, &TIMEOUT_IMMEDIATELY)) {
            scim_bridge_pdebugln (5, "The connection with the client is closed on errors");
            return false;
        }
    }

    if (event_type & SCIM_BRIDGE_AGENT_EVENT_INTERRUPT) {
        ScimBridgeMessage *message = NULL;
        while (true) {
            ScimBridgeMessage *message = poll_message ();
            if (message == NULL) break;
            if (process_message (message) == RETVAL_FAILED) {
                scim_bridge_pdebugln (5, "The connection with the client is closed on errors");
                scim_bridge_free_message (message);
                return false;
            }
            scim_bridge_free_message (message);
        }
    }

    if (event_type & SCIM_BRIDGE_AGENT_EVENT_READ) {
        if (scim_bridge_messenger_receive_message (messenger, &TIMEOUT_IMMEDIATELY)) {
            scim_bridge_pdebugln (5, "The connection with the client is closed on errors");
            return false;
        }
        while (scim_bridge_messenger_get_receiving_buffer_size (messenger) > 0) {
            ScimBridgeMessage *message;
            if (scim_bridge_messenger_poll_message (messenger, &message)) {
                break;
            } else {
                push_message (message);
                agent->interrupt ();
            }
        }
    }

    return true;
}


ScimBridgeMessage *ScimBridgeAgentClientListenerImpl::poll_message ()
{
    if (received_messages.size () > 0) {
        ScimBridgeMessage *message = received_messages.front ();
        received_messages.pop_front ();
        return message;
    } else {
        return NULL;
    }
}


void ScimBridgeAgentClientListenerImpl::push_message (ScimBridgeMessage *message)
{
    received_messages.push_back (message);
}


retval_t ScimBridgeAgentClientListenerImpl::process_message (const ScimBridgeMessage *message)
{
    scim_bridge_pdebugln (6, "process_message ()");

    const char *message_header = scim_bridge_message_get_header (message);
    if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_REGISTER_IMCONTEXT) == 0) {

        return register_imcontext ();

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_DEREGISTER_IMCONTEXT) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);

        unsigned int imcontext_id;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        return deregister_imcontext (imcontext_id);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_RESET_IMCONTEXT) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);

        unsigned int imcontext_id;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        return reset_imcontext (imcontext_id);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_ENABLE_IMCONTEXT) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);

        unsigned int imcontext_id;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        return enable_imcontext (imcontext_id);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_DISABLE_IMCONTEXT) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);

        unsigned int imcontext_id;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        return disable_imcontext (imcontext_id);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_CHANGE_FOCUS) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);
        const char *focus_in_str = scim_bridge_message_get_argument (message, 1);

        unsigned int imcontext_id;
        boolean focus_in;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str) || scim_bridge_string_to_boolean (&focus_in, focus_in_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        return change_focus (imcontext_id, focus_in);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_HANDLE_KEY_EVENT) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);
        const char *key_code_str = scim_bridge_message_get_argument (message, 1);
        const char *key_pressed_str = scim_bridge_message_get_argument (message, 2);

        unsigned int imcontext_id;
        unsigned int key_code;
        boolean key_pressed;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str) || scim_bridge_string_to_uint (&key_code, key_code_str)
            || scim_bridge_string_to_boolean (&key_pressed, key_pressed_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        unsigned int key_modifiers;
        if (key_pressed) {
            key_modifiers = SCIM_KEY_NullMask;
        } else {
            key_modifiers = SCIM_KEY_ReleaseMask;
        }

        for (size_t j = 3; j < scim_bridge_message_get_argument_count (message); ++j) {
            const char *modifier_str = scim_bridge_message_get_argument (message, j);

            if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_SHIFT) == 0) {
                key_modifiers |= SCIM_KEY_ShiftMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_CONTROL) == 0) {
                key_modifiers |= SCIM_KEY_ControlMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_ALT) == 0) {
                key_modifiers |= SCIM_KEY_AltMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_META) == 0) {
                key_modifiers |= SCIM_KEY_MetaMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_SUPER) == 0) {
                key_modifiers |= SCIM_KEY_SuperMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_HYPER) == 0) {
                key_modifiers |= SCIM_KEY_HyperMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_CAPS_LOCK) == 0) {
                key_modifiers |= SCIM_KEY_CapsLockMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_NUM_LOCK) == 0) {
                key_modifiers |= SCIM_KEY_NumLockMask;
            } else if (strcmp (modifier_str, SCIM_BRIDGE_MESSAGE_KANA_RO) == 0) {
                key_modifiers |= SCIM_KEY_QuirkKanaRoMask;
            } else {
                scim_bridge_perrorln ("Unknown modifier: %s", modifier_str);
            }
        }

        KeyEvent key_event (key_code, key_modifiers, 0);
        return handle_key_event (imcontext_id, key_event);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_CURSOR_LOCATION) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);
        const char *cursor_x_str = scim_bridge_message_get_argument (message, 1);
        const char *cursor_y_str = scim_bridge_message_get_argument (message, 2);

        unsigned int imcontext_id;
        int cursor_x;
        int cursor_y;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str) || scim_bridge_string_to_int (&cursor_x, cursor_x_str)
            || scim_bridge_string_to_int (&cursor_y, cursor_y_str)) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        return set_cursor_location (imcontext_id, cursor_x, cursor_y);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_GOTTEN) == 0) {

        if (get_surrounding_timeout_count > 0) --get_surrounding_timeout_count;

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_DELETED) == 0) {

        if (delete_surrounding_timeout_count > 0) --delete_surrounding_timeout_count;

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_REPLACED) == 0) {

        if (replace_surrounding_timeout_count > 0) --replace_surrounding_timeout_count;

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SET_PREEDIT_MODE) == 0) {

        const char *imcontext_id_str = scim_bridge_message_get_argument (message, 0);
        const char *preedit_mode_str = scim_bridge_message_get_argument (message, 1);

        unsigned int imcontext_id;
        scim_bridge_preedit_mode_t preedit_mode;
        if (scim_bridge_string_to_uint (&imcontext_id, imcontext_id_str) || preedit_mode_str == NULL) {
            scim_bridge_perrorln ("Invalid message: Close the connection.");
            return RETVAL_FAILED;
        }

        if (strcmp (preedit_mode_str, SCIM_BRIDGE_MESSAGE_FLOATING) == 0) {
            preedit_mode = PREEDIT_FLOATING;
        } else if (strcmp (preedit_mode_str, SCIM_BRIDGE_MESSAGE_HANGING) == 0) {
            preedit_mode = PREEDIT_HANGING;
        } else if (strcmp (preedit_mode_str, SCIM_BRIDGE_MESSAGE_EMBEDDED) == 0) {
            preedit_mode = PREEDIT_EMBEDDED;
        } else if (strcmp (preedit_mode_str, SCIM_BRIDGE_MESSAGE_ANY) == 0) {
            preedit_mode = PREEDIT_ANY;
        } else {
            scim_bridge_perrorln ("Unknown preedit mode: %s", preedit_mode_str);
            return RETVAL_FAILED;
        }

        return set_preedit_mode (imcontext_id, preedit_mode);

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_PREEDIT_UPDATED) == 0) {

        scim_bridge_pdebugln (1, "Unused message: Just ignore it");

    } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_STRING_COMMITED) == 0) {

        scim_bridge_pdebugln (1, "Unused message: Just ignore it");

    } else {

        scim_bridge_perrorln ("Unknown message: %s", message_header);
        return RETVAL_FAILED;

    }
    return 0;
}


/* Event Handlers */
retval_t ScimBridgeAgentClientListenerImpl::register_imcontext ()
{
    scim_bridge_pdebugln (6, "register_imcontext ()");

    const scim_bridge_imcontext_id_t imcontext_id = agent->alloc_imcontext (this);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_IMCONTEXT_REGISTERED, 1);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::deregister_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "deregister_imcontext ()");

    agent->free_imcontext (imcontext_id, this);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_IMCONTEXT_DEREGISTERED, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::reset_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "reset_imcontext ()");

    agent->reset_imcontext (imcontext_id);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_IMCONTEXT_RESETED, 1);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::enable_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "enable_imcontext ()");

    agent->enable_imcontext (imcontext_id);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_ENABLED, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::disable_imcontext (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "disable_imcontext ()");

    agent->disable_imcontext (imcontext_id);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_DISABLED, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::change_focus (scim_bridge_imcontext_id_t imcontext_id, bool focus_in)
{
    scim_bridge_pdebugln (6, "change_focus ()");

    agent->change_focus (imcontext_id, focus_in);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_FOCUS_CHANGED, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::set_cursor_location (scim_bridge_imcontext_id_t imcontext_id, int cursor_x, int cursor_y)
{
    scim_bridge_pdebugln (6, "set_cursor_location ()");

    agent->set_cursor_location (imcontext_id, cursor_x, cursor_y);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_CURSOR_LOCATION_CHANGED, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::handle_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event)
{
    scim_bridge_pdebugln (6, "handle_key_event ()");

    const bool consumed = agent->filter_key_event (imcontext_id, key_event);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_KEY_EVENT_HANDLED, 1);

    const char *consumed_str = consumed ? SCIM_BRIDGE_MESSAGE_TRUE:SCIM_BRIDGE_MESSAGE_FALSE;
    scim_bridge_message_set_argument (message, 0, consumed_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::set_preedit_mode (scim_bridge_imcontext_id_t imcontext_id, scim_bridge_preedit_mode_t preedit_mode)
{
    scim_bridge_pdebugln (6, "set_preedit_mode ()");

    agent->set_preedit_mode (imcontext_id, preedit_mode);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_PREEDIT_MODE_CHANGED, 0);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


/** Public Funtions **/
retval_t ScimBridgeAgentClientListenerImpl::imengine_status_changed (scim_bridge_imcontext_id_t imcontext_id, bool enabled)
{
    scim_bridge_pdebugln (6, "imengine_status_changed ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_IMENGINE_STATUS_CHANGED, 2);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);
    
    char *enabled_str;
    scim_bridge_string_from_boolean (&enabled_str, enabled);
    scim_bridge_message_set_argument (message, 1, enabled_str);
    free (enabled_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}

retval_t ScimBridgeAgentClientListenerImpl::set_preedit_shown (scim_bridge_imcontext_id_t imcontext_id, bool shown)
{
    scim_bridge_pdebugln (6, "set_preedit_shown ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_PREEDIT_SHOWN, 2);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    char *shown_str;
    scim_bridge_string_from_boolean (&shown_str, shown);
    scim_bridge_message_set_argument (message, 1, shown_str);
    free (shown_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::set_preedit_cursor_position (scim_bridge_imcontext_id_t imcontext_id, int cursor_position)
{
    scim_bridge_pdebugln (6, "set_preedit_cursor_position ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_PREEDIT_CURSOR_POSITION, 2);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    char *cursor_position_str;
    scim_bridge_string_from_uint (&cursor_position_str, cursor_position);
    scim_bridge_message_set_argument (message, 1, cursor_position_str);
    free (cursor_position_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::set_preedit_string (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstring)
{
    scim_bridge_pdebugln (6, "set_preedit_string ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_PREEDIT_STRING, 2);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    char *preedit_str;
    scim_bridge_wstring_to_string (&preedit_str, wstring.c_str ());
    scim_bridge_message_set_argument (message, 1, preedit_str);
    free (preedit_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::set_preedit_attributes (scim_bridge_imcontext_id_t imcontext_id, const AttributeList &attributes)
{
    scim_bridge_pdebugln (6, "set_preedit_attributes ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_PREEDIT_ATTRIBUTES, attributes.size () * 4 + 1);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    int arg_index = 1;
    for (AttributeList::const_iterator i = attributes.begin (); i != attributes.end (); ++i) {
        const Attribute &attribute = *i;

        char *begin_str;
        char *end_str;
        scim_bridge_string_from_uint (&begin_str, attribute.get_start ());
        scim_bridge_string_from_uint (&end_str, attribute.get_end ());

        const char *type_str;
        const char *value_str;

        if (attribute.get_type () == SCIM_ATTR_DECORATE) {
            type_str = SCIM_BRIDGE_MESSAGE_DECORATE;
            switch (attribute.get_value ()) {
                case SCIM_ATTR_DECORATE_UNDERLINE:
                    value_str = SCIM_BRIDGE_MESSAGE_UNDERLINE;
                    break;
                case SCIM_ATTR_DECORATE_REVERSE:
                    value_str = SCIM_BRIDGE_MESSAGE_REVERSE;
                    break;
                case SCIM_ATTR_DECORATE_HIGHLIGHT:
                    value_str = SCIM_BRIDGE_MESSAGE_HIGHLIGHT;
                    break;
                default:
                    type_str = SCIM_BRIDGE_MESSAGE_NONE;
                    value_str = SCIM_BRIDGE_MESSAGE_NONE;
            }
        } else if (attribute.get_type () == SCIM_ATTR_FOREGROUND || attribute.get_type () == SCIM_ATTR_BACKGROUND) {
            if (attribute.get_type () == SCIM_ATTR_FOREGROUND) {
                type_str = SCIM_BRIDGE_MESSAGE_FOREGROUND;
            } else {
                type_str = SCIM_BRIDGE_MESSAGE_BACKGROUND;
            }
            char *tmp_str = static_cast<char*> (alloca (sizeof (char) * (strlen (SCIM_BRIDGE_MESSAGE_COLOR) + 7)));
            value_str = tmp_str;
            strcpy (tmp_str, SCIM_BRIDGE_MESSAGE_COLOR);
            char *color_str = tmp_str + sizeof (char) * strlen (SCIM_BRIDGE_MESSAGE_COLOR);
            color_str[6] = '\0';
            const char chars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
            for (unsigned int k = 0; k < 6; ++k) {
                const unsigned int digit_index = (attribute.get_value () >> (k * 4)) % 0x10;
                color_str[5 - k] = chars[digit_index];
            }
        } else {
            type_str = SCIM_BRIDGE_MESSAGE_NONE;
            value_str = SCIM_BRIDGE_MESSAGE_NONE;
        }

        scim_bridge_message_set_argument (message, arg_index + 0, begin_str);
        scim_bridge_message_set_argument (message, arg_index + 1, end_str);
        scim_bridge_message_set_argument (message, arg_index + 2, type_str);
        scim_bridge_message_set_argument (message, arg_index + 3, value_str);

        free (begin_str);
        free (end_str);

        arg_index += 4;
    }

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::update_preedit (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "update_preedit ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_UPDATE_PREEDIT, 1);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::commit_string (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstring)
{
    scim_bridge_pdebugln (6, "commit_string ()");

    ScimBridgeMessage *message0 = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_SET_COMMIT_STRING, 2);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message0, 0, imcontext_id_str);

    char *commit_str;
    scim_bridge_wstring_to_string (&commit_str, wstring.c_str ());
    scim_bridge_message_set_argument (message0, 1, commit_str);
    free (commit_str);

    scim_bridge_messenger_push_message (messenger, message0);
    scim_bridge_free_message (message0);

    ScimBridgeMessage *message1 = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_COMMIT_STRING, 1);

    scim_bridge_message_set_argument (message1, 0, imcontext_id_str);
    free (imcontext_id_str);

    scim_bridge_messenger_push_message (messenger, message1);
    scim_bridge_free_message (message1);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::get_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, int max_before, int max_after, WideString &wstring, int &cursor_position)
{
    scim_bridge_pdebugln (6, "get_surrounding_string ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_GET_SURROUNDING_TEXT, 3);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    char *max_before_str;
    scim_bridge_string_from_uint (&max_before_str, max_before);
    scim_bridge_message_set_argument (message, 1, max_before_str);
    free (max_before_str);

    char *max_after_str;
    scim_bridge_string_from_uint (&max_after_str, max_after);
    scim_bridge_message_set_argument (message, 2, max_after_str);
    free (max_after_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    struct timeval first_time;
    gettimeofday (&first_time, NULL);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        struct timeval current_time;
        gettimeofday (&current_time, NULL);
        long remain_usec = (5 + first_time.tv_sec - current_time.tv_sec) * 1000000 + (first_time.tv_usec - current_time.tv_usec);
        if (remain_usec < 0) {
            scim_bridge_perrorln ("A timeout occurred at get_surrounding_text ()");
            ++get_surrounding_timeout_count;
            return RETVAL_IGNORED;
        }

        struct timeval timeout;
        timeout.tv_sec = remain_usec / 1000000;
        timeout.tv_usec = remain_usec % 1000000;
        if (scim_bridge_messenger_send_message (messenger, &timeout)) {
            scim_bridge_perrorln ("An IOException occurred at get_surrounding_text ()");
            return RETVAL_FAILED;
        }
    }

    while (TRUE) {
        struct timeval current_time;
        gettimeofday (&current_time, NULL);
        long remain_usec = (5 + first_time.tv_sec - current_time.tv_sec) * 1000000 + (first_time.tv_usec - current_time.tv_usec);
         
        if (remain_usec < 0) {
            scim_bridge_perrorln ("A timeout occurred at get_surrounding_text ()");
            ++get_surrounding_timeout_count;
            return RETVAL_IGNORED;
        }

        struct timeval timeout;
        timeout.tv_sec = remain_usec / 1000000;
        timeout.tv_usec = remain_usec % 1000000;

        if (scim_bridge_messenger_receive_message (messenger, &timeout)) {
            scim_bridge_perrorln ("An IOException occurred at get_surrounding_text ()");
            return RETVAL_FAILED;
        }

        ScimBridgeMessage *new_message = NULL;
        while (!scim_bridge_messenger_poll_message (messenger, &new_message)) {
            const char *message_header = scim_bridge_message_get_header (new_message);
            if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_GOTTEN) == 0) {

                if (get_surrounding_timeout_count > 0) {
                    const retval_t retval = process_message (new_message);
                    scim_bridge_free_message (new_message);
                    if (retval == RETVAL_FAILED) return RETVAL_FAILED;
                } else {
                    const char *succeeded_str = scim_bridge_message_get_argument (new_message, 0);
                    scim_bridge_pdebugln (5, "The retval: %s", succeeded_str);

                    boolean succeeded = FALSE;
                    if (scim_bridge_string_to_boolean (&succeeded, succeeded_str)) scim_bridge_perrorln ("An invalid argument for the message");

                    if (succeeded) {
                        const char *cursor_pos_str = scim_bridge_message_get_argument (new_message, 1);
                        const char *str = scim_bridge_message_get_argument (new_message, 2);

                        wchar *wstr = NULL;
                        int cursor_pos;
                        if (scim_bridge_string_to_wstring (&wstr, str) < 0 || scim_bridge_string_to_int (&cursor_pos, cursor_pos_str)) {
                            scim_bridge_perrorln ("Invalid arguments for the message");
                            succeeded = FALSE;
                        } else {
                            cursor_position = cursor_pos;
                            wstring = wstr;
                        }
                        free (wstr);
                    }
                    
                    scim_bridge_free_message (new_message);
                    return succeeded ? RETVAL_SUCCEEDED:RETVAL_IGNORED;
                }

            } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_RESET_IMCONTEXT) == 0) {
                const retval_t retval = process_message (new_message);
                scim_bridge_free_message (new_message);
                if (retval == RETVAL_FAILED) return RETVAL_FAILED;
            } else {
                push_message (new_message);
            }
        }
    }
}


retval_t ScimBridgeAgentClientListenerImpl::delete_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, int offset, int length)
{
    scim_bridge_pdebugln (6, "delete_surrounding_string ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_DELETE_SURROUNDING_TEXT, 3);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    char *offset_str;
    scim_bridge_string_from_int (&offset_str, offset);
    scim_bridge_message_set_argument (message, 1, offset_str);
    free (offset_str);

    char *length_str;
    scim_bridge_string_from_uint (&length_str, length);
    scim_bridge_message_set_argument (message, 2, length_str);
    free (length_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    struct timeval first_time;
    gettimeofday (&first_time, NULL);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        struct timeval current_time;
        gettimeofday (&current_time, NULL);
        long remain_usec = (5 + first_time.tv_sec - current_time.tv_sec) * 1000000 + (first_time.tv_usec - current_time.tv_usec);
        if (remain_usec < 0) {
            scim_bridge_perrorln ("A timeout occurred at delete_surrounding_text ()");
            ++delete_surrounding_timeout_count;
            return RETVAL_IGNORED;
        }

        struct timeval timeout;
        timeout.tv_sec = remain_usec / 1000000;
        timeout.tv_usec = remain_usec % 1000000;
        if (scim_bridge_messenger_send_message (messenger, &timeout)) {
            scim_bridge_perrorln ("An IOException occurred at delete_surrounding_text ()");
            return RETVAL_FAILED;
        }
    }

    while (TRUE) {
        struct timeval current_time;
        gettimeofday (&current_time, NULL);
        long remain_usec = (5 + first_time.tv_sec - current_time.tv_sec) * 1000000 + (first_time.tv_usec - current_time.tv_usec);
         
        if (remain_usec < 0) {
            scim_bridge_perrorln ("A timeout occurred at delete_surrounding_text ()");
            ++delete_surrounding_timeout_count;
            return RETVAL_IGNORED;
        }

        struct timeval timeout;
        timeout.tv_sec = remain_usec / 1000000;
        timeout.tv_usec = remain_usec % 1000000;

        if (scim_bridge_messenger_receive_message (messenger, &timeout)) {
            scim_bridge_perrorln ("An IOException occurred at delete_surrounding_text ()");
            return RETVAL_FAILED;
        }

        ScimBridgeMessage *new_message = NULL;
        while (!scim_bridge_messenger_poll_message (messenger, &new_message)) {
            const char *message_header = scim_bridge_message_get_header (new_message);
            if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_DELETED) == 0) {

                if (delete_surrounding_timeout_count > 0) {
                    const retval_t retval = process_message (new_message);
                    scim_bridge_free_message (new_message);
                    if (retval == RETVAL_FAILED) return RETVAL_FAILED;
                } else {
                    const char *succeeded_str = scim_bridge_message_get_argument (new_message, 0);
                    scim_bridge_pdebugln (5, "The retval: %s", succeeded_str);

                    boolean succeeded = FALSE;
                    if (scim_bridge_string_to_boolean (&succeeded, succeeded_str)) scim_bridge_perrorln ("An invalid argument for the message");
                    
                    scim_bridge_free_message (new_message);
                    return succeeded ? RETVAL_SUCCEEDED:RETVAL_IGNORED;
                }

            } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_RESET_IMCONTEXT) == 0) {
                const retval_t retval = process_message (new_message);
                scim_bridge_free_message (new_message);
                if (retval == RETVAL_FAILED) return RETVAL_FAILED;
            } else {
                push_message (new_message);
            }
        }
    }
}


retval_t ScimBridgeAgentClientListenerImpl::replace_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, const WideString &wstring, int cursor_position)
{
    scim_bridge_pdebugln (6, "replace_surrounding_string ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_REPLACE_SURROUNDING_TEXT, 3);

    char *imcontext_id_str;
    scim_bridge_string_from_uint (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    free (imcontext_id_str);

    char *str = NULL;
    if (scim_bridge_wstring_to_string (&str, wstring.c_str ()) < 0) {
        scim_bridge_free_message (message);
        return RETVAL_IGNORED;
    }
    scim_bridge_message_set_argument (message, 1, str);
    free (str);

    char *cursor_position_str;
    scim_bridge_string_from_int (&cursor_position_str, cursor_position);
    scim_bridge_message_set_argument (message, 2, cursor_position_str);
    free (cursor_position_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    struct timeval first_time;
    gettimeofday (&first_time, NULL);

    while (scim_bridge_messenger_get_sending_buffer_size (messenger) > 0) {
        struct timeval current_time;
        gettimeofday (&current_time, NULL);
        long remain_usec = (5 + first_time.tv_sec - current_time.tv_sec) * 1000000 + (first_time.tv_usec - current_time.tv_usec);
        if (remain_usec < 0) {
            scim_bridge_perrorln ("A timeout occurred at replace_surrounding_text ()");
            ++replace_surrounding_timeout_count;
            return RETVAL_IGNORED;
        }

        struct timeval timeout;
        timeout.tv_sec = remain_usec / 1000000;
        timeout.tv_usec = remain_usec % 1000000;
        if (scim_bridge_messenger_send_message (messenger, &timeout)) {
            scim_bridge_perrorln ("An IOException occurred at replace_surrounding_text ()");
            return RETVAL_FAILED;
        }
    }

    while (TRUE) {
        struct timeval current_time;
        gettimeofday (&current_time, NULL);
        long remain_usec = (5 + first_time.tv_sec - current_time.tv_sec) * 1000000 + (first_time.tv_usec - current_time.tv_usec);
         
        if (remain_usec < 0) {
            scim_bridge_perrorln ("A timeout occurred at replace_surrounding_text ()");
            ++replace_surrounding_timeout_count;
            return RETVAL_IGNORED;
        }

        struct timeval timeout;
        timeout.tv_sec = remain_usec / 1000000;
        timeout.tv_usec = remain_usec % 1000000;

        if (scim_bridge_messenger_receive_message (messenger, &timeout)) {
            scim_bridge_perrorln ("An IOException occurred at replace_surrounding_text ()");
            return RETVAL_FAILED;
        }

        ScimBridgeMessage *new_message = NULL;
        while (!scim_bridge_messenger_poll_message (messenger, &new_message)) {
            const char *message_header = scim_bridge_message_get_header (new_message);
            if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_SURROUNDING_TEXT_REPLACED) == 0) {

                if (replace_surrounding_timeout_count > 0) {
                    const retval_t retval = process_message (new_message);
                    scim_bridge_free_message (new_message);
                    if (retval == RETVAL_FAILED) return RETVAL_FAILED;
                } else {
                    const char *succeeded_str = scim_bridge_message_get_argument (new_message, 0);
                    scim_bridge_pdebugln (5, "The retval: %s", succeeded_str);

                    boolean succeeded = FALSE;
                    if (scim_bridge_string_to_boolean (&succeeded, succeeded_str)) scim_bridge_perrorln ("An invalid argument for the message");
                    
                    scim_bridge_free_message (new_message);
                    return succeeded ? RETVAL_SUCCEEDED:RETVAL_IGNORED;
                }

            } else if (strcmp (message_header, SCIM_BRIDGE_MESSAGE_RESET_IMCONTEXT) == 0) {
                const retval_t retval = process_message (new_message);
                scim_bridge_free_message (new_message);
                if (retval == RETVAL_FAILED) return RETVAL_FAILED;
            } else {
                push_message (new_message);
            }
        }
    }
}


retval_t ScimBridgeAgentClientListenerImpl::beep (scim_bridge_imcontext_id_t imcontext_id)
{
    scim_bridge_pdebugln (6, "beep ()");

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_BEEP, 1);

    char *imcontext_id_str;
    scim_bridge_string_from_int (&imcontext_id_str, imcontext_id);
    scim_bridge_message_set_argument (message, 0, imcontext_id_str);

    free (imcontext_id_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentClientListenerImpl::forward_key_event (scim_bridge_imcontext_id_t imcontext_id, const KeyEvent &key_event)
{
    scim_bridge_pdebugln (6, "forward_key_event ()");

    size_t mod_count = 0;
    if (key_event.is_shift_down ()) ++mod_count;
    if (key_event.is_control_down ()) ++mod_count;
    if (key_event.is_alt_down ()) ++mod_count;
    if (key_event.is_meta_down ()) ++mod_count;
    if (key_event.is_hyper_down ()) ++mod_count;
    if (key_event.is_super_down ()) ++mod_count;
    if (key_event.is_caps_lock_down ()) ++mod_count;
    if (key_event.is_num_lock_down ()) ++mod_count;
    
    if (key_event.mask & SCIM_KEY_QuirkKanaRoMask) ++mod_count;

    char *imcontext_id_str;
    scim_bridge_string_from_int (&imcontext_id_str, imcontext_id);

    char* pressed_str;
    scim_bridge_string_from_boolean (&pressed_str, key_event.is_key_press ());

    char *key_code_str;
    scim_bridge_string_from_uint (&key_code_str, key_event.code);

    ScimBridgeMessage *message = scim_bridge_alloc_message (SCIM_BRIDGE_MESSAGE_FORWARD_KEY_EVENT, mod_count + 3);

    scim_bridge_message_set_argument (message, 0, imcontext_id_str);
    scim_bridge_message_set_argument (message, 1, key_code_str);
    scim_bridge_message_set_argument (message, 2, pressed_str);

    size_t mod_index = 3;
    if (key_event.is_shift_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_SHIFT);
        ++mod_index;
    }
    if (key_event.is_control_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_CONTROL);
        ++mod_index;
    }
    if (key_event.is_alt_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_ALT);
        ++mod_index;
    }
    if (key_event.is_meta_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_META);
        ++mod_index;
    }
    if (key_event.is_hyper_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_HYPER);
        ++mod_index;
    }
    if (key_event.is_super_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_SUPER);
        ++mod_index;
    }
    if (key_event.is_caps_lock_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_CAPS_LOCK);
        ++mod_index;
    }
    if (key_event.is_num_lock_down ()) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_NUM_LOCK);
        ++mod_index;
    }
    
    if (key_event.mask & SCIM_KEY_QuirkKanaRoMask) {
        scim_bridge_message_set_argument (message, mod_index, SCIM_BRIDGE_MESSAGE_KANA_RO);
        ++mod_index;
    }

    free (imcontext_id_str);
    free (key_code_str);
    free (pressed_str);

    scim_bridge_messenger_push_message (messenger, message);
    scim_bridge_free_message (message);

    return RETVAL_SUCCEEDED;
}
