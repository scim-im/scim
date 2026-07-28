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

#define scim_module_init           wayland_LTX_scim_module_init
#define scim_module_exit           wayland_LTX_scim_module_exit
#define scim_frontend_module_init  wayland_LTX_scim_frontend_module_init
#define scim_frontend_module_run   wayland_LTX_scim_frontend_module_run
#define scim_frontend_module_poll_fds       wayland_LTX_scim_frontend_module_poll_fds
#define scim_frontend_module_process_events wayland_LTX_scim_frontend_module_process_events
#define scim_frontend_module_has_exited     wayland_LTX_scim_frontend_module_has_exited

#define SCIM_CONFIG_FRONTEND_WAYLAND_LANGUAGE  "/FrontEnd/Wayland/Language"

using namespace scim;

// Wayland key state (from wl_keyboard, mirrored by the grab).
#ifndef WL_KEYBOARD_KEY_STATE_PRESSED
#define WL_KEYBOARD_KEY_STATE_PRESSED  1
#endif

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

static const struct zwp_input_method_v2_listener im_listener = {
    WaylandFrontEnd::handle_im_activate,
    WaylandFrontEnd::handle_im_deactivate,
    WaylandFrontEnd::handle_im_surrounding_text,
    WaylandFrontEnd::handle_im_text_change_cause,
    WaylandFrontEnd::handle_im_content_type,
    WaylandFrontEnd::handle_im_done,
    WaylandFrontEnd::handle_im_unavailable
};

