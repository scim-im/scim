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
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#define Uses_SCIM_FRONTEND
#define Uses_SCIM_BACKEND
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_HOTKEY
#define Uses_SCIM_CONFIG_BASE
#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_UTILITY
#define Uses_SCIM_DEBUG
#define Uses_C_STRING

#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#include <sys/select.h>
#include <poll.h>

#include "scim_private.h"
#include "scim.h"

#include "scim_wayland_frontend.h"

#include <iostream>

#define scim_module_init           wayland_LTX_scim_module_init
#define scim_module_exit           wayland_LTX_scim_module_exit
#define scim_frontend_module_init  wayland_LTX_scim_frontend_module_init
#define scim_frontend_module_run   wayland_LTX_scim_frontend_module_run
#define scim_frontend_module_poll_fds       wayland_LTX_scim_frontend_module_poll_fds
#define scim_frontend_module_process_events wayland_LTX_scim_frontend_module_process_events
#define scim_frontend_module_has_exited     wayland_LTX_scim_frontend_module_has_exited

// Shared with wayland.so: the language override applies to either protocol
// generation, and only one of the two modules ever runs on a given desktop.
#define SCIM_CONFIG_FRONTEND_WAYLAND_LANGUAGE  "/FrontEnd/Wayland/Language"

// Shown by the panel while the engine is off.
#define SCIM_KEYBOARD_ICON_FILE                (SCIM_ICONDIR "/keyboard.png")

using namespace scim;

//Local static data
static Pointer <WaylandFrontEnd> _scim_frontend (0);

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Initializing Wayland FrontEnd module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Exiting Wayland FrontEnd module...\n";
        _scim_frontend.reset ();
    }

    void scim_frontend_module_init (const BackEndPointer &backend,
                                    const ConfigPointer &config,
                                    int argc,
                                    char **argv)
    {
        if (config.null () || backend.null ())
            throw FrontEndError (String ("Wayland FrontEnd couldn't run without Config and BackEnd.\n"));

        if (_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Initializing Wayland FrontEnd module (more)...\n";
            _scim_frontend = new WaylandFrontEnd (backend, config);
            _scim_frontend->init (argc, argv);
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Starting Wayland FrontEnd module...\n";
            _scim_frontend->run ();
        }
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

/* ------------------------------------------------------------------ */
/* Listener tables                                                     */
/* ------------------------------------------------------------------ */

static const struct wl_registry_listener registry_listener = {
    WaylandFrontEnd::handle_registry_global,
    WaylandFrontEnd::handle_registry_global_remove
};

static const struct zwp_input_method_v1_listener v1_im_listener = {
    WaylandFrontEnd::handle_v1_activate,
    WaylandFrontEnd::handle_v1_deactivate
};

static const struct zwp_input_method_v2_listener v2_im_listener = {
    WaylandFrontEnd::handle_v2_activate,
    WaylandFrontEnd::handle_v2_deactivate,
    WaylandFrontEnd::handle_v2_surrounding_text,
    WaylandFrontEnd::handle_v2_text_change_cause,
    WaylandFrontEnd::handle_v2_content_type,
    WaylandFrontEnd::handle_v2_done,
    WaylandFrontEnd::handle_v2_unavailable
};

static const struct zwp_input_method_keyboard_grab_v2_listener v2_grab_listener = {
    WaylandFrontEnd::handle_grab_keymap,
    WaylandFrontEnd::handle_grab_key,
    WaylandFrontEnd::handle_grab_modifiers,
    WaylandFrontEnd::handle_grab_repeat_info
};

static const struct zwp_input_method_context_v1_listener v1_context_listener = {
    WaylandFrontEnd::handle_ctx_surrounding_text,
    WaylandFrontEnd::handle_ctx_reset,
    WaylandFrontEnd::handle_ctx_content_type,
    WaylandFrontEnd::handle_ctx_invoke_action,
    WaylandFrontEnd::handle_ctx_commit_state,
    WaylandFrontEnd::handle_ctx_preferred_language
};

static const struct wl_keyboard_listener v1_kb_listener = {
    WaylandFrontEnd::handle_kb_keymap,
    WaylandFrontEnd::handle_kb_enter,
    WaylandFrontEnd::handle_kb_leave,
    WaylandFrontEnd::handle_kb_key,
    WaylandFrontEnd::handle_kb_modifiers,
    WaylandFrontEnd::handle_kb_repeat_info
};

/* ------------------------------------------------------------------ */
/* Construction                                                        */
/* ------------------------------------------------------------------ */

WaylandFrontEnd::WaylandFrontEnd (const BackEndPointer &backend,
                                      const ConfigPointer  &config)
    : FrontEndBase (backend),
      m_config (config),
      m_instance (-1),
      m_focused (false),
      m_instance_warned (false),
      m_im_on (false),
      m_password_field (false),
      m_valid_key_mask (SCIM_KEY_AllMasks),
      m_display (0),
      m_registry (0),
      m_seat (0),
      m_compositor (0),
      m_shm (0),
      m_seat_name (0),
      m_proto (PROTO_NONE),
      m_forced_proto (PROTO_NONE),
      m_v1_input_method (0),
      m_v1_input_panel (0),
      m_v1_context (0),
      m_v1_keyboard (0),
      m_v2_manager (0),
      m_v2_input_method (0),
      m_v2_grab (0),
      m_vk_manager (0),
      m_virtual_keyboard (0),
      m_serial (0),
      m_preedit_caret (0),
      m_preedit_shown (false),
      m_on_the_spot (true),
      m_xkb_context (0),
      m_xkb_keymap (0),
      m_xkb_state (0),
      m_have_current_key (false),
      m_current_key_serial (0),
      m_current_key_time (0),
      m_current_key_code (0),
      m_current_key_state (0),
      m_current_key_forwarded (false),
      m_should_exit (false),
      // Not under an #ifdef: the member is declared unconditionally, and
      // select_sink () both reads and dereferences it before anything assigns
      // one. Initialising it only where kimpanel was built left a build without
      // it (--disable-kimpanel, or a host with no libdbus) starting up on an
      // indeterminate pointer -- silently, since the file still compiles.
      m_sink (0),
      m_sink_selected (false),
      m_panel_open (false)
{
    if (!_scim_frontend.null () && _scim_frontend != this)
        throw FrontEndError (String ("Wayland -- only one frontend can be created!"));
}

WaylandFrontEnd::~WaylandFrontEnd ()
{
    m_config_reload_connection.disconnect ();
    if (m_instance >= 0)
        delete_instance (m_instance);

    if (m_xkb_state)    xkb_state_unref (m_xkb_state);
    if (m_xkb_keymap)   xkb_keymap_unref (m_xkb_keymap);
    if (m_xkb_context)  xkb_context_unref (m_xkb_context);

    drop_keyboard ();
    if (m_v1_context)       zwp_input_method_context_v1_destroy (m_v1_context);
    if (m_v1_input_panel)   zwp_input_panel_v1_destroy (m_v1_input_panel);
    if (m_v1_input_method)  zwp_input_method_v1_destroy (m_v1_input_method);

    if (m_v2_grab)          zwp_input_method_keyboard_grab_v2_release (m_v2_grab);
    if (m_virtual_keyboard) zwp_virtual_keyboard_v1_destroy (m_virtual_keyboard);
    if (m_v2_input_method)  zwp_input_method_v2_destroy (m_v2_input_method);
    if (m_v2_manager)       zwp_input_method_manager_v2_destroy (m_v2_manager);
    if (m_vk_manager)       zwp_virtual_keyboard_manager_v1_destroy (m_vk_manager);
    if (m_seat)          wl_seat_destroy (m_seat);
    if (m_registry)      wl_registry_destroy (m_registry);

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // Before the display goes. The candidates UI owns wl_ proxies of its own --
    // its surface, the popup role object and a wl_pointer -- and being a member
    // its destructor runs *after* this body, by which point
    // wl_display_disconnect () has freed everything those proxies refer to.
    // Destroying them there is a use-after-free on every exit that runs
    // destructors at all, which is what turned a compositor going away into a
    // segfault in wl_pointer_destroy () rather than an orderly shutdown.
    // finish () is idempotent, so the member destructor is left with nothing.
    m_candidates_ui.finish ();
#endif

    if (m_display)       wl_display_disconnect (m_display);
}

/* ------------------------------------------------------------------ */
/* Init / run                                                          */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::init (int argc, char **argv)
{
    // Escape hatch for testing against a compositor that speaks both, or for
    // reproducing a bug on the other generation. Detection is the default.
    for (int i = 0; i < argc; ++i) {
        if (!argv [i]) continue;
        if (!strcmp (argv [i], "--im-protocol=v1"))
            m_forced_proto = PROTO_V1;
        else if (!strcmp (argv [i], "--im-protocol=v2"))
            m_forced_proto = PROTO_V2;
    }

    reload_config_callback (m_config);
    m_config_reload_connection =
        m_config->signal_connect_reload (slot (this, &WaylandFrontEnd::reload_config_callback));

    m_display = wl_display_connect (0);
    if (!m_display)
        throw FrontEndError (String ("Wayland -- cannot connect to a Wayland display!"));

    m_registry = wl_display_get_registry (m_display);
    wl_registry_add_listener (m_registry, &registry_listener, this);

    // Two roundtrips: the first delivers the globals, the second any
    // dependent state.
    wl_display_roundtrip (m_display);
    wl_display_roundtrip (m_display);

    // Pick the generation the compositor actually offers. v2 wins when both are
    // present: it is the protocol still being developed, and it can anchor
    // candidates to the text cursor, which v1 gives us no way to do.
    if (m_forced_proto == PROTO_V1 && m_v1_input_method)
        m_proto = PROTO_V1;
    else if (m_forced_proto == PROTO_V2 && m_v2_manager)
        m_proto = PROTO_V2;
    else if (m_forced_proto != PROTO_NONE)
        throw FrontEndError (String ("Wayland -- the input-method protocol requested "
                                     "with --im-protocol is not offered by this compositor."));
    else if (m_v2_manager)
        m_proto = PROTO_V2;
    else if (m_v1_input_method)
        m_proto = PROTO_V1;
    else
        throw FrontEndError (String ("Wayland -- compositor offers neither "
                                     "zwp_input_method_manager_v2 nor zwp_input_method_v1 "
                                     "(not supported on this desktop)."));

    if (m_proto == PROTO_V2 && !m_seat)
        throw FrontEndError (String ("Wayland -- no wl_seat available."));

    // Refuse v2 without a virtual keyboard rather than starting half-working.
    // The grab below routes every key to us, and on v2 the only way back to the
    // application is zwp_virtual_keyboard_v1: with no manager to create one,
    // proto_forward_key () has nothing to send through and silently drops the
    // lot. That is not a missing feature, it is a dead keyboard the user cannot
    // even type their way out of -- the trigger hotkey turns the engine off and
    // the keys still go nowhere. Declining here leaves the launcher to fall back
    // to x11.so, which at least serves XWayland clients.
    if (m_proto == PROTO_V2 && !m_vk_manager)
        throw FrontEndError (String ("Wayland -- the compositor offers "
                                     "zwp_input_method_manager_v2 but no "
                                     "zwp_virtual_keyboard_manager_v1; there would be "
                                     "no way to return unconsumed keys to the "
                                     "application."));

    m_xkb_context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    if (!m_xkb_context)
        throw FrontEndError (String ("Wayland -- cannot create xkb context."));

    if (m_proto == PROTO_V1) {
        // Nothing to create up front: the compositor hands us a context when a
        // text field takes focus. Just start listening.
        zwp_input_method_v1_add_listener (m_v1_input_method, &v1_im_listener, this);
    } else {
        // v2 has one long-lived input-method object for the seat, and the
        // keyboard grab is taken once rather than per activation.
        m_v2_input_method =
            zwp_input_method_manager_v2_get_input_method (m_v2_manager, m_seat);
        zwp_input_method_v2_add_listener (m_v2_input_method, &v2_im_listener, this);

        m_virtual_keyboard =
            zwp_virtual_keyboard_manager_v1_create_virtual_keyboard (m_vk_manager, m_seat);

        // Grab only once there is a route back to the application; see above.
        if (!m_virtual_keyboard)
            throw FrontEndError (String ("Wayland -- cannot create a virtual keyboard."));

        proto_start_grab ();
    }

    // Bring up both candidate UIs that this build has, then let select_sink ()
    // decide which is in use. The overlay panel is created even when kimpanel is
    // taking the updates, so a panel widget removed later leaves something to
    // fall back to.
#ifdef SCIM_HAS_KIMPANEL
    bool want_kimpanel =
        m_config->read (String ("/Panel/UseKimpanel"),
                        KimpanelAgent::desktop_prefers_kimpanel ());
    if (want_kimpanel && m_kimpanel.connect ()) {
        // Clicking the indicator does what the trigger hotkey does. kimpanel
        // has no menu of ours to open, so a toggle is the useful action.
        m_kimpanel.signal_connect_trigger_engine (
            [this] () { if (m_im_on) turn_off_im (); else turn_on_im (); });
        m_kimpanel.signal_connect_panel_presence_changed (
            [this] (bool) { select_sink (); });
    }
#endif

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (proto_candidates_init ())
        configure_candidates_ui ();
    else
        SCIM_DEBUG_FRONTEND (1) << "Wayland -- no input panel surface; "
                                   "aux/candidates will not be shown.\n";
#endif

    select_sink ();

    ensure_instance ();

    // Start in whatever state the user left the input method in. The default is
    // off, so enabling SCIM as the compositor's input method never costs the
    // user their plain-ASCII keyboard until they ask for the engine.
    m_im_on = m_config->read (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT),
                              false);

    wl_display_roundtrip (m_display);

    // Status/property UI. Harmless if no panel is available.
    panel_open ();

    SCIM_DEBUG_FRONTEND (1) << "Wayland -- ready: protocol=" << proto_name ()
                            << " siid=" << m_instance
                            << " im_on=" << m_im_on
                            << " panel=" << m_panel_open
                            << " input_panel=" << (m_v1_input_panel ? 1 : 0)
                            << " seat=" << (m_seat ? 1 : 0) << "\n";
}

