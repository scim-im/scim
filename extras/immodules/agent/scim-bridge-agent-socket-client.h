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
 * @brief This is the header file for ScimBridgeAgentSocketClient.
 */

#ifndef SCIMBRIDGEAGENTSOCKETCLIENT_H_
#define SCIMBRIDGEAGENTSOCKETCLIENT_H_

#include "scim-bridge.h"

/**
 * The type value for IDs of event clients.
 */
typedef int scim_bridge_agent_event_client_id_t;

/**
 * The type value for types of sockets.
 */
typedef unsigned int scim_bridge_agent_event_type_t;

/**
 * There is no event at all.
 */
static const scim_bridge_agent_event_type_t SCIM_BRIDGE_AGENT_EVENT_NONE = 0;

/**
 * The socket is ready for reading.
 */
static const scim_bridge_agent_event_type_t SCIM_BRIDGE_AGENT_EVENT_READ = 1 << 0;

/**
 * The socket is ready for writing.
 */
static const scim_bridge_agent_event_type_t SCIM_BRIDGE_AGENT_EVENT_WRITE = 1 << 1;

/**
 * The socket caused some errors.
 */
static const scim_bridge_agent_event_type_t SCIM_BRIDGE_AGENT_EVENT_ERROR = 1 << 2;

/**
 * The client has caught an interruption.
 */
static const scim_bridge_agent_event_type_t SCIM_BRIDGE_AGENT_EVENT_INTERRUPT = 1 << 3;

/**
 * The class of clients of a socket server.
 */
class ScimBridgeAgentSocketClient
{

    public:

        /**
         * Destructor.
         * Close the socket if it's still opened.
         */
        virtual ~ScimBridgeAgentSocketClient () {}

        /**
         * Get the file discriptor for the client.
         * The file discriptor is used for pooling in the main event loop.
         *
         * @return The file discriptor for the client.
         */
        virtual int get_socket_fd () const = 0;

        /**
         * Get the wakeup condition of the client.
         * This value is used for selecting the clients in the event loop.
         * handle_events () is called when the given type of events are occurred.
         *
         * @return The wakeup conditions.
         */
        virtual scim_bridge_agent_event_type_t get_trigger_events () const = 0;

        /**
         * Handle an event in the socket.
         * This function is called when a event is occurred upon the socket.
         *
         * @param event_type The type of this event.
         * @return If this client should be kept in the event loop.
         */
        virtual bool handle_event (scim_bridge_agent_event_type_t event_type) = 0;

    protected:

        /**
         * Constructor.
         */
        ScimBridgeAgentSocketClient () {}

};
#endif                                            /*SCIMBRIDGEAGENTSOCKETCLIENT_H_*/
