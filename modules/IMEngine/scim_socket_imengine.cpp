/** @file scim_socket_imengine.cpp
 * implementation of class SocketFactory and SocketInstance.
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
 * $Id: scim_socket_imengine.cpp,v 1.21 2005/07/06 03:57:04 suzhe Exp $
 *
 */

#define Uses_SCIM_IMENGINE
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_C_STDLIB

#include <unistd.h>
#include "scim_private.h"
#include "scim.h"
#include "scim_socket_imengine.h"
#include "scim_stl_map.h"

#define scim_module_init                    socket_LTX_scim_module_init
#define scim_module_exit                    socket_LTX_scim_module_exit
#define scim_imengine_module_init           socket_LTX_scim_imengine_module_init
#define scim_imengine_module_create_factory socket_LTX_scim_imengine_module_create_factory

#define SCIM_CONFIG_IMENGINE_SOCKET_TIMEOUT "/IMEngine/Socket/Timeout"
#define SCIM_CONFIG_IMENGINE_SOCKET_ADDRESS "/IMEngine/Socket/Address"

#define SCIM_SOCKET_FRONTEND_DEF_ADDRESS    "local:/tmp/scim-socket-frontend"

#ifndef SCIM_TEMPDIR
  #define SCIM_TEMPDIR "/tmp"
#endif

using namespace scim;

class scim::SocketIMEngineGlobal
{
#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <String, String, scim_hash_string> IconRepository;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <String, String, scim_hash_string> IconRepository;
#else
typedef std::map <String, String> IconRepository;
#endif

    SocketClient             m_socket_client;
    SocketAddress            m_socket_address;
    uint32                   m_socket_magic_key;
    int                      m_socket_timeout;

    std::vector<String>      m_peer_factories;

    IconRepository           m_icon_repository;

    Signal0<void>            m_signal_reconnect;

public:
    SocketIMEngineGlobal ();
    ~SocketIMEngineGlobal ();

    bool            create_connection ();
    unsigned int    number_of_factories ();
    SocketFactory * create_factory (unsigned int index);

    void            init_transaction (Transaction &trans);
    bool            send_transaction (Transaction &trans);
    bool            receive_transaction (Transaction &trans);

    String          load_icon (const String &icon);

    Connection      connect_reconnect_signal (Slot0<void> *slot_reconnect);

private:
    void            init ();
    void            destroy ();

    void            destroy_all_icons ();
};

static SocketIMEngineGlobal *global = 0;

extern "C" {
    void scim_module_init (void)
    {
        if (!global)
            global = new SocketIMEngineGlobal;
    }

    void scim_module_exit (void)
    {
        if (global) {
            delete global;
            global = 0;
        }
    }

    unsigned int scim_imengine_module_init (const ConfigPointer &config)
    {
        if (global)
            return global->number_of_factories ();
        return 0;
    }

    IMEngineFactoryPointer scim_imengine_module_create_factory (unsigned int index)
    {
        if (!global || index >= global->number_of_factories ())
            return 0;

        SocketFactory *sf = global->create_factory (index);

        if (!sf || !sf->valid ()) {
            delete sf;
            sf = 0;
        }

        return sf;
    }
}

namespace scim {

SocketIMEngineGlobal::SocketIMEngineGlobal ()
    : m_socket_magic_key (0),
      m_socket_timeout (-1)
{
    init ();
}

SocketIMEngineGlobal::~SocketIMEngineGlobal ()
{
    destroy ();
}

void
SocketIMEngineGlobal::init ()
{
    SCIM_DEBUG_IMENGINE(1) << "Init SocketIMEngine Global.\n";

    String address = scim_get_default_socket_imengine_address ();

    m_socket_timeout = scim_get_default_socket_timeout ();
    m_socket_address.set_address (address);

    if (!m_socket_address.valid ())
        return;

    // Connect to SocketFrontEnd.
    if (!create_connection ()) {
        SCIM_DEBUG_IMENGINE(2) << " Cannot connect to SocketFrontEnd (" << address << ").\n";
        return;
    }

    SCIM_DEBUG_IMENGINE(2) << " Connected to SocketFrontEnd (" << address
                         << ") MagicKey (" << m_socket_magic_key << ").\n";

    // Init the connection, and get IMEngineFactory list.
    Transaction trans;

    // Get IMEngineFactory list.
    init_transaction (trans);

    trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_LIST);
    trans.put_data (String (""));
    if (!send_transaction (trans))
        return;

