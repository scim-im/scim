/**
 * @file scim_ibus_frontend.h
 * @brief An IBus engine FrontEnd for GNOME (and any IBus desktop).
 *
 * IBusFrontEnd is a libibus client: it registers an IBus engine ("scim") with
 * ibus-daemon and bridges the IBusEngine <-> SCIM backend contract. An IBus
 * engine's job IS the FrontEndBase contract (keys in; commit / preedit / aux /
 * lookup out), so this is a sibling of x11.so / wayland.so, launched by
 * ibus-daemon via the component manifest's <exec> (scim -f ibus).
 *
 * gnome-shell renders the lookup table from the IBus data, so no candidate UI
 * (Cairo / kimpanel) is involved here.
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

#include <ibus.h>
#include "scim_stl_map.h"

using namespace scim;

// Per-IBusEngine bridge state.
struct IBusEngineData;

// One SCIM IMEngine factory, as exposed to IBus.
struct IBusEngineInfo {
    String  uuid;       // SCIM factory UUID == IBus engine name
    String  name;       // display name (utf8)
    String  language;
    String  icon;
};

class IBusFrontEnd : public FrontEndBase
{
    ConfigPointer   m_config;

    IBusBus        *m_bus;
    IBusFactory    *m_factory;
    bool            m_embedded;      // launched by ibus-daemon (--ibus)
    bool            m_xml_mode;      // print the component XML and exit (--xml)

    // siid -> engine bridge state.
    scim_map<int, IBusEngineData *> m_engines;

    // The engine whose key is currently being processed (for forwarding).
    IBusEngineData *m_current;

    // Panel client for the SCIM status/property toolbar (half/full width,
    // punctuation, ...). Only properties/status go here; preedit/aux/lookup are
    // rendered by gnome-shell via ibus. The panel socket fd is serviced through
    // a GIOChannel in the ibus (GLib) main loop.
    PanelClient     m_panel_client;
    GIOChannel     *m_panel_iochannel;
    guint           m_panel_watch_in;
    guint           m_panel_watch_err;
    guint           m_panel_watch_hup;
    String          m_display_name;
    int             m_panel_focus_siid;   // siid currently focused in the panel

public:
    IBusFrontEnd (const BackEndPointer &backend,
                  const ConfigPointer  &config,
                  bool embedded,
                  bool xml_mode);
    virtual ~IBusFrontEnd ();

protected:
    // Backend -> IBus engine.
    virtual void show_preedit_string   (int id);
    virtual void show_aux_string       (int id);
    virtual void show_lookup_table     (int id);
    virtual void hide_preedit_string   (int id);
    virtual void hide_aux_string       (int id);
    virtual void hide_lookup_table     (int id);
    virtual void update_preedit_caret  (int id, int caret);
    virtual void update_preedit_string (int id, const WideString & str, const AttributeList & attrs);
    virtual void update_aux_string     (int id, const WideString & str, const AttributeList & attrs);
    virtual void update_lookup_table   (int id, const LookupTable & table);
    virtual void commit_string         (int id, const WideString & str);
    virtual void forward_key_event     (int id, const KeyEvent & key);

    // Backend -> panel toolbar (properties/status only).
    virtual void register_properties   (int id, const PropertyList & properties);
    virtual void update_property       (int id, const Property     & property);

public:
    virtual void init (int argc, char **argv);
    virtual void run ();

private:
    IBusEngineData * find_engine (int siid);
    void send_preedit (IBusEngineData *d);
    void reload_config_callback (const ConfigPointer &config);

    // Expose every enabled SCIM factory as an IBus engine.
    void enumerate_engines (std::vector<IBusEngineInfo> &out);
    IBusComponent * build_component ();
    void print_component_xml ();

    // Panel (status/property toolbar) connection + slots.
    bool panel_open ();
    void panel_close ();
    void panel_slot_trigger_property (int context, const String &property);
    void panel_slot_reload_config    (int context);
    static gboolean cb_panel_io (GIOChannel *source, GIOCondition cond, gpointer data);

    // Factory / engine bridge.
    IBusEngine * create_engine (const gchar *engine_name);
    void         destroy_engine (IBusEngineData *d);

    void engine_process_key (IBusEngineData *d, guint keyval, guint keycode, guint state, gboolean *consumed);
    void engine_focus_in  (IBusEngineData *d);
    void engine_focus_out (IBusEngineData *d);
    void engine_reset     (IBusEngineData *d);
    void engine_page_up   (IBusEngineData *d);
    void engine_page_down (IBusEngineData *d);
    void engine_cursor_up   (IBusEngineData *d);
    void engine_cursor_down (IBusEngineData *d);
    void engine_candidate_clicked (IBusEngineData *d, guint index, guint button, guint state);

public:
    // Static libibus trampolines (referenced by g_signal_connect in the .cpp).
    static IBusEngine * cb_create_engine (IBusFactory *factory, const gchar *engine_name, gpointer user_data);

    static gboolean cb_process_key_event (IBusEngine *engine, guint keyval, guint keycode, guint state, gpointer user_data);
    static void cb_focus_in  (IBusEngine *engine, gpointer user_data);
    static void cb_focus_out (IBusEngine *engine, gpointer user_data);
    static void cb_reset     (IBusEngine *engine, gpointer user_data);
    static void cb_enable    (IBusEngine *engine, gpointer user_data);
    static void cb_disable   (IBusEngine *engine, gpointer user_data);
    static void cb_page_up   (IBusEngine *engine, gpointer user_data);
    static void cb_page_down (IBusEngine *engine, gpointer user_data);
    static void cb_cursor_up   (IBusEngine *engine, gpointer user_data);
    static void cb_cursor_down (IBusEngine *engine, gpointer user_data);
    static void cb_candidate_clicked (IBusEngine *engine, guint index, guint button, guint state, gpointer user_data);
    static void cb_engine_destroy (gpointer user_data, GObject *where);
    static void cb_disconnected (IBusBus *bus, gpointer user_data);
};

/*
vi:ts=4:nowrap:ai:expandtab
*/
