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
#define Uses_SCIM_PANEL_CLIENT
#define Uses_SCIM_UTILITY
#define Uses_SCIM_DEBUG

#include <cstring>
#include <iostream>
#include <vector>

#include "scim_private.h"
#include "scim.h"

#include "scim_ibus_frontend.h"

#define scim_module_init           ibus_LTX_scim_module_init
#define scim_module_exit           ibus_LTX_scim_module_exit
#define scim_frontend_module_init  ibus_LTX_scim_frontend_module_init
#define scim_frontend_module_run   ibus_LTX_scim_frontend_module_run

#define SCIM_CONFIG_FRONTEND_IBUS_LANGUAGE  "/FrontEnd/IBus/Language"

#define SCIM_IBUS_BUS_NAME          "org.freedesktop.IBus.SCIM"
#define SCIM_IBUS_ENGINE_NAME       "scim"

using namespace scim;

// Per-IBusEngine bridge state.
struct IBusEngineData {
    IBusFrontEnd *frontend;
    IBusEngine   *engine;
    int           siid;

    WideString    preedit_str;
    int           preedit_caret;
    bool          preedit_visible;

    // Key currently being processed (for forwarding unconsumed keys).
    guint         cur_keyval;
    guint         cur_keycode;
    guint         cur_state;
    bool          cur_forwarded;

    IBusEngineData ()
        : frontend (0), engine (0), siid (-1),
          preedit_caret (0), preedit_visible (false),
          cur_keyval (0), cur_keycode (0), cur_state (0), cur_forwarded (false)
    { }
};

//Local static data
static Pointer <IBusFrontEnd> _scim_frontend (0);

//Module Interface
extern "C" {
    void scim_module_init (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Initializing IBus FrontEnd module...\n";
    }

    void scim_module_exit (void)
    {
        SCIM_DEBUG_FRONTEND(1) << "Exiting IBus FrontEnd module...\n";
        _scim_frontend.reset ();
    }

    void scim_frontend_module_init (const BackEndPointer &backend,
                                    const ConfigPointer &config,
                                    int argc,
                                    char **argv)
    {
        if (config.null () || backend.null ())
            throw FrontEndError (String ("IBus FrontEnd couldn't run without Config and BackEnd.\n"));

        if (_scim_frontend.null ()) {
            bool embedded = false;
            bool xml_mode = false;
            for (int i = 0; i < argc; ++i) {
                if (!argv[i]) continue;
                if (String (argv[i]) == "--ibus") embedded = true;
                if (String (argv[i]) == "--xml")  xml_mode = true;
            }

            SCIM_DEBUG_FRONTEND(1) << "Initializing IBus FrontEnd module (more)...\n";
            _scim_frontend = new IBusFrontEnd (backend, config, embedded, xml_mode);
            _scim_frontend->init (argc, argv);
        }
    }

    void scim_frontend_module_run (void)
    {
        if (!_scim_frontend.null ()) {
            SCIM_DEBUG_FRONTEND(1) << "Starting IBus FrontEnd module...\n";
            _scim_frontend->run ();
        }
    }
}

/* ------------------------------------------------------------------ */
/* Construction                                                        */
/* ------------------------------------------------------------------ */

IBusFrontEnd::IBusFrontEnd (const BackEndPointer &backend,
                            const ConfigPointer  &config,
                            bool embedded,
                            bool xml_mode)
    : FrontEndBase (backend),
      m_config (config),
      m_bus (0),
      m_factory (0),
      m_embedded (embedded),
      m_xml_mode (xml_mode),
      m_current (0),
      m_panel_iochannel (0),
      m_panel_watch_in (0),
      m_panel_watch_err (0),
      m_panel_watch_hup (0),
      m_panel_focus_siid (-1)
{
    if (!_scim_frontend.null () && _scim_frontend != this)
        throw FrontEndError (String ("IBus -- only one frontend can be created!"));
}

