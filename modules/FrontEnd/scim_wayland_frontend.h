/**
 * @file scim_wayland_frontend.h
 * @brief A Wayland FrontEnd speaking either input-method protocol generation.
 *
 * WaylandFrontEnd is a zwp_input_method client that talks whichever of the two
 * input-method protocols the compositor actually offers: input-method-v2
 * (wlroots and friends) or input-method-v1 (KWin, which never adopted v2).
 *
 * The protocol is detected, not configured. Which one exists is the
 * compositor's decision, so a switch could only ever be set wrong -- and a
 * wrong setting means an input method that silently does nothing. Detection
 * also keeps this to a single wl_display_connect (): KWin hands its input
 * method a pre-connected fd in WAYLAND_SOCKET and libwayland consumes it *and*
 * unsets the variable, so a second frontend trying to connect afterwards has no
 * route to the compositor at all. One module, one connection, protocol chosen
 * from the registry. --im-protocol=v1|v2 forces a generation for testing.
 *
 * The protocols differ in shape, not just in naming:
 *
 *  - v2 hands out one long-lived zwp_input_method_v2 for the seat and reports
 *    focus with activate/deactivate. v1 has no manager: the global *is* the
 *    input method, and each activation delivers a fresh
 *    zwp_input_method_context_v1 that is destroyed on deactivate. All v1 text
 *    requests go to that context, so there is nothing to send while unfocused.
 *  - v2 counts its own serial (one per "done" event). v1 is told the serial by
 *    the compositor via commit_state, and it must be echoed on every
 *    commit_string / preedit_string.
 *  - v2 grabs the keyboard through a dedicated
 *    zwp_input_method_keyboard_grab_v2 and returns unconsumed keys through a
 *    separate zwp_virtual_keyboard_v1; v1's grab_keyboard returns a plain
 *    wl_keyboard and unconsumed keys go back via the context's key/modifiers.
 *  - v2 positions candidates with zwp_input_popup_surface_v2 (anchored to the
 *    text cursor). v1 has only zwp_input_panel_v1 overlay panels, placed by
 *    the compositor.
 *
 * Everything above the protocol -- engine instance, panel/properties, hotkeys,
 * xkb translation, candidates, the run loop -- is shared.
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

#include "input-method-unstable-v1-client-protocol.h"
#include "input-method-unstable-v2-client-protocol.h"
#include "virtual-keyboard-unstable-v1-client-protocol.h"

// The candidate UI interface only: header-only, no cairo and no D-Bus, so it is
// available whatever was built, and the state calls below need no #ifdef.
#include "scim_candidates_sink.h"

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
#include "scim_candidates_wayland.h"
#endif

#ifdef SCIM_HAS_KIMPANEL
#include "scim_kimpanel_agent.h"
#endif

using namespace scim;

class WaylandFrontEnd : public FrontEndBase
{
    ConfigPointer   m_config;

    String          m_language;
    int             m_instance;       // the single IMEngine instance id (siid)
    bool            m_focused;        // a context is active

    // Whether ensure_instance () has already complained. Every focus change
    // retries, and one warning is a diagnosis while a stream of them is noise.
    bool            m_instance_warned;

    // Whether the engine is converting. When off, keys pass straight to the
    // application, which is how the user types plain ASCII. There is no
    // "input source" concept here as there is on ibus desktops -- the
    // compositor knows only "an input method is running" -- so without the
    // trigger hotkey below there would be no way out of the engine at all.
    bool            m_im_on;

    FrontEndHotkeyMatcher   m_frontend_hotkey_matcher;
    IMEngineHotkeyMatcher   m_imengine_hotkey_matcher;

    // Modifier bits that may take part in matching. Everything else (NumLock,
    // Super, ...) is masked off the KeyEvent first, or a hotkey configured as
    // "Control+space" would never match while an unrelated modifier happened to
    // be latched. x11.so does the same.
    uint16          m_valid_key_mask;

    // Wayland globals.
    struct wl_display    *m_display;
    struct wl_registry   *m_registry;
    struct wl_seat       *m_seat;
    struct wl_compositor *m_compositor;
    struct wl_shm        *m_shm;
    uint32_t            m_seat_name;

    // Which generation we settled on; decided in init () from the registry.
    enum Protocol { PROTO_NONE, PROTO_V1, PROTO_V2 };
    Protocol        m_proto;
    Protocol        m_forced_proto;   // --im-protocol override, PROTO_NONE if unset

    // input-method-v1 objects.
    struct zwp_input_method_v1 *m_v1_input_method;
    struct zwp_input_panel_v1  *m_v1_input_panel;
    // Valid only between activate and deactivate; all v1 text requests need it.
    struct zwp_input_method_context_v1 *m_v1_context;
    struct wl_keyboard                 *m_v1_keyboard;

    // input-method-v2 objects.
    struct zwp_input_method_manager_v2       *m_v2_manager;
    struct zwp_input_method_v2               *m_v2_input_method;
    struct zwp_input_method_keyboard_grab_v2 *m_v2_grab;
    struct zwp_virtual_keyboard_manager_v1   *m_vk_manager;
    struct zwp_virtual_keyboard_v1           *m_virtual_keyboard;

    // v1: handed to us by commit_state. v2: counted from its done events.
    // Either way it must be echoed on commit_string / preedit_string.
    uint32_t        m_serial;

    // Pending preedit.
    WideString      m_preedit_str;
    AttributeList   m_preedit_attrs;
    int             m_preedit_caret;  // caret in characters
    bool            m_preedit_shown;

    // /FrontEnd/OnTheSpot. True (the default) sends the preedit to the
    // application, which renders it inline. False draws it ourselves next to the
    // candidates and never touches the application's document until commit --
    // an application that treats preedit as a document edit re-runs its layout
    // on every keystroke, which is unusable in a large document.
    bool            m_on_the_spot;

    // xkb state for the grabbed keyboard.
    struct xkb_context *m_xkb_context;
    struct xkb_keymap  *m_xkb_keymap;
    struct xkb_state   *m_xkb_state;

    // The key currently being processed (for forwarding unconsumed keys).
    bool            m_have_current_key;
    uint32_t        m_current_key_serial;
    uint32_t        m_current_key_time;
    uint32_t        m_current_key_code;   // evdev keycode (raw wire value)
    uint32_t        m_current_key_state;
    bool            m_current_key_forwarded;

    bool            m_should_exit;

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // Cairo renderer on a zwp_input_panel_surface_v1 overlay panel for the aux
    // string and candidate lookup table. Preedit still goes to the app.
    CandidatesWayland  m_candidates_ui;
#endif

#ifdef SCIM_HAS_KIMPANEL
    // KDE's own candidate UI (D-Bus). Preferred here: the compositor that
    // speaks v1 in practice is KWin, where kimpanel is the native look and
    // places candidates better than an overlay panel can.
    KimpanelAgent   m_kimpanel;
#endif

    // Whichever of the two above is in use, or null when neither is available
    // and scim-panel-gtk draws the candidates instead. Everything pushes state at
    // it without asking which; select_sink () is the only thing that changes it,
    // and it can change while running, when a panel widget is added or removed.
    CandidatesSink *m_sink;

    // Panel client for the status/property UI (tray item or toolbar). Serviced
    // from poll_fds () / process_events () along with the wayland fd.
    PanelClient     m_panel_client;
    bool            m_panel_open;

    // Dropped in the destructor: the slot is bound to this object, and leaving
    // it in the config's signal outlives us if the module is ever unloaded.
    Connection      m_config_reload_connection;

public:
    WaylandFrontEnd (const BackEndPointer &backend,
                       const ConfigPointer  &config);
    virtual ~WaylandFrontEnd ();

protected:
    // Backend -> app (text-input transport).
    virtual void update_preedit_caret  (int id, int caret);
    virtual void update_preedit_string (int id, const WideString & str, const AttributeList & attrs);
    virtual void show_preedit_string   (int id);
    virtual void hide_preedit_string   (int id);
    virtual void commit_string         (int id, const WideString & str);
    virtual void forward_key_event     (int id, const KeyEvent & key);

    // Backend -> panel (properties/status only).
    virtual void register_properties   (int id, const PropertyList & properties);
    virtual void update_property       (int id, const Property     & property);

#if defined(SCIM_HAS_CANDIDATES_WAYLAND) || defined(SCIM_HAS_KIMPANEL)
    // Aux string + candidate lookup table -> the overlay panel renderer or
    // (on KDE) the kimpanel D-Bus backend.
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
    // shared select () loop alongside other frontends.
    bool poll_fds (std::vector<int> &fds);
    void process_events ();
    bool has_exited () const { return m_should_exit; }

private:
    void reload_config_callback (const ConfigPointer &config);

    // Panel connection + slots.
    bool panel_open ();
    void panel_close ();
    void panel_slot_trigger_property (int context, const String &property);
    void panel_slot_reload_config    (int context);
    void panel_slot_change_factory   (int context, const String &uuid);
    void panel_slot_request_factory_menu (int context);

    // Registry.
    void registry_global (uint32_t name, const char *interface, uint32_t version);
    void registry_global_remove (uint32_t name);

    // Protocol-specific operations; each branches on m_proto so that callers
    // above stay protocol-agnostic.
    void proto_start_grab ();
    void proto_send_preedit (const String &utf8, int32_t cursor);
    void proto_clear_preedit ();
    void proto_commit_string (const String &utf8);
    void proto_forward_key ();
    bool proto_candidates_init ();
    const char * proto_name () const;

    // Focus transitions, shared by both protocols' activate/deactivate.
    void enter_focus ();
    void leave_focus ();

    // input-method-v1 events.
    void v1_activate (struct zwp_input_method_context_v1 *context);
    void v1_deactivate (struct zwp_input_method_context_v1 *context);

    // input-method-v2 events.
    void v2_activate ();
    void v2_deactivate ();
    void v2_done ();
    void v2_unavailable ();

    // input-method-context-v1 events.
    void ctx_surrounding_text (const char *text, uint32_t cursor, uint32_t anchor);
    void ctx_reset ();
    void ctx_content_type (uint32_t hint, uint32_t purpose);
    void ctx_invoke_action (uint32_t button, uint32_t index);
    void ctx_commit_state (uint32_t serial);
    void ctx_preferred_language (const char *language);

    // Grabbed wl_keyboard events.
    void kb_keymap (uint32_t format, int32_t fd, uint32_t size);
    void kb_key (uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    void kb_modifiers (uint32_t serial, uint32_t mods_depressed,
                       uint32_t mods_latched, uint32_t mods_locked, uint32_t group);

    // Hotkeys: trigger (Control+space by default) toggles the engine on and
    // off, and the factory hotkeys switch engines.
    bool filter_hotkeys (const KeyEvent &scimkey);
    void turn_on_im ();
    void turn_off_im ();
    void switch_factory (const String &sfid);
    void panel_req_show_factory_menu ();
    void panel_req_update_factory_info ();

    // Helpers.
    void ensure_instance ();
    void drop_keyboard ();
    void send_modifiers_map ();
    void send_preedit ();
    void clear_preedit_on_app ();
    // Draw the preedit ourselves (kimpanel or the Cairo popup) instead of
    // handing it to the application.
    void preedit_to_panel (bool visible);
    KeyEvent wayland_key_to_scim (uint32_t key, uint32_t state) const;
    void forward_current_key ();

    // Choose between kimpanel and our overlay panel from what is available now,
    // and hand over if that has changed. Called at startup and whenever a panel
    // widget appears or goes away.
    void select_sink ();

    /** @name Sink -> frontend, whichever candidate UI is in use @{ */
    void sink_select_candidate (int cand_index);
    void sink_page_up ();
    void sink_page_down ();
    void sink_move_preedit_caret (int pos);
    /** @} */

