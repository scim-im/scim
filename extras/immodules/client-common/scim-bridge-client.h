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

#ifndef SCIMBRIDGECLIENT_H_
#define SCIMBRIDGECLIENT_H_

#include "scim-bridge.h"
#include "scim-bridge-key-event.h"
#include "scim-bridge-client-imcontext.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Initialize the client.
     *
     * @return RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAL_FAILED.
     */
    retval_t scim_bridge_client_initialize ();

    /**
     * Finalize the client.
     *
     * @return RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAL_FAILED.
     */
    retval_t scim_bridge_client_finalize ();

    /**
     * Check if the client is initialized.
     *
     * @return TRUE if it's iniitalized, otherwise it returns FALSE.
     */
    boolean scim_bridge_client_is_initialized ();

    /**
     * Open the connection with the agent.
     *
     * @return RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAL_FAILED.
     */
    retval_t scim_bridge_client_open_messenger ();

    /**
     * Close the connection with the agent.
     *
     * @return RETVAIL_SUCCEEDED if succeeded, otherwise it returns RETVAL_FAILED.
     */
    retval_t scim_bridge_client_close_messenger ();

    /**
     * Check if the connection is active.
     *
     * @return TRUE if it's active, otherwise it returns FALSE.
     */
    boolean scim_bridge_client_is_messenger_opened ();

    /**
     * Get the mesenger socket file discriptor of the client.
     *
     * @return The socket file descriptor of the client.
     */
    int scim_bridge_client_get_messenger_fd ();


    /**
     * See if the reconnection feature is enabled.
     * The client try to establish a new connection after the connection breaks if this feature is enabled.
     *
     * @return TRUE if this feature is enabled.
     */
    boolean scim_bridge_client_is_reconnection_enabled ();


    /**
     * Read a message from the socket, and dispatch it.
     * If no message is available, it returns immediately.
     *
     * @return RETVAL_SUCCEEDED if no error has occurred, otherwise it returns RETVAL_FAILED.
     */
    retval_t scim_bridge_client_read_and_dispatch ();


    /**
     * Register an imcontext into the agent.
     *
     * @param imcontext The pointer to initialize as an imcontext.
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_register_imcontext (ScimBridgeClientIMContext *imcontext);

    /**
     * Deregister an imcontext from the agent.
     *
     * @param imcontext The imcontext to deregister.
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_deregister_imcontext (ScimBridgeClientIMContext *imcontext);

    /**
     * Find the IMContext which has given id.
     *
     * @param id The IMContext. (This function returns NULL if -1 is given here)
     * @return The IMContext if it's been found out, otherwise it returns NULL.
     */
    ScimBridgeClientIMContext *scim_bridge_client_find_imcontext (scim_bridge_imcontext_id_t id);

    /**
     * Reset an imcontext.
     *
     * @param imcontext The imcontext.
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_reset_imcontext (const ScimBridgeClientIMContext *imcontext);

    /**
     * Enable/Disalbe an imcontext.
     *
     * @param imcontext The imcontext.
     * @param enabled Give it TRUE to enable imcontext , False to disable imcontext
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_set_imcontext_enabled (const ScimBridgeClientIMContext *imcontext, boolean enabled);

    /**
     * Change the focusing status of an imcontext.
     *
     * @param imcontext The imcontext.
     * @param focus_in Give it TRUE if it gains the focus, otherwise give it FALSE.
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_change_focus (const ScimBridgeClientIMContext *imcontext, boolean focus_in);

    /**
     * Handle a key event.
     *
     * @param imcontext The imcontext.
     * @param key_event The key event.
     * @paam consumed The pointer for the flag, which is set TRUE if you should not handle this event in the client.
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_handle_key_event (const ScimBridgeClientIMContext *imcontext, const ScimBridgeKeyEvent *key_event, boolean *consumed);

    /**
     * Notify the change of cursor location in the display.
     *
     * @param imcontext The imcontext.
     * @param x The x location of the cursor.
     * @param y The y location of the cursor.
     * @return RETVAIL_SUCCEDED if succeeded, otherwise it returns RETVAIL_FAILED.
     */
    retval_t scim_bridge_client_set_cursor_location (const ScimBridgeClientIMContext *imcontext, int x, int y);

    /**
     * Set the way to show the preedit.
     *
     * @param imcontext The imcontext.
     * @param mode The way to show the preedit.
     */
    retval_t scim_bridge_client_set_preedit_mode (const ScimBridgeClientIMContext *imcontext, scim_bridge_preedit_mode_t mode);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGECLIENT_H_*/