IBusFrontEnd::~IBusFrontEnd ()
{
    panel_close ();
    // Engines own the scim instances via their bridge data; drop any left.
    for (scim_map<int, IBusEngineData *>::iterator it = m_engines.begin ();
         it != m_engines.end (); ++it) {
        delete_instance (it->first);
        delete it->second;
    }
    if (m_factory) g_object_unref (m_factory);
    if (m_bus)     g_object_unref (m_bus);
}

/* ------------------------------------------------------------------ */
/* Init / run                                                          */
/* ------------------------------------------------------------------ */

void
IBusFrontEnd::reload_config_callback (const ConfigPointer & /*config*/)
{
}

void
IBusFrontEnd::init (int /*argc*/, char ** /*argv*/)
{
    reload_config_callback (m_config);
    m_config->signal_connect_reload (slot (this, &IBusFrontEnd::reload_config_callback));

    // --xml: emit the component manifest (one engine per enabled SCIM factory)
    // and stop, without touching ibus-daemon. Used to (re)generate the
    // installed component XML when the enabled engines change.
    if (m_xml_mode) {
        print_component_xml ();
        return;
    }

    ibus_init ();

    m_bus = ibus_bus_new ();
    if (!m_bus || !ibus_bus_is_connected (m_bus))
        throw FrontEndError (String ("IBus -- cannot connect to ibus-daemon."));

    g_signal_connect (m_bus, "disconnected",
                      G_CALLBACK (IBusFrontEnd::cb_disconnected), this);

    GDBusConnection *conn = ibus_bus_get_connection (m_bus);
    m_factory = ibus_factory_new (conn);
    g_object_ref_sink (m_factory);

    g_signal_connect (m_factory, "create-engine",
                      G_CALLBACK (IBusFrontEnd::cb_create_engine), this);

    if (m_embedded) {
        // Launched by ibus-daemon, which already knows our engines from the
        // installed component XML: just own the well-known name.
        ibus_bus_request_name (m_bus, SCIM_IBUS_BUS_NAME, 0);
    } else {
        // Standalone: register a component carrying every enabled SCIM factory
        // as an engine, so a running ibus-daemon learns them for this session.
        IBusComponent *component = build_component ();
        ibus_bus_register_component (m_bus, component);
        g_object_unref (component);
    }

    // Connect to (launching if needed) the SCIM panel for the status/property
    // toolbar. Non-fatal: if it can't be reached, input still works and
    // gnome-shell shows candidates. Prefer the X/XWayland display for the panel
    // window; fall back to the Wayland display.
    const char *disp = getenv ("DISPLAY");
    if (!disp || !*disp) disp = getenv ("WAYLAND_DISPLAY");
    m_display_name = disp ? String (disp) : String ();
    panel_open ();
}

void
IBusFrontEnd::run ()
{
    if (m_xml_mode)
        return;
    // Enter the IBus (GLib) main loop; returns when the bus disconnects.
    ibus_main ();
}

/* ------------------------------------------------------------------ */
/* Engine enumeration / component manifest                             */
/* ------------------------------------------------------------------ */

void
IBusFrontEnd::enumerate_engines (std::vector<IBusEngineInfo> &out)
{
    out.clear ();
    std::vector<String> uuids;
    get_factory_list_for_encoding (uuids, String ());   // all encodings
    for (size_t i = 0; i < uuids.size (); ++i) {
        IBusEngineInfo info;
        info.uuid     = uuids[i];
        info.name     = utf8_wcstombs (get_factory_name (uuids[i]));
        info.language = get_factory_language (uuids[i]);
        info.icon     = get_factory_icon_file (uuids[i]);
        if (!info.name.length ())
            info.name = uuids[i];
        out.push_back (info);
    }
}

