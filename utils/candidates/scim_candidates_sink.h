/**
 * @file scim_candidates_sink.h
 * @brief One interface for every candidate UI a frontend can present with.
 *
 * A frontend has two kinds of candidate UI available: one it draws itself (the
 * Cairo renderer on a surface it owns) and one it delegates to the desktop (KDE
 * kimpanel over D-Bus). Both consume the same stream of input-method state and
 * both report the same handful of user actions back, so they are the same
 * interface with two implementations rather than two code paths at every call
 * site.
 *
 * A host therefore picks a sink once, at startup, and pushes state at it
 * unconditionally afterwards. It never asks which kind it got.
 *
 * Deliberately free of cairo and of any display protocol: the kimpanel
 * implementation needs neither, and an IM module that owns a toolkit surface
 * implements this against that surface instead.
 *
 * Event integration is part of the interface (event_fd(), process_events())
 * because every implementation has some descriptor to watch -- an X connection,
 * a D-Bus connection -- and the host has to fold it into whatever loop it runs:
 * select() in the frontends, g_unix_fd_add() or QSocketNotifier in an IM module.
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
 * @brief A candidate UI: preedit, aux string and lookup table in, clicks out.
 */
class CandidatesSink
{
public:
    /** @brief Callback carrying an integer (candidate index / caret position). */
    typedef std::function<void (int)> IntSlot;
    /** @brief Parameterless callback (page flip). */
    typedef std::function<void ()>    VoidSlot;

    virtual ~CandidatesSink () { }

    /**
     * @brief The input method became active or inactive on the focused client.
     *
     * A delegated panel is told so it can show or withdraw itself; an
     * implementation that owns a surface takes it down instead.
     */
    virtual void enable (bool enabled) = 0;

    /** @name Input-method state @{ */
    virtual void update_preedit_string (const WideString &str,
                                        const AttributeList &attrs) = 0;
    virtual void update_preedit_caret  (int caret) = 0;
    virtual void show_preedit_string   (bool visible) = 0;
    virtual void update_aux_string     (const WideString &str,
                                        const AttributeList &attrs) = 0;
    virtual void show_aux_string       (bool visible) = 0;
    virtual void update_lookup_table   (const LookupTable &table) = 0;
    virtual void show_lookup_table     (bool visible) = 0;

    /**
     * @brief Where the text cursor is, in screen coordinates.
     *
     * Required by an implementation that has to position a window of its own
     * somewhere on the screen. One whose surface the compositor places, or which
     * is anchored to a toolkit widget, ignores it.
     */
    virtual void update_spot_location  (int x, int y) = 0;
    /** @} */

    /**
     * @name Engine indicator
     *
     * Only a delegated panel draws one; locally the tray icon does, so these
     * default to doing nothing.
     * @{
     */
    virtual void update_engine_property (const String &symbol, const String &name)
    { (void) symbol; (void) name; }
    virtual void remove_engine_property () { }
    /** @} */

    /**
     * @name Event integration
     *
     * @return event_fd(): a descriptor the host should watch, or -1 for none.
     * @{
     */
    virtual int  event_fd () const { return -1; }
    virtual void process_events () { }
    /** @} */

    /** @name User actions (sink -> host) @{ */
    virtual void signal_connect_select_candidate   (IntSlot slot) = 0;
    virtual void signal_connect_page_up            (VoidSlot slot) = 0;
    virtual void signal_connect_page_down          (VoidSlot slot) = 0;
    /** @brief Only a panel that draws the preedit can report a caret move. */
    virtual void signal_connect_move_preedit_caret (IntSlot slot) { (void) slot; }
    /** @} */
};

} // namespace scim

/*
vi:ts=4:nowrap:ai:expandtab
*/