static const struct zwp_input_method_keyboard_grab_v2_listener kb_listener = {
    WaylandFrontEnd::handle_kb_keymap,
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
      m_display (0),
      m_registry (0),
      m_seat (0),
      m_compositor (0),
      m_shm (0),
      m_seat_name (0),
      m_im_manager (0),
      m_input_method (0),
      m_grab (0),
      m_vk_manager (0),
      m_virtual_keyboard (0),
      m_serial (0),
      m_preedit_caret (0),
      m_xkb_context (0),
      m_xkb_keymap (0),
      m_xkb_state (0),
      m_have_current_key (false),
      m_current_key_time (0),
      m_current_key_code (0),
      m_current_key_state (0),
      m_current_key_forwarded (false),
      m_should_exit (false)
{
    if (!_scim_frontend.null () && _scim_frontend != this)
        throw FrontEndError (String ("Wayland -- only one frontend can be created!"));

#ifdef SCIM_HAS_KIMPANEL
    m_use_kimpanel = false;
#endif
}

WaylandFrontEnd::~WaylandFrontEnd ()
{
    if (m_instance >= 0)
        delete_instance (m_instance);

    if (m_xkb_state)    xkb_state_unref (m_xkb_state);
    if (m_xkb_keymap)   xkb_keymap_unref (m_xkb_keymap);
    if (m_xkb_context)  xkb_context_unref (m_xkb_context);

    if (m_grab)             zwp_input_method_keyboard_grab_v2_release (m_grab);
    if (m_virtual_keyboard) zwp_virtual_keyboard_v1_destroy (m_virtual_keyboard);
    if (m_input_method)     zwp_input_method_v2_destroy (m_input_method);
    if (m_im_manager)       zwp_input_method_manager_v2_destroy (m_im_manager);
    if (m_vk_manager)       zwp_virtual_keyboard_manager_v1_destroy (m_vk_manager);
    if (m_seat)             wl_seat_destroy (m_seat);
    if (m_registry)         wl_registry_destroy (m_registry);
    if (m_display)          wl_display_disconnect (m_display);
}

/* ------------------------------------------------------------------ */
/* Init / run                                                          */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::init (int /*argc*/, char ** /*argv*/)
{
    reload_config_callback (m_config);
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

    if (!m_im_manager)
        throw FrontEndError (String ("Wayland -- compositor has no zwp_input_method_manager_v2 "
                                     "(not supported on this desktop)."));
    if (!m_seat)
        throw FrontEndError (String ("Wayland -- no wl_seat available."));

    m_xkb_context = xkb_context_new (XKB_CONTEXT_NO_FLAGS);
    if (!m_xkb_context)
        throw FrontEndError (String ("Wayland -- cannot create xkb context."));

    m_input_method =
        zwp_input_method_manager_v2_get_input_method (m_im_manager, m_seat);
    zwp_input_method_v2_add_listener (m_input_method, &im_listener, this);

    if (m_vk_manager)
        m_virtual_keyboard =
            zwp_virtual_keyboard_manager_v1_create_virtual_keyboard (m_vk_manager, m_seat);

    start_grab ();

#ifdef SCIM_HAS_KIMPANEL
    // Prefer KDE kimpanel when running on Plasma (config override or desktop
    // detection). When it connects it replaces the Cairo popup for candidates.
    bool want_kimpanel =
        m_config->read (String ("/Panel/UseKimpanel"),
                        KimpanelAgent::desktop_prefers_kimpanel ());
    if (want_kimpanel && m_kimpanel.connect ()) {
        m_use_kimpanel = true;
        m_kimpanel.signal_connect_select_candidate (
            [this] (int idx) { if (m_instance >= 0 && m_focused) select_candidate (m_instance, idx); });
        m_kimpanel.signal_connect_page_up (
            [this] () { if (m_instance >= 0 && m_focused) lookup_table_page_up (m_instance); });
        m_kimpanel.signal_connect_page_down (
            [this] () { if (m_instance >= 0 && m_focused) lookup_table_page_down (m_instance); });
        m_kimpanel.signal_connect_move_preedit_caret (
            [this] (int pos) { if (m_instance >= 0 && m_focused) move_preedit_caret (m_instance, pos); });
    }
#endif

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // Cairo renderer on an input-popup-surface-v2 for aux + candidates.
    if (!use_kimpanel_ui () && m_compositor && m_shm &&
        m_candidates_ui.init (m_display, m_compositor, m_shm, m_input_method, m_seat)) {
        m_candidates_ui.signal_connect_candidate_selected (
            [this] (int idx) { candidates_ui_select_candidate (idx); });
        m_candidates_ui.signal_connect_page_up (
            [this] () { candidates_ui_page_up (); });
        m_candidates_ui.signal_connect_page_down (
            [this] () { candidates_ui_page_down (); });
        configure_candidates_ui ();
    } else {
        SCIM_DEBUG_FRONTEND (1) << "Wayland -- no input-popup surface; "
                                   "aux/candidates will not be shown.\n";
    }
#endif

    ensure_instance ();

    wl_display_roundtrip (m_display);
}

bool
WaylandFrontEnd::poll_fds (std::vector<int> &fds)
{
    if (m_display)
        fds.push_back (wl_display_get_fd (m_display));
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) {
        int kfd = m_kimpanel.connection_number ();
        if (kfd >= 0)
            fds.push_back (kfd);
    }
#endif
    return true;
}

void
WaylandFrontEnd::process_events ()
{
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ())
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
        m_should_exit = true;
        return;
    }
    if (wl_display_flush (m_display) < 0 && errno != EAGAIN)
        m_should_exit = true;
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
        if (fds.empty ())
            break;

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
            break;
        }
    }
}

void
WaylandFrontEnd::reload_config_callback (const ConfigPointer &config)
{
    m_language = config->read (String (SCIM_CONFIG_FRONTEND_WAYLAND_LANGUAGE),
                               String (""));
}

/* ------------------------------------------------------------------ */
/* Instance / grab helpers                                             */
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

    if (m_instance < 0)
        SCIM_DEBUG_FRONTEND(1) << "Wayland -- failed to create an IMEngine instance.\n";
}

