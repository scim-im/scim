/**
 * @file scim_kimpanel_agent.h
 * @brief KDE kimpanel candidate-UI backend (org.kde.kimpanel.inputmethod).
 *
 * A delegated candidate UI: instead of drawing candidates ourselves, push the
 * preedit / aux / lookup-table state to KDE Plasma's kimpanel over D-Bus and
 * let Plasma render the window. Works on both X11 and Wayland (D-Bus is
 * DE-level), so either frontend can select it on KDE.
 *
 * Uses libdbus with a private connection whose fd integrates into the
 * frontend's select() loop via connection_number()/process_events().
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

#ifndef Uses_SCIM_ATTRIBUTE
#define Uses_SCIM_ATTRIBUTE
#endif
#ifndef Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_LOOKUP_TABLE
#endif
#include <scim.h>
#include <functional>

namespace scim {

/**
 * @brief D-Bus client speaking the kimpanel input-method protocol.
 */
class KimpanelAgent
{
    class KimpanelAgentImpl;
    KimpanelAgentImpl *m_impl;

    KimpanelAgent (const KimpanelAgent &);
    const KimpanelAgent & operator = (const KimpanelAgent &);

public:
    /** @brief Callback carrying an integer (candidate index / caret pos). */
    typedef std::function<void (int)> IntSlot;
    /** @brief Parameterless callback (page flip, exit). */
    typedef std::function<void ()>    VoidSlot;

    KimpanelAgent ();
    ~KimpanelAgent ();

    /**
     * @brief Whether the running desktop prefers kimpanel (i.e. is KDE/Plasma),
     *        based on XDG_CURRENT_DESKTOP / KDE_FULL_SESSION.
     */
    static bool desktop_prefers_kimpanel ();

    /**
     * @brief Connect to the session bus, own the IM name and subscribe to
     *        panel signals.
     * @return true on success.
     */
    bool connect ();
    void close ();
    bool is_connected () const;

    /** @brief D-Bus connection fd for the caller's select loop (-1 if closed). */
    int  connection_number () const;
    /** @brief Non-blocking read + dispatch of pending D-Bus messages. */
    void process_events ();

    /** @name State push (frontend -> panel) @{ */
    void enable               (bool enabled);
    void update_preedit_string (const WideString &str);
    void update_preedit_caret (int caret);
    void show_preedit_string  (bool visible);
    void update_aux_string    (const WideString &str);
    void show_aux_string      (bool visible);
    void update_lookup_table  (const LookupTable &table);
    void show_lookup_table    (bool visible);
    void update_spot_location (int x, int y);

    /**
     * @brief Advertise the engine indicator property to the panel.
     *
     * kimpanel renders a property's label as text in the panel's own colors,
     * which is why the engine symbol is passed here rather than an icon: an
     * icon is fixed pixels and cannot follow a light or dark theme.
     *
     * @param symbol a few characters identifying the engine, drawn by the panel.
     * @param name   the full engine name, used as the tooltip.
     */
    void update_engine_property (const String &symbol, const String &name);

    /** @brief Withdraw the engine indicator property. */
    void remove_engine_property ();
    /** @} */

    /** @name Panel -> frontend callbacks @{ */
    void signal_connect_select_candidate   (IntSlot slot);
    void signal_connect_page_up            (VoidSlot slot);
    void signal_connect_page_down          (VoidSlot slot);
    void signal_connect_move_preedit_caret (IntSlot slot);
    void signal_connect_exit               (VoidSlot slot);
    /** @brief The user clicked the engine indicator in the panel. */
    void signal_connect_trigger_engine     (VoidSlot slot);
    /** @} */
};

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