    int cmd;
    if (trans.read_from_socket (m_socket_client, m_socket_timeout) &&
        trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
        trans.get_data (m_peer_factories) &&
        trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

        SCIM_DEBUG_IMENGINE(2) << " Found " << m_peer_factories.size ()
            << " IMEngine Factories.\n";
    }
}

bool
SocketIMEngineGlobal::create_connection ()
{
    // Connect to SocketFrontEnd.
    if (!m_socket_client.connect (m_socket_address))
        return false;

    if (!scim_socket_open_connection (m_socket_magic_key,
                                      String ("SocketIMEngine"),
                                      String ("SocketFrontEnd"),
                                      m_socket_client,
                                      m_socket_timeout)) {
        m_socket_client.close ();
        return false;
    }

    m_signal_reconnect.emit ();

    return true;
}

void
SocketIMEngineGlobal::destroy ()
{
    SCIM_DEBUG_IMENGINE(1) << "Destroy SocketIMEngine Global.\n";

    m_socket_client.close ();

    destroy_all_icons ();
}

unsigned int
SocketIMEngineGlobal::number_of_factories ()
{
    return m_peer_factories.size ();
}

SocketFactory *
SocketIMEngineGlobal::create_factory (unsigned int index)
{
    if (index < m_peer_factories.size ()) {
        return new SocketFactory (m_peer_factories [index]);
    }
    return 0;
}

void
SocketIMEngineGlobal::init_transaction (Transaction &trans)
{
    trans.clear ();
    trans.put_command (SCIM_TRANS_CMD_REQUEST);
    trans.put_data (m_socket_magic_key);
}

bool
SocketIMEngineGlobal::send_transaction (Transaction &trans)
{
    return trans.write_to_socket (m_socket_client);
}

bool
SocketIMEngineGlobal::receive_transaction (Transaction &trans)
{
    return trans.read_from_socket (m_socket_client, m_socket_timeout);
}

String
SocketIMEngineGlobal::load_icon (const String &icon)
{
    String local_icon = icon;

    IconRepository::const_iterator it = m_icon_repository.find (icon);

    // The icon has been loaded, just return the local copy filename.
    if (it != m_icon_repository.end ())
        local_icon = it->second;

    // This icon is already available in local system, just return.
    if (scim_load_file (local_icon, 0) != 0)
        return local_icon;

    Transaction trans;
    int cmd;
    char *bufptr = 0;
    size_t filesize = 0;

    local_icon = String ("");

    init_transaction (trans);
    trans.put_command (SCIM_TRANS_CMD_LOAD_FILE);
    trans.put_data (icon);

    // Load icon file from remote SocketFrontEnd.
    if (send_transaction (trans) && receive_transaction (trans) &&
        trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
        trans.get_data (&bufptr, filesize) &&
        trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

        String tempfile;
        String::size_type pos = icon.rfind (SCIM_PATH_DELIM);

        if (pos != String::npos) {
            tempfile = icon.substr (pos + 1, String::npos);
        } else {
            tempfile = icon;
        }

        char tmp [80];
        snprintf (tmp, 80, "%lu", (unsigned long) m_socket_magic_key);

        tempfile = String (SCIM_TEMPDIR) + String (SCIM_PATH_DELIM_STRING) +
                   String ("scim-") + String (tmp) + String ("-") +
                   tempfile;

        SCIM_DEBUG_IMENGINE(1) << "Creating temporary icon file: " << tempfile << "\n";

        std::ofstream os (tempfile.c_str ());

        if (os) {
            os.write (bufptr, filesize);
            os.close ();

            // Check if the file is written correctly.
            if (scim_load_file (tempfile, 0) == filesize) {
                m_icon_repository [icon] = tempfile;
                local_icon = tempfile;
            } else {
                unlink (tempfile.c_str ());
            }
        }
    }

    delete [] bufptr;

    return local_icon;
}

Connection
SocketIMEngineGlobal::connect_reconnect_signal (Slot0<void> *slot_reconnect)
{
    return m_signal_reconnect.connect (slot_reconnect);
}

void
SocketIMEngineGlobal::destroy_all_icons ()
{
    IconRepository::const_iterator it = m_icon_repository.begin ();

    for (; it != m_icon_repository.end (); ++ it) {
        unlink (it->second.c_str ());
    }

    m_icon_repository.clear ();
}

