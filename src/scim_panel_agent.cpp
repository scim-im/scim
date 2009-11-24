/** @file scim_panel.cpp
 *  @brief Implementation of class PanelAgent.
 */

/*
 * Smart Common Input Method
 * 
 * Copyright (c) 2005 James Su <suzhe@tsinghua.org.cn>
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
 * $Id: scim_panel_agent.cpp,v 1.8.2.1 2006/01/09 14:32:18 suzhe Exp $
 *
 */

#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS
#define Uses_SCIM_PANEL_AGENT
#define Uses_SCIM_HELPER
#define Uses_SCIM_SOCKET
#define Uses_SCIM_EVENT

#define SCIM_KEYBOARD_ICON_FILE            (SCIM_ICONDIR "/keyboard.png")

#include "scim_private.h"
#include "scim.h"
#include "scim_stl_map.h"

namespace scim {

typedef Signal0<void>
        PanelAgentSignalVoid;

typedef Signal1<void, int>
        PanelAgentSignalInt;

typedef Signal1<void, const String &>
        PanelAgentSignalString;

typedef Signal1<void, const PanelFactoryInfo &>
        PanelAgentSignalFactoryInfo;

typedef Signal1<void, const std::vector <PanelFactoryInfo> &>
        PanelAgentSignalFactoryInfoVector;

typedef Signal1<void, const LookupTable &>
        PanelAgentSignalLookupTable;

typedef Signal1<void, const Property &>
        PanelAgentSignalProperty;

typedef Signal1<void, const PropertyList &>
        PanelAgentSignalPropertyList;

typedef Signal2<void, int, int>
        PanelAgentSignalIntInt;

typedef Signal2<void, int, const Property &>
        PanelAgentSignalIntProperty;

typedef Signal2<void, int, const PropertyList &>
        PanelAgentSignalIntPropertyList;

typedef Signal2<void, int, const HelperInfo &>
        PanelAgentSignalIntHelperInfo;

typedef Signal2<void, const String &, const AttributeList &>
        PanelAgentSignalAttributeString;

enum ClientType {
    UNKNOWN_CLIENT,
    FRONTEND_CLIENT,
    HELPER_CLIENT,
    PANELCONTROL_CLIENT
};


struct ClientInfo {
    uint32       key;
    ClientType   type;
    int          awaitedTransCommand;
};

struct HelperClientStub {
    int id;
    int ref;

    HelperClientStub (int i = 0, int r = 0) : id (i), ref (r) { }
};

#if SCIM_USE_STL_EXT_HASH_MAP
typedef __gnu_cxx::hash_map <int, ClientInfo, __gnu_cxx::hash <int> >       ClientRepository;
typedef __gnu_cxx::hash_map <int, HelperInfo, __gnu_cxx::hash <int> >       HelperInfoRepository;
typedef __gnu_cxx::hash_map <uint32, String, __gnu_cxx::hash <unsigned int> > ClientContextUUIDRepository;
typedef __gnu_cxx::hash_map <String, HelperClientStub, scim_hash_string>    HelperClientIndex;
typedef __gnu_cxx::hash_map <String, std::vector < std::pair <uint32, String> >, scim_hash_string>    StartHelperICIndex;
#elif SCIM_USE_STL_HASH_MAP
typedef std::hash_map <int, ClientInfo, std::hash <int> >                   ClientRepository;
typedef std::hash_map <int, HelperInfo, std::hash <int> >                   HelperInfoRepository;
typedef std::hash_map <uint32, String, std::hash <unsigned int> >           ClientContextUUIDRepository;
typedef std::hash_map <String, HelperClientStub, scim_hash_string>          HelperClientIndex;
typedef std::hash_map <String, std::vector < std::pair <uint32, String> >, scim_hash_string>          StartHelperICIndex;
#else
typedef std::map <int, ClientInfo>                                          ClientRepository;
typedef std::map <int, HelperInfo>                                          HelperInfoRepository;
typedef std::map <uint32, String>                                           ClientContextUUIDRepository;
typedef std::map <String, HelperClientStub>                                 HelperClientIndex;
typedef std::map <String, std::vector < std::pair <uint32, String> > >                                StartHelperICIndex;
#endif

static uint32
get_helper_ic (int client, uint32 context)
{
    return (uint32) (client & 0xFFFF) | ((context & 0x7FFF) << 16);
}

static void 
get_imengine_client_context (uint32 helper_ic, int &client, uint32 &context)
{
    client   = (int) (helper_ic & 0xFFFF);
    context  = ((helper_ic >> 16) & 0x7FFF);
}

//==================================== PanelAgent ===========================
class PanelAgent::PanelAgentImpl
{
    bool                                m_should_exit;
    bool                                m_should_resident;

    int                                 m_current_screen;

    String                              m_config_name;
    String                              m_display_name;

    int                                 m_socket_timeout;
    String                              m_socket_address;
    SocketServer                        m_socket_server;

    Transaction                         m_send_trans;
    Transaction                         m_recv_trans;
    Transaction                         m_nest_trans;

    int                                 m_current_socket_client;
    uint32                              m_current_client_context;
    String                              m_current_context_uuid;

    int                                 m_last_socket_client;
    uint32                              m_last_client_context;
    String                              m_last_context_uuid;

    ClientRepository                    m_client_repository;
    HelperInfoRepository                m_helper_info_repository;
    HelperClientIndex                   m_helper_client_index;

    StartHelperICIndex                  m_start_helper_ic_index;

    ClientContextUUIDRepository         m_client_context_uuids;
    
    PanelFactoryInfo 					m_currentFactoryInfo;
    PanelFactoryInfo 					m_defaultFactoryInfo;

    HelperManager                       m_helper_manager;

    PanelAgentSignalVoid                m_signal_reload_config;
    PanelAgentSignalVoid                m_signal_turn_on;
    PanelAgentSignalVoid                m_signal_turn_off;
    PanelAgentSignalInt                 m_signal_update_screen;
    PanelAgentSignalIntInt              m_signal_update_spot_location;
    PanelAgentSignalFactoryInfo         m_signal_update_factory_info;
    PanelAgentSignalString              m_signal_show_help;
    PanelAgentSignalFactoryInfoVector   m_signal_show_factory_menu;
    PanelAgentSignalVoid                m_signal_show_preedit_string;
    PanelAgentSignalVoid                m_signal_show_aux_string;
    PanelAgentSignalVoid                m_signal_show_lookup_table;
    PanelAgentSignalVoid                m_signal_hide_preedit_string;
    PanelAgentSignalVoid                m_signal_hide_aux_string;
    PanelAgentSignalVoid                m_signal_hide_lookup_table;
    PanelAgentSignalAttributeString     m_signal_update_preedit_string;
    PanelAgentSignalInt                 m_signal_update_preedit_caret;
    PanelAgentSignalAttributeString     m_signal_update_aux_string;
    PanelAgentSignalLookupTable         m_signal_update_lookup_table;
    PanelAgentSignalPropertyList        m_signal_register_properties;
    PanelAgentSignalProperty            m_signal_update_property;
    PanelAgentSignalIntPropertyList     m_signal_register_helper_properties;
    PanelAgentSignalIntProperty         m_signal_update_helper_property;
    PanelAgentSignalIntHelperInfo       m_signal_register_helper;
    PanelAgentSignalInt                 m_signal_remove_helper;

    PanelAgentSignalVoid                m_signal_transaction_start;
    PanelAgentSignalVoid                m_signal_transaction_end;

    PanelAgentSignalVoid                m_signal_lock;
    PanelAgentSignalVoid                m_signal_unlock;

public:
    PanelAgentImpl ()
        : m_should_exit (false),
          m_should_resident (false),
          m_current_screen (0),
          m_socket_timeout (scim_get_default_socket_timeout ()),
          m_current_socket_client (-1), m_current_client_context (0),
          m_last_socket_client (-1), m_last_client_context (0),
          m_defaultFactoryInfo (PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE))),
          m_currentFactoryInfo (PanelFactoryInfo (String (""), String (_("English/Keyboard")), String ("C"), String (SCIM_KEYBOARD_ICON_FILE)))
    {
        m_socket_server.signal_connect_accept (slot (this, &PanelAgentImpl::socket_accept_callback));
        m_socket_server.signal_connect_receive (slot (this, &PanelAgentImpl::socket_receive_callback));
        m_socket_server.signal_connect_exception (slot (this, &PanelAgentImpl::socket_exception_callback));
    }

    bool initialize (const String &config, const String &display, bool resident)
    {
        m_config_name = config;
        m_display_name = display;
        m_should_resident = resident;

        m_socket_address = scim_get_default_panel_socket_address (display);

        m_socket_server.shutdown ();

        return m_socket_server.create (SocketAddress (m_socket_address));
    }

    bool valid (void) const
    {
        return m_socket_server.valid ();
    }