IBusComponent *
IBusFrontEnd::build_component ()
{
    IBusComponent *component = ibus_component_new (
        SCIM_IBUS_BUS_NAME,
        "Smart Common Input Method",
        SCIM_VERSION,
        "LGPL",
        "SCIM developers",
        "https://github.com/scim-im/scim",
        "",
        "scim");

    std::vector<IBusEngineInfo> engines;
    enumerate_engines (engines);

    for (size_t i = 0; i < engines.size (); ++i) {
        IBusEngineDesc *desc = ibus_engine_desc_new (
            engines[i].uuid.c_str (),
            engines[i].name.c_str (),
            engines[i].name.c_str (),
            engines[i].language.length () ? engines[i].language.c_str () : "other",
            "GPL",
            "SCIM developers",
            engines[i].icon.c_str (),
            "us");
        ibus_component_add_engine (component, desc);
    }

    return component;
}

static String
xml_escape (const String &s)
{
    String r;
    for (size_t i = 0; i < s.length (); ++i) {
        char c = s[i];
        switch (c) {
        case '&': r += "&amp;";  break;
        case '<': r += "&lt;";   break;
        case '>': r += "&gt;";   break;
        default:  r += c;        break;
        }
    }
    return r;
}

void
IBusFrontEnd::print_component_xml ()
{
#ifndef SCIM_BINDIR
#define SCIM_BINDIR "/usr/local/bin"
#endif
    std::vector<IBusEngineInfo> engines;
    enumerate_engines (engines);

    std::cout <<
        "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
        "<!-- Generated by 'scim -f ibus --xml'. Regenerate when the enabled\n"
        "     SCIM engines change. -->\n"
        "<component>\n"
        "    <name>" SCIM_IBUS_BUS_NAME "</name>\n"
        "    <description>Smart Common Input Method</description>\n"
        "    <exec>" SCIM_BINDIR "/scim -f ibus --no-socket -- --ibus</exec>\n"
        "    <version>" SCIM_VERSION "</version>\n"
        "    <author>SCIM developers</author>\n"
        "    <license>LGPL</license>\n"
        "    <homepage>https://github.com/scim-im/scim</homepage>\n"
        "    <textdomain>scim</textdomain>\n"
        "    <engines>\n";

    for (size_t i = 0; i < engines.size (); ++i) {
        std::cout <<
            "        <engine>\n"
            "            <name>"        << xml_escape (engines[i].uuid) << "</name>\n"
            "            <longname>"    << xml_escape (engines[i].name) << "</longname>\n"
            "            <description>" << xml_escape (engines[i].name) << "</description>\n"
            "            <language>"    << xml_escape (engines[i].language.length () ? engines[i].language : String ("other")) << "</language>\n"
            "            <license>GPL</license>\n"
            "            <author>SCIM developers</author>\n"
            "            <icon>"        << xml_escape (engines[i].icon) << "</icon>\n"
            "            <layout>us</layout>\n"
            "            <rank>1</rank>\n"
            "        </engine>\n";
    }

    std::cout <<
        "    </engines>\n"
        "</component>\n";
}

/* ------------------------------------------------------------------ */
/* Engine lifecycle                                                    */
/* ------------------------------------------------------------------ */