void
WaylandFrontEnd::start_grab ()
{
    if (!m_input_method || m_grab)
        return;
    m_grab = zwp_input_method_v2_grab_keyboard (m_input_method);
    if (m_grab)
        zwp_input_method_keyboard_grab_v2_add_listener (m_grab, &kb_listener, this);
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
    } else if (!strcmp (interface, zwp_input_method_manager_v2_interface.name)) {
        m_im_manager = static_cast<zwp_input_method_manager_v2 *> (
            wl_registry_bind (m_registry, name,
                              &zwp_input_method_manager_v2_interface, 1));
    } else if (!strcmp (interface, zwp_virtual_keyboard_manager_v1_interface.name)) {
        m_vk_manager = static_cast<zwp_virtual_keyboard_manager_v1 *> (
            wl_registry_bind (m_registry, name,
                              &zwp_virtual_keyboard_manager_v1_interface, 1));
    } else if (!strcmp (interface, wl_compositor_interface.name)) {
        uint32_t v = version < 4 ? version : 4;
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
    if (name == m_seat_name)
        m_should_exit = true;
}

/* ------------------------------------------------------------------ */
/* input-method-v2 events                                              */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::im_activate ()
{
    ensure_instance ();
    if (m_instance < 0)
        return;
    m_preedit_str = WideString ();
    m_preedit_caret = 0;
    m_focused = true;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ())
        m_kimpanel.enable (true);
#endif
    focus_in (m_instance);
}

void
WaylandFrontEnd::im_deactivate ()
{
    if (m_instance < 0)
        return;
    if (m_focused) {
        focus_out (m_instance);
        reset (m_instance);
    }
    m_focused = false;
    m_preedit_str = WideString ();
    m_preedit_caret = 0;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) {
        m_kimpanel.show_aux_string (false);
        m_kimpanel.show_lookup_table (false);
        m_kimpanel.enable (false);
    }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().hide_aux_string ();
        m_candidates_ui.ui ().hide_lookup_table ();
        m_candidates_ui.hide ();
    }
#endif
}

void
WaylandFrontEnd::im_surrounding_text (const char * /*text*/, uint32_t /*cursor*/,
                                      uint32_t /*anchor*/)
{
    // TODO(5b+): feed surrounding text to the engine
    // (update_surrounding_text) once the engine-side plumbing is wired.
}

void
WaylandFrontEnd::im_done ()
{
    // Each done event bumps the serial that commit() must echo back.
    ++m_serial;
}

void
WaylandFrontEnd::im_unavailable ()
{
    // Another input method owns this seat; we cannot run.
    m_should_exit = true;
}

/* ------------------------------------------------------------------ */
/* Keyboard grab events                                                */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::kb_keymap (uint32_t format, int32_t fd, uint32_t size)
{
    // Mirror the keymap onto the virtual keyboard so forwarded keycodes are
    // interpreted identically by the compositor.
    if (m_virtual_keyboard)
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
            }
        }
    }

    close (fd);
}

void
WaylandFrontEnd::kb_key (uint32_t /*serial*/, uint32_t time, uint32_t key, uint32_t state)
{
    if (m_instance < 0 || !m_xkb_state) {
        // No engine / keymap yet: pass the key straight through.
        if (m_virtual_keyboard) {
            zwp_virtual_keyboard_v1_key (m_virtual_keyboard, time, key, state);
            wl_display_flush (m_display);
        }
        return;
    }

    m_have_current_key      = true;
    m_current_key_time      = time;
    m_current_key_code      = key;
    m_current_key_state     = state;
    m_current_key_forwarded = false;

    KeyEvent scimkey = wayland_key_to_scim (key, state);

    bool consumed = false;
    if (m_focused)
        consumed = process_key_event (m_instance, scimkey);

    if (!consumed && !m_current_key_forwarded)
        forward_current_key ();

    m_have_current_key = false;
    wl_display_flush (m_display);
}

void
WaylandFrontEnd::kb_modifiers (uint32_t /*serial*/, uint32_t mods_depressed,
                               uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    if (m_xkb_state)
        xkb_state_update_mask (m_xkb_state, mods_depressed, mods_latched,
                               mods_locked, 0, 0, group);
    if (m_virtual_keyboard)
        zwp_virtual_keyboard_v1_modifiers (m_virtual_keyboard, mods_depressed,
                                           mods_latched, mods_locked, group);
}

