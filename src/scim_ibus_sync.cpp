/** @file scim_ibus_sync.cpp
 * @brief Implementation of ScimIBusSync (see scim_ibus_sync.h).
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

#ifdef SCIM_HAS_GSETTINGS

#define Uses_SCIM_CONFIG_PATH
#define Uses_SCIM_GLOBAL_CONFIG
#define Uses_SCIM_UTILITY
#define Uses_SCIM_DEBUG

#include <scim.h>
#include <algorithm>
#include "scim_ibus_sync.h"

#define SCIM_GNOME_INPUT_SOURCES_SCHEMA   "org.gnome.desktop.input-sources"
#define SCIM_GNOME_INPUT_SOURCES_KEY      "sources"
#define SCIM_IBUS_SOURCE_TYPE             "ibus"

namespace scim {

/* --------------------------------------------------------------------------
 * small set helpers
 * ------------------------------------------------------------------------ */

static bool
contains (const std::vector<String> &v, const String &s)
{
    return std::find (v.begin (), v.end (), s) != v.end ();
}

static void
add_unique (std::vector<String> &v, const String &s)
{
    if (!contains (v, s)) v.push_back (s);
}

/* --------------------------------------------------------------------------
 * pure reconcile helpers
 * ------------------------------------------------------------------------ */

std::vector<String>
scim_sync_compute_disabled (const std::vector<String> &all_installed,
                            const std::vector<String> &gnome_scim)
{
    std::vector<String> disabled;
    for (size_t i = 0; i < all_installed.size (); ++i)
        if (!contains (gnome_scim, all_installed[i]))
            disabled.push_back (all_installed[i]);
    return disabled;
}

std::vector<InputSource>
scim_sync_compute_sources (const std::vector<InputSource> &current,
                           const std::vector<String>      &enabled,
                           const std::vector<String>      &all_installed)
{
    std::vector<InputSource> out;
    std::vector<String>      kept_scim;

    // Keep non-SCIM entries as-is; keep SCIM entries only if still enabled.
    for (size_t i = 0; i < current.size (); ++i) {
        const InputSource &s = current[i];
        bool is_scim = (s.first == SCIM_IBUS_SOURCE_TYPE) &&
                       contains (all_installed, s.second);
        if (!is_scim) {
            out.push_back (s);
        } else if (contains (enabled, s.second)) {
            out.push_back (s);
            add_unique (kept_scim, s.second);
        }
        // else: a SCIM source that is now disabled -> drop it
    }

    // Append newly-enabled SCIM engines that are not present yet.
    for (size_t i = 0; i < enabled.size (); ++i)
        if (!contains (kept_scim, enabled[i]))
            out.push_back (InputSource (String (SCIM_IBUS_SOURCE_TYPE), enabled[i]));

    return out;
}

/* --------------------------------------------------------------------------
 * ScimIBusSync
 * ------------------------------------------------------------------------ */

ScimIBusSync::ScimIBusSync (AllInstalledFunc all_installed, ReloadFunc reload)
    : m_all_installed (all_installed),
      m_reload (reload),
      m_config_monitor (0),
      m_config_handler (0),
      m_sources (0),
      m_sources_handler (0)
{
}

ScimIBusSync::~ScimIBusSync ()
{
    stop ();
}

void
ScimIBusSync::start ()
{
    // --- watch the global config file (where the disabled list lives) ---
    String path = scim_get_user_data_dir () + String ("/global");
    GFile *file = g_file_new_for_path (path.c_str ());
    m_config_monitor = g_file_monitor_file (file, G_FILE_MONITOR_NONE, 0, 0);
    g_object_unref (file);
    if (m_config_monitor)
        m_config_handler = g_signal_connect (m_config_monitor, "changed",
                                             G_CALLBACK (cb_config_changed), this);

    // --- watch GNOME's input-source list, if the schema is installed ---
    GSettingsSchemaSource *src = g_settings_schema_source_get_default ();
    if (src) {
        GSettingsSchema *schema =
            g_settings_schema_source_lookup (src, SCIM_GNOME_INPUT_SOURCES_SCHEMA, TRUE);
        if (schema) {
            m_sources = g_settings_new (SCIM_GNOME_INPUT_SOURCES_SCHEMA);
            m_sources_handler =
                g_signal_connect (m_sources, "changed::" SCIM_GNOME_INPUT_SOURCES_KEY,
                                  G_CALLBACK (cb_sources_changed), this);
            g_settings_schema_unref (schema);
        }
    }

    // Initial pass: UNION the two sides so neither's choices are lost. An
    // engine enabled in scim-setup OR present in GNOME's list becomes enabled
    // in both. (Strict mirroring, including removals, applies only to later
    // explicit changes handled by on_config_changed / on_sources_changed.)
    m_last_disabled = current_disabled ();
    if (m_sources)
        initial_union_sync ();
}

void
ScimIBusSync::initial_union_sync ()
{
    std::vector<String> all = all_installed ();

    // Safety: if we cannot determine the installed set (e.g. the component XML
    // is missing/unreadable), do nothing rather than risk clearing the config.
    if (all.empty ())
        return;

    // union = enabled-in-scim  OR  present-in-GNOME
    std::vector<String> keep = enabled ();
    std::vector<InputSource> src = read_sources ();
    for (size_t i = 0; i < src.size (); ++i)
        if (src[i].first == SCIM_IBUS_SOURCE_TYPE && contains (all, src[i].second))
            add_unique (keep, src[i].second);

    // Enable the whole union in scim (disabled = all - keep), if that differs.
    std::vector<String> new_disabled = scim_sync_compute_disabled (all, keep);
    std::vector<String> cur_disabled = current_disabled ();
    if (new_disabled != cur_disabled) {
        scim_global_config_write (String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES),
                                  new_disabled);
        scim_global_config_flush ();
        m_last_disabled = new_disabled;
        m_reload ();
    }

    // Mirror the (now enabled) union into GNOME's source list.
    push_scim_to_gnome ();
}

