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


#include <errno.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <unistd.h>

#define Uses_SCIM_CONFIG_PATH

#include <scim.h>

#ifndef SCIM_PANEL_PROGRAM
#define SCIM_PANEL_PROGRAM (SCIM_LIBEXECDIR "/scim-panel-gtk")
#endif

#include "scim-bridge-agent-panel-client.h"
#include "scim-bridge-output.h"

using std::vector;

using namespace scim;

typedef Signal1<void, int> ScimBridgeAgentPanelClientSignalVoid;
typedef Signal2<void, int, int> ScimBridgeAgentPanelClientSignalInt;
typedef Signal2<void, int, const String&> ScimBridgeAgentPanelClientSignalString;
typedef Signal2<void, int, const WideString&> ScimBridgeAgentPanelClientSignalWideString;
typedef Signal4<void, int, const String&, const String&, const Transaction&> ScimBridgeAgentPanelClientSignalStringStringTransaction;
typedef Signal2<void, int, const KeyEvent&> ScimBridgeAgentPanelClientSignalKeyEvent;

class ScimBridgeAgentPanelClientImpl: public ScimBridgeAgentPanelClient
{

    public:

        ScimBridgeAgentPanelClientImpl (const String &config_name, const ScimBridgeDisplay *display);
        ~ScimBridgeAgentPanelClientImpl ();

        int open_connection ();
        void close_connection ();
        int get_connection_number () const;
        bool is_connected () const;

        bool has_pending_event () const;
        retval_t filter_event ();

        retval_t prepare (scim_bridge_imcontext_id_t imcontext_id);
        retval_t send ();

        void turn_on ();
        void turn_off ();
        void update_screen ();
        void show_help (const String &help);
        void show_factory_menu (const vector<PanelFactoryInfo> &menu);
        void focus_in (const scim::String &uuid);
        void focus_out ();
        void update_factory_info (const PanelFactoryInfo &info);
        void update_spot_location (int x, int y);
        void show_preedit_string ();
        void show_aux_string ();
        void show_lookup_table ();
        void hide_preedit_string ();
        void hide_aux_string ();
        void hide_lookup_table ();
        void update_preedit_string (const WideString &str, const AttributeList &attrs);
        void update_preedit_caret (int cursor_position);
        void update_aux_string (const WideString &str, const AttributeList &attrs);
        void update_lookup_table (const LookupTable &table);
        void register_properties (const PropertyList &properties);
        void update_property (const Property &property);
        void start_helper (const String &helper_uuid);
        void stop_helper (const String &helper_uuid);
        void send_helper_event (const String &helper_uuid, const Transaction &trans);
        void register_input_context (const String &uuid);
        void remove_input_context ();

        Connection signal_connect_reload_config (ScimBridgeAgentPanelClientSlotVoid *slot);
        Connection signal_connect_exit (ScimBridgeAgentPanelClientSlotVoid *slot);
        Connection signal_connect_update_lookup_table_page_size (ScimBridgeAgentPanelClientSlotInt *slot);
        Connection signal_connect_lookup_table_page_up (ScimBridgeAgentPanelClientSlotVoid *slot);
        Connection signal_connect_lookup_table_page_down (ScimBridgeAgentPanelClientSlotVoid *slot);
        Connection signal_connect_trigger_property (ScimBridgeAgentPanelClientSlotString *slot);
        Connection signal_connect_process_helper_event (ScimBridgeAgentPanelClientSlotStringStringTransaction *slot);
        Connection signal_connect_move_preedit_caret (ScimBridgeAgentPanelClientSlotInt *slot);
        Connection signal_connect_select_candidate (ScimBridgeAgentPanelClientSlotInt *slot);
        Connection signal_connect_process_key_event (ScimBridgeAgentPanelClientSlotKeyEvent *slot);
        Connection signal_connect_commit_string (ScimBridgeAgentPanelClientSlotWideString *slot);
        Connection signal_connect_forward_key_event (ScimBridgeAgentPanelClientSlotKeyEvent *slot);
        Connection signal_connect_request_help (ScimBridgeAgentPanelClientSlotVoid *slot);
        Connection signal_connect_request_factory_menu (ScimBridgeAgentPanelClientSlotVoid *slot);
        Connection signal_connect_change_factory (ScimBridgeAgentPanelClientSlotString *slot);

    private:

        String config_name;
        ScimBridgeDisplay *display;

        SocketClient socket_client;
        int socket_timeout;
        uint32_t socket_magic_key;
        Transaction sending_transaction;
        scim_bridge_imcontext_id_t prepared_imcontext_id;