int
SocketFactory::create_peer_instance (const String &encoding)
{
    int cmd;
    int si_peer_id = -1;
    uint32 val;
    Transaction trans;

    SCIM_DEBUG_IMENGINE(1) << "Create IMEngine Instance " << m_peer_uuid << ".\n";

    global->init_transaction (trans);
    trans.put_command (SCIM_TRANS_CMD_NEW_INSTANCE);
    trans.put_data (m_peer_uuid);
    trans.put_data (encoding);
    if (global->send_transaction (trans)) {
        if (global->receive_transaction (trans) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (val) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

            si_peer_id = (int) val;
        }
    }

    SCIM_DEBUG_IMENGINE(2) << " IMEngineInstance created (" << si_peer_id << ")\n";

    return si_peer_id;
}

SocketFactory::SocketFactory (const String &peer_uuid)
    : m_name (utf8_mbstowcs (_("Unknown"))),
      m_language (String ("")),
      m_peer_uuid (peer_uuid),
      m_icon_file (String ("")),
      m_ok (false)
{
    String locales;
    String iconfile;
    int cmd;
    bool m_name_ok = false;
    bool m_locale_ok = false;
    Transaction trans;

    SCIM_DEBUG_IMENGINE(1) << "Create SocketFactory " << peer_uuid << ".\n";

    // Get factory name.
    global->init_transaction (trans);
    trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_NAME);
    trans.put_data (m_peer_uuid);
    if (global->send_transaction (trans)) {
        if (global->receive_transaction (trans) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (m_name) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

            SCIM_DEBUG_IMENGINE(2) << " Name (" << utf8_wcstombs (m_name) << ")\n";

            m_name_ok = true;

        } else {
            m_name = utf8_mbstowcs (_("Unknown"));
        }
    }

    // Get factory locales
    global->init_transaction (trans);
    trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_LOCALES);
    trans.put_data (m_peer_uuid);
    if (global->send_transaction (trans)) {
        if (global->receive_transaction (trans) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (locales) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

            SCIM_DEBUG_IMENGINE(2) << " Locales (" << locales << ")\n";

            set_locales (locales);
            m_locale_ok = true;
        }
    }

    // Get factory language
    global->init_transaction (trans);
    trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_LANGUAGE);
    trans.put_data (m_peer_uuid);
    if (global->send_transaction (trans)) {
        if (global->receive_transaction (trans) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (m_language) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {

            SCIM_DEBUG_IMENGINE(2) << " Language (" << m_language << ")\n";

        } else {
            m_language.clear ();
        }
    }

    // Get icon file.
    global->init_transaction (trans);
    trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_ICON_FILE);
    trans.put_data (m_peer_uuid);
    if (global->send_transaction (trans)) {
        if (global->receive_transaction (trans) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (iconfile) &&
            trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK) {
            m_icon_file = global->load_icon (iconfile);
        }
    }

    m_ok = (m_name_ok && m_locale_ok);
}

SocketFactory::~SocketFactory ()
{
}

WideString
SocketFactory::get_name () const
{
    return m_name;
}

WideString
SocketFactory::get_authors () const
{
    int cmd;
    WideString authors;
    Transaction trans;

    SCIM_DEBUG_IMENGINE(1) << "Get Authors " << m_peer_uuid << ".\n";

    // Get factory authors.
    for (int retry = 0; retry < 3; ++retry) {
        global->init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_AUTHORS);
        trans.put_data (m_peer_uuid);

        if (global->send_transaction (trans) && global->receive_transaction (trans) &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (authors) &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_OK)
            break;

        authors = utf8_mbstowcs (_("Unknown"));

        if (!global->create_connection ())
            break;
    }

    return authors;
}

WideString
SocketFactory::get_credits () const
{
    int cmd;
    WideString credits;
    Transaction trans;

    SCIM_DEBUG_IMENGINE(1) << "Get Credits " << m_peer_uuid << ".\n";

    // Get factory credits.
    for (int retry = 0; retry < 3; ++retry) {
        global->init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_CREDITS);
        trans.put_data (m_peer_uuid);

        if (global->send_transaction (trans) && global->receive_transaction (trans) &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (credits) &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_OK)
            break;

        credits = utf8_mbstowcs (_("Unknown"));

        if (!global->create_connection ())
            break;
    }

    return credits;
}

