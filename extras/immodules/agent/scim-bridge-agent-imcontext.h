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
 * @brief This is the header file for ScimBridgeAgentIMContext.
 */

#ifndef SCIMBRIDGEAGENTIMCONTEXT_H_
#define SCIMBRIDGEAGENTIMCONTEXT_H_

#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_IMENGINE_MODULE
#define Uses_SCIM_LOOKUP_TABLE

#include <scim.h>

#include "scim-bridge.h"
#include "scim-bridge-imcontext.h"

#include "scim-bridge-agent-panel-listener-protected.h"

class ScimBridgeAgentClientListener;
class ScimBridgeAgentPanelListenerProtected;

/**
 * The public interface of IMContext.
 */
class ScimBridgeAgentIMContext
{

    public:

        /**
         * Allocate an IMContext.
         *
         * @param client_listener The client listener.
         * @return The new IMContext.
         */
        static ScimBridgeAgentIMContext *alloc (ScimBridgeAgentClientListener *client_listener);

        /**
         * Free IMContexts of a client.
         *
         * @param client_listener The client listener.
         */
        static void free_by_client (const ScimBridgeAgentClientListener *client_listener);

        /**
         * Find the specific IMContext.
         *
         * @param imcontext_id The id of the IMContext.
         * @return The IMContext which have the given id. If there isn't, it returns NULL.
         */
        static ScimBridgeAgentIMContext *find (scim_bridge_imcontext_id_t imcontext_id);

        /**
         * Initialize the class.
         *
         * @param panel_listener The panel listener.
         * @param scim_language The language used in SCIM.
         * @param scim_backend The scim backend.
         */
        static void static_initialize (ScimBridgeAgentPanelListenerProtected *panel_listener, const scim::String &scim_language, scim::BackEndPointer scim_backend);

        /**
         * Finalize the class.
         */
        static void static_finalize ();

        /**
         * Check if IMEngines are shared.
         *
         * @return true if IMEngines are shared.
         */
        static bool is_imengine_shared ();

        /**
         * Set whether IMEngines are shared or not.
         *
         * @param shared Give it true to share all the IMEngines among applications.
         */
        static void set_imengine_shared (bool shared);

        /**
         * Check if the on_the_spot input is disabled by the preference.
         *
         * @return false only if on_the_spot input is disabled by the preference.
         */
        static bool is_on_the_spot_enabled ();

        /**
         * Set whether the on_the_spot input is disabled by the preference.
         *
         * @param enabled Give it false when on_the_spot input is disabled by the preference.
         */
        static void set_on_the_spot_enabled (bool enabled);

        /**
         * Check the IME's primary status.
         *
         * @return true if the IME is turned on by default.
         */
        static bool is_enabled_by_default ();

        /**
         * Set the IME's primary status.
         *
         * @param enabled Give it true if the IME should be turned on by default.
         */
        static void set_enabled_by_default (bool enabled);

        /**
         * Get the hotkey help messages.
         *
         * @return help messages about hotkeys.
         */
        static scim::String get_help_hotkeys ();

        /**
         * Set the hotkey help messages.
         *
         * @param hotkey_str Help message about hotkeys.
         *
         */
        static void set_help_hotkeys (const scim::String &hotkey_str);

        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentIMContext () {}

        /**
         * See if this IMContext is turned on on not.
         *
         * @return true if this IMContext is turned on.
         */
        virtual bool is_enabled () = 0;

        /**
         * Set the status of IMContext.
         *
         * @param new_state Give true to turn on this IMContext, or false to turn off.
         */
        virtual void set_enabled (bool new_state) = 0;

        /**
         * Get the preedit mode of an IMContext.
         *
         * @return The preedit mode of an IMContext.
         */
        virtual scim_bridge_preedit_mode_t get_preedit_mode () const = 0;

        /**
         * Set the preedit mode of an IMContext.
         *
         * @param preedit_mode The preedit mode of an IMContext.
         */
        virtual void set_preedit_mode (scim_bridge_preedit_mode_t new_preedit_mode) = 0;

        /**
         * Get the ID of the IMContext.
         *
         * @return The id of the IMContext.
         */
        virtual scim_bridge_imcontext_id_t get_id () = 0;

        /**
         * Get the client for the IMContext.
         *
         * @return The client for the IMContext.
         */
        virtual ScimBridgeAgentClientListener *get_client_listener () = 0;

        /**
         * Filter a key event.
         *
         * @param key_event The key event.
         * @return true if this key event is consumed, and should not be handled any more.
         */
        virtual bool filter_key_event (const scim::KeyEvent &key_event) = 0;

        /**
         * Forward a key event into the client.
         *
         * @param key_event The key event.
         */
        virtual void forward_key_event (const scim::KeyEvent &key_event) = 0;

        /**
         * Focus in the IMContext.
         */
        virtual void focus_in () = 0;

        /**
         * Focus out the IMContext.
         */
        virtual void focus_out () = 0;

        /**
         * Reset this IMContext.
         */
        virtual void reset () = 0;

        /**
         * Set the cursor position.
         *
         * @param x The X location of the cursor.
         * @param y The Y location of the cursor.
         */
        virtual void set_cursor_location (int x, int y) = 0;

        /**
         * Switch to the next IMEngine.
         */
        virtual void open_next_imengine () = 0;

        /**
         * Switch to the previous IMEngine.
         */
        virtual void open_previous_imengine () = 0;

        /**
         * Switch to the IMEngine of the given uuid.
         */
        virtual void open_imengine_by_uuid (const scim::String &uuid) = 0;

        /**
         * Switch to the specific IMEngine.
         */
        virtual void open_imengine (scim::IMEngineFactoryPointer factory) = 0;

        /**
         * Update the page size of the lookup table.
         *
         * @param page_size The size of the lookup page.
         */
        virtual void update_lookup_table_page_size (int page_size) = 0;

        /**
         * Page up the lookup table.
         */
        virtual void lookup_table_page_up () = 0;

        /**
         * Page down the lookup table.
         */
        virtual void lookup_table_page_down () = 0;

        /**
         * Select the candidate in the lookup table.
         *
         * @param index The index to select.
         */
        virtual void lookup_table_select_candidate (int candidate_index) = 0;

        /**
         * Trigger the property of the IMEngines.
         *
         * @param property The property to trigger.
         */
        virtual void trigger_property (const scim::String &property) = 0;

        /**
         * Process a helper event.
         *
         * @param target_uuid The uuid of the target.
         * @param helper_uuid The uuid of the helper.
         * @param trans The transaction with the helper process.
         */
        virtual void process_helper_event (const scim::String &target_uuid, const scim::String &helper_uuid, const scim::Transaction &trans) = 0;

        /**
         * The panel requested to move the caret in the preedit.
         *
         * @param caret_pos The new cursor position in the preedit.
         */
        virtual void panel_move_preedit_caret (int caret_pos) = 0;

        /**
         * The panel requested to commit a string.
         *
         * @param wstr The string to commit.
         */
        virtual void panel_commit_string (const scim::WideString &wstr) = 0;

        /**
         * The panel requested to show help.
         */
        virtual void panel_request_help () = 0;

        /**
         * The panel requested to change IMEngine.
         *
         * @param uuid The uuid of the new IMEngine.
         */
        virtual void panel_change_factory (const scim::String &uuid) = 0;

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentIMContext () {}

};
#endif                                            /*SCIMBRIDGEAGENTIMCONTEXT_H_*/