#ifdef SCIM_HAS_CANDIDATES_WAYLAND
    // Not part of the sink: only a UI we draw ourselves has a theme to set.
    void configure_candidates_ui ();
#else
    void configure_candidates_ui () { }
#endif

public:
    // Static Wayland listener trampolines (referenced by file-scope listener
    // tables in the .cpp; must be accessible there).
    static void handle_registry_global (void *data, struct wl_registry *r,
                                        uint32_t name, const char *interface,
                                        uint32_t version);
    static void handle_registry_global_remove (void *data, struct wl_registry *r,
                                               uint32_t name);

    static void handle_v1_activate (void *data, struct zwp_input_method_v1 *im,
                                    struct zwp_input_method_context_v1 *context);
    static void handle_v1_deactivate (void *data, struct zwp_input_method_v1 *im,
                                      struct zwp_input_method_context_v1 *context);

    static void handle_v2_activate (void *data, struct zwp_input_method_v2 *im);
    static void handle_v2_deactivate (void *data, struct zwp_input_method_v2 *im);
    static void handle_v2_surrounding_text (void *data, struct zwp_input_method_v2 *im,
                                            const char *text, uint32_t cursor, uint32_t anchor);
    static void handle_v2_text_change_cause (void *data, struct zwp_input_method_v2 *im,
                                             uint32_t cause);
    static void handle_v2_content_type (void *data, struct zwp_input_method_v2 *im,
                                        uint32_t hint, uint32_t purpose);
    static void handle_v2_done (void *data, struct zwp_input_method_v2 *im);
    static void handle_v2_unavailable (void *data, struct zwp_input_method_v2 *im);

    static void handle_grab_keymap (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                    uint32_t format, int32_t fd, uint32_t size);
    static void handle_grab_key (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                 uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
    static void handle_grab_modifiers (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                       uint32_t serial, uint32_t mods_depressed,
                                       uint32_t mods_latched, uint32_t mods_locked, uint32_t group);
    static void handle_grab_repeat_info (void *data, struct zwp_input_method_keyboard_grab_v2 *g,
                                         int32_t rate, int32_t delay);

    static void handle_ctx_surrounding_text (void *data, struct zwp_input_method_context_v1 *c,
                                             const char *text, uint32_t cursor, uint32_t anchor);
    static void handle_ctx_reset (void *data, struct zwp_input_method_context_v1 *c);
    static void handle_ctx_content_type (void *data, struct zwp_input_method_context_v1 *c,
                                         uint32_t hint, uint32_t purpose);
    static void handle_ctx_invoke_action (void *data, struct zwp_input_method_context_v1 *c,
                                          uint32_t button, uint32_t index);
    static void handle_ctx_commit_state (void *data, struct zwp_input_method_context_v1 *c,
                                         uint32_t serial);
    static void handle_ctx_preferred_language (void *data, struct zwp_input_method_context_v1 *c,
                                               const char *language);

    static void handle_kb_keymap (void *data, struct wl_keyboard *kb,
                                  uint32_t format, int32_t fd, uint32_t size);
    static void handle_kb_enter (void *data, struct wl_keyboard *kb, uint32_t serial,
                                 struct wl_surface *surface, struct wl_array *keys);
    static void handle_kb_leave (void *data, struct wl_keyboard *kb, uint32_t serial,
                                 struct wl_surface *surface);
    static void handle_kb_key (void *data, struct wl_keyboard *kb, uint32_t serial,
                               uint32_t time, uint32_t key, uint32_t state);
    static void handle_kb_modifiers (void *data, struct wl_keyboard *kb, uint32_t serial,
                                     uint32_t mods_depressed, uint32_t mods_latched,
                                     uint32_t mods_locked, uint32_t group);
    static void handle_kb_repeat_info (void *data, struct wl_keyboard *kb,
                                       int32_t rate, int32_t delay);
};

/*
vi:ts=4:nowrap:ai:expandtab
*/
