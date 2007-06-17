/** @file scim_socket_config.cpp
 * implementation of SocketConfig class.
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
 *
 * $Id: scim_socket_config.cpp,v 1.23 2005/12/16 11:12:26 suzhe Exp $
 */

#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_C_STDIO
#define Uses_C_STDLIB

#include "scim_private.h"
#include "scim.h"
#include "scim_socket_config.h"

#define scim_module_init socket_LTX_scim_module_init
#define scim_module_exit socket_LTX_scim_module_exit
#define scim_config_module_init socket_LTX_scim_config_module_init
#define scim_config_module_create_config socket_LTX_scim_config_module_create_config

using namespace scim;

extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing Socket Config module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_CONFIG(1) << "Exiting Socket Config module...\n";
    }

    void scim_config_module_init ()
    {
        SCIM_DEBUG_CONFIG(1) << "Initializing Socket Config module (more)...\n";
    }

    ConfigPointer scim_config_module_create_config ()
    {
        SCIM_DEBUG_CONFIG(1) << "Creating a Socket Config instance...\n";
        return new SocketConfig ();
    }
}

namespace scim {

SocketConfig::SocketConfig ()
    : m_valid (false),
      m_socket_address (scim_get_default_socket_config_address ()),
      m_socket_timeout (scim_get_default_socket_timeout ()),
      m_connected (false)
{
    SCIM_DEBUG_CONFIG (2) << " Construct SocketConfig object.\n";

    m_valid = open_connection ();
}

SocketConfig::~SocketConfig ()
{
    m_socket_client.close ();
}

bool
SocketConfig::valid () const
{
    return ConfigBase::valid() && m_valid;
}

String
SocketConfig::get_name () const
{
    return "socket";
}

// String
bool
SocketConfig::read (const String& key, String *pStr) const
{
    if (!valid () || !pStr || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;

    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_CONFIG_STRING);
        trans.put_data (key);

        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (*pStr) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    *pStr = String ("");
    return false;
}

// int
bool
SocketConfig::read (const String& key, int *pl) const
{
    if (!valid () || !pl || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_CONFIG_INT);
        trans.put_data (key);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            uint32 val;
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (val) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                *pl = val;
                return true;
            }

            break;
        }

        if (!open_connection ())
            break;
    }

    *pl = 0;
    return false;
}

// double
bool
SocketConfig::read (const String& key, double* val) const
{
    if (!valid () || !val || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_CONFIG_DOUBLE);
        trans.put_data (key);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            String str;
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (str) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                sscanf (str.c_str (), "%lE", val);
                return true;
            }

            break;
        }

        if (!open_connection ())
            break;
    }

    *val = 0;
    return false;
}

// bool
bool
SocketConfig::read (const String& key, bool* val) const
{
    if (!valid () || !val || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;
    
    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_CONFIG_BOOL);
        trans.put_data (key);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            uint32 tmp;
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (tmp) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                *val = (bool)tmp;
                return true;
            }

            break;
        }

        if (!open_connection ())
            break;
    }

    *val = false;
    return false;
}

//String list
bool
SocketConfig::read (const String& key, std::vector <String>* val) const
{
    if (!valid () || !val || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;
    
    val->clear ();

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_CONFIG_VECTOR_STRING);
        trans.put_data (key);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (*val) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                return true;
            }

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

//int list
bool
SocketConfig::read (const String& key, std::vector <int>* val) const
{
    if (!valid () || !val || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;
    
    val->clear();

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_CONFIG_VECTOR_INT);
        trans.put_data (key);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            std::vector<uint32> vec;
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_data (vec) &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                for (uint32 i=0; i<vec.size (); ++i)
                    val->push_back ((int) vec[i]);
                return true;
            }

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

