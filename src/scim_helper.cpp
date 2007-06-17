/** @file scim_helper.cpp
 *  @brief Implementation of class HelperAgent.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2004-2005 James Su <suzhe@tsinghua.org.cn>
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
 *
 * $Id: scim_helper.cpp,v 1.13 2005/05/24 12:22:51 suzhe Exp $
 *
 */

#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_HELPER
#define Uses_SCIM_SOCKET
#define Uses_SCIM_EVENT

#include "scim_private.h"
#include "scim.h"

namespace scim {

typedef Signal3<void, const HelperAgent *, int, const String &>
        HelperAgentSignalVoid;

typedef Signal4<void, const HelperAgent *, int, const String &, const String &>
        HelperAgentSignalString;

typedef Signal4<void, const HelperAgent *, int, const String &, int>
        HelperAgentSignalInt;

typedef Signal5<void, const HelperAgent *, int, const String &, int, int>
        HelperAgentSignalIntInt;

typedef Signal4<void, const HelperAgent *, int, const String &, const Transaction &>
        HelperAgentSignalTransaction;

class HelperAgent::HelperAgentImpl
{
public:
    SocketClient socket;
    Transaction  recv;
    Transaction  send;
    uint32       magic;
    int          timeout;

    HelperAgentSignalVoid           signal_exit;
    HelperAgentSignalVoid           signal_attach_input_context;
    HelperAgentSignalVoid           signal_detach_input_context;
    HelperAgentSignalVoid           signal_reload_config;
    HelperAgentSignalInt            signal_update_screen;
    HelperAgentSignalIntInt         signal_update_spot_location;
    HelperAgentSignalString         signal_trigger_property;
    HelperAgentSignalTransaction    signal_process_imengine_event;

public:
    HelperAgentImpl () : magic (0), timeout (-1) { }
};

HelperAgent::HelperAgent ()
    : m_impl (new HelperAgentImpl ())
{
}

HelperAgent::~HelperAgent ()
{
    delete m_impl;
}

int
HelperAgent::open_connection (const HelperInfo &info,
                              const String     &display)
{
    if (m_impl->socket.is_connected ())
        close_connection ();

    SocketAddress address (scim_get_default_panel_socket_address (display));
    int timeout = scim_get_default_socket_timeout ();
    uint32 magic;

    if (!address.valid ())                 return -1;
    if (!m_impl->socket.connect (address)) return -1;
    if (!scim_socket_open_connection (magic,
                                      String ("Helper"),
                                      String ("Panel"),
                                      m_impl->socket,
                                      timeout)) {
        m_impl->socket.close ();
        return -1;
    }

    m_impl->send.clear ();
    m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
    m_impl->send.put_data (magic);
    m_impl->send.put_command (SCIM_TRANS_CMD_PANEL_REGISTER_HELPER);
    m_impl->send.put_data (info.uuid);
    m_impl->send.put_data (info.name);
    m_impl->send.put_data (info.icon);
    m_impl->send.put_data (info.description);
    m_impl->send.put_data (info.option);

    if (!m_impl->send.write_to_socket (m_impl->socket, magic)) {
        m_impl->socket.close ();
        return -1;
    }

    int cmd;
    if (m_impl->recv.read_from_socket (m_impl->socket, timeout) &&
        m_impl->recv.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
        m_impl->recv.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
        m_impl->magic = magic;
        m_impl->timeout = timeout;

        while (m_impl->recv.get_command (cmd)) {
            switch (cmd) {
                case SCIM_TRANS_CMD_HELPER_ATTACH_INPUT_CONTEXT:
                {
                    uint32 ic;
                    String ic_uuid;
                    while (m_impl->recv.get_data (ic) && m_impl->recv.get_data (ic_uuid))
                        m_impl->signal_attach_input_context (this, ic, ic_uuid);
                    break;
                }
                case SCIM_TRANS_CMD_UPDATE_SCREEN:
                {
                    uint32 screen;
                    if (m_impl->recv.get_data (screen))
                        m_impl->signal_update_screen (this, (uint32) -1, String (""), (int) screen);
                    break;
                }
            }
        }

        return m_impl->socket.get_id ();
    }

    m_impl->socket.close ();
    return -1;
}

void
HelperAgent::close_connection ()
{
    m_impl->socket.close ();
    m_impl->send.clear ();
    m_impl->recv.clear ();
    m_impl->magic = 0;
    m_impl->timeout = 0;
}

int
HelperAgent::get_connection_number () const
{
    if (m_impl->socket.is_connected ())
        return m_impl->socket.get_id ();
    return -1;
}

bool
HelperAgent::is_connected () const
{
    return m_impl->socket.is_connected ();
}

bool
HelperAgent::has_pending_event () const
{
    if (m_impl->socket.is_connected () && m_impl->socket.wait_for_data (0) > 0)
        return true;

    return false;
}

bool
HelperAgent::filter_event ()
{
    if (!m_impl->socket.is_connected () || !m_impl->recv.read_from_socket (m_impl->socket, m_impl->timeout))
        return false;

    int cmd;

    uint32 ic = (uint32) -1;
    String ic_uuid;

    if (!m_impl->recv.get_command (cmd) || cmd != SCIM_TRANS_CMD_REPLY)
        return true;

    // If there are ic and ic_uuid then read them.
    if (!(m_impl->recv.get_data_type () == SCIM_TRANS_DATA_COMMAND) &&
        !(m_impl->recv.get_data (ic) && m_impl->recv.get_data (ic_uuid)))
        return true;

    while (m_impl->recv.get_command (cmd)) {
        switch (cmd) {
            case SCIM_TRANS_CMD_EXIT:
                m_impl->signal_exit (this, ic, ic_uuid);
                break;
            case SCIM_TRANS_CMD_RELOAD_CONFIG:
                m_impl->signal_reload_config (this, ic, ic_uuid);
                break;
            case SCIM_TRANS_CMD_UPDATE_SCREEN:
            {
                uint32 screen;
                if (m_impl->recv.get_data (screen))
                    m_impl->signal_update_screen (this, ic, ic_uuid, (int) screen);
                break;
            }
            case SCIM_TRANS_CMD_UPDATE_SPOT_LOCATION:
            {
                uint32 x, y;
                if (m_impl->recv.get_data (x) && m_impl->recv.get_data (y))
                    m_impl->signal_update_spot_location (this, ic, ic_uuid, (int) x, (int) y);
                break;
            }
            case SCIM_TRANS_CMD_TRIGGER_PROPERTY:
            {
                String property;
                if (m_impl->recv.get_data (property))
                    m_impl->signal_trigger_property (this, ic, ic_uuid, property);
                break;
            }
            case SCIM_TRANS_CMD_HELPER_PROCESS_IMENGINE_EVENT:
            {
                Transaction trans;
                if (m_impl->recv.get_data (trans))
                    m_impl->signal_process_imengine_event (this, ic, ic_uuid, trans);
                break;
            }
            case SCIM_TRANS_CMD_HELPER_ATTACH_INPUT_CONTEXT:
                m_impl->signal_attach_input_context (this, ic, ic_uuid);
                break;
            case SCIM_TRANS_CMD_HELPER_DETACH_INPUT_CONTEXT:
                m_impl->signal_detach_input_context (this, ic, ic_uuid);
                break;
        }
    }
    return true;
}

void
HelperAgent::reload_config () const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_RELOAD_CONFIG);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

