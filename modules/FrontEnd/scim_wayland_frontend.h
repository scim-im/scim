/**
 * @file scim_wayland_frontend.h
 * @brief A Wayland input-method-v2 FrontEnd for native Wayland text input.
 *
 * WaylandFrontEnd is a zwp_input_method_v2 client: it grabs the keyboard,
 * feeds keys to the SCIM backend and sends commit/preedit back to the
 * compositor, which relays to the focused zwp_text_input_v3 application.
 * Unconsumed keys are forwarded to the app via zwp_virtual_keyboard_v1.
 *
 * Candidate/aux UI (own popup surface) is added in a later step; this module
 * handles the text-input transport only.
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
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA  02111-1307  USA
 */

#pragma once

#include <wayland-client.h>
#include <xkbcommon/xkbcommon.h>

#include "input-method-unstable-v2-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

#ifdef SCIM_HAS_PANEL_UI_WAYLAND
#include "scim_panel_ui_wayland.h"
#endif

using namespace scim;

class WaylandFrontEnd : public FrontEndBase
{
    ConfigPointer   m_config;

    String          m_language;
    int             m_instance;       // the single IMEngine instance id (siid)
    bool            m_focused;        // input-method active on this seat

    // Wayland globals.
    struct wl_display    *m_display;
    struct wl_registry   *m_registry;
    struct wl_seat       *m_seat;
    struct wl_compositor *m_compositor;
    struct wl_shm        *m_shm;
    uint32_t            m_seat_name;

    struct zwp_input_method_manager_v2      *m_im_manager;
    struct zwp_input_method_v2              *m_input_method;
    struct zwp_input_method_keyboard_grab_v2 *m_grab;
    struct zwp_virtual_keyboard_manager_v1  *m_vk_manager;
    struct zwp_virtual_keyboard_v1          *m_virtual_keyboard;

    // input-method-v2 serial: number of done events received; passed to commit.
    uint32_t        m_serial;

    // Pending preedit (double-buffered until the next commit).
    WideString      m_preedit_str;
    int             m_preedit_caret;  // caret in characters

    // xkb state for the grabbed keyboard.
    struct xkb_context *m_xkb_context;
    struct xkb_keymap  *m_xkb_keymap;
    struct xkb_state   *m_xkb_state;

    // The key currently being processed (for forwarding unconsumed keys).
    bool            m_have_current_key;
    uint32_t        m_current_key_time;
    uint32_t        m_current_key_code;   // evdev keycode (raw wire value)
    uint32_t        m_current_key_state;
    bool            m_current_key_forwarded;

    bool            m_should_exit;

#ifdef SCIM_HAS_PANEL_UI_WAYLAND
    // In-process Cairo renderer drawn onto an input-popup-surface-v2 for the
    // aux string and candidate lookup table. Preedit still goes to the app via
    // set_preedit_string.
    PanelUIWayland  m_panel_ui;
#endif

public:
    WaylandFrontEnd (const BackEndPointer &backend,
                     const ConfigPointer  &config);
    virtual ~WaylandFrontEnd ();

protected:
    // Backend -> app (only the text-input transport for now).
    virtual void update_preedit_caret  (int id, int caret);
    virtual void update_preedit_string (int id, const WideString & str, const AttributeList & attrs);
    virtual void show_preedit_string   (int id);
    virtual void hide_preedit_string   (int id);
    virtual void commit_string         (int id, const WideString & str);
    virtual void forward_key_event     (int id, const KeyEvent & key);

#ifdef SCIM_HAS_PANEL_UI_WAYLAND
    // Aux string + candidate lookup table -> the input-popup-surface renderer.
    virtual void update_aux_string     (int id, const WideString & str, const AttributeList & attrs);
    virtual void show_aux_string       (int id);
    virtual void hide_aux_string       (int id);
    virtual void update_lookup_table   (int id, const LookupTable & table);
    virtual void show_lookup_table     (int id);
    virtual void hide_lookup_table     (int id);
#endif

public:
    virtual void init (int argc, char **argv);
    virtual void run ();

    // Cooperative-run interface: lets the launcher service this frontend in a
    // shared select() loop alongside other frontends (e.g. x11 for XWayland).
    bool poll_fds (std::vector<int> &fds);
    void process_events ();
    bool has_exited () const { return m_should_exit; }

private:
    void reload_config_callback (const ConfigPointer &config);

    // Registry.
    void registry_global (uint32_t name, const char *interface, uint32_t version);
    void registry_global_remove (uint32_t name);

    // input-method-v2 events.
    void im_activate ();
    void im_deactivate ();
    void im_surrounding_text (const char *text, uint32_t cursor, uint32_t anchor);
    void im_done ();
    void im_unavailable ();

    // keyboard grab events.
    void kb_keymap (uint32_t format, int32_t fd, uint32_t size);
    void kb_key (uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void kb_modifiers (uint32_t serial, uint32_t mods_depressed,
                       uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

    // Helpers.
    void ensure_instance ();
    void start_grab ();
    void send_preedit ();
    KeyEvent wayland_key_to_scim (uint32_t key, uint32_t state) const;
    void forward_current_key ();

#ifdef SCIM_HAS_PANEL_UI_WAYLAND
    void refresh_panel_ui ();
    void configure_panel_ui ();
    void panel_ui_select_candidate (int cand_index);
    void panel_ui_page_up ();
    void panel_ui_page_down ();
#endif

public:
    // Static Wayland listener trampolines (referenced by file-scope listener
    // tables in the .cpp; must be accessible there).
    static void handle_registry_global (void *data, struct wl_registry *r,
                                        uint32_t name, const char *interface,
                                        uint32_t version);
    static void handle_registry_global_remove (void *data, struct wl_registry *r,
                                               uint32_t name);

    static void handle_im_activate (void *data, struct zwp_input_method_v2 *im);
    static void handle_im_deactivate (void *data, struct zwp_input_method_v2 *im);
    static void handle_im_surrounding_text (void *data, struct zwp_input_method_v2 *im,
                                            const char *text, uint32_t cursor, uint32_t anchor);
    static void handle_im_text_change_cause (void *data, struct zwp_input_method_v2 *im,
                                             uint32_t cause);
    static void handle_im_content_type (void *data, struct zwp_input_method_v2 *im,
                                        uint32_t hint, uint32_t purpose);
    static void handle_im_done (void *data, struct zwp_input_method_v2 *im);
    static void handle_im_unavailable (void *data, struct zwp_input_method_v2 *im);

    static void handle_kb_keymap (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                  uint32_t format, int32_t fd, uint32_t size);
    static void handle_kb_key (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                               uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void handle_kb_modifiers (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                     uint32_t serial, uint32_t mods_depressed,
                                     uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
    static void handle_kb_repeat_info (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                       int32_t rate, int32_t delay);
};

/*
vi:ts=4:nowrap:ai:expandtab
*/