public:
    bool run (void)
    {
        SCIM_DEBUG_MAIN (1) << "PanelAgent::run ()\n";

        return m_socket_server.run ();
    }

    void stop (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::stop ()\n";

        lock ();
        m_should_exit = true;
        unlock ();

        SocketClient  client;

        if (client.connect (SocketAddress (m_socket_address))) {
            client.close ();
        }
    }

    int get_helper_list     (std::vector <HelperInfo> & helpers) const
    {
        SCIM_DEBUG_MAIN (1) << "PanelAgent::get_helper_list ()\n";

        helpers.clear ();

        unsigned int num = m_helper_manager.number_of_helpers ();
        HelperInfo info;

        SCIM_DEBUG_MAIN (2) << "Found " << num << " Helper objects\n";

        for (unsigned int i = 0; i < num; ++i) {
            SCIM_DEBUG_MAIN (3) << "Helper " << i << " : " << info.uuid << " : " << info.name << " : " 
                                << ((info.option & SCIM_HELPER_STAND_ALONE) ? "SA " : "")
                                << ((info.option & SCIM_HELPER_AUTO_START) ? "AS " : "")
                                << ((info.option & SCIM_HELPER_AUTO_RESTART) ? "AR " : "") << "\n";

            if (m_helper_manager.get_helper_info (i, info) && info.uuid.length () &&
                (info.option & SCIM_HELPER_STAND_ALONE))
                helpers.push_back (info);
        }

        return helpers.size ();
    }

    bool move_preedit_caret             (uint32         position)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::move_preedit_caret (" << position << ")\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_MOVE_PREEDIT_CARET);
            m_send_trans.put_data ((uint32) position);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool request_help                   (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::request_help ()\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_REQUEST_HELP);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool request_factory_menu           (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::request_factory_menu ()\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_REQUEST_FACTORY_MENU);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool change_factory                 (const String  &uuid)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::change_factory (" << uuid << ")\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_CHANGE_FACTORY);
            m_send_trans.put_data (uuid);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool select_candidate               (uint32         item)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::select_candidate (" << item << ")\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_SELECT_CANDIDATE);
            m_send_trans.put_data ((uint32)item);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool lookup_table_page_up           (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::lookup_table_page_up ()\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_UP);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool lookup_table_page_down         (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::lookup_table_page_down ()\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_DOWN);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool update_lookup_table_page_size  (uint32         size)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::update_lookup_table_page_size (" << size << ")\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE_PAGE_SIZE);
            m_send_trans.put_data (size);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool trigger_property               (const String  &property)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::trigger_property (" << property << ")\n";

        int client;
        uint32 context;

        lock ();

        get_focused_context (client, context);

        if (client >= 0) {
            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
            m_send_trans.put_data ((uint32) context);
            m_send_trans.put_command (SCIM_TRANS_CMD_TRIGGER_PROPERTY);
            m_send_trans.put_data (property);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0;
    }

    bool trigger_helper_property        (int            client,
                                         const String  &property)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::trigger_helper_property (" << client << "," << property << ")\n";

        lock ();

        ClientInfo info = socket_get_client_info (client);

        if (client >= 0 && info.type == HELPER_CLIENT) {
            int fe_client;
            uint32 fe_context;
            String fe_uuid;

            fe_uuid = get_focused_context (fe_client, fe_context); 

            Socket client_socket (client);
            m_send_trans.clear ();
            m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);

            // FIXME: We presume that client and context are both less than 65536.
            // Hopefully, it should be true in any UNIXs.
            // So it's ok to combine client and context into one uint32.
            m_send_trans.put_data (get_helper_ic (fe_client, fe_context));
            m_send_trans.put_data (fe_uuid);
            m_send_trans.put_command (SCIM_TRANS_CMD_TRIGGER_PROPERTY);
            m_send_trans.put_data (property);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();

        return client >= 0 && info.type == HELPER_CLIENT;
    }

    bool start_helper                   (const String  &uuid)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::start_helper (" << uuid << ")\n";

        lock ();

        HelperClientIndex::iterator it = m_helper_client_index.find (uuid);

        if (it == m_helper_client_index.end ())
            m_helper_manager.run_helper (uuid, m_config_name, m_display_name);

        unlock ();

        return true;
    }

    bool reload_config                  (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::reload_config ()\n";

        lock ();

        m_send_trans.clear ();
        m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
        m_send_trans.put_command (SCIM_TRANS_CMD_RELOAD_CONFIG);

        for (ClientRepository::iterator it = m_client_repository.begin ();
             it != m_client_repository.end (); ++it) {
            Socket client_socket (it->first);
            m_send_trans.write_to_socket (client_socket);
        }

        unlock ();
        return true;
    }

    bool exit (void)
    {
        SCIM_DEBUG_MAIN(1) << "PanelAgent::exit ()\n";

        lock ();

        m_send_trans.clear ();
        m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
        m_send_trans.put_command (SCIM_TRANS_CMD_EXIT);
 
        for (ClientRepository::iterator it = m_client_repository.begin ();
             it != m_client_repository.end (); ++it) {
            Socket client_socket (it->first);
            m_send_trans.write_to_socket (client_socket);
        }
 
        unlock ();

        stop ();

        return true;
    }

    Connection signal_connect_reload_config              (PanelAgentSlotVoid                *slot)
    {
        return m_signal_reload_config.connect (slot);
    }

    Connection signal_connect_turn_on                    (PanelAgentSlotVoid                *slot)
    {
        return m_signal_turn_on.connect (slot);
    }

    Connection signal_connect_turn_off                   (PanelAgentSlotVoid                *slot)
    {
        return m_signal_turn_off.connect (slot);
    }

    Connection signal_connect_update_screen              (PanelAgentSlotInt                 *slot)
    {
        return m_signal_update_screen.connect (slot);
    }

    Connection signal_connect_update_spot_location       (PanelAgentSlotIntInt              *slot)
    {
        return m_signal_update_spot_location.connect (slot);
    }

    Connection signal_connect_update_factory_info        (PanelAgentSlotFactoryInfo         *slot)
    {
        return m_signal_update_factory_info.connect (slot);
    }

    Connection signal_connect_show_help                  (PanelAgentSlotString              *slot)
    {
        return m_signal_show_help.connect (slot);
    }

    Connection signal_connect_show_factory_menu          (PanelAgentSlotFactoryInfoVector   *slot)
    {
        return m_signal_show_factory_menu.connect (slot);
    }

    Connection signal_connect_show_preedit_string        (PanelAgentSlotVoid                *slot)
    {
        return m_signal_show_preedit_string.connect (slot);
    }

    Connection signal_connect_show_aux_string            (PanelAgentSlotVoid                *slot)
    {
        return m_signal_show_aux_string.connect (slot);
    }

    Connection signal_connect_show_lookup_table          (PanelAgentSlotVoid                *slot)
    {
        return m_signal_show_lookup_table.connect (slot);
    }

    Connection signal_connect_hide_preedit_string        (PanelAgentSlotVoid                *slot)
    {
        return m_signal_hide_preedit_string.connect (slot);
    }

    Connection signal_connect_hide_aux_string            (PanelAgentSlotVoid                *slot)
    {
        return m_signal_hide_aux_string.connect (slot);
    }

    Connection signal_connect_hide_lookup_table          (PanelAgentSlotVoid                *slot)
    {
        return m_signal_hide_lookup_table.connect (slot);
    }

    Connection signal_connect_update_preedit_string      (PanelAgentSlotAttributeString     *slot)
    {
        return m_signal_update_preedit_string.connect (slot);
    }

    Connection signal_connect_update_preedit_caret       (PanelAgentSlotInt                 *slot)
    {
        return m_signal_update_preedit_caret.connect (slot);
    }

    Connection signal_connect_update_aux_string          (PanelAgentSlotAttributeString     *slot)
    {
        return m_signal_update_aux_string.connect (slot);
    }

    Connection signal_connect_update_lookup_table        (PanelAgentSlotLookupTable         *slot)
    {
        return m_signal_update_lookup_table.connect (slot);
    }

    Connection signal_connect_register_properties        (PanelAgentSlotPropertyList        *slot)
    {
        return m_signal_register_properties.connect (slot);
    }

    Connection signal_connect_update_property            (PanelAgentSlotProperty            *slot)
    {
        return m_signal_update_property.connect (slot);
    }

    Connection signal_connect_register_helper_properties (PanelAgentSlotIntPropertyList     *slot)
    {
        return m_signal_register_helper_properties.connect (slot);
    }

    Connection signal_connect_update_helper_property     (PanelAgentSlotIntProperty         *slot)
    {
        return m_signal_update_helper_property.connect (slot);
    }

    Connection signal_connect_register_helper            (PanelAgentSlotIntHelperInfo       *slot)
    {
        return m_signal_register_helper.connect (slot);
    }

    Connection signal_connect_remove_helper              (PanelAgentSlotInt                 *slot)
    {
        return m_signal_remove_helper.connect (slot);
    }

    Connection signal_connect_transaction_start    (PanelAgentSlotVoid                *slot)
    {
        return m_signal_transaction_start.connect (slot);
    }

    Connection signal_connect_transaction_end      (PanelAgentSlotVoid                *slot)
    {
        return m_signal_transaction_end.connect (slot);
    }

    Connection signal_connect_lock                       (PanelAgentSlotVoid                *slot)
    {
        return m_signal_lock.connect (slot);
    }

    Connection signal_connect_unlock                     (PanelAgentSlotVoid                *slot)
    {
        return m_signal_unlock.connect (slot);
    }

