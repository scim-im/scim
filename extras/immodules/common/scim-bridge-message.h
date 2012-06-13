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
 * @brief This header describes abut the data type of messages.
 */
#ifndef SCIMBRIDGEMESSAGE_H_
#define SCIMBRIDGEMESSAGE_H_

#include "scim-bridge.h"

/**
 * The struct type of message.
 */
typedef struct _ScimBridgeMessage ScimBridgeMessage;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Allocate a message.
     * All the arguments are set to '\0'.
     *
     * @param header The header of the message.
     * @param argument_count The number of arguments.
     * @return The new message.
     */
    ScimBridgeMessage *scim_bridge_alloc_message (const char *header, size_t argument_count);

    /**
     * Free a message.
     *
     * @param message The message to free.
     */
    void scim_bridge_free_message (ScimBridgeMessage *message);

    /**
     * Get the header of a message.
     *
     * @param message The message.
     * @return The header of the message.
     */
    const char *scim_bridge_message_get_header (const ScimBridgeMessage *message);

    /**
     * Get the number of message arguments.
     *
     * @param message The message.
     * @return The argument count of the message.
     */
    size_t scim_bridge_message_get_argument_count (const ScimBridgeMessage *message);

    /**
     * Get the specific argument of the message.
     *
     * @param message The message.
     * @param index The index of the argument.
     * @return The argument if the index is valid. Otherwise it returns NULL.
     */
    const char *scim_bridge_message_get_argument (const ScimBridgeMessage *message, size_t index);

    /**
     * Set the specific argument of the message.
     *
     * @param message The message.
     * @param index The index of the argument. If invalid index is give, it fails.
     * @param argument The new argument value. If NULL is give, it fails.
     * @return RETVAL_SUCCEEDED if succeeded, otherwise it returns RETVAL_FAILED.
     */
    retval_t scim_bridge_message_set_argument (ScimBridgeMessage *message, size_t index, const char *argument);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEMESSAGE_H_*/
