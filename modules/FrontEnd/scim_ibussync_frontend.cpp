/** @file scim_ibussync_frontend.cpp
 * @brief A FrontEnd module that keeps the enabled SCIM engine set and GNOME's
 *        input-source list in sync.
 *
 * Unlike an input frontend it processes no keys; it exists so the engine-list
 * coordinator can run inside the same daemon as the backend (typically loaded
 * as "scim -f socket,ibussync"). That lets it read the installed engines
 * straight from the shared backend and re-apply the disabled list on the very
 * backend the SocketFrontEnd serves to a relay-mode ibus.so -- no separate
 * process, no socket round-trip.
 *
 * The reconciliation logic lives in ScimIBusSync (GFileMonitor on the config +
 * GSettings watch on org.gnome.desktop.input-sources). Those are GLib sources,
 * so the frontend drives the default GMainContext: run() spins a GMainLoop when
 * loaded alone, and poll_fds()/process_events() embed the context in the shared
 * cooperative select() loop when co-loaded with other frontends.
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

#define Uses_SCIM_FRONTEND
#define Uses_SCIM_BACKEND
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_CONFIG_PATH
#include <scim.h>

#include <glib.h>
#include <ibus.h>
#include "scim_ibus_sync.h"

#include <set>
#include <vector>

using namespace scim;

#define scim_module_init                    ibussync_LTX_scim_module_init
#define scim_module_exit                    ibussync_LTX_scim_module_exit
#define scim_frontend_module_init           ibussync_LTX_scim_frontend_module_init
#define scim_frontend_module_run            ibussync_LTX_scim_frontend_module_run
#define scim_frontend_module_poll_fds       ibussync_LTX_scim_frontend_module_poll_fds
#define scim_frontend_module_process_events ibussync_LTX_scim_frontend_module_process_events
#define scim_frontend_module_has_exited     ibussync_LTX_scim_frontend_module_has_exited

class IBusSyncFrontEnd : public FrontEndBase
{
    ConfigPointer            m_config;
    IBusBus                 *m_bus;
    ScimIBusSync            *m_sync;
    GMainLoop               *m_loop;
    bool                     m_should_exit;

    // State for embedding the default GMainContext in a foreign poll loop.
    std::vector<GPollFD>     m_gpollfds;
    gint                     m_gpoll_max_priority;
    gint                     m_gpoll_nfds;
    bool                     m_gpoll_prepared;

public:
    IBusSyncFrontEnd (const BackEndPointer &backend, const ConfigPointer &config)
        : FrontEndBase (backend),
          m_config (config),
          m_bus (0),
          m_sync (0),
          m_loop (0),
          m_should_exit (false),
          m_gpollfds (8),
          m_gpoll_max_priority (0),
          m_gpoll_nfds (0),
          m_gpoll_prepared (false)
    {
    }

    virtual ~IBusSyncFrontEnd ()
    {
        delete m_sync;
        if (m_bus)
            g_object_unref (m_bus);
        g_main_context_release (g_main_context_default ());
    }

    virtual void init (int /*argc*/, char ** /*argv*/)
    {
        ibus_init ();
        m_bus = ibus_bus_new ();

        // Own the default context so prepare/check/dispatch may be called from
        // poll_fds()/process_events() (single-threaded; no other owner).
        g_main_context_acquire (g_main_context_default ());

        m_sync = new ScimIBusSync (
            [this] () { return compute_all_installed (); },
            [this] () {
                // Ask the socket daemon first: it holds the only real backend,
                // so reconciling ours alone would just re-filter a factory list
                // that is itself stale. The reload command travels over the
                // config connection, and the daemon reconciles its factories as
                // part of handling it.
                if (!m_config.null ())
                    m_config->reload ();
                reload_disabled_factories ();
            });
        m_sync->start ();
    }

    // Loaded alone: just run the GLib loop the sync watches live on.
    virtual void run ()
    {
        m_should_exit = false;
        m_loop = g_main_loop_new (0, FALSE);
        g_main_loop_run (m_loop);
        g_main_loop_unref (m_loop);
        m_loop = 0;
    }

    // Co-loaded: hand our GLib fds to the shared select() loop, and dispatch
    // ready sources when it wakes.
    bool poll_fds (std::vector<int> &fds)
    {
        GMainContext *ctx = g_main_context_default ();
        gint timeout = 0;

        g_main_context_prepare (ctx, &m_gpoll_max_priority);

        gint n = g_main_context_query (ctx, m_gpoll_max_priority, &timeout,
                                       m_gpollfds.data (), (gint) m_gpollfds.size ());
        if (n > (gint) m_gpollfds.size ()) {
            m_gpollfds.resize (n);
            n = g_main_context_query (ctx, m_gpoll_max_priority, &timeout,
                                      m_gpollfds.data (), (gint) m_gpollfds.size ());
        }
        m_gpoll_nfds     = n;
        m_gpoll_prepared = true;

        for (gint i = 0; i < n; ++i)
            fds.push_back (m_gpollfds[i].fd);
        return n > 0;
    }

    void process_events ()
    {
        if (!m_gpoll_prepared)
            return;
        m_gpoll_prepared = false;

        GMainContext *ctx = g_main_context_default ();

        // The shared loop woke on some fd but does not tell us which; re-poll
        // our own fds without blocking to fill in the revents check() needs.
        if (m_gpoll_nfds > 0)
            g_poll (m_gpollfds.data (), m_gpoll_nfds, 0);

        if (g_main_context_check (ctx, m_gpoll_max_priority,
                                  m_gpollfds.data (), m_gpoll_nfds))
            g_main_context_dispatch (ctx);
    }

    bool has_exited () const { return m_should_exit; }