private:
    bool socket_check_client_connection         (const Socket   &client)
    {
        SCIM_DEBUG_MAIN (3) << "PanelAgent::socket_check_client_connection (" << client.get_id () << ")\n";
 
        unsigned char buf [sizeof(uint32)];
 
        int nbytes = client.read_with_timeout (buf, sizeof(uint32), m_socket_timeout);
 
        if (nbytes == sizeof (uint32))
            return true;
 
        if (nbytes < 0) {
            SCIM_DEBUG_MAIN (4) << "Error occurred when reading socket: " << client.get_error_message () << ".\n";
        } else {
            SCIM_DEBUG_MAIN (4) << "Timeout when reading socket.\n";
        }
 
        return false;
    }

    void socket_accept_callback                 (SocketServer   *server,
                                                 const Socket   &client)
    {
        SCIM_DEBUG_MAIN (2) << "PanelAgent::socket_accept_callback (" << client.get_id () << ")\n";

        lock ();
        if (m_should_exit) {
            SCIM_DEBUG_MAIN (3) << "Exit Socket Server Thread.\n";
            server->shutdown ();
        }
        unlock ();
    }

    void socket_receive_callback                (SocketServer   *server,
                                                 const Socket   &client)
    {
        int     client_id = client.get_id ();
        int     cmd     = 0;
        uint32  key     = 0;
        uint32  context = 0;
        String  uuid;
        bool    current = false;
        bool    last    = false;
 
        ClientInfo client_info;
 
        SCIM_DEBUG_MAIN (1) << "PanelAgent::socket_receive_callback (" << client_id << ")\n";
 
        // If the connection is closed then close this client.
        if (!socket_check_client_connection (client)) {
            socket_close_connection (server, client);
            return;
        }
 
        client_info = socket_get_client_info (client_id);
 
        // If it's a new client, then request to open the connection first.
        if (client_info.type == UNKNOWN_CLIENT) {
            socket_open_connection (server, client);
            return;
        }
 
        // If can not read the transaction,
        // or the transaction is not started with SCIM_TRANS_CMD_REQUEST,
        // or the key is mismatch,
        // just return.
        if (!m_recv_trans.read_from_socket (client, m_socket_timeout) ||
            !m_recv_trans.get_command (cmd) || cmd != SCIM_TRANS_CMD_REQUEST ||
            !m_recv_trans.get_data (key)    || key != (uint32) client_info.key)
            return;
 
        if (client_info.type == FRONTEND_CLIENT) {
            if (m_recv_trans.get_data (context)) {
                SCIM_DEBUG_MAIN (1) << "PanelAgent::FrontEnd Client, context = " << context << "\n";
                socket_transaction_start();
                while (m_recv_trans.get_command (cmd)) {
                    SCIM_DEBUG_MAIN (3) << "PanelAgent::cmd = " << cmd << "\n";

                    if (cmd == SCIM_TRANS_CMD_PANEL_REGISTER_INPUT_CONTEXT) {
                        if (m_recv_trans.get_data (uuid)) {
                            SCIM_DEBUG_MAIN (2) << "PanelAgent::register_input_context (" << client_id << "," << "," << context << "," << uuid << ")\n";
                            uint32 ctx = get_helper_ic (client_id, context);
                            m_client_context_uuids [ctx] = uuid;
                        }
                        continue;
                    }

                    if (cmd == SCIM_TRANS_CMD_PANEL_REMOVE_INPUT_CONTEXT) {
                        uint32 ctx = get_helper_ic (client_id, context);
                        m_client_context_uuids.erase (ctx);
                        continue;
                    }

                    if (cmd == SCIM_TRANS_CMD_FOCUS_IN) {
                        if (m_recv_trans.get_data (uuid)) {
                            SCIM_DEBUG_MAIN (2) << "PanelAgent::focus_in (" << client_id << "," << "," << context << "," << uuid << ")\n";
                            lock (); 
                            if (m_current_socket_client >= 0) {
                                m_last_socket_client  = m_current_socket_client;
                                m_last_client_context = m_current_client_context;
                                m_last_context_uuid   = m_current_context_uuid;
                            }
                            m_current_socket_client  = client_id;
                            m_current_client_context = context;
                            m_current_context_uuid   = uuid;
                            unlock ();
                        }
                        continue;
                    }

                    current = last = false;
                    uuid.clear ();

                    // Get the context uuid from the client context registration table.
                    {
                        ClientContextUUIDRepository::iterator it = m_client_context_uuids.find (get_helper_ic (client_id, context));
                        if (it != m_client_context_uuids.end ())
                            uuid = it->second;
                    }

                    if (m_current_socket_client == client_id && m_current_client_context == context) {
                        current = true;
                        if (!uuid.length ()) uuid = m_current_context_uuid;
                    } else if (m_last_socket_client == client_id && m_last_client_context == context) {
                        last = true;
                        if (!uuid.length ()) uuid = m_last_context_uuid;
                    }

                    // Skip to the next command and continue, if it's not current or last focused.
                    if (!uuid.length ()) {
                        SCIM_DEBUG_MAIN (3) << "PanelAgent:: Couldn't find context uuid.\n";
                        while (m_recv_trans.get_data_type () != SCIM_TRANS_DATA_COMMAND && m_recv_trans.get_data_type () != SCIM_TRANS_DATA_UNKNOWN)
                            m_recv_trans.skip_data ();
                        continue;
                    }

                    if (cmd == SCIM_TRANS_CMD_START_HELPER) {
                        socket_start_helper (client_id, context, uuid);
                        continue;
                    }
                    if (cmd == SCIM_TRANS_CMD_SEND_HELPER_EVENT) {
                        socket_send_helper_event (client_id, context, uuid);
                        continue;
                    }
                    if (cmd == SCIM_TRANS_CMD_STOP_HELPER) {
                        socket_stop_helper (client_id, context, uuid);
                        continue;
                    }
					
                   if (cmd == SCIM_TRANS_CMD_PANEL_SHOW_FACTORY_MENU)
                        socket_show_factory_menu ();
 
                    // If it's not focused, just continue.
                    if ((!current && !last) || (last && m_current_socket_client >= 0)) {
                        SCIM_DEBUG_MAIN (3) << "PanelAgent::Not current focused.\n";
                        while (m_recv_trans.get_data_type () != SCIM_TRANS_DATA_COMMAND && m_recv_trans.get_data_type () != SCIM_TRANS_DATA_UNKNOWN)
                            m_recv_trans.skip_data ();
                        continue;
                    }
 
                    // Client must focus in before do any other things.
                    if (cmd == SCIM_TRANS_CMD_PANEL_TURN_ON)
                        socket_turn_on ();
                    else if (cmd == SCIM_TRANS_CMD_PANEL_TURN_OFF)
                        socket_turn_off ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_SCREEN)
                        socket_update_screen ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_SPOT_LOCATION)
                        socket_update_spot_location ();
                    else if (cmd == SCIM_TRANS_CMD_PANEL_UPDATE_FACTORY_INFO)
                        socket_update_factory_info ();
                    else if (cmd == SCIM_TRANS_CMD_SHOW_PREEDIT_STRING)
                        socket_show_preedit_string ();
                    else if (cmd == SCIM_TRANS_CMD_SHOW_AUX_STRING)
                        socket_show_aux_string ();
                    else if (cmd == SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE)
                        socket_show_lookup_table ();
                    else if (cmd == SCIM_TRANS_CMD_HIDE_PREEDIT_STRING)
                        socket_hide_preedit_string ();
                    else if (cmd == SCIM_TRANS_CMD_HIDE_AUX_STRING)
                        socket_hide_aux_string ();
                    else if (cmd == SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE)
                        socket_hide_lookup_table ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_PREEDIT_STRING)
                        socket_update_preedit_string ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_PREEDIT_CARET)
                        socket_update_preedit_caret ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_AUX_STRING)
                        socket_update_aux_string ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE)
                        socket_update_lookup_table ();
                    else if (cmd == SCIM_TRANS_CMD_REGISTER_PROPERTIES)
                        socket_register_properties ();
                    else if (cmd == SCIM_TRANS_CMD_UPDATE_PROPERTY)
                        socket_update_property ();
                    else if (cmd == SCIM_TRANS_CMD_PANEL_SHOW_HELP)
                        socket_show_help ();
                    else if (cmd == SCIM_TRANS_CMD_FOCUS_OUT) {
                        SCIM_DEBUG_MAIN (2) << "PanelAgent::focus_out (" << client_id << "," << "," << context << ")\n";
                        lock ();
                        if (m_current_socket_client >= 0) {
                            m_last_socket_client  = m_current_socket_client;
                            m_last_client_context = m_current_client_context;
                            m_last_context_uuid   = m_current_context_uuid;
                        }
                        m_current_socket_client  = -1;
                        m_current_client_context = 0;
                        m_current_context_uuid   = String ("");
                        unlock ();
                        socket_turn_off ();
                    }
                }
                socket_transaction_end();
            }
        } else if (client_info.type == HELPER_CLIENT) {
            socket_transaction_start();
            while (m_recv_trans.get_command (cmd)) {
                if (cmd == SCIM_TRANS_CMD_PANEL_REGISTER_HELPER) {    
                    socket_helper_register_helper (client_id);
                } else if (cmd == SCIM_TRANS_CMD_COMMIT_STRING) {
                    socket_helper_commit_string (client_id);
                } else if (cmd == SCIM_TRANS_CMD_PROCESS_KEY_EVENT ||
                           cmd == SCIM_TRANS_CMD_PANEL_SEND_KEY_EVENT) {
                    socket_helper_send_key_event (client_id);
                } else if (cmd == SCIM_TRANS_CMD_FORWARD_KEY_EVENT) {
                    socket_helper_forward_key_event (client_id);
                } else if (cmd == SCIM_TRANS_CMD_PANEL_SEND_IMENGINE_EVENT) {
                    socket_helper_send_imengine_event (client_id);
                } else if (cmd == SCIM_TRANS_CMD_REGISTER_PROPERTIES) {
                    socket_helper_register_properties (client_id);
                } else if (cmd == SCIM_TRANS_CMD_UPDATE_PROPERTY) {
                    socket_helper_update_property (client_id);
                } else if (cmd == SCIM_TRANS_CMD_RELOAD_CONFIG) {
                    reload_config ();
                    m_signal_reload_config ();
                }
            }
            socket_transaction_end();
        } else if (client_info.type == PANELCONTROL_CLIENT) {
            SCIM_DEBUG_MAIN (1) << "PanelAgent::PanelController Client\n";
            socket_transaction_start();
            while (m_recv_trans.get_command (cmd)) {
				SCIM_DEBUG_MAIN (3) << "PanelAgent::cmd = " << cmd << "\n";
				
				if(previous_command_is_outstanding_for_client(client_id))
                		return; //if the response to a previous command is outstanding we ignore the current command 
					
                if (cmd == SCIM_TRANS_CMD_CONTROLLER_REQUEST_FACTORY_MENU) {
                	register_awaited_command_for_client(client_id, SCIM_TRANS_CMD_PANEL_SHOW_FACTORY_MENU);
                    socket_panelcontroller_request_factory_menu ();
                }
                if (cmd == SCIM_TRANS_CMD_CONTROLLER_CHANGE_FACTORY) {
                	register_awaited_command_for_client(client_id, SCIM_TRANS_CMD_PANEL_UPDATE_FACTORY_INFO);
                    socket_panelcontroller_change_factory ();
                }
                if (cmd == SCIM_TRANS_CMD_CONTROLLER_GET_CURRENT_FACTORY) {
                	register_awaited_command_for_client(client_id, SCIM_TRANS_CMD_PANEL_RETURN_CURRENT_FACTORY_INFO);
                    socket_panelcontroller_get_current_factory (client_id);
                }
                if (cmd == SCIM_TRANS_CMD_CONTROLLER_GET_CURRENT_CONTEXT) {
					register_awaited_command_for_client(client_id, SCIM_TRANS_CMD_PANEL_RETURN_CURRENT_CONTEXT);
                    socket_panelcontroller_get_current_frontend_client_and_context (client_id);
                }
            }
            socket_transaction_end();
        }
    }
    
    bool previous_command_is_outstanding_for_client(int client_id){
    	ClientRepository::iterator it = m_client_repository.find (client_id);
		return (it->second.awaitedTransCommand != 0);
    }
    
    void register_awaited_command_for_client(int client_id, int cmd){
    	ClientRepository::iterator it = m_client_repository.find (client_id);
		it->second.awaitedTransCommand = cmd;
    }
    
    int expected_command_for_client(int client_id){
    	ClientRepository::iterator it = m_client_repository.find (client_id);
		return it->second.awaitedTransCommand;
    }
    
    void socket_panelcontroller_get_current_frontend_client_and_context(int client_id)
    {
    	SCIM_DEBUG_MAIN (2) << "PanelAgent::socket_panelcontroller_get_current_frontend_client_and_context ()\n";
        inform_waiting_client_of_current_context(client_id);
    }
	
	void socket_panelcontroller_request_factory_menu()
	{
		SCIM_DEBUG_MAIN (2) << "PanelAgent::socket_panelcontroller_request_factory_menu ()\n";
        request_factory_menu();
	}
	
	void socket_panelcontroller_change_factory ()
	{
		String requested_uuid;
		m_recv_trans.get_data(requested_uuid);
		SCIM_DEBUG_MAIN (2) << "PanelAgent::socket_panelcontroller_change_factory ()" << requested_uuid << "\n";
		change_factory(requested_uuid);
	}
	
	void socket_panelcontroller_get_current_factory	(int client_id)
	{
		SCIM_DEBUG_MAIN (2) << "PanelAgent::socket_panelcontroller_get_current_factory ()\n";
		inform_waiting_client_of_current_factory(client_id);
	}
	
    void socket_exception_callback              (SocketServer   *server,
                                                 const Socket   &client)
    {
        SCIM_DEBUG_MAIN (2) << "PanelAgent::socket_exception_callback (" << client.get_id () << ")\n";

        socket_close_connection (server, client);
    }

    bool socket_open_connection                 (SocketServer   *server,
                                                 const Socket   &client)
    {
        SCIM_DEBUG_MAIN (3) << "PanelAgent::socket_open_connection (" << client.get_id () << ")\n";

        uint32 key;
        String type = scim_socket_accept_connection (key,
                                                     String ("Panel"),
                                                     String ("FrontEnd,Helper,PanelController"),
                                                     client,
                                                     m_socket_timeout);

        if (type.length ()) {
            ClientInfo info;
            info.key = key;
            info.awaitedTransCommand = 0;
			if		(type=="FrontEnd") 	{info.type = FRONTEND_CLIENT;}
			else if	(type=="Helper") 	{info.type = HELPER_CLIENT;}
			else 						{info.type = PANELCONTROL_CLIENT;}
            SCIM_DEBUG_MAIN (4) << "Add client to repository. Type=" << type << " key=" << key << "\n";
            lock ();
            m_client_repository [client.get_id ()] = info;
            unlock ();
            return true;
        }

        SCIM_DEBUG_MAIN (4) << "Close client connection " << client.get_id () << "\n";
        server->close_connection (client);
        return false;
    }

    void socket_close_connection                (SocketServer   *server,
                                                 const Socket   &client)
    {
        SCIM_DEBUG_MAIN (3) << "PanelAgent::socket_close_connection (" << client.get_id () << ")\n";
 
        lock ();

        ClientInfo client_info = socket_get_client_info (client.get_id ());

        m_client_repository.erase (client.get_id ());
 
        server->close_connection (client);
 
        // Exit panel if there is no connected client anymore.
        if (client_info.type != UNKNOWN_CLIENT && m_client_repository.size () == 0 && !m_should_resident) {
            SCIM_DEBUG_MAIN (4) << "Exit Socket Server Thread.\n";
            server->shutdown ();
        }

        unlock ();

        if (client_info.type == FRONTEND_CLIENT) {
            SCIM_DEBUG_MAIN(4) << "It's a FrontEnd client.\n";
            // The focused client is closed.
            if (m_current_socket_client == client.get_id ()) {
 
                lock ();

                m_current_socket_client = -1;
                m_current_client_context = 0;
                m_current_context_uuid = String ("");
 
                unlock ();

                socket_transaction_start ();
                socket_turn_off ();
                socket_transaction_end ();
            }

            if (m_last_socket_client == client.get_id ()) {

                lock ();

                m_last_socket_client = -1;
                m_last_client_context = 0;
                m_last_context_uuid = String ("");

                unlock ();
            }
 
            // Erase all associated Client Context UUIDs.
            std::vector <uint32> ctx_list;
            ClientContextUUIDRepository::iterator it = m_client_context_uuids.begin ();
            for (; it != m_client_context_uuids.end (); ++it) {
                if ((it->first & 0xFFFF) == (client.get_id () & 0xFFFF))
                    ctx_list.push_back (it->first);
            }

            for (size_t i = 0; i < ctx_list.size (); ++i)
                m_client_context_uuids.erase (ctx_list [i]);

        } else if (client_info.type == HELPER_CLIENT) {
            SCIM_DEBUG_MAIN(4) << "It's a Helper client.\n";

            lock ();

            HelperInfoRepository::iterator hiit = m_helper_info_repository.find (client.get_id ());

            if (hiit != m_helper_info_repository.end ()) {
                bool restart = false;
                String uuid = hiit->second.uuid;
  
                HelperClientIndex::iterator it = m_helper_client_index.find (uuid);
                if ((hiit->second.option & SCIM_HELPER_AUTO_RESTART) && it->second.ref > 0)
                    restart = true;
  
                m_helper_client_index.erase (uuid);
                m_helper_info_repository.erase (hiit);
  
                if (restart)
                    m_helper_manager.run_helper (uuid, m_config_name, m_display_name);
            }

            unlock ();

            socket_transaction_start ();
            m_signal_remove_helper (client.get_id ());
            socket_transaction_end ();
        }
    }

    const ClientInfo & socket_get_client_info   (int client)
    {
        static ClientInfo null_client = { 0, UNKNOWN_CLIENT };

        ClientRepository::iterator it = m_client_repository.find (client);

        if (it != m_client_repository.end ())
            return it->second;

        return null_client;
    }