WideString
SocketFactory::get_help () const
{
    int cmd;
    WideString help;
    Transaction trans;

    SCIM_DEBUG_IMENGINE(1) << "Get Help " << m_peer_uuid << ".\n";

    // Get factory help.
    for (int retry = 0; retry < 3; ++retry) {
        global->init_transaction (trans);
        trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_HELP);
        trans.put_data (m_peer_uuid);

        if (global->send_transaction (trans) && global->receive_transaction (trans) &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_REPLY &&
            trans.get_data (help)    &&
            trans.get_command (cmd)  && cmd == SCIM_TRANS_CMD_OK)
            break;

        help = utf8_mbstowcs (_("Unknown"));

        if (!global->create_connection ())
            break;
    }

    return help;
}

String
SocketFactory::get_uuid () const
{
    return m_peer_uuid;
}

String
SocketFactory::get_icon_file () const
{
    return m_icon_file;
}

String
SocketFactory::get_language () const
{
    if (m_language.length ())
        return m_language;
    else
        return IMEngineFactoryBase::get_language ();
}

IMEngineInstancePointer
SocketFactory::create_instance (const String& encoding, int id)
{
    int si_peer_id = create_peer_instance (encoding);

    SCIM_DEBUG_IMENGINE(2) << " IMEngineInstance created (" << si_peer_id << ")\n";

    return new SocketInstance (this, encoding, id, si_peer_id);
}

SocketInstance::SocketInstance (SocketFactory *factory,
                                const String& encoding,
                                int           id,
                                int           peer_id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_peer_id (peer_id)
{
    m_signal_reconnect_connection = global->connect_reconnect_signal (slot (this, &SocketInstance::reconnect_callback));
}

SocketInstance::~SocketInstance ()
{
    Transaction trans;

    SCIM_DEBUG_IMENGINE(1) << "Destroy IMEngine Instance " << m_peer_id << ".\n";

    m_signal_reconnect_connection.disconnect ();

    if (m_peer_id >= 0) {
        global->init_transaction (trans);

        trans.put_command (SCIM_TRANS_CMD_DELETE_INSTANCE);
        trans.put_data (m_peer_id);

        commit_transaction (trans);
    }
}

bool
SocketInstance::process_key_event (const KeyEvent& key)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "process_key_event (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_PROCESS_KEY_EVENT);
    trans.put_data (m_peer_id);
    trans.put_data (key);

    return commit_transaction (trans);
}

void
SocketInstance::move_preedit_caret (unsigned int pos)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "move_preedit_caret (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_MOVE_PREEDIT_CARET);
    trans.put_data (m_peer_id);
    trans.put_data ((uint32) pos);

    commit_transaction (trans);
}

void
SocketInstance::select_candidate (unsigned int item)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "select_candidate (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_SELECT_CANDIDATE);
    trans.put_data (m_peer_id);
    trans.put_data ((uint32) item);

    commit_transaction (trans);
}

void
SocketInstance::update_lookup_table_page_size (unsigned int page_size)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "update_lookup_table_page_size (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE_PAGE_SIZE);
    trans.put_data (m_peer_id);
    trans.put_data ((uint32) page_size);

    commit_transaction (trans);
}

void
SocketInstance::lookup_table_page_up ()
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "lookup_table_page_up (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_UP);
    trans.put_data (m_peer_id);

    commit_transaction (trans);
}

void
SocketInstance::lookup_table_page_down ()
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "lookup_table_page_up (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_DOWN);
    trans.put_data (m_peer_id);

    commit_transaction (trans);
}

void
SocketInstance::reset ()
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "reset (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_RESET);
    trans.put_data (m_peer_id);

    commit_transaction (trans);
}

void
SocketInstance::focus_in ()
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "focus_in (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_FOCUS_IN);
    trans.put_data (m_peer_id);

    commit_transaction (trans);
}

void
SocketInstance::focus_out ()
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "focus_out (" << m_peer_id << ")\n";

    trans.put_command (SCIM_TRANS_CMD_FOCUS_OUT);
    trans.put_data (m_peer_id);

    commit_transaction (trans);
}