/* ------------------------------------------------------------------ */
/* Panel (status / properties)                                         */
/* ------------------------------------------------------------------ */

bool
WaylandFrontEnd::panel_open ()
{
    // Launches scim-panel-gtk on demand. Non-fatal: input works without it, we
    // just lose the status/property UI.
    if (m_panel_client.open_connection (m_config->get_name ()) < 0) {
        SCIM_DEBUG_FRONTEND(1) << "Wayland -- cannot connect to the SCIM panel.\n";
        m_panel_open = false;
        return false;
    }

    m_panel_client.signal_connect_trigger_property (
        slot (this, &WaylandFrontEnd::panel_slot_trigger_property));
    m_panel_client.signal_connect_reload_config (
        slot (this, &WaylandFrontEnd::panel_slot_reload_config));
    m_panel_client.signal_connect_change_factory (
        slot (this, &WaylandFrontEnd::panel_slot_change_factory));
    m_panel_client.signal_connect_request_factory_menu (
        slot (this, &WaylandFrontEnd::panel_slot_request_factory_menu));

    m_panel_open = true;
    return true;
}

void
WaylandFrontEnd::panel_close ()
{
    m_panel_client.close_connection ();
    m_panel_open = false;
}

void
WaylandFrontEnd::panel_slot_trigger_property (int context, const String &property)
{
    // The user picked a property in the tray menu or on the toolbar.
    if (context == m_instance)
        trigger_property (context, property);
}

void
WaylandFrontEnd::panel_slot_reload_config (int /*context*/)
{
    m_config->reload ();
}

void
WaylandFrontEnd::panel_slot_change_factory (int context, const String &uuid)
{
    if (context != m_instance)
        return;
    // An empty uuid is the panel's "turn off" entry, which is how the user gets
    // back to plain ASCII from the tray menu instead of the trigger hotkey.
    if (!uuid.length ())
        turn_off_im ();
    else
        switch_factory (uuid);
}

void
WaylandFrontEnd::panel_slot_request_factory_menu (int context)
{
    // The panel is asking which engines exist -- for its tray menu, or to fill
    // the list it shows on the toolbar. Only the frontend knows, so without this
    // reply the panel's engine list stays empty and there is no way to switch
    // engines from the menu at all.
    if (context == m_instance)
        panel_req_show_factory_menu ();
}

void
WaylandFrontEnd::register_properties (int id, const PropertyList &properties)
{
    if (m_panel_open && id == m_instance) {
        m_panel_client.prepare (id);
        m_panel_client.register_properties (id, properties);
        m_panel_client.send ();
    }
}

void
WaylandFrontEnd::update_property (int id, const Property &property)
{
    if (m_panel_open && id == m_instance) {
        m_panel_client.prepare (id);
        m_panel_client.update_property (id, property);
        m_panel_client.send ();
    }
}

bool
WaylandFrontEnd::poll_fds (std::vector<int> &fds)
{
    if (m_display)
        fds.push_back (wl_display_get_fd (m_display));
    if (m_panel_open) {
        int pfd = m_panel_client.get_connection_number ();
        if (pfd >= 0)
            fds.push_back (pfd);
    }
    if (m_sink) {
        int sink_fd = m_sink->event_fd ();
        if (sink_fd >= 0)
            fds.push_back (sink_fd);
    }
#ifdef SCIM_HAS_KIMPANEL
    // Watched even while kimpanel is not the sink in use: this is the connection
    // a panel widget's arrival is announced on.
    if (m_kimpanel.is_connected () && m_sink != &m_kimpanel) {
        int kfd = m_kimpanel.event_fd ();
        if (kfd >= 0)
            fds.push_back (kfd);
    }
#endif
    return true;
}