IBusEngine *
IBusFrontEnd::create_engine (const gchar *engine_name)
{
    static int s_serial = 0;

    if (!engine_name)
        return 0;

    // Map the requested IBus engine name to a SCIM factory: each factory's
    // UUID is its engine name; the generic "scim" name uses the default.
    String encoding = String ("UTF-8");
    String sfid;
    if (strcmp (engine_name, SCIM_IBUS_ENGINE_NAME) == 0) {
        String language = m_config->read (String (SCIM_CONFIG_FRONTEND_IBUS_LANGUAGE), String (""));
        if (!language.length ())
            language = scim_get_locale_language (scim_get_current_locale ());
        sfid = get_default_factory (language, encoding);
    } else {
        // A specific engine was requested. If it isn't loaded, it may have just
        // been enabled: re-apply the disabled list and re-check before giving
        // up. ibus-daemon owns this process's lifetime, so we do NOT watch the
        // config -- instead we reload lazily, right when an engine is asked for.
        // (When the process wasn't running, its next launch reads fresh config
        // anyway, so the two together cover every case.)
        if (!validate_factory (String (engine_name)))
            reload_disabled_factories ();

        if (validate_factory (String (engine_name))) {
            sfid = String (engine_name);
        } else {
            SCIM_DEBUG_FRONTEND(1) << "IBus -- unknown engine requested: " << engine_name << "\n";
            return 0;
        }
    }

    GDBusConnection *conn = ibus_bus_get_connection (m_bus);
    gchar *path = g_strdup_printf ("/org/freedesktop/IBus/Engine/%d", ++s_serial);
    IBusEngine *engine = ibus_engine_new (engine_name, path, conn);
    g_free (path);
    if (!engine)
        return 0;

    int siid = new_instance (sfid, encoding);
    if (siid < 0) {
        SCIM_DEBUG_FRONTEND(1) << "IBus -- failed to create an IMEngine instance.\n";
        g_object_unref (engine);
        return 0;
    }

    IBusEngineData *d = new IBusEngineData ();
    d->frontend = this;
    d->engine   = engine;
    d->siid     = siid;
    m_engines[siid] = d;

    g_signal_connect (engine, "process-key-event", G_CALLBACK (cb_process_key_event), d);
    g_signal_connect (engine, "focus-in",  G_CALLBACK (cb_focus_in),  d);
    g_signal_connect (engine, "focus-out", G_CALLBACK (cb_focus_out), d);
    g_signal_connect (engine, "reset",     G_CALLBACK (cb_reset),     d);
    g_signal_connect (engine, "enable",    G_CALLBACK (cb_enable),    d);
    g_signal_connect (engine, "disable",   G_CALLBACK (cb_disable),   d);
    g_signal_connect (engine, "page-up",   G_CALLBACK (cb_page_up),   d);
    g_signal_connect (engine, "page-down", G_CALLBACK (cb_page_down), d);
    g_signal_connect (engine, "cursor-up",   G_CALLBACK (cb_cursor_up),   d);
    g_signal_connect (engine, "cursor-down", G_CALLBACK (cb_cursor_down), d);
    g_signal_connect (engine, "candidate-clicked", G_CALLBACK (cb_candidate_clicked), d);
    g_object_weak_ref (G_OBJECT (engine), cb_engine_destroy, d);

    return engine;
}

void
IBusFrontEnd::destroy_engine (IBusEngineData *d)
{
    if (!d) return;
    if (m_current == d)
        m_current = 0;
    if (m_panel_client.get_connection_number () >= 0) {
        m_panel_client.prepare (d->siid);
        m_panel_client.remove_input_context (d->siid);
        m_panel_client.send ();
    }
    if (m_panel_focus_siid == d->siid)
        m_panel_focus_siid = -1;
    m_engines.erase (d->siid);
    delete_instance (d->siid);
    delete d;
}

IBusEngineData *
IBusFrontEnd::find_engine (int siid)
{
    scim_map<int, IBusEngineData *>::iterator it = m_engines.find (siid);
    return it == m_engines.end () ? 0 : it->second;
}

/* ------------------------------------------------------------------ */
/* Panel (status/property toolbar)                                     */
/* ------------------------------------------------------------------ */