/* ------------------------------------------------------------------ */
/* Key translation / forwarding                                        */
/* ------------------------------------------------------------------ */

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
    if (!m_have_current_key || m_current_key_forwarded || !m_virtual_keyboard)
        return;
    zwp_virtual_keyboard_v1_key (m_virtual_keyboard, m_current_key_time,
                                 m_current_key_code, m_current_key_state);
    m_current_key_forwarded = true;
}

/* ------------------------------------------------------------------ */
/* Backend -> compositor (text input transport)                        */
/* ------------------------------------------------------------------ */

void
WaylandFrontEnd::send_preedit ()
{
    if (!m_input_method)
        return;

    String utf8 = utf8_wcstombs (m_preedit_str);

    int caret = m_preedit_caret;
    if (caret < 0) caret = 0;
    if (caret > (int) m_preedit_str.length ()) caret = (int) m_preedit_str.length ();
    String prefix = utf8_wcstombs (m_preedit_str.substr (0, caret));
    int32_t cursor = static_cast<int32_t> (prefix.length ());

    zwp_input_method_v2_set_preedit_string (m_input_method, utf8.c_str (),
                                            cursor, cursor);
    zwp_input_method_v2_commit (m_input_method, m_serial);
    wl_display_flush (m_display);
}

void
WaylandFrontEnd::update_preedit_caret (int id, int caret)
{
    if (id != m_instance) return;
    m_preedit_caret = caret;
    send_preedit ();
}

void
WaylandFrontEnd::update_preedit_string (int id, const WideString & str,
                                        const AttributeList & /*attrs*/)
{
    if (id != m_instance) return;
    m_preedit_str = str;
    if (m_preedit_caret > (int) str.length ())
        m_preedit_caret = (int) str.length ();
    send_preedit ();
}

void
WaylandFrontEnd::show_preedit_string (int id)
{
    if (id != m_instance) return;
    send_preedit ();
}

void
WaylandFrontEnd::hide_preedit_string (int id)
{
    if (id != m_instance) return;
    m_preedit_str = WideString ();
    m_preedit_caret = 0;
    if (m_input_method) {
        zwp_input_method_v2_set_preedit_string (m_input_method, "", 0, 0);
        zwp_input_method_v2_commit (m_input_method, m_serial);
        wl_display_flush (m_display);
    }
}

void
WaylandFrontEnd::commit_string (int id, const WideString & str)
{
    if (id != m_instance || !m_input_method) return;
    String utf8 = utf8_wcstombs (str);
    zwp_input_method_v2_commit_string (m_input_method, utf8.c_str ());
    zwp_input_method_v2_commit (m_input_method, m_serial);
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
/* Aux + candidate lookup table -> popup renderer or kimpanel           */
/* ------------------------------------------------------------------ */

#ifdef SCIM_HAS_CANDIDATES_WAYLAND

void
WaylandFrontEnd::refresh_candidates_ui ()
{
    if (m_candidates_ui.is_ready ())
        m_candidates_ui.update ();
}

void
WaylandFrontEnd::configure_candidates_ui ()
{
    if (!m_candidates_ui.is_ready ())
        return;
    // Apply the configured font AND colors, matching the legacy GTK panel.
    m_candidates_ui.ui ().set_theme (scim_candidates_theme_from_config (m_config));
}

void
WaylandFrontEnd::candidates_ui_select_candidate (int cand_index)
{
    if (m_instance >= 0 && m_focused)
        select_candidate (m_instance, cand_index);
}

void
WaylandFrontEnd::candidates_ui_page_up ()
{
    if (m_instance >= 0 && m_focused)
        lookup_table_page_up (m_instance);
}

void
WaylandFrontEnd::candidates_ui_page_down ()
{
    if (m_instance >= 0 && m_focused)
        lookup_table_page_down (m_instance);
}

#endif // SCIM_HAS_CANDIDATES_WAYLAND

#if defined(SCIM_HAS_CANDIDATES_WAYLAND) || defined(SCIM_HAS_KIMPANEL)

void
WaylandFrontEnd::update_aux_string (int id, const WideString & str,
                                    const AttributeList & attrs)
{
    if (id != m_instance) return;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) { m_kimpanel.update_aux_string (str); return; }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().update_aux_string (str, attrs);
        refresh_candidates_ui ();
    }
