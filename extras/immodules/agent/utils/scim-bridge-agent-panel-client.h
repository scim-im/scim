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
 * @brief This is the header of the panel clients for SCIMBridge.
 */

#ifndef SCIMBRIDGEAGENTPANELCLIENT_H
#define SCIMBRIDGEAGENTPANELCLIENT_H

#define Uses_SCIM_EVENT
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_SIGNAL
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#define Uses_SCIM_TRANS_COMMANDS

#include <scim.h>

#include "scim-bridge.h"
#include "scim-bridge-display.h"
#include "scim-bridge-imcontext.h"

typedef scim::Slot1<void, int> ScimBridgeAgentPanelClientSlotVoid;
typedef scim::Slot2<void, int, int> ScimBridgeAgentPanelClientSlotInt;
typedef scim::Slot2<void, int, const scim::String&> ScimBridgeAgentPanelClientSlotString;
typedef scim::Slot2<void, int, const scim::WideString&> ScimBridgeAgentPanelClientSlotWideString;
typedef scim::Slot4<void, int, const scim::String&, const scim::String&, const scim::Transaction&> ScimBridgeAgentPanelClientSlotStringStringTransaction;
typedef scim::Slot2<void, int, const scim::KeyEvent&> ScimBridgeAgentPanelClientSlotKeyEvent;


/**
 * The class of panel clients.
 */
class ScimBridgeAgentPanelClient
{

    public:

        /**
         * Alloc an panel client.
         *
         * @param config_name The name of the configuration.
         * @param display The display.
         * @return The new panel client.
         */
        static ScimBridgeAgentPanelClient *alloc (const scim::String &config_name, const ScimBridgeDisplay *display);


        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentPanelClient ();


        /**
         * Open a connection with the panel.
         */
        virtual int open_connection () = 0;
        

        /**
         * Close a connection with the panel.
         */
        virtual void close_connection () = 0;


        /**
         * Get the connection number with the panel.
         *
         * @return The connection number with the panel.
         */
        virtual int get_connection_number () const = 0;


        /**
         * See if the connection with the panel is active.
         *
         * @return true if the connection is active.
         */
        virtual bool is_connected () const = 0;


        /**
         * See if there is pending events.
         *
         * @return true if there is pending events.
         */
        virtual bool has_pending_event () const = 0;
        

        /**
         * Filter events from the panel.
         *
         * @return RETVAL_FAILED if errors have occurred.
         */
        virtual retval_t filter_event () = 0;


        /**
         * Prepare the panel to do some actions.
         *
         * @param imcontext_id.
         * @return RETVAL_FAILED if errors have occurred.
         */
        virtual retval_t prepare (scim_bridge_imcontext_id_t imcontext_id) = 0;
        

        /**
         * Send stored commands to the panel.
         *
         * @return RETVAL_FAILED if errors have occurred.
         */
        virtual retval_t send () = 0;


        /**
         * Turn on the IME on the panel.
         */
        virtual void turn_on () = 0;

        /**
         * Turn off the IME on the panel.
         */
        virtual void turn_off () = 0;


        /**
         * Update specific screen of the panel.
         */
        virtual void update_screen () = 0;


        /**
         * Show the help text.
         *
         * @param help_text The text to show.
         */
        virtual void show_help (const scim::String &help_text) = 0;


        /**
         * Show the factory menu.
         *
         * @param menu The menu items.
         */
        virtual void show_factory_menu (const std::vector<scim::PanelFactoryInfo> &menu) = 0;
        

        /**
         * Focus in the specific IME.
         *
         * @param uuid The uuid of the IME.
         */
        virtual void focus_in (const scim::String &uuid) = 0;


        /**
         * Focus out the current IME.
         */
        virtual void focus_out () = 0;


        /**
         * Update the status information of the IME on the panel.
         *
         * @param info The infomation of the IME.
         */
        virtual void update_factory_info (const scim::PanelFactoryInfo &info) = 0;
        

        /**
         * Update the cursor location.
         *
         * @param x The X location of the cursor.
         * @param y The Y location of the cursor.
         */
        virtual void update_spot_location (int x, int y) = 0;


        /**
         * Show the floating preedit string.
         */
        virtual void show_preedit_string () = 0;


        /**
         * Show the aux string.
         */
        virtual void show_aux_string () = 0;

        /**
         * Show the lookup table.
         */
        virtual void show_lookup_table () = 0;


        /**
         * Hide the floating preedit string.
         */
        virtual void hide_preedit_string () = 0;


        /**
         * Hide the aux string.
         */
        virtual void hide_aux_string () = 0;