void
WaylandFrontEnd::process_events ()
{
    // Drain the panel socket first; a dead connection is reopened so the panel
    // (or its tray item) can come and go without taking the frontend with it.
    if (m_panel_open && m_panel_client.has_pending_event ()) {
        if (!m_panel_client.filter_event ()) {
            panel_close ();
            panel_open ();
            // A restarted panel knows nothing about us, so tell it again --
            // otherwise its tray item stays nameless and iconless until the
            // next focus change.
            if (m_panel_open && m_instance >= 0 && m_focused) {
                m_panel_client.prepare (m_instance);
                m_panel_client.register_input_context (m_instance, get_instance_uuid (m_instance));
                m_panel_client.focus_in (m_instance, get_instance_uuid (m_instance));
                if (m_im_on) m_panel_client.turn_on  (m_instance);
                else         m_panel_client.turn_off (m_instance);
                m_panel_client.send ();
                panel_req_update_factory_info ();
            }
        }
    }

    if (m_sink)
        m_sink->process_events ();

#ifdef SCIM_HAS_KIMPANEL
    if (m_kimpanel.is_connected () && m_sink != &m_kimpanel)
        m_kimpanel.process_events ();
#endif

    if (!m_display)
        return;

    // Non-blocking dispatch: dispatch anything already queued, then read from
    // the fd only if data is actually available (this method may be called by
    // the shared loop when another frontend's fd woke the select).
    if (wl_display_prepare_read (m_display) == 0) {
        struct pollfd pfd;
        pfd.fd = wl_display_get_fd (m_display);
        pfd.events = POLLIN;
        pfd.revents = 0;
        if (poll (&pfd, 1, 0) > 0 && (pfd.revents & POLLIN))
            wl_display_read_events (m_display);
        else
            wl_display_cancel_read (m_display);
    } else {
        wl_display_dispatch_pending (m_display);
    }

    if (wl_display_dispatch_pending (m_display) < 0) {
        std::cerr << "Wayland -- exiting: wayland dispatch failed: "
                  << strerror (errno) << "\n";
        m_should_exit = true;
        return;
    }
    if (wl_display_flush (m_display) < 0 && errno != EAGAIN) {
        std::cerr << "Wayland -- exiting: wayland flush failed: "
                  << strerror (errno) << "\n";
        m_should_exit = true;
    }
}

void
WaylandFrontEnd::run ()
{
    if (!m_display)
        return;

    m_should_exit = false;

    while (!m_should_exit) {
        process_events ();
        if (m_should_exit)
            break;

        std::vector<int> fds;
        poll_fds (fds);
        if (fds.empty ()) {
            std::cerr << "Wayland -- exiting: nothing left to poll.\n";
            break;
        }

        fd_set read_fds;
        FD_ZERO (&read_fds);
        int max_fd = -1;
        for (size_t i = 0; i < fds.size (); ++i) {
            FD_SET (fds[i], &read_fds);
            if (fds[i] > max_fd) max_fd = fds[i];
        }

        if (select (max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            if (errno == EINTR)
                continue;
            std::cerr << "Wayland -- exiting: select failed: "
                      << strerror (errno) << "\n";
            break;
        }
    }
}

void
WaylandFrontEnd::reload_config_callback (const ConfigPointer &config)
{
    m_language = config->read (String (SCIM_CONFIG_FRONTEND_WAYLAND_LANGUAGE),
                               String (""));

    // Shared with x11.so, so the preference applies to a session however its
    // applications happen to be reaching us.
    bool on_the_spot =
        config->read (String (SCIM_CONFIG_FRONTEND_ON_THE_SPOT), true);
    if (on_the_spot != m_on_the_spot) {
        m_on_the_spot = on_the_spot;
        // Drop whatever the path we just left is still showing.
        if (m_on_the_spot) preedit_to_panel (false);
        else               proto_clear_preedit ();
    }

    m_frontend_hotkey_matcher.load_hotkeys (config);
    m_imengine_hotkey_matcher.load_hotkeys (config);

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // The candidate appearance is part of the configuration too, and this was the
    // only part of it applied once at startup and never again: a font, color or
    // shape changed in scim-setup did nothing until the session was restarted,
    // however faithfully the reload itself arrived.
    configure_candidates_ui ();
#endif

    KeyEvent mask_key;
    scim_string_to_key (mask_key,
        config->read (String (SCIM_CONFIG_HOTKEYS_FRONTEND_VALID_KEY_MASK),
                      String ("Shift+Control+Alt+Lock")));
    m_valid_key_mask = (mask_key.mask > 0) ? mask_key.mask
                                           : (uint16) SCIM_KEY_AllMasks;
    m_valid_key_mask |= SCIM_KEY_ReleaseMask;

    // The SCIM_DEBUG macros filter by mask/level themselves, so this costs
    // only the hotkey lookup when debugging is off.
    KeyEventList hk;
    m_frontend_hotkey_matcher.find_hotkeys (SCIM_FRONTEND_HOTKEY_TRIGGER, hk);
    SCIM_DEBUG_FRONTEND (2) << "Wayland -- valid key mask 0x"
                            << std::hex << m_valid_key_mask << std::dec
                            << ", " << hk.size () << " trigger hotkey(s).\n";
    for (size_t i = 0; i < hk.size (); ++i) {
        String ks;
        scim_key_to_string (ks, hk [i]);
        SCIM_DEBUG_FRONTEND (2) << "  trigger: " << ks << "\n";
    }
}

/* ------------------------------------------------------------------ */
/* Instance helper                                                      */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::ensure_instance ()
{
    if (m_instance >= 0)
        return;

    String encoding = String ("UTF-8");
    String language = m_language;
    if (!language.length ())
        language = scim_get_locale_language (scim_get_current_locale ());

    String sfid = get_default_factory (language, encoding);
    m_instance = new_instance (sfid, encoding);

    if (m_instance >= 0) {
        m_instance_warned = false;
        return;
    }

    // Reachable when the backend cannot name a factory yet -- an empty factory
    // repository is the only way get_default_factory () returns nothing. That
    // happens when we start before the session backend can serve its engine
    // list, so it is transient and every caller retries. Say so once: silence
    // here reads as "the input method is running" while nothing works.
    if (!m_instance_warned) {
        m_instance_warned = true;
        std::cerr << "Wayland -- no IMEngine instance yet (the backend has no "
                     "factories); retrying.\n";
    }
    SCIM_DEBUG_FRONTEND(1) << "Wayland -- failed to create an IMEngine instance.\n";
}

/* ------------------------------------------------------------------ */
/* Hotkeys                                                             */
/* ------------------------------------------------------------------ */

bool
WaylandFrontEnd::filter_hotkeys (const KeyEvent &scimkey)
{
    bool ok = false;

    // Both matchers see every event, presses and releases alike: the default
    // next/previous-factory bindings are release-triggered
    // ("Control+Shift+Shift_L+KeyRelease"), so dropping releases here would
    // make them unmatchable.
    m_frontend_hotkey_matcher.push_key_event (scimkey);
    m_imengine_hotkey_matcher.push_key_event (scimkey);

    FrontEndHotkeyAction action = m_frontend_hotkey_matcher.get_match_result ();

    if (action == SCIM_FRONTEND_HOTKEY_TRIGGER) {
        if (m_im_on) turn_off_im ();
        else         turn_on_im ();
        ok = true;
    } else if (action == SCIM_FRONTEND_HOTKEY_ON) {
        if (!m_im_on) turn_on_im ();
        ok = true;
    } else if (action == SCIM_FRONTEND_HOTKEY_OFF) {
        if (m_im_on) turn_off_im ();
        ok = true;
    } else if (action == SCIM_FRONTEND_HOTKEY_NEXT_FACTORY) {
        if (m_instance >= 0)
            switch_factory (get_next_factory (String (""), String ("UTF-8"),
                                              get_instance_uuid (m_instance)));
        ok = true;
    } else if (action == SCIM_FRONTEND_HOTKEY_PREVIOUS_FACTORY) {
        if (m_instance >= 0)
            switch_factory (get_previous_factory (String (""), String ("UTF-8"),
                                                  get_instance_uuid (m_instance)));
        ok = true;
    } else if (action == SCIM_FRONTEND_HOTKEY_SHOW_FACTORY_MENU) {
        panel_req_show_factory_menu ();
        ok = true;
    } else if (m_imengine_hotkey_matcher.is_matched ()) {
        switch_factory (m_imengine_hotkey_matcher.get_match_result ());
        ok = true;
    }

    return ok;
}

void
WaylandFrontEnd::turn_on_im ()
{
    if (m_im_on || m_instance < 0)
        return;

    m_im_on = true;
    // One input method serves every application here, so the state is global
    // and worth remembering across restarts.
    m_config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), true);

    panel_req_update_factory_info ();

    if (m_panel_open) {
        m_panel_client.prepare (m_instance);
        m_panel_client.turn_on (m_instance);
        m_panel_client.send ();
    }
    if (m_sink)
        m_sink->enable (true);
}

