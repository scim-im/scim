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
 * @brief This is the header file for ScimBridgeAgentClientListener.
 */

#ifndef SCIMBRIDGEAGENTCLIENTLISTENER_H_
#define SCIMBRIDGEAGENTCLIENTLISTENER_H_

#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_EVENT
#include <scim.h>

#include "scim-bridge.h"
#include "scim-bridge-imcontext.h"

#include "scim-bridge-agent-socket-client.h"

class ScimBridgeAgentProtected;

/**
 * The class of clients listeners.
 */
class ScimBridgeAgentClientListener: public ScimBridgeAgentSocketClient
{

    public:

        /**
         * Allocate a new client listener.
         *
         * @param socket_fd The socket for the client.
         * @param agent The agent.
         * @return The new client listener.
         */
        static ScimBridgeAgentClientListener *alloc (int socket_fd, ScimBridgeAgentProtected *agent);
        
        /**
         * Destructor.
         */
        virtual ~ScimBridgeAgentClientListener () {}

        /**
         * Notify the changing status of an IMEngine.
         *
         * @param imcontext_id The id of the IMContext.
         * @param enabled The new status for the IMContext.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t imengine_status_changed (scim_bridge_imcontext_id_t imcontext_id, bool enabled) = 0;

        /**
         * Set the visibility of the preedit.
         *
         * @param imcontext_id The id of the IMContext.
         * @param shown The visibility of the preedit.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t set_preedit_shown (scim_bridge_imcontext_id_t imcontext_id, bool shown) = 0;

        /**
         * Set the cursor position in the preedit.
         *
         * @param imcontext_id The id of the IMContext.
         * @param cursor_position The cursor position in the preedit.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t set_preedit_cursor_position (scim_bridge_imcontext_id_t imcontext_id, int cursor_position) = 0;

        /**
         * Set the preedit string.
         *
         * @param imcontext_id The id of the IMContext.
         * @param wstring The new preedit string.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t set_preedit_string (scim_bridge_imcontext_id_t imcontext_id, const scim::WideString &wstring) = 0;

        /**
         * Set the attributes of the preedit.
         *
         * @param imcontext_id The id of the IMContext.
         * @param attributes The preedit attributes.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t set_preedit_attributes (scim_bridge_imcontext_id_t imcontext_id, const scim::AttributeList &attributes) = 0;

        /**
         * Update the preedit.
         *
         * @param imcontext_id The id of the IMContext.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t update_preedit (scim_bridge_imcontext_id_t imcontext_id) = 0;

        /**
         * Commit a string.
         *
         * @param imcontext_id The ID of the IMContext.
         * @param wstring The string to commit.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t commit_string (scim_bridge_imcontext_id_t imcontext_id, const scim::WideString &wstring) = 0;

        /**
         * Make a beep sound.
         *
         * @param imcontext_id The ID of the IMContext.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t beep (scim_bridge_imcontext_id_t imcontext_id) = 0;

        /**
         * Forward a key event into a client.
         *
         * @param imcontext_id The id of the IMContext.
         * @param key_event The key event.
         * @return RETVAL_FAILED if it failed, otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t forward_key_event (scim_bridge_imcontext_id_t imcontext_id, const scim::KeyEvent &key_event) = 0;

        /**
         * Get the surrounding string of the preedit cursor.
         *
         * @parm imcontext_id The id of the IMContext.
         * @param before_max The maximum characters (in wstr) to fetch before the cursor.
         * @param before_max The maximum characters (in wstr) to fetch after the cursor.
         * @param wstring The surrounding string.
         * @param cursor_position The cursor position in the preedit string gotten by this function.
         * @return REVAL_FAILED when a serious error has occurred, or RETVAL_IGNORED if an timeout has happened. Otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t get_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, int before_max, int after_max, scim::WideString &wstring, int &cursor_position) = 0;

        /**
         * Delete the surrounding string of the preedit cursor.
         *
         * @parm imcontext_id The id of the IMContext.
         * @param offset The begining offset to remove from the cursor.
         * @param length The length of the string to remove.
         * @return REVAL_FAILED when a serious error has occurred, or RETVAL_IGNORED if an timeout has happened. Otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t delete_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, int offset, int length) = 0;

        /**
         * Replace the surrounding string of the preedit cursor.
         *
         * @parm imcontext_id The id of the IMContext.
         * @param wstring The new surrounding string.
         * @param cursor_position The new cursor position in the preedit.
         * @return REVAL_FAILED when a serious error has occurred, or RETVAL_IGNORED if an timeout has happened. Otherwise it returns RETVAL_SUCCEEDED.
         */
        virtual retval_t replace_surrounding_string (scim_bridge_imcontext_id_t imcontext_id, const scim::WideString &wstring, int cursor_position) = 0;

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentClientListener () {}

};
#endif                                            /*SCIMBRIDGEAGENTCLIENTLISTENER_H_*/
