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

#include "scim-bridge-client.h"
#include "scim-bridge-client-protected.h"
#include "scim-bridge-output.h"

#include "scim-bridge-client-qt.h"

#ifdef QT4
using namespace Qt;
#endif

/* Static variables */
static ScimBridgeClientQt *client = NULL;


/* Bindings */
void scim_bridge_client_messenger_opened ()
{
    client->messenger_opened ();
}


void scim_bridge_client_messenger_closed ()
{
    client->messenger_closed ();
}


/* Implementations */
ScimBridgeClientQt::ScimBridgeClientQt (): socket_notifier (NULL) {
    client = this;

    if (scim_bridge_client_initialize ()) {
        scim_bridge_perrorln ("Failed to init scim bridge...");
    } else {
        scim_bridge_client_open_messenger ();
    }
    ScimBridgeClientIMContext::static_initialize ();
}


ScimBridgeClientQt::~ScimBridgeClientQt () {
    if (scim_bridge_client_finalize ()) {
        scim_bridge_perrorln ("Failed to finalize scim bridge...");
    }

    ScimBridgeClientIMContext::static_finalize ();

    client = NULL;
}


void ScimBridgeClientQt::messenger_opened ()
{
    const int fd = scim_bridge_client_get_messenger_fd ();
    socket_notifier = new QSocketNotifier (fd, QSocketNotifier::Read);
    connect (socket_notifier, SIGNAL (activated (int)), this, SLOT (handle_message ()));

    ScimBridgeClientIMContext::connection_opened ();
}


void ScimBridgeClientQt::messenger_closed ()
{
    if (socket_notifier) {
        socket_notifier->setEnabled (false);
        socket_notifier->deleteLater ();
        socket_notifier = NULL;
    }

    ScimBridgeClientIMContext::connection_closed ();
}


void ScimBridgeClientQt::handle_message ()
{
    const int socket_fd = scim_bridge_client_get_messenger_fd ();

    fd_set read_set;
    FD_ZERO (&read_set);
    FD_SET (socket_fd, &read_set);

    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    if (select (socket_fd + 1, &read_set, NULL, NULL, &timeout) > 0) {
        if (scim_bridge_client_read_and_dispatch ()) {
            scim_bridge_perrorln ("An IOException occurred at handle_message ()");
            return;
        }
    }
}