void
WaylandFrontEnd::turn_off_im ()
{
    if (!m_im_on)
        return;

    m_im_on = false;
    m_config->write (String (SCIM_CONFIG_FRONTEND_IM_OPENED_BY_DEFAULT), false);

    panel_req_update_factory_info ();

    // Drop whatever was being composed; leaving a preedit on screen that no
    // further key can affect would strand the text in the application.
    if (m_instance >= 0)
        reset (m_instance);
    m_preedit_str = WideString ();
    m_preedit_attrs = AttributeList ();
    m_preedit_caret = 0;
    m_preedit_shown = false;
    if (m_on_the_spot) {
        clear_preedit_on_app ();
        if (m_display)
            wl_display_flush (m_display);
    } else {
        preedit_to_panel (false);
    }

    if (m_panel_open && m_instance >= 0) {
        m_panel_client.prepare (m_instance);
        m_panel_client.turn_off (m_instance);
        m_panel_client.send ();
    }
    if (m_sink) {
        m_sink->show_aux_string (false);
        m_sink->show_lookup_table (false);
        m_sink->enable (false);
    }
}

void
WaylandFrontEnd::switch_factory (const String &sfid)
{
    if (m_instance < 0 || !sfid.length ())
        return;

    String encoding = String ("UTF-8");
    if (!validate_factory (sfid, encoding))
        return;

    String language = m_language;
    if (!language.length ())
        language = scim_get_locale_language (scim_get_current_locale ());

    turn_off_im ();

    // replace_instance () builds a fresh IMEngineInstance, and a new instance has
    // never been focused -- an engine only becomes active, and only registers its
    // properties, on focus_in. Without this bracket the icon and the panel would
    // show the new engine while keys were still handed to an unfocused one, so
    // switching appeared to do nothing. Switching from the tray menu happened to
    // work only because clicking the tray moved focus off the text field and
    // back, and that activate delivered the focus_in as a side effect.
    if (m_focused)
        focus_out (m_instance);

    replace_instance (m_instance, sfid);

    if (m_panel_open) {
        m_panel_client.prepare (m_instance);
        m_panel_client.register_input_context (m_instance, get_instance_uuid (m_instance));
        m_panel_client.send ();
    }

    set_default_factory (language, sfid);

    if (m_focused)
        focus_in (m_instance);

    // Picking an engine means wanting to use it, so end up on regardless of the
    // state we were in.
    turn_on_im ();
}

void
WaylandFrontEnd::panel_req_show_factory_menu ()
{
    if (!m_panel_open || m_instance < 0)
        return;

    std::vector<String> uuids;
    if (!get_factory_list_for_encoding (uuids, String ("UTF-8")))
        return;

    std::vector<PanelFactoryInfo> menu;
    for (size_t i = 0; i < uuids.size (); ++i)
        menu.push_back (PanelFactoryInfo (uuids [i],
                                          utf8_wcstombs (get_factory_name (uuids [i])),
                                          get_factory_language (uuids [i]),
                                          get_factory_icon_file (uuids [i]),
                                          get_factory_symbol (uuids [i])));

    m_panel_client.prepare (m_instance);
    m_panel_client.show_factory_menu (m_instance, menu);
    m_panel_client.send ();
}

void
WaylandFrontEnd::panel_req_update_factory_info ()
{
    // Without this the panel never learns which engine is active, so its tray
    // item has no name and no icon to show -- it registers and then renders as
    // nothing. x11.so has always sent this; the wayland frontends did not.
    if (m_instance < 0)
        return;

    PanelFactoryInfo info;
    if (m_im_on) {
        String uuid = get_instance_uuid (m_instance);
        info = PanelFactoryInfo (uuid,
                                 utf8_wcstombs (get_factory_name (uuid)),
                                 get_factory_language (uuid),
                                 get_factory_icon_file (uuid),
                                 get_factory_symbol (uuid));
    } else {
        info = PanelFactoryInfo (String (""), String (_("English/Keyboard")),
                                 String ("C"), String (SCIM_KEYBOARD_ICON_FILE),
                                 String (_("En")));
    }

    // kimpanel draws the symbol as text in the panel's own colors, which is
    // the one indicator on this desktop that follows a light or dark theme.
    // Independent of the scim panel: either, both or neither may be running.
    if (m_sink)
        m_sink->update_engine_property (info.symbol, info.name);

#ifdef SCIM_HAS_KIMPANEL
    // The indicator is sent to kimpanel even when the candidates are not: on
    // Wayland select_sink () leaves the list to the compositor-placed popup (see
    // there), but the indicator is drawn in the panel itself, where placement
    // never came into it.
    if (m_kimpanel.is_connected () && m_kimpanel.panel_present () &&
        m_sink != &m_kimpanel)
        m_kimpanel.update_engine_property (info.symbol, info.name);
#endif

    if (!m_panel_open)
        return;

    m_panel_client.prepare (m_instance);
    m_panel_client.update_factory_info (m_instance, info);
    m_panel_client.send ();
}

void
WaylandFrontEnd::drop_keyboard ()
{
    if (!m_v1_keyboard)
        return;
    // Destroy the proxy rather than sending wl_keyboard::release: that request
    // only exists since wl_keyboard version 3, and grab_keyboard creates the
    // keyboard as a child new_id of the version-1 zwp_input_method_context_v1,
    // so the proxy inherits version 1. Sending release is a fatal protocol
    // error ("invalid method 0 (since 1 < 3)") that kills the connection. v1
    // has no ungrab request; the grab ends with the context.
    wl_keyboard_destroy (m_v1_keyboard);
    m_v1_keyboard = 0;
}

/* ------------------------------------------------------------------ */
/* Registry                                                            */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::registry_global (uint32_t name, const char *interface, uint32_t version)
{
    if (!strcmp (interface, wl_seat_interface.name)) {
        if (!m_seat) {
            uint32_t v = version < 7 ? version : 7;
            m_seat = static_cast<wl_seat *> (
                wl_registry_bind (m_registry, name, &wl_seat_interface, v));
            m_seat_name = name;
        }
    } else if (!strcmp (interface, zwp_input_method_v1_interface.name)) {
        m_v1_input_method = static_cast<zwp_input_method_v1 *> (
            wl_registry_bind (m_registry, name, &zwp_input_method_v1_interface, 1));
    } else if (!strcmp (interface, zwp_input_method_manager_v2_interface.name)) {
        m_v2_manager = static_cast<zwp_input_method_manager_v2 *> (
            wl_registry_bind (m_registry, name,
                              &zwp_input_method_manager_v2_interface, 1));
    } else if (!strcmp (interface, zwp_virtual_keyboard_manager_v1_interface.name)) {
        m_vk_manager = static_cast<zwp_virtual_keyboard_manager_v1 *> (
            wl_registry_bind (m_registry, name,
                              &zwp_virtual_keyboard_manager_v1_interface, 1));
    } else if (!strcmp (interface, zwp_input_panel_v1_interface.name)) {
        m_v1_input_panel = static_cast<zwp_input_panel_v1 *> (
            wl_registry_bind (m_registry, name, &zwp_input_panel_v1_interface, 1));
    } else if (!strcmp (interface, wl_compositor_interface.name)) {
        // Up to 6, which is where wl_surface gains preferred_buffer_scale --
        // how the candidate surface learns it is on a HiDPI output.
        //
        // The ceiling has to be a compile-time one. The surface listener in
        // utils/candidates is as long as the headers this was built against
        // made it, and libwayland dispatches an event by indexing that array
        // with the opcode, unchecked. Asking for 6 where the headers stopped at
        // 4 would have the compositor send an event past the end of it.
        //
        // wl_compositor_interface.version cannot serve: that struct lives in
        // libwayland-client, so it reports the runtime library rather than the
        // headers. Keep it as a second ceiling anyway, for the reverse case of
        // a library older than the headers.
        //
        // No #ifdef on the 6: configure requires wayland-client 1.22, the
        // release that added both this event and the wl_pointer one the
        // candidate window's listener is initialised with, so headers without
        // it cannot build this tree at all.
        uint32_t want = 6;
        if (want > (uint32_t) wl_compositor_interface.version)
            want = (uint32_t) wl_compositor_interface.version;
        uint32_t v = version < want ? version : want;
        m_compositor = static_cast<wl_compositor *> (
            wl_registry_bind (m_registry, name, &wl_compositor_interface, v));
    } else if (!strcmp (interface, wl_shm_interface.name)) {
        m_shm = static_cast<wl_shm *> (
            wl_registry_bind (m_registry, name, &wl_shm_interface, 1));
    }
}

void
WaylandFrontEnd::registry_global_remove (uint32_t name)
{
    if (name == m_seat_name && m_seat) {
        std::cerr << "Wayland -- exiting: the seat went away.\n";
        m_should_exit = true;
    }
}