private:
    // Every engine SCIM registers with ibus carries our markers, so the ibus
    // registry itself tells us which engines are ours --
    // and ibus.so builds that registry by walking the IMEngine modules
    // directly, without consulting the disabled list. It is therefore the
    // authoritative "everything installed" set, enabled or not.
    //
    // Deriving it from our own backend instead would be wrong here: this
    // process reaches factories through the socket proxy, whose factory list is
    // fixed when the module loads. An engine enabled afterwards is in neither
    // that list nor the disabled list, so it fell out of the union entirely and
    // could never be pushed to GNOME.
    //
    // Empty (safely) when ibus is unreachable -- ScimIBusSync then bails.
    std::vector<String> compute_all_installed ()
    {
        std::vector<String> result;

        if (!m_bus || !ibus_bus_is_connected (m_bus))
            return result;

        GList *engines = ibus_bus_list_engines (m_bus);

        for (GList *p = engines; p; p = p->next) {
            IBusEngineDesc *desc = IBUS_ENGINE_DESC (p->data);
            const gchar    *name = ibus_engine_desc_get_name (desc);
            const gchar    *dom  = ibus_engine_desc_get_textdomain (desc);

            if (name && *name && dom &&
                !g_strcmp0 (dom, SCIM_IBUS_ENGINE_TEXTDOMAIN))
                result.push_back (String (name));
        }

        g_list_foreach (engines, (GFunc) g_object_unref, 0);
        g_list_free (engines);

        return result;
    }
};

static Pointer <IBusSyncFrontEnd> _scim_frontend (0);

extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_FRONTEND (1) << "Initializing IBusSync FrontEnd module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_FRONTEND (1) << "Exiting IBusSync FrontEnd module...\n";
        _scim_frontend.reset ();
    }

    void scim_frontend_module_init (const BackEndPointer &backend,
                                    const ConfigPointer &config,
                                    int argc,
                                    char **argv)
    {
        if (config.null () || backend.null ())
            throw FrontEndError (String ("IBusSync FrontEnd couldn't run without Config and BackEnd.\n"));

        if (_scim_frontend.null ()) {
            _scim_frontend = new IBusSyncFrontEnd (backend, config);
            _scim_frontend->init (argc, argv);
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ())
            _scim_frontend->run ();
    }

    bool scim_frontend_module_poll_fds (std::vector<int> &fds)
    {
        return !_scim_frontend.null () ? _scim_frontend->poll_fds (fds) : false;
    }

    void scim_frontend_module_process_events (void)
    {
        if (!_scim_frontend.null ())
            _scim_frontend->process_events ();
    }

    bool scim_frontend_module_has_exited (void)
    {
        return !_scim_frontend.null () ? _scim_frontend->has_exited () : false;
    }
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