#endif
}

void
WaylandFrontEnd::show_aux_string (int id)
{
    if (id != m_instance) return;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) { m_kimpanel.show_aux_string (true); return; }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().show_aux_string ();
        refresh_candidates_ui ();
    }
#endif
}

void
WaylandFrontEnd::hide_aux_string (int id)
{
    if (id != m_instance) return;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) { m_kimpanel.show_aux_string (false); return; }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().hide_aux_string ();
        refresh_candidates_ui ();
    }
#endif
}

void
WaylandFrontEnd::update_lookup_table (int id, const LookupTable & table)
{
    if (id != m_instance) return;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) { m_kimpanel.update_lookup_table (table); return; }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().update_lookup_table (table);
        refresh_candidates_ui ();
    }
#endif
}

void
WaylandFrontEnd::show_lookup_table (int id)
{
    if (id != m_instance) return;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) { m_kimpanel.show_lookup_table (true); return; }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().show_lookup_table ();
        refresh_candidates_ui ();
    }
#endif
}

void
WaylandFrontEnd::hide_lookup_table (int id)
{
    if (id != m_instance) return;
#ifdef SCIM_HAS_KIMPANEL
    if (use_kimpanel_ui ()) { m_kimpanel.show_lookup_table (false); return; }
#endif
#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    if (m_candidates_ui.is_ready ()) {
        m_candidates_ui.ui ().hide_lookup_table ();
        refresh_candidates_ui ();
    }
#endif
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
WaylandFrontEnd::handle_im_activate (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->im_activate ();
}

void
WaylandFrontEnd::handle_im_deactivate (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->im_deactivate ();
}

void
WaylandFrontEnd::handle_im_surrounding_text (void *data, struct zwp_input_method_v2 *,
                                             const char *text, uint32_t cursor, uint32_t anchor)
{
    static_cast<WaylandFrontEnd *> (data)->im_surrounding_text (text, cursor, anchor);
}

void
WaylandFrontEnd::handle_im_text_change_cause (void *, struct zwp_input_method_v2 *,
                                              uint32_t)
{
}

void
WaylandFrontEnd::handle_im_content_type (void *, struct zwp_input_method_v2 *,
                                         uint32_t, uint32_t)
{
}

void
WaylandFrontEnd::handle_im_done (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->im_done ();
}

void
WaylandFrontEnd::handle_im_unavailable (void *data, struct zwp_input_method_v2 *)
{
    static_cast<WaylandFrontEnd *> (data)->im_unavailable ();
}

void
WaylandFrontEnd::handle_kb_keymap (void *data, struct zwp_input_method_keyboard_grab_v2 *,
                                   uint32_t format, int32_t fd, uint32_t size)
{
    static_cast<WaylandFrontEnd *> (data)->kb_keymap (format, fd, size);
}

void
WaylandFrontEnd::handle_kb_key (void *data, struct zwp_input_method_keyboard_grab_v2 *,
                                uint32_t serial, uint32_t time, uint32_t key, uint32_t state)
{
    static_cast<WaylandFrontEnd *> (data)->kb_key (serial, time, key, state);
}

void
WaylandFrontEnd::handle_kb_modifiers (void *data, struct zwp_input_method_keyboard_grab_v2 *,
                                      uint32_t serial, uint32_t mods_depressed,
                                      uint32_t mods_latched, uint32_t mods_locked, uint32_t group)
{
    static_cast<WaylandFrontEnd *> (data)->kb_modifiers (serial, mods_depressed,
                                                         mods_latched, mods_locked, group);
}

void
WaylandFrontEnd::handle_kb_repeat_info (void *, struct zwp_input_method_keyboard_grab_v2 *,
                                        int32_t, int32_t)
{
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