/* ------------------------------------------------------------------ */
/* input-method-v1 events                                              */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::v1_activate (struct zwp_input_method_context_v1 *context)
{
    // A text field took focus. The context is the only object that accepts text
    // requests, and it lives exactly until the matching deactivate.
    if (m_v1_context)
        zwp_input_method_context_v1_destroy (m_v1_context);
    drop_keyboard ();

    m_v1_context = context;
    m_serial  = 0;
    zwp_input_method_context_v1_add_listener (m_v1_context, &v1_context_listener, this);

    // Take the hardware keyboard: without this the compositor keeps routing
    // keys to the application and we never see them. The returned object is a
    // plain wl_keyboard, so the standard listener applies.
    m_v1_keyboard = zwp_input_method_context_v1_grab_keyboard (m_v1_context);
    if (m_v1_keyboard)
        wl_keyboard_add_listener (m_v1_keyboard, &v1_kb_listener, this);

    // A keymap we already compiled stays valid across contexts, but the new
    // context needs its modifier names before it can read our bitmasks.
    send_modifiers_map ();

    enter_focus ();
}

void
WaylandFrontEnd::v2_activate ()
{
    // v2 keeps one input-method object and one grab for the whole session, so
    // there is nothing to create here -- only the focus transition.
    enter_focus ();
}

void
WaylandFrontEnd::v2_deactivate ()
{
    leave_focus ();
}

void
WaylandFrontEnd::v2_done ()
{
    // Each done event bumps the serial that commit () must echo back.
    ++m_serial;
}

void
WaylandFrontEnd::v2_unavailable ()
{
    // Another input method owns this seat; we cannot run.
    std::cerr << "Wayland -- exiting: another input method owns this seat.\n";
    m_should_exit = true;
}

/* ------------------------------------------------------------------ */
/* Focus transitions (shared)                                          */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::enter_focus ()
{
    ensure_instance ();
    if (m_instance < 0)
        return;   // ensure_instance () has said why

    m_preedit_str = WideString ();
    m_preedit_caret = 0;
    m_focused = true;
    // Belongs to the text input we just left. The new one says so itself with a
    // content_type event; until then assume an ordinary field, which is what
    // both protocols' initial purpose ("normal") means.
    m_password_field = false;
    SCIM_DEBUG_FRONTEND (2) << "Wayland -- activate: protocol=" << proto_name ()
                            << " siid=" << m_instance
                            << " xkb=" << (m_xkb_state ? 1 : 0) << "\n";

    if (m_panel_open) {
        m_panel_client.prepare (m_instance);
        m_panel_client.register_input_context (m_instance, get_instance_uuid (m_instance));
        m_panel_client.focus_in (m_instance, get_instance_uuid (m_instance));
        // Report the real state: turning the panel on unconditionally would
        // show the engine as active while keys still go straight to the app.
        if (m_im_on) m_panel_client.turn_on  (m_instance);
        else         m_panel_client.turn_off (m_instance);
        m_panel_client.send ();
    }
    // Outside the block above: this also drives the kimpanel engine indicator,
    // which is there whether or not scim-panel-gtk is running. Gating it on the
    // panel connection left the Plasma indicator stale on every focus change in
    // a session without one.
    panel_req_update_factory_info ();

    if (m_sink)
        m_sink->enable (true);
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // Only now is there a text input for the compositor to place the popup
    // against; see CandidatesWayland::set_active (). Activation is a protocol
    // state no other candidate UI has, so it stays off the sink interface.
    if (m_candidates_ui.is_ready ())
        m_candidates_ui.set_active (true);
#endif
    focus_in (m_instance);
}

void
WaylandFrontEnd::leave_focus ()
{
    if (m_instance >= 0 && m_focused) {
        if (m_panel_open) {
            m_panel_client.prepare (m_instance);
            m_panel_client.turn_off (m_instance);
            m_panel_client.focus_out (m_instance);
            m_panel_client.send ();
        }
        focus_out (m_instance);
        reset (m_instance);
    }

    m_focused = false;
    m_password_field = false;
    m_preedit_str = WideString ();
    m_preedit_attrs = AttributeList ();
    m_preedit_caret = 0;
    m_preedit_shown = false;
    if (!m_on_the_spot)
        preedit_to_panel (false);

    if (m_sink) {
        m_sink->show_aux_string (false);
        m_sink->show_lookup_table (false);
        m_sink->enable (false);
    }
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // Staying unmapped until the compositor activates us again: the surface we
    // would be positioned against is going away.
    if (m_candidates_ui.is_ready ())
        m_candidates_ui.set_active (false);
#endif
}

void
WaylandFrontEnd::v1_deactivate (struct zwp_input_method_context_v1 *context)
{
    leave_focus ();

    drop_keyboard ();
    // The compositor names the context it is retiring; destroying anything else
    // would kill a context we still need.
    if (context && context == m_v1_context) {
        zwp_input_method_context_v1_destroy (m_v1_context);
        m_v1_context = 0;
    } else if (context) {
        zwp_input_method_context_v1_destroy (context);
    }
}

/* ------------------------------------------------------------------ */
/* input-method-context-v1 events                                      */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::ctx_surrounding_text (const char * /*text*/, uint32_t /*cursor*/,
                                         uint32_t /*anchor*/)
{
    // TODO: feed surrounding text to the engine (update_surrounding_text) once
    // the engine-side plumbing is wired. Same gap as the v2 frontend.
}

void
WaylandFrontEnd::ctx_reset ()
{
    // The application dropped its input state (e.g. the text changed under us).
    if (m_instance >= 0)
        reset (m_instance);
    m_preedit_str = WideString ();
    m_preedit_caret = 0;
}

void
WaylandFrontEnd::ctx_content_type (uint32_t /*hint*/, uint32_t purpose)
{
    // Neither input-method XML carries the content_purpose enum -- both defer to
    // the text-input protocol, which lives on the compositor's side and is not
    // vendored here -- so the two values acted on are spelled out.
    // zwp_text_input_v1 (what v1's content_type reports) and zwp_text_input_v3
    // (v2's) agree that password is 8. Only v3 defines 9 as pin; v1 numbers date
    // there, which is why it is read on v2 alone.
    const uint32_t PURPOSE_PASSWORD = 8;
    const uint32_t PURPOSE_V3_PIN   = 9;

    bool sensitive = (purpose == PURPOSE_PASSWORD) ||
                     (m_proto == PROTO_V2 && purpose == PURPOSE_V3_PIN);

    if (sensitive == m_password_field)
        return;

    m_password_field = sensitive;
    if (!m_password_field)
        return;

    // Entering the field: drop anything the engine was composing before it can
    // be drawn anywhere. Everything from here on is forwarded untouched, so
    // nothing would come along later to clear it.
    if (m_instance >= 0)
        reset (m_instance);
    m_preedit_str   = WideString ();
    m_preedit_attrs = AttributeList ();
    m_preedit_caret = 0;
    m_preedit_shown = false;
    if (m_on_the_spot) clear_preedit_on_app ();
    else               preedit_to_panel (false);
    if (m_sink) {
        m_sink->show_aux_string (false);
        m_sink->show_lookup_table (false);
    }
    if (m_display)
        wl_display_flush (m_display);

    // The hint is only as good as the toolkit that sets it, so say when it was
    // honoured; "the IME did nothing in that one field" is otherwise a puzzle.
    SCIM_DEBUG_FRONTEND (2) << "Wayland -- password/PIN field: "
                               "passing keys through untouched.\n";

    // TODO: the remaining purposes and hints (digits-only, URL, e-mail) could
    // pick a conversion mode per field once the backend exposes one. Only the
    // sensitive ones are acted on here, because those are the ones where doing
    // nothing is a leak rather than a missing convenience.
}

void
WaylandFrontEnd::ctx_invoke_action (uint32_t /*button*/, uint32_t index)
{
    // The user clicked inside our input panel surface. v2 has no equivalent
    // event -- there we hit-test wl_pointer ourselves.
    if (m_instance >= 0 && m_focused)
        select_candidate (m_instance, index);
}

void
WaylandFrontEnd::ctx_commit_state (uint32_t serial)
{
    // Every commit_string / preedit_string must echo the newest serial, or the
    // compositor discards the request as stale.
    m_serial = serial;
}

void
WaylandFrontEnd::ctx_preferred_language (const char * /*language*/)
{
    // The engine is chosen by SCIM config, not per text field; ignoring this
    // keeps the active engine stable while the user moves between fields.
}

