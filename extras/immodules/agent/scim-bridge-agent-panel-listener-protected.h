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
 * @brief This is the header file for the protected interface of ScimBridgeAgentPanelListener.
 */

#ifndef SCIMBRIDGEAGENTPANELLISTENERPROTECTED_H_
#define SCIMBRIDGEAGENTPANELLISTENERPROTECTED_H_

#define Uses_SCIM_PANEL_CLIENT

#include <scim.h>

#include <vector>

#include "scim-bridge-imcontext.h"

/**
 * Protected interface of panel listeners.
 */
class ScimBridgeAgentPanelListenerProtected
{

    public:

        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentPanelListenerProtected () {}

        /**
         * Prepare for the send messages to the panel agent.
         * It starts storing messages into the buffer.
         *
         * @param imcontext_id The id of the IMContext.
         */
        virtual void prepare (scim_bridge_imcontext_id_t imcontext_id) = 0;

        /**
         * Send stored messages to the panel agent.
         */
        virtual void send () = 0;

        /**
         * Tell the panel to focus in.
         *
         * @param factory_uuid The uuid of the IME.
         */
        virtual void focus_in (const scim::String &factory_uuid) = 0;

        /**
         * Tell the panel to focus out.
         */
        virtual void focus_out () = 0;

        /**
         * Tell the panel to update the current screen.
         */
        virtual void update_screen () = 0;

        /**
         * Tell the panel to update the cursor location information.
         *
         * @param x The X location of the curosor.
         * @param y The Y location of the curosor.
         */
        virtual void update_cursor_location (int x, int y) = 0;

        /**
         * Tell the panel to reload the IME's information.
         *
         * @param factory_info The information of the IME.
         */
        virtual void update_factory_info (const scim::PanelFactoryInfo &factory_info) = 0;

        /**
         * Tell the panel to turn off the current IME.
         */
        virtual void turn_on () = 0;

        /**
         * Tell the panel to turn on the IME.
         */
        virtual void turn_off () = 0;

        /**
         * Tell the panel to show aux string.
         *
         * @param str The string.
         * @param attrs The attributes.
         */
        virtual void set_aux_string (const scim::WideString &str, const scim::AttributeList &attrs) = 0;

        /**
         * Tell the panel to show the aux string.
         */
        virtual void show_aux_string () = 0;

        /**
         * Tell the panel to hide the aux string.
         */
        virtual void hide_aux_string () = 0;

        /**
         * Tell the panel to update the lookup table.
         *
         * @param table The lookup table.
         */
        virtual void set_lookup_table (const scim::LookupTable &table) = 0;

        /**
         * Tell the panel to hide the lookup table.
         */
        virtual void show_lookup_table () = 0;

        /**
         * Tell the panel to hide the lookup table.
         */
        virtual void hide_lookup_table () = 0;

        /**
         * Tell the panel to set the cursor position in the preedit.
         *
         * @param cursor_pos The cursor position.
         */
        virtual void set_preedit_cursor_position (int cursor_pos) = 0;

        /**
         * Tell the panel to set the preedit string.
         *
         * @param str The preedit string.
         * @param attrs The preedit attributes.
         */
        virtual void set_preedit_string (const scim::WideString &str, const scim::AttributeList &attrs) = 0;

        /**
         * Tell the panel to show the preedit.
         */
        virtual void show_preedit () = 0;

        /**
         * Tell the panel to hide the preedit.
         */
        virtual void hide_preedit () = 0;

        /**
         * Tell the panel to start a helper.
         *
         * @param helper_uuid The uuid of the helper.
         */
        virtual void start_helper (const scim::String &helper_uuid) = 0;

        /**
         * Tell the panel to stop a helper.
         *
         * @param helper_uuid The uuid of the helper.
         */
        virtual void stop_helper (const scim::String &helper_uuid) = 0;

        /**
         * Tell the panel to send messages to a helper.
         *
         * @param helper_uuid The uuid of the helper.
         * @param trans The trunsaction.
         */
        virtual void send_helper_event (const scim::String &helper_uuid, const scim::Transaction &trans) = 0;

        /**
         * Tell the panel to register properties.
         *
         * @param properties The properties.
         */
        virtual void register_properties (const scim::PropertyList &properties) = 0;

        /**
         * Tell the panel to update a property.
         *
         * @param property The property.
         */
        virtual void update_property (const scim::Property &property) = 0;

        /**
         * Tell the panel to show the factory menu.
         *
         * @param menu The menu items.
         */
        virtual void show_factory_menu (const std::vector<scim::PanelFactoryInfo> &menu) = 0;

        /**
         * Tell the panel to show the help text.
         *
         * @param string The help text.
         */
        virtual void show_help (const scim::String &string) = 0;

        /**
         * tell the panel to register the input context (IMContext).
         *
         * @param factory_uuid The uuid of the IME associating with the current IMContext.
         */
        virtual void register_input_context (const scim::String &factory_uuid) = 0;

        /**
         * tell the panel to deregister the input context (IMContext).
         */
        virtual void deregister_input_context () = 0;

    protected:

        /**
         * Destructor.
         */
        ScimBridgeAgentPanelListenerProtected () {}

};
#endif                                            /*SCIMBRIDGEAGENTPANELLISTENERPROTECTED_H_*/
