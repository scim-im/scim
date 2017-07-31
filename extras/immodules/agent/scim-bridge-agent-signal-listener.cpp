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
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#include "scim-bridge-output.h"

#include "scim-bridge-agent-protected.h"
#include "scim-bridge-agent-signal-listener.h"

/* Static variable */
static bool signal_occurred = FALSE;

static int pipe_out = -1;
static int pipe_in = -1;

/* Class definition */
class ScimBridgeAgentSignalListenerImpl: public ScimBridgeAgentSignalListener
{

    public:

        ScimBridgeAgentSignalListenerImpl (ScimBridgeAgentProtected *new_agent);
        ~ScimBridgeAgentSignalListenerImpl ();

        int get_socket_fd () const;

        scim_bridge_agent_event_type_t get_trigger_events () const;

        bool handle_event (scim_bridge_agent_event_type_t event_type);

    private:

        ScimBridgeAgentProtected *agent;

};

/* Helper functions */
static void sig_quit (int sig)
{
    if (!signal_occurred) {
        signal_occurred = true;
        send (pipe_in, "", sizeof (char), MSG_NOSIGNAL);
    }
}


static void static_finalize ()
{
    if (pipe_out >= 0) close (pipe_out);
    if (pipe_in >= 0) close (pipe_in);
}


/* Implementations */
ScimBridgeAgentSignalListener *ScimBridgeAgentSignalListener::alloc (ScimBridgeAgentProtected *new_agent)
{
    static bool first_time = true;

    if (first_time) {
        int pipe_pair[2];
        if (socketpair (PF_UNIX, SOCK_STREAM, 0, pipe_pair)) {
            scim_bridge_perrorln ("Cannot make a pipe for the signal listener: %s", strerror (errno));
            return NULL;
        }

        pipe_out = pipe_pair[0];
        pipe_in = pipe_pair[1];

        struct sigaction quit_action;
        quit_action.sa_handler = &sig_quit;
        quit_action.sa_flags = 0;
        sigemptyset (&quit_action.sa_mask);
        sigaction (SIGTERM, &quit_action, NULL);
        sigaction (SIGINT, &quit_action, NULL);
        sigaction (SIGQUIT, &quit_action, NULL);
        sigaction (SIGHUP, &quit_action, NULL);

        atexit (&static_finalize);

        ScimBridgeAgentSignalListenerImpl *signal_listener = new ScimBridgeAgentSignalListenerImpl (new_agent);

        first_time = false;
        return signal_listener;
    } else {
        return NULL;
    }
}


ScimBridgeAgentSignalListenerImpl::ScimBridgeAgentSignalListenerImpl (ScimBridgeAgentProtected *new_agent): agent (new_agent)
{
}


ScimBridgeAgentSignalListenerImpl::~ScimBridgeAgentSignalListenerImpl ()
{
}


int ScimBridgeAgentSignalListenerImpl::get_socket_fd () const
{
    return pipe_in;
}


scim_bridge_agent_event_type_t ScimBridgeAgentSignalListenerImpl::get_trigger_events () const
{
    return SCIM_BRIDGE_AGENT_EVENT_READ;
}


bool ScimBridgeAgentSignalListenerImpl::handle_event (scim_bridge_agent_event_type_t event_type)
{
    if (signal_occurred) agent->quit ();
    return true;
}