void
IBusFrontEnd::register_properties (int id, const PropertyList &properties)
{
    if (id == m_panel_focus_siid && m_panel_client.get_connection_number () >= 0) {
        m_panel_client.prepare (id);
        m_panel_client.register_properties (id, properties);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::update_property (int id, const Property &property)
{
    if (id == m_panel_focus_siid && m_panel_client.get_connection_number () >= 0) {
        m_panel_client.prepare (id);
        m_panel_client.update_property (id, property);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::panel_slot_trigger_property (int context, const String &property)
{
    // A toolbar property was clicked (half/full width, punctuation, ...).
    if (find_engine (context)) {
        m_panel_client.prepare (context);
        trigger_property (context, property);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::panel_slot_reload_config (int /*context*/)
{
    if (!m_config.null ())
        m_config->reload ();
}

gboolean
IBusFrontEnd::cb_panel_io (GIOChannel * /*source*/, GIOCondition cond, gpointer /*data*/)
{
    if (_scim_frontend.null ())
        return FALSE;
    IBusFrontEnd *fe = _scim_frontend;

    if (cond == G_IO_IN) {
        if (!fe->m_panel_client.filter_event ()) {
            fe->panel_close ();
            fe->panel_open ();
            return FALSE;
        }
    } else if (cond == G_IO_ERR || cond == G_IO_HUP) {
        fe->panel_close ();
        fe->panel_open ();
        return FALSE;
    }
    return TRUE;
}

bool
IBusFrontEnd::panel_open ()
{
    // Connects to the SCIM panel (launching scim-panel-gtk on demand). Only the
    // toolbar (properties/status) is used here; candidates come from gnome-shell.
    if (m_panel_client.open_connection (m_config->get_name (), m_display_name) < 0) {
        SCIM_DEBUG_FRONTEND(1) << "IBus -- cannot connect to the SCIM panel.\n";
        return false;
    }

    int fd = m_panel_client.get_connection_number ();
    if (fd < 0)
        return false;

    m_panel_client.signal_connect_trigger_property (slot (this, &IBusFrontEnd::panel_slot_trigger_property));
    m_panel_client.signal_connect_reload_config    (slot (this, &IBusFrontEnd::panel_slot_reload_config));

    m_panel_iochannel = g_io_channel_unix_new (fd);
    m_panel_watch_in  = g_io_add_watch (m_panel_iochannel, G_IO_IN,  cb_panel_io, 0);
    m_panel_watch_err = g_io_add_watch (m_panel_iochannel, G_IO_ERR, cb_panel_io, 0);
    m_panel_watch_hup = g_io_add_watch (m_panel_iochannel, G_IO_HUP, cb_panel_io, 0);
    return true;
}

void
IBusFrontEnd::panel_close ()
{
    if (m_panel_watch_in)  { g_source_remove (m_panel_watch_in);  m_panel_watch_in  = 0; }
    if (m_panel_watch_err) { g_source_remove (m_panel_watch_err); m_panel_watch_err = 0; }
    if (m_panel_watch_hup) { g_source_remove (m_panel_watch_hup); m_panel_watch_hup = 0; }
    if (m_panel_iochannel) { g_io_channel_unref (m_panel_iochannel); m_panel_iochannel = 0; }
    m_panel_client.close_connection ();
}

/* ------------------------------------------------------------------ */
/* Engine events -> backend                                            */
/* ------------------------------------------------------------------ */

void
IBusFrontEnd::engine_process_key (IBusEngineData *d, guint keyval, guint keycode,
                                  guint state, gboolean *consumed)
{
    uint16 mask = 0;
    if (state & IBUS_SHIFT_MASK)   mask |= SCIM_KEY_ShiftMask;
    if (state & IBUS_LOCK_MASK)    mask |= SCIM_KEY_CapsLockMask;
    if (state & IBUS_CONTROL_MASK) mask |= SCIM_KEY_ControlMask;
    if (state & IBUS_MOD1_MASK)    mask |= SCIM_KEY_AltMask;
    if (state & IBUS_MOD4_MASK)    mask |= SCIM_KEY_SuperMask;
    if (state & IBUS_MOD5_MASK)    mask |= SCIM_KEY_NumLockMask;
    if (state & IBUS_RELEASE_MASK) mask |= SCIM_KEY_ReleaseMask;

    // IBus keyvals are X11 keysyms, matching scim's key codes.
    KeyEvent scimkey (static_cast<uint32> (keyval), mask);

    m_current       = d;
    d->cur_keyval   = keyval;
    d->cur_keycode  = keycode;
    d->cur_state    = state;
    d->cur_forwarded = false;

    bool ate = process_key_event (d->siid, scimkey);

    m_current = 0;
    *consumed = ate ? TRUE : FALSE;
}

void
IBusFrontEnd::engine_focus_in (IBusEngineData *d)
{
    m_current = d;
    focus_in (d->siid);

    // Bring up the toolbar for this engine's properties.
    if (m_panel_client.get_connection_number () >= 0) {
        m_panel_focus_siid = d->siid;
        m_panel_client.prepare (d->siid);
        m_panel_client.register_input_context (d->siid, get_instance_uuid (d->siid));
        m_panel_client.focus_in (d->siid, get_instance_uuid (d->siid));
        m_panel_client.turn_on (d->siid);
        m_panel_client.send ();
    }
}

void
IBusFrontEnd::engine_focus_out (IBusEngineData *d)
{
    focus_out (d->siid);

    if (m_panel_client.get_connection_number () >= 0) {
        m_panel_client.prepare (d->siid);
        m_panel_client.turn_off (d->siid);
        m_panel_client.focus_out (d->siid);
        m_panel_client.send ();
    }
    if (m_panel_focus_siid == d->siid)
        m_panel_focus_siid = -1;

    if (m_current == d)
        m_current = 0;
}

void
IBusFrontEnd::engine_reset (IBusEngineData *d)
{
    reset (d->siid);
}

void
IBusFrontEnd::engine_page_up (IBusEngineData *d)
{
    lookup_table_page_up (d->siid);
}

void
IBusFrontEnd::engine_page_down (IBusEngineData *d)
{
    lookup_table_page_down (d->siid);
}

void
IBusFrontEnd::engine_cursor_up (IBusEngineData * /*d*/)
{
}

void
IBusFrontEnd::engine_cursor_down (IBusEngineData * /*d*/)
{
}

void
IBusFrontEnd::engine_candidate_clicked (IBusEngineData *d, guint index,
                                        guint /*button*/, guint /*state*/)
{
    select_candidate (d->siid, static_cast<int> (index));
}

/* ------------------------------------------------------------------ */
/* Backend -> IBus engine                                              */
/* ------------------------------------------------------------------ */

void
IBusFrontEnd::send_preedit (IBusEngineData *d)
{
    if (!d) return;
    String utf8 = utf8_wcstombs (d->preedit_str);
    IBusText *text = ibus_text_new_from_string (utf8.c_str ());
    ibus_engine_update_preedit_text (d->engine, text,
                                     static_cast<guint> (d->preedit_caret),
                                     d->preedit_visible ? TRUE : FALSE);
}

void
IBusFrontEnd::show_preedit_string (int id)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;
    d->preedit_visible = true;
    send_preedit (d);
}

void
IBusFrontEnd::hide_preedit_string (int id)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;
    d->preedit_visible = false;
    ibus_engine_hide_preedit_text (d->engine);
}

void
IBusFrontEnd::update_preedit_caret (int id, int caret)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;
    d->preedit_caret = caret;
    send_preedit (d);
}

void
IBusFrontEnd::update_preedit_string (int id, const WideString & str,
                                     const AttributeList & /*attrs*/)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;
    d->preedit_str = str;
    if (d->preedit_caret > (int) str.length ())
        d->preedit_caret = (int) str.length ();
    // TODO(5e): map scim AttributeList -> IBusText attributes (underline etc).
    send_preedit (d);
}

void
IBusFrontEnd::show_aux_string (int id)
{
    IBusEngineData *d = find_engine (id);
    if (d) ibus_engine_show_auxiliary_text (d->engine);
}

void
IBusFrontEnd::hide_aux_string (int id)
{
    IBusEngineData *d = find_engine (id);
    if (d) ibus_engine_hide_auxiliary_text (d->engine);
}

void
IBusFrontEnd::update_aux_string (int id, const WideString & str,
                                 const AttributeList & /*attrs*/)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;
    IBusText *text = ibus_text_new_from_string (utf8_wcstombs (str).c_str ());
    ibus_engine_update_auxiliary_text (d->engine, text, TRUE);
}

void
IBusFrontEnd::show_lookup_table (int id)
{
    IBusEngineData *d = find_engine (id);
    if (d) ibus_engine_show_lookup_table (d->engine);
}

void
IBusFrontEnd::hide_lookup_table (int id)
{
    IBusEngineData *d = find_engine (id);
    if (d) ibus_engine_hide_lookup_table (d->engine);
}

void
IBusFrontEnd::update_lookup_table (int id, const LookupTable & table)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;

    int page = table.get_current_page_size ();
    guint cursor = table.is_cursor_visible ()
                 ? static_cast<guint> (table.get_cursor_pos_in_current_page ()) : 0;

    IBusLookupTable *lut = ibus_lookup_table_new (
        page > 0 ? page : 1, cursor, table.is_cursor_visible () ? TRUE : FALSE, TRUE);

    for (int i = 0; i < page; ++i) {
        IBusText *cand = ibus_text_new_from_string (
            utf8_wcstombs (table.get_candidate_in_current_page (i)).c_str ());
        ibus_lookup_table_append_candidate (lut, cand);
        IBusText *label = ibus_text_new_from_string (
            utf8_wcstombs (table.get_candidate_label (i)).c_str ());
        ibus_lookup_table_append_label (lut, label);
    }

    ibus_engine_update_lookup_table (d->engine, lut, TRUE);
}

