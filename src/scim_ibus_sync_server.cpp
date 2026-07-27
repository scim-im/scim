/** @file scim_ibus_sync_server.cpp
 * @brief Persistent coordinator that keeps the enabled SCIM engine set and the
 *        GNOME/ibus input-source list in sync.
 *
 * Launched (and supervised) by the scim autostart on a GNOME/ibus session. It
 * is the ONLY continuously-running watcher: ibus.so is started and killed by
 * ibus-daemon, so it cannot reliably watch the config or D-Bus -- it instead
 * reloads lazily on demand.
 *
 * The set of installed SCIM engines is derived from TWO authoritative sources,
 * reconciled by UUID (a SCIM factory UUID never collides with another engine's
 * name):
 *
 *   - what ibus actually knows:  ibus_bus_list_engines() over D-Bus. This is
 *     the only 100%-correct view of ibus's registry -- ibus-daemon loaded its
 *     components under its own environment, which a file read from here could
 *     not reproduce. But it lists EVERY engine (xkb, other IMEs, ...) with no
 *     "this is SCIM" marker.
 *   - which of those are ours:  scim's own enumeration, obtained by running
 *     `scim -f ibus --xml` (the same command that generates the component
 *     manifest). Spawned on demand so the persistent coordinator stays light
 *     and always sees freshly-installed engines, and so the list reflects
 *     scim's environment rather than a stale on-disk file.
 *
 * all-installed = { e in ibus_bus_list_engines() : e.name is a SCIM uuid }.
 */

/*
 * Smart Common Input Method
 *
 * Copyright (c) 2026 SCIM developers
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_UTILITY
#define Uses_SCIM_SOCKET
#define Uses_SCIM_TRANSACTION
#include <scim.h>

#include <glib.h>
#include <glib/gstdio.h>
#include <ibus.h>
#include "scim_ibus_sync.h"

#include <set>
#include <vector>
#include <iostream>
#include <unistd.h>

using namespace scim;

#ifndef SCIM_BINDIR
#define SCIM_BINDIR  "/usr/bin"
#endif

// scim's own authoritative list of installed SCIM engine uuids, obtained by
// running the same enumeration that generates the ibus component manifest.
// Env override SCIM_PROGRAM for testing / non-standard installs.
static std::vector<String>
read_our_uuids ()
{
    std::vector<String> uuids;

    const char *env = getenv ("SCIM_PROGRAM");
    String prog = (env && *env) ? String (env) : String (SCIM_BINDIR "/scim");

    const char *argv[] = { prog.c_str (), "-f", "ibus", "--xml", 0 };

    gchar *out = 0;
    GError *err = 0;
    gboolean ok = g_spawn_sync (0, (gchar **) argv, 0,
                                G_SPAWN_STDERR_TO_DEV_NULL, 0, 0,
                                &out, 0, 0, &err);
    if (!ok) {
        if (err) g_error_free (err);
        return uuids;
    }

    // Parse the emitted component XML with the ibus parser (robust; correctly
    // ignores the component-level <name> and reads only the <engine> names).
    if (out && *out) {
        gchar *tmp = 0;
        gint fd = g_file_open_tmp ("scim-ibus-sync-XXXXXX.xml", &tmp, 0);
        if (fd >= 0) {
            close (fd);
            if (g_file_set_contents (tmp, out, -1, 0)) {
                IBusComponent *comp = ibus_component_new_from_file (tmp);
                if (comp) {
                    GList *engines = ibus_component_get_engines (comp);
                    for (GList *p = engines; p; p = p->next) {
                        const gchar *name =
                            ibus_engine_desc_get_name (IBUS_ENGINE_DESC (p->data));
                        if (name && *name)
                            uuids.push_back (String (name));
                    }
                    g_list_free (engines);
                    g_object_unref (comp);
                }
            }
            g_unlink (tmp);
        }
        g_free (tmp);
    }
    g_free (out);
    return uuids;
}

// Ask a running SocketFrontEnd (the warm shared backend) for its factory list
// over the socket -- no fork, no engine loading, just a round-trip. Sets
// @reachable when a daemon actually answered the handshake, so the caller can
// fall back to the fork-based enumeration when no warm daemon is running.
static std::vector<String>
read_our_uuids_via_daemon (bool &reachable)
{
    std::vector<String> uuids;
    reachable = false;

    SocketAddress address;
    address.set_address (scim_get_default_socket_frontend_address ());

    SocketClient client;
    if (!client.connect (address))
        return uuids;

    uint32 magic = 0;
    if (!scim_socket_open_connection (magic, String ("SocketIMEngine"),
                                      String ("SocketFrontEnd"), client, 1000)) {
        client.close ();
        return uuids;
    }
    reachable = true;

    Transaction trans;
    trans.clear ();
    trans.put_command (SCIM_TRANS_CMD_REQUEST);
    trans.put_data (magic);
    trans.put_command (SCIM_TRANS_CMD_GET_FACTORY_LIST);
    trans.put_data (String (""));   // all encodings

    if (trans.write_to_socket (client)) {
        int cmd;
        if (!(trans.read_from_socket (client, 5000) &&
              trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_REPLY &&
              trans.get_data (uuids) &&
              trans.get_command (cmd) && cmd == SCIM_TRANS_CMD_OK))
            uuids.clear ();
    }

    client.close ();
    return uuids;
}

// The installed SCIM engines that ibus actually knows: every engine ibus lists
// whose name is one of ours. Empty (safely) when ibus-daemon is unreachable or
// scim's enumeration fails -- ScimIBusSync then bails rather than wiping state.
static std::vector<String>
all_installed_uuids (IBusBus *bus)
{
    std::vector<String> result;

    if (!bus || !ibus_bus_is_connected (bus))
        return result;

    // Prefer the warm daemon (a socket round-trip); fall back to a `scim -f ibus
    // --xml` fork only when no SocketFrontEnd is running.
    bool reachable = false;
    std::vector<String> our = read_our_uuids_via_daemon (reachable);
    if (!reachable)
        our = read_our_uuids ();
    if (our.empty ())
        return result;
    std::set<String> ours (our.begin (), our.end ());

    GList *engines = ibus_bus_list_engines (bus);
    for (GList *p = engines; p; p = p->next) {
        const gchar *name = ibus_engine_desc_get_name (IBUS_ENGINE_DESC (p->data));
        if (name && *name && ours.count (String (name)))
            result.push_back (String (name));
    }
    g_list_free_full (engines, g_object_unref);

    return result;
}

int
main (int argc, char *argv[])
{
    (void) argc;
    (void) argv;

    ibus_init ();

    IBusBus *bus = ibus_bus_new ();

    std::cerr << "scim-ibus-sync: " << all_installed_uuids (bus).size ()
              << " installed SCIM engine(s) known to ibus.\n";

    ScimIBusSync sync (
        [bus] () { return all_installed_uuids (bus); },  // ibus list, filtered to ours
        [] () { });                                      // reload: no-op (ibus.so reloads lazily)

    sync.start ();

    GMainLoop *loop = g_main_loop_new (0, FALSE);
    g_main_loop_run (loop);
    g_main_loop_unref (loop);

    if (bus) g_object_unref (bus);
    return 0;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