private:
    void socket_turn_on                         (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_turn_on ()\n";

        m_signal_turn_on ();
    }

    void socket_turn_off                        (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_turn_off ()\n";
		//inform_waiting_clients_of_factory_update(m_defaultFactoryInfo);	
		
        m_signal_turn_off ();
    }

    void socket_update_screen                   (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_screen ()\n";

        uint32 num;
        if (m_recv_trans.get_data (num) && ((int) num) != m_current_screen) {
            SCIM_DEBUG_MAIN(4) << "New Screen number = " << num << "\n";
            m_signal_update_screen ((int) num);
            helper_all_update_screen ((int) num);
            m_current_screen = (num);
        }
    }

    void socket_update_spot_location            (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_spot_location ()\n";

        uint32 x, y;
        if (m_recv_trans.get_data (x) && m_recv_trans.get_data (y)) {
            SCIM_DEBUG_MAIN(4) << "New Spot location x=" << x << " y=" << y << "\n";
            m_signal_update_spot_location ((int)x, (int)y);
            helper_all_update_spot_location ((int)x, (int)y);
        }
    }

    void socket_update_factory_info             (void)
    {	
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_factory_info ()\n";

        PanelFactoryInfo info;
        if (m_recv_trans.get_data (info.uuid) && m_recv_trans.get_data (info.name) &&
            m_recv_trans.get_data (info.lang) && m_recv_trans.get_data (info.icon)) {
            SCIM_DEBUG_MAIN(4) << "New Factory info uuid=" << info.uuid << " name=" << info.name << "\n";
            info.lang = scim_get_normalized_language (info.lang);
            m_currentFactoryInfo = info;            
            m_signal_update_factory_info (info);
            inform_waiting_clients_of_factory_update(info);
        }
    }
    
    bool inform_waiting_clients_of_factory_update(PanelFactoryInfo info){
    	bool message_was_forwarded = false;
		SCIM_DEBUG_MAIN (1) << "PanelAgent::Checking if message needs forwarding\n";
    	for(ClientRepository::iterator it = m_client_repository.begin ();
	    	it != m_client_repository.end (); ++it){
	   		if(it->second.awaitedTransCommand == SCIM_TRANS_CMD_PANEL_UPDATE_FACTORY_INFO){
	        	uint32 context = m_current_client_context;
	        	Socket client_socket (it->first);
					            
				m_send_trans.clear ();
				m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
				m_send_trans.put_data ((uint32) context);	//this is the REAL context. i.e. it is 0 when the desktop is focused
		    	m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_UPDATE_FACTORY_INFO);
		        m_send_trans.put_data (info.uuid);
		        m_send_trans.put_data (info.name);
		        m_send_trans.put_data (info.lang);
		        m_send_trans.put_data (info.icon);
		        m_send_trans.write_to_socket (client_socket);
			
				SCIM_DEBUG_MAIN (2) << "Forwarded message " << "SCIM_TRANS_CMD_PANEL_UPDATE_FACTORY_INFO" << "to " << it->first << "\n";
				it->second.awaitedTransCommand = 0;
				message_was_forwarded = true;
				break;	//client iteration
	        }
	    }
	    return message_was_forwarded;
    }
    
    void inform_waiting_client_of_current_context(int client_id){
    	SCIM_DEBUG_MAIN (1) << "PanelAgent::Checking if message needs forwarding\n";
    	uint32 context = m_current_client_context;
    	Socket client_socket (client_id);
    	
    	m_send_trans.clear ();
		m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
		m_send_trans.put_data ((uint32) context);	//this is the REAL context. i.e. it is 0 when the desktop is focused
    	m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_RETURN_CURRENT_CONTEXT);
		m_send_trans.put_data ((uint32) m_current_socket_client);  //this is the REAL client. i.e. it is -1 when the desktop is focused
		m_send_trans.put_data ((uint32) m_current_client_context); //this is the REAL context. i.e. it is 0 when the desktop is focused
        m_send_trans.write_to_socket (client_socket);
        m_client_repository[client_id].awaitedTransCommand = 0;
        SCIM_DEBUG_MAIN (2) << "Forwarded message " << "SCIM_TRANS_CMD_PANEL_RETURN_CURRENT_CONTEXT\n";
    }
    
    void inform_waiting_client_of_current_factory(int client_id){
    	SCIM_DEBUG_MAIN (1) << "PanelAgent::Checking if message needs forwarding\n";
    	uint32 context = m_current_client_context;
    	Socket client_socket (client_id);
    	
    	m_send_trans.clear ();
		m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
		m_send_trans.put_data ((uint32) context);	//this is the REAL context. i.e. it is 0 when the desktop is focused
    	m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_RETURN_CURRENT_FACTORY_INFO);
        m_send_trans.put_data (m_currentFactoryInfo.uuid);
        m_send_trans.put_data (m_currentFactoryInfo.name);
        m_send_trans.put_data (m_currentFactoryInfo.lang);
        m_send_trans.put_data (m_currentFactoryInfo.icon);
        m_send_trans.write_to_socket (client_socket);
        m_client_repository[client_id].awaitedTransCommand = 0;
        SCIM_DEBUG_MAIN (2) << "Forwarded message " << "SCIM_TRANS_CMD_PANEL_RETURN_CURRENT_FACTORY_INFO\n";
    }

    void socket_show_help                       (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_show_help ()\n";

        String help;
        if (m_recv_trans.get_data (help))
            m_signal_show_help (help);
    }

    void socket_show_factory_menu               (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_show_factory_menu ()\n";

		PanelFactoryInfo info;
        std::vector <PanelFactoryInfo> vec;
		
        while (m_recv_trans.get_data (info.uuid) && m_recv_trans.get_data (info.name) &&
               m_recv_trans.get_data (info.lang) && m_recv_trans.get_data (info.icon)) {
            info.lang = scim_get_normalized_language (info.lang);
            vec.push_back (info);
        }
        if (vec.size ()){
        	if(!inform_waiting_clients_of_factory_menu(vec))        	
        		m_signal_show_factory_menu (vec);
        }
    }
    
    bool inform_waiting_clients_of_factory_menu(std::vector <PanelFactoryInfo> menu){
    	bool message_was_forwarded = false;
		SCIM_DEBUG_MAIN (1) << "PanelAgent::Checking if message needs forwarding\n";
    	
		menu.push_back(m_defaultFactoryInfo);
    	for(ClientRepository::iterator it = m_client_repository.begin ();
	    	it != m_client_repository.end (); ++it){
	   		if(it->second.awaitedTransCommand == SCIM_TRANS_CMD_PANEL_SHOW_FACTORY_MENU){
	        	uint32 context = m_current_client_context;
	        	Socket client_socket (it->first);
					            
				m_send_trans.clear ();
				m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
				m_send_trans.put_data ((uint32) context);	//this is the REAL context. i.e. it is 0 when the desktop is focused
		        m_send_trans.put_command (SCIM_TRANS_CMD_PANEL_SHOW_FACTORY_MENU);
	            for (size_t i = 0; i < menu.size (); ++ i) {
	                m_send_trans.put_data (menu [i].uuid);
	                m_send_trans.put_data (menu [i].name);
	                m_send_trans.put_data (menu [i].lang);
	                m_send_trans.put_data (menu [i].icon);
	            }
		        m_send_trans.write_to_socket (client_socket);
			
				SCIM_DEBUG_MAIN (2) << "Forwarded message " << "SCIM_TRANS_CMD_PANEL_SHOW_FACTORY_MENU" << "to " << it->first << "\n";
				it->second.awaitedTransCommand = 0;
				message_was_forwarded = true;
				break;	//client iteration
	        }
	    }
	    return message_was_forwarded;
    }

    void socket_show_preedit_string             (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_show_preedit_string ()\n";
        m_signal_show_preedit_string ();
    }

    void socket_show_aux_string                 (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_show_aux_string ()\n";
        m_signal_show_aux_string ();
    }

    void socket_show_lookup_table               (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_show_lookup_table ()\n";
        m_signal_show_lookup_table ();
    }

    void socket_hide_preedit_string             (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_hide_preedit_string ()\n";
        m_signal_hide_preedit_string ();
    }

    void socket_hide_aux_string                 (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_hide_aux_string ()\n";
        m_signal_hide_aux_string ();
    }

    void socket_hide_lookup_table               (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_hide_lookup_table ()\n";
        m_signal_hide_lookup_table ();
    }

    void socket_update_preedit_string           (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_preedit_string ()\n";

        String str;
        AttributeList attrs;
        if (m_recv_trans.get_data (str) && m_recv_trans.get_data (attrs))
            m_signal_update_preedit_string (str, attrs);
    }

    void socket_update_preedit_caret            (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_preedit_caret ()\n";

        uint32 caret;
        if (m_recv_trans.get_data (caret))
            m_signal_update_preedit_caret ((int) caret);
    }

    void socket_update_aux_string               (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_aux_string ()\n";

        String str;
        AttributeList attrs;
        if (m_recv_trans.get_data (str) && m_recv_trans.get_data (attrs))
            m_signal_update_aux_string (str, attrs);
    }

    void socket_update_lookup_table             (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_lookup_table ()\n";

        CommonLookupTable table;
        if (m_recv_trans.get_data (table))
            m_signal_update_lookup_table (table);
    }

    void socket_register_properties             (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_register_properties ()\n";

        PropertyList properties;

        if (m_recv_trans.get_data (properties))
            m_signal_register_properties (properties);
    }

    void socket_update_property                 (void)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_update_property ()\n";

        Property property;
        if (m_recv_trans.get_data (property))
            m_signal_update_property (property);
    }

    void socket_start_helper                    (int client, uint32 context, const String &ic_uuid)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_start_helper ()\n";

        String uuid;
        if (m_recv_trans.get_data (uuid) && uuid.length ()) {
            HelperClientIndex::iterator it = m_helper_client_index.find (uuid);

            uint32 ic;
 
            lock ();

            ic      = get_helper_ic (client, context);
 
            SCIM_DEBUG_MAIN(5) << "Helper UUID =" << uuid << "  IC UUID =" << ic_uuid <<"\n";

            if (it == m_helper_client_index.end ()) {
                SCIM_DEBUG_MAIN(5) << "Run this Helper.\n";
                m_start_helper_ic_index [uuid].push_back (std::make_pair (ic, ic_uuid));
                m_helper_manager.run_helper (uuid, m_config_name, m_display_name);
            } else {
                SCIM_DEBUG_MAIN(5) << "Increase the Reference count.\n";
                Socket client_socket (it->second.id);
                m_send_trans.clear ();
                m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
                m_send_trans.put_data (ic);
                m_send_trans.put_data (ic_uuid);
                m_send_trans.put_command (SCIM_TRANS_CMD_HELPER_ATTACH_INPUT_CONTEXT);
                m_send_trans.write_to_socket (client_socket);
                ++ it->second.ref;
            }

            unlock ();
        }
    }

    void socket_stop_helper                     (int client, uint32 context, const String &ic_uuid)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_stop_helper ()\n";

        String uuid;
        if (m_recv_trans.get_data (uuid) && uuid.length ()) {
            HelperClientIndex::iterator it = m_helper_client_index.find (uuid);

            uint32 ic;
 
            lock ();

            ic      = get_helper_ic (client, context);
 
            SCIM_DEBUG_MAIN(5) << "Helper UUID =" << uuid << "  IC UUID =" << ic_uuid <<"\n";

            if (it != m_helper_client_index.end ()) {
                SCIM_DEBUG_MAIN(5) << "Decrase the Reference count.\n";
                -- it->second.ref;
                Socket client_socket (it->second.id);
                m_send_trans.clear ();
                m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
                m_send_trans.put_data (ic);
                m_send_trans.put_data (ic_uuid);
                m_send_trans.put_command (SCIM_TRANS_CMD_HELPER_DETACH_INPUT_CONTEXT);
                if (it->second.ref <= 0)
                    m_send_trans.put_command (SCIM_TRANS_CMD_EXIT);
                m_send_trans.write_to_socket (client_socket);
            }

            unlock ();
        }
    }

    void socket_send_helper_event               (int client, uint32 context, const String &ic_uuid)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_send_helper_event ()\n";

        String uuid;
        if (m_recv_trans.get_data (uuid) && m_recv_trans.get_data (m_nest_trans) &&
            uuid.length () && m_nest_trans.valid ()) {
            HelperClientIndex::iterator it = m_helper_client_index.find (uuid);
            if (it != m_helper_client_index.end ()) {
                Socket client_socket (it->second.id);
 
                lock ();

                m_send_trans.clear ();
                m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
 
                // FIXME: We presume that client and context are both less than 65536.
                // Hopefully, it should be true in any UNIXs.
                // So it's ok to combine client and context into one uint32.
                m_send_trans.put_data (get_helper_ic (client, context));
                m_send_trans.put_data (ic_uuid);
                m_send_trans.put_command (SCIM_TRANS_CMD_HELPER_PROCESS_IMENGINE_EVENT);
                m_send_trans.put_data (m_nest_trans);
                m_send_trans.write_to_socket (client_socket);

                unlock ();
            }
        }
    }

    void socket_helper_register_properties      (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_register_properties (" << client << ")\n";

        PropertyList properties;
        if (m_recv_trans.get_data (properties))
            m_signal_register_helper_properties (client, properties);
    }

    void socket_helper_update_property          (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_update_property (" << client << ")\n";

        Property property;
        if (m_recv_trans.get_data (property))
            m_signal_update_helper_property (client, property);
    }

    void socket_helper_send_imengine_event      (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_send_imengine_event (" << client << ")\n";

        uint32 target_ic;
        String target_uuid;

        HelperInfoRepository::iterator hiit = m_helper_info_repository.find (client);

        if (m_recv_trans.get_data (target_ic)    &&
            m_recv_trans.get_data (target_uuid)  &&
            m_recv_trans.get_data (m_nest_trans) &&
            m_nest_trans.valid ()                &&
            hiit != m_helper_info_repository.end ()) {

            int     target_client;
            uint32  target_context;

            get_imengine_client_context (target_ic, target_client, target_context);

            int     focused_client;
            uint32  focused_context;
            String  focused_uuid;
 
            focused_uuid = get_focused_context (focused_client, focused_context);
 
            if (target_ic == (uint32) (-1)) {
                target_client  = focused_client;
                target_context = focused_context;
            }
 
            if (target_uuid.length () == 0)
                target_uuid = focused_uuid;
 
            ClientInfo  client_info = socket_get_client_info (target_client);

            SCIM_DEBUG_MAIN(5) << "Target UUID = " << target_uuid << "  Focused UUId = " << focused_uuid << "\nTarget Client = " << target_client << "\n";

            if (client_info.type == FRONTEND_CLIENT) {
                Socket socket_client (target_client);
                lock ();
                m_send_trans.clear ();
                m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
                m_send_trans.put_data (target_context);
                m_send_trans.put_command (SCIM_TRANS_CMD_PROCESS_HELPER_EVENT);
                m_send_trans.put_data (target_uuid);
                m_send_trans.put_data (hiit->second.uuid);
                m_send_trans.put_data (m_nest_trans);
                m_send_trans.write_to_socket (socket_client);
                unlock ();
            }
        }
    }

    void socket_helper_key_event_op (int client, int cmd)
    {
        uint32 target_ic;
        String target_uuid;
        KeyEvent key;

        if (m_recv_trans.get_data (target_ic)    &&
            m_recv_trans.get_data (target_uuid)  && 
            m_recv_trans.get_data (key)          &&
            !key.empty ()) {

            int     target_client;
            uint32  target_context;
 
            get_imengine_client_context (target_ic, target_client, target_context);

            int     focused_client;
            uint32  focused_context;
            String  focused_uuid;
 
            focused_uuid = get_focused_context (focused_client, focused_context);
 
            if (target_ic == (uint32) (-1)) {
                target_client  = focused_client;
                target_context = focused_context;
            }
 
            if (target_uuid.length () == 0)
                target_uuid = focused_uuid;
 
            if (target_client  == focused_client &&
                target_context == focused_context &&
                target_uuid    == focused_uuid) {
                ClientInfo client_info = socket_get_client_info (target_client);
                if (client_info.type == FRONTEND_CLIENT) {
                    Socket socket_client (target_client);
                    lock ();
                    m_send_trans.clear ();
                    m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
                    m_send_trans.put_data (target_context);
                    m_send_trans.put_command (cmd);
                    m_send_trans.put_data (key);
                    m_send_trans.write_to_socket (socket_client);
                    unlock ();
                }
            }
        }
    }

    void socket_helper_send_key_event (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_send_key_event (" << client << ")\n";

        socket_helper_key_event_op (client, SCIM_TRANS_CMD_PROCESS_KEY_EVENT);
    }

    void socket_helper_forward_key_event        (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_forward_key_event (" << client << ")\n";

        socket_helper_key_event_op (client, SCIM_TRANS_CMD_FORWARD_KEY_EVENT);
    }

    void socket_helper_commit_string            (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_commit_string (" << client << ")\n";

        uint32 target_ic;
        String target_uuid;
        WideString wstr;

        if (m_recv_trans.get_data (target_ic)    &&
            m_recv_trans.get_data (target_uuid)  && 
            m_recv_trans.get_data (wstr)         &&
            wstr.length ()) {

            int     target_client;
            uint32  target_context;
 
            get_imengine_client_context (target_ic, target_client, target_context);

            int     focused_client;
            uint32  focused_context;
            String  focused_uuid;
 
            focused_uuid = get_focused_context (focused_client, focused_context);
 
            if (target_ic == (uint32) (-1)) {
                target_client  = focused_client;
                target_context = focused_context;
            }
 
            if (target_uuid.length () == 0)
                target_uuid = focused_uuid;
 
            if (target_client  == focused_client &&
                target_context == focused_context &&
                target_uuid    == focused_uuid) {
                ClientInfo client_info = socket_get_client_info (target_client);
                if (client_info.type == FRONTEND_CLIENT) {
                    Socket socket_client (target_client);
                    lock ();
                    m_send_trans.clear ();
                    m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
                    m_send_trans.put_data (target_context);
                    m_send_trans.put_command (SCIM_TRANS_CMD_COMMIT_STRING);
                    m_send_trans.put_data (wstr);
                    m_send_trans.write_to_socket (socket_client);
                    unlock ();
                }
            }
        }
    }

    void socket_helper_register_helper          (int client)
    {
        SCIM_DEBUG_MAIN(4) << "PanelAgent::socket_helper_register_helper (" << client << ")\n";

        HelperInfo info;

        bool result = false;

        lock ();

        Socket socket_client (client);
        m_send_trans.clear ();
        m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);

        if (m_recv_trans.get_data (info.uuid) &&
            m_recv_trans.get_data (info.name) &&
            m_recv_trans.get_data (info.icon) &&
            m_recv_trans.get_data (info.description) &&
            m_recv_trans.get_data (info.option) &&
            info.uuid.length () &&
            info.name.length ()) {

            SCIM_DEBUG_MAIN(4) << "New Helper uuid=" << info.uuid << " name=" << info.name << "\n";

            HelperClientIndex::iterator it = m_helper_client_index.find (info.uuid);

            if (it == m_helper_client_index.end ()) {
                m_helper_info_repository [client] = info;
                m_helper_client_index [info.uuid] = HelperClientStub (client, 1);
                m_send_trans.put_command (SCIM_TRANS_CMD_OK);

                StartHelperICIndex::iterator icit = m_start_helper_ic_index.find (info.uuid);

                if (icit != m_start_helper_ic_index.end ()) {
                    m_send_trans.put_command (SCIM_TRANS_CMD_HELPER_ATTACH_INPUT_CONTEXT);
                    for (size_t i = 0; i < icit->second.size (); ++i) {
                        m_send_trans.put_data (icit->second [i].first);
                        m_send_trans.put_data (icit->second [i].second);
                    }
                    m_start_helper_ic_index.erase (icit);
                }

                m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_SCREEN);
                m_send_trans.put_data ((uint32)m_current_screen);

                result = true;
            } else {
                m_send_trans.put_command (SCIM_TRANS_CMD_FAIL);
            }
        }

        m_send_trans.write_to_socket (socket_client);

        unlock ();

        if (result)
            m_signal_register_helper (client, info);
    }

    void helper_all_update_spot_location        (int x, int y)
    {
        SCIM_DEBUG_MAIN (5) << "PanelAgent::helper_all_update_spot_location (" << x << "," << y << ")\n";

        HelperInfoRepository::iterator hiit = m_helper_info_repository.begin ();
 
        int client;
        uint32 context;
        String uuid;
 
        uuid = get_focused_context (client, context); 
 
        lock ();

        m_send_trans.clear ();
        m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
 
        // FIXME: We presume that client and context are both less than 65536.
        // Hopefully, it should be true in any UNIXs.
        // So it's ok to combine client and context into one uint32.
        m_send_trans.put_data (get_helper_ic (client, context));
        m_send_trans.put_data (uuid);
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_SPOT_LOCATION);
        m_send_trans.put_data ((uint32) x);
        m_send_trans.put_data ((uint32) y);
 
        for (; hiit != m_helper_info_repository.end (); ++ hiit) {
            if (hiit->second.option & SCIM_HELPER_NEED_SPOT_LOCATION_INFO) {
                Socket client_socket (hiit->first);
                m_send_trans.write_to_socket (client_socket);
            }
        }

        unlock ();
    }

    void helper_all_update_screen               (int screen)
    {
        SCIM_DEBUG_MAIN (5) << "PanelAgent::helper_all_update_screen (" << screen << ")\n";

        HelperInfoRepository::iterator hiit = m_helper_info_repository.begin ();
 
        int client;
        uint32 context;
        String uuid;
 
        lock ();

        uuid = get_focused_context (client, context); 
 
        m_send_trans.clear ();
        m_send_trans.put_command (SCIM_TRANS_CMD_REPLY);
 
        // FIXME: We presume that client and context are both less than 65536.
        // Hopefully, it should be true in any UNIXs.
        // So it's ok to combine client and context into one uint32.
        m_send_trans.put_data (get_helper_ic (client, context));
        m_send_trans.put_data (uuid);
        m_send_trans.put_command (SCIM_TRANS_CMD_UPDATE_SCREEN);
        m_send_trans.put_data ((uint32) screen);
 
        for (; hiit != m_helper_info_repository.end (); ++ hiit) {
            if (hiit->second.option & SCIM_HELPER_NEED_SCREEN_INFO) {
                Socket client_socket (hiit->first);
                m_send_trans.write_to_socket (client_socket);
            }
        }

        unlock ();
    }

    const String & get_focused_context (int &client, uint32 &context, bool force_last_context = false) const
    {
        if (m_current_socket_client >= 0) {
            client = m_current_socket_client;
            context = m_current_client_context;
            return m_current_context_uuid;
        } else {
            client = m_last_socket_client;
            context = m_last_client_context;
            return m_last_context_uuid;
        }
    }