        ScimBridgeAgentPanelClientSignalVoid  signal_reload_config;
        ScimBridgeAgentPanelClientSignalVoid signal_exit;
        ScimBridgeAgentPanelClientSignalInt signal_update_lookup_table_page_size;
        ScimBridgeAgentPanelClientSignalVoid signal_lookup_table_page_up;
        ScimBridgeAgentPanelClientSignalVoid signal_lookup_table_page_down;
        ScimBridgeAgentPanelClientSignalString signal_trigger_property;
        ScimBridgeAgentPanelClientSignalStringStringTransaction signal_process_helper_event;
        ScimBridgeAgentPanelClientSignalInt signal_move_preedit_caret;
        ScimBridgeAgentPanelClientSignalInt signal_select_candidate;
        ScimBridgeAgentPanelClientSignalKeyEvent signal_process_key_event;
        ScimBridgeAgentPanelClientSignalWideString signal_commit_string;
        ScimBridgeAgentPanelClientSignalKeyEvent signal_forward_key_event;
        ScimBridgeAgentPanelClientSignalVoid signal_request_help;
        ScimBridgeAgentPanelClientSignalVoid signal_request_factory_menu;
        ScimBridgeAgentPanelClientSignalString signal_change_factory;

        retval_t launch_panel ();

};

/* Implementations */
ScimBridgeAgentPanelClient *ScimBridgeAgentPanelClient::alloc (const String &config_name, const ScimBridgeDisplay *display)
{
    return new ScimBridgeAgentPanelClientImpl (config_name, display);
}


ScimBridgeAgentPanelClient::ScimBridgeAgentPanelClient ()
{
}


ScimBridgeAgentPanelClient::~ScimBridgeAgentPanelClient ()
{
}


ScimBridgeAgentPanelClientImpl::ScimBridgeAgentPanelClientImpl (const String &new_config_name, const ScimBridgeDisplay *new_display):
config_name (new_config_name), display (NULL), socket_timeout (scim_get_default_socket_timeout ()), socket_magic_key (0), prepared_imcontext_id (-1)
{
    display = scim_bridge_alloc_display ();
    scim_bridge_copy_display (display, new_display);
}


ScimBridgeAgentPanelClientImpl::~ScimBridgeAgentPanelClientImpl ()
{
    if (socket_client.is_connected ()) close_connection ();
    scim_bridge_free_display (display);
}


retval_t ScimBridgeAgentPanelClientImpl::launch_panel ()
{
    scim_bridge_pdebugln (7, "launch_panel ()");

    String panel_program = scim_global_config_read (SCIM_GLOBAL_CONFIG_DEFAULT_PANEL_PROGRAM, String (SCIM_PANEL_PROGRAM));
    if (panel_program [0] != SCIM_PATH_DELIM) panel_program = String (SCIM_LIBEXECDIR) + String (SCIM_PATH_DELIM_STRING) + panel_program;

    struct stat stat_buf;
    if (lstat (panel_program.c_str (), &stat_buf) || !S_ISREG (stat_buf.st_mode) || access (panel_program.c_str (), X_OK)) {
        panel_program = String (SCIM_PANEL_PROGRAM);
    }
    scim_bridge_pdebugln (5, "Launch \"%s\"", panel_program.c_str ());

    if (display != NULL) {
        const pid_t retval = fork ();
        if (retval == 0) {
            if (daemon (0, 0) == 0) {
                char *display_name_cstr = strdup (scim_bridge_display_get_name (display));
                char *conf_name_cstr = strdup (config_name.c_str ());
                char *panel_program_cstr = strdup (panel_program.c_str ());
                char *my_argv[] = {panel_program_cstr, const_cast<char*>("--no-stay"), const_cast<char*>("--display"), display_name_cstr, const_cast<char*>("-c"), conf_name_cstr, const_cast<char*>("-d"), 0};
                if (execv (panel_program_cstr, my_argv)) abort ();
            } else {
                scim_bridge_perrorln ("Failed to forking for a panel process: %s", errno == 0 ? "Unknown reason":strerror (errno));
                abort ();
            }
            return RETVAL_FAILED;
        } else if (retval > 0) {
            waitpid (retval, NULL, 0);
            return RETVAL_SUCCEEDED;
        } else {
            scim_bridge_perrorln ("Failed to launch a panel: %s", errno == 0 ? "Unknown reason":strerror (errno));
            return RETVAL_FAILED;
        }
    } else {
        scim_bridge_println ("No display for the panel");
        return RETVAL_FAILED;
    }
}