void
SocketInstance::trigger_property (const String &property)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "trigger_property (" << m_peer_id << ", " << property << ")\n";

    trans.put_command (SCIM_TRANS_CMD_TRIGGER_PROPERTY);
    trans.put_data (m_peer_id);
    trans.put_data (property);

    commit_transaction (trans);
}

void
SocketInstance::process_helper_event (const String &helper_uuid, const Transaction &helper_trans)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "process_helper_event (" << m_peer_id << ", " << helper_uuid << ")\n";

    trans.put_command (SCIM_TRANS_CMD_PROCESS_HELPER_EVENT);
    trans.put_data (m_peer_id);
    trans.put_data (helper_uuid);
    trans.put_data (helper_trans);

    commit_transaction (trans);
}

void
SocketInstance::update_client_capabilities (unsigned int cap)
{
    Transaction trans;

    global->init_transaction (trans);

    SCIM_DEBUG_IMENGINE(1) << "update_client_capabilities (" << m_peer_id << ", " << cap << ")\n";

    trans.put_command (SCIM_TRANS_CMD_UPDATE_CLIENT_CAPABILITIES);
    trans.put_data (m_peer_id);
    trans.put_data ((uint32) cap);

    commit_transaction (trans);
}

bool
SocketInstance::commit_transaction (Transaction &trans)
{
    SCIM_DEBUG_IMENGINE(2) << " commit_transaction:\n";

    bool ret = false;

    if (m_peer_id >= 0) {
        if (global->send_transaction (trans)) {
            while (1) {
                if (!global->receive_transaction (trans)) break;
                if (!do_transaction (trans, ret)) return ret;
            }
        }
    }

    if (global->create_connection ())
        reset ();

    return ret;
}

