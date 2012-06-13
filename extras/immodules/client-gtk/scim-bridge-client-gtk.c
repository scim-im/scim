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

#include <unistd.h>

#include <sys/select.h>
#include <sys/types.h>

#include <glib.h>

#include <gtk/gtk.h>

#include "scim-bridge-client.h"
#include "scim-bridge-client-gtk.h"
#include "scim-bridge-client-imcontext-gtk.h"
#include "scim-bridge-client-protected.h"
#include "scim-bridge-imcontext.h"
#include "scim-bridge-output.h"

/* Private Variables */
static GIOChannel *messenger_iochannel = NULL;
static guint messenger_event_source = -1;

static boolean initialized = FALSE;


/* Private Functions */
static gboolean handle_message (GIOChannel *source, GIOCondition condition, gpointer data)
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
            return FALSE;
        }
    }

    return TRUE;
}


/* Public Functions */
void scim_bridge_client_gtk_initialize ()
{
    scim_bridge_pdebugln (5, "scim_bridge_client_gtk_initialize ()");

    if (initialized) {
        return;
    } else {
        initialized = TRUE;
    }

    if (scim_bridge_client_initialize ()) {
        scim_bridge_perrorln ("Failed to initialize scim-bridge...");
    } else {
        scim_bridge_client_open_messenger ();
    }

    scim_bridge_client_imcontext_static_initialize ();
}


void scim_bridge_client_gtk_finalize ()
{
    scim_bridge_pdebugln (5, "scim_bridge_client_gtk_finalize ()");

    if (!initialized) {
        return;
    } else {
        initialized = FALSE;
    }

    scim_bridge_client_finalize ();
    scim_bridge_client_imcontext_static_finalize ();
}


void scim_bridge_client_messenger_opened ()
{
    if (messenger_iochannel == NULL) {
        messenger_iochannel = g_io_channel_unix_new (scim_bridge_client_get_messenger_fd ());
        messenger_event_source = g_io_add_watch (messenger_iochannel, G_IO_IN | G_IO_ERR | G_IO_HUP | G_IO_NVAL, &handle_message, NULL);
    }

    scim_bridge_client_imcontext_connection_opened ();
}


void scim_bridge_client_messenger_closed ()
{
    if (messenger_iochannel != NULL) {
        g_io_channel_unref (messenger_iochannel);
        messenger_iochannel = NULL;
        g_source_remove (messenger_event_source);
        messenger_event_source = -1;
    }

    scim_bridge_client_imcontext_connection_closed ();
}