int ScimBridgeAgentPanelClientImpl::open_connection ()
{
    if (socket_client.is_connected ()) close_connection ();
    SocketAddress address (scim_get_default_panel_socket_address (scim_bridge_display_get_name (display)));

    if (!socket_client.connect (address)) {
        launch_panel ();
        usleep (100000);
    }
        
    // Try to establish the connection six times.
    for (int i = 0; i < 6; ++i) {
        if (!socket_client.connect (address)) {
            usleep (100000);
        } else {
            if (scim_socket_open_connection (socket_magic_key, String ("FrontEnd"), String ("Panel"), socket_client, socket_timeout)) {
                return socket_client.get_id ();
            }
        }
        socket_client.close ();
    }

    return -1;
}


void ScimBridgeAgentPanelClientImpl::close_connection ()
{
    socket_client.close ();
    socket_magic_key = 0;
    prepared_imcontext_id = -1;
}


int ScimBridgeAgentPanelClientImpl::get_connection_number () const
{
    return socket_client.get_id ();
}


bool ScimBridgeAgentPanelClientImpl::is_connected () const
{
    return socket_client.is_connected ();
}


bool ScimBridgeAgentPanelClientImpl::has_pending_event () const
{
    return socket_client.is_connected () && socket_client.wait_for_data (0) > 0;
}


retval_t ScimBridgeAgentPanelClientImpl::filter_event ()
{
    Transaction receiving_transaction;

    if (!socket_client.is_connected () || !receiving_transaction.read_from_socket (socket_client, socket_timeout)) return RETVAL_FAILED;

    int command;
    if (!receiving_transaction.get_command (command) || command != SCIM_TRANS_CMD_REPLY) return RETVAL_SUCCEEDED;

    // No IMContext ID is available; this is a global command.
    if (receiving_transaction.get_data_type () == SCIM_TRANS_DATA_COMMAND) {
        while (receiving_transaction.get_command (command)) {
            switch (command) {
                case SCIM_TRANS_CMD_RELOAD_CONFIG:
                    signal_reload_config (-1);
                    break;
                case SCIM_TRANS_CMD_EXIT:
                    signal_exit (-1);
                    break;
                default:
                    break;
            }
        }

    } else {

        // Check if there is a command for the specific IMContext.
        uint32_t imcontext_id;
        if (!receiving_transaction.get_data (imcontext_id)) return RETVAL_SUCCEEDED;

        while (receiving_transaction.get_command (command)) {
            switch (command) {
                case SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE_PAGE_SIZE:
                {
                    uint32_t size;
                    if (receiving_transaction.get_data (size)) signal_update_lookup_table_page_size (imcontext_id, (int) size);
                }
                break;
                case SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_UP:
                {
                    signal_lookup_table_page_up (imcontext_id);
                }
                break;
                case SCIM_TRANS_CMD_LOOKUP_TABLE_PAGE_DOWN:
                {
                    signal_lookup_table_page_down (imcontext_id);
                }
                break;
                case SCIM_TRANS_CMD_TRIGGER_PROPERTY:
                {
                    String property;
                    if (receiving_transaction.get_data (property)) signal_trigger_property (imcontext_id, property);
                }
                break;
                case SCIM_TRANS_CMD_PROCESS_HELPER_EVENT:
                {
                    String target_uuid;
                    String helper_uuid;
                    Transaction trans;
                    if (receiving_transaction.get_data (target_uuid) && receiving_transaction.get_data (helper_uuid) && receiving_transaction.get_data (trans)) signal_process_helper_event (imcontext_id, target_uuid, helper_uuid, trans);
                }
                break;
                case SCIM_TRANS_CMD_MOVE_PREEDIT_CARET:
                {
                    uint32_t cursor_pos;
                    if (receiving_transaction.get_data (cursor_pos)) signal_move_preedit_caret (imcontext_id, cursor_pos);
                }
                break;
                case SCIM_TRANS_CMD_SELECT_CANDIDATE:
                {
                    uint32_t item;
                    if (receiving_transaction.get_data (item)) signal_select_candidate (imcontext_id, item);
                }
                break;
                case SCIM_TRANS_CMD_PROCESS_KEY_EVENT:
                {
                    KeyEvent key_event;
                    if (receiving_transaction.get_data (key_event)) signal_process_key_event (imcontext_id, key_event);
                }
                break;
                case SCIM_TRANS_CMD_FORWARD_KEY_EVENT:
                {
                    KeyEvent key_event;
                    if (receiving_transaction.get_data (key_event)) signal_forward_key_event (imcontext_id, key_event);
                }
                break;
                case SCIM_TRANS_CMD_COMMIT_STRING:
                {
                    WideString wstr;
                    if (receiving_transaction.get_data (wstr)) signal_commit_string (imcontext_id, wstr);
                }
                break;
                case SCIM_TRANS_CMD_PANEL_REQUEST_HELP:
                {
                    signal_request_help (imcontext_id);
                }
                break;
                case SCIM_TRANS_CMD_PANEL_REQUEST_FACTORY_MENU:
                {
                    signal_request_factory_menu (imcontext_id);
                }
                break;
                case SCIM_TRANS_CMD_PANEL_CHANGE_FACTORY:
                {
                    String factory_uuid;
                    if (receiving_transaction.get_data (factory_uuid)) signal_change_factory (imcontext_id, factory_uuid);
                }
                break;
                default:
                    break;
            }
        }
    }

    return RETVAL_SUCCEEDED;
}