private:
    void socket_transaction_start (void)
    {
        m_signal_transaction_start();
    }

    void socket_transaction_end (void)
    {
        m_signal_transaction_end();
    }

    void lock (void)
    {
        m_signal_lock ();
    }
    void unlock (void)
    {
        m_signal_unlock ();
    }
};

PanelAgent::PanelAgent ()
    : m_impl (new PanelAgentImpl ())
{
}

PanelAgent::~PanelAgent ()
{
    delete m_impl;
}

bool
PanelAgent::initialize (const String &config, const String &display, bool resident)
{
    return m_impl->initialize (config, display, resident);
}

bool
PanelAgent::valid (void) const
{
    return m_impl->valid ();
}

bool
PanelAgent::run (void)
{
    return m_impl->run ();
}

void
PanelAgent::stop (void)
{
    m_impl->stop ();
}

int
PanelAgent::get_helper_list (std::vector <HelperInfo> & helpers) const
{
    return m_impl->get_helper_list (helpers);
}

bool
PanelAgent::move_preedit_caret             (uint32         position)
{
    return m_impl->move_preedit_caret (position);
}

bool
PanelAgent::request_help                   (void)
{
    return m_impl->request_help ();
}

bool
PanelAgent::request_factory_menu           (void)
{
    return m_impl->request_factory_menu ();
}