bool
SocketInstance::do_transaction (Transaction &trans, bool &ret)
{
    int cmd = -1;
    bool cont = false;

    ret = false;

    SCIM_DEBUG_IMENGINE(2) << " Do transaction:\n";

    if (trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY) {
        while (trans.get_command (cmd)) {
            switch (cmd) {
                case SCIM_TRANS_CMD_SHOW_PREEDIT_STRING:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  show_preedit_string ()\n";
                    show_preedit_string ();
                    break;
                }
                case SCIM_TRANS_CMD_SHOW_AUX_STRING:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  show_aux_string ()\n";
                    show_aux_string ();
                    break;
                }
                case SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  show_lookup_table ()\n";
                    show_lookup_table ();
                    break;
                }
                case SCIM_TRANS_CMD_HIDE_PREEDIT_STRING:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  hide_preedit_string ()\n";
                    hide_preedit_string ();
                    break;
                }
                case SCIM_TRANS_CMD_HIDE_AUX_STRING:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  hide_aux_string ()\n";
                    hide_aux_string ();
                    break;
                }
                case SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  hide_lookup_table ()\n";
                    hide_lookup_table ();
                    break;
                }
                case SCIM_TRANS_CMD_UPDATE_PREEDIT_CARET:
                {
                    uint32 caret;
                    if (trans.get_data (caret)) {
                        SCIM_DEBUG_IMENGINE(3) << "  update_preedit_caret (" << caret << ")\n";
                        update_preedit_caret (caret);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_UPDATE_PREEDIT_STRING:
                {
                    WideString str;
                    AttributeList attrs;
                    if (trans.get_data (str) && trans.get_data (attrs)) {
                        SCIM_DEBUG_IMENGINE(3) << "  update_preedit_string ()\n";
                        update_preedit_string (str, attrs);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_UPDATE_AUX_STRING:
                {
                    WideString str;
                    AttributeList attrs;
                    if (trans.get_data (str) && trans.get_data (attrs)) {
                        SCIM_DEBUG_IMENGINE(3) << "  update_aux_string ()\n";
                        update_aux_string (str, attrs);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE:
                {
                    CommonLookupTable table;
                    if (trans.get_data (table)) {
                        SCIM_DEBUG_IMENGINE(3) << "  update_lookup_table ()\n";
                        update_lookup_table (table);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_COMMIT_STRING:
                {
                    WideString str;
                    if (trans.get_data (str)) {
                        SCIM_DEBUG_IMENGINE(3) << "  commit_string ()\n";
                        commit_string (str);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_FORWARD_KEY_EVENT:
                {
                    KeyEvent key;
                    if (trans.get_data (key)) {
                        SCIM_DEBUG_IMENGINE(3) << "  forward_key_event ()\n";
                        forward_key_event (key);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_REGISTER_PROPERTIES:
                {
                    PropertyList proplist;
                    if (trans.get_data (proplist)) {
                        SCIM_DEBUG_IMENGINE(3) << "  register_properties ()\n";

                        // Load icon files of these properties from remote SocketFrontEnd.
                        for (PropertyList::iterator it = proplist.begin (); it != proplist.end (); ++it)
                            it->set_icon (global->load_icon (it->get_icon ()));

                        register_properties (proplist);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_UPDATE_PROPERTY:
                {
                    Property prop;
                    if (trans.get_data (prop)) {
                        SCIM_DEBUG_IMENGINE(3) << "  update_property ()\n";

                        // Load the icon file of this property from remote SocketFrontEnd.
                        prop.set_icon (global->load_icon (prop.get_icon ()));

                        update_property (prop);
                    }
                    break;
                }
                case SCIM_TRANS_CMD_BEEP:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  beep ()\n";
                    beep ();
                    break;
                }
                case SCIM_TRANS_CMD_START_HELPER:
                {
                    String helper_uuid;
                    if (trans.get_data (helper_uuid)) {
                        SCIM_DEBUG_IMENGINE(3) << "  start_helper (" << helper_uuid << ")\n";
                        start_helper (helper_uuid);
                    }
                    break; 
                }
                case SCIM_TRANS_CMD_STOP_HELPER:
                {
                    String helper_uuid;
                    if (trans.get_data (helper_uuid)) {
                        SCIM_DEBUG_IMENGINE(3) << "  stop_helper (" << helper_uuid << ")\n";
                        stop_helper (helper_uuid);
                    }
                    break; 
                }
                case SCIM_TRANS_CMD_SEND_HELPER_EVENT:
                {
                    String helper_uuid;
                    Transaction temp_trans;
                    if (trans.get_data (helper_uuid) && trans.get_data (temp_trans)) {
                        SCIM_DEBUG_IMENGINE(3) << "  send_helper_event (" << helper_uuid << ")\n";
                        send_helper_event (helper_uuid, temp_trans);
                    }
                    break; 
                }
                case SCIM_TRANS_CMD_OK:
                {
                    SCIM_DEBUG_IMENGINE(3) << "  ret = true\n";
                    ret = true;
                    break;
                }
                case SCIM_TRANS_CMD_GET_SURROUNDING_TEXT:
                {
                    WideString text;
                    int cursor;
                    uint32 maxlen_before;
                    uint32 maxlen_after;
                    Transaction temp_trans;
                    if (trans.get_data (maxlen_before) && trans.get_data (maxlen_after)) {
                        global->init_transaction (temp_trans);
                        if (get_surrounding_text (text, cursor, (int) maxlen_before, (int) maxlen_after)) {
                            temp_trans.put_command (SCIM_TRANS_CMD_GET_SURROUNDING_TEXT);
                            temp_trans.put_data (text);
                            temp_trans.put_data ((uint32) cursor);
                        } else {
                            temp_trans.put_command (SCIM_TRANS_CMD_FAIL);
                        }
                        global->send_transaction (temp_trans);
                    }
                    cont = true;
                    break;
                }
                case SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT:
                {
                    uint32 offset;
                    uint32 len;
                    Transaction temp_trans;
                    if (trans.get_data (offset) && trans.get_data (len)) {
                        global->init_transaction (temp_trans);
                        if (delete_surrounding_text ((int) offset, (int) len)) {
                            temp_trans.put_command (SCIM_TRANS_CMD_DELETE_SURROUNDING_TEXT);
                            temp_trans.put_command (SCIM_TRANS_CMD_OK);
                        } else {
                            temp_trans.put_command (SCIM_TRANS_CMD_FAIL);
                        }
                        global->send_transaction (temp_trans);
                    }
                    cont = true;
                    break;
                }
                default:
                    SCIM_DEBUG_IMENGINE(3) << "  Strange cmd: " << cmd << "\n";;
            }
        }
    } else {
        SCIM_DEBUG_IMENGINE(3) << "  Failed to get cmd: " << cmd << "\n";
    }

    SCIM_DEBUG_IMENGINE(2) << " End of Do transaction\n";

    return cont;
}

void
SocketInstance::reconnect_callback (void)
{
    m_peer_id = m_factory->create_peer_instance (get_encoding ());
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