/* ------------------------------------------------------------------ */
/* Grabbed keyboard events                                             */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::kb_keymap (uint32_t format, int32_t fd, uint32_t size)
{
    // Mirror the keymap onto the virtual keyboard so keycodes we forward are
    // interpreted identically by the compositor. v1 forwards through the
    // context, which already shares the seat keymap, so this is v2 only.
    if (m_proto == PROTO_V2 && m_virtual_keyboard)
        zwp_virtual_keyboard_v1_keymap (m_virtual_keyboard, format, fd, size);

    if (format == XKB_KEYMAP_FORMAT_TEXT_V1 && m_xkb_context) {
        char *map = static_cast<char *> (
            mmap (0, size, PROT_READ, MAP_PRIVATE, fd, 0));
        if (map != MAP_FAILED) {
            struct xkb_keymap *keymap = xkb_keymap_new_from_string (
                m_xkb_context, map, XKB_KEYMAP_FORMAT_TEXT_V1,
                XKB_KEYMAP_COMPILE_NO_FLAGS);
            munmap (map, size);
            if (keymap) {
                if (m_xkb_state)  xkb_state_unref (m_xkb_state);
                if (m_xkb_keymap) xkb_keymap_unref (m_xkb_keymap);
                m_xkb_keymap = keymap;
                m_xkb_state  = xkb_state_new (m_xkb_keymap);
                send_modifiers_map ();
            }
        }
    }

    close (fd);
}

void
WaylandFrontEnd::kb_key (uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    // v1 can only speak through a live context; v2 through its input method.
    bool can_send = (m_proto == PROTO_V2) ? (m_v2_input_method != 0)
                                          : (m_v1_context != 0);
    if (m_instance < 0 || !m_xkb_state || !can_send) {
        // No engine / keymap / context yet: pass the key straight through.
        if (can_send) {
            m_have_current_key      = true;
            m_current_key_serial    = serial;
            m_current_key_time      = time;
            m_current_key_code      = key;
            m_current_key_state     = state;
            m_current_key_forwarded = false;
            proto_forward_key ();
            m_have_current_key      = false;
            wl_display_flush (m_display);
        }
        return;
    }

    m_have_current_key      = true;
    m_current_key_serial    = serial;
    m_current_key_time      = time;
    m_current_key_code      = key;
    m_current_key_state     = state;
    m_current_key_forwarded = false;

    KeyEvent scimkey = wayland_key_to_scim (key, state);
    scimkey.mask &= m_valid_key_mask;

    // Hotkeys come first and are checked whether or not the engine is on --
    // otherwise the trigger that turns it back on would itself be swallowed.
    // Focus is still required: with no active text input there is nowhere to
    // send a commit, so toggling the engine could only mislead, and consuming
    // the key would take it from an application that may use it as a shortcut.
    // A password or PIN field takes nothing but the hotkeys: the engine never
    // sees the keys, so nothing is composed and nothing is drawn. The hotkeys
    // stay live so the user can still toggle the engine for the next field.
    bool consumed = false;
    bool hotkey = false;
    if (m_focused && filter_hotkeys (scimkey)) {
        consumed = true;
        hotkey = true;
    } else if (m_focused && m_im_on && !m_password_field) {
        consumed = process_key_event (m_instance, scimkey);
    }

    SCIM_DEBUG_FRONTEND (3) << "Wayland key: evdev=" << key
                            << " sym=0x" << std::hex << scimkey.code
                            << " mask=0x" << scimkey.mask << std::dec
                            << " state=" << state
                            << " focused=" << m_focused
                            << " im_on=" << m_im_on
                            << " siid=" << m_instance
                            << " hotkey=" << hotkey
                            << " consumed=" << consumed << "\n";

    // A key release is never consumed, whatever the engine says. Engines report
    // releases as handled (scim-tables returns true for every one) because they
    // have no use for them, but swallowing a release leaves the compositor
    // believing the key is still held, and it autorepeats forever. Same fix as
    // the ibus frontend's IBUS_RELEASE_MASK guard.
    if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
        consumed = false;

    if (!consumed && !m_current_key_forwarded)
        forward_current_key ();

    m_have_current_key = false;
    wl_display_flush (m_display);
}

void
WaylandFrontEnd::kb_modifiers (uint32_t serial, uint32_t mods_depressed,
                                 uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    if (m_xkb_state)
        xkb_state_update_mask (m_xkb_state, mods_depressed, mods_latched,
                               mods_locked, 0, 0, group);
    // Mirror the modifier state back so the application sees the same shift /
    // ctrl state we do while we hold the grab.
    if (m_proto == PROTO_V2) {
        if (m_virtual_keyboard)
            zwp_virtual_keyboard_v1_modifiers (m_virtual_keyboard, mods_depressed,
                                               mods_latched, mods_locked, group);
    } else if (m_v1_context) {
        zwp_input_method_context_v1_modifiers (m_v1_context, serial, mods_depressed,
                                              mods_latched, mods_locked, group);
    }
}

/* ------------------------------------------------------------------ */
/* Key translation / forwarding                                        */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::send_modifiers_map ()
{
    // The bitmasks we pass to the context's modifiers/keysym requests are the
    // seat keymap's, so hand over that keymap's modifier names in index order
    // for the compositor to interpret them. wl_array holds the names as a
    // sequence of NUL-terminated strings.
    if (!m_v1_context || !m_xkb_keymap)
        return;

    struct wl_array map;
    wl_array_init (&map);

    xkb_mod_index_t num = xkb_keymap_num_mods (m_xkb_keymap);
    for (xkb_mod_index_t i = 0; i < num; ++i) {
        const char *name = xkb_keymap_mod_get_name (m_xkb_keymap, i);
        if (!name)
            name = "";
        size_t len = strlen (name) + 1;
        void *p = wl_array_add (&map, len);
        if (!p)
            break;
        memcpy (p, name, len);
    }

    zwp_input_method_context_v1_modifiers_map (m_v1_context, &map);
    wl_array_release (&map);
}

KeyEvent
WaylandFrontEnd::wayland_key_to_scim (uint32_t key, uint32_t state) const
{
    // evdev keycode -> xkb keycode.
    xkb_keycode_t keycode = key + 8;
    xkb_keysym_t sym = xkb_state_key_get_one_sym (m_xkb_state, keycode);

    uint16 mask = 0;
    if (xkb_state_mod_name_is_active (m_xkb_state, XKB_MOD_NAME_SHIFT,
                                      XKB_STATE_MODS_EFFECTIVE) > 0)
        mask |= SCIM_KEY_ShiftMask;
    if (xkb_state_mod_name_is_active (m_xkb_state, XKB_MOD_NAME_CAPS,
                                      XKB_STATE_MODS_EFFECTIVE) > 0)
        mask |= SCIM_KEY_CapsLockMask;
    if (xkb_state_mod_name_is_active (m_xkb_state, XKB_MOD_NAME_CTRL,
                                      XKB_STATE_MODS_EFFECTIVE) > 0)
        mask |= SCIM_KEY_ControlMask;
    if (xkb_state_mod_name_is_active (m_xkb_state, XKB_MOD_NAME_ALT,
                                      XKB_STATE_MODS_EFFECTIVE) > 0)
        mask |= SCIM_KEY_AltMask;
    if (xkb_state_mod_name_is_active (m_xkb_state, XKB_MOD_NAME_LOGO,
                                      XKB_STATE_MODS_EFFECTIVE) > 0)
        mask |= SCIM_KEY_SuperMask;
    if (xkb_state_mod_name_is_active (m_xkb_state, XKB_MOD_NAME_NUM,
                                      XKB_STATE_MODS_EFFECTIVE) > 0)
        mask |= SCIM_KEY_NumLockMask;

    if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
        mask |= SCIM_KEY_ReleaseMask;

    // xkb keysyms use the X11 keysym numbering, matching scim's key codes.
    return KeyEvent (static_cast<uint32> (sym), mask);
}

void
WaylandFrontEnd::forward_current_key ()
{
    if (!m_have_current_key || m_current_key_forwarded)
        return;
    proto_forward_key ();
    m_current_key_forwarded = true;
}

/* ------------------------------------------------------------------ */
/* Backend -> compositor (text input transport)                        */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::send_preedit ()
{
    // No protocol guard here: proto_send_preedit () checks the object that the
    // negotiated generation actually uses. Testing m_v1_context would silently
    // drop every preedit on v2, where that pointer is always null because v2 has
    // no per-activation context.
    String utf8 = utf8_wcstombs (m_preedit_str);

    int caret = m_preedit_caret;
    if (caret < 0) caret = 0;
    if (caret > (int) m_preedit_str.length ()) caret = (int) m_preedit_str.length ();
    String prefix = utf8_wcstombs (m_preedit_str.substr (0, caret));

    proto_send_preedit (utf8, static_cast<int32_t> (prefix.length ()));
    wl_display_flush (m_display);
}

/* ------------------------------------------------------------------ */
/* Protocol-specific operations                                        */
/* ------------------------------------------------------------------ */

const char *
WaylandFrontEnd::proto_name () const
{
    return m_proto == PROTO_V2 ? "input-method-v2"
         : m_proto == PROTO_V1 ? "input-method-v1"
                               : "none";
}

void
WaylandFrontEnd::proto_start_grab ()
{
    if (m_proto == PROTO_V2) {
        if (!m_v2_input_method || m_v2_grab)
            return;
        m_v2_grab = zwp_input_method_v2_grab_keyboard (m_v2_input_method);
        if (m_v2_grab)
            zwp_input_method_keyboard_grab_v2_add_listener (m_v2_grab,
                                                           &v2_grab_listener, this);
    }
    // v1 grabs per context, from v1_activate ().
}

