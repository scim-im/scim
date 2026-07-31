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

#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_UTILITY
#define Uses_SCIM_DEBUG
#include "scim_private.h"
#include <scim.h>

#include "scim_kimpanel_agent.h"

#include <dbus/dbus.h>
#include <vector>
#include <string>
#include <cstdlib>

namespace scim {

// kimpanel D-Bus addresses.
#define KIMPANEL_IM_PATH       "/kimpanel"
#define KIMPANEL_IM_INTERFACE  "org.kde.kimpanel.inputmethod"
#define KIMPANEL_IM_NAME       "org.kde.kimpanel.inputmethod"
#define KIMPANEL_PANEL_IFACE   "org.kde.impanel"
// The panel widget owns this name. Nothing renders our candidates without it.
#define KIMPANEL_PANEL_NAME    "org.kde.impanel"

// Key of the engine indicator property. Properties are colon separated fields
// -- "key:label:icon:tooltip:hint" -- so the key itself must not contain ':'.
#define KIMPANEL_ENGINE_PROP_KEY "/SCIM/Engine"

class KimpanelAgent::KimpanelAgentImpl
{
public:
    DBusConnection *m_conn;

    IntSlot   m_select_candidate;
    VoidSlot  m_page_up;
    VoidSlot  m_page_down;
    IntSlot   m_move_caret;
    VoidSlot  m_exit;
    VoidSlot  m_trigger_engine;

    bool      m_engine_registered;

    KimpanelAgentImpl () : m_conn (0), m_engine_registered (false) { }

    ~KimpanelAgentImpl () { close (); }

    void close ()
    {
        if (m_conn) {
            dbus_connection_remove_filter (m_conn, filter_trampoline, this);
            dbus_connection_close (m_conn);
            dbus_connection_unref (m_conn);
            m_conn = 0;
        }
    }

    bool connect ()
    {
        if (m_conn)
            return true;

        DBusError err;
        dbus_error_init (&err);

        m_conn = dbus_bus_get_private (DBUS_BUS_SESSION, &err);
        if (!m_conn || dbus_error_is_set (&err)) {
            SCIM_DEBUG_FRONTEND(1) << "kimpanel -- cannot connect to session bus: "
                                   << (err.message ? err.message : "?") << "\n";
            dbus_error_free (&err);
            if (m_conn) { dbus_connection_unref (m_conn); m_conn = 0; }
            return false;
        }

        dbus_connection_set_exit_on_disconnect (m_conn, FALSE);

        // Refuse to take this path when no panel widget is listening. kimpanel
        // is display-only: we emit signals and something else draws them, so
        // with no host every candidate update is silently discarded and the
        // user sees a preedit with no candidate list at all. Failing here lets
        // the caller fall back to its own candidate window instead.
        //
        // Checked once, at startup: a panel widget added to the desktop later
        // will not be picked up until the input method restarts.
        if (!dbus_bus_name_has_owner (m_conn, KIMPANEL_PANEL_NAME, &err)) {
            SCIM_DEBUG_FRONTEND(1) << "kimpanel -- no " KIMPANEL_PANEL_NAME
                                      " on the bus; not using kimpanel.\n";
            dbus_error_free (&err);
            dbus_connection_close (m_conn);
            dbus_connection_unref (m_conn);
            m_conn = 0;
            return false;
        }
        dbus_error_free (&err);

        // Own the input-method name so the panel knows an IM is present.
        dbus_bus_request_name (m_conn, KIMPANEL_IM_NAME,
                               DBUS_NAME_FLAG_REPLACE_EXISTING |
                               DBUS_NAME_FLAG_ALLOW_REPLACEMENT,
                               &err);
        dbus_error_free (&err);

        // Listen for panel -> IM signals.
        dbus_bus_add_match (m_conn,
                            "type='signal',interface='" KIMPANEL_PANEL_IFACE "'",
                            &err);
        dbus_error_free (&err);
        dbus_connection_flush (m_conn);

        dbus_connection_add_filter (m_conn, filter_trampoline, this, 0);
        return true;
    }

    /* --- outgoing signals --- */