bool
PanelAgent::change_factory                 (const String  &uuid)
{
    return m_impl->change_factory (uuid);
}

bool
PanelAgent::select_candidate               (uint32         item)
{
    return m_impl->select_candidate (item);
}

bool
PanelAgent::lookup_table_page_up           (void)
{
    return m_impl->lookup_table_page_up ();
}

bool
PanelAgent::lookup_table_page_down         (void)
{
    return m_impl->lookup_table_page_down ();
}

bool
PanelAgent::update_lookup_table_page_size  (uint32         size)
{
    return m_impl->update_lookup_table_page_size (size);
}

bool
PanelAgent::trigger_property               (const String  &property)
{
    return m_impl->trigger_property (property);
}

bool
PanelAgent::trigger_helper_property        (int            client,
                                            const String  &property)
{
    return m_impl->trigger_helper_property (client, property);
}

bool
PanelAgent::start_helper                   (const String  &uuid)
{
    return m_impl->start_helper (uuid);
}

bool
PanelAgent::reload_config                  (void)
{
    return m_impl->reload_config ();
}

bool
PanelAgent::exit                           (void)
{
    return m_impl->exit ();
}

Connection
PanelAgent::signal_connect_reload_config              (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_reload_config (slot);
}

Connection
PanelAgent::signal_connect_turn_on                    (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_turn_on (slot);
}