void
WaylandFrontEnd::proto_send_preedit (const String &utf8, int32_t cursor)
{
    // An empty preedit must reach the application as a null string rather than
    // as "". set_preedit_string's text argument is not nullable, so the only way
    // to send null is to skip the request entirely and let commit apply the
    // pending state's initial value. It matters because GTK raises preedit-start
    // and preedit-end on that string becoming non-null and null again, not on
    // its length: an empty-but-not-null preedit leaves VTE terminals with
    // im_preedit_active still set, and they stop painting their cursor until a
    // focus change. gnome-shell normalizes "" to null on its own, so the symptom
    // only shows on compositors that relay what we send verbatim.
    //
    // A negative cursor index means "no cursor" and is still wanted by v1 below,
    // whose text argument leaves us no equivalent of null.
    if (!utf8.length ())
        cursor = -1;

    if (m_proto == PROTO_V2) {
        if (!m_v2_input_method)
            return;
        if (utf8.length ())
            zwp_input_method_v2_set_preedit_string (m_v2_input_method,
                                                   utf8.c_str (),
                                                   cursor, cursor);
        zwp_input_method_v2_commit (m_v2_input_method, m_serial);
        return;
    }

    if (!m_v1_context)
        return;
    // preedit_cursor is consumed by the following preedit_string, so it has to
    // be sent first; the index is a byte offset into the preedit text.
    zwp_input_method_context_v1_preedit_cursor (m_v1_context, cursor);
    // The "commit" argument is what the application should keep if the preedit
    // is dropped without us committing (e.g. on unfocus). Empty: SCIM discards
    // uncommitted input on reset / focus_out, and every other frontend behaves
    // that way, so leaving raw phonetic keys behind would be inconsistent.
    zwp_input_method_context_v1_preedit_string (m_v1_context, m_serial,
                                               utf8.c_str (), "");
}

void
WaylandFrontEnd::proto_clear_preedit ()
{
    proto_send_preedit (String (""), 0);
}

void
WaylandFrontEnd::proto_commit_string (const String &utf8)
{
    if (m_proto == PROTO_V2) {
        if (!m_v2_input_method)
            return;
        // The preedit is cleared by omission: not sending set_preedit_string in
        // this batch leaves the pending preedit at its initial value, and commit
        // replaces the current state with the pending one. Sending "" instead
        // would clear the text but keep it non-null, which is the case VTE
        // terminals mishandle (see proto_send_preedit ()).
        zwp_input_method_v2_commit_string (m_v2_input_method, utf8.c_str ());
        zwp_input_method_v2_commit (m_v2_input_method, m_serial);
        return;
    }

    if (!m_v1_context)
        return;
    // Drop the preedit before committing: the application would otherwise keep
    // showing the composing text next to the text we just committed.
    proto_clear_preedit ();
    zwp_input_method_context_v1_commit_string (m_v1_context, m_serial, utf8.c_str ());
}

void
WaylandFrontEnd::proto_forward_key ()
{
    // Forward the original evdev keycode rather than reconstructing one from the
    // keysym, so the compositor sees exactly what the user pressed.
    if (m_proto == PROTO_V2) {
        if (m_virtual_keyboard)
            zwp_virtual_keyboard_v1_key (m_virtual_keyboard, m_current_key_time,
                                         m_current_key_code, m_current_key_state);
        return;
    }

    if (m_v1_context)
        // v1 also wants the keyboard event's own serial, as the protocol
        // requires: "the arguments should be the ones from the
        // wl_keyboard::key event".
        zwp_input_method_context_v1_key (m_v1_context, m_current_key_serial,
                                         m_current_key_time, m_current_key_code,
                                         m_current_key_state);
}

bool
WaylandFrontEnd::proto_candidates_init ()
{
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (!m_compositor || !m_shm)
        return false;
    if (m_proto == PROTO_V2)
        return m_v2_input_method &&
               m_candidates_ui.init (m_display, m_compositor, m_shm,
                                     m_v2_input_method, m_seat);
    return m_v1_input_panel &&
           m_candidates_ui.init_input_panel (m_display, m_compositor, m_shm,
                                             m_v1_input_panel, m_seat);
#else
    return false;
#endif
}

void
WaylandFrontEnd::clear_preedit_on_app ()
{
    proto_clear_preedit ();
}

void
WaylandFrontEnd::preedit_to_panel (bool visible)
{
    if (m_sink) {
        m_sink->update_preedit_string (m_preedit_str, m_preedit_attrs);
        m_sink->update_preedit_caret (m_preedit_caret);
        m_sink->show_preedit_string (visible);
    }
}

void
WaylandFrontEnd::update_preedit_caret (int id, int caret)
{
    if (id != m_instance) return;
    m_preedit_caret = caret;
    if (m_on_the_spot) send_preedit ();
    else               preedit_to_panel (m_preedit_shown);
}

void
WaylandFrontEnd::update_preedit_string (int id, const WideString & str,
                                          const AttributeList & attrs)
{
    if (id != m_instance) return;
    m_preedit_str   = str;
    m_preedit_attrs = attrs;
    if (m_preedit_caret > (int) str.length ())
        m_preedit_caret = (int) str.length ();
    if (m_on_the_spot) send_preedit ();
    else               preedit_to_panel (m_preedit_shown);
}

void
WaylandFrontEnd::show_preedit_string (int id)
{
    if (id != m_instance) return;
    m_preedit_shown = true;
    if (m_on_the_spot) send_preedit ();
    else               preedit_to_panel (true);
}

void
WaylandFrontEnd::hide_preedit_string (int id)
{
    if (id != m_instance) return;
    m_preedit_shown = false;
    m_preedit_str   = WideString ();
    m_preedit_attrs = AttributeList ();
    m_preedit_caret = 0;
    if (m_on_the_spot) {
        clear_preedit_on_app ();
        if (m_display)
            wl_display_flush (m_display);
    } else {
        preedit_to_panel (false);
    }
}

void
WaylandFrontEnd::commit_string (int id, const WideString & str)
{
    if (id != m_instance) return;
    proto_commit_string (utf8_wcstombs (str));
    wl_display_flush (m_display);
}

void
WaylandFrontEnd::forward_key_event (int id, const KeyEvent & /*key*/)
{
    if (id != m_instance) return;
    // Forward the physical key currently being processed. We forward the
    // original evdev keycode rather than reconstructing one from the keysym,
    // so the compositor sees exactly what the user pressed.
    forward_current_key ();
}

/* ------------------------------------------------------------------ */
/* Aux + candidate lookup table -> panel renderer or kimpanel           */
/* ------------------------------------------------------------------ */

#ifdef SCIM_HAS_CANDIDATES_WAYLAND

void
WaylandFrontEnd::configure_candidates_ui ()
{
    if (!m_candidates_ui.is_ready ())
        return;
    m_candidates_ui.ui ().set_theme (scim_candidates_theme_from_config (m_config));
}

#endif // SCIM_HAS_CANDIDATES_WAYLAND

// Not under either #ifdef: whichever candidate UI was built reports its actions
// through these, so gating them on one of the two leaves the other calling a
// symbol that does not exist -- and a frontend module only finds that out when
// the launcher dlopen()s it.

void
WaylandFrontEnd::select_sink ()
{
    CandidatesSink *want = 0;

    // Our own surface first, and kimpanel only as the fallback -- the reverse of
    // what the IM modules do, for a reason that is specific to being the
    // compositor's input method.
    //
    // A candidate list has to sit next to the text being typed, and on Wayland
    // only the compositor knows where that is. Our surface is placed by the
    // compositor for exactly that purpose (zwp_input_popup_surface_v2 is
    // anchored to the text cursor, and a v1 overlay panel is positioned against
    // the focused text input). kimpanel is told where to draw with
    // UpdateSpotLocation in screen coordinates -- a number no Wayland client can
    // learn, and which no Wayland client could act on if it did -- so it puts
    // the list wherever it last was, typically the corner of the screen.
    //
    // The IM modules are a different case and rightly prefer kimpanel: a toolkit
    // widget does know its cursor position, so the panel can be told. Here the
    // indicator still goes to kimpanel either way, which is the part of it that
    // never depended on placement; see panel_req_update_factory_info ().
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ())
        want = &m_candidates_ui;
#endif
#ifdef SCIM_HAS_KIMPANEL
    // Only while a panel widget is actually listening; kimpanel draws nothing
    // itself, so without one every update would go nowhere.
    if (!want && m_kimpanel.is_connected () && m_kimpanel.panel_present ())
        want = &m_kimpanel;
