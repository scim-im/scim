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

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

#include "scim-bridge-output.h"
#include "scim-bridge-path.h"

#include "scim-bridge-agent-protected.h"
#include "scim-bridge-agent-accept-listener.h"
#include "scim-bridge-agent-client-listener.h"

/* Class definition */
class ScimBridgeAgentAcceptListenerImpl: public ScimBridgeAgentAcceptListener
{

    public:

        ScimBridgeAgentAcceptListenerImpl (ScimBridgeAgentProtected *new_agent);
        ~ScimBridgeAgentAcceptListenerImpl ();

        retval_t initialize ();

        int get_socket_fd () const;

        scim_bridge_agent_event_type_t get_trigger_events () const;

        bool handle_event (scim_bridge_agent_event_type_t event_type);

    private:

        ScimBridgeAgentProtected *agent;

        int socket_fd;

};

/* Implementations */
ScimBridgeAgentAcceptListener *ScimBridgeAgentAcceptListener::alloc (ScimBridgeAgentProtected *agent)
{
    ScimBridgeAgentAcceptListenerImpl *socket_listener = new ScimBridgeAgentAcceptListenerImpl (agent);

    if (socket_listener->initialize ()) {
        delete socket_listener;
        return NULL;
    } else {
        return socket_listener;
    }
}


ScimBridgeAgentAcceptListenerImpl::ScimBridgeAgentAcceptListenerImpl (ScimBridgeAgentProtected *new_agent): agent (new_agent), socket_fd (-1)
{
}


ScimBridgeAgentAcceptListenerImpl::~ScimBridgeAgentAcceptListenerImpl ()
{
    if (socket_fd >= 0) {
        close (socket_fd);
        socket_fd = -1;

        const char *socket_path = scim_bridge_path_get_socket ();
        unlink (socket_path);
    }
}


retval_t ScimBridgeAgentAcceptListenerImpl::initialize ()
{
    const char *socket_path = scim_bridge_path_get_socket ();

    socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        scim_bridge_perrorln ("Cannot create an unix domain socket: %s", strerror (errno));
        return RETVAL_FAILED;
    }

    struct sockaddr_un socket_addr;
    memset (&socket_addr, 0, sizeof (struct sockaddr_un));
    socket_addr.sun_family = AF_UNIX;
    strncpy (socket_addr.sun_path, socket_path, sizeof (socket_addr.sun_path) - 1);
    
    const int MAX_TRIAL = 5;
    for (int i = 0; i < MAX_TRIAL; ++i) {
        scim_bridge_pdebugln (8, "Pinging for the another agent process...");
        if (connect (socket_fd, (struct sockaddr*)&socket_addr, SUN_LEN (&socket_addr))) {
            if (i == MAX_TRIAL - 1) {
                scim_bridge_pdebugln (6, "It seems like there is no other agent for the old socket.");
                break;
            } else {
                switch (errno) {
                    case EAGAIN:
                    case EINTR:
                    case EALREADY:
                    case EINPROGRESS:
                        scim_bridge_pdebugln (1, "Opps, no retval from the socket!");
                        usleep (50000);
                        break;
                    case EACCES:
                    case EPERM:
                    case ECONNREFUSED:
                    case ENOTSOCK:
                    default:
                        scim_bridge_pdebugln (6, "It seems like there is no other agent for the old socket.");
                        i = MAX_TRIAL;
                        break;
                }
            }
        } else {
            scim_bridge_pdebugln (9, "It seems like there is already another agent process...");
            socket_fd = -1;
            return RETVAL_FAILED;
        }
    }
    close (socket_fd);
    unlink (socket_path);
    
    socket_fd = socket (PF_UNIX, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        scim_bridge_perrorln ("Cannot create an unix domain socket: %s", strerror (errno));
        return RETVAL_FAILED;
    }

    const int socket_flags = fcntl (socket_fd, F_GETFL);
    if (socket_flags < 0) {
        scim_bridge_perrorln ("Failed to get the flags of the socket");
        close (socket_fd);
        socket_fd = -1;
        return RETVAL_FAILED;
    }
    if (fcntl (socket_fd, F_SETFL, socket_flags | O_NONBLOCK)) {
        scim_bridge_perrorln ("Failed to set the flags of the socket");
        close (socket_fd);
        socket_fd = -1;
        return RETVAL_FAILED;
    }
    
    if (bind (socket_fd, (struct sockaddr*)&socket_addr, SUN_LEN (&socket_addr)) != 0) {
        scim_bridge_perrorln ("Cannot bind the socket: %s", strerror (errno));
        close (socket_fd);
        socket_fd = -1;
        return RETVAL_FAILED;
    }

    if (listen (socket_fd, 64)) {
        scim_bridge_perrorln ("Cannot start listening the socket: %s", strerror (errno));
        close (socket_fd);
        socket_fd = -1;
        return RETVAL_FAILED;
    }
    return RETVAL_SUCCEEDED;
}


int ScimBridgeAgentAcceptListenerImpl::get_socket_fd () const
{
    return socket_fd;
}


scim_bridge_agent_event_type_t ScimBridgeAgentAcceptListenerImpl::get_trigger_events () const
{
    return SCIM_BRIDGE_AGENT_EVENT_READ | SCIM_BRIDGE_AGENT_EVENT_ERROR;
}


bool ScimBridgeAgentAcceptListenerImpl::handle_event (scim_bridge_agent_event_type_t event_type)
{
    if (event_type & SCIM_BRIDGE_AGENT_EVENT_ERROR) {
        //scim_bridge_perrorln ("FIXME: A critical error: %s", strerror (errno));
        //agent->quit ();
    } else {
        struct sockaddr_un empty_socket_addr;
        socklen_t empty_socket_size = sizeof (empty_socket_addr);

        int fd = accept (socket_fd, (sockaddr*) (&empty_socket_addr), &empty_socket_size);
        if (fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                scim_bridge_perrorln ("Opps, it seems like failed to establish a connection with a client...");
                usleep (50);
            } else {
                scim_bridge_perrorln ("FIXME: A critical error occurred on the socket: %s", strerror (errno));
                agent->quit ();
            }
        } else {
            scim_bridge_pdebugln (5, "A connection established.");
            agent->add_client_listener (ScimBridgeAgentClientListener::alloc (fd, agent));
        }
    }

    return true;
}