void
ScimIBusSync::stop ()
{
    if (m_config_monitor) {
        if (m_config_handler)
            g_signal_handler_disconnect (m_config_monitor, m_config_handler);
        g_object_unref (m_config_monitor);
        m_config_monitor = 0;
        m_config_handler = 0;
    }
    if (m_sources) {
        if (m_sources_handler)
            g_signal_handler_disconnect (m_sources, m_sources_handler);
        g_object_unref (m_sources);
        m_sources = 0;
        m_sources_handler = 0;
    }
}

/* --------------------------------------------------------------------------
 * config / GNOME accessors
 * ------------------------------------------------------------------------ */

std::vector<String>
ScimIBusSync::current_disabled ()
{
    scim_global_config_reload ();
    std::vector<String> d;
    d = scim_global_config_read (String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES), d);
    return d;
}

std::vector<String>
ScimIBusSync::all_installed ()
{
    return m_all_installed ();
}

std::vector<String>
ScimIBusSync::enabled ()
{
    std::vector<String> all = all_installed ();
    std::vector<String> dis = current_disabled ();
    std::vector<String> en;
    for (size_t i = 0; i < all.size (); ++i)
        if (!contains (dis, all[i]))
            en.push_back (all[i]);
    return en;
}

std::vector<InputSource>
ScimIBusSync::read_sources ()
{
    std::vector<InputSource> out;
    if (!m_sources) return out;

    GVariant *v = g_settings_get_value (m_sources, SCIM_GNOME_INPUT_SOURCES_KEY);
    if (!v) return out;

    GVariantIter iter;
    const gchar *type = 0, *id = 0;
    g_variant_iter_init (&iter, v);
    while (g_variant_iter_next (&iter, "(&s&s)", &type, &id))
        out.push_back (InputSource (String (type ? type : ""), String (id ? id : "")));

    g_variant_unref (v);
    return out;
}

void
ScimIBusSync::write_sources (const std::vector<InputSource> &s)
{
    if (!m_sources) return;

    GVariantBuilder b;
    g_variant_builder_init (&b, G_VARIANT_TYPE ("a(ss)"));
    for (size_t i = 0; i < s.size (); ++i)
        g_variant_builder_add (&b, "(ss)", s[i].first.c_str (), s[i].second.c_str ());

    g_settings_set_value (m_sources, SCIM_GNOME_INPUT_SOURCES_KEY, g_variant_builder_end (&b));
    g_settings_sync ();
}

/* --------------------------------------------------------------------------
 * sync directions
 * ------------------------------------------------------------------------ */

void
ScimIBusSync::on_config_changed ()
{
    std::vector<String> d = current_disabled ();

    // Our own pull-write echoing back through the file monitor -- already
    // applied, skip to avoid a redundant reload.
    if (d == m_last_disabled)
        return;

    m_last_disabled = d;
    m_reload ();                 // reconcile the backend to the new disabled list
    if (m_sources)
        push_scim_to_gnome ();   // mirror the change into GNOME's source list
}

void
ScimIBusSync::push_scim_to_gnome ()
{
    if (!m_sources) return;

    std::vector<InputSource> current = read_sources ();
    std::vector<InputSource> desired =
        scim_sync_compute_sources (current, enabled (), all_installed ());

    if (desired != current)
        write_sources (desired);
}

void
ScimIBusSync::on_sources_changed ()
{
    if (!m_sources) return;

    // scim uuids the user keeps in GNOME's list.
    std::vector<String> all = all_installed ();

    // Safety: without a known installed set, do not touch the disabled list
    // (avoids wiping it when the component XML is unreadable).
    if (all.empty ())
        return;

    std::vector<InputSource> src = read_sources ();
    std::vector<String> gnome_scim;
    for (size_t i = 0; i < src.size (); ++i)
        if (src[i].first == SCIM_IBUS_SOURCE_TYPE && contains (all, src[i].second))
            add_unique (gnome_scim, src[i].second);

    std::vector<String> new_disabled = scim_sync_compute_disabled (all, gnome_scim);
    std::vector<String> cur_disabled = current_disabled ();

    if (new_disabled == cur_disabled)
        return;                  // nothing changed (or our own push echo)

    scim_global_config_write (String (SCIM_GLOBAL_CONFIG_DISABLED_IMENGINE_FACTORIES),
                              new_disabled);
    scim_global_config_flush ();
    m_last_disabled = new_disabled;   // memo so the file-monitor echo is skipped
    m_reload ();
}

/* --------------------------------------------------------------------------
 * static trampolines
 * ------------------------------------------------------------------------ */

void
ScimIBusSync::cb_config_changed (GFileMonitor * /*mon*/, GFile * /*file*/, GFile * /*other*/,
                                 GFileMonitorEvent event, gpointer user_data)
{
    // Coalesce the burst a non-atomic write produces to a single reaction.
    if (event != G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT &&
        event != G_FILE_MONITOR_EVENT_CREATED &&
        event != G_FILE_MONITOR_EVENT_RENAMED)
        return;
    static_cast<ScimIBusSync *> (user_data)->on_config_changed ();
}

void
ScimIBusSync::cb_sources_changed (GSettings * /*settings*/, gchar * /*key*/, gpointer user_data)
{
    static_cast<ScimIBusSync *> (user_data)->on_sources_changed ();
}

} // namespace scim

#endif // SCIM_HAS_GSETTINGS

/*
vi:ts=4:nowrap:ai:expandtab
*/