retval_t ScimBridgeAgentPanelClientImpl::prepare (scim_bridge_imcontext_id_t imcontext_id)
{
    if (!socket_client.is_connected ()) {
        if (open_connection ()) return RETVAL_FAILED;
    }
    
    if (prepared_imcontext_id == -1) {
        sending_transaction.clear ();
        sending_transaction.put_command (SCIM_TRANS_CMD_REQUEST);
        sending_transaction.put_data (socket_magic_key);
        sending_transaction.put_data ((uint32_t) imcontext_id);

        int command;
        uint32_t data;
        sending_transaction.get_command (command);
        sending_transaction.get_data (data);
        sending_transaction.get_data (data);

        prepared_imcontext_id = imcontext_id;
        return RETVAL_SUCCEEDED;
    } else {
        scim_bridge_pdebugln (8, "The panel client is already prepared!");
        return RETVAL_FAILED;
    }
}


retval_t ScimBridgeAgentPanelClientImpl::send ()
{
    if (!socket_client.is_connected ()) return RETVAL_SUCCEEDED;

    if (prepared_imcontext_id == -1) return RETVAL_SUCCEEDED;

    prepared_imcontext_id = -1;
    if (sending_transaction.get_data_type () != SCIM_TRANS_DATA_UNKNOWN) {
        return sending_transaction.write_to_socket (socket_client, 0x4d494353);
    } else {
        return RETVAL_SUCCEEDED;
    }
}


void ScimBridgeAgentPanelClientImpl::turn_on ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_TURN_ON);
}


void ScimBridgeAgentPanelClientImpl::turn_off ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_TURN_OFF);
}


void ScimBridgeAgentPanelClientImpl::update_screen ()
{
    if (prepared_imcontext_id != -1 && display != NULL) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_SCREEN);
        sending_transaction.put_data ((uint32_t) scim_bridge_display_get_screen_number (display));
    }
}


void ScimBridgeAgentPanelClientImpl::show_help (const String &help)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_SHOW_HELP);
        sending_transaction.put_data (help);
    }
}


void ScimBridgeAgentPanelClientImpl::show_factory_menu (const vector<PanelFactoryInfo> &menu)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_SHOW_FACTORY_MENU);
        for (size_t i = 0; i < menu.size (); ++i) {
            sending_transaction.put_data (menu[i].uuid);
            sending_transaction.put_data (menu[i].name);
            sending_transaction.put_data (menu[i].lang);
            sending_transaction.put_data (menu[i].icon);
        }
    }
}


void ScimBridgeAgentPanelClientImpl::focus_in (const String &uuid)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_FOCUS_IN);
        sending_transaction.put_data (uuid);
    }
}


void ScimBridgeAgentPanelClientImpl::focus_out ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_FOCUS_OUT);
}


void ScimBridgeAgentPanelClientImpl::update_factory_info (const PanelFactoryInfo &info)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_UPDATE_FACTORY_INFO);
        sending_transaction.put_data (info.uuid);
        sending_transaction.put_data (info.name);
        sending_transaction.put_data (info.lang);
        sending_transaction.put_data (info.icon);
    }
}


void ScimBridgeAgentPanelClientImpl::update_spot_location (int x, int y)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_SPOT_LOCATION);
        sending_transaction.put_data ((uint32_t) x);
        sending_transaction.put_data ((uint32_t) y);
    }
}


void ScimBridgeAgentPanelClientImpl::show_preedit_string ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_SHOW_PREEDIT_STRING);
}


void ScimBridgeAgentPanelClientImpl::show_aux_string ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_SHOW_AUX_STRING);
}


void ScimBridgeAgentPanelClientImpl::show_lookup_table ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_SHOW_LOOKUP_TABLE);
}


void ScimBridgeAgentPanelClientImpl::hide_preedit_string ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_HIDE_PREEDIT_STRING);
}