    void emit_bool (const char *name, bool value)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, name);
        if (!msg) return;
        dbus_bool_t v = value ? TRUE : FALSE;
        dbus_message_append_args (msg, DBUS_TYPE_BOOLEAN, &v, DBUS_TYPE_INVALID);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    void emit_int (const char *name, int value)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, name);
        if (!msg) return;
        dbus_int32_t v = value;
        dbus_message_append_args (msg, DBUS_TYPE_INT32, &v, DBUS_TYPE_INVALID);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    void emit_text_attr (const char *name, const String &text)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, name);
        if (!msg) return;
        const char *t = text.c_str ();
        const char *a = "";
        dbus_message_append_args (msg, DBUS_TYPE_STRING, &t,
                                  DBUS_TYPE_STRING, &a, DBUS_TYPE_INVALID);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    void emit_two_ints (const char *name, int a, int b)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, name);
        if (!msg) return;
        dbus_int32_t ia = a, ib = b;
        dbus_message_append_args (msg, DBUS_TYPE_INT32, &ia,
                                  DBUS_TYPE_INT32, &ib, DBUS_TYPE_INVALID);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    static void append_string_array (DBusMessageIter *iter,
                                     const std::vector<String> &items)
    {
        DBusMessageIter arr;
        dbus_message_iter_open_container (iter, DBUS_TYPE_ARRAY, "s", &arr);
        for (size_t i = 0; i < items.size (); ++i) {
            const char *s = items[i].c_str ();
            dbus_message_iter_append_basic (&arr, DBUS_TYPE_STRING, &s);
        }
        dbus_message_iter_close_container (iter, &arr);
    }

    void emit_lookup_table (const std::vector<String> &labels,
                            const std::vector<String> &texts,
                            const std::vector<String> &attrs,
                            bool has_prev, bool has_next)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, "UpdateLookupTable");
        if (!msg) return;
        DBusMessageIter iter;
        dbus_message_iter_init_append (msg, &iter);
        append_string_array (&iter, labels);
        append_string_array (&iter, texts);
        append_string_array (&iter, attrs);
        dbus_bool_t bp = has_prev ? TRUE : FALSE;
        dbus_bool_t bn = has_next ? TRUE : FALSE;
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &bp);
        dbus_message_iter_append_basic (&iter, DBUS_TYPE_BOOLEAN, &bn);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    void emit_string (const char *name, const String &value)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, name);
        if (!msg) return;
        const char *s = value.c_str ();
        dbus_message_append_args (msg, DBUS_TYPE_STRING, &s, DBUS_TYPE_INVALID);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    void emit_string_array (const char *name, const std::vector<String> &items)
    {
        if (!m_conn) return;
        DBusMessage *msg = dbus_message_new_signal (
            KIMPANEL_IM_PATH, KIMPANEL_IM_INTERFACE, name);
        if (!msg) return;
        DBusMessageIter iter;
        dbus_message_iter_init_append (msg, &iter);
        append_string_array (&iter, items);
        dbus_connection_send (m_conn, msg, 0);
        dbus_message_unref (msg);
    }

    void flush ()
    {
        if (m_conn)
            dbus_connection_flush (m_conn);
    }

    /* --- incoming signals --- */

    static DBusHandlerResult filter_trampoline (DBusConnection *,
                                                DBusMessage *msg, void *data)
    {
        return static_cast<KimpanelAgentImpl *> (data)->filter (msg);
    }

    DBusHandlerResult filter (DBusMessage *msg)
    {
        if (dbus_message_is_signal (msg, KIMPANEL_PANEL_IFACE, "SelectCandidate")) {
            int idx = get_int_arg (msg);
            if (m_select_candidate) m_select_candidate (idx);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (dbus_message_is_signal (msg, KIMPANEL_PANEL_IFACE, "LookupTablePageUp")) {
            if (m_page_up) m_page_up ();
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (dbus_message_is_signal (msg, KIMPANEL_PANEL_IFACE, "LookupTablePageDown")) {
            if (m_page_down) m_page_down ();
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (dbus_message_is_signal (msg, KIMPANEL_PANEL_IFACE, "MovePreeditCaret")) {
            int pos = get_int_arg (msg);
            if (m_move_caret) m_move_caret (pos);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (dbus_message_is_signal (msg, KIMPANEL_PANEL_IFACE, "TriggerProperty")) {
            // The panel echoes back the whole property key it was given.
            const char *key = 0;
            DBusError err;
            dbus_error_init (&err);
            if (dbus_message_get_args (msg, &err, DBUS_TYPE_STRING, &key,
                                       DBUS_TYPE_INVALID) &&
                key && String (key) == String (KIMPANEL_ENGINE_PROP_KEY)) {
                if (m_trigger_engine) m_trigger_engine ();
            }
            dbus_error_free (&err);
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        if (dbus_message_is_signal (msg, KIMPANEL_PANEL_IFACE, "Exit")) {
            if (m_exit) m_exit ();
            return DBUS_HANDLER_RESULT_HANDLED;
        }
        return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
    }

    static int get_int_arg (DBusMessage *msg)
    {
        DBusMessageIter iter;
        if (!dbus_message_iter_init (msg, &iter))
            return 0;
        if (dbus_message_iter_get_arg_type (&iter) != DBUS_TYPE_INT32)
            return 0;
        dbus_int32_t v = 0;
        dbus_message_iter_get_basic (&iter, &v);
        return v;
    }

    void process_events ()
    {
        if (!m_conn) return;
        dbus_connection_read_write (m_conn, 0);
        while (dbus_connection_dispatch (m_conn) == DBUS_DISPATCH_DATA_REMAINS)
            ;
    }
};

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

KimpanelAgent::KimpanelAgent ()
    : m_impl (new KimpanelAgentImpl ())
{
}

KimpanelAgent::~KimpanelAgent ()
{
    delete m_impl;
}

bool
KimpanelAgent::desktop_prefers_kimpanel ()
{
    const char *xdg = getenv ("XDG_CURRENT_DESKTOP");
    if (xdg) {
        String s (xdg);
        if (s.find ("KDE") != String::npos || s.find ("kde") != String::npos)
            return true;
    }
    if (getenv ("KDE_FULL_SESSION"))
        return true;
    return false;
}

bool
KimpanelAgent::connect ()
{
    return m_impl->connect ();
}

void
KimpanelAgent::close ()
{
    m_impl->close ();
}

bool
KimpanelAgent::is_connected () const
{
    return m_impl->m_conn != 0;
}

int
KimpanelAgent::connection_number () const
{
    if (!m_impl->m_conn)
        return -1;
    int fd = -1;
    if (!dbus_connection_get_unix_fd (m_impl->m_conn, &fd))
        return -1;
    return fd;
}

void
KimpanelAgent::process_events ()
{
    m_impl->process_events ();
}

void
KimpanelAgent::enable (bool enabled)
{
    m_impl->emit_bool ("Enable", enabled);
    m_impl->flush ();
}

void
KimpanelAgent::update_preedit_string (const WideString &str)
{
    m_impl->emit_text_attr ("UpdatePreeditText", utf8_wcstombs (str));
    m_impl->flush ();
}

void
KimpanelAgent::update_preedit_caret (int caret)
{
    m_impl->emit_int ("UpdatePreeditCaret", caret);
    m_impl->flush ();
}

void
KimpanelAgent::show_preedit_string (bool visible)
{
    m_impl->emit_bool ("ShowPreedit", visible);
    m_impl->flush ();
}

void
KimpanelAgent::update_aux_string (const WideString &str)
{
    m_impl->emit_text_attr ("UpdateAux", utf8_wcstombs (str));
    m_impl->flush ();
}

void
KimpanelAgent::show_aux_string (bool visible)
{
    m_impl->emit_bool ("ShowAux", visible);
    m_impl->flush ();
}

void
KimpanelAgent::update_lookup_table (const LookupTable &table)
{
    std::vector<String> labels, texts, attrs;
    int page = table.get_current_page_size ();
    for (int i = 0; i < page; ++i) {
        labels.push_back (utf8_wcstombs (table.get_candidate_label (i)));
        texts.push_back (utf8_wcstombs (table.get_candidate_in_current_page (i)));
        attrs.push_back (String ());
    }

    int start = table.get_current_page_start ();
    bool has_prev = start > 0;
    bool has_next = start + page < static_cast<int> (table.number_of_candidates ());

    m_impl->emit_lookup_table (labels, texts, attrs, has_prev, has_next);

    if (table.is_cursor_visible ())
        m_impl->emit_int ("UpdateLookupTableCursor",
                          table.get_cursor_pos_in_current_page ());
    m_impl->flush ();
}

void
KimpanelAgent::show_lookup_table (bool visible)
{
    m_impl->emit_bool ("ShowLookupTable", visible);
    m_impl->flush ();
}

void
KimpanelAgent::update_spot_location (int x, int y)
{
    m_impl->emit_two_ints ("UpdateSpotLocation", x, y);
    m_impl->flush ();
}

void
KimpanelAgent::update_engine_property (const String &symbol, const String &name)
{
    // "key:label:icon:tooltip:hint". The icon field is left empty on purpose:
    // given an icon the panel draws that instead of the label, and an icon
    // cannot adapt to the panel's light or dark colours the way text does.
    String prop = String (KIMPANEL_ENGINE_PROP_KEY) + ":" +
                  symbol + "::" + name + ":";

    if (!m_impl->m_engine_registered) {
        std::vector<String> props;
        props.push_back (prop);
        m_impl->emit_string_array ("RegisterProperties", props);
        m_impl->m_engine_registered = true;
    }

    m_impl->emit_string ("UpdateProperty", prop);
    m_impl->flush ();
}

void
KimpanelAgent::remove_engine_property ()
{
    if (!m_impl->m_engine_registered)
        return;

    m_impl->emit_string ("RemoveProperty", String (KIMPANEL_ENGINE_PROP_KEY));
    m_impl->m_engine_registered = false;
    m_impl->flush ();
}

void
KimpanelAgent::signal_connect_select_candidate (IntSlot slot)
{
    m_impl->m_select_candidate = slot;
}

void
KimpanelAgent::signal_connect_page_up (VoidSlot slot)
{
    m_impl->m_page_up = slot;
}

void
KimpanelAgent::signal_connect_page_down (VoidSlot slot)
{
    m_impl->m_page_down = slot;
}

void
KimpanelAgent::signal_connect_move_preedit_caret (IntSlot slot)
{
    m_impl->m_move_caret = slot;
}

void
KimpanelAgent::signal_connect_trigger_engine (VoidSlot slot)
{
    m_impl->m_trigger_engine = slot;
}

void
KimpanelAgent::signal_connect_exit (VoidSlot slot)
{
    m_impl->m_exit = slot;
}

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
