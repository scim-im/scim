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
 * @brief This header describes abut fucntions used for sending and receving messages.
 */
#ifndef SCIMBRIDGEMESSAGENGER_H_
#define SCIMBRIDGEMESSAGENGER_H_

#include "scim-bridge.h"
#include "scim-bridge-message.h"

/**
 * The struct type of Messenger.
 */
typedef struct _ScimBridgeMessenger ScimBridgeMessenger;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Allocate a new messenger.
     *
     * @param socket_fd The file discriptor for the socket.
     * @return The new messenger.
     */
    ScimBridgeMessenger *scim_bridge_alloc_messenger (int socket_fd);

    /**
     * Free a messenger, and close the socket if it's still open.\n
     * Do not use "free ()" for variables for this type.
     *
     * @param messenger The messenger.
     */
    void scim_bridge_free_messenger (ScimBridgeMessenger *messenger);

    /**
     * Get the file descriptor which assosicates with this messenger.
     *
     * @param messenger The messenger.
     * @return The assosicated file descriptor.
     */
    int scim_bridge_messenger_get_socket_fd (const ScimBridgeMessenger *messenger);

    /**
     * Close the socket of the messenger.
     *
     * @param messenger The messenger.
     * @return RETVAL_SUCCEEDED if no error has been occurred, otherwise it return RETVAL_FAILED.
     */
    retval_t scim_bridge_close_messenger (ScimBridgeMessenger *messenger);

    /**
     * Check if the messenger is closed.
     *
     * @param messenger The messeger
     * @return TRUE if closed, otherwise it returns FALSE.
     */
    boolean scim_bridge_messenger_is_closed (const ScimBridgeMessenger *messenger);

    /**
     * Push a messenge into the sending buffer.
     *
     * @param messenger The messenger.
     * @param message The message to send.
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it return RETVAL_FAILED.
     */
    retval_t scim_bridge_messenger_push_message (ScimBridgeMessenger *messenger, const ScimBridgeMessage *message);

    /**
     * Poll a messenge from the received buffer.
     *
     * @param messenger The messenger.
     * @param message The pointer for the received message. It returns NULL if no message is available.
     * @return RETVAL_SUCCEEDED if it succeeded, otherwise it return RETVAL_FAILED.
     */
    retval_t scim_bridge_messenger_poll_message (ScimBridgeMessenger *messenger, ScimBridgeMessage **message);

    /**
     * Send the oldest stored message if it can.
     *
     * @param messenger The messenger.
     * @param timeout The maximum time to block. It will blocks until all the stored messages is sent if you give NULL here.
     * @return RETVAL_SUCCEEDED if no error has been occurred, otherwise it return RETVAL_FAILED.
     */
    retval_t scim_bridge_messenger_send_message (ScimBridgeMessenger *messenger, const struct timeval *timeout);

    /**
     * Receive a message and store it.
     *
     * @param messenger The messenger.
     * @param timeout The maximum time to block. It will blocks until one whole message is available if you give NULL here.
     * @return RETVAL_SUCCEEDED if no error has been occurred, otherwise it return RETVAL_FAILED.
     */
    retval_t scim_bridge_messenger_receive_message (ScimBridgeMessenger *messenger, const struct timeval *timeout);

    /**
     * Get the size of sendig buffer.
     * it implies that there is a message to send if the retval is greater thatn 0.
     *
     * @param messenger The messenger.
     * @return The size of messages to send.
     */
    ssize_t scim_bridge_messenger_get_sending_buffer_size (const ScimBridgeMessenger *messenger);

    /**
     * Get the size of receiving buffer.
     * It implies that there might be a message to poll if the retval is greater than 0.
     *
     * @param messenger The messenger.
     * @return The size of received messages.
     */
    ssize_t scim_bridge_messenger_get_receiving_buffer_size (const ScimBridgeMessenger *messenger);

#ifdef __cplusplus
}
#endif
#endif                                            /*SCIMBRIDGEMESSAGENGER_H_*/