void
IBusFrontEnd::commit_string (int id, const WideString & str)
{
    IBusEngineData *d = find_engine (id);
    if (!d) return;
    IBusText *text = ibus_text_new_from_string (utf8_wcstombs (str).c_str ());
    ibus_engine_commit_text (d->engine, text);
}

void
IBusFrontEnd::forward_key_event (int id, const KeyEvent & /*key*/)
{
    IBusEngineData *d = find_engine (id);
    if (!d || !d->engine) return;
    // Forward the physical key currently being processed.
    if (m_current == d && !d->cur_forwarded) {
        ibus_engine_forward_key_event (d->engine, d->cur_keyval, d->cur_keycode,
                                       d->cur_state);
        d->cur_forwarded = true;
    }
}

/* ------------------------------------------------------------------ */
/* Static libibus trampolines                                          */
/* ------------------------------------------------------------------ */

IBusEngine *
IBusFrontEnd::cb_create_engine (IBusFactory * /*factory*/, const gchar *engine_name,
                                gpointer user_data)
{
    return static_cast<IBusFrontEnd *> (user_data)->create_engine (engine_name);
}

gboolean
IBusFrontEnd::cb_process_key_event (IBusEngine * /*engine*/, guint keyval,
                                    guint keycode, guint state, gpointer user_data)
{
    IBusEngineData *d = static_cast<IBusEngineData *> (user_data);
    gboolean consumed = FALSE;
    d->frontend->engine_process_key (d, keyval, keycode, state, &consumed);
    return consumed;
}

