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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "scim-bridge-output.h"

#include "scim-bridge-agent-interruption-listener.h"

/* Class definition */
class ScimBridgeAgentInterruptionListenerImpl: public ScimBridgeAgentInterruptionListener
{

    public:

        ScimBridgeAgentInterruptionListenerImpl ();
        ~ScimBridgeAgentInterruptionListenerImpl ();

        retval_t initialize ();

        int get_socket_fd () const;

        scim_bridge_agent_event_type_t get_trigger_events () const;

        bool handle_event (scim_bridge_agent_event_type_t event_type);

        bool is_interrupted () const;

        void interrupt ();

        void clear_interruption ();

    private:

        bool interrupted;

        int pipe_in;
        int pipe_out;

};

/* Implementations */
ScimBridgeAgentInterruptionListener *ScimBridgeAgentInterruptionListener::alloc ()
{

    ScimBridgeAgentInterruptionListenerImpl *interruption_listener = new ScimBridgeAgentInterruptionListenerImpl ();
    if (interruption_listener->initialize ()) {
        delete interruption_listener;
        interruption_listener = NULL;
    }

    return interruption_listener;
}


ScimBridgeAgentInterruptionListenerImpl::ScimBridgeAgentInterruptionListenerImpl (): interrupted (false), pipe_in (-1), pipe_out (-1)
{
}


ScimBridgeAgentInterruptionListenerImpl::~ScimBridgeAgentInterruptionListenerImpl ()
{
    if (pipe_in >= 0) close (pipe_in);
    if (pipe_out >= 0) close (pipe_out);
}


retval_t ScimBridgeAgentInterruptionListenerImpl::initialize ()
{
    int pipe_pair[2];

    if (socketpair (PF_UNIX, SOCK_STREAM, 0, pipe_pair)) {
        scim_bridge_perrorln ("Cannot make a pipe for a interruption listener: %s", strerror (errno));
        return RETVAL_FAILED;
    }

    pipe_out = pipe_pair[0];
    pipe_in = pipe_pair[1];
    scim_bridge_pdebugln (2, "The interruption pipe: (%d, %d)", pipe_in, pipe_out);

    return RETVAL_SUCCEEDED;
}


int ScimBridgeAgentInterruptionListenerImpl::get_socket_fd () const
{
    return pipe_out;
}


scim_bridge_agent_event_type_t ScimBridgeAgentInterruptionListenerImpl::get_trigger_events () const
{
    return SCIM_BRIDGE_AGENT_EVENT_READ;
}


bool ScimBridgeAgentInterruptionListenerImpl::handle_event (scim_bridge_agent_event_type_t event_type)
{
    return true;
}


void ScimBridgeAgentInterruptionListenerImpl::interrupt ()
{
    scim_bridge_pdebugln (1, "An interruption occurred");
    if (interrupted) return;

    interrupted = true;
    if (send (pipe_in, " ", sizeof (char), MSG_NOSIGNAL | MSG_DONTWAIT) < 0 && errno != EAGAIN) {
        scim_bridge_perrorln ("Failed to make an interruption: %s", strerror (errno));
    }
}


bool ScimBridgeAgentInterruptionListenerImpl::is_interrupted () const
{
    return interrupted;
}


void ScimBridgeAgentInterruptionListenerImpl::clear_interruption ()
{
    scim_bridge_pdebugln (1, "The interruption is cleared");
    char c;
    recv (pipe_out, &c, sizeof (char), MSG_DONTWAIT);
    interrupted = false;
}