void
HelperAgent::register_properties (const PropertyList &properties) const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_REGISTER_PROPERTIES);
        m_impl->send.put_data (properties);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

void
HelperAgent::update_property (const Property &property) const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_UPDATE_PROPERTY);
        m_impl->send.put_data (property);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

void
HelperAgent::send_imengine_event (int                ic,
                                  const String      &ic_uuid,
                                  const Transaction &trans) const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_PANEL_SEND_IMENGINE_EVENT);
        m_impl->send.put_data ((uint32)ic);
        m_impl->send.put_data (ic_uuid);
        m_impl->send.put_data (trans);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

void
HelperAgent::send_key_event (int            ic,
                             const String   &ic_uuid,
                             const KeyEvent &key) const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_PANEL_SEND_KEY_EVENT);
        m_impl->send.put_data ((uint32)ic);
        m_impl->send.put_data (ic_uuid);
        m_impl->send.put_data (key);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

void
HelperAgent::forward_key_event (int            ic,
                                const String   &ic_uuid,
                                const KeyEvent &key) const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_FORWARD_KEY_EVENT);
        m_impl->send.put_data ((uint32)ic);
        m_impl->send.put_data (ic_uuid);
        m_impl->send.put_data (key);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

void
HelperAgent::commit_string (int               ic,
                            const String     &ic_uuid,
                            const WideString &wstr) const
{
    if (m_impl->socket.is_connected ()) {
        m_impl->send.clear ();
        m_impl->send.put_command (SCIM_TRANS_CMD_REQUEST);
        m_impl->send.put_data (m_impl->magic);
        m_impl->send.put_command (SCIM_TRANS_CMD_COMMIT_STRING);
        m_impl->send.put_data ((uint32)ic);
        m_impl->send.put_data (ic_uuid);
        m_impl->send.put_data (wstr);
        m_impl->send.write_to_socket (m_impl->socket, m_impl->magic);
    }
}

Connection
HelperAgent::signal_connect_exit (HelperAgentSlotVoid *slot)
{
    return m_impl->signal_exit.connect (slot);
}

Connection
HelperAgent::signal_connect_attach_input_context (HelperAgentSlotVoid *slot)
{
    return m_impl->signal_attach_input_context.connect (slot);
}

Connection
HelperAgent::signal_connect_detach_input_context (HelperAgentSlotVoid *slot)
{
    return m_impl->signal_detach_input_context.connect (slot);
}

Connection
HelperAgent::signal_connect_reload_config (HelperAgentSlotVoid *slot)
{
    return m_impl->signal_reload_config.connect (slot);
}

Connection
HelperAgent::signal_connect_update_screen (HelperAgentSlotInt *slot)
{
    return m_impl->signal_update_screen.connect (slot);
}

Connection
HelperAgent::signal_connect_update_spot_location (HelperAgentSlotIntInt *slot)
{
    return m_impl->signal_update_spot_location.connect (slot);
}

Connection
HelperAgent::signal_connect_trigger_property (HelperAgentSlotString *slot)
{
    return m_impl->signal_trigger_property.connect (slot);
}

Connection
HelperAgent::signal_connect_process_imengine_event (HelperAgentSlotTransaction *slot)
{
    return m_impl->signal_process_imengine_event.connect (slot);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/