Connection
PanelAgent::signal_connect_turn_off                   (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_turn_off (slot);
}

Connection
PanelAgent::signal_connect_update_screen              (PanelAgentSlotInt                 *slot)
{
    return m_impl->signal_connect_update_screen (slot);
}

Connection
PanelAgent::signal_connect_update_spot_location       (PanelAgentSlotIntInt              *slot)
{
    return m_impl->signal_connect_update_spot_location (slot);
}

Connection
PanelAgent::signal_connect_update_factory_info        (PanelAgentSlotFactoryInfo         *slot)
{
    return m_impl->signal_connect_update_factory_info (slot);
}

Connection
PanelAgent::signal_connect_show_help                  (PanelAgentSlotString              *slot)
{
    return m_impl->signal_connect_show_help (slot);
}

Connection
PanelAgent::signal_connect_show_factory_menu          (PanelAgentSlotFactoryInfoVector   *slot)
{
    return m_impl->signal_connect_show_factory_menu (slot);
}

Connection
PanelAgent::signal_connect_show_preedit_string        (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_show_preedit_string (slot);
}

Connection
PanelAgent::signal_connect_show_aux_string            (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_show_aux_string (slot);
}

Connection
PanelAgent::signal_connect_show_lookup_table          (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_show_lookup_table (slot);
}

