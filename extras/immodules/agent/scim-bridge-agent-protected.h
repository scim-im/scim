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

#ifndef SCIMBRIDGEAGENTPROTECTED_H_
#define SCIMBRIDGEAGENTPROTECTED_H_

/**
 * @file
 * @author Ryo Dairiki <ryo-dairiki@users.sourceforge.net>
 * @brief This is the header file for the protected interface of ScimBridgeAgent.
 */

#define Uses_SCIM_EVENT
#define Uses_SCIM_TRANSACTION

#include <scim.h>

#include "scim-bridge-display.h"
#include "scim-bridge-imcontext.h"

class ScimBridgeAgentClientListener;
class ScimBridgeAgentIMContext;
class ScimBridgeAgentPanelListener;

/**
 * The protected interfaces of the agent.
 */
class ScimBridgeAgentProtected
{

    public:

        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentProtected () {}

        /**
         * Make an interruption upon the main event loop of the agent.
         */
        virtual void interrupt () = 0;

        /**
         * Quit the agent process.
         */
        virtual void quit () = 0;

        /**
         * Load the configuration.
         */
        virtual void load_config () = 0;

        /**
         * Save the configuration.
         */
        virtual void save_config () = 0;

        /**
         * Add a new client application.
         *
         * @param client_lisener The new client listener for the application.
         */
        virtual void add_client_listener (ScimBridgeAgentClientListener *client_listener) = 0;

        /**
         * Remove a client application.
         *
         * @param client_listener The client listener for the application.
         */
        virtual void remove_client_listener (ScimBridgeAgentClientListener *client_listener) = 0;

        /**
         * Filter hot key events.
         *
         * @param imcontext_id The ID of IMContext.
         * @param key_event The key event.
         * @return Whether the key event is consumed or not.
         */
        virtual bool filter_hotkeys (scim_bridge_imcontext_id_t imcontext_id, const scim::KeyEvent &key_event) = 0;

        /**
         * Filter a key event.
         *
         * @param imcontext_id The ID of IMContext.
         * @param key_event The key event.
         * @return Whether the key event is consumed or not.
         */
        virtual bool filter_key_event (scim_bridge_imcontext_id_t imcontext_id, const scim::KeyEvent &key_event) = 0;

        /**
         * Request to show the factory menu.
         */
        virtual void request_factory_menu () = 0;

        /**
         * Allocate an IMContext.
         *
         * @param client_listener The client listener.
         * @return The ID of the new IMContext.
         */
        virtual scim_bridge_imcontext_id_t alloc_imcontext (ScimBridgeAgentClientListener *client_listener) = 0;

        /**
         * Free an IMContext.
         *
         * @param client_listener The client listener.
         * @param imcontext_id The ID of the IMContext.
         */
        virtual void free_imcontext (scim_bridge_imcontext_id_t imcontext_id, const ScimBridgeAgentClientListener *client_listener) = 0;

        /**
         * Reset an IMContext.
         *
         * @param imcontext_id The ID of the IMContext.
         */
        virtual void reset_imcontext (scim_bridge_imcontext_id_t imcontext_id) = 0;

        /**
         * Enable an IMContext.
         *
         * @param imcontext_id The ID of the IMContext.
         */
        virtual void enable_imcontext (scim_bridge_imcontext_id_t imcontext_id) = 0;

        /**
         * Disable an IMContext.
         *
         * @param imcontext_id The ID of the IMContext.
         */
        virtual void disable_imcontext (scim_bridge_imcontext_id_t imcontext_id) = 0;

        /**
         * Set the cursor location in the display.
         *
         * @param imcontext_id The ID of the IMContext.
         * @param cursor_x The X location of the cursor.
         * @param cursor_y The Y location of the cursor.
         */
        virtual void set_cursor_location (scim_bridge_imcontext_id_t imcontext_id, int cursor_x, int cursor_y) = 0;

        /**
         * Set the preedit mode of an IMContext.
         *
         * @param imcontext_id The ID of the IMContext.
         * @param preedit_mode The new preedit mode.
         */
        virtual void set_preedit_mode (scim_bridge_imcontext_id_t imcontext_id, scim_bridge_preedit_mode_t preedit_mode) = 0;

        /**
         * Change the focus of an IMContext.
         *
         * @param imcontext_id The ID of the IMContext.
         * @param focus_in If it grabs an focus or not.
         */
        virtual void change_focus (scim_bridge_imcontext_id_t imcontext_id, bool focus_in) = 0;

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentProtected () {}

};
#endif                                            /*SCIMBRIDGEAGENTPROTECTED_H_*/