void IBusFrontEnd::cb_focus_in  (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_focus_in (d); }
void IBusFrontEnd::cb_focus_out (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_focus_out (d); }
void IBusFrontEnd::cb_reset     (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_reset (d); }
void IBusFrontEnd::cb_enable    (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_focus_in (d); }
void IBusFrontEnd::cb_disable   (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_focus_out (d); }
void IBusFrontEnd::cb_page_up   (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_page_up (d); }
void IBusFrontEnd::cb_page_down (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_page_down (d); }
void IBusFrontEnd::cb_cursor_up   (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_cursor_up (d); }
void IBusFrontEnd::cb_cursor_down (IBusEngine *, gpointer u)
{ IBusEngineData *d = static_cast<IBusEngineData *> (u); d->frontend->engine_cursor_down (d); }

void
IBusFrontEnd::cb_candidate_clicked (IBusEngine *, guint index, guint button,
                                    guint state, gpointer u)
{
    IBusEngineData *d = static_cast<IBusEngineData *> (u);
    d->frontend->engine_candidate_clicked (d, index, button, state);
}

void
IBusFrontEnd::cb_engine_destroy (gpointer user_data, GObject * /*where*/)
{
    IBusEngineData *d = static_cast<IBusEngineData *> (user_data);
    // The engine object is being finalized; its pointer is now invalid.
    d->engine = 0;
    d->frontend->destroy_engine (d);
}

void
IBusFrontEnd::cb_disconnected (IBusBus * /*bus*/, gpointer /*user_data*/)
{
    ibus_quit ();
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