void ScimBridgeAgentPanelClientImpl::hide_aux_string ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_HIDE_AUX_STRING);
}


void ScimBridgeAgentPanelClientImpl::hide_lookup_table ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_HIDE_LOOKUP_TABLE);
}


void ScimBridgeAgentPanelClientImpl::update_preedit_string (const WideString &str, const AttributeList &attrs)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_PREEDIT_STRING);
        sending_transaction.put_data (utf8_wcstombs (str));
        sending_transaction.put_data (attrs);
    }
}


void ScimBridgeAgentPanelClientImpl::update_preedit_caret (int caret)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_PREEDIT_CARET);
        sending_transaction.put_data ((uint32_t) caret);
    }
}


void ScimBridgeAgentPanelClientImpl::update_aux_string (const WideString &str, const AttributeList &attrs)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_AUX_STRING);
        sending_transaction.put_data (utf8_wcstombs (str));
        sending_transaction.put_data (attrs);
    }
}


void ScimBridgeAgentPanelClientImpl::update_lookup_table (const LookupTable &table)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_LOOKUP_TABLE);
        sending_transaction.put_data (table);
    }
}


void ScimBridgeAgentPanelClientImpl::register_properties (const PropertyList &properties)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_REGISTER_PROPERTIES);
        sending_transaction.put_data (properties);
    }
}


void ScimBridgeAgentPanelClientImpl::update_property (const Property &property)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_UPDATE_PROPERTY);
        sending_transaction.put_data (property);
    }
}


void ScimBridgeAgentPanelClientImpl::start_helper (const String &helper_uuid)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_START_HELPER);
        sending_transaction.put_data (helper_uuid);
    }
}


void ScimBridgeAgentPanelClientImpl::stop_helper (const String &helper_uuid)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_STOP_HELPER);
        sending_transaction.put_data (helper_uuid);
    }
}


void ScimBridgeAgentPanelClientImpl::send_helper_event (const String &helper_uuid, const Transaction &trans)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_SEND_HELPER_EVENT);
        sending_transaction.put_data (helper_uuid);
        sending_transaction.put_data (trans);
    }
}


void ScimBridgeAgentPanelClientImpl::register_input_context (const String &uuid)
{
    if (prepared_imcontext_id != -1) {
        sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_REGISTER_INPUT_CONTEXT);
        sending_transaction.put_data (uuid);
    }
}


void ScimBridgeAgentPanelClientImpl::remove_input_context ()
{
    if (prepared_imcontext_id != -1) sending_transaction.put_command (SCIM_TRANS_CMD_PANEL_REMOVE_INPUT_CONTEXT);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_reload_config (ScimBridgeAgentPanelClientSlotVoid *slot)
{
    return signal_reload_config.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_exit (ScimBridgeAgentPanelClientSlotVoid *slot)
{
    return signal_exit.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_update_lookup_table_page_size (ScimBridgeAgentPanelClientSlotInt *slot)
{
    return signal_update_lookup_table_page_size.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_lookup_table_page_up (ScimBridgeAgentPanelClientSlotVoid *slot)
{
    return signal_lookup_table_page_up.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_lookup_table_page_down (ScimBridgeAgentPanelClientSlotVoid *slot)
{
    return signal_lookup_table_page_down.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_trigger_property (ScimBridgeAgentPanelClientSlotString *slot)
{
    return signal_trigger_property.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_process_helper_event (ScimBridgeAgentPanelClientSlotStringStringTransaction *slot)
{
    return signal_process_helper_event.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_move_preedit_caret (ScimBridgeAgentPanelClientSlotInt *slot)
{
    return signal_move_preedit_caret.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_select_candidate (ScimBridgeAgentPanelClientSlotInt *slot)
{
    return signal_select_candidate.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_process_key_event (ScimBridgeAgentPanelClientSlotKeyEvent *slot)
{
    return signal_process_key_event.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_commit_string (ScimBridgeAgentPanelClientSlotWideString *slot)
{
    return signal_commit_string.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_forward_key_event (ScimBridgeAgentPanelClientSlotKeyEvent *slot)
{
    return signal_forward_key_event.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_request_help (ScimBridgeAgentPanelClientSlotVoid *slot)
{
    return signal_request_help.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_request_factory_menu (ScimBridgeAgentPanelClientSlotVoid *slot)
{
    return signal_request_factory_menu.connect (slot);
}


Connection ScimBridgeAgentPanelClientImpl::signal_connect_change_factory (ScimBridgeAgentPanelClientSlotString *slot)
{
    return signal_change_factory.connect (slot);
}
