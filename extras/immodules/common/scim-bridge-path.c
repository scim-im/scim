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

#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>

#include "scim-bridge-display.h"
#include "scim-bridge-path.h"

/* Private variables */
static const char COMPAT_VERSION[] = "0.3.0";

static const char SOCKET_DIR[] = "/tmp/";
static const char SOCKET_NAME[] = "socket";

static const char LOCKFILE_DIR[] = "/tmp/";
static const char LOCKFILE_NAME[] = "lockfile";

static const char AGENT_DIR[] = "";
static const char AGENT_NAME[] = "scim-bridge";

static const char HOST_NAME[] = "localhost";

static char *lockfile_path = NULL;
static char *socket_path = NULL;
static char *agent_path = NULL;

/* Private function */
static void static_initialize ()
{
    ScimBridgeDisplay *display = scim_bridge_alloc_display ();
    scim_bridge_display_fetch_current (display);

    const int display_number = scim_bridge_display_get_display_number (display);
    const int screen_number = scim_bridge_display_get_screen_number (display);

    scim_bridge_free_display (display);

    const uid_t uid = geteuid ();
    const size_t lockfile_path_length = snprintf (NULL, 0, "%s%s-%s.%s-%d@%s:%d.%d", LOCKFILE_DIR, AGENT_NAME, COMPAT_VERSION, LOCKFILE_NAME, uid, HOST_NAME, display_number, screen_number);
    lockfile_path = malloc (sizeof (char) * (lockfile_path_length + 1));
    sprintf (lockfile_path, "%s%s-%s.%s-%d@%s:%d.%d", LOCKFILE_DIR, AGENT_NAME, COMPAT_VERSION, LOCKFILE_NAME, uid, HOST_NAME, display_number, screen_number);

    const size_t socket_path_length = snprintf (NULL, 0, "%s%s-%s.%s-%d@%s:%d.%d", SOCKET_DIR, AGENT_NAME, COMPAT_VERSION, SOCKET_NAME, uid, HOST_NAME, display_number, screen_number);
    socket_path = malloc (sizeof (char) * (socket_path_length + 1));
    sprintf (socket_path, "%s%s-%s.%s-%d@%s:%d.%d", SOCKET_DIR, AGENT_NAME, COMPAT_VERSION, SOCKET_NAME, uid, HOST_NAME, display_number, screen_number);

    const size_t agent_path_length = snprintf (NULL, 0, "%s%s", AGENT_DIR, AGENT_NAME);
    agent_path = malloc (sizeof (char) * (agent_path_length + 1));
    sprintf (agent_path, "%s%s", AGENT_DIR, AGENT_NAME);
}


/* Implementations */
const char *scim_bridge_path_get_lockfile ()
{
    if (lockfile_path == NULL) static_initialize ();
    return lockfile_path;
}


const char *scim_bridge_path_get_socket ()
{
    if (socket_path == NULL) static_initialize ();
    return socket_path;
}


const char *scim_bridge_path_get_agent ()
{
    if (agent_path == NULL) static_initialize ();
    return agent_path;
}