        /**
         * Hide the lookup table.
         */
        virtual void hide_lookup_table () = 0;

        
        /**
         * Update the string in the flaoting preedit.
         *
         * @param str The string to show.
         * @param attrs The attributes of the string.
         */
        virtual void update_preedit_string (const scim::WideString &str, const scim::AttributeList &attrs) = 0;
        
        /**
         * Update the cursor position in the flaoting preedit.
         *
         * @param cursor_position The cursor position.
         */
        virtual void update_preedit_caret (int cursor_position) = 0;
        
        /**
         * Update the aux string.
         *
         * @param str The string to show.
         * @param attrs The attributes of the string.
         */
        virtual void update_aux_string (const scim::WideString &str, const scim::AttributeList &attrs) = 0;


        /**
         * Update the lookup table.
         *
         * @param table The lookup table.
         */
        virtual void update_lookup_table (const scim::LookupTable &table) = 0;

        /**
         * Register properties into the panel.
         *
         * @param properties The properties.
         */
        virtual void register_properties (const scim::PropertyList &properties) = 0;

        /**
         * Update property.
         *
         * @param property The property.
         */
        virtual void update_property (const scim::Property &property) = 0;


        /**
         * Launch a helper process.
         *
         * @param helper_uuid The uuid of the helper.
         */
        virtual void start_helper (const scim::String &helper_uuid) = 0;


        /**
         * Shutdown a helper process.
         *
         * @param helper_uuid The uuid of the helper.
         */
        virtual void stop_helper (const scim::String &helper_uuid) = 0;


        /**
         * Send a message to a helper.
         *
         * @param helper_uuid The uuid of the helper.
         * @param trans The transaction with the helper process.
         */
        virtual void send_helper_event (const scim::String &helper_uuid, const scim::Transaction &trans) = 0;


        /**
         * Register a input context (IMContext).
         *
         * @param uuid The uuid of the IME.
         */
        virtual void register_input_context (const scim::String &uuid) = 0;


        /**
         * Deregister the current input context (IMContext).
         */
        virtual void remove_input_context () = 0;


        /**
         * Connect the signal of reload_config.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_reload_config (ScimBridgeAgentPanelClientSlotVoid *slot) = 0;


        /**
         * Connect the signal of exit.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_exit (ScimBridgeAgentPanelClientSlotVoid *slot) = 0;


        /**
         * Connect the signal of update_lookup_table_page_size.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_update_lookup_table_page_size (ScimBridgeAgentPanelClientSlotInt *slot) = 0;


        /**
         * Connect the signal of lookup_table_page_up.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_lookup_table_page_up (ScimBridgeAgentPanelClientSlotVoid *slot) = 0;


        /**
         * Connect the signal of lookup_table_page_down.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_lookup_table_page_down (ScimBridgeAgentPanelClientSlotVoid *slot) = 0;


        /**
         * Connect the signal of trigger_property.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_trigger_property (ScimBridgeAgentPanelClientSlotString *slot) = 0;


        /**
         * Connect the signal of process_helper_event.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_process_helper_event (ScimBridgeAgentPanelClientSlotStringStringTransaction *slot) = 0;


        /**
         * Connect the signal of move_preedit_caret.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_move_preedit_caret (ScimBridgeAgentPanelClientSlotInt *slot) = 0;


        /**
         * Connect the signal of select_candidate.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_select_candidate (ScimBridgeAgentPanelClientSlotInt *slot) = 0;


        /**
         * Connect the signal of process_key_event.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_process_key_event (ScimBridgeAgentPanelClientSlotKeyEvent *slot) = 0;


        /**
         * Connect the signal of commit_string.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_commit_string (ScimBridgeAgentPanelClientSlotWideString *slot) = 0;


        /**
         * Connect the signal of forward_key_event.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_forward_key_event (ScimBridgeAgentPanelClientSlotKeyEvent *slot) = 0;


        /**
         * Connect the signal of request_help.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_request_help (ScimBridgeAgentPanelClientSlotVoid *slot) = 0;


        /**
         * Connect the signal of request_factory_menu.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_request_factory_menu (ScimBridgeAgentPanelClientSlotVoid *slot) = 0;


        /**
         * Connect the signal of change_factory.
         *
         * @param slot The slot to connect the signal.
         * @return The new connection.
         */
        virtual scim::Connection signal_connect_change_factory (ScimBridgeAgentPanelClientSlotString *slot) = 0;

    protected:

        ScimBridgeAgentPanelClient ();
};
#endif                                            /*SCIMBRIDGEAGENTPANELCLIENT_H*/