#endif

    if (m_sink_selected && want == m_sink)
        return;
    m_sink_selected = true;

    // Take down whatever the outgoing one is showing: it has no idea it is being
    // replaced and would leave a candidate window on screen for good.
    if (m_sink) {
        m_sink->show_preedit_string (false);
        m_sink->show_aux_string (false);
        m_sink->show_lookup_table (false);
        m_sink->remove_engine_property ();
        m_sink->enable (false);
    }

    m_sink = want;

    if (m_sink) {
        m_sink->signal_connect_select_candidate (
            [this] (int idx) { sink_select_candidate (idx); });
        m_sink->signal_connect_page_up (
            [this] () { sink_page_up (); });
        m_sink->signal_connect_page_down (
            [this] () { sink_page_down (); });
        m_sink->signal_connect_move_preedit_caret (
            [this] (int pos) { sink_move_preedit_caret (pos); });

        // Nothing in flight is replayed: the frontend keeps no copy of the aux
        // string or lookup table, and this happens when a panel widget is added
        // or removed rather than mid-composition.
        if (m_im_on && m_focused)
            m_sink->enable (true);
    } else {
        // Preedit still reaches the application, so the user can type and see
        // something -- but any engine that converts through a candidate list is
        // unusable, and silence here makes that look like a broken engine.
        std::cerr << "Wayland -- no candidate UI: the compositor offers no "
                     "surface to draw one on and no kimpanel applet is present. "
                     "Candidates and the aux string will not be shown.\n";
    }

    // Last, and outside the branch: the engine indicator is the one piece of
    // state a panel cannot rediscover on its own, and it goes to kimpanel
    // whether or not kimpanel is the sink.
    panel_req_update_factory_info ();
}

void
WaylandFrontEnd::sink_select_candidate (int cand_index)
{
    if (m_instance >= 0 && m_focused)
        select_candidate (m_instance, cand_index);
}

void
WaylandFrontEnd::sink_page_up ()
{
    if (m_instance >= 0 && m_focused)
        lookup_table_page_up (m_instance);
}

void
WaylandFrontEnd::sink_page_down ()
{
    if (m_instance >= 0 && m_focused)
        lookup_table_page_down (m_instance);
}

void
WaylandFrontEnd::sink_move_preedit_caret (int pos)
{
    if (m_instance >= 0 && m_focused)
        move_preedit_caret (m_instance, pos);
}

#if defined(SCIM_HAS_CANDIDATES_WAYLAND) || defined(SCIM_HAS_KIMPANEL)

void
WaylandFrontEnd::update_aux_string (int id, const WideString & str,
                                      const AttributeList & attrs)
{
    if (id != m_instance) return;
    if (m_sink) m_sink->update_aux_string (str, attrs);
}

void
WaylandFrontEnd::show_aux_string (int id)
{
    if (id != m_instance) return;
    if (m_sink) m_sink->show_aux_string (true);
}

void
WaylandFrontEnd::hide_aux_string (int id)
{
    if (id != m_instance) return;
    if (m_sink) m_sink->show_aux_string (false);
}

void
WaylandFrontEnd::update_lookup_table (int id, const LookupTable & table)
{
    if (id != m_instance) return;
    if (m_sink) m_sink->update_lookup_table (table);
}

void
WaylandFrontEnd::show_lookup_table (int id)
{
    if (id != m_instance) return;
    if (m_sink) m_sink->show_lookup_table (true);
}

void
WaylandFrontEnd::hide_lookup_table (int id)
{
    if (id != m_instance) return;
    if (m_sink) m_sink->show_lookup_table (false);
}

#endif // SCIM_HAS_CANDIDATES_WAYLAND || SCIM_HAS_KIMPANEL

/* ------------------------------------------------------------------ */
/* Static listener trampolines                                         */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::handle_registry_global (void *data, struct wl_registry *,
                                           uint32_t name, const char *interface,
                                           uint32_t version)
{
    static_cast<WaylandFrontEnd *> (data)->registry_global (name, interface, version);
}

void
WaylandFrontEnd::handle_registry_global_remove (void *data, struct wl_registry *,
                                                  uint32_t name)
{
    static_cast<WaylandFrontEnd *> (data)->registry_global_remove (name);
}

void
WaylandFrontEnd::handle_v1_activate (void *data, struct zwp_input_method_v1 *,
                                       struct zwp_input_method_context_v1 *context)
{
    static_cast<WaylandFrontEnd *> (data)->v1_activate (context);
}

void
WaylandFrontEnd::handle_v1_deactivate (void *data, struct zwp_input_method_v1 *,
                                         struct zwp_input_method_context_v1 *context)
{
    static_cast<WaylandFrontEnd *> (data)->v1_deactivate (context);
}

void
WaylandFrontEnd::handle_ctx_surrounding_text (void *data, struct zwp_input_method_context_v1 *,
                                                const char *text, uint32_t cursor, uint32_t anchor)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_surrounding_text (text, cursor, anchor);
}

void
WaylandFrontEnd::handle_ctx_reset (void *data, struct zwp_input_method_context_v1 *)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_reset ();
}

void
WaylandFrontEnd::handle_ctx_content_type (void *data, struct zwp_input_method_context_v1 *,
                                            uint32_t hint, uint32_t purpose)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_content_type (hint, purpose);
}

void
WaylandFrontEnd::handle_ctx_invoke_action (void *data, struct zwp_input_method_context_v1 *,
                                             uint32_t button, uint32_t index)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_invoke_action (button, index);
}

void
WaylandFrontEnd::handle_ctx_commit_state (void *data, struct zwp_input_method_context_v1 *,
                                            uint32_t serial)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_commit_state (serial);
}

void
WaylandFrontEnd::handle_ctx_preferred_language (void *data, struct zwp_input_method_context_v1 *,
                                                  const char *language)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_preferred_language (language);
}

void
WaylandFrontEnd::handle_v2_activate (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->v2_activate ();
}

void
WaylandFrontEnd::handle_v2_deactivate (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->v2_deactivate ();
}

void
WaylandFrontEnd::handle_v2_surrounding_text (void *data, struct zwp_input_method_v2 *,
                                             const char *text, uint32_t cursor, uint32_t anchor)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_surrounding_text (text, cursor, anchor);
}

void
WaylandFrontEnd::handle_v2_text_change_cause (void *, struct zwp_input_method_v2 *, uint32_t)
{
}

void
WaylandFrontEnd::handle_v2_content_type (void *data, struct zwp_input_method_v2 *,
                                         uint32_t hint, uint32_t purpose)
{
    static_cast<WaylandFrontEnd *> (data)->ctx_content_type (hint, purpose);
}

void
WaylandFrontEnd::handle_v2_done (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->v2_done ();
}

void
WaylandFrontEnd::handle_v2_unavailable (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->v2_unavailable ();
}

void
WaylandFrontEnd::handle_grab_keymap (void *data, struct zwp_input_method_keyboard_grab_v2 *,
                                     uint32_t format, int32_t fd, uint32_t size)
{
    static_cast<WaylandFrontEnd *> (data)->kb_keymap (format, fd, size);
}

void
WaylandFrontEnd::handle_grab_key (void *data, struct zwp_input_method_keyboard_grab_v2 *,
                                  uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    static_cast<WaylandFrontEnd *> (data)->kb_key (serial, time, key, state);
}

void
WaylandFrontEnd::handle_grab_modifiers (void *data, struct zwp_input_method_keyboard_grab_v2 *,
                                        uint32_t serial, uint32_t mods_depressed,
                                        uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    static_cast<WaylandFrontEnd *> (data)->kb_modifiers (serial, mods_depressed,
                                                        mods_latched, mods_locked, group);
}

void
WaylandFrontEnd::handle_grab_repeat_info (void *, struct zwp_input_method_keyboard_grab_v2 *,
                                          int32_t, int32_t)
{
}

void
WaylandFrontEnd::handle_kb_keymap (void *data, struct wl_keyboard *,
                                     uint32_t format, int32_t fd, uint32_t size)
{
    static_cast<WaylandFrontEnd *> (data)->kb_keymap (format, fd, size);
}

void
WaylandFrontEnd::handle_kb_enter (void *, struct wl_keyboard *, uint32_t,
                                    struct wl_surface *, struct wl_array *)
{
    // The grab is not tied to a surface; focus is tracked via activate.
}

void
WaylandFrontEnd::handle_kb_leave (void *, struct wl_keyboard *, uint32_t,
                                    struct wl_surface *)
{
}

void
WaylandFrontEnd::handle_kb_key (void *data, struct wl_keyboard *, uint32_t serial,
                                  uint32_t time, uint32_t key, uint32_t state)
{
    static_cast<WaylandFrontEnd *> (data)->kb_key (serial, time, key, state);
}

void
WaylandFrontEnd::handle_kb_modifiers (void *data, struct wl_keyboard *, uint32_t serial,
                                        uint32_t mods_depressed, uint32_t mods_latched,
                                        uint32_t mods_locked, uint32_t group)
{
    static_cast<WaylandFrontEnd *> (data)->kb_modifiers (serial, mods_depressed,
                                                           mods_latched, mods_locked, group);
}

void
WaylandFrontEnd::handle_kb_repeat_info (void *, struct wl_keyboard *, int32_t, int32_t)
{
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