Connection
PanelAgent::signal_connect_hide_preedit_string        (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_hide_preedit_string (slot);
}

Connection
PanelAgent::signal_connect_hide_aux_string            (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_hide_aux_string (slot);
}

Connection
PanelAgent::signal_connect_hide_lookup_table          (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_hide_lookup_table (slot);
}

Connection
PanelAgent::signal_connect_update_preedit_string      (PanelAgentSlotAttributeString     *slot)
{
    return m_impl->signal_connect_update_preedit_string (slot);
}

Connection
PanelAgent::signal_connect_update_preedit_caret       (PanelAgentSlotInt                 *slot)
{
    return m_impl->signal_connect_update_preedit_caret (slot);
}

Connection
PanelAgent::signal_connect_update_aux_string          (PanelAgentSlotAttributeString     *slot)
{
    return m_impl->signal_connect_update_aux_string (slot);
}

Connection
PanelAgent::signal_connect_update_lookup_table        (PanelAgentSlotLookupTable         *slot)
{
    return m_impl->signal_connect_update_lookup_table (slot);
}

Connection
PanelAgent::signal_connect_register_properties        (PanelAgentSlotPropertyList        *slot)
{
    return m_impl->signal_connect_register_properties (slot);
}

Connection
PanelAgent::signal_connect_update_property            (PanelAgentSlotProperty            *slot)
{
    return m_impl->signal_connect_update_property (slot);
}

Connection
PanelAgent::signal_connect_register_helper_properties (PanelAgentSlotIntPropertyList     *slot)
{
    return m_impl->signal_connect_register_helper_properties (slot);
}

Connection
PanelAgent::signal_connect_update_helper_property     (PanelAgentSlotIntProperty         *slot)
{
    return m_impl->signal_connect_update_helper_property (slot);
}

Connection
PanelAgent::signal_connect_register_helper            (PanelAgentSlotIntHelperInfo       *slot)
{
    return m_impl->signal_connect_register_helper (slot);
}

Connection
PanelAgent::signal_connect_remove_helper              (PanelAgentSlotInt                 *slot)
{
    return m_impl->signal_connect_remove_helper (slot);
}

Connection
PanelAgent::signal_connect_transaction_start    (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_transaction_start (slot);
}

Connection
PanelAgent::signal_connect_transaction_end      (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_transaction_end (slot);
}

Connection
PanelAgent::signal_connect_lock                       (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_lock (slot);
}

Connection
PanelAgent::signal_connect_unlock                     (PanelAgentSlotVoid                *slot)
{
    return m_impl->signal_connect_unlock (slot);
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/