// write the value (return true on success)
bool
SocketConfig::write (const String& key, const String& value)
{
    if (!valid () || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_SET_CONFIG_STRING);
        trans.put_data (key);
        trans.put_data (value);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

bool
SocketConfig::write (const String& key, int value)
{
    if (!valid () || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_SET_CONFIG_INT);
        trans.put_data (key);
        trans.put_data ((uint32)value);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) { 
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

bool
SocketConfig::write (const String& key, double value)
{
    if (!valid () || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;
    
    char buf [256];
    snprintf (buf, 255, "%lE", value);

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_SET_CONFIG_DOUBLE);
        trans.put_data (key);
        trans.put_data (String (buf));
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

bool
SocketConfig::write (const String& key, bool value)
{
    if (!valid () || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_SET_CONFIG_BOOL);
        trans.put_data (key);
        trans.put_data ((uint32)value);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

bool
SocketConfig::write (const String& key, const std::vector <String>& value)
{
    if (!valid () || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_SET_CONFIG_VECTOR_STRING);
        trans.put_data (key);
        trans.put_data (value);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

bool
SocketConfig::write (const String& key, const std::vector <int>& value)
{
    if (!valid () || key.empty()) return false;
    if (!m_connected && !open_connection ()) return false;

    std::vector <uint32> vec;

    for (uint32 i=0; i<value.size (); ++i)
        vec.push_back ((uint32) value [i]);

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_SET_CONFIG_VECTOR_INT);
        trans.put_data (key);
        trans.put_data (vec);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;
            
            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

// permanently writes all changes
bool
SocketConfig::flush()
{
    if (!valid ()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_FLUSH_CONFIG);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                gettimeofday (&m_update_timestamp, 0);
                return true;
            }
            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

// delete entries
bool
SocketConfig::erase (const String& key)
{
    if (!valid ()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_ERASE_CONFIG);
        trans.put_data (key);
 
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK)
                return true;

            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

bool
SocketConfig::reload ()
{
    if (!valid ()) return false;
    if (!m_connected && !open_connection ()) return false;

    Transaction trans;
    int cmd;

    for (int retry = 0; retry < 3; ++retry) {
        init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_RELOAD_CONFIG);
 
        // The reload process may take very long time, so wait a little longer time.
        if (trans.write_to_socket (m_socket_client) &&
            trans.read_from_socket (m_socket_client, m_socket_timeout * 10)) {
            if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
                trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
                String str;
                if (read (String (SCIM_CONFIG_UPDATE_TIMESTAMP), &str)) {
                    std::vector <String> strs;
                    if (scim_split_string_list (strs, str, ':') == 2) {
                        time_t sec = (time_t) strtol (strs [0].c_str (), 0, 10);
                        suseconds_t usec = (suseconds_t) strtol (strs [1].c_str (), 0, 10);
 
                        // The config file is newer, so load it.
                        if (m_update_timestamp.tv_sec < sec ||
                            (m_update_timestamp.tv_sec == sec && m_update_timestamp.tv_usec < usec)) {
                            m_update_timestamp.tv_sec = sec;
                            m_update_timestamp.tv_usec = usec;
                            return ConfigBase::reload ();
                        }
                    }
                }
            }
            break;
        }

        if (!open_connection ())
            break;
    }

    return false;
}

void
SocketConfig::init_transaction (Transaction &trans) const
{
    trans.clear ();
    trans.put_command (SCIM_TRANS_CMD_REQUEST);
    trans.put_data (m_socket_magic_key);
}

bool
SocketConfig::open_connection () const
{
    SocketAddress socket_address (m_socket_address);

    m_connected = false;

    // Connect to SocketFrontEnd.
    if (!m_socket_client.connect (socket_address)) {
        SCIM_DEBUG_CONFIG (2) << " Cannot connect to SocketFrontEnd (" << m_socket_address << ").\n";
        return false;
    }

    // Init the connection,
    if (!scim_socket_open_connection (m_socket_magic_key,
                                      String ("SocketConfig"),
                                      String ("SocketFrontEnd"),
                                      m_socket_client,
                                      m_socket_timeout)) {
        m_socket_client.close ();
        return false;
    }

    m_connected = true;
    gettimeofday (&m_update_timestamp, 0);
    return true;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
