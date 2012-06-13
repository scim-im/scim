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
 * @brief This is the common header of IMContext over the all clients.
 */

#ifndef SCIMBRIDGECLIENTIMCONTEXT_H_
#define SCIMBRIDGECLIENTIMCONTEXT_H_

#include "scim-bridge.h"
#include "scim-bridge-attribute.h"
#include "scim-bridge-imcontext.h"
#include "scim-bridge-key-event.h"

/**
 * The struct of IMContext.
 */
typedef struct _ScimBridgeClientIMContext ScimBridgeClientIMContext;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Set the id of an IMContext.
     *
     * @param imcontext The IMContext.
     * @param new_id The new id.
     */
    void scim_bridge_client_imcontext_set_id (ScimBridgeClientIMContext *imcontext, scim_bridge_imcontext_id_t new_id);

    /**
     * Get the id of an IMContext.
     * This function should return -1 if it's not been registered to the agent.
     *
     * @param imcontext The IMContext.
     * @return The id of the IMContext.
     */
    scim_bridge_imcontext_id_t scim_bridge_client_imcontext_get_id (const ScimBridgeClientIMContext *imcontext);

    /**
     * Set the preedit string of an IMContext.
     *
     * @param imcontext The IMContext.
     * @param preedit_string The preedit string encoded in UTF8.
     */
    void scim_bridge_client_imcontext_set_preedit_string (ScimBridgeClientIMContext *imcontext, const char *preedit_string);

    /**
     * Set the visibility of the preedit of an IMContext.
     *
     * @param imcontext The IMContext.
     * @param preedit_shown The visibility of the preedit.
     */
    void scim_bridge_client_imcontext_set_preedit_shown (ScimBridgeClientIMContext *imcontext, boolean preedit_shown);

    /**
     * Set the cursor position (= caret index) in the preedit of an IMContext.
     *
     * @param imcontext The IMContext.
     * @param cursor_position The cursor position.
     */
    void scim_bridge_client_imcontext_set_preedit_cursor_position (ScimBridgeClientIMContext *imcontext, int cursor_position);

    /**
     * Set the attributes (= appearance) of the preedit string.
     *
     * @param imcontext The IMContext.
     * @param preedit_attributes The array of the attributes.
     * @param attribute_count The number of attributes.
     */
    void scim_bridge_client_imcontext_set_preedit_attributes (ScimBridgeClientIMContext *imcontext, ScimBridgeAttribute** const preedit_attributes, int attribute_count);

    /**
     * Update the preedit of an IMContext.
     *
     * @param imcontext The IMContext.
     */
    void scim_bridge_client_imcontext_update_preedit (ScimBridgeClientIMContext *imcontext);

    /**
     * Set the commit string of an IMContext.
     *
     * @param imcontext The IMContext.
     * @param commit_string The commit string encoded in UTF8.
     */
    void scim_bridge_client_imcontext_set_commit_string (ScimBridgeClientIMContext *imcontext, const char *commit_string);

    /**
     * Commit a string.
     *
     * @param imcontext The IMContext.
     */
    void scim_bridge_client_imcontext_commit (ScimBridgeClientIMContext *imcontext);

    /**
     * Make a beep sound.
     *
     * @param imcontext The IMContext.
     */
    void scim_bridge_client_imcontext_beep (ScimBridgeClientIMContext *imcontext);

    /**
     * Add an dummy event into the GUI event queue.
     *
     * @param imcontext The IMContext.
     * @param key_event The dummy key event.
     */
    void scim_bridge_client_imcontext_forward_key_event (ScimBridgeClientIMContext *imcontext, const ScimBridgeKeyEvent *key_event);

    /**
     * Get the surrounding text of an IMContext.
     *
     * @param imcontext The IMContext.
     * @param before_max The maximum number of characters (in wchar) to fetch before the cursor (= caret).
     * @param after_max The maximum number of characters (in wchar) to fetch after the cursor (= caret).
     * @param string The pointer for the gotten string.
     * @param cursor_position The cursor position in the gotten string (in wchar).
     * @return It returns TRUE if succeeded to get it, otherwise it returns FALSE.
     */
    boolean scim_bridge_client_imcontext_get_surrounding_text (ScimBridgeClientIMContext *imcontext, int before_max, int after_max, char **string, int *cursor_position);

    /**
     * Delete the surrounding text of an IMContext.
     * The string reffered as "the surrounding text" here is the string gotten by get_surrounding_text () previously.
     *
     * @param imcontext The IMContext.
     * @param offset The begining offset (in wchar) of the part of the surrounding string to remove.
     * @param length The length (in wchar) of the part of the surrounding string to remove.
     */
    boolean scim_bridge_client_imcontext_delete_surrounding_text (ScimBridgeClientIMContext *imcontext, int offset, int length);

    /**
     * Replace the surrounding text of an IMContext.
     * The string reffered as "the surrounding text" here is the string gotten by get_surrounding_text () previously.
     *
     * @param imcontext The IMContext.
     * @param cursor_position The cursor position (in wchar) in the new surrounding string.
     * @param string The new surrounding text encoded in UTF8.
     */
    boolean scim_bridge_client_imcontext_replace_surrounding_text (ScimBridgeClientIMContext *imcontext, int cursor_position, const char *string);
    
    /**
     * Notify the status of the IMEngine has been changed.
     *
     * @param imcontext The IMContext.
     * @param enabled The new status.
     */
    void scim_bridge_client_imcontext_imengine_status_changed (ScimBridgeClientIMContext *imcontext, boolean enabled);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGECLIENTIMCONTEXT_H_*/
